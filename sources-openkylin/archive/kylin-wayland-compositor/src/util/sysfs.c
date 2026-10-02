// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <inttypes.h>
#include <stdlib.h>
#include <kywc/log.h>

#include "util/sysfs.h"

bool sysfs_read_uint64(const char *filename, uint64_t *val)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        kywc_log(KYWC_ERROR, "Reading %s failed", filename);
        return false;
    }

    char buffer[64] = { 0 };

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        kywc_log(KYWC_ERROR, "Couldn't read from '%s'", filename);
        fclose(fp);
        return false;
    }
    fclose(fp);

    buffer[strcspn(buffer, "\n")] = '\0';

    char *endptr = NULL;
    errno = 0;
    unsigned long long result = strtoull(buffer, &endptr, 10);

    if (endptr == buffer) {
        kywc_log(KYWC_ERROR, "No digits found in '%s'", filename);
        return false;
    }

    if (errno == ERANGE) {
        kywc_log(KYWC_ERROR, "Value out of range in '%s'", filename);
        return false;
    }

    if (*endptr != '\0') {
        kywc_log(KYWC_ERROR, "Invalid characters in number '%s': '%s'", filename, endptr);
        return false;
    }

    if (result > UINT64_MAX) {
        kywc_log(KYWC_ERROR, "Value too large for uint64_t in '%s'", filename);
        return false;
    }

    *val = (uint64_t)result;
    return true;
}

bool sysfs_write_uint64(const char *filename, uint64_t val)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        kywc_log(KYWC_ERROR, "Writing %s failed", filename);
        return false;
    }

    if (fprintf(fp, "%" PRIu64, val) < 0) {
        kywc_log(KYWC_ERROR, "Couldn't write an unsigned integer to '%s'", filename);
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}

size_t sysfs_read_data(const char *filename, void *buf, size_t size)
{
    if (!filename || !buf || size == 0) {
        kywc_log(KYWC_ERROR, "Invalid parameters for reading file");
        return 0;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        kywc_log(KYWC_ERROR, "Reading %s failed", filename);
        return 0;
    }

    size_t len = fread(buf, 1, size, fp);

    if (ferror(fp)) {
        kywc_log(KYWC_ERROR, "Error occurred while reading '%s'", filename);
        len = 0;
    }

    fclose(fp);
    return len;
}
