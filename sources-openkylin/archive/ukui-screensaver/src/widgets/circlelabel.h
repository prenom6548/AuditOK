/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef CIRCLELABEL_H
#define CIRCLELABEL_H

#include <QPixmap>
#include <QLabel>

class CircleLabel : public QLabel
{
    Q_OBJECT
public:
    CircleLabel(QWidget *parent = nullptr);
    CircleLabel(QPixmap pixmap, QWidget *parent = nullptr);
    ~CircleLabel();

    void setPixmap(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void rebuildCache();

    QPixmap mPixmap;
    QPixmap m_cachedPixmap;
    QSize m_cacheSize;
    qreal m_cacheDevicePixelRatio = 1.0;
};

#endif // CIRCLELABEL_H
