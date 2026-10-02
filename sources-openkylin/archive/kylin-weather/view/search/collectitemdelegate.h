/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#ifndef COLLECTITEMDELEGATE_H
#define COLLECTITEMDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QItemDelegate>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QEvent>
#include <QDebug>
#include <QStandardItemModel>

#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>
#include <QScopedPointer>
#include <QPushButton>
#include <QModelIndex>
#include "global/viewvar.h"
#include "controller/Interaction.h"

class CollectItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class TestCollectItemDelegate;
public:
    CollectItemDelegate(QWidget *parent);

private:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QMap<QModelIndex, QStyleOptionButton *> m_btns;
    QScopedPointer<QPushButton> m_pOpenButton;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index);
    bool m_btnEnable = true;
    QPixmap changeHighLightColor(QColor color, QString path) const;
    void drawCustomButton(QStyleOptionButton *button, QPainter *painter) const;
signals:
    void delCollectCity(QModelIndex index);        //删除收藏城市
    void changeCityFromCollect(QModelIndex index); //从收藏城市切换主界面城市
    void sigAddCityFromCollect(); //切换添加收藏城市界面
};

#endif // COLLECTITEMDELEGATE_H
