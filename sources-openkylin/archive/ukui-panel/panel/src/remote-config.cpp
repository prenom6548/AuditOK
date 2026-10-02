/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "remote-config.h"
#include <config-loader.h>
#include <types.h>
#include <island.h>
#include <function-control.h>
#include <QJsonArray>

RemoteConfig::RemoteConfig(QObject *parent) : RemoteConfigSource(parent)
        , m_mergeIcons(GeneralConfigDefine::MergeIcons::Never)
        , m_panelLocation(GeneralConfigDefine::PanelLocation::Bottom)
        , m_panelSizePolicy(GeneralConfigDefine::PanelSizePolicy::Small)
        , m_allowCustomSize(false)
        , m_panelAutoHide(false)
        , m_panelLock(false)
        , m_taskBarIconsShowedOn(GeneralConfigDefine::TaskBarIconsShowedOn::AllPanels)
        , m_panelType(GeneralConfigDefine::PanelType::Version4)
        , m_showPanelOnAllScreens(true)
        , m_showSystemTrayOnAllPanels(true)
        , m_panelLocationNext(GeneralConfigDefine::PanelLocation::Bottom)
        , m_panelSizePolicyNext(GeneralConfigDefine::PanelSizePolicy::Small)
        , m_panelAutoHideNext(false)
        , m_panelLockNext(false)
{
    qRegisterMetaType<GeneralConfigDefine::MergeIcons>("GeneralConfigDefine::MergeIcons");
    qRegisterMetaType<GeneralConfigDefine::PanelLocation>("GeneralConfigDefine::PanelLocation");
    qRegisterMetaType<GeneralConfigDefine::PanelSizePolicy>("GeneralConfigDefine::PanelSizePolicy");
    qRegisterMetaType<GeneralConfigDefine::TaskBarIconsShowedOn>("GeneralConfigDefine::TaskBarIconsShowedOn");

    m_taskManagerConfig = UkuiQuick::ConfigLoader::getConfig("org.ukui.panel.taskManager");
    m_systemTrayConfig = UkuiQuick::ConfigLoader::getConfig("org.ukui.systemTray");
    m_mergeIcons = m_taskManagerConfig->getValue(QStringLiteral("mergeIcons")).value<GeneralConfigDefine::MergeIcons>();
    m_trayIconsInhibited = m_systemTrayConfig->getValue(QStringLiteral("trayIconsInhibited")).toStringList();
    m_trayIconsCanInhibited = m_systemTrayConfig->getValue(QStringLiteral("trayIconsCanInhibited")).toStringList();

    m_taskBarIconsShowedOn = m_taskManagerConfig->getValue(QStringLiteral("taskBarIconsShowedOn")).value<GeneralConfigDefine::TaskBarIconsShowedOn>();

    connect(m_taskManagerConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onTaskManagerConfigChanged);
    connect(m_systemTrayConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onSystemTrayConfigChanged);

    m_panelConfig = UkuiQuick::ConfigLoader::getConfig("panel", UkuiQuick::ConfigLoader::Local, "ukui-panel");
    m_panelConfig->addGroupInfo(UkuiQuick::Island::viewGroupName(), UkuiQuick::Island::viewGroupKey());
    for(auto config : m_panelConfig->children(UkuiQuick::Island::viewGroupName())) {
        if(config->getValue("id").toString() == "org.ukui.panel") {
            m_mainViewConfig = config;
        } else if(config->getValue("id").toString() == "org.ukui.panelNext") {
            m_mainViewNextConfig = config;
        } else if(config->getValue("id").toString() == "org.ukui.panelTopBar") {
            m_mainViewTopBarConfig = config;
        }
    }

    connect(m_panelConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onPanelConfigChanged);

    m_showPanelOnAllScreens = m_panelConfig->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
    m_panelType = m_panelConfig->getValue(QStringLiteral("currentPanelVersion")).value<GeneralConfigDefine::PanelType>();
    if (m_mainViewConfig) {
        m_disabledWidgets = m_mainViewConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
        m_panelSizePolicy = m_mainViewConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
        m_allowCustomSize = m_mainViewConfig->contains(QStringLiteral("customPanelSize"));
        m_panelAutoHide = m_mainViewConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
        m_panelLock = m_mainViewConfig->getValue(QStringLiteral("lockPanel")).toBool();
        m_panelWidgets = m_mainViewConfig->getValue(QStringLiteral("widgets")).toStringList();
        m_showSystemTrayOnAllPanels = m_mainViewConfig->getValue(QStringLiteral("showSystemTrayOnAllPanels")).toBool();
        connect(m_mainViewConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onPanelViewConfigChanged, Qt::UniqueConnection);
        updatePanelLocation();
    }
    if (m_mainViewNextConfig) {
        m_disabledWidgetsNext = m_mainViewNextConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
        m_panelSizePolicyNext = m_mainViewNextConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
        m_panelAutoHideNext = m_mainViewNextConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
        m_panelLockNext = m_mainViewNextConfig->getValue(QStringLiteral("lockPanel")).toBool();
        m_panelWidgets = m_mainViewNextConfig->getValue(QStringLiteral("widgets")).toStringList();
        connect(m_mainViewNextConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onPanelViewConfigNextChanged, Qt::UniqueConnection);
        updatePanelLocationNext();
    }

    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-v11"));
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-taskManager-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), false);
    onFunctionControlChanged(QStringLiteral("ukui-panel-taskManager-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), true);
        onFunctionControlChanged(QStringLiteral("ukui-panel-taskManager-v11"), true);
    });
}

RemoteConfig::~RemoteConfig()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-v11"));
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-taskManager-v11"));
}

GeneralConfigDefine::MergeIcons RemoteConfig::mergeIcons() const
{
    if (m_controlConfig.contains(QStringLiteral("mergeIcons"))) {
        return static_cast<GeneralConfigDefine::MergeIcons>(m_controlConfig.value(QStringLiteral("mergeIcons")).toInt());
    }
    return m_mergeIcons;
}

GeneralConfigDefine::PanelLocation RemoteConfig::panelLocation() const
{
    if (m_controlConfig.contains(QStringLiteral("panelLocation"))) {
        return static_cast<GeneralConfigDefine::PanelLocation>(m_controlConfig.value(QStringLiteral("panelLocation")).toInt());
    }
    return m_panelLocation;
}

GeneralConfigDefine::PanelSizePolicy RemoteConfig::panelSizePolicy() const
{
    if (m_controlConfig.contains(QStringLiteral("panelSizePolicy"))) {
        return static_cast<GeneralConfigDefine::PanelSizePolicy>(m_controlConfig.value(QStringLiteral("panelSizePolicy")).toInt());
    }
    return m_panelSizePolicy;
}

bool RemoteConfig::allowCustomSize() const
{
    return m_allowCustomSize;
}

bool RemoteConfig::panelAutoHide() const
{
    if (m_controlConfig.contains(QStringLiteral("panelAutoHide"))) {
        return m_controlConfig.value(QStringLiteral("panelAutoHide")).toBool();
    }
    return m_panelAutoHide;
}

bool RemoteConfig::panelLock() const
{
    if (m_controlConfig.contains(QStringLiteral("panelLock"))) {
        return m_controlConfig.value(QStringLiteral("panelLock")).toBool();
    }
    return m_panelLock;
}

void RemoteConfig::setMergeIcons(GeneralConfigDefine::MergeIcons mergeIcons)
{
    if (m_controlConfig.contains(QStringLiteral("mergeIcons"))) {
        qDebug() << "RemoteConfig::setMergeIcons() is not available cause this function is controled!";
        return;
    }
    if (mergeIcons != m_mergeIcons) {
        m_mergeIcons = mergeIcons;
        m_taskManagerConfig->setValue(QStringLiteral("mergeIcons"), mergeIcons);
        Q_EMIT mergeIconsChanged(m_mergeIcons);
    }
}

void RemoteConfig::setPanelLocation(GeneralConfigDefine::PanelLocation panelLocation)
{
    if (m_controlConfig.contains(QStringLiteral("panelLocation"))) {
        qDebug() << "RemoteConfig::setPanelLocation() is not available cause this function is controled!";
        return;
    }
    if (panelLocation != m_panelLocation) {
        m_panelLocation = panelLocation;

        UkuiQuick::Types::Pos pos;
        switch (m_panelLocation) {
            default:
            case GeneralConfigDefine::Bottom:
                pos = UkuiQuick::Types::Bottom;
                break;
            case GeneralConfigDefine::Left:
                pos = UkuiQuick::Types::Left;
                break;
            case GeneralConfigDefine::Top:
                pos = UkuiQuick::Types::Top;
                break;
            case GeneralConfigDefine::Right:
                pos = UkuiQuick::Types::Right;
                break;
        }
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("position"), pos);
        }
        Q_EMIT panelLocationChanged(m_panelLocation);
    }
}

void RemoteConfig::setPanelSizePolicy(GeneralConfigDefine::PanelSizePolicy panelSizePolicy)
{
    if (m_controlConfig.contains(QStringLiteral("panelSizePolicy"))) {
        qDebug() << "RemoteConfig::setPanelSizePolicy() is not available cause this function is controled!";
        return;
    }

    if (panelSizePolicy != m_panelSizePolicy) {
        m_panelSizePolicy = panelSizePolicy;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("panelSizePolicy"), panelSizePolicy);
        }
        Q_EMIT panelSizePolicyChanged(m_panelSizePolicy);
    }
}

void RemoteConfig::setPanelAutoHide(bool panelAutoHide)
{
    if (m_controlConfig.contains(QStringLiteral("panelAutoHide"))) {
        qDebug() << "RemoteConfig::setPanelAutoHide() is not available cause this function is controled!";
        return;
    }

    if (panelAutoHide != m_panelAutoHide) {
        m_panelAutoHide = panelAutoHide;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("panelAutoHide"), panelAutoHide);
        }
        Q_EMIT panelAutoHideChanged(m_panelAutoHide);
    }
}

void RemoteConfig::setPanelLock(bool panelLock)
{
    if (m_controlConfig.contains(QStringLiteral("panelLock"))) {
        qDebug() << "RemoteConfig::setPanelLock() is not available cause this function is controled!";
        return;
    }

    if (panelLock != m_panelLock) {
        m_panelLock = panelLock;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("lockPanel"), panelLock);
        }
        Q_EMIT panelLockChanged(m_panelLock);
    }
}

void RemoteConfig::setPanelWidgets(QStringList panelWidgets)
{
    if (panelWidgets != m_panelWidgets) {
        m_panelWidgets = panelWidgets;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("widgets"), m_panelWidgets);
        }
        Q_EMIT panelWidgetsChanged(m_panelWidgets);
    }
}

void RemoteConfig::setTrayIconsInhibited(QStringList trayIconsInhibited)
{
    if (trayIconsInhibited != m_trayIconsInhibited) {
        m_trayIconsInhibited = trayIconsInhibited;
        m_systemTrayConfig->setValue(QStringLiteral("trayIconsInhibited"), trayIconsInhibited);
        Q_EMIT trayIconsInhibitedChanged(m_trayIconsInhibited);
    }
}

void RemoteConfig::onPanelViewConfigChanged(const QString &key)
{
    if(!m_mainViewConfig) {
        return;
    }
    if (key == QStringLiteral("disabledWidgets")) {
        m_disabledWidgets = m_mainViewConfig->getValue(key).toStringList();
        Q_EMIT disabledWidgetsChanged(m_disabledWidgets);
    } else if(key == "panelSizePolicy") {
        auto value = m_mainViewConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
        if(value != m_panelSizePolicy) {
            m_panelSizePolicy = value;
            Q_EMIT panelSizePolicyChanged(m_panelSizePolicy);
        }
    } else if (key == "customPanelSize") {
        auto allow = m_mainViewConfig->contains(QStringLiteral("customPanelSize"));
        if (allow != m_allowCustomSize) {
            m_allowCustomSize = allow;
            Q_EMIT allowCustomSizeChanged(m_allowCustomSize);
        }
    } else if (key == "panelAutoHide") {
        bool value = m_mainViewConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
        if(value != m_panelAutoHide) {
            m_panelAutoHide = value;
            Q_EMIT panelAutoHideChanged(m_panelAutoHide);
        }
    } else if (key == "lockPanel") {
        bool value = m_mainViewConfig->getValue(QStringLiteral("lockPanel")).toBool();
        if(value != m_panelLock) {
            m_panelLock = value;
            Q_EMIT panelLockChanged(m_panelLock);
        }
    } else if (key == "widgets") {
        QStringList value = m_mainViewConfig->getValue(QStringLiteral("widgets")).toStringList();
        if(value != m_panelWidgets) {
            m_panelWidgets = value;
            Q_EMIT panelWidgetsChanged(m_panelWidgets);
        }
    } else if (key == "position") {
        updatePanelLocation();
        Q_EMIT panelLocationChanged(m_panelLocation);
    } else if (key == "showSystemTrayOnAllPanels") {
        bool value = m_mainViewConfig->getValue(QStringLiteral("showSystemTrayOnAllPanels")).toBool();
        if(value != m_showSystemTrayOnAllPanels) {
            m_showSystemTrayOnAllPanels = value;
            Q_EMIT showSystemTrayOnAllPanelsChanged(m_showSystemTrayOnAllPanels);
        }
    }
}

void RemoteConfig::onPanelConfigChanged(const QString &key)
{
    if (key == "currentPanelVersion") {
        auto value = m_panelConfig->getValue(QStringLiteral("currentPanelVersion")).value<GeneralConfigDefine::PanelType>();
        if(value != m_panelType) {
            m_panelType = value;
            Q_EMIT panelTypeChanged(m_panelType);
        }
    } else if (key == "showPanelOnAllScreens") {
        bool value = m_panelConfig->getValue(QStringLiteral("showPanelOnAllScreens")).toBool();
        if(value != m_showPanelOnAllScreens) {
            m_showPanelOnAllScreens = value;
            Q_EMIT showPanelOnAllScreensChanged(m_showPanelOnAllScreens);
        }
    }
}

void RemoteConfig::onTaskManagerConfigChanged(const QString &key)
{
    if(key == "mergeIcons") {
        auto value = m_taskManagerConfig->getValue(QStringLiteral("mergeIcons")).value<GeneralConfigDefine::MergeIcons>();
        if(value != m_mergeIcons) {
            m_mergeIcons = value;
            Q_EMIT mergeIconsChanged(m_mergeIcons);
        }
    } else if (key == "taskBarIconsShowedOn") {
        auto value = m_taskManagerConfig->getValue(QStringLiteral("taskBarIconsShowedOn")).value<GeneralConfigDefine::TaskBarIconsShowedOn>();
        if(value != m_taskBarIconsShowedOn) {
            m_taskBarIconsShowedOn = value;
            Q_EMIT taskBarIconsShowedOnChanged(m_taskBarIconsShowedOn);
        }
    }
}

void RemoteConfig::onSystemTrayConfigChanged(const QString &key)
{
    if(key == "trayIconsInhibited") {
        auto value = m_systemTrayConfig->getValue(QStringLiteral("trayIconsInhibited")).toStringList();
        if(value != m_trayIconsInhibited) {
            m_trayIconsInhibited = value;
            Q_EMIT trayIconsInhibitedChanged(m_trayIconsInhibited);
        }
    }
    if(key == "trayIconsCanInhibited") {
        auto value = m_systemTrayConfig->getValue(QStringLiteral("trayIconsCanInhibited")).toStringList();
        if(value != m_trayIconsCanInhibited) {
            m_trayIconsCanInhibited = value;
            Q_EMIT trayIconsCanInhibitedChanged(m_trayIconsCanInhibited);
        }
    }

}

void RemoteConfig::updatePanelLocation()
{
    if(!m_mainViewConfig) {
        return;
    }
    auto pos = m_mainViewConfig->getValue(QStringLiteral("position")).value<UkuiQuick::Types::Pos>();
    switch (pos) {
        case UkuiQuick::Types::Left:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Left;
            break;
        case UkuiQuick::Types::Top:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Top;
            break;
        case UkuiQuick::Types::Right:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Right;
            break;
        default:
        case UkuiQuick::Types::Bottom:
            m_panelLocation = GeneralConfigDefine::PanelLocation::Bottom;
            break;
    }
}

QStringList RemoteConfig::disabledWidgets() const
{
    if (m_controlConfig.contains(QStringLiteral("disableWidgets"))) {
        return disableWidgetsByControl();
    }
    if (m_controlConfig.contains(QStringLiteral("enableWidgets"))) {
        return (m_disabledWidgets.toSet().subtract(enableWidgetsByControl().toSet())).toList();
    }
    return m_disabledWidgets;
}

GeneralConfigDefine::TaskBarIconsShowedOn RemoteConfig::taskBarIconsShowedOn() const
{
    if (m_controlConfig.contains(QStringLiteral("taskbarIconsShowedOn"))) {
        return static_cast<GeneralConfigDefine::TaskBarIconsShowedOn>(m_controlConfig.value(QStringLiteral("taskbarIconsShowedOn")).toInt());
    }
    return m_taskBarIconsShowedOn;
}

bool RemoteConfig::showPanelOnAllScreens() const
{
    if (m_controlConfig.contains(QStringLiteral("showPanelOnAllScreens"))) {
        return m_controlConfig.value(QStringLiteral("showPanelOnAllScreens")).toBool();
    }
    return m_showPanelOnAllScreens;
}

bool RemoteConfig::showSystemTrayOnAllPanels() const
{
    if (m_controlConfig.contains(QStringLiteral("showSystemTrayOnAllPanels"))) {
        return m_controlConfig.value(QStringLiteral("showSystemTrayOnAllPanels")).toBool();
    }
    return m_showSystemTrayOnAllPanels;
}

void RemoteConfig::disableWidget(const QString &id, bool disable)
{
    if ((m_controlConfig.contains(QStringLiteral("disableWidgets")) ||
        m_controlConfig.contains(QStringLiteral("enableWidgets"))) &&
        (disableWidgetsByControl().contains(id) || enableWidgetsByControl().contains(id))) {
        qDebug() << "RemoteConfig::disableWidget() is not available cause this function is controled!" << id << disable;
        return;
    }
    if (!m_mainViewConfig) {
        return;
    }

    if (disable) {
        if (m_disabledWidgets.contains(id)) {
            return;
        }
        m_disabledWidgets.append(id);

    } else if (m_disabledWidgets.removeAll(id) == 0) {
        return;
    }
    m_mainViewConfig->setValue(QStringLiteral("disabledWidgets"), m_disabledWidgets);
}

void RemoteConfig::setTaskBarIconsShowedOn(GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn)
{
    if (m_controlConfig.contains(QStringLiteral("taskbarIconsShowedOn"))) {
        qDebug() << "RemoteConfig::setTaskBarIconsShowedOn() is not available cause this function is controled!";
        return;
    }

    if (taskBarIconsShowedOn != m_taskBarIconsShowedOn) {
        m_taskBarIconsShowedOn = taskBarIconsShowedOn;
        if(m_taskManagerConfig) {
            m_taskManagerConfig->setValue(QStringLiteral("taskBarIconsShowedOn"), taskBarIconsShowedOn);
        }
        Q_EMIT taskBarIconsShowedOnChanged(m_taskBarIconsShowedOn);
    }
}

void RemoteConfig::setPanelType(GeneralConfigDefine::PanelType panelType)
{
    if (panelType != m_panelType) {
        m_panelType = panelType;
        Q_EMIT panelTypeChanged(m_panelType);
        if(m_panelConfig) {
            m_panelConfig->setValue(QStringLiteral("currentPanelVersion"), m_panelType);
        }
    }
}

void RemoteConfig::setShowPanelOnAllScreens(bool showPanelOnAllScreens)
{
    if (m_controlConfig.contains(QStringLiteral("showPanelOnAllScreens"))) {
        qDebug() << "RemoteConfig::setShowPanelOnAllScreens() is not available cause this function is controled!";
        return;
    }

    if (showPanelOnAllScreens != m_showPanelOnAllScreens) {
        m_showPanelOnAllScreens = showPanelOnAllScreens;
        if(m_panelConfig) {
            m_panelConfig->setValue(QStringLiteral("showPanelOnAllScreens"), showPanelOnAllScreens);
        }
        Q_EMIT showPanelOnAllScreensChanged(m_showPanelOnAllScreens);
    }
}

void RemoteConfig::setShowSystemTrayOnAllPanels(bool showSystemTrayOnAllPanels)
{
    if (m_controlConfig.contains(QStringLiteral("showSystemTrayOnAllPanels"))) {
        qDebug() << "RemoteConfig::setShowSystemTrayOnAllPanels() is not available cause this function is controled!";
        return;
    }

    if (showSystemTrayOnAllPanels != m_showSystemTrayOnAllPanels) {
        m_showSystemTrayOnAllPanels = showSystemTrayOnAllPanels;
        if(m_mainViewConfig) {
            m_mainViewConfig->setValue(QStringLiteral("showSystemTrayOnAllPanels"), showSystemTrayOnAllPanels);
        }
        Q_EMIT showSystemTrayOnAllPanelsChanged(m_showSystemTrayOnAllPanels);
    }
}

void RemoteConfig::setPanelLocationNext(GeneralConfigDefine::PanelLocation panelLocation)
{
    if (panelLocation != m_panelLocationNext) {
        m_panelLocationNext = panelLocation;

        UkuiQuick::Types::Pos pos;
        switch (m_panelLocationNext) {
            default:
            case GeneralConfigDefine::Bottom:
                pos = UkuiQuick::Types::Bottom;
                break;
            case GeneralConfigDefine::Left:
                pos = UkuiQuick::Types::Left;
                break;
            case GeneralConfigDefine::Top:
                pos = UkuiQuick::Types::Top;
                break;
            case GeneralConfigDefine::Right:
                pos = UkuiQuick::Types::Right;
                break;
        }
        if(m_mainViewNextConfig) {
            m_mainViewNextConfig->setValue(QStringLiteral("position"), pos);
        }
        Q_EMIT panelLocationNextChanged(m_panelLocationNext);
    }
}

void RemoteConfig::setPanelSizePolicyNext(GeneralConfigDefine::PanelSizePolicy panelSizePolicy)
{
    if (panelSizePolicy != m_panelSizePolicyNext) {
        m_panelSizePolicyNext = panelSizePolicy;
        if(m_mainViewNextConfig) {
            m_mainViewNextConfig->setValue(QStringLiteral("panelSizePolicy"), panelSizePolicy);
        }
        Q_EMIT panelSizePolicyNextChanged(m_panelSizePolicyNext);
    }
}

void RemoteConfig::setPanelAutoHideNext(bool panelAutoHide)
{
    if (panelAutoHide != m_panelAutoHideNext) {
        m_panelAutoHideNext = panelAutoHide;
        if(m_mainViewNextConfig) {
            m_mainViewNextConfig->setValue(QStringLiteral("panelAutoHide"), panelAutoHide);
        }
        Q_EMIT panelAutoHideNextChanged(m_panelAutoHideNext);
    }
}

void RemoteConfig::setPanelLockNext(bool panelLock)
{
    if (panelLock != m_panelLockNext) {
        m_panelLockNext = panelLock;
        if(m_mainViewNextConfig) {
            m_mainViewNextConfig->setValue(QStringLiteral("lockPanel"), panelLock);
        }
        Q_EMIT panelLockNextChanged(m_panelLockNext);
    }
}

void RemoteConfig::onPanelViewConfigNextChanged(const QString &key)
{
    if(!m_mainViewNextConfig) {
        return;
    }
    if (key == QStringLiteral("disabledWidgets")) {
        m_disabledWidgetsNext = m_mainViewNextConfig->getValue(key).toStringList();
        Q_EMIT disabledWidgetsNextChanged(m_disabledWidgetsNext);
    } else if(key == "panelSizePolicy") {
        auto value = m_mainViewNextConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
        if(value != m_panelSizePolicyNext) {
            m_panelSizePolicyNext = value;
            Q_EMIT panelSizePolicyNextChanged(m_panelSizePolicyNext);
        }
    } else if (key == "panelAutoHide") {
        bool value = m_mainViewNextConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
        if(value != m_panelAutoHideNext) {
            m_panelAutoHideNext = value;
            Q_EMIT panelAutoHideNextChanged(m_panelAutoHideNext);
        }
    } else if (key == "lockPanel") {
        bool value = m_mainViewNextConfig->getValue(QStringLiteral("lockPanel")).toBool();
        if(value != m_panelLockNext) {
            m_panelLockNext = value;
            Q_EMIT panelLockNextChanged(m_panelLockNext);
        }
    } else if (key == "widgets") {
        QStringList value = m_mainViewNextConfig->getValue(QStringLiteral("widgets")).toStringList();
        if(value != m_panelWidgetsNext) {
            m_panelWidgetsNext = value;
            Q_EMIT panelWidgetsNextChanged(m_panelWidgetsNext);
        }
    } else if (key == "position") {
        updatePanelLocationNext();
        Q_EMIT panelLocationNextChanged(m_panelLocationNext);
    }
}

void RemoteConfig::updatePanelLocationNext()
{
    if(!m_mainViewNextConfig) {
        return;
    }
    auto pos = m_mainViewNextConfig->getValue(QStringLiteral("position")).value<UkuiQuick::Types::Pos>();
    switch (pos) {
        case UkuiQuick::Types::Left:
            m_panelLocationNext = GeneralConfigDefine::PanelLocation::Left;
            break;
        case UkuiQuick::Types::Top:
            m_panelLocationNext = GeneralConfigDefine::PanelLocation::Top;
            break;
        case UkuiQuick::Types::Right:
            m_panelLocationNext = GeneralConfigDefine::PanelLocation::Right;
            break;
        default:
        case UkuiQuick::Types::Bottom:
            m_panelLocationNext = GeneralConfigDefine::PanelLocation::Bottom;
            break;
    }
}

QStringList RemoteConfig::disableWidgetsByControl() const
{
    QStringList disableWidgets;
    for (QVariant &disableWidget : m_controlConfig.value(QStringLiteral("disableWidgets")).toArray().toVariantList()) {
        disableWidgets.append(disableWidget.toString());
    }
    return disableWidgets;
}

QStringList RemoteConfig::enableWidgetsByControl() const
{
    QStringList enableWidgets;
    for (QVariant &enableWidget : m_controlConfig.value(QStringLiteral("enableWidgets")).toArray().toVariantList()) {
        enableWidgets.append(enableWidget.toString());
    }
    return enableWidgets;
}

void RemoteConfig::disableWidgetNext(const QString &id, bool disable)
{
    if (!m_mainViewNextConfig) {
        return;
    }

    if (disable) {
        if (m_disabledWidgetsNext.contains(id)) {
            return;
        }
        m_disabledWidgetsNext.append(id);

    } else if (m_disabledWidgetsNext.removeAll(id) == 0) {
        return;
    }

    m_mainViewNextConfig->setValue(QStringLiteral("disabledWidgets"), m_disabledWidgetsNext);
    if(!m_mainViewTopBarConfig) {
        return;
    }
    m_mainViewTopBarConfig->setValue(QStringLiteral("disabledWidgets"), m_disabledWidgetsNext);
}

//获取管控配置的生效值
template <typename T>
T getConfigValue(const QJsonObject &cfg, const QString &key, T localVal)
{
    if (cfg.contains(key)) {
        return cfg.value(key).toVariant().value<T>();
    }
    return localVal;
}

void RemoteConfig::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-v11") && name != QStringLiteral("ukui-panel-taskManager-v11")) {
        return;
    }
    //保存旧配置，仅当配置的值发生变化时才需要发送信号
    QJsonObject oldControlConfig = m_controlConfig;
    m_controlConfig = exit ? QJsonObject() : UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "Remote config function control config:" << m_controlConfig;

    auto checkConfigChanged = [this, &oldControlConfig](const QString& key, auto defaultValue, auto signal) {
        auto oldValue = getConfigValue(oldControlConfig, key, defaultValue);
        auto newValue = getConfigValue(m_controlConfig, key, defaultValue);
        if (oldValue != newValue) {
            Q_EMIT (this->*signal)(newValue);
        }
    };

    if (name == QStringLiteral("ukui-panel-v11")) {
        //panelLocation 0-bottom; 1-left; 2-top; 3-right
        checkConfigChanged(QStringLiteral("panelLocation"), m_panelLocation, &RemoteConfig::panelLocationChanged);
        //panelSizePolicy 0-Small; 1-Medium; 2-Large; 3-Custom
        checkConfigChanged(QStringLiteral("panelSizePolicy"), m_panelSizePolicy, &RemoteConfig::panelSizePolicyChanged);
        //panelAutoHide
        checkConfigChanged(QStringLiteral("panelAutoHide"), m_panelAutoHide, &RemoteConfig::panelAutoHideChanged);
        //panelLock
        checkConfigChanged(QStringLiteral("panelLock"), m_panelLock, &RemoteConfig::panelLockChanged);
        //disableWidgets
        checkConfigChanged(QStringLiteral("disableWidgets"), m_disabledWidgets, &RemoteConfig::disabledWidgetsChanged);
        //showPanelOnAllScreens
        checkConfigChanged(QStringLiteral("showPanelOnAllScreens"), m_showPanelOnAllScreens, &RemoteConfig::showPanelOnAllScreensChanged);
        //showSystemTrayOnAllPanels
        checkConfigChanged(QStringLiteral("showSystemTrayOnAllPanels"), m_showSystemTrayOnAllPanels, &RemoteConfig::showSystemTrayOnAllPanelsChanged);
    }

    if (name == QStringLiteral("ukui-panel-taskManager-v11")) {
        //mergeIcons
        checkConfigChanged(QStringLiteral("mergeIcons"), m_mergeIcons, &RemoteConfig::mergeIconsChanged);
        //taskbarIconsShowedOn
        checkConfigChanged(QStringLiteral("taskbarIconsShowedOn"), m_taskBarIconsShowedOn, &RemoteConfig::taskBarIconsShowedOnChanged);
    }
}

void RemoteConfig::setPanelWidgetsNext(QStringList panelWidgets)
{
    if (panelWidgets != m_panelWidgetsNext) {
        m_panelWidgetsNext = panelWidgets;
        if(m_mainViewNextConfig) {
            m_mainViewNextConfig->setValue(QStringLiteral("widgets"), m_panelWidgetsNext);
        }
        Q_EMIT panelWidgetsNextChanged(m_panelWidgetsNext);
    }
}

void RemoteConfig::updateMainViewConfig()
{
    for(auto config : m_panelConfig->children(UkuiQuick::Island::viewGroupName())) {
        if(config->getValue("id").toString() == "org.ukui.panel") {
            if(!m_mainViewConfig) {
                m_mainViewConfig = config;
                m_disabledWidgets = m_mainViewConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
                Q_EMIT disabledWidgetsChanged(m_disabledWidgets);
                m_panelSizePolicy = m_mainViewConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
                Q_EMIT panelSizePolicyChanged(m_panelSizePolicy);
                m_panelAutoHide = m_mainViewConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
                Q_EMIT panelAutoHideChanged(m_panelAutoHide);
                m_panelLock = m_mainViewConfig->getValue(QStringLiteral("lockPanel")).toBool();
                Q_EMIT panelLockChanged(m_panelLock);
                m_panelWidgets = m_mainViewConfig->getValue(QStringLiteral("widgets")).toStringList();
                Q_EMIT panelWidgetsChanged(m_panelWidgets);
                m_showSystemTrayOnAllPanels = m_mainViewConfig->getValue(QStringLiteral("showSystemTrayOnAllPanels")).toBool();
                Q_EMIT showSystemTrayOnAllPanelsChanged(m_showSystemTrayOnAllPanels);
                connect(m_mainViewConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onPanelViewConfigChanged, Qt::UniqueConnection);
                updatePanelLocation();
                Q_EMIT panelLocationChanged(m_panelLocation);
            }
        } else if(config->getValue("id").toString() == "org.ukui.panelNext") {
            if(!m_mainViewNextConfig) {
                m_mainViewNextConfig = config;
                m_disabledWidgetsNext = m_mainViewNextConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
                Q_EMIT disabledWidgetsNextChanged(m_disabledWidgetsNext);
                m_panelSizePolicyNext = m_mainViewNextConfig->getValue(QStringLiteral("panelSizePolicy")).value<GeneralConfigDefine::PanelSizePolicy>();
                Q_EMIT panelSizePolicyNextChanged(m_panelSizePolicyNext);
                m_panelAutoHideNext = m_mainViewNextConfig->getValue(QStringLiteral("panelAutoHide")).toBool();
                Q_EMIT panelAutoHideNextChanged(m_panelAutoHideNext);
                m_panelLockNext = m_mainViewNextConfig->getValue(QStringLiteral("lockPanel")).toBool();
                Q_EMIT panelLockNextChanged(m_panelLockNext);
                m_panelWidgets = m_mainViewNextConfig->getValue(QStringLiteral("widgets")).toStringList();
                Q_EMIT panelWidgetsNextChanged(m_panelWidgetsNext);
                connect(m_mainViewNextConfig, &UkuiQuick::Config::configChanged, this, &RemoteConfig::onPanelViewConfigNextChanged, Qt::UniqueConnection);
                updatePanelLocationNext();
                Q_EMIT panelLocationNextChanged(m_panelLocationNext);
            }
        } else if(config->getValue("id").toString() == "org.ukui.panelTopBar") {
            if(!m_mainViewTopBarConfig) {
                m_mainViewTopBarConfig = config;
            }
        }
    }
}
