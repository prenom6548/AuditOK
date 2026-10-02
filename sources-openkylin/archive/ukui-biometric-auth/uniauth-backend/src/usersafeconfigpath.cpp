/*
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "usersafeconfigpath.h"

#include <QString>
#include <QByteArray>
#include <QFileInfo>

#include <cstring>
#include <pwd.h>

namespace {

bool relativePathLooksSafe(const char *relativeUnderHome)
{
    if (!relativeUnderHome || relativeUnderHome[0] == '\0' || relativeUnderHome[0] == '/') {
        return false;
    }
    if (std::strstr(relativeUnderHome, "..") != nullptr) {
        return false;
    }
    return true;
}

/** 配置文件存在时须为普通文件、不得为软链接；若父目录已存在则不得为软链接且须为目录。 */
QString iniPathUnderPwHome(const QString &userName, const char *relativeUnderHome)
{
    if (!relativePathLooksSafe(relativeUnderHome)) {
        return QString();
    }

    const QByteArray userUtf = userName.toUtf8();
    struct passwd *pw = getpwnam(userUtf.constData());
    if (!pw || !pw->pw_dir || pw->pw_dir[0] != '/') {
        return QString();
    }

    const QString filePath
        = QString::fromUtf8(pw->pw_dir) + QLatin1Char('/') + QString::fromUtf8(relativeUnderHome);
    const int slash = filePath.lastIndexOf(QLatin1Char('/'));
    if (slash <= 0) {
        return QString();
    }
    const QString parentPath = filePath.left(slash);

    const QFileInfo parentFi(parentPath);
    if (parentFi.exists()) {
        if (parentFi.isSymLink() || !parentFi.isDir()) {
            return QString();
        }
    }

    const QFileInfo fileFi(filePath);
    if (fileFi.isSymLink()) {
        return QString();
    }
    if (!fileFi.exists()) {
        return filePath;
    }
    if (!fileFi.isFile()) {
        return QString();
    }
    return filePath;
}

} // namespace

QString safeUserBiometricConfPath(const QString &userName)
{
    return iniPathUnderPwHome(userName, ".biometric_auth/ukui_biometric.conf");
}

QString safeUserLegacy990BiometricConfPath(const QString &userName)
{
    return iniPathUnderPwHome(userName, ".biometric-auth/ukui-biometric.conf");
}
