/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "message-processor.h"

#include <QCommandLineParser>
#include <QDebug>
#include <unistd.h>
#include "rep_remote-config_replica.h"

MessageProcessor::MessageProcessor(QObject* parent) : QObject(parent)
{
}

MessageProcessor::~MessageProcessor()
{
    if(m_configReplica) {
        m_configReplica->disconnect();
        delete m_configReplica;
        m_configReplica = nullptr;
    }
}

bool MessageProcessor::preprocessMessage(const QStringList &message)
{
    QCommandLineOption helpOption = m_parser.addHelpOption();
    QCommandLineOption versionOption = m_parser.addVersionOption();
    QCommandLineOption panelLocationOption("panelLocation", QObject::tr("Show current panel location"));
    QCommandLineOption panelSizePolicyOption("panelSizePolicy", QObject::tr("Show current panel size policy"));
    QCommandLineOption panelAutoHideOption("panelAutoHide", QObject::tr("Show current panel auto hide status"));
    QCommandLineOption panelLockOption("panelLock", QObject::tr("Show current panel lock status"));
    QCommandLineOption disabledWidgetsOption("disabledWidgets", QObject::tr("Show current panel disabledWidgets"));
    QCommandLineOption trayIconsInhibitedOption("trayIconsInhibited", QObject::tr("Show current inhibited tray icons"));
    QCommandLineOption taskBarIconsShowedOnOption("taskBarIconsShowedOn", QObject::tr("Show where taskBar icons showed on"));
    QCommandLineOption showPanelOnAllScreensOption("showPanelOnAllScreens", QObject::tr("Whether show panel on all screens"));
    QCommandLineOption showSystemTrayOnAllPanelsOption("showSystemTrayOnAllPanels", QObject::tr("Whether show system tray on all panels"));
    QCommandLineOption panelTypeOption("panelType", QObject::tr("Show Current panel type"));

    m_parser.addOption(panelLocationOption);
    m_parser.addOption(panelSizePolicyOption);
    m_parser.addOption(panelAutoHideOption);
    m_parser.addOption(panelLockOption);
    m_parser.addOption(disabledWidgetsOption);
    m_parser.addOption(trayIconsInhibitedOption);
    m_parser.addOption(taskBarIconsShowedOnOption);
    m_parser.addOption(showPanelOnAllScreensOption);
    m_parser.addOption(showSystemTrayOnAllPanelsOption);
    m_parser.addOption(panelTypeOption);

    m_parser.parse(message);

    if(message.size() <= 1 || !m_parser.unknownOptionNames().isEmpty() || m_parser.isSet(helpOption)) {
        m_parser.showHelp();
    }

    if (m_parser.isSet(versionOption)) {
        m_parser.showVersion();
    }
    m_qroNode.connectToNode(nodeUrl());
    m_configReplica = m_qroNode.acquire<RemoteConfigReplica>();
    connect(m_configReplica, &QRemoteObjectReplica::initialized, this, [=, this]() {
        auto panelType = m_configReplica->panelType();
        QString message;
        if (m_parser.isSet(panelLocationOption)) {
            QMetaEnum me = QMetaEnum::fromType<GeneralConfigDefine::PanelLocation>();
            message = me.valueToKey(panelType == GeneralConfigDefine::Version4 ? m_configReplica->panelLocation() :
                    m_configReplica->panelLocationNext());
        } else if (m_parser.isSet(panelSizePolicyOption)) {
            QMetaEnum me = QMetaEnum::fromType<GeneralConfigDefine::PanelSizePolicy>();
            message = me.valueToKey(panelType == GeneralConfigDefine::Version4 ? m_configReplica->panelSizePolicy() :
                    m_configReplica->panelSizePolicyNext());
        } else if (m_parser.isSet(panelAutoHideOption)) {
            message = (panelType == GeneralConfigDefine::Version4
                          ? m_configReplica->panelAutoHide()
                          : m_configReplica->panelAutoHideNext())? "true" : "false";
        } else if (m_parser.isSet(panelLockOption)) {
            message = (panelType == GeneralConfigDefine::Version4
                          ? m_configReplica->panelLock()
                          : m_configReplica->panelLockNext())? "true" : "false";
        } else if (m_parser.isSet(disabledWidgetsOption)) {
            message = panelType == GeneralConfigDefine::Version4 ? m_configReplica->disabledWidgets().join(",") :
                m_configReplica->disabledWidgetsNext().join(",");
        } else if (m_parser.isSet(trayIconsInhibitedOption)) {
            message = m_configReplica->trayIconsInhibited().join(",");
        } else if (m_parser.isSet(taskBarIconsShowedOnOption)) {
            QMetaEnum me = QMetaEnum::fromType<GeneralConfigDefine::TaskBarIconsShowedOn>();
            message = me.valueToKey(m_configReplica->taskBarIconsShowedOn());
        } else if (m_parser.isSet(panelTypeOption)) {
            QMetaEnum me = QMetaEnum::fromType<GeneralConfigDefine::PanelType>();
            message = me.valueToKey(panelType);
        } else if (m_parser.isSet(showPanelOnAllScreensOption)) {
            message =m_configReplica->showPanelOnAllScreens()? "true" : "false";
        } else if (m_parser.isSet(showSystemTrayOnAllPanelsOption)) {
            message =m_configReplica->showSystemTrayOnAllPanels()? "true" : "false";
        } else {
            m_parser.showHelp();
        }
        fputs(qPrintable(message + "\n"), stdout);
        qApp->quit();
    });

    return false;
}

QUrl MessageProcessor::nodeUrl()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland")
                             ? QLatin1String("WAYLAND_DISPLAY")
                             : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    return QUrl(QStringLiteral("local:ukui-panel-config-") + QString(qgetenv("USER")) + QStringLiteral("-") + QString::number(getuid()) + QStringLiteral("-") + display);
}
