/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

#include <QSlider>
#include <QPainter>
#include <QStyleOptionSlider>
#include "colorsliderstyle.h"

class GradientSlider : public QSlider
{
    Q_OBJECT

    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QVector<QColor> colors READ colors WRITE setColors)
    Q_PROPERTY(QLinearGradient gradient READ gradient WRITE setGradient STORED false)
    Q_PROPERTY(QColor firstColor READ firstColor WRITE setFirstColor STORED false)
    Q_PROPERTY(QColor lastColor READ lastColor WRITE setLastColor STORED false)

public:
    explicit GradientSlider(QWidget *parent = 0);
    ~GradientSlider();

    QBrush background() const { return back; }
    void setBackground(QBrush bg);

    QVector<QColor> colors() const { return col_list; }
    void setColors(QVector<QColor> bg);
    void setGradient(QLinearGradient bg);
    QLinearGradient gradient() const ;

    void setFirstColor(QColor c);
    void setLastColor(QColor c);
    QColor firstColor() const;
    QColor lastColor() const;

protected:
    void paintEvent(QPaintEvent *ev);

private:
    QVector<QColor> col_list;
    QBrush back;
    ColorSliderStyle *style = nullptr;
};

#endif // GRADIENTSLIDER_H
