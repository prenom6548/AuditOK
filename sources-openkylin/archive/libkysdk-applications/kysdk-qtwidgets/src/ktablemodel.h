/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KTABLEMODEL_H
#define KTABLEMODEL_H

#include <QStandardItemModel>
namespace kdk
{

/**
 * @brief 重置标题
 * @since 2.5.2.0
 */
class KTableModelPrivate;

class KTableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit KTableModel(QObject *parent = nullptr);

protected:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    Q_DECLARE_PRIVATE(KTableModel)
    KTableModelPrivate *const d_ptr;
};
}
#endif // KTABLEMODEL_H
