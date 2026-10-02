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
#include "desktoplistmodel.h"

#include <algorithm>
#include <QMetaEnum>
#include <QGuiApplication>
#include <QScreen>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>
#include "desktopmanager.h"

DesktopListModel::DesktopListModel(QObject *parent) : QAbstractListModel(parent)
{
    if (!m_desktopManager) {
        m_desktopManager = DesktopManager::getInstance();
    }

    QObject::connect(m_desktopManager, &DesktopManager::desktopAppended, this, &DesktopListModel::onDesktopAppend);
    QObject::connect(m_desktopManager, &DesktopManager::desktopRemoved, this, &DesktopListModel::onDesktopRemoved);
    QObject::connect(m_desktopManager, &DesktopManager::desktopMoved, this, &DesktopListModel::onDesktopMoved);
    QObject::connect(m_desktopManager, &DesktopManager::currentDesktopChanged, this, &DesktopListModel::currentDesktopChanged);
    QObject::connect(m_desktopManager, &DesktopManager::renameDesktopSignal, this, &DesktopListModel::renameDesktopSignal);

    if (m_desktopManager->isInitialized()) {
        init();
    } else {
        QObject::connect(m_desktopManager, &DesktopManager::initialized, this, [this]{
            init();
        });
    }
}

int DesktopListModel::rowCount(const QModelIndex &parent) const
{
    return m_desktopList.count();
}

QVariant DesktopListModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= m_desktopList.count()) {
        return QVariant();
    }

    Desktop desktop = m_desktopList.at(i);
    switch (static_cast<Role>(role)) {
    case Role::Id:
        return i + 1;
    case Role::Uuid:
            return desktop.uuid();
    case Role::CustomName:
        return desktop.customName();
    case Role::Name:
        if (desktop.customName().isEmpty()) {
            int desktopNumber = i + 1;
            return tr("Desktop") + QString(" %1").arg(desktopNumber);
        } else {
            return desktop.customName();
        }
    case Role::ItemType:
        return desktop.type() == Desktop::DesktopType::NewDesktop ? QString("newDesktop")
                                                                  : QString("workSpace");
    default:
        return {};
    }
}

bool DesktopListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    if (role == Role::CustomName) {
        m_desktopList[index.row()].setCustomName(value.toString());
        Q_EMIT dataChanged(index, index, QVector<int>{Role::CustomName, Role::Name});
        m_desktopManager->setDesktopName(m_desktopList[index.row()].uuid(), value.toString());
        m_desktopManager->saveDesktopNames(getDesktopNames());
        return true;
    } else if (role == Role::ItemType) {
        m_desktopList[index.row()].setDesktopType(Desktop::DesktopType(value.toInt()));
        Q_EMIT dataChanged(index, index, QVector<int>{Role::ItemType});
    } else if (role == Role::Uuid) {
        m_desktopList[index.row()].setUuid(value.toString());
        Q_EMIT dataChanged(index, index, QVector<int>{Role::Uuid});
        return true;
    }

    return false;
}

QHash<int, QByteArray> DesktopListModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

bool DesktopListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row > m_desktopList.count() || m_desktopList.count() > MAX_DESKTOP_COUNT) {
        return false;
    }

    beginInsertRows(parent, row, row);

    Desktop desktop;
    desktop.setDesktopType(Desktop::DesktopType::WorkSpace);
    desktop.setUuid(m_desktopManager->getDesktopUuid(row + 1));
    m_desktopList.insert(m_desktopList.count() - 1, desktop);

    endInsertRows();

    return true;
}

bool DesktopListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!rowIsValid(row)) {
        return false;
    }

    beginRemoveRows(parent, row, row);
    m_desktopList.removeAt(row);
    endRemoveRows();
    return true;
}

bool DesktopListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                const QModelIndex &destinationParent, int destinationChild)
{
    if (!rowIsValid(sourceRow) || !rowIsValid(destinationChild)) {
        return false;
    }
    // only support count == 1
    // 需要注意 beginMoveRows 单个节点移动时，上移差是1，下移差是2
    if (sourceRow < destinationChild) {
        // 右移/下移
        beginMoveRows(sourceParent,sourceRow, sourceRow, destinationParent, destinationChild + 1);
    } else {
        // 左移/上移
        beginMoveRows(sourceParent,sourceRow, sourceRow, destinationParent, destinationChild);
    }
    m_desktopList.move(sourceRow, destinationChild);
    endMoveRows();
    return true;
}

void DesktopListModel::removeDesktop(const QString &desktop)
{
    m_desktopManager->removeDesktop(desktop);
}

void DesktopListModel::moveDesktop(const QString &srcDesktop, int destDesktopIndex)
{
    m_desktopManager->moveDesktop(srcDesktop, destDesktopIndex);
}

void DesktopListModel::appendDesktop()
{
    m_desktopManager->appendDesktop();
}

void DesktopListModel::setCurrentDesktop(const QString &desktop)
{
    m_desktopManager->setCurrentDesktop(desktop);
}

QString DesktopListModel::getCurrentDesktop()
{
    return m_desktopManager->getCurrentDesktop();
}

QString DesktopListModel::getCurrentDesktopName()
{
    int row = getModelIndexByUuid(getCurrentDesktop());
    return row >= 0 ? data(index(row, 0), Role::Name).toString() : "";
}

QString DesktopListModel::getOutputUuid(int outputIndex)
{
    return m_desktopManager->getOutputUuid(outputIndex);
}

bool DesktopListModel::isCurrentDesktopShowingDesktop()
{
    return m_desktopManager->isCurrentDesktopShowingDesktop();
}

QRect DesktopListModel::getFullScreenGeometry(const QString &desktop, int screenIndex)
{
    return m_desktopManager->getFullScreenGeometry(desktop, screenIndex);
}

void DesktopListModel::openDesktopMenu(const QString &desktop, QQuickItem *item)
{
    m_desktopManager->openDesktopMenu(desktop, item);
}

bool DesktopListModel::rowIsValid(int row) const
{
    return row >= 0 && row < m_desktopList.count();
}

void DesktopListModel::init()
{
    beginResetModel();
    m_desktopList.clear();

    const auto desktopCount = m_desktopManager->getNumberOfDesktops();
    QStringList desktopNameList = m_desktopManager->getDesktopNames();
    for (int desktopIndex = 1; desktopIndex <= desktopCount; desktopIndex++) {
        Desktop desktop;
        desktop.setDesktopType(Desktop::DesktopType::WorkSpace);
        if (desktopIndex < desktopNameList.size()) {
            desktop.setCustomName(desktopNameList.at(desktopIndex - 1));
        }
        desktop.setUuid(m_desktopManager->getDesktopUuid(desktopIndex));
        m_desktopList.append(desktop);
    }

    // 在末尾添加空的Desktop,指代“newDesktop”
    if (desktopCount < MAX_DESKTOP_COUNT) {
        Desktop desktop;
        desktop.setDesktopType(Desktop::DesktopType::NewDesktop);
        m_desktopList.append(desktop);
    }

    endResetModel();
}

QStringList DesktopListModel::getDesktopNames()
{
    QStringList nameList;
    for (const auto &desktop : m_desktopList) {
        nameList.append(desktop.customName());
    }
    return nameList;
}

int DesktopListModel::getModelIndexByUuid(const QString &uuid)
{
    for (int i = 0; i < m_desktopList.count(); ++i) {
        if (m_desktopList.at(i).uuid() == uuid) {
            return i;
        }
    }
    return -1;
}

void DesktopListModel::onDesktopAppend(const QString &desktop)
{
    //当工作区数量达到最大时，再点击“新建工作区”不需要model新增item，只需将model最后一项的属性ItemType由“newDesktop”改为“workSpace”，并修改其他必须修改的属性
    //其他情况下则新增item
    if (rowCount() == MAX_DESKTOP_COUNT) {
        // 修改属性前发送信号，否则item接收不到
        Q_EMIT desktopNewToAppended(desktop);
        setData(index(MAX_DESKTOP_COUNT - 1, 0), Desktop::DesktopType::WorkSpace, Role::ItemType);
        setData(index(MAX_DESKTOP_COUNT - 1, 0), "", Role::CustomName);
        setData(index(MAX_DESKTOP_COUNT - 1, 0), desktop, Role::Uuid);
    } else {
        insertRows(rowCount() - 1, 1, QModelIndex());
        Q_EMIT desktopAppended(desktop);
    }
    m_desktopManager->setCurrentDesktop(desktop);
    m_desktopManager->saveDesktopNames(getDesktopNames());
}

void DesktopListModel::onDesktopRemoved(const QString &desktop)
{
    int row = getModelIndexByUuid(desktop);
    //当工作区数量是最大时，且最后一项为“workSpace”，由于要播放删除动画，需要先添加“newDesktop”，再删除desktopIndex对应工作区
    if (rowCount() == MAX_DESKTOP_COUNT && index(MAX_DESKTOP_COUNT - 1, 0).data(Role::ItemType).toString() == QString("workSpace")) {
        insertRows(MAX_DESKTOP_COUNT, 1, QModelIndex());
        setData(index(MAX_DESKTOP_COUNT, 0), Desktop::DesktopType::NewDesktop, Role::ItemType);
        setData(index(MAX_DESKTOP_COUNT, 0), "", Role::CustomName);
        setData(index(MAX_DESKTOP_COUNT, 0), "", Role::Uuid);
    }
    removeRows(row, 1, QModelIndex());
    Q_EMIT desktopRemoved(desktop);
    m_desktopManager->saveDesktopNames(getDesktopNames());
#ifdef KWINEFFECT
    for (int i = row; i < m_desktopList.count(); ++i) {
        setData(index(i, 0), QString::number(i + 1), Role::Uuid);
    }
#endif
}

void DesktopListModel::onDesktopMoved(const QString &desktop, int destDesktopIndex)
{
    int row = getModelIndexByUuid(desktop);
    moveRows(QModelIndex(), row, 1, QModelIndex(), destDesktopIndex - 1);
    m_desktopManager->saveDesktopNames(getDesktopNames());
#ifdef KWINEFFECT
    setData(index(row, 0), QString::number(row + 1), Role::Uuid);
    setData(index(destDesktopIndex - 1, 0), QString::number(destDesktopIndex), Role::Uuid);
#endif
}
