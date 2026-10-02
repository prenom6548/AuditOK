/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
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

#ifndef FRMLUNARCALENDARWIDGET_H
#define FRMLUNARCALENDARWIDGET_H

#include <QWidget>
#include <QGSettings>
#include "lunarcalendarwidget.h"
#include <QSystemTrayIcon>
#include "unistd.h"
#include "fcntl.h"
#include <KWindowSystem>
#include <QFileInfo>
#include "windowmanager/windowmanager.h"
#include <ukuiwindowhelper/ukuiwindowhelper.h>

namespace Ui {
class frmLunarCalendarWidget;
}

class frmLunarCalendarWidget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.kylin_calendar") //调用DBus一定要加这一行

public:
    explicit frmLunarCalendarWidget(QWidget *parent = 0);
    ~frmLunarCalendarWidget();
    bool isWidgetVisible = false;
    void checkSingle(QStringList path);
    LunarCalendarWidget *m_widget;
    bool isWayland;
    bool isRTLLanguage();
    void setRect(QRect &rect , int x, int y,int width , int height);
    void changeWidowposFromClickButton(bool restore = false);
    bool status;
    enum PanelPosition{
            Bottom = 0, //!< The bottom side of the screen.
            Top,    //!< The top side of the screen.
            Left,   //!< The left side of the screen.
            Right   //!< The right side of the screen.
        };
protected:
    void paintEvent(QPaintEvent *);

private:
    void translator();
    int m_panelPosition;
    int m_nPanelSize;
    QStringList m_argName;
    bool ll = false;
    bool m_isshow;
    QSystemTrayIcon *m_trayIcon = nullptr;
    Ui::frmLunarCalendarWidget *ui;
    int m_num;
    QGSettings *transparency_gsettings;
    QGSettings *calendar_gsettings;
    QGSettings *m_panelGSettings = nullptr;
    bool eventFilter(QObject *, QEvent *);
    bool yijiIsUp = true;
    void changeEvent(QEvent *event);
    QPair<UkuiWindowHelper::Position, int> getSlideEffectPos();
    bool m_isFirstObject = false; //判断是否是唯一的对象

private Q_SLOTS:
    void cboxCalendarStyle_currentIndexChanged(int index);
    void cboxSelectType_currentIndexChanged(int index);
    void cboxWeekNameFormat_currentIndexChanged(bool FirstDayisSun);
    void ckShowLunar_stateChanged(bool arg1);

public Q_SLOTS:
    //true: 顯示大窗口
    //false: 顯示小窗口
    void showAlmanac(bool big);
    void onActivateRequest();
    int command_Control(QString cmd1); //命令控制
    void initPanelGSettings();
    void slotdayWantToChange(int day,int month,int year);
    void slotUpdateSlideEffect();

Q_SIGNALS:
    void yijiFChangeDown();
    void onShowToday();
    void Up(int n);
};

#endif // FRMLUNARCALENDARWIDGET_H
