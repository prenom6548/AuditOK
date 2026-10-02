// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <kywc/log.h>
#include <sys/socket.h>

#include "util/dialog.h"

#define BUFFER_SIZE 4096

json_object *dialog_get_reply(int socket_fd)
{
    if (socket_fd < 0) {
        return NULL;
    }

    size_t data_size = BUFFER_SIZE;
    char *response_data = malloc(data_size);
    if (!response_data) {
        kywc_log(KYWC_ERROR, "WLDIALOG: Failed to allocate memory\n");
        return NULL;
    }

    size_t total_bytes = 0;
    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t bytes_read = read(socket_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            /* data capacity expansion */
            if (total_bytes + bytes_read + 1 > data_size) {
                size_t new_size = data_size * 2;
                while (new_size < total_bytes + bytes_read + 1) {
                    new_size *= 2;
                }

                char *temp = realloc(response_data, new_size);
                if (!temp) {
                    kywc_log(KYWC_ERROR, "WLDIALOG: Failed to reallocate memory\n");
                    free(response_data);
                    return NULL;
                }
                response_data = temp;
                data_size = new_size;
            }

            memcpy(response_data + total_bytes, buffer, bytes_read);
            total_bytes += bytes_read;
            continue;
        } else if (bytes_read == 0) {
            kywc_log(KYWC_WARN, "WLDIALOG: fd %d closed\n", socket_fd);
            break;
        } else {
            if (errno == EAGAIN) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            kywc_log(KYWC_ERROR, "WLDIALOG: Error reading FD %d: %s\n", socket_fd, strerror(errno));
            free(response_data);
            return NULL;
        }
    }
    response_data[total_bytes] = '\0';
    kywc_log(KYWC_DEBUG, "Get socket data: %s", response_data);

    json_object *result = json_tokener_parse(response_data);
    free(response_data);
    if (!result) {
        kywc_log(KYWC_ERROR, "WLDIALOG: Failed to parse JSON from socket data");
        return NULL;
    }

    if (!json_object_is_type(result, json_type_object)) {
        kywc_log(KYWC_ERROR, "WLDIALOG: Parsed JSON is not an object");
        json_object_put(result);
        return NULL;
    }

    kywc_log(KYWC_DEBUG, "WLDIALOG: Extracted valid JSON: %s", json_object_to_json_string(result));
    return result;
}

static bool send_request(int socket_fd, const char *req)
{
    if (socket_fd < 0 || !req) {
        return false;
    }

    size_t len = strlen(req);
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(socket_fd, req + sent, len - sent);
        if (n <= 0) {
            goto fail;
        }
        sent += (size_t)n;
    }

    if (write(socket_fd, "\n", 1) != 1) {
        goto fail;
    }

    return true;

fail:
    kywc_log_errno(KYWC_ERROR, "WLDIALOG: Failed to send data");
    return false;
}

bool dialog_send_request(int socket_fd, json_object *req)
{
    if (socket_fd < 0 || !req) {
        return false;
    }
    const char *request = json_object_to_json_string(req);
    return send_request(socket_fd, request);
}

bool dialog_close(int socket_fd)
{
    json_object *obj = json_object_new_object();
    json_object_object_add(obj, "action", json_object_new_int64(0));
    json_object *control = json_object_new_object();
    json_object_object_add(control, "control", obj);

    const char *json_str = json_object_to_json_string(control);
    bool ret = send_request(socket_fd, json_str);
    json_object_put(control);

    return ret;
}
