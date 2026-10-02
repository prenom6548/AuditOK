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

#include "kywcdesktopmanager.h"

#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

#include "../screens-manager.h"

KywcDesktopManager::KywcDesktopManager(QObject *parent) : AbstractDesktopManager(parent)
{
    if (UkuiQuick::WorkspaceManager::self()->isAvailable() && UkuiQuick::WorkspaceManager::self()->isInitialized()) {
        initWorkspaces();
    } else {
        connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::availableChanged, this, [this]() {
            if (UkuiQuick::WorkspaceManager::self()->isAvailable() && UkuiQuick::WorkspaceManager::self()->isInitialized()) {
                initWorkspaces();
            }
        });
        connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::initialized, this, [this]() {
            if (UkuiQuick::WorkspaceManager::self()->isAvailable() && UkuiQuick::WorkspaceManager::self()->isInitialized()) {
                initWorkspaces();
            }
        });
        qDebug() << "[KywcDesktopManager] WorkspaceManager::self() not available yet, waiting for initialization";
    }

    initOutputs();
}

void KywcDesktopManager::removeDesktop(const QString &desktop)
{
    if (desktop.isEmpty()) {
        return;
    }
    UkuiQuick::WorkspaceManager::self()->requestRemoveWorkspace(desktop);
}

void KywcDesktopManager::moveDesktop(const QString &srcDesktop, int destDesktopIndex)
{
    int srcDesktopIndex = getDesktopIndex(srcDesktop);
    if (!validateDesktopIndex(srcDesktopIndex) || !validateDesktopIndex(destDesktopIndex)) {
        return;
    }
    UkuiQuick::WorkspaceManager::self()->requestMoveWorkspace(srcDesktop, destDesktopIndex - 1);
    m_workspaceList.move(srcDesktopIndex - 1, destDesktopIndex - 1);
    // desktopMoved行为不等价于positionChanged，移动一个工作区则在它之前的工作区position都会被影响，因此同步发送desktopMoved信号
    Q_EMIT desktopMoved(srcDesktop, destDesktopIndex);
}

void KywcDesktopManager::appendDesktop()
{
    if (!m_workspaceManagerInitialized) {
        qWarning() << "[KywcDesktopManager] WorkspaceManager::self() not available yet, appendDesktop failed!";
        return;
    }
    UkuiQuick::WorkspaceManager::self()->requestCreateWorkspace();
}

QString KywcDesktopManager::getCurrentDesktop()
{
    return m_currentDesktop;
}

void KywcDesktopManager::setCurrentDesktop(const QString &desktop)
{
    if (desktop.isEmpty()) {
        return;
    }
    UkuiQuick::WorkspaceManager::self()->requestActivateWorkspace(desktop);
}

int KywcDesktopManager::getNumberOfDesktops()
{
    return m_workspaceList.size();
}

bool KywcDesktopManager::isCurrentDesktopShowingDesktop()
{
    // wlcom 内部已实现，不需要获取该状态
    return false;
}

QRect KywcDesktopManager::getFullScreenGeometry(const QString &desktop, int screenIndex)
{
    Q_UNUSED(desktop);
    const auto screens = screensList();
    if (screenIndex >= 0 && screenIndex < screens.size()) {
        return screens.at(screenIndex)->geometry();
    } else {
        // 索引超出范围则返回主屏参数
        auto *screen = QGuiApplication::primaryScreen();
        return screen ? screen->geometry() : QRect();
    }
}

void KywcDesktopManager::setDesktopName(const QString &desktop, const QString name)
{
    if (desktop.isEmpty()) {
        return;
    }
    UkuiQuick::WorkspaceManager::self()->requestRenameWorkspace(desktop, name);
}

int KywcDesktopManager::getDesktopIndex(const QString &id)
{
    return findWorkspaceIndex(id) + 1;
}

QString KywcDesktopManager::getDesktopUuid(int desktopIndex)
{
    if (!validateDesktopIndex(desktopIndex)) {
        return QString();
    }
    return m_workspaceList[desktopIndex - 1].id;
}

QString KywcDesktopManager::getOutputUuid(int outputIndex)
{
    const auto screens = screensList();
    QString outputName;
    if (outputIndex >= 0 && screens.size() > outputIndex) {
        outputName = screens.at(outputIndex)->internal()->name();
    }

    for (auto *output : m_outputList) {
        if (output && output->name() == outputName) {
            return output->uuid();
        }
    }

    qWarning() << "[Multitask]: getOutputUuid failed, invalid outputIndex" << outputIndex;
    return QString();
}

bool KywcDesktopManager::isInitialized()
{
    return m_workspaceManagerInitialized;
}

void KywcDesktopManager::onWorkSpaceAdded(const QString &id, const UkuiQuick::Workspace &workspace)
{
    m_workspaceList.append(workspace);
    Q_EMIT desktopAppended(workspace.id);
}

int KywcDesktopManager::findWorkspaceIndex(const QString &id)
{
    for (int i = 0; i < m_workspaceList.size(); i++) {
        if (m_workspaceList[i].id == id) {
            return i;
        }
    }
    return -1;
}

void KywcDesktopManager::onWorkSpaceDeleted(const QString &id)
{
    int index = findWorkspaceIndex(id);
    if (index >= 0) {
        m_workspaceList.removeAt(index);
        Q_EMIT desktopRemoved(id);
    }
}

void KywcDesktopManager::onWorkspaceNameChanged(const QString &id, const QString &name)
{
    int index = findWorkspaceIndex(id);
    if (index >= 0) {
        m_workspaceList[index].name = name;
    }
}

void KywcDesktopManager::onWorkspacePositionChanged(const QString &id, uint32_t position)
{
    int index = findWorkspaceIndex(id);
    if (index >= 0) {
        m_workspaceList[index].position = position;
    }
}

void KywcDesktopManager::onWorkspaceActiveChanged(const QString &id, bool active)
{
    int index = findWorkspaceIndex(id);
    if (index >= 0) {
        m_workspaceList[index].active = active;
    }
}

void KywcDesktopManager::onCurrentWorkspaceChanged(const QString &id)
{
    if (m_currentDesktop != id) {
        QString oldCurrentDesktop = m_currentDesktop;
        m_currentDesktop = id;
        Q_EMIT currentDesktopChanged(oldCurrentDesktop, id);
    }
}

bool KywcDesktopManager::validateDesktopIndex(int desktopIndex)
{
    return desktopIndex >= 1 && desktopIndex <= getNumberOfDesktops();
}

void KywcDesktopManager::initWorkspaces()
{
    // Prevent duplicate initialization
    if (m_workspaceManagerInitialized) {
        return;
    }

    // init workspace list
    m_workspaceList = UkuiQuick::WorkspaceManager::self()->workspaces();

    // init currentDesktop
    for (auto &workspace : m_workspaceList) {
        if (workspace.active) {
            m_currentDesktop = workspace.id;
            break;
        }
    }

    connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::workspaceAdded, this, &KywcDesktopManager::onWorkSpaceAdded, Qt::QueuedConnection);
    connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::workspaceRemoved, this, &KywcDesktopManager::onWorkSpaceDeleted, Qt::QueuedConnection);
    connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::workspaceNameChanged, this, &KywcDesktopManager::onWorkspaceNameChanged, Qt::QueuedConnection);
    connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::workspacePositionChanged, this, &KywcDesktopManager::onWorkspacePositionChanged, Qt::QueuedConnection);
    connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::workspaceActiveChanged, this, &KywcDesktopManager::onWorkspaceActiveChanged, Qt::QueuedConnection);
    connect(UkuiQuick::WorkspaceManager::self(), &UkuiQuick::WorkspaceManager::currentWorkspaceChanged, this, &KywcDesktopManager::onCurrentWorkspaceChanged, Qt::QueuedConnection);

    m_workspaceManagerInitialized = true;
    Q_EMIT workspaceManagerInitialized();
    qDebug() << "[KywcDesktopManager] WorkspaceManager::self() initialized successfull";
}

void KywcDesktopManager::initOutputs()
{
    m_outputManager = new UkuiQuick::OutputManager(this);
    connect(m_outputManager, &UkuiQuick::OutputManager::initialized, this, [this] {
        m_outputList = m_outputManager->outputs();
    });
    connect(m_outputManager, &UkuiQuick::OutputManager::outputAdded, this,
            [this](UkuiQuick::Output *output) {
        m_outputList.append(output);
    });
    connect(m_outputManager, &UkuiQuick::OutputManager::outputRemoved, this,
            [this](const QString &uuid) {
        for (auto *output : m_outputList) {
            if (output && output->uuid() == uuid) {
                m_outputList.removeOne(output);
                return;
            }
        }
    });
}

QList<Screen *> KywcDesktopManager::screensList()
{
    return ScreensManager::instance()->screens();
}
