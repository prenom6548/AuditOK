// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util/file.h"
#include "util/logger.h"
#include "util/macros.h"
#include "util/string.h"
#include "util/time.h"

#define MAX_FILE_SIZE (1024 * 1024) // 1M

static FILE *log_fp = NULL;
static enum kywc_log_level log_level = KYWC_WARN;
static bool log_in_realtime = true;
// log ratelimit
static uint32_t ratelimit_interval = 5000; // ms
static uint32_t ratelimit_burst = 10;
// TLS for thread
static __thread struct {
    uint32_t start; // ms
    uint32_t printed;
    uint32_t missed;
} ratelimit_state = { 0 };

static const char *level_colors[] = {
    [KYWC_SILENT] = "",         [KYWC_FATAL] = "\x1B[7;31m", [KYWC_ERROR] = "\x1B[1;31m",
    [KYWC_WARN] = "\x1B[1;35m", [KYWC_INFO] = "\x1B[1;34m",  [KYWC_DEBUG] = "\x1B[1;90m",
};

static const char *level_headers[] = {
    [KYWC_SILENT] = "",     [KYWC_FATAL] = "[FATAL]", [KYWC_ERROR] = "[ERROR]",
    [KYWC_WARN] = "[WARN]", [KYWC_INFO] = "[INFO]",   [KYWC_DEBUG] = "[DEBUG]",
};

static inline bool ratelimit_allow(void)
{
    /* log is disabled when burst is 0 */
    if (ratelimit_burst == 0) {
        return false;
    }
    /* rate limit is disabled when interval is 0 */
    if (ratelimit_interval == 0) {
        return true;
    }

    uint32_t now = current_time_msec();
    if (ratelimit_state.start == 0) {
        ratelimit_state.start = now;
    }

    if (now - ratelimit_state.start >= ratelimit_interval) {
        if (ratelimit_state.missed > 0) {
            fprintf(log_fp, "[Log Ratelimited] : %d messages suppressed in %d ms\n",
                    ratelimit_state.missed, ratelimit_interval);
            fflush(log_fp);
        }

        ratelimit_state.start = now;
        ratelimit_state.printed = 0;
        ratelimit_state.missed = 0;
    }

    if (ratelimit_state.printed < ratelimit_burst) {
        ratelimit_state.printed++;
        return true;
    } else {
        ratelimit_state.missed++;
        return false;
    }
}

static void log_file(enum kywc_log_level level, const char *fmt, va_list args, bool limited)
{
    if (!log_fp || level > log_level) {
        return;
    }

    if (limited && !ratelimit_allow()) {
        return;
    }

    char *buffer;
    size_t size;
    FILE *fp = open_memstream(&buffer, &size);
    if (!fp) {
        return;
    }

    bool log_color = log_fp == stdout;

    if (log_in_realtime) {
        time_t time_log = time(NULL);
        struct tm *tm_log = localtime(&time_log);
        fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s: ", tm_log->tm_year + 1900,
                tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min,
                tm_log->tm_sec, log_color ? level_colors[level] : level_headers[level]);
    } else {
        struct timespec ts = { 0 };
        clock_gettime(CLOCK_MONOTONIC, &ts);
        fprintf(fp, "[%02d:%02d:%02d.%06ld] %s: ", (int)(ts.tv_sec / 60 / 60),
                (int)(ts.tv_sec / 60 % 60), (int)(ts.tv_sec % 60), ts.tv_nsec / 1000,
                log_color ? level_colors[level] : level_headers[level]);
    }

    vfprintf(fp, fmt, args);
    if (log_color) {
        fprintf(fp, "\x1B[0m");
    }
    fprintf(fp, "\n");

    fclose(fp);
    fprintf(log_fp, "%s", buffer);
    /* fflush is needed when log to file */
    fflush(log_fp);
    free(buffer);
}

void kywc_log(enum kywc_log_level level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_file(level, format, args, false);
    va_end(args);
}

void kywc_vlog(enum kywc_log_level level, const char *format, va_list args)
{
    log_file(level, format, args, false);
}

void kywc_log_ratelimited(enum kywc_log_level level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_file(level, format, args, true);
    va_end(args);
}

void kywc_vlog_ratelimited(enum kywc_log_level level, const char *format, va_list args)
{
    log_file(level, format, args, true);
}

static enum kywc_log_level detect_log_level(enum kywc_log_level level)
{
    /* set logger log_level from env */
    const char *level_str = getenv("KYWC_LOG_LEVEL");
    if (!level_str) {
        return level;
    }

    if (strcmp(level_str, "DEBUG") == 0) {
        return KYWC_DEBUG;
    } else if (strcmp(level_str, "INFO") == 0) {
        return KYWC_INFO;
    } else if (strcmp(level_str, "WARN") == 0) {
        return KYWC_WARN;
    } else if (strcmp(level_str, "ERROR") == 0) {
        return KYWC_ERROR;
    } else if (strcmp(level_str, "FATAL") == 0) {
        return KYWC_FATAL;
    } else if (strcmp(level_str, "SILENT") == 0) {
        return KYWC_SILENT;
    } else {
        fprintf(stderr, "logger: env error, support (DEBUG|INFO|WARN|ERROR|FATAL|SILENT)\n");
        return level;
    }
}

static void rotate_file_if_needed(int dir_fd, const char *file)
{
    struct stat st;
    if (fstatat(dir_fd, file, &st, AT_SYMLINK_NOFOLLOW) != 0) {
        return;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "logger: %s not a regular file\n", file);
        return;
    }

    if (st.st_size < MAX_FILE_SIZE) {
        return;
    }

    char timestamp[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

    char *new_name = string_create("%s.%s", file, timestamp);
    if (!new_name) {
        return;
    }

    int ret = renameat(dir_fd, file, dir_fd, new_name);
    free(new_name);
    if (ret != 0) {
        fprintf(stderr, "logger: rename file failed: %s\n", strerror(errno));
    }
}

void logger_init(enum kywc_log_level level, const char *file, bool realtime)
{
    log_in_realtime = realtime;
    log_level = detect_log_level(level);
    fprintf(stdout, "logger: current log level is %s\n", level_headers[log_level]);

    /* don't log to file */
    if (STRING_INVALID(file)) {
        log_fp = stdout;
        return;
    }

    char *log_dir = string_expand_path("~/.log");
    if (!log_dir) {
        fprintf(stderr, "get log dir failed\n");
        return;
    }

    if (!file_exists(log_dir)) {
        fprintf(stdout, "logger: %s not exist, create it\n", log_dir);
        int ret = mkdir(log_dir, S_IRWXU | S_IRWXG);
        if (ret) {
            fprintf(stderr, "create log dir failed: %s\n", strerror(errno));
            free(log_dir);
            return;
        }
    }

    int dir_fd = open(log_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd == -1) {
        fprintf(stderr, "logger: cannot open log directory '%s': %s\n", log_dir, strerror(errno));
        free(log_dir);
        return;
    }

    // check log file size
    rotate_file_if_needed(dir_fd, file);
    close(dir_fd);

    char *log_path = string_join_path(log_dir, NULL, file);
    free(log_dir);
    fprintf(stdout, "logger: path is %s\n", log_path);

    log_fp = fopen(log_path, "a"); // appending mode
    free(log_path);
    if (!log_fp) {
        fprintf(stderr, "logger: create log file failed: %s\n", strerror(errno));
        return;
    }
}

enum kywc_log_level kywc_log_get_level(void)
{
    return log_level;
}

void logger_set_level(enum kywc_log_level level)
{
    if (level == log_level) {
        return;
    }

    log_level = level;
    kywc_log(KYWC_SILENT, "Logger: current log level is %s", level_headers[level]);
}

void logger_set_rate_limit(unsigned interval, unsigned burst)
{
    ratelimit_interval = MIN(interval, 10000);
    ratelimit_burst = MIN(burst, 1000);
}

void logger_finish(void)
{
    if (!log_fp) {
        return;
    }

    fflush(log_fp);
    fclose(log_fp);
}
