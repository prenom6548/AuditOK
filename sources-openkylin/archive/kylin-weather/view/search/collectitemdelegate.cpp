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
#include "collectitemdelegate.h"
#include "view/horscreen/picturetowhite.h"

CollectItemDelegate::CollectItemDelegate(QWidget *parent) : QStyledItemDelegate(parent) 
{
 
}

QSize CollectItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(190, 48);
}

void CollectItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        QVariant variant = index.data(Qt::UserRole);
        CollectItemData data = variant.value<CollectItemData>();
        painter->save();

        //画笔设置透明（边框）
        painter->setPen(Qt::NoPen);
        // 反锯齿
        painter->setRenderHint(QPainter::Antialiasing);

        //画圆角
        QRectF rect;
        rect.setX(option.rect.x());                    // x
        rect.setY(option.rect.y());                    // y
        rect.setWidth(option.rect.width());            // w
        rect.setHeight(option.rect.height());          // y
        const qreal radius = ViewVar::SAME_ITEMRADIUS; // radius
        
        painter->restore();

        /* 【深色模式】常规：不透明度10%； Hover：不透明度18%；  Click: 不透明度24%；
           【浅色模式】常规：不透明度5%； Hover：不透明度110%；  Click: 不透明度15%； */

        painter->save();
        if (option.state.testFlag(QStyle::State_HasFocus)) {
            if (ViewVar::s_themeStyle) {
                painter->setBrush(QBrush(QColor("#000000")));
                painter->setOpacity(0.15);
            } else {
                painter->setBrush(QBrush(QColor("#ffffff")));
                painter->setOpacity(0.24);
            }
            if (ViewVar::m_editStyle) {
                if (data.cityName == ViewVar::COLLENTTEXT) {
                    if (ViewVar::s_themeStyle) {
                        painter->setBrush(QBrush(QColor("#000000")));
                        painter->setOpacity(0);
                    } else {
                        painter->setBrush(QBrush(QColor("#ffffff")));
                        painter->setOpacity(0);
                    }
                }
            }
        } else if (option.state & QStyle::State_MouseOver) {
            if (ViewVar::s_themeStyle) {
                painter->setBrush(QBrush(QColor("#000000")));
                painter->setOpacity(0.10);
            } else {
                painter->setBrush(QBrush(QColor("#ffffff")));
                painter->setOpacity(0.18);
            }
            if (ViewVar::m_editStyle) {
                if (data.cityName == ViewVar::COLLENTTEXT) {
                    if (ViewVar::s_themeStyle) {
                        painter->setBrush(QBrush(QColor("#000000")));
                        painter->setOpacity(0);
                    } else {
                        painter->setBrush(QBrush(QColor("#ffffff")));
                        painter->setOpacity(0);
                    }
                }
            }
        } else {
            if (ViewVar::s_themeStyle) {
                painter->setBrush(QBrush(QColor("#000000")));
                painter->setOpacity(0.05);
            } else {
                painter->setBrush(QBrush(QColor("#ffffff")));
                painter->setOpacity(0.1);
            }
            if (ViewVar::m_editStyle) {
                if (data.cityName == ViewVar::COLLENTTEXT) {
                    if (ViewVar::s_themeStyle) {
                        painter->setBrush(QBrush(QColor("#000000")));
                        painter->setOpacity(0);
                    } else {
                        painter->setBrush(QBrush(QColor("#ffffff")));
                        painter->setOpacity(0);
                    }
                }
            }
        }
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect, ViewVar::SAME_ITEMRADIUS,ViewVar::SAME_ITEMRADIUS);

        painter->restore();

        QColor highLightColor = QApplication::palette().highlight().color();
        QColor buttonColor = QApplication::palette().button().color();
        QString delIconLight = ":/res/contorl_icons/close_button_light.svg";
        QString delIconDark = ":/res/contorl_icons/close_button_dark.svg";

        // 编辑状态样式
        if (ViewVar::m_editStyle) {
            QStyleOptionButton *delButton = m_btns.value(index);
            delButton = new QStyleOptionButton();
            delButton->state |= QStyle::State_Enabled;
            delButton->icon = QIcon(delIconLight);
            delButton->iconSize = ViewVar::COLLECT_BTNSIZE;
            (const_cast<CollectItemDelegate *>(this))->m_btns.insert(index, delButton);

            if (data.cityName == ViewVar::COLLENTTEXT)  {
                
            } else if (CoreVar::LOCATION_CITY_ID == data.cityId && index.row() == 0)  {
                painter->setPen(ViewVar::m_collectPen);
                painter->setOpacity(ViewVar::SAME_OPACITY_2);
                QString cityname = data.cityName.mid(0, 3);
                if (data.cityName != cityname) {
                    cityname = cityname + "...";
                }
                
                QPixmap pixmap ;
                PictureToWhite picToWhite;
                pixmap = picToWhite.drawSymbolicColoredPixmap(QPixmap(QIcon(":/res/image/ukui-icon-Location/select-symbolic.png").pixmap(QSize(23, 23))));
                
                pixmap = pixmap.scaled(QSize(23, 23), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                painter->drawPixmap(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 20) / 2, pixmap);

                painter->drawText(option.rect.adjusted(36, 0, -4, -4), Qt::AlignVCenter | Qt::AlignLeft, cityname);
                painter->drawText(option.rect.adjusted(140, 0, -4, -4), Qt::AlignVCenter | Qt::AlignLeft, data.tmp);
                //天气图标
                if (data.cond_code == "") {
                    painter->drawText(option.rect.adjusted(ViewVar::COLLECT_CODEXL1, ViewVar::COLLECT_SAMEYL + 3,
                    ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),Qt::AlignVCenter | Qt::AlignLeft, data.cond_code);
                } else {
                    QString iconPath = ViewVar::COLLECT_ICONPATH + data.cond_code + ViewVar::COLLECT_ICONSUFFIX;
                    QPixmap pixmap(iconPath);
                    pixmap = pixmap.scaled(QSize(16, 16), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    painter->drawPixmap(option.rect.x() + 120, option.rect.y() + (option.rect.height() - 16) / 2, pixmap);
                }
            } else if (!(data.cityName == "" || data.tmp == "" || data.cond_code == "")) {
                painter->setPen(ViewVar::m_collectPen);
                painter->setOpacity(ViewVar::SAME_OPACITY_2);
                QString cityname = data.cityName.mid(0, 3);
                if (data.cityName != cityname) {
                    cityname = cityname + "...";
                }
                painter->drawText(option.rect.adjusted(ViewVar::COLLECT_NAMEXL, 0, 
                                                       ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),
                                                       Qt::AlignVCenter | Qt::AlignLeft, cityname); //绘制城市名字
                painter->drawText(option.rect.adjusted(ViewVar::COLLECT_TMPXL, ViewVar::COLLECT_SAMEYL,
                                                       ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),
                                                       Qt::AlignVCenter | Qt::AlignLeft, data.tmp);
                //天气图标
                if (data.cond_code == "") {
                    painter->drawText(option.rect.adjusted(ViewVar::COLLECT_CODEXL, ViewVar::COLLECT_SAMEYL + 3,
                                                           ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),
                                                           Qt::AlignVCenter | Qt::AlignLeft, data.cond_code);
                } else {
                    QString iconPath = ViewVar::COLLECT_ICONPATH + data.cond_code + ViewVar::COLLECT_ICONSUFFIX;
                    QPixmap pixmap(iconPath);
                    pixmap = pixmap.scaled(QSize(16, 16), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

                    painter->drawPixmap(option.rect.x() + ViewVar::COLLECT_ICONXL,
                                        option.rect.y() + (option.rect.height() - ViewVar::COLLECT_ICONYL) / 2, pixmap);
                }
                painter->setPen(Qt::NoPen); //去边框                                
                //设置背景颜色
                if (ViewVar::s_themeStyle) {
                    ViewVar::m_collectPen = QPen(Qt::black);
                    delButton->icon = QIcon(delIconLight);
                } else {
                    ViewVar::m_collectPen = QPen(Qt::white);
                    delButton->icon = QIcon(delIconDark);
                }
                QPalette btnPal = delButton->palette;
                btnPal.setColor(QPalette::Button, Qt::transparent); //设置按钮的背景颜色
                delButton->iconSize = QSize(30, 30);
                delButton->palette = btnPal;
                delButton->rect = option.rect.adjusted(ViewVar::COLLECT_BTNXL, ViewVar::COLLECT_BTNYL, ViewVar::COLLECT_BTNXR, ViewVar::COLLECT_BTNYR); // button位置
                
                // 替换QApplication::style()->drawControl()以避免wayland环境下的崩溃
                // 手动绘制按钮
                drawCustomButton(delButton, painter);
            }
        } else {
            if (data.cityName == ViewVar::COLLENTTEXT)  {
            painter->save();

            QRect addCityRect;
            QSize addCitySize(30, 30);
            QPoint addCityPoint(rect.left() + 75, rect.top() + 7);
            addCityRect = QRect(addCityPoint, addCitySize);
            QPixmap addCityPixmap;
            PictureToWhite picToWhite;
            addCityPixmap = picToWhite.drawSymbolicColoredPixmap(QPixmap(QIcon::fromTheme("list-add-symbolic").pixmap(addCityRect.size())));
            QFont font = QFont();
            font.setPointSizeF(25);
            painter->setFont(font);
            painter->drawPixmap(addCityRect, addCityPixmap);
            painter->restore();

            } else if (CoreVar::LOCATION_CITY_ID == data.cityId && index.row() == 0)  {
                painter->setPen(ViewVar::m_collectPen);
                painter->setOpacity(ViewVar::SAME_OPACITY_2);
                QString cityname = data.cityName.mid(0, 3);
                if (data.cityName != cityname) {
                    cityname = cityname + "...";
                }

                // 定位图标
                QPixmap pixmap;
                PictureToWhite picToWhite;
                pixmap = picToWhite.drawSymbolicColoredPixmap(QPixmap(QIcon(":/res/image/ukui-icon-Location/select-symbolic.png").pixmap(QSize(23, 23))));
                pixmap = pixmap.scaled(QSize(23, 23), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                painter->drawPixmap(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 20) / 2, pixmap);

                painter->drawText(option.rect.adjusted(36, 0, -4, -4), Qt::AlignVCenter | Qt::AlignLeft, cityname);
                painter->drawText(option.rect.adjusted(140, 0, -4, -4), Qt::AlignVCenter | Qt::AlignLeft, data.tmp);
                //天气图标
                if (data.cond_code == "") {
                    painter->drawText(option.rect.adjusted(ViewVar::COLLECT_CODEXL1, ViewVar::COLLECT_SAMEYL + 3,
                    ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),Qt::AlignVCenter | Qt::AlignLeft, data.cond_code);
                } else {
                    QString iconPath = ViewVar::COLLECT_ICONPATH + data.cond_code + ViewVar::COLLECT_ICONSUFFIX;
                    QPixmap pixmap(iconPath);
                    pixmap = pixmap.scaled(QSize(16, 16), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    painter->drawPixmap(option.rect.x() + 120, option.rect.y() + (option.rect.height() - 16) / 2, pixmap);
                }
            } else if (!(data.cityName == "" || data.tmp == "" || data.cond_code == "")) {
                painter->setPen(ViewVar::m_collectPen);
                painter->setOpacity(ViewVar::SAME_OPACITY_2);
                QString cityname = data.cityName.mid(0, 3);
                if (data.cityName != cityname) {
                    cityname = cityname + "...";
                }
            // painter->drawText(option.rect.adjusted(ViewVar::COLLECT_NAMEXL, 0, 
            //                                        ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR), 
            //                                        Qt::AlignVCenter | Qt::AlignLeft, cityname); //绘制城市名字
                painter->drawText(option.rect.adjusted(16, 0, -4, -4), Qt::AlignVCenter | Qt::AlignLeft, cityname);
            // painter->drawText(option.rect.adjusted(ViewVar::COLLECT_TMPXL1, ViewVar::COLLECT_SAMEYL,
            //                                        ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),
            //                                        Qt::AlignVCenter | Qt::AlignLeft, data.tmp);
                painter->drawText(option.rect.adjusted(140, 0, -4, -4), Qt::AlignVCenter | Qt::AlignLeft, data.tmp);
                //天气图标
                if (data.cond_code == "") {
                     painter->drawText(option.rect.adjusted(ViewVar::COLLECT_CODEXL1, ViewVar::COLLECT_SAMEYL + 3,
                     ViewVar::COLLECT_SAMEXR, ViewVar::COLLECT_SAMEYR),Qt::AlignVCenter | Qt::AlignLeft, data.cond_code);
                } else {
                    QString iconPath = ViewVar::COLLECT_ICONPATH + data.cond_code + ViewVar::COLLECT_ICONSUFFIX;
                    QPixmap pixmap(iconPath);
                    pixmap = pixmap.scaled(QSize(16, 16), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                // painter->drawPixmap(option.rect.x() + ViewVar::COLLECT_ICONXL,
                                    // option.rect.y() + (option.rect.height() - ViewVar::COLLECT_ICONYL) / 2, pixmap);
                // painter->drawPixmap(0 + 93, [y + (h - 16)] / 2
                    painter->drawPixmap(option.rect.x() + 120, option.rect.y() + (option.rect.height() - 16) / 2, pixmap);
                }
            }  
        }
                                                        
    }
}

bool CollectItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
    //判断是自动画出的，没有数据的item
    if (model->data(index, Qt::UserRole).isNull()) {
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *e = (QMouseEvent *)event;
        if (ViewVar::m_editStyle) {
            QStyleOptionButton *btns = m_btns.value(index);
            if (btns->rect.contains(e->x(), e->y())) {
                btns->state &= (~QStyle::State_Sunken);
                // if (index.row() != 0 && index.column() != 0) {
                    emit delCollectCity(index);
                // }
                
            } 
        }else {
            QVariant variant = index.data(Qt::UserRole);
            CollectItemData data = variant.value<CollectItemData>();
            if (data.cityName == ViewVar::COLLENTTEXT) {
                emit sigAddCityFromCollect();
            } else{
                emit changeCityFromCollect(index);
            }
            
        }
    }
    Q_UNUSED(option);
    return true;
}

QPixmap CollectItemDelegate::changeHighLightColor(QColor color, QString path) const
{
    QImage image(path);

    QColor rgb(color);
    QColor trans(qRgba(0,0,0,1));
    int whiteColorKey = 255 - 50;
    for (int j = 0; j < image.height(); ++j) {
        for (int i = 0; i < image.width(); ++i) {
            QColor color(image.pixel(i, j));
            //不处理透明像素点,不处理白色
            if (color == trans || (color.red() > whiteColorKey 
                                  && color.green()>whiteColorKey 
                                  && color.blue() > whiteColorKey)) {
                continue;
            }
            image.setPixel(i, j, rgb.rgb());
        }
    }
    return QPixmap::fromImage(image);
}

void CollectItemDelegate::drawCustomButton(QStyleOptionButton *button, QPainter *painter) const
{
    if (!button || !painter) {
        return;
    }
    
    painter->save();
    
    // 设置透明背景（与原代码保持一致）
    painter->setBrush(Qt::transparent);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setPen(Qt::NoPen);
    
    // 如果需要绘制按钮背景（当前使用透明背景，可根据需要修改）
    // painter->drawRoundedRect(button->rect, 4, 4);
    
    // 绘制图标
    if (!button->icon.isNull()) {
        QRect iconRect = button->rect;
        QSize iconSize = button->iconSize;
        
        // 确保图标大小合理
        if (iconSize.isEmpty()) {
            iconSize = QSize(16, 16); // 默认图标大小
        }
        
        // 计算图标居中位置
        int x = iconRect.x() + (iconRect.width() - iconSize.width()) / 2;
        int y = iconRect.y() + (iconRect.height() - iconSize.height()) / 2;
        QRect centeredIconRect(x, y, iconSize.width(), iconSize.height());
        
        // 获取图标模式（根据按钮状态）
        QIcon::Mode iconMode = QIcon::Normal;
        if (!(button->state & QStyle::State_Enabled)) {
            iconMode = QIcon::Disabled;
        } else if (button->state & QStyle::State_Selected) {
            iconMode = QIcon::Selected;
        }
        
        // 获取图标状态
        QIcon::State iconState = (button->state & QStyle::State_On) ? QIcon::On : QIcon::Off;
        
        // 绘制图标
        QPixmap pixmap = button->icon.pixmap(iconSize, iconMode, iconState);
        painter->drawPixmap(centeredIconRect, pixmap);
    }
    
    painter->restore();
}
