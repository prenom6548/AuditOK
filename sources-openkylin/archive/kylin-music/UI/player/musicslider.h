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

#ifndef MUSICSLIDER_H
#define MUSICSLIDER_H

#include <QObject>
#include <QSlider>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>

class MusicSlider : public QSlider
{
    Q_OBJECT

public:
    MusicSlider(QWidget *parent = nullptr);
    void isPlaying(bool isPlaying);
    void initStyle();
    void changeStyleColor();
    void setPosition(int pos);
#ifndef WHITE_TEST
    int calcValueFromMouseX(int mouseX);
#endif
private:
    bool m_canSetPosition { true };

protected:
#ifndef WHITE_TEST
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
#endif

private:
    bool m_isPlaying;
};

#endif // MUSICSLIDER_H
