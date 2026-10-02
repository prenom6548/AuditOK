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

#ifndef UKUI_STARTUP_MANAGER_H
#define UKUI_STARTUP_MANAGER_H
#include <QObject>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/surface.h>
#include <QObject>
#include <QPointer>

struct ukui_startup_management_v1;
using namespace KWayland::Client;

namespace UkuiQuick::WaylandClient {
class UkuiStartupManagement : public QObject
{
    Q_OBJECT
public:
    explicit UkuiStartupManagement(QObject *parent = nullptr);
    ~UkuiStartupManagement() override;

    bool isValid() const;
    void release();
    void destroy();
    void setup(ukui_startup_management_v1 *shell) const;
    void setEventQueue(EventQueue *queue) const;

    EventQueue *eventQueue() const;

    void setStartupGeometry(uint32_t pid, int32_t x, int32_t y, uint32_t width, uint32_t height);

Q_SIGNALS:
    void interfaceAboutToBeReleased();
    void interfaceAboutToBeDestroyed();
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;

};
}


#endif //UKUI_STARTUP_MANAGER_H
