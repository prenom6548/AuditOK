/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
#include "appwindowlistmodel.h"
#include "windowsmanager.h"
#include <QMetaEnum>
#include <QDebug>


AppWindowListModel::AppWindowListModel(QObject *parent) : QAbstractListModel(parent)
{
    if (!m_windowManager) {
        m_windowManager = WindowsManager::getInstance();
    }
    connectSignals();
    loadWindows();
}

void AppWindowListModel::connectSignals()
{
    QObject::connect(m_windowManager, &WindowsManager::appWindowAdd, this, &AppWindowListModel::onWindowAdd);
    QObject::connect(m_windowManager, &WindowsManager::appWindowRemoved, this, &AppWindowListModel::onWindowRemoved);
    QObject::connect(m_windowManager, &WindowsManager::appWindowActivated, this, &AppWindowListModel::onWindowActived);
    QObject::connect(m_windowManager, &WindowsManager::appWindowDesktopChanged, this, &AppWindowListModel::onWindowDesktopChanged);
    QObject::connect(m_windowManager, &WindowsManager::titleChanged, this, &AppWindowListModel::onWindowTitleChanged);
    QObject::connect(m_windowManager, &WindowsManager::iconChanged, this, &AppWindowListModel::onWindowIconChanged);
    QObject::connect(m_windowManager, &WindowsManager::geometryChanged, this, &AppWindowListModel::onWindowGeometryChanged);
}

int AppWindowListModel::rowCount(const QModelIndex &parent) const
{
    return m_windowList.count();
}

QVariant AppWindowListModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= m_windowList.count()) {
        return QVariant();
    }
    QString wid = m_windowList.at(i);
    switch (static_cast<Role>(role)) {
    case Role::Id:
        return wid;
    case Role::Uuid:
        return wid;
    case Role::Caption:
        return m_windowManager->windowTitle(wid);
    case Role::Geometry:
        return m_windowManager->geometry(wid);
    case Role::Minimized:
        return m_windowManager->isMinimized(wid);
    case Role::DesktopsId:
        return QVariant::fromValue<QStringList>(m_windowManager->windowDesktops(wid));
    case Role::ScreenId:
        return m_windowManager->windowScreenIndex(wid);
    case Role::Icon:
        return m_windowManager->windowIcon(wid);
    default:
        return {};
    }
}

QHash<int, QByteArray> AppWindowListModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

void AppWindowListModel::removeAppWindow(QString wid)
{
    m_windowManager->removeAppWindow(wid);
}

void AppWindowListModel::moveAppWindowToDesktop(const QString &wid, const QString& srcDesktop, const QString& destDesktop)
{
    m_windowManager->moveAppWindowToDesktop(wid, srcDesktop, destDesktop);
}

void AppWindowListModel::activateAppWindow(QString wid)
{
    m_windowManager->activateAppWindow(wid);
}

bool AppWindowListModel::hasModalChildWindow(QString wid)
{
    return m_windowManager->hasModalChildWindow(wid);
}

void AppWindowListModel::loadWindows()
{
    beginResetModel();
    m_windowList = getWindows();
    endResetModel();
}

void AppWindowListModel::addItem(QString wid)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_windowList.prepend(wid);
    endInsertRows();
}

void AppWindowListModel::onWindowAdd(QString wid)
{
    if (!m_windowManager->skipSwitcher(wid)) {
        addItem(wid);
    }
}

void AppWindowListModel::onWindowRemoved(QString windowId)
{
    for (int index = 0; index < m_windowList.size(); ++index) {
        if (m_windowList.at(index) == windowId) {
            beginRemoveRows(QModelIndex(), index, index);
            m_windowList.removeAt(index);
            endRemoveRows();
            Q_EMIT appWindowRemoved(windowId);
            break;
        }
    }
}

void AppWindowListModel::onWindowActived(QString wid)
{
    for (int i = 0; i < m_windowList.size(); ++i) {
        if (m_windowList.at(i) != wid) {
            continue;
        }

        if (i == 0) {
            return;
        }

        if (!beginMoveRows(QModelIndex(), i, i, QModelIndex(), 0)) {
            qWarning() << "onAppWindowActived beginMoveRows failed";
            return;
        }

        m_windowList.move(i, 0);
        endMoveRows();
        break;
    }
}

void AppWindowListModel::onWindowDesktopChanged(QString windowId)
{
    notifyDataChanged(windowId, Role::DesktopsId);
}

void AppWindowListModel::onWindowIconChanged(QString windowId)
{
    notifyDataChanged(windowId, Role::Icon);
}

void AppWindowListModel::onWindowTitleChanged(QString windowId)
{
    notifyDataChanged(windowId, Role::Caption);
}

void AppWindowListModel::onWindowGeometryChanged(QString windowId)
{
    notifyDataChanged(windowId, Role::Geometry);
}

// TODO: remove?
QVariantMap AppWindowListModel::getItemData(int row) const
{
    if (row < 0 || row >= rowCount()) {
        return {};
    }
    QVariantMap result;
    QModelIndex idx = index(row, 0);
    const auto roles = roleNames();
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        if (it.key() == Role::Icon) {
            result[QString::fromUtf8(it.value())] = data(idx, it.key()).value<QIcon>().name();
        } else {
            result[QString::fromUtf8(it.value())] = data(idx, it.key());
        }
    }
    return result;
}

QStringList AppWindowListModel::getWindows()
{
    QStringList widList = m_windowManager->windows();
    for (int i = widList.size() - 1; i >= 0; --i) {
        QString wid = widList.at(i);
        bool skip = m_windowManager->skipSwitcher(wid);
        if (skip) {
            qDebug() << "[AppWindowListModel] window" << wid << "skipSwitcher:" << skip;
            widList.removeAt(i);
        }
    }
    qDebug() << "[AppWindowListModel] getWindows() - final windows:" << widList.count();
    return widList;
}

int AppWindowListModel::findWindowIndex(const QString &wid)
{
    for (int i = 0; i < m_windowList.size(); ++i) {
        if (m_windowList.at(i) == wid) {
            return i;
        }
    }
    qWarning() << "[AppWindowListModel] can not find window" << wid;
    return -1;
}

void AppWindowListModel::notifyDataChanged(const QString &windowId, int role)
{
    int windowIndex = findWindowIndex(windowId);
    if (windowIndex >= 0) {
        Q_EMIT dataChanged(index(windowIndex, 0), index(windowIndex, 0), QVector<int>{role});
    }
}
