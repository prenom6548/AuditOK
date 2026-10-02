/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_SIDEBAR_IMAGE_ITEM_H
#define UKUI_SIDEBAR_IMAGE_ITEM_H

#include <QQuickPaintedItem>
#include <QImage>

class ImageItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
public:
    explicit ImageItem(QQuickItem *parent = nullptr);

    QImage image() const;
    void setImage(QImage image);
    void paint(QPainter *painter) override;

Q_SIGNALS:
    void imageChanged();

private:
    QImage m_image;
};


#endif //UKUI_SIDEBAR_IMAGE_ITEM_H
