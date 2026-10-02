// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdbool.h>
#include <stdlib.h>

#include <systemd/sd-bus.h>
#include <systemd/sd-journal.h>

static const char *target = "kylin-wlcom.target";

static bool env_needs_drop(char *env)
{
    char *p = strchr(env, '=');
    if (!p) {
        return false;
    }

    *p = '\0';
    return getenv(env) == NULL &&
           (strcmp(env, "DISPLAY") == 0 || strcmp(env, "XAUTHORITY") == 0 ||
            strcmp(env, "WAYLAND_DISPLAY") == 0 || strcmp(env, "WAYLAND_SOCKET") == 0 ||
            strncmp(env, "XDG_", 4) == 0);
}

static void drop_session_envs(sd_bus *bus)
{
    sd_bus_message *reply = NULL;
    int r = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                               "org.freedesktop.DBus.Properties", "Get", NULL, &reply, "ss",
                               "org.freedesktop.systemd1.Manager", "Environment");
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to call Environment: %s", strerror(-r));
        return;
    }

    r = sd_bus_message_enter_container(reply, 'v', "as");
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to parse Environment: %s", strerror(-r));
        sd_bus_message_unref(reply);
        return;
    }

    char **env_vars = NULL;
    r = sd_bus_message_read_strv(reply, &env_vars);
    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to read array of Environment: %s", strerror(-r));
        return;
    }

    int num = 0;
    for (int i = 0; env_vars[i]; i++) {
        // keep only those that need to be dropped
        if (env_needs_drop(env_vars[i])) {
            env_vars[num++] = env_vars[i];
        } else {
            free(env_vars[i]);
        }
    }
    env_vars[num] = NULL;

    if (num == 0) {
        free(env_vars);
        return;
    }

    for (int i = 0; env_vars[i]; i++) {
        sd_journal_print(LOG_NOTICE, "Unset environment %s", env_vars[i]);
    }

    sd_bus_message *request = NULL;
    r = sd_bus_message_new_method_call(bus, &request, "org.freedesktop.systemd1",
                                       "/org/freedesktop/systemd1",
                                       "org.freedesktop.systemd1.Manager", "UnsetEnvironment");
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to create method UnsetEnvironment: %s", strerror(-r));
        goto finish;
    }

    r = sd_bus_message_append_strv(request, env_vars);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to append string array: %s", strerror(-r));
        goto finish;
    }

    r = sd_bus_call(bus, request, 0, NULL, NULL);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to call UnsetEnvironment: %s", strerror(-r));
    }

finish:
    sd_bus_message_unref(request);
    for (int i = 0; i < num; i++) {
        free(env_vars[i]);
    }
    free(env_vars);
}

static int properties_changed(sd_bus_message *m, void *userdata, sd_bus_error *error)
{
    const char *interface = NULL;
    int r = sd_bus_message_read_basic(m, 's', &interface);
    if (r < 0) {
        return r;
    }

    if (strcmp(interface, "org.freedesktop.systemd1.Unit") != 0) {
        return 0;
    }

    r = sd_bus_message_skip(m, "a{sv}as");
    if (r < 0) {
        return r;
    }

    char *active_state = NULL;
    r = sd_bus_get_property_string(sd_bus_message_get_bus(m), "org.freedesktop.systemd1",
                                   sd_bus_message_get_path(m), "org.freedesktop.systemd1.Unit",
                                   "ActiveState", NULL, &active_state);
    if (r < 0) {
        return r;
    }

    if (strcmp(active_state, "inactive") == 0) {
        sd_journal_print(LOG_NOTICE, "Target %s is inactive, Exiting...", target);
        sd_bus *bus = userdata;
        sd_event_exit(sd_bus_get_event(bus), 0);
        sd_bus_detach_event(bus);
    }

    free(active_state);
    return 0;
}

static int start_target(sd_bus *bus)
{
    int r = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager", "StartUnit", NULL, NULL, "ss",
                               target, "replace");
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to call StartUnit: %s", strerror(-r));
        return r;
    }

    sd_journal_print(LOG_NOTICE, "Successfully StartUnit: %s", target);

    char *path = NULL;
    r = sd_bus_path_encode("/org/freedesktop/systemd1/unit", target, &path);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to encode path: %s", strerror(-r));
        return r;
    }

    r = sd_bus_match_signal(bus, NULL, "org.freedesktop.systemd1", path,
                            "org.freedesktop.DBus.Properties", "PropertiesChanged",
                            properties_changed, bus);
    free(path);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to add match: %s", strerror(-r));
        return r;
    }

    r = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                           "org.freedesktop.systemd1.Manager", "Subscribe", NULL, NULL, NULL);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to subscribe: %s", strerror(-r));
    } else {
        sd_journal_print(LOG_NOTICE, "Listening for %s to become inactive...", target);
    }

    return r;
}

static void stop_target(sd_bus *bus)
{
    int r = sd_bus_call_method(bus, "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                               "org.freedesktop.systemd1.Manager", "StopUnit", NULL, NULL, "ss",
                               target, "replace");
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to call StopUnit: %s", strerror(-r));
    } else {
        sd_journal_print(LOG_NOTICE, "Successfully StopUnit: %s", target);
    }
}

static int on_signal(sd_event_source *s, const struct signalfd_siginfo *si, void *userdata)
{
    sd_journal_print(LOG_NOTICE, "Caught signal %d, Exiting...", si->ssi_signo);
    sd_bus *bus = userdata;
    sd_event_exit(sd_bus_get_event(bus), 0);
    sd_bus_detach_event(bus);
    return 0;
}

int main(int argc, char *argv[])
{
    sd_bus *bus = NULL;
    sd_event *event = NULL;

    int r = sd_bus_open_user(&bus);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to connect to session bus: %s", strerror(-r));
        goto finish;
    }

    // use sd-event for event-loop
    r = sd_event_default(&event);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to create event loop: %s", strerror(-r));
        goto finish;
    }

    r = sd_bus_attach_event(bus, event, SD_EVENT_PRIORITY_NORMAL);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Failed to attach bus to event loop: %s", strerror(-r));
        goto finish;
    }

    sd_event_add_signal(event, NULL, SIGTERM | SD_EVENT_SIGNAL_PROCMASK, on_signal, bus);

    drop_session_envs(bus);

    r = start_target(bus);
    if (r < 0) {
        goto finish;
    }

    r = sd_event_loop(event);
    if (r < 0) {
        sd_journal_print(LOG_ERR, "Event loop failed: %s", strerror(-r));
    }

    stop_target(bus);

finish:
    sd_journal_print(LOG_NOTICE, "Exited");
    sd_event_unref(event);
    sd_bus_unref(bus);
    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
