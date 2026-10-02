/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>
 * Copyright 2016 by Sebastian Kügler <sebas@kde.org>
 * Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                    Work sponsored by the LiMux project of
 *                    the city of Munich.
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef XRANDRMANAGER_H
#define XRANDRMANAGER_H

#include <QObject>
#include <QTimer>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QGSettings/qgsettings.h>
#include <QScreen>


#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/log.h>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>
#include <usd_base_class.h>

#include "xrandroutput.h"
#include "xrandr-dbus.h"
#include "xrandr-adaptor.h"
#include "xrandr-config.h"
#include "usd_base_class.h"
#include "usd_global_define.h"

#define SAVE_CONFIG_TIME 800

const QString C_SETTINGS_XRANDR_DEFAULT_PRIMARY_NAME =  "default-primary";
const QString C_SETTINGS_XRANDR_DEFAULT_PRIMARY_NUMBER = "default-primary-number";

class XrandrManager: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KScreen")

public:
    XrandrManager();
    ~XrandrManager() override;

public:
    bool start();
    void stop();
    void active();
    int initAllOutputs();
    void applyConfig();
    void writeConfig();

    int getCurrentMode();
    int getCurrentRotation();
    bool getInitStatus() { return m_isInitFinish; };
    void doOutputChanged(KScreen::Output *senderOutput);
    void doOutputModesChanged();
    void lightLastScreen();
    void outputConnectedWithoutConfigFile(KScreen::Output *senderOutput ,char outputCount);
    void setOutputsModeToCloneWithPreferredMode();
    void setOutputsModeToFirst(bool isFirstMode);
    void setOutputsModeToExtend();
    void setOutputsModeToExtendWithPreferredMode();
    bool checkPrimaryOutputsIsSetable();
    bool readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode eMode);
    bool checkSettable(UsdBaseClass::eScreenMode eMode);
    void sendOutputsModeToDbus();
    void calibrateTouchDevice();
    void onlyCalibrate();

    double getSingleScreenScale(QString &errorMsg);
    bool setSingleScreenScale(double scale, QString &errorMsg);
    QSize getScreenSizeWithXlib(QString outputName, int &width, int &height);
    double getPreferredScale(const KScreen::OutputPtr output);
    QString getOuputModIdWithScale(const KScreen::OutputPtr output);

    UsdBaseClass::eScreenMode discernScreenMode();

public Q_SLOTS:
    void doOutputsConfigurationChanged();
    void doRotationChanged(const QString &rotation);
    void doOutputAdded(const KScreen::OutputPtr &output);
    void doOutputRemoved(int outputId);

    void setOutputsMode(QString modeName);
    void setOutputsParam(QString screensParam);
    void doSaveConfigTimeOut();
    /*台式机screen旋转后触摸*/
    void doCalibrate(const QString screenMap);
    void getInitialConfig();
    QString getOutputsInfo();
protected:
    QMultiMap<QString, QString> m_mateFileTag; //存放标签的属性值
    QMultiMap<QString, int>     m_IntDate;

private:
    void disableCrtc();
    int getConnectScreensCount();
    void showKDS();
    //更新显示器列表时断开所有显示器信号，避免显示器更新列表时响应信号触发保存了非预期配置。
    void disconnectAllOutputSlot();
    int connectAllOutputSlot();

    void configureSecondaryOutput(int offsetX);
    int configurePrimaryOutput();
    /**
     * @brief setMaxScaleToXsettings
     */
    void setMaxScaleToXsettings();
    int configureOuputInExtendMode( KScreen::OutputPtr &output, int offsetX);
    int realTimeConfigHadDiffWithConfig();
    bool hasConfigurationChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool validateOutputForComparison(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    KScreen::OutputPtr findOutputByName(const QString& name);
    bool hasSizeChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool hasRotationChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool hasScaleChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool hasPrimaryStatusChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool hasEnabledStatusChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool hasPositionChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool hasCurrentModeChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target);
    bool isFirstScreenMode();
    void verifyAndRetryConfig();
private:
    enum eScreenSignal {
        isNone = 0,
        isOutputChanged = 1<<0,
        isPosChanged = 1<<1,
        isSizeChanged = 1<<2,
        isCurrentModeIdChanged = 1<<3,
        isRotationChanged = 1<<4,
        isConnectedChanged = 1<<5,
        isEnabledChanged = 1<<6,
        isPrimaryChanged = 1<<7,
        isClonesChanged = 1<<8,
        isReplicationSourceChanged = 1<<9,
        isScaleChanged = 1<<10,
        isLogicalSizeChanged = 1<<11,
        isFollowPreferredModeChanged = 1<<12,
        isModesChanged = 1 << 13,
    };

    QTimer                *m_acitveTimer = nullptr;
    QTimer                *m_outputsInitTimer = nullptr;
    QTimer                *m_screenSignalTimer = nullptr;
    QTimer                *m_offUsbScreenTimer = nullptr;
    QTimer                *m_onUsbScreenTimer = nullptr;

    QMetaEnum             m_outputModeEnum;
    QGSettings            *m_xrandrSettings = nullptr;
    QGSettings            *m_pXsettingsSettings = nullptr;
    QStringList           m_modesChangeOutputs;

    QDBusInterface        *m_statusManagerDbus = nullptr;

    xrandrDbus                      *m_xrandrDbus = nullptr;
    XrandrAdaptor                   *m_xrandrAdaptor = nullptr;
    KScreen::ConfigPtr                m_configPtr = nullptr;
    std::unique_ptr<xrandrConfig> m_outputsConfig = nullptr;
    std::unique_ptr<xrandrConfig> m_outputsConfigForOther = nullptr;
    KScreen::GetConfigOperation *m_getConfigOperation = nullptr;
    bool    m_isSetting = false;
    int     m_outputsChangedSignal = 0;
    bool    m_applyConfigWhenSave = false;
    int     m_waylandMode = 0;
    //对于wayland环境，在outputadd后需要重新获取显示器list，那在重新获取显示list期间保存配置的timer触发了信号后，也需要忽略掉信号。
    bool    m_isInitFinish = false;
    int     m_configRetryTimes = 0;
};

#endif // XRANDRMANAGER_H
