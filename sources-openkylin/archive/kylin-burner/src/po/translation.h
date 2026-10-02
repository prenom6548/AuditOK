/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef TRANSLATION_H
#define TRANSLATION_H
#define	TRANSLATION_DOMAIN "kylin-burner"
using namespace std;

#ifdef __cplusplus
extern "C"{
#include <locale.h>
#include <libintl.h>
}
#include <QString>

static inline void _KYBi18nLoad(QString _domain, QString _path)
{
    bindtextdomain(_domain.toStdString().c_str(),
                   _path.toStdString().c_str());
}
static inline void _KYBi18nLoad(string _domain, string _path)
{
    bindtextdomain(_domain.c_str(), _path.c_str());
}
static inline void _KYBi18nLoad(const char *_domain, const char *_path)
{
    bindtextdomain(_domain, _path);
}

static inline void _KYBi18nDomain(QString _domain)
{
    textdomain(_domain.toStdString().c_str());
}
static inline void _KYBi18nDomain(string _domain)
{
    textdomain(_domain.c_str());
}
static inline void _KYBi18nDomain(const char *_domain)
{
    textdomain(_domain);
}

static inline void _KYBi18nInit(QString _domain, QString _path)
{
    _KYBi18nLoad(_domain, _path);
    _KYBi18nDomain(_domain);
}

static inline void _KYBi18nInit(string _domain, string _path)
{
    _KYBi18nLoad(_domain, _path);
    _KYBi18nDomain(_domain);
}

static inline void _KYBi18nInit(const char *_domain, const char *_path)
{
    _KYBi18nLoad(_domain, _path);
    _KYBi18nDomain(_domain);
}
/*
 * 使用fromLocal8Bit 避免中文乱码问题。
 */
static inline QString _i18n(const char *msgid)
{
    return QString::fromLocal8Bit(dgettext(TRANSLATION_DOMAIN, msgid));
}
static inline QString _i18n(QString msgid)
{
    return QString::fromLocal8Bit(dgettext(TRANSLATION_DOMAIN, msgid.toStdString().c_str()));
}
static inline QString _i18n(string msgid)
{
    return QString::fromLocal8Bit(dgettext(TRANSLATION_DOMAIN, msgid.c_str()));
}

#else
#include <locale.h>
#include <libintl.h>
#define _KYBi18nLoad(domain, path) bindtextdomain((domain), (path));
#define _i18n(str) gettext((str))
#endif

#define _(str) _i18n((str))
#endif // TRANSLATION_H
