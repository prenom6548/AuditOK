/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef NETCONNECT_H
#define NETCONNECT_H

#include "kswitchbutton.h"
#include "kborderlessbutton.h"
using namespace kdk;


#include <QObject>
#include <QtPlugin>
#include <QTranslator>
#include <QApplication>


#include <QTimer>
#include <QStringList>
#include <QString>
#include <QGSettings>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QPropertyAnimation>
#include <QPoint>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QFileSystemWatcher>

#include <kylin-nm/kylin-nm-interface.h>
#include <kylin-nm/kylinnetworkmanager.h>
#include "lanitem.h"
#include "../component/DeviceFrame/deviceframe.h"
#include "itemframe.h"
#include "../component/Divider/divider.h"
#include "netdetail.h"

enum {
    DISCONNECTED,
    NOINTERNET,
    CONNECTED
};

class NetConnect : public  Interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kylin.network")
    Q_INTERFACES(Interface)

public:
    NetConnect();
    ~NetConnect();

    QWidget * pluginUi() Q_DECL_OVERRIDE;   // 插件主界面---setPluginType后调用
    void setPluginType(PluginType type, bool useSwitch = true) Q_DECL_OVERRIDE;     // 设置插件类型
    void setParentWidget(QWidget* widget){
        if (parentWidget != nullptr) {
            return;
        }
        parentWidget = widget;
    }
    void setWidgetVisable(bool) {;}
    bool checkPluginIsHidden();
    void hideTitle();

private:
    bool m_isSimpleMode = true;
    bool m_useSwitch = true;

    void initSystemDbus();
    void initUi();
    void initComponent();
    void initConnect();
    void runExternalApp();

    void showDesktopNotify(const QString &message, QString soundName);

    //开关相关
    void setSwitchStatus();
    void hideLayout(QVBoxLayout * layout);
    void showLayout(QVBoxLayout * layout);

    int getInsertPos(QString connName, QVBoxLayout* layout);

    void activeConnect(QString uuid, QString deviceName);
    void deActiveConnect(QString uuid);

    //获取设备列表
    void getDeviceStatusMap(QMap<QString, bool> &map);
    void initNet();
    void initNetListFromDevice(QString deviceName);
    //处理列表增加
    void addLanItem(ItemFrame *frame, QString devName, KyWiredItem item);
    //增加设备
    void addDeviceFrame(QString devName, bool isEnable);
    //减少设备
    void removeDeviceFrame(QString devName);
    //增加一项
    void addOneLanFrame(ItemFrame *frame, QString devName, QStringList infoList);
    //减少一项
    void removeOneLanFrame(ItemFrame *frame, QString deviceName, QString uuid);

    //单个lan连接状态变化
    void itemActiveConnectionStatusChanged(LanItem *item, KyConnectState status);

    //显示网速
    void showRate();
    void getDetailConfInfo(QString devName, QString uuid, ConInfo &conInfo);


    bool getWiredMainSwitch();

    void loadLanguageFromConfig();

    void retranslateUi();

    QString oobeInstallerConfigPath();

// protected:
//     bool eventFilter(QObject *w,QEvent *e);

private:

    KyNetworkManager* manager;
    QThread*        thread;
    QWidget* parentWidget = nullptr;

    QWidget*        pluginWidget = nullptr;
    bool            mFirstLoad = true;
    bool            msettingLoad = false;

    QVBoxLayout*    m_mainLayout = nullptr;
    QFrame*         m_titleFrame = nullptr;
    QHBoxLayout*    m_titleLayout = nullptr;
    QLabel*         m_titleLabel = nullptr;
    KSwitchButton*  m_wiredSwitch = nullptr;
    Divider*        m_titleDivider = nullptr;

    QFrame*         m_scrollFrame = nullptr;
    QVBoxLayout*    m_scrollLayout = nullptr;
    QScrollArea*    m_scrollArea = nullptr;

    QWidget*        m_listWidget = nullptr;
    QVBoxLayout*    m_scrollAreaLayout = nullptr;

    Divider*        m_settingsDivider = nullptr;
    QFrame*         m_settingsFrame = nullptr;
    QHBoxLayout*    m_settingsLayout = nullptr;
    KBorderlessButton* m_settingsLabel = nullptr;

    QMap<QString, bool> deviceStatusMap;
    QMap<QString, ItemFrame *> deviceFrameMap;

    QTimer * m_setNetSpeedTimer = nullptr;
    NetDetail* m_netdtail = nullptr;

    PluginType m_pluginType;

    QDBusInterface *m_pSysBusIntfs;

    QFileSystemWatcher *m_fileWatcher; 
    QTranslator *m_translator;      

Q_SIGNALS:
    void updatePluginHidden(bool);

    //general
    void setWiredEnabled(bool enabled);
    //有线无线公用
    void deleteConnect(const QString &connectUuid);
    void activateConnection(const QString connectUuid, const QString deviceName);
    void deactivateConnection(const QString &activeConnectUuid);
    //wired 单设备启用禁用
    void setWiredDeviceEnable(const QString &interface, bool enable);

    void createWiredConnect(KyConnectSetting connectSettingsInfo);
    void updateIpv4AndIpv6SettingInfo(const QString &uuid, const KyConnectSetting &connectSettingsInfo);

    void languageChanged();


private Q_SLOTS:
    void updateLanInfo(QString deviceName, QString connectUuid, QString connectName, QString connectPath);

    void onLanAdd(QString devName, QString connectUuid, QString connectName, QString connectPath);
    void onLanRemove(QString dbusPath);

    void onActiveConnectionChanged(QString deviceName, QString uuid, KyConnectState status);

//    void onDeviceStatusChanged();
    void onDeviceNameChanged(QString, QString);
    void onDeviceCarrierChange(QString deviceName, bool pluged);

    void onDeviceManagedChange(QString, bool);
    void onWiredEnabledChanged(bool);

    void onDeviceAdd(QString deviceName);
    void onDeviceRemove(QString deviceName);

    void onSetNetSpeed();
    void onShowDetailPage(QString, QString);
    void onUpdateSettings(QString, QString);

    void onWiredMainSwitchSLot(bool);
    void onWiredMainSwitchChanged(bool);

    void onConfigFileChanged(const QString &path); 
};

#endif // NETCONNECT_H
