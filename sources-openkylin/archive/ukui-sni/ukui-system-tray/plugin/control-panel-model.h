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

#ifndef CONTROL_PANEL_MODEL_H
#define CONTROL_PANEL_MODEL_H

#include <QSortFilterProxyModel>
#include "tray-items-model.h"
#include "items-filter-model.h"

class QQuickItem;

class ControlPanelModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(bool enable READ enable NOTIFY enableChanged)
public:
    static ControlPanelModel* self();

    bool enable() const;
    void setEnable(bool enable);

    Q_INVOKABLE void activate(int modelIndex);
    Q_INVOKABLE void showContextMenu(const QModelIndex &index, QQuickItem *item);

Q_SIGNALS:
    void rowCountChanged();
    void enableChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    explicit ControlPanelModel(QObject* parent = nullptr);
    TrayItemsModel* m_sourceModel = nullptr;
    bool m_enable = false;
};



#endif //CONTROL_PANEL_MODEL_H
