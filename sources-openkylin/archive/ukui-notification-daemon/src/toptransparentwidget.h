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

#ifndef TOPTRANSPARENTWIDGET_H
#define TOPTRANSPARENTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QQueue>
#include <QPointer>
#include <QGSettings>
#include "popupitemwidget.h"
#include "adaptscreeninfo.h"
#include "platformInfo.h"
#include <QDBusReply>
#include <QDebug>
#define MARGIN 5
#define UKUI_PANEL_SETTING "org.ukui.panel.settings"

#define SSWND_DBUS_SERVICE     "org.ukui.ScreenSaverWnd"
#define SSWND_DBUS_PATH        "/"
#define SSWND_DBUS_INTERFACE   "org.ukui.ScreenSaverWnd"

class topTransparentWidget : public QWidget
{
    Q_OBJECT
public:
    topTransparentWidget(QObject *parent = nullptr);
    ~topTransparentWidget();
    void initUiLayout();
    void setWidgetAttribute();
    void setWidgetPos(int x, int y);
    void AddPopupItemWidget(NotifyReceiveInfo *entryInfo);                     //添加新弹窗
    void AddCenterPopupItemWidget(NotifyReceiveInfo *entryInfo);                     //添加霸屏弹窗
    void replacePopupItemWidget(uint oldInfoId, NotifyReceiveInfo *entryInfo); //替换弹窗内容
    void exitPopupWidget(QWidget *w);
    void addEntryInfo(NotifyReceiveInfo *entryInfo);
    void consumeEntities();
    void initPanelSite();
    void setNotifyPopWidgetSite();
    bool isPopupExist(const uint replacesId); //判断要替换的通知是否存在
    uint isPopupExist(const QString appName);
//    uint isPopupExist(NotifyReceiveInfo *notifyInfo);
    void popupItemAddInfo(NotifyReceiveInfo *notifyInfo);
    void closePopupWidgetAccordId(int id);
    void initScreenNotifications();

    enum PanelStatePosition
    {
        PanelDown = 0,
        PanelUp,
        PanelLeft,
        PanelRight
    };
    enum ClosedReason
    {
        TIMEOUT = 1,
        CLOSEDBYUSER = 2,
        CLOSEDBYAPP = 3,
        UNDEFINED = 4,
        HANDLEDBYUSER = 20
    };

    adaptScreenInfo *m_pSreenInfo = nullptr;
    int m_ListWidgetHeight = 0;
    QList<popupItemWidget *> m_popWidgetList;
    QPointer<NotifyReceiveInfo> m_currentNotify;
    QQueue<NotifyReceiveInfo *> m_entities; //单弹窗模式下的通知队列
    QList<NotifyReceiveInfo *> m_pWaitingQueue;
    uint m_fixNotifyNum = 0;
    int m_ipanelPosition = 0;
    int m_ipanelHeight = 46;
    int m_iScreenXGeometry = 0;
    int m_iScreenYGeometry = 0;
    int m_iScreenWidth = 0;
    int m_iScreenHeight = 0;

protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void actionInvoked(uint id, QString actionKey);
    void notificationClosed(uint id, uint closeReson);

private:
    QVBoxLayout *m_pMainLayout = nullptr;
    QGSettings *m_pPanelSetting = nullptr;
    QPoint m_point;
    QPropertyAnimation *m_posAnimation = nullptr;

private slots:
    void hideAnimationFinishSlots(QWidget *w, int id);    //隐藏动画完成槽函数
    void TransformGroundGlassAreaSlots(const QVariant &value);
    void addWaittingPopupWidgetSlots();
    void panelSiteSlots(QString key);
    void animationActionSlots(const uint time, const int distance);
};

#endif // TOPTRANSPARENTWIDGET_H
