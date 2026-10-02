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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "show-desktop.h"

#include <window-manager.h>
#include <function-control.h>
#include <QJsonObject>

namespace UkuiPanel {

ShowDesktop::ShowDesktop(QObject *parent) : QObject(parent)
{
    initFunctionControl();
}

ShowDesktop::~ShowDesktop()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-showDesktop-v11"));
}

void ShowDesktop::showDesktop()
{
    UkuiQuick::WindowManager::setShowingDesktop(!UkuiQuick::WindowManager::showingDesktop());
}

bool ShowDesktop::disableMouseLeftButton() const
{
    return m_disableMouseLeftButton;
}

void ShowDesktop::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-showDesktop-v11")) {
        return;
    }

    auto controlConfig = exit ? QJsonObject(): UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "ShowDesktop function control config:" << controlConfig;
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

void ShowDesktop::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-showDesktop-v11"));
    onFunctionControlChanged(QStringLiteral("ukui-panel-showDesktop-v11"), false);
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-showDesktop-v11"), true);
    });
}

} // UkuiPanel
