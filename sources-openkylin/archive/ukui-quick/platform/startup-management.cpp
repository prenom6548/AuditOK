/*
*  * Copyright (C) 2025, KylinSoft Co., Ltd.
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

/*!
    \class UkuiQuick::StartupManagement
    \brief 启动管理类，管理应用启动时的窗口几何位置设置。

    StartupManagement是一个单例类，提供应用启动时的窗口几何位置设置功能，
    允许在应用启动时指定窗口的初始位置和大小，窗管将根据此信息播放窗口动画。
    注意：此接口目前仅适用于wlcom合成器

    \section1 主要功能

    \list
    \li 设置启动窗口位置
    \li 管理应用启动参数
    \endlist

    \section1 使用示例

    在C++中使用：
    \code
    auto startup = UkuiQuick::StartupManagement::self()
    startup->setStartupGeometry(pid, 100, 100, 800, 600)
    \endcode

    \sa AppLauncher, Settings
*/

#include "startup-management.h"

#include <QGuiApplication>
#include <QDebug>

#include "registry.h"
#include "ukui-startup-manager.h"

namespace UkuiQuick {
static StartupManagement* instance = nullptr;
class StartupManagementPrivate
{
public:
    WaylandClient::UkuiStartupManagement *management = nullptr;
};
StartupManagement* StartupManagement::self()
{
    if (!instance) {
        instance = new StartupManagement();
    }
    return instance;
}

StartupManagement::~StartupManagement()
{
    if (d) {
        delete d;
        d = nullptr;
    }
    instance = nullptr;
}

void StartupManagement::setStartupGeometry(uint32_t pid, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    if (d->management && d->management->isValid()) {
        d->management->setStartupGeometry(pid, x, y, width, height);
    }
}

StartupManagement::StartupManagement(QObject* parent) : d(new StartupManagementPrivate)
{
    auto connection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!connection) {
        return;
    }

    auto registry = new UkuiQuick::WaylandClient::Registry(qGuiApp);
    registry->create(connection);
    registry->setup();
    connection->roundtrip();

    const WaylandClient::Registry::AnnouncedInterface interface = registry->interface(UkuiQuick::WaylandClient::Registry::Interface::UkuiStartupManagement);

    if (interface.name == 0) {
        qWarning() << "The compositor does not support the UkuiStartupManagement protocol";
        return;
    }

    d->management = registry->createUkuiStartupManagement(interface.name, interface.version, this);
}
}
