/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "search-button.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QDBusReply>
#include <QProcess>
#include <startup-management.h>
#include <function-control.h>
#include <QJsonObject>
#include <QMenu>
#include <QQuickWindow>

SearchButton::SearchButton(QObject *parent) : QObject(parent)
{
    initFunctionControl();
}

SearchButton::~SearchButton()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-search-v11"));
}

void SearchButton::openUkuiSearch(int x, int y, int width, int height)
{
    uint searchPid =  QDBusConnection::sessionBus().interface()->servicePid(QStringLiteral("com.ukui.search.service"));
    UkuiQuick::StartupManagement::self()->setStartupGeometry(searchPid, x, y, width, height);

    QDBusInterface interface(QStringLiteral("com.ukui.search.service"), QStringLiteral("/"), QStringLiteral("org.ukui.search.service"), QDBusConnection::sessionBus());
    interface.asyncCall(QStringLiteral("mainWindowSwitch"));
}

void SearchButton::openSearchSetting()
{
    bool res(false);
    QDBusInterface appLaunchInterface(QStringLiteral("com.kylin.ProcessManager"),
                                      QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                      QStringLiteral("com.kylin.ProcessManager.AppLauncher"),
                                      QDBusConnection::sessionBus());
    if(!appLaunchInterface.isValid()) {
        qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        res = false;
    } else {
        appLaunchInterface.setTimeout(10000);
        QDBusReply<void> reply = appLaunchInterface.call("LaunchAppWithArguments", "/usr/share/applications/ukui-control-center.desktop", QStringList{"-m", "search"});
        if(reply.isValid()) {
            res = true;
        } else {
            qWarning() << "ProcessManager dbus called failed!" << reply.error();
            res = false;
        }
    }
    if (!res) {
        //打开控制面板对应页面
        QProcess::startDetached("ukui-control-center", {"-m", "search"});
    }
}

void SearchButton::openContextMenu(QQuickItem *item)
{
    QMenu *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *searchSettings = new QAction(QIcon::fromTheme("system-settings-symbolic"), tr("Search settings"), menu);
    connect(searchSettings, &QAction::triggered, this, &SearchButton::openSearchSetting);
    menu->addAction(searchSettings);

    //设置父窗口
    menu->winId();
    if(menu->windowHandle()) {
        menu->windowHandle()->setTransientParent(item->window());
    }
    menu->popup(QCursor::pos());
}

bool SearchButton::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

bool SearchButton::disableMouseRightButton() const
{
    return m_disableMouseRightButton;
}

void SearchButton::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-search-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-search-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-search-v11"), true);
    });
}

void SearchButton::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-search-v11")) {
        return;
    }

    auto controlConfig = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "SearchButton function control config:" << controlConfig;
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
    //是否禁用右键点击
    if (controlConfig.contains(QStringLiteral("disableMouseRightButton"))) {
        bool disable = controlConfig.value(QStringLiteral("disableMouseRightButton")).toBool();
        if (disable != m_disableMouseRightButton) {
            m_disableMouseRightButton = disable;
            Q_EMIT disableMouseRightButtonChanged();
        }
    } else {
        m_disableMouseRightButton = false;
        Q_EMIT disableMouseRightButtonChanged();
    }
}
