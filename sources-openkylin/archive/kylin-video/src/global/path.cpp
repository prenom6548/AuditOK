/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "path.h"

#include <QLibraryInfo>
#include <QLocale>
#include <QRegExp>
#include <QFile>
#include <QDir>

#include <ukui-log4qt.h>

QString Paths::m_app_path = "";
QString Paths::m_log_path = "";
QString Paths::m_config_path = "";

void Paths::setAppPath(QString path) {
    m_app_path = path;
}

QString Paths::previewPath()
{
    QString fulldir = configPath() + "/preview";
    QDir dir(fulldir);
    if (!dir.exists()) {
        dir.mkdir(fulldir);
    }
    return fulldir;
}

QString Paths::appPath() {
    return m_app_path;
}

QString Paths::translationPath()
{
    QString t_path;
#ifdef NOTKAIMING
    if (QDir("/usr/share/kylin-video/translations").exists()) {
        t_path = "/usr/share/kylin-video/translations";
        return t_path;
    }
    else {
        return appPath() + "/translations";
    }
#else
    t_path = "/opt/apps/top.openkylin.kylin-video/files/share/kylin-video/translations";
    return t_path;
#endif

}

QString Paths::shortcutsPath() {
    return appPath() + "/shortcuts";
}

QString Paths::qtTranslationPath() {
    return QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translations
}

void Paths::setConfigPath(QString path) {
    m_config_path = path;
    QDir dir(m_config_path);
    if(!dir.exists())
    {
        dir.mkdir(m_config_path);
    }
}

void Paths::setLogPath(QString path)
{
    m_log_path = path;
    QDir dir(m_config_path);
    if(!dir.exists())
    {
        dir.mkdir(m_log_path);
    }
}

QString Paths::configPath() {
    if (!m_config_path.isEmpty()) {
        return m_config_path;
    } else {
        QDir dir(QDir::homePath() + "/.config/kylin-video");
        if(!dir.exists())
        {
            dir.mkdir(QDir::homePath() + "/.config/kylin-video");
        }
        return QDir::homePath() + "/.config/kylin-video";
    }
}

QString Paths::iniPath() {
    return configPath();
}

QString Paths::logPath()
{
    if (!m_log_path.isEmpty()) {
        return m_log_path;
    } else {
        QDir dir(QDir::homePath() + "/.cache/kylin-video");
        if(!dir.exists())
        {
            bool ret = dir.mkpath(QDir::homePath() + "/.cache/kylin-video");
            if(!ret)
            {
                KyInfo() << "log path create error!";
            }
        }
        return QDir::homePath() + "/.cache/kylin-video";
    }
}

QString Paths::subtitleStyleFile() {
    return configPath() + "/styles.ass";
}

