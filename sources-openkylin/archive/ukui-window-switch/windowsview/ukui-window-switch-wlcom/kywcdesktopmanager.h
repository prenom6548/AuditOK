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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */

#ifndef KYWCDESKTOPMANAGER_H
#define KYWCDESKTOPMANAGER_H

#include "../abstractdesktopmanager.h"
#include <QObject>
#include <QList>

#include <workspace-manager.h>
#include <output-manager.h>

class Screen;

class KywcDesktopManager : public AbstractDesktopManager
{
    Q_OBJECT
public:
    explicit KywcDesktopManager(QObject *parent = nullptr);
    ~KywcDesktopManager() = default;

    // AbstractDesktopManager interface
    void removeDesktop(const QString &desktop) override;
    void moveDesktop(const QString &srcDesktop, int destDesktopIndex) override;
    void appendDesktop() override;
    void setCurrentDesktop(const QString &desktop) override;
    QString getCurrentDesktop() override;
    int getNumberOfDesktops() override;
    bool isCurrentDesktopShowingDesktop() override;
    QRect getFullScreenGeometry(const QString &desktop, int screenIndex) override;
    void setDesktopName(const QString &desktop, const QString name) override;
    int getDesktopIndex(const QString &id) override;
    QString getDesktopUuid(int desktopIndex) override;
    QString getOutputUuid(int outputIndex) override;
    bool isInitialized() override;

private slots:
    void onWorkSpaceAdded(const QString &id, const UkuiQuick::Workspace &workspace);
    void onWorkSpaceDeleted(const QString &id);
    void onWorkspaceNameChanged(const QString &id, const QString &name);
    void onWorkspacePositionChanged(const QString &id, uint32_t position);
    void onWorkspaceActiveChanged(const QString &id, bool active);
    void onCurrentWorkspaceChanged(const QString &id);

private:
    int findWorkspaceIndex(const QString &id);
    bool validateDesktopIndex(int desktopIndex);
    QList<Screen *> screensList();
    void initWorkspaces();
    void initOutputs();

private:
    UkuiQuick::OutputManager *m_outputManager = nullptr;
    QList<UkuiQuick::Workspace> m_workspaceList;
    QList<UkuiQuick::Output *> m_outputList;
    QString m_currentDesktop;
    bool m_workspaceManagerInitialized = false;
};

#endif // KYWCDESKTOPMANAGER_H
