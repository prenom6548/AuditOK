/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "sidebardelegate.h"
#include "sizedate.h"
#include <QPainterPath>
#include <QDebug>
SideBarDelegate::SideBarDelegate()
{
    this->setObjectName("Kylin-photo-viewer_QStyledItemDelegate_sideBar");
}

QSize SideBarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_UNUSED(option);
    return QSize(94 + 6, 56 + 6);
}

void SideBarDelegate::paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const
{
    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y());
    rect.setWidth(option.rect.width());
    rect.setHeight(option.rect.height());
	
    //画圆角矩形
    const qreal radius = 4;
    QPainterPath path;
    path.moveTo(rect.topRight() - QPointF(radius, 0));
    path.lineTo(rect.topLeft() + QPointF(radius, 0));
    path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
    path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
    path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
    path.lineTo(rect.bottomRight() - QPointF(radius, 0));
    path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
    path.lineTo(rect.topRight() + QPointF(0, radius));
    path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));
    painter->setRenderHints(QPainter::SmoothPixmapTransform);

    QGSettings *m_pGsettingThemeData;
    if (QGSettings::isSchemaInstalled(Variable::FITTHEMEWINDOW.toLocal8Bit())) {
        m_pGsettingThemeData = new QGSettings(Variable::FITTHEMEWINDOW.toLocal8Bit());
    }
    QString themeStyle = m_pGsettingThemeData->get("styleName").toString();
    if ("ukui-dark" == themeStyle || "ukui-black" == themeStyle) {
        if (option.state & QStyle::State_Selected) {
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(Qt::black);
            painter->setOpacity(0.4);
            painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), radius, radius);
            painter->restore();
            painter->save();
            QColor highColor = option.palette.color(QPalette::Highlight);
            painter->setPen(QPen(highColor, 2));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), radius, radius);
            painter->restore();
        }else if (option.state & QStyle::State_MouseOver) {
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(Qt::black);
            painter->setOpacity(0.4);
            painter->drawPath(path);
            painter->restore();
        }else{
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(Qt::black);
            painter->setOpacity(0.4);
            painter->drawPath(path);
            painter->restore();
        }
    }else{
        if (option.state & QStyle::State_Selected) {
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(Qt::white);
            painter->setOpacity(0.9);
            painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), radius, radius);
            painter->restore();
            painter->save();
            QColor highColor = option.palette.color(QPalette::Highlight);
            painter->setPen(QPen(highColor, 2));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(rect.adjusted(2, 2, -2, -2), radius, radius);
            painter->restore();
        }else if (option.state & QStyle::State_MouseOver) {
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(Qt::white);
            painter->setOpacity(0.9);
            painter->drawPath(path);
            painter->restore();
        }else{
            painter->save();
            painter->setPen(QPen(Qt::NoPen));
            painter->setBrush(Qt::white);
            painter->setOpacity(0.5);
            painter->drawPath(path);
            painter->restore();
        }
    }

    //绘制缩略图
    painter->save();
    painter->setOpacity(1);
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QRect iconRect;
    iconRect = QRect(rect.x()-3, rect.y()-1, SIDEITEM_SIZE.width(), SIDEITEM_SIZE.height());
    icon.paint(painter, iconRect);
}
