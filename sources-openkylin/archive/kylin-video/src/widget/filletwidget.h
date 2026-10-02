/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef FILLETWIDGET_H
#define FILLETWIDGET_H

#include <QWidget>

class FilletWidget : public QWidget
{
    Q_OBJECT
    friend class FilletWidgetTest;
public:
    explicit FilletWidget(QWidget *parent = nullptr);
    void setColor(QColor c){m_color = c;}
    void setRadius(int radius){m_radius = radius;repaint();}

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor m_color;
    int m_radius;
};
#endif // FILLETWIDGET_H
