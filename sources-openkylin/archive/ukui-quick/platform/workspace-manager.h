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

#ifndef UKUI_QUICK_WORKSPACE_MANAGER_H
#define UKUI_QUICK_WORKSPACE_MANAGER_H

#include <QList>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>

#include <optional>

namespace UkuiQuick {

struct Workspace
{
    QString id;
    QString name;
    int position = -1;
    bool active = false;
};

using WorkspaceList = QList<Workspace>;

class WorkspaceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ isAvailable NOTIFY availableChanged)
public:
    static WorkspaceManager *self();
    ~WorkspaceManager() override;

    bool isAvailable() const;
    bool isInitialized() const;
    WorkspaceList workspaces() const;

    Q_INVOKABLE bool requestCreateWorkspace(const QString &name = QString(), int position = -1);
    Q_INVOKABLE bool requestRemoveWorkspace(const QString &id);
    Q_INVOKABLE bool requestRenameWorkspace(const QString &id, const QString &name);
    Q_INVOKABLE bool requestMoveWorkspace(const QString &id, int position);
    Q_INVOKABLE bool requestActivateWorkspace(const QString &id);

Q_SIGNALS:
    void availableChanged(bool);
    void initialized();
    void workspacesChanged();
    void workspaceAdded(const QString &id, const Workspace &workspace);
    void workspaceRemoved(const QString &id);
    void workspaceNameChanged(const QString &id, const QString &name);
    void workspacePositionChanged(const QString &id, uint32_t position);
    void workspaceActiveChanged(const QString &id, bool active);
    void currentWorkspaceChanged(const QString &id);

private:
    explicit WorkspaceManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(WorkspaceManager)
    Q_DISABLE_MOVE(WorkspaceManager)

    class Private;
    Private *d = nullptr;
};

} // namespace UkuiQuick

Q_DECLARE_METATYPE(UkuiQuick::Workspace)
Q_DECLARE_METATYPE(UkuiQuick::WorkspaceList)


#endif // UKUI_QUICK_WORKSPACE_MANAGER_H
