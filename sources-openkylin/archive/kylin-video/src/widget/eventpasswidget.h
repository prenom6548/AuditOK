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
#ifndef EVENTPASSWIDGET_H
#define EVENTPASSWIDGET_H

#include <QWidget>

#include "core/mpvtypes.h"

class EventPassWidget : public QWidget
{
    Q_OBJECT

    friend class unitConstruct;
public:
    explicit EventPassWidget(QWidget *parent = nullptr);
    ~EventPassWidget();

    void setMouseUsed(bool used);

signals:
    void mousePressed();

private slots:
    void playStateChange();
    void videIdChange(int vid);

private:
    void initGlobalSig();
    void onMove();

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
            *m_mouseTrackTimer,
            *m_lMouseClickTimer;

    qint64 m_timePressStart;

protected:
    void mousePressEvent(QMouseEvent *e) override;
#ifndef WHITE_TEST
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
#endif
    bool event(QEvent *e) override;
};

#endif // EVENTPASSWIDGET_H
