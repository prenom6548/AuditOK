/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_WORKSPACE_MANAGER_H
#define UKUI_WORKSPACE_MANAGER_H

#include <QObject>
#include <QList>
#include <KWayland/Client/event_queue.h>

struct kywc_workspace_manager_v1;
struct kywc_workspace_v1;
using namespace KWayland::Client;

namespace UkuiQuick::WaylandClient {

namespace Test {
class UkuiWorkspaceManagerTest;
}
class UkuiWorkspace;

class UkuiWorkspaceManager : public QObject
{
    Q_OBJECT
public:
    explicit UkuiWorkspaceManager(QObject *parent = nullptr);
    ~UkuiWorkspaceManager() override;

    void setup(kywc_workspace_manager_v1 *manager);
    void setEventQueue(EventQueue *queue);
    EventQueue *eventQueue() const;
    void release();
    void destroy();

    bool isValid() const;
    bool isInitialized() const;

    QList<UkuiWorkspace *> workspaces() const;
    UkuiWorkspace* workspace(const QString& uuid) const;
    uint32_t workspaceCount() const;
    bool requestCreateWorkspace(const QString &name, uint32_t position);
    bool requestRemoveWorkspace(const QString& uuid);
    bool stop();

Q_SIGNALS:
    void workspaceCreated(UkuiQuick::WaylandClient::UkuiWorkspace *workspace, const QString &uuid);
    void workspaceRemoved(const QString &uuid);
    void interfaceAboutToBeReleased();
    void interfaceAboutToBeDestroyed();
    void initialized();
    void invalidated();
    void removed();
    void done();
private:
    friend class Registry;
    friend class Test::UkuiWorkspaceManagerTest;

    enum class State {
        Unbound,
        Bound,
        Stopping,
        Invalidated
    };

    class Private;
    QScopedPointer<Private> d;
};

class UkuiWorkspace : public QObject
{
    Q_OBJECT
public:
    ~UkuiWorkspace() override;
    void setup(kywc_workspace_v1 *workspace);

    bool isValid() const;
    QString name() const;
    uint32_t position() const;
    bool isActivated() const;
    QString uuid() const;

    bool setName(const QString &name);
    bool setPosition(uint32_t position);
    bool activate();
    bool remove();

Q_SIGNALS:
    void nameChanged(const QString &name);
    void positionChanged(uint32_t position);
    void activated();
    void deactivated();
    void removed();

private:
    //向服务端发 destroy 请求
    void release();
    //释放本地资源
    void destroy();

    friend class UkuiWorkspaceManager;
    friend class Test::UkuiWorkspaceManagerTest;
    explicit UkuiWorkspace(QObject *parent = nullptr);
    enum class State {
        Invalidated,
        Ready
    };

    class Private;
    QScopedPointer<Private> d;
};

}

#endif //UKUI_WORKSPACE_MANAGER_H
