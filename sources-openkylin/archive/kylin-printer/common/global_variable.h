/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef GLOBAL_VARIABLE
#define GLOBAL_VARIABLE

#include <QString>
#include <qglobal.h>
#include <libkysysinfo.h>

static QString getPlatForm(void)
{
    QString platform{QString()};

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    platform = "V10Professional";
#else
    platform = "V10";
#endif


    // char *projectName = kdk_system_get_projectName();
    // if (projectName == NULL) {
    //     return platform;
    // }

    // platform = QString(projectName);

    // free(projectName);

    return platform;
}

#ifdef PROJECT_NAME
__attribute__((unused)) static const QString APP_NAME(PROJECT_NAME);
#else
__attribute__((unused)) static const QString APP_NAME("empty");
#endif

__attribute__((unused)) static const QString APP_ICON_NAME(APP_NAME);

// system info
__attribute__((unused)) static const QString SYSTEM_VERSION(getPlatForm());



// binary path
#ifdef TEST_DEBUG
__attribute__((unused)) static const QString APP_PATH(QString("%1/%2").arg(PROJECT_BINARY_DIR).arg(APP_NAME));
#else
__attribute__((unused)) static const QString APP_PATH(QString("/usr/bin/%1").arg(APP_NAME));
#endif


#endif // GLOBAL_VARIABLE
