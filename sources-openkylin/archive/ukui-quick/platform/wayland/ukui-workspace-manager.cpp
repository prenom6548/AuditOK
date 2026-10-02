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

#include "ukui-workspace-manager.h"

#include "wayland-pointer_p.h"
#include "wayland-kywc-workspace-v1-client-protocol.h"
#include <QDebug>
#include <optional>

namespace UkuiQuick::WaylandClient {
namespace
{
QString fromNullableUtf8(const char *text)
{
    return text ? QString::fromUtf8(text) : QString();
}
}

class UkuiWorkspaceManager::Private
{
public:
    Private(UkuiWorkspaceManager *q);

    bool canSendRequests() const;
    bool canReceiveEvents() const;

    void initListener();
    void commitPending();
    void removeWorkspace(UkuiWorkspace *workspace);
    void transitionTo(UkuiWorkspaceManager::State newState);
    void cleanupWorkspaces(bool remote);

    static struct kywc_workspace_manager_v1_listener s_managerListener;
    static void workspaceCallback(void *data, kywc_workspace_manager_v1 *manager, kywc_workspace_v1 *workspace, const char *uuid);
    static void doneCallback(void *data, kywc_workspace_manager_v1 *manager);
    static void finishedCallback(void *data, kywc_workspace_manager_v1 *manager);

    WaylandPointer<kywc_workspace_manager_v1, kywc_workspace_manager_v1_destroy> m_manager;
    EventQueue *m_queue = nullptr;
    UkuiWorkspaceManager *q;
    QList<UkuiWorkspace *> m_workspaces;
    QList<UkuiWorkspace *> m_pendingWorkspaces;
    UkuiWorkspaceManager::State m_state = UkuiWorkspaceManager::State::Unbound;
    bool m_initialized = false;
};

/**
 * workspaceCallback： 工作区新建回调，wayland 合成器创建新工作区时触发
 * doneCallback： 合成器完成一轮工作区的更新之后触发，在此回调之前可能会有多个工作区更新的事件
 * finishedCallback： 合成器即将销毁 kywc_workspace_manager_v1 接口，
    并且完成最后的事件发送，客户端应该在回调中释放所有相关的资源
 */
kywc_workspace_manager_v1_listener UkuiWorkspaceManager::Private::s_managerListener = {
    .workspace = workspaceCallback,
    .done = doneCallback,
    .finished = finishedCallback
};

class UkuiWorkspace::Private
{
public:
    Private(UkuiWorkspace *q);
    void initListener(kywc_workspace_v1 *workspace);
    void commitPending();
    void transitionTo(UkuiWorkspace::State newState);
    bool isInvalidated() const;
    void destroy();
    void release();

    static struct kywc_workspace_v1_listener s_workspaceListener;
    static void nameCallback(void *data, kywc_workspace_v1 *workspace, const char *name);
    static void positionCallback(void *data, kywc_workspace_v1 *workspace, uint32_t position);
    static void activatedCallback(void *data, kywc_workspace_v1 *workspace);
    static void deactivatedCallback(void *data, kywc_workspace_v1 *workspace);
    static void removedCallback(void *data, kywc_workspace_v1 *workspace);

    WaylandPointer<kywc_workspace_v1, kywc_workspace_v1_destroy> m_workspace;
    UkuiWorkspace *q;
    QString m_name;
    uint32_t m_position = 0;
    bool m_activated = false;
    QString m_uuid;
    State m_state = State::Invalidated;
    std::optional<QString> m_pendingName;
    std::optional<uint32_t> m_pendingPosition;
    std::optional<bool> m_pendingActivated;
};

kywc_workspace_v1_listener UkuiWorkspace::Private::s_workspaceListener = {
    .name = nameCallback,
    .position = positionCallback,
    .activated = activatedCallback,
    .deactivated = deactivatedCallback,
    .removed = removedCallback
};

UkuiWorkspaceManager::Private::Private(UkuiWorkspaceManager *q) : q(q)
{
}

UkuiWorkspaceManager::UkuiWorkspaceManager(QObject *parent) : QObject(parent), d(new Private(this))
{
}

UkuiWorkspaceManager::~UkuiWorkspaceManager() = default;

void UkuiWorkspaceManager::release()
{
    if (!d->m_manager) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->transitionTo(State::Invalidated);
    d->cleanupWorkspaces(true);
    d->m_manager.release();
}

void UkuiWorkspaceManager::destroy()
{
    if (!d->m_manager) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->transitionTo(State::Invalidated);
    d->cleanupWorkspaces(false);
    d->m_manager.destroy();
}

void UkuiWorkspaceManager::setup(kywc_workspace_manager_v1 *manager)
{
    Q_ASSERT(!d->m_manager);
    Q_ASSERT(manager);
    d->m_manager.setup(manager);
    d->transitionTo(State::Bound);
    d->initListener();
}

void UkuiWorkspaceManager::setEventQueue(EventQueue *queue)
{
    d->m_queue = queue;
}

EventQueue *UkuiWorkspaceManager::eventQueue() const
{
    return d->m_queue;
}

bool UkuiWorkspaceManager::isValid() const
{
    return d->m_manager && d->m_state != State::Unbound && d->m_state != State::Invalidated;
}

bool UkuiWorkspaceManager::isInitialized() const
{
    return d->m_initialized;
}

QList<UkuiWorkspace*> UkuiWorkspaceManager::workspaces() const
{
    return d->m_workspaces;
}

UkuiWorkspace* UkuiWorkspaceManager::workspace(const QString& uuid) const
{
    for (auto* workspace: d->m_workspaces) {
        if (workspace->uuid() == uuid) {
            return workspace;
        }
    }
    return nullptr;
}

uint32_t UkuiWorkspaceManager::workspaceCount() const
{
    return d->m_workspaces.size();
}

bool UkuiWorkspaceManager::requestCreateWorkspace(const QString &name, uint32_t position)
{
    if (!d->canSendRequests()) {
        qWarning() << "UkuiWorkspaceManager::requestCreateWorkspace: can not send requests";
        return false;
    }
    kywc_workspace_manager_v1_create_workspace(d->m_manager,
                                               name.isNull() ? nullptr : qPrintable(name),
                                               position);
    return true;
}

bool UkuiWorkspaceManager::requestRemoveWorkspace(const QString &uuid)
{
    if (!d->canSendRequests()) {
        qWarning() << "UkuiWorkspaceManager::requestRemoveWorkspace: can not send requests";
        return false;
    }
    for (auto* workspace: d->m_workspaces) {
        if (workspace->uuid() == uuid) {
            workspace->remove();
            return true;
        }
    }
    return false;
}

bool UkuiWorkspaceManager::stop()
{
    if (!d->canSendRequests()) {
        qWarning() << "UkuiWorkspaceManager::stop: can not send requests";
        return false;
    }
    d->transitionTo(State::Stopping);
    kywc_workspace_manager_v1_stop(d->m_manager);
    return true;
}

bool UkuiWorkspaceManager::Private::canSendRequests() const
{
    return m_manager && m_state == State::Bound;
}

bool UkuiWorkspaceManager::Private::canReceiveEvents() const
{
    return m_manager && (m_state == State::Bound || m_state == State::Stopping);
}


void UkuiWorkspaceManager::Private::initListener()
{
    kywc_workspace_manager_v1_add_listener(m_manager, &s_managerListener, this);
}

void UkuiWorkspaceManager::Private::commitPending()
{
    auto newWorkspaces = std::move(m_pendingWorkspaces);
    const auto existingSnapshot = m_workspaces;

    for (auto *workspace : newWorkspaces) {
        workspace->d->commitPending();
        workspace->d->transitionTo(UkuiWorkspace::State::Ready);
        m_workspaces.append(workspace);
        Q_EMIT q->workspaceCreated(workspace, workspace->d->m_uuid);
    }

    for (auto *workspace : existingSnapshot) {
        workspace->d->commitPending();
    }
}

void UkuiWorkspaceManager::Private::transitionTo(UkuiWorkspaceManager::State newState)
{
    if (m_state == newState) {
        return;
    }
    m_state = newState;
    if (m_state == State::Invalidated) {
        Q_EMIT q->invalidated();
    }
}

void UkuiWorkspaceManager::Private::removeWorkspace(UkuiWorkspace *workspace)
{
    m_pendingWorkspaces.removeAll(workspace);
    if (m_workspaces.contains(workspace)) {
        m_workspaces.removeAll(workspace);
        Q_EMIT q->workspaceRemoved(workspace->d->m_uuid);
    }
    workspace->release();
    workspace->deleteLater();
}

void UkuiWorkspaceManager::Private::cleanupWorkspaces(bool remote)
{
    const auto allWorkspaces = m_workspaces + m_pendingWorkspaces;
    m_workspaces.clear();
    m_pendingWorkspaces.clear();
    for (auto *workspace : allWorkspaces) {
        if (remote) {
            workspace->release();
        } else {
            workspace->destroy();
        }
        workspace->deleteLater();
    }
}

void UkuiWorkspaceManager::Private::workspaceCallback(void *data, kywc_workspace_manager_v1 *manager, kywc_workspace_v1 * workspace_v1, const char *uuid)
{
    std::unique_ptr<kywc_workspace_v1, decltype(&kywc_workspace_v1_destroy)>
    workspaceV1Guard(workspace_v1, kywc_workspace_v1_destroy);

    auto p = reinterpret_cast<UkuiWorkspaceManager::Private *>(data);
    if (p->m_manager != manager) {
        qDebug() << "workspaceCallback: manager not match";
        return;
    }
    if (!p->canReceiveEvents()) {
        qDebug() << "workspaceCallback: can not receive events";
        return;
    }
    //转移所有权，由 UkuiWorkspace 负责管理
    auto workspaceV1 = workspaceV1Guard.release();

    auto workspace = new UkuiWorkspace(p->q);
    workspace->setup(workspaceV1);
    workspace->d->m_uuid = UkuiQuick::WaylandClient::fromNullableUtf8(uuid);
    if (p->m_queue) {
        p->m_queue->addProxy(workspaceV1);
    }
    p->m_pendingWorkspaces.append(workspace);
    connect(workspace, &UkuiWorkspace::removed, p->q, [p, workspace]() {
        p->removeWorkspace(workspace);
    });
}

void UkuiWorkspaceManager::Private::doneCallback(void *data, kywc_workspace_manager_v1 *manager)
{
    auto p = reinterpret_cast<UkuiWorkspaceManager::Private *>(data);
    if (p->m_manager != manager) {
        return;
    }
    if (!p->canReceiveEvents()) {
        return;
    }
    p->commitPending();

    // 第一次发送初始化完成信号
    if (!p->m_initialized) {
        p->m_initialized = true;
        Q_EMIT p->q->initialized();
    }
    Q_EMIT p->q->done();
}

void UkuiWorkspaceManager::Private::finishedCallback(void *data, kywc_workspace_manager_v1 *manager)
{
    auto p = reinterpret_cast<UkuiWorkspaceManager::Private *>(data);
    if (p->m_manager != manager) {
        return;
    }
    if (p->m_state == State::Invalidated) {
        return;
    }
    p->q->destroy();
}

UkuiWorkspace::Private::Private(UkuiWorkspace *q) : q(q)
{
}

UkuiWorkspace::UkuiWorkspace(QObject *parent) : QObject(parent), d(new Private(this))
{
}

UkuiWorkspace::~UkuiWorkspace() = default;

void UkuiWorkspace::setup(kywc_workspace_v1 *workspace)
{
    Q_ASSERT(!d->m_workspace);
    Q_ASSERT(workspace);
    d->m_workspace.setup(workspace);
    d->initListener(workspace);
}

bool UkuiWorkspace::isValid() const
{
    return d->m_state == State::Ready;
}

QString UkuiWorkspace::name() const
{
    return d->m_name;
}

uint32_t UkuiWorkspace::position() const
{
    return d->m_position;
}

bool UkuiWorkspace::isActivated() const
{
    return d->m_activated;
}

QString UkuiWorkspace::uuid() const
{
    return d->m_uuid;
}

bool UkuiWorkspace::setName(const QString &name)
{
    if (!d->m_workspace || d->isInvalidated()) {
        qWarning() << "UkuiWorkspace::setName called when invalidated";
        return false;
    }
    if (name.isNull()) {
        qWarning() << "UkuiWorkspace::setName called with null name";
        return false;
    }
    if (name == d->m_name) {
        qWarning() << "UkuiWorkspace::setName called with same name";
        return false;
    }
    kywc_workspace_v1_set_name(d->m_workspace, qPrintable(name));
    return true;
}

bool UkuiWorkspace::setPosition(uint32_t position)
{
    if (!d->m_workspace || d->isInvalidated()) {
        qWarning() << "failed: UkuiWorkspace::setPosition called when invalidated";
        return false;
    }
    if (position == d->m_position) {
        qWarning() << "UkuiWorkspace::setPosition called with same position";
        return false;
    }
    kywc_workspace_v1_set_position(d->m_workspace, position);
    return true;
}

bool UkuiWorkspace::activate()
{
    if (!d->m_workspace || d->isInvalidated()) {
        qWarning() << "UkuiWorkspace::activate called when invalidated";
        return false;
    }
    kywc_workspace_v1_activate(d->m_workspace);
    return true;
}

bool UkuiWorkspace::remove()
{
    if (!d->m_workspace || d->isInvalidated()) {
        qWarning() << "UkuiWorkspace::remove called when invalidated";
        return false;
    }
    kywc_workspace_v1_remove(d->m_workspace);
    return true;
}

void UkuiWorkspace::release()
{
    d->release();
}

void UkuiWorkspace::destroy()
{
    d->destroy();
}

void UkuiWorkspace::Private::initListener(kywc_workspace_v1 *workspace)
{
    kywc_workspace_v1_add_listener(workspace, &s_workspaceListener, this);
}

void UkuiWorkspace::Private::transitionTo(UkuiWorkspace::State newState)
{
    m_state = newState;
}

bool UkuiWorkspace::Private::isInvalidated() const
{
    return m_state == State::Invalidated;
}

void UkuiWorkspace::Private::destroy()
{
    transitionTo(State::Invalidated);
    if (m_workspace) {
        m_workspace.destroy();
    }
}

void UkuiWorkspace::Private::release()
{
    transitionTo(State::Invalidated);
    if (m_workspace) {
        m_workspace.release();
    }
}

void UkuiWorkspace::Private::nameCallback(void *data, kywc_workspace_v1 *workspace, const char *name)
{
    auto p = reinterpret_cast<Private *>(data);
    if (p->m_workspace != workspace) {
        return;
    }
    p->m_pendingName = fromNullableUtf8(name);
}

void UkuiWorkspace::Private::positionCallback(void *data, kywc_workspace_v1 *workspace, uint32_t position)
{
    auto p = reinterpret_cast<Private *>(data);
    if (p->m_workspace != workspace) {
        return;
    }
    p->m_pendingPosition = position;
}

void UkuiWorkspace::Private::activatedCallback(void *data, kywc_workspace_v1 *workspace)
{
    auto p = reinterpret_cast<Private *>(data);
    if (p->m_workspace != workspace) {
        return;
    }
    p->m_pendingActivated = true;
}

void UkuiWorkspace::Private::deactivatedCallback(void *data, kywc_workspace_v1 *workspace)
{
    auto p = reinterpret_cast<Private *>(data);
    if (p->m_workspace != workspace) {
        return;
    }
    p->m_pendingActivated = false;
}

void UkuiWorkspace::Private::removedCallback(void *data, kywc_workspace_v1 *workspace)
{
    auto p = reinterpret_cast<UkuiWorkspace::Private *>(data);
    if (p->m_workspace != workspace) {
        return;
    }
    p->transitionTo(State::Invalidated);
    Q_EMIT p->q->removed();
}

void UkuiWorkspace::Private::commitPending()
{
    bool nameChanged = false;
    bool positionChanged = false;
    bool activatedChanged = false;
    if (m_pendingName && m_name != *m_pendingName) {
        m_name = *m_pendingName;
        nameChanged = true;
    }
    m_pendingName.reset();

    if (m_pendingPosition && m_position != *m_pendingPosition) {
        m_position = *m_pendingPosition;
        positionChanged = true;
    }
    m_pendingPosition.reset();

    if (m_pendingActivated.has_value() && m_activated != *m_pendingActivated) {
        m_activated = *m_pendingActivated;
        activatedChanged = true;
    }
    m_pendingActivated.reset();

    if (nameChanged) {
        Q_EMIT q->nameChanged(m_name);
    }
    if (positionChanged) {
        Q_EMIT q->positionChanged(m_position);
    }
    if (activatedChanged) {
        if (m_activated) {
            Q_EMIT q->activated();
        } else {
            Q_EMIT q->deactivated();
        }
    }
}

}
