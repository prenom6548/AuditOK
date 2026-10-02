/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include <QString>
#include <QStringList>

#define DEFAULT_SCROLL_SENSITIVITY 30

class QSettings;

class Preferences {
public:

    Preferences();
    ~Preferences();

    void reset();
    void save();
    void load();
    void initInfo();
    void updateCameraConfig();
    void updateKmreAutoStartConfig();
    void updatePhoneInfoConfig();
    void updatecustomConfig();
    void updatePhoneBrandIndex();
    void updatePhoneImei();
    void updateDNS();
    void updateDockerNetwork(const QString &mode, const QString &device);
    void updateScrollSensitivity(int sensitivity);

    QString m_cameraDeviceName;
    bool m_KmreAutoStart;
    QString displayType;
    QString cpuType;
    QString gpuVendor;
    QString gpuModel;
    bool m_PhoneInfoCustom;
    QString PhoneVendor;
    QString PhoneBrand;
    QString PhoneName;
    QString PhoneModel;
    QString PhoneIMEI;
    QString PhoneEquip;
    QString PhoneSerialno;
    QString PhoneBoard;
    int PhonepresetbrandIndex;
    int PhonepresetmodelIndex;
    bool m_defaultdns;
    QString m_dns;
    int mScrollSensitivity;
    const int mMinScrollSensitivity;
    const int mMaxScrollSensitivity;
    QString DockerNetworkMode;
    QString DockerNetworkDevice;

    QSettings *m_settings = nullptr;
};

#endif // _PREFERENCES_H_
