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

#include "ktablemodel.h"
#include "themeController.h"
#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QStyleOptionButton>

namespace kdk
{

class Q_DECL_HIDDEN KTableModelPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTableModel)
public:
    KTableModelPrivate(KTableModel *parent);

    void changeTheme();

private:
    KTableModel *q_ptr;
};

KTableModel::KTableModel(QObject *parent)
    : QStandardItemModel(parent)
    , d_ptr(new KTableModelPrivate(this))
{
}

QVariant KTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    (void)section;

    if (orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    }

    return QVariant();
}

KTableModelPrivate::KTableModelPrivate(KTableModel *parent)
    : q_ptr(parent)
{
    setParent(parent);
}

void KTableModelPrivate::changeTheme()
{
    Q_Q(KTableModel);
    initThemeStyle();
}

}

#include "ktablemodel.moc"
#include "moc_ktablemodel.cpp"
