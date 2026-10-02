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

SecurityFunctionControl *SecurityFunctionControl::instance()
{
    static SecurityFunctionControl instance(nullptr);
    return &instance;
}

bool SecurityFunctionControl::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

bool SecurityFunctionControl::disableMouseRightButton() const
{
    return m_disableMouseRightButton;
}

bool SecurityFunctionControl::disableMouseHover() const
{
    return m_disableMouseHover;
}

QStringList SecurityFunctionControl::appWhiteList() const
{
    return m_appWhiteList;
}

QStringList SecurityFunctionControl::appBlackList() const
{
    return m_appBlackList;
}

QStringList SecurityFunctionControl::leftClickBlackList() const
{
    return m_leftClickBlackList;
}

QStringList SecurityFunctionControl::rightClickBlackList() const
{
    return m_rightClickBlackList;
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

bool SecurityFunctionControl::canLeftClick(const QString &desktop) const
{
    if (!m_leftClickBlackList.isEmpty()) {
        if (std::any_of(m_leftClickBlackList.constBegin(), m_leftClickBlackList.constEnd(), [&](const QString& app) {
            return desktop.endsWith(app);
        })) {
            return false;
        }
    }
    return true;
}

bool SecurityFunctionControl::canRightClick(const QString &desktop) const
{
    if (!m_rightClickBlackList.isEmpty()) {
        if (std::any_of(m_rightClickBlackList.constBegin(), m_rightClickBlackList.constEnd(), [&](const QString& app) {
            return desktop.endsWith(app);
        })) {
            return false;
        }
    }
    return true;
}

bool SecurityFunctionControl::controlEnabled() const
{
    return m_controlEnabled;
}

SecurityFunctionControl::SecurityFunctionControl(QObject *parent) : QObject(parent)
{
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-sni-v11"));
    onFunctionControlChanged("ukui-sni-v11", false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [&](const QString &name) {
        onFunctionControlChanged(name);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [&]() {
        onFunctionControlChanged(QStringLiteral("ukui-sni-v11"), true);
    });
}

bool SecurityFunctionControl::compareListBySort(QStringList &list1, QStringList &list2)
{
    if (list1.size() != list2.size())
        return false;

    list1.sort();
    list2.sort();
    return list1 == list2;
}

void SecurityFunctionControl::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-sni-v11")) {
        return;
    }
    //管控中有值时，设置为管控值，否则设置为配置文件中的值
    auto config = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();
    qDebug() << "ukui-sni function control config:" << config;

    bool controlEnabled = !config.isEmpty();
    if (m_controlEnabled != controlEnabled) {
        m_controlEnabled = controlEnabled;
        Q_EMIT controlEnabledChanged(m_controlEnabled);
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

    //左键点击黑名单
    if (config.contains(QStringLiteral("leftClickBlackList"))) {
        QStringList leftClickBlackList;
        for (QVariant &app : config.value(QStringLiteral("leftClickBlackList")).toArray().toVariantList()) {
            leftClickBlackList.append(app.toString());
        }
        if (!compareListBySort(m_leftClickBlackList, leftClickBlackList)) {
            m_leftClickBlackList = leftClickBlackList;
            Q_EMIT leftClickBlackListChanged();
        }
    } else if (!m_leftClickBlackList.isEmpty()) {
        m_leftClickBlackList.clear();
        Q_EMIT leftClickBlackListChanged();
    }

    //右键点击黑名单
    if (config.contains(QStringLiteral("rightClickBlackList"))) {
        QStringList rightClickBlackList;
        for (QVariant &app : config.value(QStringLiteral("rightClickBlackList")).toArray().toVariantList()) {
            rightClickBlackList.append(app.toString());
        }
        if (!compareListBySort(m_rightClickBlackList, rightClickBlackList)) {
            m_rightClickBlackList = rightClickBlackList;
            Q_EMIT rightClickBlackListChanged();
        }
    } else if (!m_rightClickBlackList.isEmpty()) {
        m_rightClickBlackList.clear();
        Q_EMIT rightClickBlackListChanged();
    }
}
