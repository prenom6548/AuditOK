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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QObject>
#include <KWayland/Client/event_queue.h>

struct wl_compositor;
struct wl_region;
struct wl_surface;

using namespace KWayland::Client;

namespace UkuiQuick::WaylandClient {
class Compositor : public QObject
{
    Q_OBJECT
public:
    explicit Compositor(QObject *parent = nullptr);
    ~Compositor() override;

    static Compositor *fromApplication(QObject *parent = nullptr);

    bool isValid() const;
    void setup(wl_compositor *compositor);
    void release();
    void destroy();
    void setEventQueue(EventQueue *queue);
    EventQueue *eventQueue();

    wl_surface *createSurface();
    wl_region *createRegion();

Q_SIGNALS:
    void removed();
private:
    class Private;
    QScopedPointer<Private> d;

};
}


#endif //COMPOSITOR_H
