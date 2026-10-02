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
#include "titlesearchlistdelegate.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include "model/titlesearchmodel.h"

TitleSearchListDelegate::TitleSearchListDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

TitleSearchListDelegate::~TitleSearchListDelegate()
{

}

QSize TitleSearchListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(292, 32);
}

void TitleSearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid())
    {
        QString cityName = index.data(TitleSearchModel::CityName).toString();

        painter->save();

        // 修复：使用clipRect限制绘制区域，确保绘制不超出option.rect范围
        painter->setClipRect(option.rect, Qt::IntersectClip);
        
        // 修复：先清除背景区域，避免滚动时出现残留线条
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->eraseRect(option.rect);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        if ((option.state & QStyle::State_MouseOver) || (option.state & QStyle::State_Selected)) {
            ViewVar::m_collectPen = QPen(Qt::white);
            painter->setPen(Qt::NoPen); //去边框
            //设置背景颜色
            painter->setRenderHint(QPainter::Antialiasing);
            QColor color = qApp->palette().highlight().color();
            QBrush brush = QBrush(color);
            painter->setBrush(brush); //设置画刷形式
            // 使用drawRoundedRect绘制圆角矩形
            painter->drawRoundedRect(option.rect, ViewVar::SAME_ITEMRADIUS, ViewVar::SAME_ITEMRADIUS);
        } else {
            if (ViewVar::s_themeStyle) {
                ViewVar::m_collectPen = QPen(Qt::black);
            } else {
                ViewVar::m_collectPen = QPen(Qt::white);
            }
        }
        if (!(cityName == "")) {
            painter->setPen(ViewVar::m_collectPen);
            painter->setOpacity(ViewVar::SAME_OPACITY_2);
            painter->drawText(option.rect.adjusted(ViewVar::SEARCH_NAMEXL, ViewVar::SEARCH_NAMEYL,
                                                   ViewVar::SEARCH_NAMEXR, ViewVar::SEARCH_NAMEYR),
                              Qt::AlignVCenter | Qt::AlignLeft, cityName); //绘制城市名字
        }
        painter->setPen(ViewVar::m_collectPen);
        painter->restore();

    }
}

bool TitleSearchListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) 
{
    //判断是自动画出的，没有数据的item
    if (model->data(index, Qt::UserRole).isNull()) {
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        emit changeCityFromSearch(index);
    }
    Q_UNUSED(option);
    Q_UNUSED(index);
    return true;

}
