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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "security-function-control.h"
#include "function-control.h"

namespace UkuiMenu {

UkuiMenu::SecurityFunctionControl *UkuiMenu::SecurityFunctionControl::instance()
{
    static SecurityFunctionControl instance(nullptr);
    return &instance;
}

bool UkuiMenu::SecurityFunctionControl::disable() const
{
    return m_disable;
}

bool UkuiMenu::SecurityFunctionControl::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

bool UkuiMenu::SecurityFunctionControl::disableMouseRightButton() const
{
    return m_disableMouseRightButton;
}

bool UkuiMenu::SecurityFunctionControl::disableMouseHover() const
{
    return m_disableMouseHover;
}

QStringList UkuiMenu::SecurityFunctionControl::appWhiteList() const
{
    return m_appWhiteList;
}

QStringList UkuiMenu::SecurityFunctionControl::appBlackList() const
{
    return m_appBlackList;
}

bool SecurityFunctionControl::disableUninstallAction() const
{
    return m_disableUninstallAction;
}

bool SecurityFunctionControl::windowMaximize() const
{
    return m_windowMaximize;
}

bool SecurityFunctionControl::showFoldButton() const
{
    return m_showFoldButton;
}

QString SecurityFunctionControl::powerButtonSoleOperation() const
{
    return m_powerButtonSoleOperation;
}

bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const
{
    if (!m_appWhiteList.isEmpty()) {
        if (!std::any_of(m_appWhiteList.constBegin(), m_appWhiteList.constEnd(), [&](const QString& app) {
            return desktop.endsWith(app);
        })) {
            return false;
        }
    }
    if (!m_appBlackList.isEmpty()) {
        if (std::any_of(m_appBlackList.constBegin(), m_appBlackList.constEnd(), [&](const QString& app) {
            return desktop.endsWith(app);
        })) {
            return false;
        }
    }
    return true;
}

bool SecurityFunctionControl::windowMaximizeControl() const
{
    return m_windowMaximizeControl;
}

UkuiMenu::SecurityFunctionControl::SecurityFunctionControl(QObject *parent) : QObject(parent)
{
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-menu-v11"));
    onFunctionControlChanged("ukui-menu-v11", false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [&](const QString &name) {
        onFunctionControlChanged(name);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [&]() {
        onFunctionControlChanged(QStringLiteral("ukui-menu-v11"), true);
    });
}

bool UkuiMenu::SecurityFunctionControl::compareListBySort(QStringList &list1, QStringList &list2)
{
    if (list1.size() != list2.size())
        return false;

    list1.sort();
    list2.sort();
    return list1 == list2;
}

void UkuiMenu::SecurityFunctionControl::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-menu-v11")) {
        return;
    }
    //管控中有值时，设置为管控值，否则设置为配置文件中的值
    auto config = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "ukui-menu function control config:" << config;
    //禁用开始菜单
    if (config.contains(QStringLiteral("disable"))) {
        bool disable = config.value(QStringLiteral("disable")).toBool();
        if (m_disable != disable) {
            m_disable = disable;
            Q_EMIT disableChanged();
        }
    } else {
        if (m_disable != false) {
            m_disable = false;
            Q_EMIT disableChanged();
        }
    }

    //禁用左键点击
    if (config.contains(QStringLiteral("disableMouseLeftButton"))) {
        bool disableMouseLeftButton = config.value(QStringLiteral("disableMouseLeftButton")).toBool();
        if (m_disableMouseLeftButton != disableMouseLeftButton) {
            m_disableMouseLeftButton = disableMouseLeftButton;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    } else {
        if (m_disableMouseLeftButton != false) {
            m_disableMouseLeftButton = false;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    }

    //禁用右键点击
    if (config.contains(QStringLiteral("disableMouseRightButton"))) {
        bool disableMouseRightButton = config.value(QStringLiteral("disableMouseRightButton")).toBool();
        if (m_disableMouseRightButton != disableMouseRightButton) {
            m_disableMouseRightButton = disableMouseRightButton;
            Q_EMIT disableMouseRightButtonChanged();
        }
    } else {
        if (m_disableMouseRightButton != false) {
            m_disableMouseRightButton = false;
            Q_EMIT disableMouseRightButtonChanged();
        }
    }

    //禁用鼠标悬浮
    if (config.contains(QStringLiteral("disableMouseHover"))) {
        bool disableMouseHover = config.value(QStringLiteral("disableMouseHover")).toBool();
        if (m_disableMouseHover != disableMouseHover) {
            m_disableMouseHover = disableMouseHover;
            Q_EMIT disableMouseHoverChanged();
        }
    } else {
        if (m_disableMouseHover != false) {
            m_disableMouseHover = false;
            Q_EMIT disableMouseHoverChanged();
        }
    }

    //应用白名单
    if (config.contains(QStringLiteral("appWhiteList"))) {
        QStringList appWhiteList;
        for (QVariant &app : config.value(QStringLiteral("appWhiteList")).toArray().toVariantList()) {
            appWhiteList.append(app.toString());
        }
        if (!compareListBySort(m_appWhiteList, appWhiteList)) {
            m_appWhiteList = appWhiteList;
            Q_EMIT appWhiteListChanged();
        }
    } else if (!m_appWhiteList.isEmpty()) {
        m_appWhiteList.clear();
        Q_EMIT appWhiteListChanged();
    }

    //应用黑名单
    if (config.contains(QStringLiteral("appBlackList"))) {
        QStringList appBlackList;
        for (QVariant &app : config.value(QStringLiteral("appBlackList")).toArray().toVariantList()) {
            appBlackList.append(app.toString());
        }
        if (!compareListBySort(m_appBlackList, appBlackList)) {
            m_appBlackList= appBlackList;
            Q_EMIT appBlackListChanged();
        }
    } else if (!m_appBlackList.isEmpty()) {
        m_appBlackList.clear();
        Q_EMIT appBlackListChanged();
    }

    //禁止卸载应用
    if (config.contains(QStringLiteral("disableUninstallAction"))) {
        bool disableUninstallAction = config.value(QStringLiteral("disableUninstallAction")).toBool();
        if (m_disableUninstallAction != disableUninstallAction) {
            m_disableUninstallAction = disableUninstallAction;
            Q_EMIT disableUninstallActionChanged();
        }
    } else {
        if (m_disableUninstallAction != false) {
            m_disableUninstallAction = false;
            Q_EMIT disableUninstallActionChanged();
        }
    }

    //开始菜单显示效果
    {
        bool windowMaximizeControl = false;
        bool showFoldButton = false;
        if (config.contains(QStringLiteral("windowMaximize"))) {
            windowMaximizeControl = true;
            showFoldButton = false;
            bool windowMaximize = config.value(QStringLiteral("windowMaximize")).toBool();
            if (m_windowMaximize != windowMaximize) {
                m_windowMaximize = windowMaximize;
                Q_EMIT windowMaximizeChanged();
            }
        } else {
            windowMaximizeControl = false;
            showFoldButton = true;
            if (m_windowMaximize != false) {
                m_windowMaximize = false;
            }
        }
        if (m_windowMaximizeControl != windowMaximizeControl) {
            m_windowMaximizeControl = windowMaximizeControl;
            Q_EMIT windowMaximizeControlChanged();
        }
        if (m_showFoldButton != showFoldButton) {
            m_showFoldButton = showFoldButton;
            Q_EMIT showFoldButtonChanged();
        }
    }

    //电源按钮交互行为
    if (config.contains(QStringLiteral("powerButtonSoleOperation"))) {
        QString powerButtonSoleOperation = config.value(QStringLiteral("powerButtonSoleOperation")).toString();
        if (m_powerButtonSoleOperation != powerButtonSoleOperation) {
            m_powerButtonSoleOperation = powerButtonSoleOperation;
            Q_EMIT powerButtonSoleOperationChanged();
        }
    } else {
        if (m_powerButtonSoleOperation != "") {
            m_powerButtonSoleOperation = "";
            Q_EMIT powerButtonSoleOperationChanged();
        }
    }
}
} // UkuiMenu
