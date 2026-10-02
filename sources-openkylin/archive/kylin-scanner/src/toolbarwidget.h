/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef TOOLBARWIDGET_H
#define TOOLBARWIDGET_H

#include "globalsignal.h"
#include "svghandler.h"
#include "saneobject.h"

#include <QDebug>

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <kborderlessbutton.h>
#include <QMutex>
#include "runningdialog.h"
#include "utils/HorizontalOrVerticalMode.h"
#define ToolBarWidgetColor QColor(249, 249, 249, 0.8*255)
#define ToolBarWidgetBlur 18
#define ToolBarWidgetFixedSize QSize(421, 36)
#define ToolBarWidgetTableModeSize QSize(522, 64)
#define ToolBarWidgetButtonSize QSize(36, 36)
#define ToolBarWidgetTableModeButtonSize QSize(48, 48)
#define ToolBarWidgetFrameSize QSize(1, 16)
#define ToolBarWidgetZoomButtonSize QSize(20, 20)
#define ToolBarWidgetTableModeZoomButtonSize QSize(36, 36)

class ToolBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBarWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

    void themeChangedIconBlackSettings();
    void themeChangedIconWhiteSettings();
    void ocrOtherIconState(bool flag);
    void cropOtherIconState(bool flag);
signals:

protected:
    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;


public slots:
    void percentageChangedSlot();

    void beautyButtonClickedSlot();
    void rectifyButtonClickedSlot();
    void ocrButtonClickedSlot();
    void cropButtonClickedSlot();
    void rotateButtonClickedSlot();
    void mirrorButtonClickedSlot();
    void watermarkButtonClickedSlot();
    void zoomoutButtonClickedSlot();
    void zoominButtonClickedSlot();
    void initTimer();
    void setRectifyButtonEnable();
    void rotateChangedSlot(bool isPCMode);
	
private:

    kdk::KBorderlessButton *m_beautyButton = nullptr;
    kdk::KBorderlessButton *m_rectifyButton = nullptr;
    kdk::KBorderlessButton *m_ocrButton = nullptr;

    QFrame *m_leftFrame = nullptr;

    kdk::KBorderlessButton *m_cropButton = nullptr;
    kdk::KBorderlessButton *m_rotateButton = nullptr;
    kdk::KBorderlessButton *m_mirrorButton = nullptr;
    kdk::KBorderlessButton *m_watermarkButton = nullptr;

    QFrame *m_rightFrame = nullptr;

    kdk::KBorderlessButton *m_zoomOutButton = nullptr; // Make small
    QLabel *m_percentageLabel = nullptr;
    kdk::KBorderlessButton *m_zoomInButton = nullptr; // Make large

    QHBoxLayout *m_mainHLayout = nullptr;
    RunningDialog *beautyRunningDialog = nullptr;
    QTimer *m_timer = nullptr;
    QMutex *m_mutex = nullptr;
	HorizontalOrVerticalMode m_mode;
	
    void drawFrame();
};

#endif // TOOLBARWIDGET_H
