/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__

#include <libintl.h>
#include <locale.h>
#include <string>

static inline void kmI18nInit(const char* domain, const char* path)
{
    bindtextdomain(domain, path);
    bind_textdomain_codeset (domain, "UTF-8");
    textdomain(domain);
}

static inline void kmI18nInit(const std::string& domain, const std::string& path)
{
    kmI18nInit(domain.c_str(), path.c_str());
}

#define _(str) gettext((str))

#endif // !__TRANSLATION_H__
