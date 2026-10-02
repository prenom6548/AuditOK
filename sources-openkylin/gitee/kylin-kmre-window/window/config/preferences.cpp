/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "preferences.h"
#include "kmreenv.h"
#include "sessionsettings.h"
#include "utils.h"

#include <QSettings>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <syslog.h>
#include <QStandardPaths>
#include <QDir>

#define DEFAULT_SCROLL_SENSITIVITY 30

using DisplayPlatform = SessionSettings::DisplayPlatform;

namespace KmreConfig 
{

Preferences::Preferences()
    : QObject(nullptr)
    , mKmreSettings(nullptr)
    , mScrollSensitivity(DEFAULT_SCROLL_SENSITIVITY)
    , mMinScrollSensitivity(0)
    , mMaxScrollSensitivity(100)
    , mFileWatcher(new QFileSystemWatcher(this))
{
    reload();

    if (mFileWatcher->addPath(KmreEnv::GetKmreConfigFile())) {
        connect(mFileWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(slotConfigFileChanged(const QString &)));
    }
    else {
        syslog(LOG_WARNING, "[%s] Add config file to file watcher failed!", __func__);
    }
}

Preferences::~Preferences()
{
    save();
}

void Preferences::reload()
{
    std::lock_guard<std::mutex> lk(mMutex);
    mKmreSettings.reset(new QSettings(KmreEnv::GetKmreConfigFile(), QSettings::IniFormat));
    mKmreSettings->setIniCodec("UTF-8");

    reset();
    load();
}

void Preferences::reset()
{
    record_log = false;
    notification = true;
    m_recordWithAudio = false;
    m_recordFullscreen = true;
    m_toScreenHide = false;
    m_cameraDeviceName = "";
    m_code = "";
    mScrollSensitivity = DEFAULT_SCROLL_SENSITIVITY;
}

void Preferences::save()
{
    std::lock_guard<std::mutex> lk(mMutex);

    mKmreSettings->beginGroup("debug");
    mKmreSettings->setValue("record_log", record_log);
    mKmreSettings->setValue("notification", notification);
    mKmreSettings->endGroup();

    mKmreSettings->beginGroup("camera");
    mKmreSettings->setValue("device", m_cameraDeviceName);
    mKmreSettings->endGroup();

    mKmreSettings->beginGroup("settings");
    mKmreSettings->setValue("screenshot_window_hide", m_toScreenHide);
    mKmreSettings->setValue("scroll_sensitivity", mScrollSensitivity);
    mKmreSettings->endGroup();
    mKmreSettings->sync();
}

void Preferences::load()
{
    mKmreSettings->beginGroup("debug");
    record_log = mKmreSettings->value("record_log", record_log).toBool();
    notification = mKmreSettings->value("notification", notification).toBool();
    mKmreSettings->endGroup();

    mKmreSettings->beginGroup("camera");
    m_cameraDeviceName = mKmreSettings->value("device", m_cameraDeviceName).toString();
    mKmreSettings->endGroup();

    mKmreSettings->beginGroup("settings");
    m_toScreenHide = mKmreSettings->value("screenshot_window_hide", m_toScreenHide).toBool();
    m_code = mKmreSettings->value("code", m_code).toString();
    mScrollSensitivity = mKmreSettings->value("scroll_sensitivity", mScrollSensitivity).toInt();
    mKmreSettings->endGroup();
}

void Preferences::updateCameraConfig()
{
    std::lock_guard<std::mutex> lk(mMutex);

    mKmreSettings->beginGroup("camera");
    mKmreSettings->setValue("device", m_cameraDeviceName);
    mKmreSettings->endGroup();
    mKmreSettings->sync();
}

void Preferences::updateWindowHidestatus()
{
    std::lock_guard<std::mutex> lk(mMutex);

    mKmreSettings->beginGroup("settings");
    mKmreSettings->setValue("screenshot_window_hide", m_toScreenHide);
    mKmreSettings->endGroup();
    mKmreSettings->sync();
}

void Preferences::updatelockcode()
{
    std::lock_guard<std::mutex> lk(mMutex);

    mKmreSettings->beginGroup("settings");
    mKmreSettings->setValue("code", m_code);
    mKmreSettings->endGroup();
    mKmreSettings->sync();
}

void Preferences::updateScrollSensitivity(int sensitivity)
{
    std::lock_guard<std::mutex> lk(mMutex);

    mScrollSensitivity = sensitivity;

    mKmreSettings->beginGroup("settings");
    mKmreSettings->setValue("scroll_sensitivity", sensitivity);
    mKmreSettings->endGroup();
    mKmreSettings->sync();
}

void Preferences::slotConfigFileChanged(const QString &path)
{
    syslog(LOG_INFO, "[%s] Config file(%s) changed!", __func__, path.toStdString().c_str());

    if (path == KmreEnv::GetKmreConfigFile()) {
        reload();
    }

    if (!mFileWatcher->files().contains(path)) {
        if (!mFileWatcher->addPath(path)) {
            syslog(LOG_WARNING, "[%s] re-add file to file watcher failed!", __func__);
        }
    }
}

Feature::Feature(QObject *parent)
    : QObject(parent)
{
    QString confName = KmreEnv::GetGlobalConfigPath().append("/feature_config.ini");
    settings = new QSettings(confName, QSettings::IniFormat, this);
    settings->setIniCodec("UTF-8");

    initUiFeature();
}

void Feature::initUiFeature()
{
    QString group_str = "pc_mode";

    if ((!group_str.isEmpty()) && settings) {
        std::lock_guard<std::mutex> lk(featureMapLock);
        mCurrentUiFeature.clear();
        settings->beginGroup(group_str);
        mCurrentUiFeature.insert("back", settings->value("back", "all").toStringList());
        mCurrentUiFeature.insert("to_top", settings->value("to_top", "none").toStringList());
        mCurrentUiFeature.insert("screen_shot", settings->value("screen_shot", "none").toStringList());
        mCurrentUiFeature.insert("full", settings->value("full", "all").toStringList());
        mCurrentUiFeature.insert("minimize", settings->value("minimize", "all").toStringList());
        mCurrentUiFeature.insert("close", settings->value("close", "all").toStringList());
        mCurrentUiFeature.insert("settings", settings->value("settings", "none").toStringList());
        mCurrentUiFeature.insert("game_key", settings->value("game_key", "none").toStringList());
        mCurrentUiFeature.insert("joy_stick", settings->value("joy_stick", "none").toStringList());
        mCurrentUiFeature.insert("shack_screen", settings->value("shack_screen", "none").toStringList());
        mCurrentUiFeature.insert("orientation_switch", settings->value("orientation_switch", "none").toStringList());
        if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
            mCurrentUiFeature.insert("screen_record_sharing", settings->value("screen_record_sharing", "none").toStringList());
        } else if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::WAYLAND) {
            if (SessionSettings::getInstance().hasWaylandRemoteAccessSupport()) {
                mCurrentUiFeature.insert("screen_record_sharing", settings->value("screen_record_sharing", "none").toStringList());
            }
        }
        mCurrentUiFeature.insert("virtual_keyboard", settings->value("virtual_keyboard", "none").toStringList());
        mCurrentUiFeature.insert("virtual_gps", settings->value("virtual_gps", "none").toStringList());
        mCurrentUiFeature.insert("gravity_sensor", settings->value("gravity_sensor", "none").toStringList());
        mCurrentUiFeature.insert("mobile_data_folder", settings->value("mobile_data_folder", "none").toStringList());
        mCurrentUiFeature.insert("mobile_photo_folder", settings->value("mobile_photo_folder", "none").toStringList());
        mCurrentUiFeature.insert("wechat_download_folder", settings->value("wechat_download_folder", "none").toStringList());
        mCurrentUiFeature.insert("qq_download_folder", settings->value("qq_download_folder", "none").toStringList());
        mCurrentUiFeature.insert("lock_screen", settings->value("lock_screen", "none").toStringList());
        mCurrentUiFeature.insert("scroll_sensitivity", settings->value("scroll_sensitivity", "none").toStringList());
        mCurrentUiFeature.insert("close_all_apps", settings->value("close_all_apps", "all").toStringList());
        mCurrentUiFeature.insert("help", settings->value("help", "all").toStringList());
        mCurrentUiFeature.insert("quit", settings->value("quit", "all").toStringList());
        settings->endGroup();
    }
}

bool Feature::isEnabled(const QString &pkgName, const QString &name)
{
    std::lock_guard<std::mutex> lk(featureMapLock);

    QStringList m_strlist = mCurrentUiFeature[name];
    if (m_strlist.contains("none", Qt::CaseInsensitive)) {
        return false;
    }
    else {
        if (m_strlist.contains("all", Qt::CaseInsensitive)) {
            return true;
        }
        else {
            if (m_strlist.contains(pkgName, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    return false;
}

}
