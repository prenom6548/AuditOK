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
#ifndef SEARCHDELEGATE_H
#define SEARCHDELEGATE_H

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
#include "global/corevar.h"
class SearchDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class TestSearchDelegate;
public:
    SearchDelegate(QWidget *parent);

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const; //画item样式，包括悬停，选中等
    //    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index)
    //    const;//改变item大小，但未起作用--可保留，后续继续处理
    QMap<QModelIndex, QStyleOptionButton *> m_btns; //加号按钮
    QScopedPointer<QPushButton> m_pOpenButton;
    //此事件可对在item上的操作做出反应
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index);

signals:
    //    void addCollectCity(QString cityName,QString cityId);
    //    void changeCityFromSearch(QString cityId);
    void addCollectCity(QModelIndex m_index);       //添加收藏城市
    void changeCityFromSearch(QModelIndex m_index); //从搜索列表改变当前城市
};

#endif // SEARCHITEMDELEGATE_H
