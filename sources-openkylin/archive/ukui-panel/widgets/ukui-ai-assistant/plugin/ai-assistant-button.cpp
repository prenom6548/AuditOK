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

#include "ai-assistant-button.h"


#include <QDebug>
#include <window-manager.h>
#include <app-launcher.h>
#include <application-info.h>
#include <function-control.h>
#include <QDBusConnection>
#include <QJsonObject>
#include <QDBusPendingCall>

AiAssistantButton::AiAssistantButton(QObject *parent) : QObject(parent)
{
    m_appInfo = new UkuiSearch::ApplicationInfo(this);
    m_appInfo->desktopFilePathFromName(QStringLiteral("kylin-aiassistant"), m_desktopFile);
    if (m_desktopFile.isEmpty()) {
        qWarning() << "AiAssistantButton: kylin-aiassistant desktop file not found";
    } else {
        fetchIcon();
    }
    connect(m_appInfo, &UkuiSearch::ApplicationInfo::appDBItems2BAdd, this, [this](const QStringList &desktopFiles) {
        for (const QString &desktopFile : desktopFiles) {
            if (desktopFile.endsWith(QStringLiteral("kylin-aiassistant.desktop"))) {
                m_appInfo->desktopFilePathFromName(QStringLiteral("kylin-aiassistant"), m_desktopFile);
                fetchIcon();
                Q_EMIT availableChanged();
            }
        }
    });
    connect(m_appInfo, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, [this](const QStringList &desktopFiles) {
        for (const QString &desktopFile: desktopFiles) {
            if (desktopFile.endsWith(QStringLiteral("kylin-aiassistant.desktop"))) {
                m_desktopFile.clear();
                m_appInfo->desktopFilePathFromName(QStringLiteral("kylin-aiassistant"), m_desktopFile);
                if (!m_desktopFile.isEmpty()) {
                    fetchIcon();
                }
                Q_EMIT availableChanged();
            }
        }
    });
    connect(m_appInfo, &UkuiSearch::ApplicationInfo::appDBItems2BUpdate, this, [this](UkuiSearch::ApplicationInfoMap infoMap) {
        for (const QString &desktopFile : infoMap.keys()) {
            if (desktopFile == m_desktopFile) {
                auto props = infoMap[desktopFile];
                if (props.contains(UkuiSearch::ApplicationProperty::Property::Icon)) {
                    auto newicon = props[UkuiSearch::ApplicationProperty::Property::Icon].toString();
                    if (newicon != m_icon) {
                        m_icon = newicon;
                        Q_EMIT iconChanged();
                    }
                }
                break;
            }
        }
    });

    initFunctionControl();
}

AiAssistantButton::~AiAssistantButton()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-ai-assistant-v11"));
}

void AiAssistantButton::pressed()
{
    qDebug() << "AiAssistantButton::pressed";
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.kylin_aiassistant"),
                                                  QStringLiteral("/org/ukui/kylin_aiassistant"),
                                                  QStringLiteral("org.ukui.kylin_aiassistant"),
                                                  QStringLiteral("showOrHideView"));
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(msg)));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [&] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "AiAssistantButton::pressed: failed to call showOrHideView" << self->error();
        }
        self->deleteLater();
    });
}

bool AiAssistantButton::available() const
{
    return !m_desktopFile.isEmpty();
}

QString AiAssistantButton::icon() const
{
    return m_icon;
}

bool AiAssistantButton::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

void AiAssistantButton::fetchIcon()
{
    auto icon = m_appInfo->getInfo(m_desktopFile, UkuiSearch::ApplicationProperty::Property::Icon).toString();
    if (icon != m_icon) {
        m_icon = icon;
        Q_EMIT iconChanged();
    }
}

void AiAssistantButton::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-ai-assistant-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-ai-assistant-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-ai-assistant-v11"), true);
    });
}

void AiAssistantButton::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-ai-assistant-v11")) {
        return;
    }

    auto controlConfig = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "AiAssistantButton function control config:" << controlConfig;
    //是否禁用左键点击
    if (controlConfig.contains(QStringLiteral("disableMouseLeftButton"))) {
        bool disable = controlConfig.value(QStringLiteral("disableMouseLeftButton")).toBool();
        if (disable != m_disableMouseLeftButton) {
            m_disableMouseLeftButton = disable;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    } else {
        m_disableMouseLeftButton = false;
        Q_EMIT disableMouseLeftButtonChanged();
    }
}
