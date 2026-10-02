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

#ifndef UKUI_BLUR_MANAGER_H
#define UKUI_BLUR_MANAGER_H

#include <QObject>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/surface.h>

struct ukui_blur_manager_v1;
struct ukui_blur_surface_v1;
using namespace KWayland::Client;
namespace UkuiQuick::WaylandClient {
class UkuiBlur;
class UkuiBlurManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new UkuiBlurManager.
     * Note: after constructing the UkuiBlurManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use UkuiBlurManager prefer using
     * Registry::createUkuiBlurManager.
     **/
    explicit UkuiBlurManager(QObject* parent = nullptr);

    ~UkuiBlurManager() override;

    /**
     * @returns @c true if managing a ukui_blur_manager_v1.
     **/
    bool isValid() const;
    /**
     * Setup this UkuiBlurManager to manage the @p manager.
     * When using Registry::createUkuiBlurManager there is no need to call this
     * method.
     **/
    void setup(ukui_blur_manager_v1 *manager);
    /**
     * Releases the ukui_blur_manager_v1 interface.
     * After the interface has been released the UkuiBlurManager instance is no
     * longer valid and can be setup with another ukui_blur_manager_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this UkuiBlurManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new ukui_blur_manager_v1 interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, compositor, &BlurManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Blur.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Blur.
     **/
    EventQueue *eventQueue();

    UkuiBlur *getBlur(KWayland::Client::Surface *surface, QObject *parent = nullptr);

Q_SIGNALS:
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class UkuiBlur : public QObject
{
    Q_OBJECT
public:
    ~UkuiBlur() override;

    void setup(ukui_blur_surface_v1 *blur);
    void release();
    void destroy();
    bool isValid() const;

    void setRegion(QRegion region);
    void setLevel(uint32_t level);
    void setOpacity(const qreal opacity) const;
    void setPreferredType(uint32_t type) const;

private:
    friend class UkuiBlurManager;
    explicit UkuiBlur(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}

#endif //UKUI_BLUR_MANAGER_H
