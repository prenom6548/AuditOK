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

#include "workspace-demo-controller.h"

#include "workspace-manager.h"

#include <QDateTime>
#include <QVariantMap>

#include <utility>

namespace
{
constexpr int MaxLogLines = 80;

QString boolText(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

QString workspaceNameForId(const QVariantList &workspaces, const QString &id)
{
    for (const auto &item : workspaces) {
        const auto workspace = item.toMap();
        if (workspace.value(QStringLiteral("id")).toString() == id) {
            return workspace.value(QStringLiteral("name"), QStringLiteral("<unknown>")).toString();
        }
    }

    return QStringLiteral("<unknown>");
}
}

WorkspaceDemoController::WorkspaceDemoController(QObject *parent)
    : QObject(parent)
    , m_manager(UkuiQuick::WorkspaceManager::self())
{
    connect(m_manager, &UkuiQuick::WorkspaceManager::availableChanged, this, [this](bool available) {
        Q_EMIT availableChanged();

        if (!available) {
            m_workspaces.clear();
            Q_EMIT workspacesChanged();
            selectWorkspace(QString());
            addLog(QStringLiteral("signal availableChanged: property=available value=false"));
            return;
        }

        refresh();
        addLog(QStringLiteral("signal availableChanged: property=available value=%1")
                       .arg(boolText(available)));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::initialized, this, [this] {
        Q_EMIT initializedChanged();
        refresh();
        addLog(QStringLiteral("signal initializedChanged: property=initialized value=%1 workspaceCount=%2")
                       .arg(boolText(initialized()))
                       .arg(m_workspaces.size()));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::workspacesChanged, this, [this] {
        refresh();
        addLog(QStringLiteral("signal workspacesChanged: property=workspaces count=%1")
                       .arg(m_workspaces.size()));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::workspaceAdded, this,
            [this](const QString &id, const UkuiQuick::Workspace &workspace) {
        const QString workspaceId = workspace.id.isEmpty() ? id : workspace.id;
        refresh();
        addLog(QStringLiteral("signal workspaceAdded: property=workspaces addedId=%1 workspaceId=%2 name=%3 position=%4 active=%5 count=%6")
                       .arg(id)
                       .arg(workspaceId)
                       .arg(workspace.name)
                       .arg(workspace.position)
                       .arg(boolText(workspace.active))
                       .arg(m_workspaces.size()));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::workspaceRemoved, this, [this](const QString &id) {
        const QString workspaceName = workspaceNameForId(m_workspaces, id);
        refresh();
        addLog(QStringLiteral("signal workspaceRemoved: property=workspaces removedId=%1 name=%2 count=%3")
                       .arg(id)
                       .arg(workspaceName)
                       .arg(m_workspaces.size()));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::workspaceNameChanged, this,
            [this](const QString &id, const QString &name) {
        refresh();
        addLog(QStringLiteral("signal workspaceNameChanged: property=name id=%1 value=%2")
                       .arg(id, name));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::workspacePositionChanged, this,
            [this](const QString &id, uint32_t position) {
        refresh();
        addLog(QStringLiteral("signal workspacePositionChanged: property=position id=%1 value=%2")
                       .arg(id)
                       .arg(position));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::workspaceActiveChanged, this,
            [this](const QString &id, bool active) {
        refresh();
        addLog(QStringLiteral("signal workspaceActiveChanged: property=active id=%1 value=%2")
                       .arg(id, boolText(active)));
    });
    connect(m_manager, &UkuiQuick::WorkspaceManager::currentWorkspaceChanged, this, [this](const QString &id) {
        refresh();
        addLog(QStringLiteral("signal currentWorkspaceChanged: property=currentWorkspace id=%1")
                       .arg(id));
    });

    addLog(QStringLiteral("workspace demo controller started"));
    refresh();
}

bool WorkspaceDemoController::available() const
{
    return m_manager && m_manager->isAvailable();
}

bool WorkspaceDemoController::initialized() const
{
    return m_manager && m_manager->isInitialized();
}

QVariantList WorkspaceDemoController::workspaces() const
{
    return m_workspaces;
}

QString WorkspaceDemoController::selectedId() const
{
    return m_selectedId;
}

QString WorkspaceDemoController::selectedName() const
{
    return selectedWorkspace().value(QStringLiteral("name")).toString();
}

int WorkspaceDemoController::selectedPosition() const
{
    return selectedWorkspace().value(QStringLiteral("position"), -1).toInt();
}

bool WorkspaceDemoController::hasSelection() const
{
    return !m_selectedId.isEmpty();
}

QStringList WorkspaceDemoController::logs() const
{
    return m_logs;
}

void WorkspaceDemoController::refresh()
{
    rebuildWorkspaceList();
}

void WorkspaceDemoController::selectWorkspace(const QString &id)
{
    if (m_selectedId == id) {
        return;
    }

    m_selectedId = id;
    Q_EMIT selectedIdChanged();
    Q_EMIT selectedWorkspaceChanged();
}

bool WorkspaceDemoController::createWorkspace(const QString &name, int position)
{
    if (!available()) {
        addLog(QStringLiteral("create rejected: workspace manager unavailable"));
        return false;
    }

    const QString workspaceName = name.trimmed().isEmpty()
            ? QStringLiteral("Workspace %1").arg(QDateTime::currentDateTime().toString(QStringLiteral("HHmmss")))
            : name.trimmed();
    const bool ok = m_manager->requestCreateWorkspace(workspaceName, position);
    addLog(QStringLiteral("create request: name=%1 position=%2 result=%3")
                   .arg(workspaceName)
                   .arg(position)
                   .arg(boolText(ok)));
    return ok;
}

bool WorkspaceDemoController::removeSelectedWorkspace()
{
    if (!hasSelection()) {
        addLog(QStringLiteral("remove rejected: no workspace selected"));
        return false;
    }

    const QString workspaceId = m_selectedId;
    const QString workspaceName = selectedName();
    const bool ok = m_manager->requestRemoveWorkspace(workspaceId);
    addLog(QStringLiteral("remove request: id=%1 name=%2 result=%3")
                   .arg(workspaceId, workspaceName, boolText(ok)));
    return ok;
}

bool WorkspaceDemoController::renameSelectedWorkspace(const QString &name)
{
    if (!hasSelection()) {
        addLog(QStringLiteral("rename rejected: no workspace selected"));
        return false;
    }

    const QString workspaceName = name.trimmed();
    if (workspaceName.isEmpty()) {
        addLog(QStringLiteral("rename rejected: empty name"));
        return false;
    }

    const bool ok = m_manager->requestRenameWorkspace(m_selectedId, workspaceName);
    addLog(QStringLiteral("rename request: id=%1 name=%2 result=%3")
                   .arg(m_selectedId, workspaceName, boolText(ok)));
    return ok;
}

bool WorkspaceDemoController::moveSelectedWorkspace(int position)
{
    if (!hasSelection()) {
        addLog(QStringLiteral("move rejected: no workspace selected"));
        return false;
    }

    const bool ok = m_manager->requestMoveWorkspace(m_selectedId, position);
    addLog(QStringLiteral("move request: id=%1 position=%2 result=%3")
                   .arg(m_selectedId)
                   .arg(position)
                   .arg(boolText(ok)));
    return ok;
}

bool WorkspaceDemoController::moveSelectedWorkspaceBy(int offset)
{
    if (!hasSelection()) {
        addLog(QStringLiteral("move rejected: no workspace selected"));
        return false;
    }

    return moveSelectedWorkspace(selectedPosition() + offset);
}

bool WorkspaceDemoController::activateSelectedWorkspace()
{
    if (!hasSelection()) {
        addLog(QStringLiteral("activate rejected: no workspace selected"));
        return false;
    }

    const bool ok = m_manager->requestActivateWorkspace(m_selectedId);
    addLog(QStringLiteral("activate request: id=%1 result=%2").arg(m_selectedId, boolText(ok)));
    return ok;
}

void WorkspaceDemoController::clearLogs()
{
    if (m_logs.isEmpty()) {
        return;
    }

    m_logs.clear();
    Q_EMIT logsChanged();
}

void WorkspaceDemoController::rebuildWorkspaceList()
{
    QVariantList nextWorkspaces;

    if (initialized()) {
        const auto workspaceList = m_manager->workspaces();
        for (const auto &workspace : workspaceList) {
            QVariantMap item;
            item.insert(QStringLiteral("id"), workspace.id);
            item.insert(QStringLiteral("name"), workspace.name);
            item.insert(QStringLiteral("position"), workspace.position);
            item.insert(QStringLiteral("active"), workspace.active);
            nextWorkspaces.append(item);
        }
    }

    m_workspaces = nextWorkspaces;
    Q_EMIT workspacesChanged();

    QString firstId;
    QString activeId;
    bool selectedStillExists = false;
    for (const auto &item : std::as_const(m_workspaces)) {
        const auto workspace = item.toMap();
        const QString id = workspace.value(QStringLiteral("id")).toString();
        if (firstId.isEmpty()) {
            firstId = id;
        }
        if (workspace.value(QStringLiteral("active")).toBool()) {
            activeId = id;
        }
        if (id == m_selectedId) {
            selectedStillExists = true;
        }
    }

    if (!selectedStillExists) {
        selectWorkspace(activeId.isEmpty() ? firstId : activeId);
    } else {
        Q_EMIT selectedWorkspaceChanged();
    }
}

void WorkspaceDemoController::addLog(const QString &message)
{
    const QString line = QStringLiteral("[%1] %2")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")), message);
    m_logs.prepend(line);
    while (m_logs.size() > MaxLogLines) {
        m_logs.removeLast();
    }
    Q_EMIT logsChanged();
}

QVariantMap WorkspaceDemoController::selectedWorkspace() const
{
    for (const auto &item : m_workspaces) {
        const auto workspace = item.toMap();
        if (workspace.value(QStringLiteral("id")).toString() == m_selectedId) {
            return workspace;
        }
    }

    return {};
}
