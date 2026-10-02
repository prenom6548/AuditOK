/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "control-panel-model.h"
#include "security-function-control.h"

#include <QDebug>
#include <mutex>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

static std::once_flag onceFlag;
static ControlPanelModel* g_instance = nullptr;


ControlPanelModel* ControlPanelModel::self()
{
    std::call_once(onceFlag, [ & ] {
        g_instance = new ControlPanelModel();
    });
    return g_instance;
}

bool ControlPanelModel::enable() const
{
    return m_enable;
}

void ControlPanelModel::setEnable(bool enable)
{
    if (m_enable == enable) return;
    m_enable = enable;
    Q_EMIT enableChanged();
}

void ControlPanelModel::activate(int modelIndex)
{
    if (SecurityFunctionControl::instance()->canLeftClick("ukui-sidebar")) {
        Q_UNUSED(modelIndex);
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.ukui.Sidebar"),
                                                                  QStringLiteral("/org/ukui/Sidebar"),
                                                                  QStringLiteral("org.ukui.Sidebar"),
                                                                  QStringLiteral("shortcutsActive"));
        QDBusConnection::sessionBus().send(message);
    }
}

void ControlPanelModel::showContextMenu(const QModelIndex &index, QQuickItem *item)
{
    if (SecurityFunctionControl::instance()->canRightClick("ukui-sidebar")) {
        m_sourceModel->showContextMenu(mapToSource(index), item);
    }
}

bool ControlPanelModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    return m_sourceModel->getControlPanelItems().contains(sourceIndex.data(TrayItemsModel::Id).toString());
}

bool ControlPanelModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return m_sourceModel->getControlPanelItems().indexOf(source_left.data(TrayItemsModel::Id).toString())
            < m_sourceModel->getControlPanelItems().indexOf(source_right.data(TrayItemsModel::Id).toString());
}

ControlPanelModel::ControlPanelModel(QObject* parent)
{
    QSortFilterProxyModel::setSourceModel(TrayItemsModel::instance());
    m_sourceModel = TrayItemsModel::instance();

    connect(this, &QAbstractItemModel::rowsInserted, this, &ControlPanelModel::rowCountChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &ControlPanelModel::rowCountChanged);
    sort(0);
}
