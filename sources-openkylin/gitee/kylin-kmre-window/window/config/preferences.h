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

#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QPoint>
#include <QMap>
#include <QSettings>
#include <QFileSystemWatcher>
#include <memory>
#include <shared_mutex>
#include "singleton.h"

namespace KmreConfig 
{

class Preferences : public QObject, public kmre::SingletonP<Preferences>
{
    Q_OBJECT
public:
    void save();
    void reload();
    void updateCameraConfig();
    void updateWindowHidestatus();
    void updatelockcode();
    void updateScrollSensitivity(int sensitivity);

    bool record_log;
    bool notification;
    bool m_recordWithAudio;
    bool m_recordFullscreen;
    bool m_toScreenHide;
    int mScrollSensitivity;
    const int mMinScrollSensitivity;
    const int mMaxScrollSensitivity;

    QString m_code;
    QString m_cameraDeviceName;

private slots:
    void slotConfigFileChanged(const QString &path);

private:
    std::mutex mMutex;
    std::shared_ptr<QSettings> mKmreSettings;
    QFileSystemWatcher *mFileWatcher;

    Preferences();
    ~Preferences();

    void load();
    void reset();
    friend SingletonP<Preferences>;
};

class Feature : public QObject, public kmre::SingletonP<Feature>
{
    Q_OBJECT
public:
    explicit Feature(QObject *parent = 0);

    bool isEnabled(const QString &pkgName, const QString &name);
    void initUiFeature();

private:
    QMap<QString, QStringList> mCurrentUiFeature;
    QSettings *settings = nullptr;
    std::mutex featureMapLock;

    friend SingletonP<Feature>;
};

}
#endif // _PREFERENCES_H_
