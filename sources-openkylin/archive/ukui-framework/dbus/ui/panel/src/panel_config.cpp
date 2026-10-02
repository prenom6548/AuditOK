/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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

#include "panel_config.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <unistd.h>

ConfigUkPanel::ConfigUkPanel(QObject *parent)
{
    initEnum();
    connectToSource();
    // connectSignal();
}

template<typename T>
bool ConfigUkPanel::convertEnumValue(const QString &str,
                                     const QMetaEnum &meta,
                                     T &result) const
{
    bool ok = false;
    const int value = meta.keyToValue(str.toLatin1().constData(), &ok);
    if(ok) {
        result = static_cast<T>(value);
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, "Invalid args:" + str);
        } else {
            qDebug()<<"convertEnumValue error:"<< str;
        }
    }

    return ok;
}


template<typename T>
bool ConfigUkPanel::convertEnumValue(const int &value,
                                     const QMetaEnum &meta,
                                     T &result) const
{
    const QString key = meta.valueToKey(value);
    if (key.isEmpty()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, "Invalid args:" + value);
        } else {
            qDebug()<<"convertEnumValue error"<< value;
        }
        return false;
    }
    result = static_cast<T>(value);
    return true;
}

void printMethodsAndSignals(const QRemoteObjectDynamicReplica *replica) {
    const QMetaObject *meta = replica->metaObject();
    int methodCount = meta->methodCount();
    for (int i = 0; i < methodCount; ++i) {
        QMetaMethod method = meta->method(i);
        if (method.methodType() == QMetaMethod::Method) {
            qDebug() << "Method:" << method.name() << "Signature:" << method.methodSignature();
        } else if (method.methodType() == QMetaMethod::Signal) {
            qDebug() << "Signal:" << method.name() << "Signature:" << method.methodSignature();
        }
    }

    for (int i = 0; i < meta->propertyCount(); ++i) {
        qDebug()<<"property"<<meta->property(i).name()<<meta->property(i).typeName();
    }
}

ConfigUkPanel::~ConfigUkPanel()
{

}

int ConfigUkPanel::GetMergeIcons() const
{
    PEEK_SERVER_STATE_RET_INT();
    GeneralConfigDefine::MergeIcons mergeType;

    if (!convertEnumValue(m_pIrep->property(c_mergeIcons).toInt(), m_mergeEnum, mergeType)) {
        if (calledFromDBus())  {
            sendErrorReply(QDBusError::Failed, "convertEnumValue type error.");
        }
    }

    return mergeType;
}

bool ConfigUkPanel::SetMergeIcon(int type)
{
    PEEK_SERVER_STATE_RET_BOOL();
    GeneralConfigDefine::MergeIcons mergeType;
    if (!convertEnumValue(type, m_mergeEnum, mergeType)) {
        if (calledFromDBus())  {
            sendErrorReply(QDBusError::Failed, "convertEnumValue type error.");
        }
    }
    m_pIrep->setProperty(c_mergeIcons, mergeType);
    return true;
}

int ConfigUkPanel::GetPanelLocation() const
{
    PEEK_SERVER_STATE_RET_INT();

    return m_pIrep->property(c_panelLocation).toInt();
}

bool ConfigUkPanel::SetPanelLocation(int location)
{
    PEEK_SERVER_STATE_RET_INT();

    if (m_pIrep->property(c_panelLock).toBool()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::NotSupported, "Panel settings are currently locked");
        }
        return false;
    }

    GeneralConfigDefine::PanelLocation panelLocation;

    if (!convertEnumValue(location,
                          m_locationEnum,
                          panelLocation)) {
        if (calledFromDBus())  {
            sendErrorReply(QDBusError::Failed, "convertEnumValue type error.");
        }
        return false;
    }

    m_pIrep->setProperty(c_panelLocation, panelLocation);
    return true;
}

int ConfigUkPanel::GetPanelSizePolicy() const
{
    PEEK_SERVER_STATE_RET_INT();

    return m_pIrep->property(c_panelSizePolicy).toInt();
}

bool ConfigUkPanel::SetPanelSizePolicy(int size)
{
    PEEK_SERVER_STATE_RET_BOOL();

    if (m_pIrep->property(c_panelLock).toBool()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::NotSupported, "Panel settings are currently locked");
        }
        return false;
    }

    GeneralConfigDefine::PanelSizePolicy panelSize;
    if (!convertEnumValue(size, m_sizeEnum, panelSize)) {
        if (calledFromDBus())  {
            sendErrorReply(QDBusError::Failed, "convertEnumValue type error.");
        }
        return false;
    }

    m_pIrep->setProperty(c_panelSizePolicy, panelSize);

    return true;
}

bool ConfigUkPanel::GetPanelAutoHide() const
{
    PEEK_SERVER_STATE_RET_BOOL();

    return m_pIrep->property(c_panelAutoHide).toBool();
}

bool ConfigUkPanel::SetPanelAutoHide(bool isHide)
{
    PEEK_SERVER_STATE_RET_BOOL();

    m_pIrep->setProperty(c_panelAutoHide, isHide);
    return true;
}

bool ConfigUkPanel::GetPanelLockState() const
{
    PEEK_SERVER_STATE_RET_BOOL();

    return m_pIrep->property(c_panelLock).toBool();
}

bool ConfigUkPanel::SetPanelLockState(bool lockState)
{
    PEEK_SERVER_STATE_RET_BOOL();

    m_pIrep->setProperty(c_panelLock, lockState);
    return true;
}

bool ConfigUkPanel::GetShowPanelOnAllScreens() const
{
    PEEK_SERVER_STATE_RET_BOOL();

    return m_pIrep->property(c_showPanelOnAllScreens).toBool();
}

bool ConfigUkPanel::SetShowPanelOnAllScreens(bool isShow)
{
    PEEK_SERVER_STATE_RET_BOOL();

    m_pIrep->setProperty(c_showPanelOnAllScreens,isShow);
    return true;
}

bool ConfigUkPanel::SetShowSystemTrayOnAllPanels(bool isShow)
{
    PEEK_SERVER_STATE_RET_BOOL();

    m_pIrep->setProperty(c_showSystemTrayOnAllPanels, isShow);
    return true;
}

bool ConfigUkPanel::GetShowSystemTrayOnAllPanels() const
{
    PEEK_SERVER_STATE_RET_BOOL();

    return m_pIrep->property(c_showSystemTrayOnAllPanels).toBool();
}

int ConfigUkPanel::GetTaskBarIconsShowedOn() const
{
    PEEK_SERVER_STATE_RET_INT();

    return m_pIrep->property(c_taskBarIconsShowedOn).toInt();
}

bool ConfigUkPanel::SetTaskBarIconsShowedOn(int showedOn)
{
    PEEK_SERVER_STATE_RET_BOOL();
    if (!m_pIrep->property(c_showPanelOnAllScreens).toBool()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::NotSupported, "panel just show on primary screen");
        }
        return false;
    }

    GeneralConfigDefine::TaskBarIconsShowedOn taskShowedOn;
    if (!convertEnumValue(showedOn, m_taskBarShowOnEnum, taskShowedOn)) {
        return false;
    }

    m_pIrep->setProperty(c_taskBarIconsShowedOn, taskShowedOn);
    return true;
}

bool ConfigUkPanel::GetShowTaskView() const
{
    PEEK_SERVER_STATE_RET_BOOL();
    qDebug()<<m_pIrep->property(c_disabledWidgets).toStringList();
    return !m_pIrep->property(c_disabledWidgets).toStringList().contains(C_TASK_VIEW);
}

bool ConfigUkPanel::SetShowTaskView(bool isShow)
{
    PEEK_SERVER_STATE_RET_BOOL();
    QStringList tempList = m_pIrep->property(c_disabledWidgets).toStringList();
    if (!isShow) {
        tempList.removeAll(C_TASK_VIEW);
    } else if (!tempList.contains(C_TASK_VIEW)) {
        tempList.append(C_TASK_VIEW);
    }
    m_pIrep->setProperty(c_disabledWidgets, tempList);

    QMetaObject::invokeMethod(m_pIrep, "disableWidget", Qt::DirectConnection,
                              Q_ARG(QString, C_TASK_VIEW), Q_ARG(bool, !isShow));

    return true;
}

bool ConfigUkPanel::GetShowSearch() const
{
    PEEK_SERVER_STATE_RET_BOOL();
    qDebug()<<m_pIrep->property(c_disabledWidgets).toStringList();
    return !m_pIrep->property(c_disabledWidgets).toStringList().contains(C_SEARCH_V);
}

bool ConfigUkPanel::SetShowSearch(bool isShow)
{
    PEEK_SERVER_STATE_RET_BOOL();
    QStringList tempList = m_pIrep->property(c_disabledWidgets).toStringList();
    if (!isShow) {
        tempList.removeAll(C_SEARCH_V);
    } else if (!tempList.contains(C_SEARCH_V)) {
        tempList.append(C_SEARCH_V);
    }
    m_pIrep->setProperty(c_disabledWidgets, tempList);

    QMetaObject::invokeMethod(m_pIrep, "disableWidget", Qt::DirectConnection,
                              Q_ARG(QString, C_SEARCH_V), Q_ARG(bool, !isShow));
    return true;

}

QStringList ConfigUkPanel::GetQuickLaunchers() const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                            QStringLiteral("org.ukui.taskManager"), QStringLiteral("/taskManager"),
                            QStringLiteral("org.ukui.taskManager"), QStringLiteral("quickLaunchers"));
    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().call(msg);
    if (!reply.isValid()) {
        qCritical() << "GetQuickLaunchers failed with error:" << reply.error();
        return {};
    }
    return reply.value();
}

void ConfigUkPanel::SetQuickLaunchers(const QStringList &applications)
{
    for (auto &application : applications) {
        QDBusMessage msg = QDBusMessage::createMethodCall(
                                QStringLiteral("org.ukui.taskManager"), QStringLiteral("/taskManager"),
                                QStringLiteral("org.ukui.taskManager"), QStringLiteral("addQuickLauncher"));
        msg.setArguments({application});
        auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(msg)));
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [&] (QDBusPendingCallWatcher *self) {
            if (self->isError()) {
                qWarning() << "SetQuickLaunchers: failed to call addQuickLauncher" << self->error();
            }
            self->deleteLater();
        });
    }
}

void ConfigUkPanel::initConnection()
{
    m_showSearch = m_pIrep->property(c_disabledWidgets).toStringList().contains(C_SEARCH_V);
    m_showTaskView = m_pIrep->property(c_disabledWidgets).toStringList().contains(C_TASK_VIEW);
    connectSignal();
}

void ConfigUkPanel::connectToSource()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ?
                             QLatin1String("WAYLAND_DISPLAY") :
                             QLatin1String("DISPLAY");

    QString display(qgetenv(displayEnv.toUtf8().data()));
    QUrl address(QStringLiteral("local:ukui-panel-config-")  +
                 QString(qgetenv("USER")) + QStringLiteral("-") + QString::number(getuid()) +
                 QStringLiteral("-") + display);

    qDebug() << "ukui-framework connect to source:" << m_qroNode.connectToNode(address);
    m_pIrep = m_qroNode.acquireDynamic("RemoteConfig");
    // 用于测试打印类型
//    connect(m_pIrep, &QRemoteObjectDynamicReplica::stateChanged, this, [this](QRemoteObjectDynamicReplica::State state, QRemoteObjectDynamicReplica::State OldState){
//        qDebug()<<"rep change"<<state<<OldState;
//        printMethodsAndSignals(m_pIrep);
//    });

    connect(m_pIrep, &QRemoteObjectDynamicReplica::initialized, this, &ConfigUkPanel::initConnection);

}

void ConfigUkPanel::connectSignal() const
{
    connect(m_pIrep, SIGNAL(mergeIconsChanged(GeneralConfigDefine::MergeIcons)), this, SLOT(onMergeIconsChanged(GeneralConfigDefine::MergeIcons)));
    connect(m_pIrep, SIGNAL(panelAutoHideChanged(bool)), this, SLOT(onPanelAutoHideChanged(bool)));
    connect(m_pIrep, SIGNAL(panelLocationChanged(GeneralConfigDefine::PanelLocation)), this, SLOT(onPanelLocationChanged(GeneralConfigDefine::PanelLocation)));
    connect(m_pIrep, SIGNAL(panelLockChanged(bool)), this, SLOT(onPanelLockStateChanged(bool)));
    connect(m_pIrep, SIGNAL(panelSizePolicyChanged(GeneralConfigDefine::PanelSizePolicy)), this, SLOT(onPanelSizePolicyChanged(GeneralConfigDefine::PanelSizePolicy)));
    connect(m_pIrep, SIGNAL(showPanelOnAllScreensChanged(bool)), this, SLOT(onShowPanelOnAllScreensChanged(bool)));
    connect(m_pIrep, SIGNAL(showSystemTrayOnAllPanelsChanged(bool)), this, SLOT(onShowSystemTrayOnAllPanelsChanged(bool)));
    connect(m_pIrep, SIGNAL(taskBarIconsShowedOnChanged(GeneralConfigDefine::TaskBarIconsShowedOn)), this, SLOT(onTaskBarIconsShowedOnChanged(GeneralConfigDefine::TaskBarIconsShowedOn)));
    connect(m_pIrep, SIGNAL(disabledWidgetsChanged(QStringList)), this, SLOT(onDisableWidgetsChanged(QStringList)));

}

void ConfigUkPanel::initEnum()
{
    m_mergeEnum = QMetaEnum::fromType<GeneralConfigDefine::MergeIcons>();
    m_sizeEnum = QMetaEnum::fromType<GeneralConfigDefine::PanelSizePolicy>();
    m_locationEnum = QMetaEnum::fromType<GeneralConfigDefine::PanelLocation>();
    m_taskBarShowOnEnum = QMetaEnum::fromType<GeneralConfigDefine::TaskBarIconsShowedOn>();
}

void ConfigUkPanel::sourceStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState)
{
    qDebug() << "PanelUkccPlugin sourceStateChanged" << oldState << state;
    Q_EMIT connectStateChanged(isConnected());
}

bool ConfigUkPanel::isConnected() const
{
    return m_pIrep->state() ==
           QRemoteObjectReplica::State::Valid;
}

bool ConfigUkPanel::island() const
{
    PEEK_SERVER_STATE_RET_BOOL();

    return false;
}

void ConfigUkPanel::onMergeIconsChanged(GeneralConfigDefine::MergeIcons mergeIcons)
{
    qDebug()<<__FUNCTION__<<mergeIcons;
    Q_EMIT MergeIconsChanged(mergeIcons);
}

void ConfigUkPanel::onPanelSizePolicyChanged(GeneralConfigDefine::PanelSizePolicy sizePolicy)
{
    qDebug()<<__func__<<sizePolicy;
    Q_EMIT PanelSizePolicyChanged(sizePolicy);
}

void ConfigUkPanel::onPanelAutoHideChanged(bool isAutoHide)
{
    qDebug()<<__func__<<isAutoHide;
    Q_EMIT PanelAutoHideChanged(isAutoHide);
}

void ConfigUkPanel::onPanelLockStateChanged(bool lockState)
{
    qDebug()<<__func__<<lockState;
    Q_EMIT PanelLockStateChanged(lockState);
}

void ConfigUkPanel::onShowSystemTrayOnAllPanelsChanged(bool isAll)
{
    qDebug()<<__func__<<isAll;
    Q_EMIT ShowSystemTrayOnAllPanelsChanged(isAll);
}

void ConfigUkPanel::onPanelLocationChanged(GeneralConfigDefine::PanelLocation location)
{
    qDebug()<<__func__<<location;
    Q_EMIT PanelLocationChanged(location);
}

void ConfigUkPanel::onShowPanelOnAllScreensChanged(bool isAll)
{
    qDebug()<<__func__<<isAll;
    Q_EMIT ShowPanelOnAllScreensChanged(isAll);
}

void ConfigUkPanel::onTaskBarIconsShowedOnChanged(GeneralConfigDefine::TaskBarIconsShowedOn showPolicy)
{
    qDebug()<<__func__<<showPolicy;
    Q_EMIT TaskBarIconsShowOnChanged(showPolicy);
}

void ConfigUkPanel::onDisableWidgetsChanged(QStringList list)
{
    qDebug()<<__func__<<list;
    if (m_showSearch == list.contains(C_SEARCH_V)) {
        m_showSearch = list.contains(C_SEARCH_V);
        Q_EMIT ShowSearchChanged(m_showSearch);
    }
    if (m_showTaskView == list.contains(C_TASK_VIEW)) {
        m_showTaskView = list.contains(C_TASK_VIEW);
        Q_EMIT ShowTaskViewChanged(m_showTaskView);
    }
}

