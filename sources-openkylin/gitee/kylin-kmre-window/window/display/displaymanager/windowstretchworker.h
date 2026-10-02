/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QWidget>
#include <QMainWindow>

class KmreWindow;

class WindowStretchWorker : public QObject
{
    Q_OBJECT

    enum CornerEdge
    {
        CE_Unknow = -1,
        CE_Top = 0,
        CE_TopLeft,
        CE_TopRight,
        CE_Left,
        CE_Right,
        CE_Bottom,
        CE_BottomLeft,
        CE_BottomRight,
    };

public:
    WindowStretchWorker(KmreWindow *window);
    ~WindowStretchWorker();
    
    void init();
    void setMouseDirection(QWidget *w, const QPoint &pos);
    void resizeWindowSize(QPoint point, bool updateDisplay);
    bool isWindowStretchReady(){return m_windowStretchReady;}
    void updateGeoAndRatio(const QSize& minSz);

public slots:
    
    void setEnabled(bool b);

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    bool updateOldSizeAndAspectRatio();

private:
    KmreWindow *mMainWindow = nullptr;
    bool m_enableResizing = false;
    bool m_windowSizeChanged = false;
    QSize m_oldSize;
    QSize m_minSize;
    QPoint m_oldPoint;
    QPoint m_oldPos;
    CornerEdge m_cornerEdge = CE_Unknow;
    qreal m_aspectRatio = 1.0;
    bool m_enabled;
    bool m_windowStretchReady;
};
