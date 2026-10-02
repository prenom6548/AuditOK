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
#include "searchdelegate.h"

SearchDelegate::SearchDelegate(QWidget *parent) : QStyledItemDelegate(parent), m_pOpenButton(new QPushButton()) {}

void SearchDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        QVariant variant = index.data(Qt::UserRole);
        ItemData data = variant.value<ItemData>();
        QStyleOptionButton *button = m_btns.value(index);
        button = new QStyleOptionButton();

        //        button->text = "+";
        button->state |= QStyle::State_Enabled;
        button->icon = QIcon(":/res/contorl_icons/add_normal.png");
        button->iconSize = ViewVar::COLLECT_BTNSIZE;
        (const_cast<SearchDelegate *>(this))->m_btns.insert(index, button);
        painter->save();
        //画圆角
        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());
        const qreal radius = ViewVar::SAME_ITEMRADIUS;
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

        if (option.state & QStyle::State_MouseOver) {
            if (ViewVar::s_themeStyle) {
                ViewVar::m_collectPen = QPen(Qt::black);
            } else {
                ViewVar::m_collectPen = QPen(Qt::white);
            }
            if (data.cityName == "") {
                painter->setPen(Qt::NoPen); //去边框
                //设置背景颜色
                painter->setRenderHint(QPainter::Antialiasing);
                painter->drawRoundRect(ViewVar::SEARCH_ITEMRECT);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setBrush(ViewVar::s_colorItem); //设置画刷形式
                painter->setOpacity(ViewVar::SAME_OPACITY_0);
                painter->drawPath(path);
            } else {
                painter->setPen(Qt::NoPen); //去边框
                //设置按钮样式
                QPalette btnPal = button->palette;
                btnPal.setColor(QPalette::Button, Qt::transparent); //设置按钮的背景颜色
                //                btnPal.setColor(QPalette::ButtonText, QColor(255, 255, 255, 0)); //设置按钮文字颜色
                button->palette = btnPal;
                button->rect = option.rect.adjusted(ViewVar::SEARCH_BTNXL, ViewVar::SEARCH_BTNYL, ViewVar::SEARCH_BTNXR,
                                                    ViewVar::SEARCH_BTNYR); // button位置
                //设置背景颜色
                painter->setRenderHint(QPainter::Antialiasing);
                painter->drawRoundRect(ViewVar::SEARCH_ITEMRECT);
                painter->setRenderHint(QPainter::Antialiasing);
                if (ViewVar::s_themeStyle) {
                    painter->setBrush(QBrush(QColor("#000000")));
                    painter->setOpacity(0.15);
                } else {
                    painter->setBrush(QBrush(QColor("#ffffff")));
                    painter->setOpacity(0.24);
                }
                QColor color = qApp->palette().highlight().color();
                QBrush brush = QBrush(color);
                // painter->setBrush(brush); //设置画刷形式
                painter->drawPath(path);
            }

        } else if (option.state & QStyle::State_Selected) {
            if (ViewVar::s_themeStyle) {
                ViewVar::m_collectPen = QPen(Qt::black);
            } else {
                ViewVar::m_collectPen = QPen(Qt::white);
            }
            if (data.cityName == "") {
                painter->setPen(Qt::NoPen); //去边框
                //设置背景颜色
                painter->setRenderHint(QPainter::Antialiasing);
                painter->drawRoundRect(ViewVar::SEARCH_ITEMRECT);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setBrush(QColor(120, 120, 120)); //设置画刷形式
                painter->setOpacity(0);
                painter->drawPath(path);
            } else {
                painter->setPen(Qt::NoPen); //去边框
                //设置按钮样式
                QPalette btnPal = button->palette;
                btnPal.setColor(QPalette::Button, Qt::transparent); //设置按钮的背景颜色
                //            btnPal.setColor(QPalette::ButtonText, QColor(255, 255, 255,0));//设置按钮文字颜色
                button->palette = btnPal;
                button->rect = option.rect.adjusted(ViewVar::SEARCH_BTNXL + 20, ViewVar::SEARCH_BTNYL,
                                                    ViewVar::SEARCH_BTNXR, ViewVar::SEARCH_BTNYR); // button位置
                //设置背景颜色
                painter->setRenderHint(QPainter::Antialiasing);
                painter->drawRoundRect(ViewVar::SEARCH_ITEMRECT);
                painter->setRenderHint(QPainter::Antialiasing);
                QColor color = qApp->palette().highlight().color();
                QBrush brush = QBrush(color);
                painter->setBrush(brush); //设置画刷形式
                painter->drawPath(path);
            }
        } else {
            if (ViewVar::s_themeStyle) {
                ViewVar::m_collectPen = QPen(Qt::black);
            } else {
                ViewVar::m_collectPen = QPen(Qt::white);
            }
        }
        if (!(data.cityName == "")) {
            painter->setPen(ViewVar::m_collectPen);
            painter->setOpacity(ViewVar::SAME_OPACITY_2);
            painter->drawText(option.rect.adjusted(ViewVar::SEARCH_NAMEXL, ViewVar::SEARCH_NAMEYL,
                                                   ViewVar::SEARCH_NAMEXR, ViewVar::SEARCH_NAMEYR),
                              Qt::AlignVCenter | Qt::AlignLeft, data.cityName); //绘制城市名字
        }
        painter->setPen(ViewVar::m_collectPen);
        painter->restore();
        //        QWidget *pWidget =  m_pOpenButton.data();----备用
//        QApplication::style()->drawControl(QStyle::CE_PushButton, button, painter);
    }
}

bool SearchDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    //判断是自动画出的，没有数据的item
    if (model->data(index, Qt::UserRole).isNull()) {
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = (QMouseEvent *)event;
        if (m_btns.contains(index)) {
            QStyleOptionButton *btns = m_btns.value(index);
            // if (btns->rect.contains(e->x(), e->y())) {
                // btns->state &= (~QStyle::State_Sunken); //必须要加这一句
                // emit addCollectCity(index);
            // } else {
                // emit changeCityFromSearch(index);
            // }
            emit addCollectCity(index);
        }
    }
    Q_UNUSED(option);
    Q_UNUSED(index);
    return true;
}
