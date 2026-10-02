// SPDX-FileCopyrightText: 2026 The seatd contributors
// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-login.h>
#include <pthread.h>
#include <unistd.h>

#include <kywc/log.h>

#include "backend/session.h"
#include "session_p.h"

static const char *logind_dst = "org.freedesktop.login1";
static const char *logind_path = "/org/freedesktop/login1";
static const char *logind_interface_seat = "org.freedesktop.login1.Seat";
static const char *logind_interface_session = "org.freedesktop.login1.Session";
static const char *logind_interface_manager = "org.freedesktop.login1.Manager";

struct logind {
    sd_bus *bus;
    pthread_mutex_t mutex;

    struct session *session;

    bool active, initial_setup;
    char *id, *seat, *path, *seat_path;
};

static void _destroy(struct logind *logind)
{
    if (logind->bus) {
        sd_bus_unref(logind->bus);
    }
    pthread_mutex_destroy(&logind->mutex);
    free(logind->id);
    free(logind->seat);
    free(logind->path);
    free(logind->seat_path);
    free(logind);
}

static void logind_destroy(struct session *session)
{
    struct logind *logind = session->impl_data;
    pthread_mutex_lock(&logind->mutex);
    if (sd_bus_call_method_async(logind->bus, NULL, logind_dst, logind->path,
                                 logind_interface_session, "ReleaseControl", NULL, NULL,
                                 NULL) < 0) {
        kywc_log(KYWC_WARN, "Could not release control of session");
    }
    pthread_mutex_unlock(&logind->mutex);

    _destroy(logind);
}

static const char *seat_name(struct session *session)
{
    struct logind *logind = session->impl_data;
    return logind->seat;
}

static int ping_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    if (sd_bus_message_is_method_error(m, NULL)) {
        const sd_bus_error *error = sd_bus_message_get_error(m);
        kywc_log(KYWC_ERROR, "Ping failed: %s: %s", error->name, error->message);
        return -1;
    }
    return 0;
}

static void check_pending_events(struct logind *logind)
{
    uint64_t queued_read, queued_write;
    sd_bus_get_n_queued_read(logind->bus, &queued_read);
    sd_bus_get_n_queued_write(logind->bus, &queued_write);

    if (queued_read == 0 && queued_write == 0) {
        return;
    }

    // The sd_bus instance has queued data, so a dispatch is required.
    // However, we likely already drained our socket, so there will not be
    // anything to read. Instead, send a ping request to logind so that the
    // user will be woken up by its response.
    if (sd_bus_call_method_async(logind->bus, NULL, logind_dst, logind_path,
                                 "org.freedesktop.DBus.Peer", "Ping", ping_handler, logind,
                                 NULL) < 0) {
        kywc_log(KYWC_ERROR, "Could not send ping message");
        return;
    }
}

static int open_device(struct session *session, const char *path)
{
    int tmpfd = -1;
    sd_bus_message *msg = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    struct logind *logind = session->impl_data;

    pthread_mutex_lock(&logind->mutex);

    struct stat st;
    if (stat(path, &st) < 0) {
        kywc_log(KYWC_ERROR, "Could not stat path '%s'", path);
        return -1;
    }

    if (sd_bus_call_method(logind->bus, logind_dst, logind->path, logind_interface_session,
                           "TakeDevice", &error, &msg, "uu", major(st.st_rdev),
                           minor(st.st_rdev)) < 0) {
        kywc_log(KYWC_ERROR, "Could not take device: %s", error.message);
        goto out;
    }

    int paused = 0;
    if (sd_bus_message_read(msg, "hb", &tmpfd, &paused) < 0) {
        kywc_log(KYWC_ERROR, "Could not parse D-Bus response of TakeDevice");
        goto out;
    }

    // The original fd seems to be closed when the message is freed so we just clone it.
    tmpfd = fcntl(tmpfd, F_DUPFD_CLOEXEC, 0);

out:
    sd_bus_error_free(&error);
    sd_bus_message_unref(msg);
    check_pending_events(logind);
    pthread_mutex_unlock(&logind->mutex);
    return tmpfd;
}

static void close_device(struct session *session, int fd)
{
    struct stat st = { 0 };
    if (fstat(fd, &st) < 0) {
        kywc_log(KYWC_ERROR, "Could not stat fd %d", fd);
        return;
    }

    struct logind *logind = session->impl_data;
    pthread_mutex_lock(&logind->mutex);
    sd_bus_error error = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(logind->bus, logind_dst, logind->path, logind_interface_session,
                           "ReleaseDevice", &error, NULL, "uu", major(st.st_rdev),
                           minor(st.st_rdev)) < 0) {
        kywc_log(KYWC_ERROR, "Could not close device: %s", error.message);
    }
    close(fd);

    sd_bus_error_free(&error);
    check_pending_events(logind);
    pthread_mutex_unlock(&logind->mutex);
}

static bool change_vt(struct session *session, int vt)
{
    struct logind *logind = session->impl_data;
    pthread_mutex_lock(&logind->mutex);
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int ret = sd_bus_call_method(logind->bus, logind_dst, logind->seat_path, logind_interface_seat,
                                 "SwitchTo", &error, NULL, "u", (uint32_t)vt);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Could not switch session: %s", error.message);
    }
    sd_bus_error_free(&error);
    check_pending_events(logind);
    pthread_mutex_unlock(&logind->mutex);
    return ret;
}

static int get_fd(struct session *session)
{
    struct logind *logind = session->impl_data;
    int fd = sd_bus_get_fd(logind->bus);
    if (fd >= 0) {
        return fd;
    }

    errno = -fd;
    return -1;
}

static int poll_connection(struct logind *logind, int timeout)
{
    struct pollfd fd = {
        .fd = sd_bus_get_fd(logind->bus),
        .events = POLLIN,
    };

    if (poll(&fd, 1, timeout) == -1) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        } else {
            return -1;
        }
    }

    if (fd.revents & (POLLERR | POLLHUP)) {
        errno = ECONNRESET;
        return -1;
    }
    return 0;
}

static int dispatch_and_execute(struct session *session, int timeout)
{
    struct logind *logind = session->impl_data;
    pthread_mutex_lock(&logind->mutex);
    if (logind->initial_setup) {
        logind->initial_setup = false;
        if (logind->active) {
            session_set_active(logind->session);
        } else {
            session_set_inactive(logind->session);
        }
    }

    int total_dispatched = 0;
    int dispatched = 0;
    while ((dispatched = sd_bus_process(logind->bus, NULL)) > 0) {
        total_dispatched += dispatched;
    }
    if (total_dispatched == 0 && timeout != 0) {
        if (poll_connection(logind, timeout) == -1) {
            kywc_log(KYWC_ERROR, "Could not poll connection: %s", strerror(errno));
            return -1;
        }
        while ((dispatched = sd_bus_process(logind->bus, NULL)) > 0) {
            total_dispatched += dispatched;
        }
    }
    check_pending_events(logind);
    pthread_mutex_unlock(&logind->mutex);
    return total_dispatched;
}

static int session_activate(struct logind *logind)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int ret = sd_bus_call_method(logind->bus, logind_dst, logind->path, logind_interface_session,
                                 "Activate", &error, NULL, NULL);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Could not activate session: %s", error.message);
    }
    sd_bus_error_free(&error);
    return ret;
}

static int session_check_active(struct logind *logind, bool *active)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int ret = sd_bus_get_property_trivial(logind->bus, logind_dst, logind->path,
                                          logind_interface_session, "Active", &error, 'b', active);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Could not check if session is active: %s", error.message);
    }
    sd_bus_error_free(&error);
    return ret;
}

static int take_control(struct logind *logind)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int ret = sd_bus_call_method(logind->bus, logind_dst, logind->path, logind_interface_session,
                                 "TakeControl", &error, NULL, "b", false);
    if (ret < 0) {
        kywc_log(KYWC_ERROR, "Could not take control of session: %s", error.message);
    }
    sd_bus_error_free(&error);
    return ret;
}

static void set_active(struct logind *logind, bool active)
{
    if (logind->active == active) {
        return;
    }

    logind->active = active;
    kywc_log(KYWC_INFO, "%sabling seat", active ? "En" : "Dis");
    if (active) {
        session_set_active(logind->session);
    } else {
        session_set_inactive(logind->session);
    }
}

static int pause_device(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct logind *logind = userdata;

    uint32_t major, minor;
    const char *type;
    if (sd_bus_message_read(msg, "uus", &major, &minor, &type) < 0) {
        kywc_log(KYWC_ERROR, "Could not parse D-Bus response of PauseDevice");
        return 0;
    }

    if (strcmp(type, "gone") == 0) {
        kywc_log(KYWC_DEBUG, "Device removed: %d:%d", major, minor);
        return 0;
    }

    if (strcmp(type, "pause") == 0) {
        kywc_log(KYWC_DEBUG, "Device paused: %d:%d", major, minor);
        set_active(logind, false);
        if (sd_bus_call_method(logind->bus, logind_dst, logind->path, logind_interface_session,
                               "PauseDeviceComplete", ret_error, NULL, "uu", major, minor) < 0) {
            kywc_log(KYWC_ERROR, "Could not send PauseDeviceComplete signal: %s",
                     ret_error->message);
        }
    }

    return 0;
}

static int properties_changed(sd_bus_message *msg, void *userdata, sd_bus_error *ret_error)
{
    struct logind *logind = userdata;
    int ret = 0;

    // PropertiesChanged arg 1: interface
    const char *interface;
    ret = sd_bus_message_read_basic(msg, 's', &interface);
    if (ret < 0) {
        goto error;
    }

    if (strcmp(interface, logind_interface_session) != 0) {
        // not interesting for us; ignore
        return 0;
    }

    // PropertiesChanged arg 2: changed properties with values
    ret = sd_bus_message_enter_container(msg, 'a', "{sv}");
    if (ret < 0) {
        goto error;
    }

    const char *s;
    while ((ret = sd_bus_message_enter_container(msg, 'e', "sv")) > 0) {
        ret = sd_bus_message_read_basic(msg, 's', &s);
        if (ret < 0) {
            goto error;
        }

        if (strcmp(s, "Active") == 0) {
            const char *field = "Active";
            ret = sd_bus_message_enter_container(msg, 'v', "b");
            if (ret < 0) {
                goto error;
            }

            bool value;
            ret = sd_bus_message_read_basic(msg, 'b', &value);
            if (ret < 0) {
                goto error;
            }

            kywc_log(KYWC_DEBUG, "%s state changed: %d", field, value);
            set_active(logind, value);
            ret = sd_bus_message_exit_container(msg);
            if (ret < 0) {
                goto error;
            }
        } else {
            ret = sd_bus_message_skip(msg, "{sv}");
            if (ret < 0) {
                goto error;
            }
        }

        ret = sd_bus_message_exit_container(msg);
        if (ret < 0) {
            goto error;
        }
    }
    if (ret < 0) {
        goto error;
    }

    ret = sd_bus_message_exit_container(msg);
    if (ret < 0) {
        goto error;
    }

    // PropertiesChanged arg 3: changed properties without values
    ret = sd_bus_message_enter_container(msg, 'a', "s");
    if (ret < 0) {
        goto error;
    }

    while ((ret = sd_bus_message_read_basic(msg, 's', &s)) > 0) {
        if (strcmp(s, "Active") == 0) {
            sd_bus_error error = SD_BUS_ERROR_NULL;
            const char *field = "Active";
            bool value;
            ret = sd_bus_get_property_trivial(logind->bus, logind_dst, logind->path,
                                              logind_interface_session, field, &error, 'b', &value);
            if (ret < 0) {
                kywc_log(KYWC_ERROR, "Could not get '%s' property: %s", field, error.message);
                continue;
            }

            kywc_log(KYWC_DEBUG, "%s state changed: %d", field, value);
            set_active(logind, value);
        }
    }
    if (ret < 0) {
        goto error;
    }

    ret = sd_bus_message_exit_container(msg);

error:
    if (ret < 0) {
        kywc_log(KYWC_WARN, "Could not parse D-Bus PropertiesChanged: %s", strerror(-ret));
    }
    return 0;
}

static bool add_signal_matches(struct logind *logind)
{
    static const char *property_interface = "org.freedesktop.DBus.Properties";
    if (sd_bus_match_signal(logind->bus, NULL, logind_dst, logind->path, logind_interface_session,
                            "PauseDevice", pause_device, logind) < 0) {
        kywc_log(KYWC_ERROR, "Failed to add D-Bus match: PauseDevice");
        return false;
    }
    if (sd_bus_match_signal(logind->bus, NULL, logind_dst, logind->path, property_interface,
                            "PropertiesChanged", properties_changed, logind) < 0) {
        kywc_log(KYWC_ERROR, "Failed to add D-Bus match: PropertiesChanged");
        return false;
    }
    return true;
}

static bool find_session_path(struct logind *logind)
{
    bool ret = false;
    sd_bus_message *msg = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(logind->bus, logind_dst, logind_path, logind_interface_manager,
                           "GetSession", &error, &msg, "s", logind->id) < 0) {
        kywc_log(KYWC_ERROR, "Could not get session: %s", error.message);
        goto out;
    }

    const char *path;
    if (sd_bus_message_read(msg, "o", &path) < 0) {
        kywc_log(KYWC_ERROR, "Could not parse D-Bus response of GetSession");
        goto out;
    }
    logind->path = strdup(path);
    ret = true;

out:
    sd_bus_error_free(&error);
    sd_bus_message_unref(msg);
    return ret;
}

static bool find_seat_path(struct logind *logind)
{
    bool ret = false;
    sd_bus_message *msg = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(logind->bus, logind_dst, logind_path, logind_interface_manager,
                           "GetSeat", &error, &msg, "s", logind->seat) < 0) {
        kywc_log(KYWC_ERROR, "Could not get seat: %s", error.message);
        goto out;
    }

    const char *path;
    if (sd_bus_message_read(msg, "o", &path) < 0) {
        kywc_log(KYWC_ERROR, "Could not parse D-Bus response of GetSeat");
        goto out;
    }
    logind->seat_path = strdup(path);
    ret = true;

out:
    sd_bus_error_free(&error);
    sd_bus_message_unref(msg);
    return ret;
}

static bool get_display_session(char **session_id)
{
    if (!session_id) {
        return false;
    }

    char *xdg_session_id = getenv("XDG_SESSION_ID");
    if (xdg_session_id) {
        // Check whether the supplied session ID is valid.
        if (sd_session_is_active(xdg_session_id) < 0) {
            kywc_log(KYWC_ERROR, "Could not check session active");
            return false;
        }
        *session_id = strdup(xdg_session_id);
        return true;
    }

    // Use the active session of preocess.
    if (sd_pid_get_session(getpid(), session_id) == 0) {
        return true;
    }

    // Find any active sessions if the process isn't part of one.
    if (sd_uid_get_display(getuid(), session_id) < 0) {
        kywc_log(KYWC_ERROR, "Could not get primary session for user");
        free(*session_id);
        *session_id = NULL;
        return false;
    }

    return true;
}

static void set_type(struct logind *logind, const char *type)
{
    sd_bus_error error = SD_BUS_ERROR_NULL;
    if (sd_bus_call_method(logind->bus, logind_dst, logind->path, logind_interface_session,
                           "SetType", &error, NULL, "s", type) < 0) {
        kywc_log(KYWC_ERROR, "Could not set session type: %s", error.message);
    }
    sd_bus_error_free(&error);
}

bool logind_create(struct session *session)
{
    struct logind *logind = calloc(1, sizeof(*logind));
    if (!logind) {
        return false;
    }
    pthread_mutex_init(&logind->mutex, NULL);

    if (sd_bus_default_system(&logind->bus) < 0) {
        goto error;
    }
    if (!get_display_session(&logind->id)) {
        goto error;
    }
    if (sd_session_get_seat(logind->id, &logind->seat) < 0) {
        kywc_log(KYWC_ERROR, "Failed to get seat");
        goto error;
    }
    if (!find_session_path(logind)) {
        goto error;
    }
    if (!find_seat_path(logind)) {
        goto error;
    }
    if (!add_signal_matches(logind)) {
        goto error;
    }
    if (session_activate(logind) < 0) {
        goto error;
    }
    if (session_check_active(logind, &logind->active) < 0) {
        goto error;
    }
    if (take_control(logind) < 0) {
        goto error;
    }
    set_type(logind, "wayland");

    logind->initial_setup = true;
    logind->session = session;

    check_pending_events(logind);

    struct session_interface *impl = session->impl;
    impl->destroy = logind_destroy;
    impl->open = open_device;
    impl->close = close_device;
    impl->change_vt = change_vt;
    impl->seat_name = seat_name;
    impl->get_fd = get_fd;
    impl->dispatch = dispatch_and_execute;
    session->impl_data = logind;

    return true;

error:
    _destroy(logind);
    return false;
}
