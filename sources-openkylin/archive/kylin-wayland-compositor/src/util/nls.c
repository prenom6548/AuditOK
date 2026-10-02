// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include "util/file.h"
#include "util/macros.h"
#include "util/nls.h"
#include "util/string.h"

#if HAVE_NLS

static const struct locale *find_locale(const struct locale locales[], size_t size)
{
    const char *locale = setlocale(LC_MESSAGES, NULL);

    if (STRING_INVALID(locale) || strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0) {
        return NULL;
    }

    size_t base_len = strcspn(locale, ".@");
    if (base_len > 0) {
        for (size_t i = 0; i < size; i++) {
            if (strlen(locales[i].lang) == base_len &&
                strncmp(locales[i].lang, locale, base_len) == 0) {
                return &locales[i];
            }
        }
    }

    size_t lang_len = strcspn(locale, "_.@");
    if (lang_len == 0) {
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        if (strlen(locales[i].lang) == lang_len &&
            strncmp(locales[i].lang, locale, lang_len) == 0) {
            return &locales[i];
        }
    }

    return NULL;
}

static bool write_locale_file(const struct locale *entry, const char *dir, const char *domain)
{
    char *path = string_create("%s/%s/LC_MESSAGES", dir, entry->lang);
    if (!path) {
        return false;
    }

    if (!file_mkdir_p(path)) {
        free(path);
        return false;
    }

    char *file = string_create("%s/%s.mo", path, domain);
    free(path);
    if (!file) {
        return false;
    }

    FILE *fp = fopen(file, "w+b");
    free(file);
    if (!fp) {
        return false;
    }

    size_t size = fwrite(entry->data, 1, entry->size, fp);
    fflush(fp);
    fclose(fp);

    return size == entry->size;
}

bool nls_setup_message(const char *domain, const struct locale locales[], size_t size)
{
    if (STRING_INVALID(domain)) {
        return false;
    }

    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (!runtime_dir || runtime_dir[0] != '/') {
        return false;
    }

    char *dir = string_create("%s/%s", runtime_dir, domain);
    if (!dir) {
        return false;
    }

    const struct locale *entry = find_locale(locales, size);
    if (!entry || !write_locale_file(entry, dir, domain)) {
        free(dir);
        return false;
    }

    bindtextdomain(domain, dir);
    bind_textdomain_codeset(domain, "UTF-8");
    textdomain(domain);

    free(dir);
    return true;
}

#endif
