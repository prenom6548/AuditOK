/*
 * Ukui-shutdown-timer
 *
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
 *
 * Authors:  liushanwen <liushanwen@kylinos.cn>
 *
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QCoreApplication>
#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <stdio.h>
#include <unistd.h>
#include <QProcess>
#include <QStringLiteral>
#include <QMouseEvent>
#include <QGSettings>
#include <QTranslator>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QLocale>
#include <QFile>
#include <QDir>
#include "confirmareawidget.h"
#include "timeshowwidget.h"
#include "comboxwidget.h"
#include "dropdownmenu.h"
#include "kwidget.h"
#include "kmessagebox.h"
#include "kaboutdialog.h"
#include "usermanual.h"
#include "ukuiwindowhelper.h"

#define UKUITIMESHUTDOWN "org.ukui.time-shutdown.settings"
#define ORG_UKUI_STYLE            "org.ukui.style"
#define SETTAGSFILE "/home/kylin/.config/ukui/tagfile.txt"

using namespace kdk;

/* app主类 */
class Widget : public KWidget
{
    Q_OBJECT
public:
    Widget(KWidget *parent = nullptr);
    ~Widget();


private:
    void initTranslation();
    void initWindowSettings();
    void initMemberVariable();
    void initLayout();

    void initSignalSlots();
    void initDropDownBoxLabelStatus();

    // 初始化托盘图标
    void createActions();
    void createTrayIcon();

    QString getCurrentVersion();

    // getstting初始化、值获取、设置getsetting值
    void initGSettings();
    void initStyleGSettings();
    void getComBoxShutdownFrequency();
    void getShutDownTime();
    bool getTimedShutdownState();
    void initTimeShutdownWidget();
    void setTimeShutdownValue(bool value);
    void setShutDownTimeValue(QString value);
    void setFrequencyValue(QString value);

    // 根据关机频率来选择是否能进行定时关机
    void setWeekSelectShutdownFrequency();
    void setShutdownFrequency(QList<int> list);

    // 设置距离下一次关机时间
    void setNextShutDownTime();
    int getCurrentShutDownNum();
    int caculateNextShutDownNum(int currentTmp, QList<int> weekSelect);
    //计算下次关机所需时间
    QString getCalculateShutDownTime(QList<int> weekSelect);
    bool    determineShutdownTime();

private:
    enum SelectSection{
        NEVER_SHUTDON = 0,
        ONLY_ONCE = 1,
        EVERYDAY = 2,
        MONDAY = 3,
        SUNDAY = 9
    };

    QLabel      *m_pTimeRemainLabel         = nullptr;
    QList<int>   m_weekSelect;
    QString      m_selectIndexGsetting;
    QString      m_pShowDownTime;
    QTimer      *m_pMonitorTime             = nullptr;
    QWidget     *m_pTransparentWidget       = nullptr;
    QGSettings  *m_pTimeShutdownGSettings   = nullptr;
    QGSettings  *m_pStyleGSettings          = nullptr;
    QString m_traslate;
    QString m_traslateHours;
    QString m_traslateMinute;
    QTranslator *m_ptranslator;

    /* 托盘栏图标 */
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    int          m_hours;                                               // 时
    int          m_minute;                                              // 分钟
    int          m_currentTmp = -1;
    int          m_nextCurrenttmp = -2;
    bool         m_timeShutdown = false;

    int m_transparency = 100;

    DropDownMenu      *m_pDropDownBox       = nullptr;
    comBoxWidget      *m_pComBoxWidget      = nullptr;
    confirmAreaWidget *m_pConfirmAreaWidget = nullptr;
    timeShowWidget    *m_pTimeShowWidget    = nullptr;
    BlankShadowWidget *m_pBlankShadowWidget = nullptr;
    UkuiWindowHelper *m_pWindowHelper = nullptr;

protected:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event) override;

public slots:
    void dropDownBoxShowHideSlots();
    void updatedropDownBoxSelectSlots();
    void confirmButtonSlots();
    void canceButtonSlots();
    void threadSlots();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showScreen();
    void bootOptionsFilter(const QString&);
signals:
    void hideDropDownBox();
};
#endif // WIDGET_H
