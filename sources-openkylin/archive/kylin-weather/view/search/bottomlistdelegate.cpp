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
#include "bottomlistdelegate.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QEvent>
#include <QDebug>
#include "model/titlesearchmodel.h"

BottomListDelegate::BottomListDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

BottomListDelegate::~BottomListDelegate()
{

}

QSize BottomListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(30, 25);
}

void BottomListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
        painter->save();

        // QVariant variant = index.data(Qt::UserRole);
        // ItemData data = variant.value<ItemData>();
        int type = index.data(TitleSearchModel::CityStyle).toInt();
        QString id = index.data(TitleSearchModel::CityId).toString();

        // 整体item 矩形区域
        QRectF itemRect;
        itemRect.setX(option.rect.x());
        itemRect.setY(option.rect.y());
        itemRect.setWidth(option.rect.width() - 1);
        itemRect.setHeight(option.rect.height() - 1);

        QPainterPath path;
        // 鼠标悬停或者选中时改变背景色
        if (option.state.testFlag(QStyle::State_MouseOver)) {
            // path.setFillRule(Qt::WindingFill);
            // path.addRoundedRect(itemRect, 0, 0);

        } else if (option.state.testFlag(QStyle::State_HasFocus)) {
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(itemRect, 0, 0);

        }

        // 标识
        QSize iconSize(25, 25);
        QPoint iconPoint(itemRect.left() , itemRect.top());
        QRect iconRect = QRect(iconPoint, iconSize);
        QPixmap iconPixmap;
        
        // 绘制图片
        if (type == TitleSearchModel::CityType::Location) {
            if (index.data(TitleSearchModel::CityId).toString() != "") {
                if (CoreVar::CURRENT_CITY_ID != id || CoreVar::GET_CITY_INFO != true || CoreVar::CURRENT_CITY_TYPE != CityType::Location) {
                    iconPixmap = QPixmap(QIcon(":/res/image/ukui-icon-Location/normal-symbolic.png").pixmap(iconRect.size()));
                } else {
                    iconPixmap = QPixmap(QIcon(":/res/image/ukui-icon-Location/select-symbolic.png").pixmap(iconRect.size()));
                }
                painter->drawPixmap(iconRect, iconPixmap);

            }
        } else if (type == TitleSearchModel::CityType::Collection) {
            if (index.data(TitleSearchModel::CityId).toString() != "") {
                if (CoreVar::CURRENT_CITY_ID != id || CoreVar::GET_CITY_INFO != true || CoreVar::CURRENT_CITY_TYPE != CityType::Collection) {
                    iconPixmap = QPixmap(QIcon(":/res/image/ukui-icon-point/normal-symbolic.png").pixmap(iconRect.size()));
                } else {
                    iconPixmap = QPixmap(QIcon(":/res/image/ukui-icon-point/select-symbolic.png").pixmap(iconRect.size()));
                }
            }
            painter->drawPixmap(iconRect, iconPixmap);
        } 

        painter->restore();
    }
}
