/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "globalthemehelper.h"
#include "globaltheme.h"
#include "customglobaltheme.h"

#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QPixmap>
#include <QThread>

#include <QGSettings/QGSettings>

#include <QFileSystemWatcher>

#include <QDebug>
#include <QTimer>
#include "libkypackages.h"

#define SYSTEM_THEME_DIR QString("/usr/share/config/globaltheme/")
#define USER_THEME_DIR QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).arg("globaltheme/")

static GlobalThemeHelper *gInstance = nullptr;

GlobalThemeHelper *GlobalThemeHelper::getInstance()
{
    if (!gInstance) {
        gInstance = new GlobalThemeHelper;
        auto fsWatcher = new QFileSystemWatcher(GlobalThemeHelper::getInstance());
        fsWatcher->addPath(SYSTEM_THEME_DIR);
        fsWatcher->connect(fsWatcher, &QFileSystemWatcher::directoryChanged, GlobalThemeHelper::getInstance(), [=]{
            QTimer::singleShot(500, [=]{
                QDir systemDir(SYSTEM_THEME_DIR);
                auto systemThemes = systemDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
                bool changed = false;
                for (auto theme : systemThemes) {
                    if (!gInstance->d_ptr->globalThemes.keys().contains(theme)) {
                        auto globalTheme = new GlobalTheme(theme);
                        gInstance->d_ptr->globalThemes.insert(theme, globalTheme);
                        changed = true;
                    }
                }
                if (changed)
                    Q_EMIT GlobalThemeHelper::getInstance()->globalThemesChanged();
            });
        });
    }
    return gInstance;
}

QString GlobalThemeHelper::getCurrentGlobalThemeName()
{
    Q_D(GlobalThemeHelper);
    return d_func()->getCurrentThemeName();
}

GlobalTheme *GlobalThemeHelper::getCurrentGlobalTheme()
{
    Q_D(GlobalThemeHelper);
    return d_func()->globalThemes.value(getCurrentGlobalThemeName());
}

bool GlobalThemeHelper::getCurrentGlobalThemeModified()
{
    Q_D(GlobalThemeHelper);
    return d_func()->getCurrentThemeModified();
}

QStringList GlobalThemeHelper::getAllGlobalThemeNames()
{
    Q_D(GlobalThemeHelper);
    auto list = d_func()->globalThemes.keys();
    list.removeAll("custom");
    list.append("custom");
    return list;
}

QList<GlobalTheme *> GlobalThemeHelper::getAllGlobalThemes()
{
    Q_D(GlobalThemeHelper);
    auto list = d_func()->globalThemes.values();
    list.removeOne(getCustomTheme());
    list.append(getCustomTheme());
    return list;
}

QPixmap GlobalThemeHelper::getThemeThumbnail(const QString &theme)
{
    return QPixmap();
}

bool GlobalThemeHelper::setGlobalTheme(const QString &theme)
{
    Q_D(GlobalThemeHelper);
    return d_func()->setCurrentTheme(theme);
}

void GlobalThemeHelper::invalidateCurrentGlobalTheme()
{
    Q_D(GlobalThemeHelper);
    if (d_func()->settings) {
        d_func()->settings->set("isModified", true);
    }
}

GlobalTheme *GlobalThemeHelper::getCustomTheme()
{
    return d_func()->globalThemes.value("custom");
}

void GlobalThemeHelper::syncCustomThemeFromCurrentTheme()
{
    auto customTheme = qobject_cast<CustomGlobalTheme *>(d_func()->globalThemes.value("custom"));
    customTheme->loadFromOtherGlobalTheme(getCurrentGlobalTheme());
}

void GlobalThemeHelper::updateCustomThemeSetting(const QString &functionName, const QVariant &value)
{
    auto customTheme = qobject_cast<CustomGlobalTheme *>(d_func()->globalThemes.value("custom"));
    customTheme->updateValue(functionName, value);
}

void GlobalThemeHelper::loadThumbnail(GlobalTheme *theme)
{
    if (theme->getThemeName() == "custom") {
        theme->loadThumbnail();
    } else {
        auto thread = new QThread;
        auto thumbnailer = new Thumbnailer(theme);
        thumbnailer->moveToThread(thread);
        connect(thumbnailer, &Thumbnailer::finished, theme, &GlobalTheme::updateThumbnail);
        connect(thread, &QThread::started, thumbnailer, &Thumbnailer::doThumbnail);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->start();
    }
}

GlobalThemeHelper::GlobalThemeHelper(QObject *parent) : QObject(parent)
{
    d_ptr = new GlobalThemeHelperPrivate;

    d_ptr->initThemes();
}

/*!
 * \brief GlobalThemeHelperPrivate::initThemes
 * 初始化可用主题，从系统目录和用户目录中遍历获取，主题名称是文件夹的名称
 */
void GlobalThemeHelperPrivate::initThemes()
{
    if (QGSettings::isSchemaInstalled("org.ukui.globaltheme.settings")) {
        settings = new QGSettings("org.ukui.globaltheme.settings", "/org/ukui/globaltheme/settings/");
    }

    QDir systemDir(SYSTEM_THEME_DIR);
    auto systemThemes = systemDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    QDir userDir(USER_THEME_DIR);
    auto userThemes = userDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList themes;
    char** themePaths = kdk_package_get_standard_path("THEME_CONF_PATH");
    if (themePaths != nullptr) {
        for (int idx = 0; themePaths[idx] != nullptr; idx++) {
            qDebug() << "kdk_package_get_standard_path path index" << idx << "theme path" << themePaths[idx];
            QString globalThemePath = QString(themePaths[idx]);
            QDir globalThemeDir = QDir(globalThemePath);
            if (!globalThemeDir.exists()) {
                qDebug() << Q_FUNC_INFO << "global theme path not exist, skip" << globalThemeDir;
                continue;
            }
            auto globalThemes = globalThemeDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
            themes << globalThemes;
            qDebug() << Q_FUNC_INFO << "kdk_package_get_standard_path add global theme" << globalThemes << globalThemePath;
        }
    }
    if (themes.isEmpty()) {
        themes << systemThemes;
        themes << userThemes;
        qDebug() << Q_FUNC_INFO << "get themes is empty, add system theme" << themes.size();
    }
    themes.removeDuplicates();
    for (QString theme : themes) {
        auto globalTheme = new GlobalTheme(theme);
        globalThemes.insert(theme, globalTheme);
    }

    auto customTheme = new CustomGlobalTheme;
    globalThemes.insert("custom", customTheme);
}

QString GlobalThemeHelperPrivate::getCurrentThemeName()
{
    if (settings) {
        return settings->get("globalThemeName").toString();
    } else {
        return "Light-Seeking";
    }
}

bool GlobalThemeHelperPrivate::setCurrentTheme(const QString &theme)
{
    if (!getCurrentThemeModified() && theme == getCurrentThemeName()) {
        qInfo()<<"duplicated setting operation";
        return false;
    }

    QStringList themes = globalThemes.keys();
    if (themes.contains(theme)) {
        if (settings) {
            settings->set("globalThemeName", theme);
            settings->set("isModified", false);
            return true;
        }
    }

    qWarning()<<"invalid theme name:"<<theme<<"avaliable are:"<<themes;
    return false;
}

bool GlobalThemeHelperPrivate::getCurrentThemeModified()
{
    if (settings) {
        return settings->get("isModified").toBool();
    } else {
        return false;
    }
}

QString GlobalThemeHelperPrivate::getThemeWidgetStyleName(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return nullptr;
    } else {
        return globalTheme->getWidgetStyleName();
    }
}

bool GlobalThemeHelperPrivate::getThemeSupportLightDarkMode(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return false;
    } else {
        return globalTheme->getSupportLightDarkMode();
    }
}

QString GlobalThemeHelperPrivate::getThemeGtkStyleName(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return nullptr;
    } else {
        return globalTheme->getGtkThemeName();
    }
}

QString GlobalThemeHelperPrivate::getThemeIconThemeName(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return nullptr;
    } else {
        return globalTheme->getIconThemeName();
    }
}

QString GlobalThemeHelperPrivate::getThemeCursorThemeName(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return nullptr;
    } else {
        return globalTheme->getCursorThemeName();
    }
}

QString GlobalThemeHelperPrivate::getThemeWallPaper(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return nullptr;
    } else {
        return globalTheme->getWallPaperPath();
    }
}

bool GlobalThemeHelperPrivate::getThemeSupportBlur(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return false;
    } else {
        return globalTheme->getSupportBlur();
    }
}

bool GlobalThemeHelperPrivate::getThemeSupportTransparency(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return false;
    } else {
        return globalTheme->getSupportTransparency();
    }
}

bool GlobalThemeHelperPrivate::getThemeSupportAnimation(const QString &theme)
{
    auto globalTheme = globalThemes.value(theme);
    if (!globalTheme) {
        return false;
    } else {
        return globalTheme->getSupportAnimation();
    }
}

Thumbnailer::Thumbnailer(GlobalTheme *theme) : QObject(nullptr)
{
    if (theme->realPath().isEmpty())
        return;

    QDir dir(theme->realPath());
    auto files = dir.entryList(QDir::Files);
    for (QString file : files) {
        if (file.contains("preview")) {
            picturePath = dir.filePath(file);
            break;
        }
    }
}

void Thumbnailer::doThumbnail()
{
    if (picturePath.isEmpty()) {
        Q_EMIT finished(QPixmap());
    } else {
        QPixmap thumbnail = QPixmap(picturePath);
        Q_EMIT finished(thumbnail);
    }
}
