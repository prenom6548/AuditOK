// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <unistd.h>

#include "util/string.h"
#include "xwayland_p.h"

#define COOKIE_SIZE 16
#define FAMILY_LOCAL 256
#define FAMILY_WILD 65535

static void generate_xauthority_cookie(unsigned char *cookie)
{
    static bool seeded = false;
    if (!seeded) {
        srand((uint32_t)time(NULL));
        seeded = true;
    }

    for (int i = 0; i < COOKIE_SIZE; i++) {
        cookie[i] = (unsigned char)(rand() & 0xFF);
    }
}

static int write_uint16_be(FILE *fp, uint16_t val)
{
    uint16_t net_val = htons(val);
    return (fwrite(&net_val, sizeof(uint16_t), 1, fp) == 1) ? 0 : -1;
}

static int write_string_entry(FILE *fp, const char *str)
{
    uint16_t len = str ? (uint16_t)strlen(str) : 0;

    if (write_uint16_be(fp, len) != 0) {
        return -1;
    }

    if (len > 0 && fwrite(str, 1, len, fp) != len) {
        return -1;
    }
    return 0;
}

#define CHECK_WRITE(expr)                                                                          \
    do {                                                                                           \
        if ((expr) != 0)                                                                           \
            return -1;                                                                             \
    } while (0)

static int write_xauthority_entry(FILE *fp, uint16_t family, const char *address,
                                  const char *display, const char *name,
                                  const unsigned char *cookie)
{
    CHECK_WRITE(write_uint16_be(fp, family));
    CHECK_WRITE(write_string_entry(fp, address));
    CHECK_WRITE(write_string_entry(fp, display));
    CHECK_WRITE(write_string_entry(fp, name));
    CHECK_WRITE(write_uint16_be(fp, COOKIE_SIZE));
    CHECK_WRITE(fwrite(cookie, 1, COOKIE_SIZE, fp) != COOKIE_SIZE ? -1 : 0);
    return 0;
}

char *generate_xauthority_file(int display)
{
    char hostname[256] = { 0 };
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        kywc_log(KYWC_ERROR, "get hostname failed");
        return false;
    }

    char display_str[16];
    int n = snprintf(display_str, sizeof(display_str), "%d", display);
    if (n < 0 || (size_t)n >= sizeof(display_str)) {
        kywc_log(KYWC_ERROR, "display number truncated");
        return false;
    }

    unsigned char cookie[COOKIE_SIZE];
    generate_xauthority_cookie(cookie);

    char *path = string_expand_path("~/.Xauthority");
    if (!path) {
        kywc_log(KYWC_ERROR, "expand Xauthority path failed");
        return false;
    }

    FILE *fp = fopen(path, "w+b");
    if (!fp) {
        kywc_log(KYWC_ERROR, "open Xauthority failed");
        free(path);
        return false;
    }

    bool success = false;
    const char *proto_name = "MIT-MAGIC-COOKIE-1";
    if (write_xauthority_entry(fp, FAMILY_LOCAL, hostname, display_str, proto_name, cookie) != 0) {
        goto cleanup;
    }

    if (write_xauthority_entry(fp, FAMILY_WILD, "", display_str, proto_name, cookie) != 0) {
        goto cleanup;
    }

    if (fflush(fp) != 0) {
        goto cleanup;
    }

    if (ferror(fp)) {
        goto cleanup;
    }

    kywc_log(KYWC_INFO, "create Xauthority success %s", path);
    success = true;

cleanup:
    if (fp) {
        fclose(fp);
    }

    if (!success) {
        remove(path);
        return NULL;
    }

    return path;
}