/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "userconfigsettings.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QRegExp>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>

UserConfigSettings::UserConfigSettings(const QString &fileName, Format format, const QString &userName, QObject *parent)
    : QSettings(fileName, format, parent), m_fileName(fileName), m_uid(0), m_gid(0), m_hasModifications(false)
{
    // 通过用户名获取 UID/GID
    struct passwd *pwd = getpwnam(userName.toLocal8Bit().constData());
    if (pwd) {
        m_uid = pwd->pw_uid;
        m_gid = pwd->pw_gid;
        initPermissions();
    } else {
        qWarning() << "UserConfigSettings: Failed to get user info for:" << userName;
    }
}

UserConfigSettings::UserConfigSettings(const QString &fileName, Format format, uid_t uid, gid_t gid, QObject *parent)
    : QSettings(fileName, format, parent), m_fileName(fileName), m_uid(uid), m_gid(gid), m_hasModifications(false)
{
    initPermissions();
}

UserConfigSettings::~UserConfigSettings()
{
    // 如果有修改，在析构时修正权限
    if (m_hasModifications) {
        fixPermissions();
    }
}

void UserConfigSettings::initPermissions()
{
    // 识别公共目录边界（通过获取用户家目录，返回家目录的直接子目录）
    m_commonDirBoundary = detectCommonDirBoundary(m_fileName);
    qDebug() << "initPermissions:" << m_fileName << m_commonDirBoundary;
    // 确保配置目录存在
    ensureConfigDir();
}

QString UserConfigSettings::detectCommonDirBoundary(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString absolutePath = fileInfo.absolutePath();

    // 通过获取用户家目录，返回家目录的直接子目录
    QString homePath;
    QString userName;

    // 如果已有 UID，通过 UID 获取家目录
    if (m_uid != 0) {
        struct passwd *pwd = getpwuid(m_uid);
        if (pwd) {
            if (pwd->pw_dir) {
                homePath = QString(pwd->pw_dir);
            }
            if (pwd->pw_name) {
                userName = QString(pwd->pw_name);
                if (homePath.isEmpty() && !userName.isEmpty()) {
                    if ("lightdm" == userName) {
                        homePath = "/var/lib/lightdm";
                    } else if ("sddm" == userName) {
                        homePath = "/var/lib/sddm";
                    } else if ("gdm" == userName) {
                        homePath = "/var/lib/gdm";
                    } else {
                        homePath = QString("/home/%1").arg(userName);
                    }
                }
            }
        }
    }

    // 如果通过 UID 获取失败，返回空字符串；家目录不存在，不处理
    if (homePath.isEmpty() || !QDir(homePath).exists()) {
        return QString();
    }

    // 如果找到了家目录，返回家目录的直接子目录
    if (!homePath.isEmpty() && absolutePath.startsWith(homePath + "/")) {
        QString relativePath = absolutePath.mid(homePath.length() + 1); // 去掉家目录前缀和斜杠
        if (!relativePath.isEmpty()) {
            int firstSlash = relativePath.indexOf("/");
            if (firstSlash > 0) {
                // 返回家目录的直接子目录
                return homePath + "/" + relativePath.left(firstSlash);
            } else {
                // 如果相对路径没有斜杠，说明配置目录就是家目录的直接子目录
                return absolutePath;
            }
        }
    }

    return QString();
}

bool UserConfigSettings::ensureConfigDir()
{
    if (!m_commonDirBoundary.isEmpty()) {
        QDir boundaryDir(m_commonDirBoundary);
        QString tempDir = m_commonDirBoundary;
        if (tempDir.endsWith("/") && tempDir.length() > 1) {
            tempDir.chop(1);
        }
        int lastSlash = tempDir.lastIndexOf('/');
        QString parentDir;
        if (lastSlash > 0) {
            parentDir = tempDir.left(lastSlash);
        }
        bool isBoundaryDirExists = boundaryDir.exists();
        bool isParentDirExists = (!parentDir.isEmpty() && QDir(parentDir).exists());
        if (!isBoundaryDirExists && isParentDirExists) {
            // 边界目录不存在，且父目录存在则创建并设置权限
            if (!boundaryDir.mkpath(m_commonDirBoundary)) {
                qWarning() << "UserConfigSettings: Failed to create boundary directory:" << m_commonDirBoundary;
                return false;
            }
            // 设置边界目录权限
            if (chown(m_commonDirBoundary.toLocal8Bit().constData(), m_uid, m_gid) != 0) {
                qWarning() << "UserConfigSettings: Failed to chown boundary directory:" << m_commonDirBoundary
                           << "error:" << strerror(errno);
            }
        } else {
            // 递归修正边界目录下所有本配置文件父目录的权限
            fixDirPermissionsRecursive(m_commonDirBoundary);
        }
    }
    return true;
}

bool UserConfigSettings::fixPermissions()
{
    if (m_uid == 0 && m_gid == 0) {
        return false;
    }

    // 修正配置文件权限
    QFile configFile(m_fileName);
    if (configFile.exists()) {
        if (chown(m_fileName.toLocal8Bit().constData(), m_uid, m_gid) != 0) {
            qWarning() << "UserConfigSettings: Failed to chown config file:" << m_fileName
                       << "error:" << strerror(errno);
        }
    }

    return true;
}

bool UserConfigSettings::fixDirPermissionsRecursive(const QString &dirPath)
{
    if (m_uid == 0 && m_gid == 0) {
        qWarning() << "UserConfigSettings: Invalid UID/GID";
        return false;
    }

    if (!m_fileName.startsWith(dirPath)) {
        qWarning() << "UserConfigSettings: File path is not in the directory:" << dirPath;
        return false;
    }

    QFileInfo fileInfo(m_fileName);
    // 从配置目录开始，向上逐层修正权限，直到用户直接子目录
    QString currentPath = fileInfo.absolutePath();
    QString homePath = dirPath;

    // 逐层修正目录权限
    while (!currentPath.isEmpty()) {
        // 只修正到用户直接子目录
        if (currentPath == homePath) {
            break;
        }

        if (QDir(currentPath).exists()) {
            if (chown(currentPath.toLocal8Bit().constData(), m_uid, m_gid) != 0) {
                qWarning() << "UserConfigSettings: Failed to chown directory:" << currentPath
                           << "error:" << strerror(errno);
            }
        }

        // 移动到父目录
        QDir parentDir(currentPath);
        if (parentDir.cdUp()) {
            QString parentPath = parentDir.absolutePath();
            // 如果父目录不是边界目录的一部分，停止
            if (!parentPath.startsWith(homePath)) {
                break;
            }
            currentPath = parentPath;
        } else {
            break;
        }
    }

    return true;
}

bool UserConfigSettings::sync()
{
    // 调用父类的 sync（QSettings::sync() 返回 void）
    QSettings::sync();

    // 检查同步状态
    bool success = (status() == QSettings::NoError);

    if (success) {
        // 同步成功后修正权限
        m_hasModifications = true;
        fixPermissions();
    }

    return success;
}
