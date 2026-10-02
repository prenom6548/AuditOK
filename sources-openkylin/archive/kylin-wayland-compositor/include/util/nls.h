// SPDX-FileCopyrightText: 2023 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef _UTIL_NLS_H_
#define _UTIL_NLS_H_

#include <locale.h>
#include <stdbool.h>
#include <string.h>

#if HAVE_NLS
#include <libintl.h>
#define tr gettext
struct locale {
    const char *lang;
    const char *data;
    size_t size;
};
bool nls_setup_message(const char *domain, const struct locale locales[], size_t size);
#else
#define tr(s) (s)
#endif

static __attribute__((unused)) inline bool nls_layout_is_right_to_left(void)
{
    const char *locale = setlocale(LC_MESSAGES, NULL);
    return locale &&
           (strstr(locale, "ug_CN") || strstr(locale, "kk_KZ") || strstr(locale, "ky_KG"));
}

static __attribute__((unused)) inline bool nls_text_is_right_align(void)
{
    const char *locale = setlocale(LC_MESSAGES, NULL);
    return locale && (strstr(locale, "kk_KZ") || strstr(locale, "ky_KG"));
}

#endif /* _UTIL_NLS_H_ */
