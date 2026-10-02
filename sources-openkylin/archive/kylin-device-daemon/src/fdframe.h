/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 * 
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef __FDFRAME_H__
#define __FDFRAME_H__

#include <QFrame>
#include <QPainterPath>
#include <QGSettings>
#include <QLabel>
#include <QPropertyAnimation>
#include <QShowEvent>
#include "clickLabel.h"

#ifdef KY_SDK_WAYLANDHELPER
#include <ukuiwindowhelper/ukuiwindowhelper.h>
#endif

class FDFrame : public QFrame 
{
    Q_OBJECT
    Q_PROPERTY(int labeltWidth READ readlabeltWidth WRITE setlabeltWidth)
public:
    explicit FDFrame(QWidget* parent = nullptr);
    virtual ~FDFrame();

    void initOpacityGSettings();
    void showNoIcon();
    void setEjectOnCentra(int height);

protected:
    void showEvent(QShowEvent *event) override;

public:
    ClickLabel *m_animLabel;
    int m_mainWindowHigh;

private:
    // QGSettings
    QGSettings *m_gsTransOpacity = nullptr;
    qreal m_curTransOpacity = 1;
    int m_hoverFlag = 0;
    // int m_width;
    bool m_letfToRight = true;
    QLabel *m_ejectLabel;
    QLabel *m_ejectIcon;
    QPropertyAnimation *m_enterAnimation;
    QPropertyAnimation *m_leaveAnimation;
    QString m_currentThemeMode;

    const int m_width = 388;
    const int m_height = 112;
    
#ifdef KY_SDK_WAYLANDHELPER
    UkuiWindowHelper* m_windowHelper = nullptr;
#endif
    
    void paintEvent(QPaintEvent * event);
    bool eventFilter(QObject *obj, QEvent *event);
    int readlabeltWidth();
    void setlabeltWidth(int width);
    void setanimColor();


public Q_SLOTS:
    void on_updateSize();
    void on_cursorLeave();
    //void on_hideframe();
};

#endif
