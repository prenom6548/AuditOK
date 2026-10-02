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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PROMPTWIDGET_H
#define PROMPTWIDGET_H

#include <QWidget>
#include <QX11Info>
#include <QHBoxLayout>
#include <QLabel>

class PromptWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PromptWidget(QWidget *parent = nullptr);
    void initUi();
    void initColor();
    void setColor(QColor c){m_color = c;}
    void setRadius(int radius){m_radius = radius; repaint();}

protected:
#ifndef WHITE_TEST
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
#endif
private:
    int volunmPosX;
    int volunmPosY;
    int volunmPosWidth;
    int volunmPosHeight;
    int m_radius;
    QColor m_color;
    QHBoxLayout *HLayout = nullptr;
    QLabel *iconLabel = nullptr;
    QLabel *promptLabel = nullptr;
};

#endif // PROMPTWIDGET_H
