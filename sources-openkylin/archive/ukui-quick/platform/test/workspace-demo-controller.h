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

#ifndef WORKSPACE_DEMO_CONTROLLER_H
#define WORKSPACE_DEMO_CONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QVariantList>

namespace UkuiQuick {
class WorkspaceManager;
}

class WorkspaceDemoController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)
    Q_PROPERTY(QVariantList workspaces READ workspaces NOTIFY workspacesChanged)
    Q_PROPERTY(QString selectedId READ selectedId WRITE selectWorkspace NOTIFY selectedIdChanged)
    Q_PROPERTY(QString selectedName READ selectedName NOTIFY selectedWorkspaceChanged)
    Q_PROPERTY(int selectedPosition READ selectedPosition NOTIFY selectedWorkspaceChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectedWorkspaceChanged)
    Q_PROPERTY(QStringList logs READ logs NOTIFY logsChanged)

public:
    explicit WorkspaceDemoController(QObject *parent = nullptr);

    bool available() const;
    bool initialized() const;
    QVariantList workspaces() const;

    QString selectedId() const;
    QString selectedName() const;
    int selectedPosition() const;
    bool hasSelection() const;

    QStringList logs() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void selectWorkspace(const QString &id);
    Q_INVOKABLE bool createWorkspace(const QString &name, int position);
    Q_INVOKABLE bool removeSelectedWorkspace();
    Q_INVOKABLE bool renameSelectedWorkspace(const QString &name);
    Q_INVOKABLE bool moveSelectedWorkspace(int position);
    Q_INVOKABLE bool moveSelectedWorkspaceBy(int offset);
    Q_INVOKABLE bool activateSelectedWorkspace();
    Q_INVOKABLE void clearLogs();

Q_SIGNALS:
    void availableChanged();
    void initializedChanged();
    void workspacesChanged();
    void selectedIdChanged();
    void selectedWorkspaceChanged();
    void logsChanged();

private:
    void rebuildWorkspaceList();
    void addLog(const QString &message);
    QVariantMap selectedWorkspace() const;

    UkuiQuick::WorkspaceManager *m_manager = nullptr;
    QVariantList m_workspaces;
    QString m_selectedId;
    QStringList m_logs;
};

#endif // WORKSPACE_DEMO_CONTROLLER_H
