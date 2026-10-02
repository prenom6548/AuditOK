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

#ifndef GENERALSETTINGWIDGET_H
#define GENERALSETTINGWIDGET_H

#include "settingscrollcontent.h"

#include <QPushButton>
#include <QComboBox>
#include <QMap>
#include <QFileSystemWatcher>
#include <vector>

class QSpinBox;
class QCheckBox;
class Preferences;
class IpItem;
class NetMaskItem;
class QDBusInterface;

enum  DockerNetworkModeIndex
{
    bridge = 0,
};

struct DockerNetworkMode
{
    DockerNetworkModeIndex mode;
    QString name;
    QString modeName;
};

struct DockerNetworkDevice
{
    QString DeviceName;
    QString ip;
    QString netmask;
    QString gateway;
};

class GeneralSettingWidget : public SettingScrollContent
{
    Q_OBJECT
public:
    explicit GeneralSettingWidget(QWidget *parent = 0);
    ~GeneralSettingWidget();

    void setKmreAutoStart(bool checked);
    void setDockerNetworkMode(int index);
    void logcollect();
    void getlogcollectstatus();
    void startCopyLogFils();
    void setDockerDns(int index);
    void saveNetSetting();
    QString getKmreVersion();
    Preferences *m_pref = nullptr;

private:
    void setDockerBridgeModeSettings();

signals:
    void opendeveloperWidget();
    void restartEnv();

public slots:
    void updateCurrentCameraDevice(const QString &deviceName);
    void updateCameraDevices();

private:
    QLabel *AutoStarttitle = nullptr;
    QLabel *Shortcuttitle = nullptr;
    QLabel *Abouttitle = nullptr;
    QCheckBox *AutoCheckbox;
    QPushButton *m_saveBtn = nullptr;
    QLabel *m_cameraLabel = nullptr;
    QComboBox *m_cameraComboBox = nullptr;
    QMap<QString,QString> m_cameraMap;
    QFileSystemWatcher *watcher = nullptr;
    QWidget *mBridgeModeWidget;
    const std::vector<DockerNetworkMode> DockerNetworkModeList;
    DockerNetworkModeIndex CurrDockerNetworkModeIndex;
    std::vector<DockerNetworkDevice> DockerNetworkDevices;
    QComboBox *DockerNetworkModeCb;
    IpItem *m_ipItem;
    NetMaskItem *m_netmaskItem;
    QLineEdit *m_dnsline = nullptr;
    QPushButton *mBridgeModeSaveBtn = nullptr;
    QPushButton *m_logBtn = nullptr;
    QString m_displayInfo;
    QString m_logPath;
    QPushButton *m_version = nullptr;
    QString dns1;
    QString dns2;
    bool defaultdns = false;
    int clickNum = 0;

    QDBusInterface *m_systemBusInterface = nullptr;
};

#endif // GENERALSETTINGWIDGET_H
