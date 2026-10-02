/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef POWERLISTWIDGET_H
#define POWERLISTWIDGET_H

#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QDBusReply>
#include <QScrollBar>
#include <QDBusInterface>

#include "mylistwidget.h"
#include "poweritemwidget.h"
#include "lock-dialog/lockdialogmodel.h"

class MyListWidget;
class PowerItemWidget;
class QListWidgetItem;
class LockDialogModel;
#define LISTSZIE(x) (sizeof(x) / sizeof(x[0]))
//电源管理界面
#define BTN_ITEM_SIZE_WIDTH 204
#define BTN_ITEM_SIZE_HEIGHT 200
#define BTN_ICON_SIZE_WIDTH 48
#define BTN_ICON_SIZE_HEIGHT 48

enum powerType
{
    SAVER,
    SESSION
};

enum stateType
{
    REBOOT,
    SHUTDOWN,
    SLEEP,
    HIBERNATE,
    LOGOUT,
    SWITCHUSER,
    LOCKSCREEN,
    UPGRADETHENREBOOT,
    UPGRADETHENSHUTDOWN,
    NOTHING
};

class PowerListWidget : public MyListWidget
{
    Q_OBJECT
public:
    explicit PowerListWidget(LockDialogModel *model, QWidget *parent = nullptr);
    ~PowerListWidget();
    LockDialogModel *m_modelLockDialog;

    void updateWidgetSize();
    void setSystemSuspend1();
    void setPowerType(int type);
    inline bool getScrollShowStatus()
    {
        return m_scrollIsShow;
    }

    struct Btn_Data_Struct
    {
        QString m_strName;
        QString m_strToolTip;
        QString m_strIcon;
        bool m_show_flag;

        int m_item_width;
        int m_item_height;
        int m_icon_width;
        int m_icon_height;
        QString canFuncName;
        QString setFuncName;
        int m_inhibitType;
        //        void (*func)();
    } powerBtnList[9]
        = { // 1.switchuser
            { .m_strName = tr("SwitchUser"),
              .m_strToolTip = tr(""),
              .m_strIcon = ":/image/assets/switchuser.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanSwitchUser",
              .setFuncName = "SwitchUser",
              .m_inhibitType = SWITCHUSER },
            // 2.Hibernate
            { .m_strName = tr("Hibernate"),
              .m_strToolTip = tr("Turn off your computer, but the app stays open, When the computer is turned on, it "
                                 "can be restored to the state you left"),
              .m_strIcon = ":/image/assets/hibernate.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanHibernate",
              .setFuncName = "Hibernate",
              .m_inhibitType = HIBERNATE },
            // 3.Suspend
            { .m_strName = tr("Suspend"),
              .m_strToolTip = tr("The computer stays on, but consumes less power, The app stays open and can quickly "
                                 "wake up and revert to where you left off"),
              .m_strIcon = ":/image/assets/suspend.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanSuspend",
              .setFuncName = "Suspend",
              .m_inhibitType = SLEEP },
            // 4.LockScreen
            { .m_strName = tr("LockScreen"),
              .m_strToolTip = tr(""),
              .m_strIcon = ":/image/assets/lockscreen.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanLockScreen",
              .setFuncName = "LockScreen",
              .m_inhibitType = LOCKSCREEN },
            // 5.Logout
            { .m_strName = tr("Log Out"),
              .m_strToolTip
              = tr("The current user logs out of the system, terminates the session, and returns to the login page"),
              .m_strIcon = ":/image/assets/logout.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanLogout",
              .setFuncName = "Logout",
              .m_inhibitType = LOGOUT },
            // 6.upgradethenreboot
            { .m_strName = tr("UpgradeThenRestart"),
              .m_strToolTip = tr(""),
              .m_strIcon = ":/image/assets/reboot.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CheckSystemUpgradeReboot",
              .setFuncName = "UpgradeThenRestart",
              .m_inhibitType = UPGRADETHENREBOOT },
            // 7.reboot
            { .m_strName = tr("Restart"),
              .m_strToolTip = tr("Close all apps, turn off your computer, and then turn your computer back on"),
              .m_strIcon = ":/image/assets/reboot.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanReboot",
              .setFuncName = "Reboot",
              .m_inhibitType = REBOOT },
            // 8.upgradethenshutdown
            { .m_strName = tr("UpgradeThenShutdown"),
              .m_strToolTip = tr(""),
              .m_strIcon = ":/image/assets/shutdown.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CheckSystemUpgradeShutdown",
              .setFuncName = "UpgradeThenShutdown",
              .m_inhibitType = UPGRADETHENREBOOT },
            // 9.Shut Down
            { .m_strName = tr("Shut Down"),
              .m_strToolTip = tr("Close all apps, and then shut down your computer"),
              .m_strIcon = ":/image/assets/shutdown.svg",
              .m_show_flag = true,
              .m_item_width = BTN_ITEM_SIZE_WIDTH,
              .m_item_height = BTN_ITEM_SIZE_HEIGHT,
              .m_icon_width = BTN_ICON_SIZE_WIDTH,
              .m_icon_height = BTN_ICON_SIZE_HEIGHT,
              .canFuncName = "CanPowerOff",
              .setFuncName = "PowerOff",
              .m_inhibitType = SHUTDOWN }
          };

    void setSystemPowerStatus(QString);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void mulUsersLogined(int inhibitType, bool iscommand);
    void showInhibitWarning(QStringList list, int type, bool iscommand);
    void lockScreenClicked();
    void suspendClicked();
    void switchuserClicked();
    void sureShutDown(int inhibitType, bool iscommand);
    void powerWidgetClicked();  /*发送电源管理鼠标点击信号，在eventfilter中点击listwidget的空白处，
                                      收不到鼠标点击事件，只有在mouseReleaseEvent中能过滤到*/

public Q_SLOTS:
    void onUpdateListInfo();
    void onListItemClicked(QListWidgetItem *item);

private:
    //    QDBusInterface *sessionInterface;
    //    QDBusInterface *loginInterface;

    bool m_canSuspend;
    bool m_canHibernate;
    bool m_canReboot;
    bool m_canShutDown;

    bool m_canLockScreen = true;
    bool m_canSwitchUser = true;
    bool m_canLogout = true;

    double m_curFontSize;
    double m_ptToPx = 1.0;

    int m_powerType = SAVER;
    int m_powerBtnNum = 0;

    bool m_scrollIsShow = false;

    void initUI();
    void updateBtnShowFlag();
    void initConnections();
    void refreshTranslate();
    //    void initDBusInterface();
};

#endif // POWERLISTWIDGET_H
