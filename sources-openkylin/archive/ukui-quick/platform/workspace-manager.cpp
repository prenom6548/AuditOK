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
 */

#include "workspace-manager.h"

#include "registry.h"
#include "ukui-workspace-manager.h"
#include "wayland-integration_p.h"
#include <KWayland/Client/connection_thread.h>

#include <QCoreApplication>
#include <QPointer>
#include <algorithm>
#include <QDebug>
#include <qguiapplication.h>
#include <mutex>

namespace UkuiQuick {

class WorkspaceManager::Private
{
public:
    explicit Private(WorkspaceManager *q);
    ~Private();
    void bindWorkspaceManager(quint32 name, quint32 version);
    static Workspace buildWorkspace(const WaylandClient::UkuiWorkspace *workspace);
    // release: 是否向服务器请求释放资源
    void cleanupWorkspaceManager(bool release);

    WorkspaceList workspaces() const;

    void initialize();
    void setAvailable(bool newAvailable);
    void setInitialized(bool newInitialized);

    WorkspaceManager *q = nullptr;
    QPointer<WaylandClient::Registry> m_registry;
    WaylandClient::UkuiWorkspaceManager *m_manager {nullptr};
    bool m_available = false;
    bool m_initialized = false;
};

WorkspaceManager::Private::Private(WorkspaceManager *q)
    : q(q)
{
}

WorkspaceManager::Private::~Private()
{
    cleanupWorkspaceManager(true);
}

WorkspaceList WorkspaceManager::Private::workspaces() const
{
    WorkspaceList workspaceList;
    if (!m_manager) {
        return workspaceList;
    }

    workspaceList.reserve(m_manager->workspaces().size());

    for (auto *workspace : m_manager->workspaces()) {
        auto info = buildWorkspace(workspace);
        workspaceList.append(info);
    }

    std::sort(workspaceList.begin(), workspaceList.end(), [](const Workspace &lhs, const Workspace &rhs) {
        return lhs.position < rhs.position;
    });

    return workspaceList;
}

void WorkspaceManager::Private::initialize()
{
    m_registry = WaylandIntegration::self()->registry();

    connect(m_registry, &WaylandClient::Registry::ukuiWorkspaceManagerAnnounced,
                     q, [this](quint32 name, quint32 version) {
        bindWorkspaceManager(name, version);
    });
    connect(m_registry, &WaylandClient::Registry::ukuiWorkspaceManagerRemoved,
                     q, [this](quint32) {
        cleanupWorkspaceManager(true);
    });

    const auto interfaceInfo = m_registry->interface(WaylandClient::Registry::Interface::UkuiWorkspaceManager);
    if (interfaceInfo.name != 0) {
        bindWorkspaceManager(interfaceInfo.name, interfaceInfo.version);
    }
}

void WorkspaceManager::Private::bindWorkspaceManager(quint32 name, quint32 version)
{
    if (m_manager || !m_registry) {
        return;
    }

    m_manager = m_registry->createUkuiWorkspaceManager(name, version, q);
    if (!m_manager) {
        setAvailable(false);
        return;
    }
    setAvailable(true);
    connect(m_manager, &WaylandClient::UkuiWorkspaceManager::workspaceCreated,
            q, [this](const WaylandClient::UkuiWorkspace* workspace, const QString& id) {
        connect(workspace, &WaylandClient::UkuiWorkspace::nameChanged,q, [id, this](const QString& name){
             Q_EMIT q->workspaceNameChanged(id, name);
        });
        connect(workspace, &WaylandClient::UkuiWorkspace::positionChanged, q, [id, this](uint32_t position) {
            Q_EMIT q->workspacePositionChanged(id, position);
        });
        connect(workspace, &WaylandClient::UkuiWorkspace::activated,q, [id, this]() {
            Q_EMIT q->workspaceActiveChanged(id, true);
            Q_EMIT q->currentWorkspaceChanged(id);
        });
        connect(workspace, &WaylandClient::UkuiWorkspace::deactivated, q, [id, this]() {
            Q_EMIT q->workspaceActiveChanged(id, false);
        });

        Q_EMIT q->workspaceAdded(id, buildWorkspace(workspace));
    });
    connect(m_manager, &WaylandClient::UkuiWorkspaceManager::done, q, &WorkspaceManager::workspacesChanged);
    connect(m_manager, &WaylandClient::UkuiWorkspaceManager::workspaceRemoved, q, [this](const QString &id) {
        Q_EMIT q->workspaceRemoved(id);
    });
    connect(m_manager, &WaylandClient::UkuiWorkspaceManager::initialized, q, [this]() {
        setInitialized(true);
    });
    connect(m_manager, &WaylandClient::UkuiWorkspaceManager::invalidated, q, [this]() {
        cleanupWorkspaceManager(false);
    });
}

Workspace WorkspaceManager::Private::buildWorkspace(const WaylandClient::UkuiWorkspace* workspace)
{
    Workspace info;
    info.id = workspace->uuid();
    info.name = workspace->name();
    info.position = static_cast<int>(workspace->position());
    info.active = workspace->isActivated();
    return info;
}

void WorkspaceManager::Private::cleanupWorkspaceManager(bool release)
{
    if (m_manager) {
        if (release) {
            m_manager->release();
        } else {
            m_manager->destroy();
        }
        m_manager->deleteLater();
        m_manager = nullptr;
    }

    setAvailable(false);
    setInitialized(false);
}

void WorkspaceManager::Private::setAvailable(bool newAvailable)
{
    if (m_available == newAvailable) {
        return;
    }

    m_available = newAvailable;
    Q_EMIT q->availableChanged(newAvailable);
}

void WorkspaceManager::Private::setInitialized(bool newInitialized)
{
    if (m_initialized == newInitialized) {
        return;
    }

    m_initialized = newInitialized;
    if (m_initialized) {
        Q_EMIT q->initialized();
    }
}

WorkspaceManager *WorkspaceManager::self()
{
    static std::once_flag initialized;
    static WorkspaceManager *instance = nullptr;
    std::call_once(initialized, [] {
        instance = new WorkspaceManager(qGuiApp);
    });
    return instance;
}

/*!
    \class UkuiQuick::Workspace
    \brief 工作区数据结构，描述单个工作区的属性信息

    该结构体包含工作区的基本属性：
    \list
        \li id: 工作区的唯一标识符（UUID）
        \li name: 工作区的显示名称
        \li position: 工作区在列表中的位置索引（-1 表示未设置）
        \li active: 工作区是否为当前激活状态
    \endlist
    \sa WorkspaceManager, WorkspaceList
 */

/*!
    \typealias UkuiQuick::WorkspaceList
 */

/*!
    \class UkuiQuick::WorkspaceManager
    \brief 工作区管理单例类，提供工作区的创建、删除、重命名、移动和激活功能

    该类基于 Wayland 协议与系统工作区服务交互，提供以下核心能力：

    \list
        \li 管理工作区生命周期（创建、删除）
        \li 操作工作区属性（重命名、移动位置）
        \li 控制工作区状态（激活指定工作区）
        \li 监听工作区变化并发送信号通知
    \endlist

    \note 该类为单例模式，通过 self() 方法获取实例

    \sa Workspace , WorkspaceList
*/

WorkspaceManager::WorkspaceManager(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->initialize();
}

WorkspaceManager::~WorkspaceManager()
{
    delete d;
    d = nullptr;
}

/*!
   \property WorkspaceManager::available
   \brief 检查工作区管理服务是否可用,服务可用返回 true ，否则返回 false
   \note 不可用状态时， 使用 request 相关接口会失败
 */
bool WorkspaceManager::isAvailable() const
{
    return d->m_available;
}


/*!
 * \brief 检查工作区管理服务是否已初始化完成
 * 初始化完成返回 true，否则返回 false
 * \note 初始化完成意味着可以通过 workspaces() 方法获取所有工作区信息
 */
bool WorkspaceManager::isInitialized() const
{
    return d->m_initialized;
}

/*!
 * \brief 获取所有工作区列表
 * \note 列表已按照 position 升序排列
 * \sa Workspace , WorkspaceList
 */
WorkspaceList WorkspaceManager::workspaces() const
{
    return d->workspaces();
}

/*!
 * \brief 请求创建 workspace，\a name 工作区名称，\a position 工作区位置
 *  返回是否成功发送请求
 */
bool WorkspaceManager::requestCreateWorkspace(const QString &name, int position)
{
    if (d->m_manager) {
        auto count = d->m_manager->workspaceCount();
        uint32_t actualPosition = count;
        if (position >= 0 && position < static_cast<int>(count)) {
            actualPosition = static_cast<uint32_t>(position);
        }
        return d->m_manager->requestCreateWorkspace(name, actualPosition);
    }
    return false;
}

/*!
 * \brief 请求删除 workspace，\a id 工作区唯一id
 *  返回是否成功发送请求
 */
bool WorkspaceManager::requestRemoveWorkspace(const QString &id)
{
    if (d->m_manager) {
        return d->m_manager->requestRemoveWorkspace(id);
    }
    return false;
}

/*!
 * \brief 请求重命名 workspace，\a id 工作区唯一id，\a name 工作区名称
 *  返回是否成功发送请求
 */
bool WorkspaceManager::requestRenameWorkspace(const QString &id, const QString &name)
{
    if (d->m_manager) {
        if (auto* workspace = d->m_manager->workspace(id)) {
            return workspace->setName(name);
        }
    }
    return false;
}

/*!
 * \brief 请求移动 workspace，\a id 工作区唯一id，\a position 工作区位置
 *  返回是否成功发送请求
 */
bool WorkspaceManager::requestMoveWorkspace(const QString &id, int position)
{
    if (d->m_manager) {
        if (auto* workspace = d->m_manager->workspace(id)) {
            // 确保 position 在有效范围内
            auto count = d->m_manager->workspaceCount();
            uint32_t actualPosition = 0;
            if (count > 0) {
                if (position < 0) {
                    actualPosition = 0;
                } else if (position >= static_cast<int>(count)) {
                    actualPosition = count - 1;
                } else {
                    actualPosition = static_cast<uint32_t>(position);
                }
            }

            return workspace->setPosition(actualPosition);
        }
    }
    return false;
}

/*!
 * \brief 请求激活 workspace，\a id 工作区唯一id
 *  返回是否成功发送请求
 */
bool WorkspaceManager::requestActivateWorkspace(const QString &id)
{
    if (d->m_manager) {
        if (auto* workspace = d->m_manager->workspace(id)) {
            return workspace->activate();
        }
    }
    return false;
}

/*!
    \fn void WorkspaceManager::initializedChanged()
    所有工作区初始化完成会发送此信号，代表 workspaces 接口获取完整的工作区状态
 */

/*!
    \fn void WorkspaceManager::workspacesChanged()
    每一轮工作区的属性的改变后都会发送此信号， 也可监听单独的信号改变

    \sa workspaceAdded workspaceNameChanged workspacePositionChanged workspaceActiveChanged 等
 */

/*!
    \fn void WorkspaceManager::workspaceAdded(const QString &id, const Workspace &workspace)
    新增工作区时会发送此信号，\a id 工作区唯一id, \a Workspace 工作区属性信息
 */

/*!
    \fn void WorkspaceManager::workspaceRemoved(const QString &id)
    删除工作区会发送此信号，\a id 工作区唯一id
 */

/*!
    \fn void WorkspaceManager::workspaceNameChanged(const QString &id, const QString &name)
    工作区名称改变会发送此信号，\a id 工作区唯一id， \a name 工作区名称
 */

/*!
    \fn void WorkspaceManager::workspacePositionChanged(const QString &id, uint32_t position)
    工作区位置改变会发送此信号，\a id 工作区唯一id， \a position 工作区位置
*/

/*!
    \fn void WorkspaceManager::workspaceActiveChanged(const QString &id, bool active)
    工作区活动状态改变会发送此信号，\a id 工作区唯一id， \a active 工作区是否激活
*/

/*!
    \fn void WorkspaceManager::currentWorkspaceChanged(const QString &id)
    当前工作区改变会发送此信号，\a id 工作区唯一id
*/

} // namespace UkuiQuick
