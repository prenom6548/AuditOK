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
#ifndef PLAYGLWIDGET_H
#define PLAYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>


#include "eventpasswidget.h"
#include "core/mpvtypes.h"
#include "core/mpvlibload.h"
#include "core/qthelper.h"


class PlayGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    PlayGLWidget(QWidget *parent = 0, double s = 1.0);
    ~PlayGLWidget();
    void setMpvHandle(mpv::qt::Handle mh){mpv_h = mh;initMpvGL();}

    void drawMpv();

private slots:
    void swapped();

private:
    mpv::qt::Handle mpv_h;
    mpv_render_context *m_render_context;

    double m_screenScale;

    static void on_update(void *ctx);
    void initMpvGL();

protected:
    void initializeGL() override;
    void paintGL() override;

public:
    void setMouseUsed(bool used);

signals:
    void mousePressed();

private slots:
    void playStateChange();
    void videIdChange(int vid);

private:
    void initGlobalSig();

    bool m_mouseUsed,
         m_isMouseEnter,
         m_dbClicked,
         m_hasVideo,
         m_isMousePressed = false;

    int m_volumeChange;

    QPoint m_posStart;
    QPoint m_posLast;
    QPoint m_posCurrent;
    QPoint m_mousePosPressed;

    QTimer  *m_checkMouseTimer,
            *m_lMouseClickTimer;

    qint64 m_timePressStart;

protected:
#ifndef WHITE_TEST
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
#endif
};

#endif // PLAYGLWIDGET_H
