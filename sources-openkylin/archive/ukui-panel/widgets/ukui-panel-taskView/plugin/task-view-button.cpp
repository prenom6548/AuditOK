
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

#include "task-view-button.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <function-control.h>
#include <QJsonObject>

TaskViewButton::TaskViewButton(QObject *parent) : QObject(parent)
{
    initFunctionControl();
}

TaskViewButton::~TaskViewButton()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-taskView-v11"));
}

void TaskViewButton::showTaskView()
{
    QDBusInterface interface(QStringLiteral("org.ukui.KWin"), QStringLiteral("/MultitaskView"), QStringLiteral("org.kde.KWin.MultitaskView"), QDBusConnection::sessionBus());
    interface.asyncCall(QStringLiteral("show"));
}

bool TaskViewButton::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

void TaskViewButton::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-taskView-v11")) {
        return;
    }

    auto controlConfig = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "TaskViewButton function control config:" << controlConfig;
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

void TaskViewButton::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-taskView-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-taskView-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-taskView-v11"), true);
    });
}
