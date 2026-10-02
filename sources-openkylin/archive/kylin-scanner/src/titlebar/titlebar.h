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
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QColor>
#include <QFont>
#include <QKeySequence>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPalette>
#include <QPoint>
#include <QWidget>
#include <QStyleOption>
#include <QSize>
#include "../saneobject.h"
//#include "../globalsignal.h"

namespace Ui {
class TitleBar;
}

#define TitlebarLogoSize  QSize(24,24)
#define TitlebarFontSize  14
#define TitlebarButtonSize  QSize(30,30)
#define TitlebarHeight  40
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"

#include "src/globalsignal.h"

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void updateMaxButtonStatus(bool isMax);
protected:
    void paintEvent(QPaintEvent *event) override;


Q_SIGNALS:

public slots:
    void on_m_minBtn_clicked();
    void on_m_maxBtn_clicked(bool checked);
    void on_m_closeBtn_clicked();

    void showHelpDialog();

    int warnCloseWindow();

    void fontSizeChanged();
private:
    Ui::TitleBar *ui;
    QMenu *m_menu = nullptr;
    QGSettings *m_themeData = nullptr;
};

#endif // TITLEBAR_H
