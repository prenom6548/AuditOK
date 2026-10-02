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

#include "toptransparentwidget.h"
#include "centerpopupwidget.h"
#include "notifyreceiveinfo.h"
#include <QPainterPath>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QTime>
#include <QGraphicsDropShadowEffect>
#include "commondefinition.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include "windowmanager/windowmanager.h"

/* 最外层透明窗口四周各1xp阴影 */
#define TOP_SPACING 0
#define BOTTON_SPACING -3
#define LEFT_SPACING 0
#define RIGHT_SPACING 0

topTransparentWidget::topTransparentWidget(QObject *parent)
{
    Q_UNUSED(parent);
    /* 初始化UI布局和窗口属性 */
    initUiLayout();
    /* 初始化任务栏相关Gsettings，监听任务栏高度和方向 */
    initPanelSite();
    /* 实例化屏幕信息类，监听屏幕变化 */
    m_pSreenInfo = new adaptScreenInfo(this);
    connect(m_pSreenInfo, &adaptScreenInfo::updataTopWidgetSite, this, [=]() { setNotifyPopWidgetSite(); });
    /* 设置通知弹窗的位置 */
    setNotifyPopWidgetSite();
    /* 初始化 通知与侧边栏联动动画 */
    if (PLATFORM::g_platformType == PLATFORM::Intel) {
        QDBusConnection::sessionBus().connect(
            QString(""),
            QString("/org/ukui/Sidebar"),
            "org.ukui.Sidebar",
            "animationAction",
            this,
            SLOT(animationActionSlots(uint, int)));
        m_posAnimation = new QPropertyAnimation(this, "pos");
    }
    return;
}

topTransparentWidget::~topTransparentWidget()
{
    if (m_pPanelSetting) {
        delete m_pPanelSetting;
        m_pPanelSetting = NULL;
    }
}

/* 初始化UI布局和窗口属性 */
void topTransparentWidget::initUiLayout()
{
    //初始化布局
    m_pMainLayout = new QVBoxLayout(this);
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);
    m_pMainLayout->setDirection(QBoxLayout::BottomToTop);
    m_pMainLayout->setSpacing(5);
    this->setContentsMargins(LEFT_SPACING, TOP_SPACING, RIGHT_SPACING, BOTTON_SPACING);
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        this->setFixedWidth(MASTER::topWidgetWidth + LEFT_SPACING + RIGHT_SPACING);
    } else if(PLATFORM::g_platformType == PLATFORM::Intel) {
        this->setFixedWidth(INTER::topWidgetWidth + LEFT_SPACING + RIGHT_SPACING);
    }

    this->setLayout(m_pMainLayout);
    //设置窗口属性
    setWidgetAttribute();
}

/* 设置窗口属性 */
void topTransparentWidget::setWidgetAttribute()
{
    setWindowFlags(
        Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint); //无边框、禁止拖动、禁止改变大小、不受窗管管理
    setAttribute(Qt::WA_TranslucentBackground);                    //透明背景
    setAttribute(Qt::WA_AlwaysShowToolTips);
    return;
}

/* 设置窗口位置 */
void topTransparentWidget::setWidgetPos(int x, int y)
{
    // 增加x11 wayland 判断
//    qInfo() << "set pop-up window X:" << x;
//    qInfo() << "set pop-up window Y:" << y;
//    this->move(x, y);

    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        qInfo() << "set pop-up window X:" << x;
        qInfo() << "set pop-up window Y:" << y;
        this->move(x, y);

    } else {
        kdk::WindowManager::setGeometry(this->windowHandle(),QRect(x,
                                                                   y,
                                                                   this->width(),
                                                                   this->height()));
    }


    return;
}

/* 添加弹窗到TopWidget中 */
void topTransparentWidget::AddPopupItemWidget(NotifyReceiveInfo *entryInfo)
{
    if (entryInfo == nullptr) {
        return;
    }
    popupItemWidget *popw = new popupItemWidget(this, entryInfo);

    if (m_popWidgetList.count() == 0) {
        m_ListWidgetHeight = 0;
    }


    /* 绑定弹窗退出信号 */
    connect(popw, &popupItemWidget::notificationClosedSignal, this, &topTransparentWidget::notificationClosed);
    connect(popw, &popupItemWidget::actionInvokedSignal, this, &topTransparentWidget::actionInvoked);

    //隐藏动画完成，退出
    connect(popw, &popupItemWidget::outAnimationFinishSignal, this, &topTransparentWidget::hideAnimationFinishSlots);
    connect(popw, &popupItemWidget::animationAction, this, &topTransparentWidget::TransformGroundGlassAreaSlots);

    m_popWidgetList.append(popw);

    //布局
    if (m_popWidgetList.count() == 1) {
        m_ListWidgetHeight += popw->height();
        this->setFixedHeight(m_ListWidgetHeight + TOP_SPACING + BOTTON_SPACING);
        m_pMainLayout->addWidget(popw);
        popw->setGeometry(0, 0, popw->width(), popw->height());
        this->update();
   } else {
        if(PLATFORM::g_platformType == PLATFORM::V101){
            if (m_ListWidgetHeight < m_pSreenInfo->m_screenHeight - MASTER::topWidgeReservedHeight) {
                m_ListWidgetHeight += popw->height() + MASTER::popupWidgetSpaceHeight;
                this->setFixedHeight(m_ListWidgetHeight + TOP_SPACING + BOTTON_SPACING);
                m_pMainLayout->addWidget(popw);
                popw->setGeometry(0, 0, popw->width(), popw->height());
                this->update();
            } else { //如果超出屏幕高度，则加入等待队列
                m_pWaitingQueue.append(entryInfo);
                m_popWidgetList.removeAt(m_popWidgetList.count() - 1);
                delete popw;
                popw = NULL;
            }
        }
        else if(PLATFORM::g_platformType == PLATFORM::Intel){
            if (m_ListWidgetHeight < m_pSreenInfo->m_screenHeight - INTER::topWidgeReservedHeight) {
               m_ListWidgetHeight += popw->height() + INTER::popupWidgetSpaceHeight;
               this->setFixedHeight(m_ListWidgetHeight + TOP_SPACING + BOTTON_SPACING);
               m_pMainLayout->addWidget(popw);
               popw->setGeometry(0, 0, popw->width(), popw->height());
               this->update();
           } else {
               m_pWaitingQueue.append(entryInfo);
               m_popWidgetList.removeAt(m_popWidgetList.count() - 1);
               delete popw;
               popw = NULL;
           }
        }
    }
    return;
}

void topTransparentWidget::AddCenterPopupItemWidget(NotifyReceiveInfo *entryInfo)
{
    if (entryInfo == nullptr) {
        return;
    }
    CenterPopupWidget *centerPopw = new CenterPopupWidget(nullptr,entryInfo);

    /* 绑定弹窗一系列动作 */
    //点击弹窗右上角关闭按钮，退出
    connect(centerPopw, &CenterPopupWidget::clickedCloseBtn, this, [=](int id, uint closedReason){
        emit notificationClosed(id, closedReason);
    });
    //点击动作按钮，退出
    connect(centerPopw, &CenterPopupWidget::clickedActionButton, this, [=](int id, QString actionKey){
        if (!actionKey.isEmpty()) {
            emit actionInvoked(id, actionKey);
        }
    });
}

/* 替换通知弹窗的内容 */
void topTransparentWidget::replacePopupItemWidget(uint oldInfoId, NotifyReceiveInfo *replaceInfo)
{
    for (int i = 0; i < m_popWidgetList.size(); ++i) {
        popupItemWidget *popwidget = m_popWidgetList.at(i);
        if (popwidget->getPopupWidgetId() == oldInfoId) {
            popwidget->replaceWidgetDate(replaceInfo);
            break;
        }
    }
}

/* 删除popwidget弹窗 */
void topTransparentWidget::exitPopupWidget(QWidget *w)
{
    popupItemWidget* popw = dynamic_cast<popupItemWidget *>(w);
    m_ListWidgetHeight = m_ListWidgetHeight - w->height() - MASTER::popupWidgetSpaceHeight;
    m_pMainLayout->removeWidget(popw);
    m_popWidgetList.removeOne(popw);
    delete popw;
    popw = NULL;
    if (m_ListWidgetHeight <= 0) {
        this->setFixedHeight(0);
    } else {
        this->setFixedHeight(m_ListWidgetHeight + TOP_SPACING + BOTTON_SPACING);
    }
    if (0 == m_popWidgetList.count()) {
        this->hide();
        return;
    }
    this->update();
    return;
}

/* 通知信息加入到通知队列中（单弹窗模式中使用） */
void topTransparentWidget::addEntryInfo(NotifyReceiveInfo *entryInfo)
{
    m_entities.enqueue(entryInfo);
    return;
}

/* 添加一条通知到TopWidget中显示（单弹窗模式中使用）*/
void topTransparentWidget::consumeEntities()
{
    if (!m_currentNotify.isNull()) {
        m_currentNotify->deleteLater();
        m_currentNotify = nullptr;
    }

    if (m_entities.isEmpty()) {
        m_currentNotify = nullptr;
        qApp->quit();
        return;
    }

    m_currentNotify = m_entities.dequeue();
    this->setProperty("blurRegion", QRegion(QRect(0, 0, MASTER::topWidgetWidth, this->height())));
    AddPopupItemWidget(m_currentNotify);
    this->show();
    setNotifyPopWidgetSite();
    return;
}

/* 初始化任务栏相关Gsettings，获取任务栏高度和方向 */
void topTransparentWidget::initPanelSite()
{
    if (QGSettings::isSchemaInstalled(UKUI_PANEL_SETTING))
        m_pPanelSetting = new QGSettings(UKUI_PANEL_SETTING);
    if (m_pPanelSetting != nullptr) {
        connect(m_pPanelSetting, &QGSettings::changed, this, &topTransparentWidget::panelSiteSlots);
        QStringList keys = m_pPanelSetting->keys();
        /* 获取任务栏位置 */
        if (keys.contains("panelposition")) {
            m_ipanelPosition = m_pPanelSetting->get("panelposition").toInt();
        }
        /* 获取任务栏高度 */
        if (keys.contains("panelsize")) {
            m_ipanelHeight = m_pPanelSetting->get("panelsize").toInt();
        }
    }
    return;
}

/* 设置通知弹窗的位置 */
void topTransparentWidget::setNotifyPopWidgetSite()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        m_iScreenXGeometry = m_pSreenInfo->m_nScreen_x;
        m_iScreenYGeometry = m_pSreenInfo->m_nScreen_y;
        m_iScreenWidth = m_pSreenInfo->m_screenWidth;
        m_iScreenHeight = m_pSreenInfo->m_screenHeight;
        switch (m_ipanelPosition) {
            case 1:
                setWidgetPos(m_iScreenXGeometry + m_iScreenWidth - MASTER::topWidgetWidth - MASTER::topWidgeMargin,
                     m_iScreenYGeometry + m_ipanelHeight + MASTER::topWidgeMargin);
                break;
            case 2:
                setWidgetPos(m_iScreenXGeometry + m_iScreenWidth - MASTER::topWidgetWidth - MASTER::topWidgeMargin,
                     m_iScreenYGeometry + MASTER::topWidgeMargin);
                break;
            case 3:
                setWidgetPos(m_iScreenXGeometry + m_iScreenWidth - MASTER::topWidgetWidth - m_ipanelHeight - MASTER::topWidgeMargin,
                     m_iScreenYGeometry + MASTER::topWidgeMargin);
                break;
            default:
                setWidgetPos(m_iScreenXGeometry + m_iScreenWidth - MASTER::topWidgetWidth - MASTER::topWidgeMargin,
                     m_iScreenYGeometry + MASTER::topWidgeMargin);
                break;
        }
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        //获取侧边栏显示的宽度(Intel项目中通知弹窗跟随侧边栏展开和收纳）
        int sidebarWidth = 0;
        QDBusMessage msg = QDBusMessage::createMethodCall(
            "org.ukui.Sidebar", "/org/ukui/Sidebar", "org.ukui.Sidebar", "getPresentSidebarWidth");
        QDBusMessage response = QDBusConnection::sessionBus().call(msg);
        if (response.type() == QDBusMessage::ReplyMessage) {
            sidebarWidth = response.arguments().takeFirst().toInt();
        } else {
            qCritical() << "getPresentSidebarWidth error:" << response.type();
            sidebarWidth = 0;
        }
        //获取当前屏幕旋转状态  (四种状态 "normal" "upside-down" "left" "right")
        QString rotationState = "normal";
        QDBusMessage rotationMsg = QDBusMessage::createMethodCall(
            "com.kylin.statusmanager.interface", "/", "com.kylin.statusmanager.interface", "get_current_rotation");
        QDBusMessage response2 = QDBusConnection::sessionBus().call(rotationMsg);
        if (response2.type() == QDBusMessage::ReplyMessage) {
            rotationState = response2.arguments().takeFirst().toString();
        } else {
            qCritical() << "get_current_rotation error:" << response.type();
        }
        //获取当前主屏幕信息
        m_iScreenXGeometry = m_pSreenInfo->m_nScreen_x;
        m_iScreenYGeometry = m_pSreenInfo->m_nScreen_y;
        m_iScreenWidth = m_pSreenInfo->m_screenWidth;
        m_iScreenHeight = m_pSreenInfo->m_screenHeight;
        //根据任务栏的位置、侧边栏位置和旋转状态决定通知弹窗的显示位置
        switch (m_ipanelPosition) {
            case topTransparentWidget::PanelDown:
                setWidgetPos(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - sidebarWidth - INTER::topWidgeRightMargin), m_iScreenYGeometry + INTER::topWidgeTopMargin);
                m_point.setX(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - sidebarWidth - INTER::topWidgeRightMargin));
                m_point.setY(m_iScreenYGeometry + INTER::topWidgeTopMargin);
                break;
            case topTransparentWidget::PanelUp:
                setWidgetPos(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - sidebarWidth - INTER::topWidgeRightMargin), m_iScreenYGeometry + INTER::topWidgeTopMargin + m_ipanelHeight); // 获取任务栏高度
                m_point.setX(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - sidebarWidth - INTER::topWidgeRightMargin));
                m_point.setY(m_iScreenYGeometry + INTER::topWidgeTopMargin + m_ipanelHeight);
                break;
            case topTransparentWidget::PanelLeft:
                setWidgetPos(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - INTER::topWidgeRightMargin), m_iScreenYGeometry + INTER::topWidgeTopMargin);
                m_point.setX(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - INTER::topWidgeRightMargin));
                m_point.setY(m_iScreenYGeometry + INTER::topWidgeTopMargin);
                break;
            case topTransparentWidget::PanelRight:
                setWidgetPos(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - sidebarWidth - INTER::topWidgeRightMargin - m_ipanelHeight), m_iScreenYGeometry + INTER::topWidgeTopMargin);
                m_point.setX(m_iScreenXGeometry + (m_iScreenWidth - INTER::topWidgetWidth - sidebarWidth - INTER::topWidgeRightMargin - m_ipanelHeight));
                m_point.setY(m_iScreenYGeometry + INTER::topWidgeTopMargin);
                break;
            default:
                break;
        }
    }
    return;
}

/* 判断popupWidget是否在topWidget中 */
bool topTransparentWidget::isPopupExist(const uint replacesId)
{
    for (int i = 0; i < m_popWidgetList.size(); ++i) {
        popupItemWidget *popwidget = m_popWidgetList.at(i);
        if (popwidget->getPopupWidgetId() == replacesId
                && popwidget->isPopupWidgetEnable())
            return true;
    }
    return false;
}

/* 判断popupWidget是否在topWidget中
 * 若存在返回 弹窗ID;否则返回 0*/
uint topTransparentWidget::isPopupExist(const QString appName)
{
    for (int i = 0; i < m_popWidgetList.size(); ++i) {
        popupItemWidget *popwidget = m_popWidgetList.at(i);
        if (popwidget->getAppName() == appName && popwidget->isPopupWidgetEnable()) {
            return popwidget->getPopupWidgetId();
        }
    }
    return 0;
}

void topTransparentWidget::popupItemAddInfo(NotifyReceiveInfo *notifyInfo)
{
    for (int i = 0; i < m_popWidgetList.size(); ++i) {
        popupItemWidget *popwidget = m_popWidgetList.at(i);
        if (popwidget->getAppName() == notifyInfo->appName()) {
            //popwidget中添加一条通知内容
            popwidget->addNotifyInfo(notifyInfo);
            break;
        }
    }
}

/* 重新绘制背景色 */
void topTransparentWidget::paintEvent(QPaintEvent *event)
{
    //重新计算并设置topWidget高度
    int topwidgetHeight = 0;
    for(int i=0; i<m_popWidgetList.size(); i++) {
        topwidgetHeight += m_popWidgetList.at(i)->height();
    }
    topwidgetHeight = topwidgetHeight + TOP_SPACING + BOTTON_SPACING;
    if(PLATFORM::g_platformType == PLATFORM::V101) {
        topwidgetHeight += MASTER::popupWidgetSpaceHeight * (m_popWidgetList.size());
    } else if(PLATFORM::g_platformType == PLATFORM::Intel) {
        topwidgetHeight += INTER::popupWidgetSpaceHeight * (m_popWidgetList.size());
    }
    this->setFixedHeight(topwidgetHeight);

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QPainterPath path;
    opt.rect.adjust(0, 0, 0, 0);
    path.addRoundedRect(opt.rect, 6, 6);
    p.setRenderHint(QPainter::Antialiasing); //反锯齿
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

/* 隐藏动画完成槽函数 */
void topTransparentWidget::hideAnimationFinishSlots(QWidget *w, int id)
{
    popupItemWidget *popw = dynamic_cast<popupItemWidget *>(w);
    popw->hide();
    exitPopupWidget(w);
    addWaittingPopupWidgetSlots();

    if (0 == this->m_popWidgetList.count()) {
        qApp->quit();
    }
    return;
}

/* 通过id关闭对应的弹窗窗口 */
void topTransparentWidget::closePopupWidgetAccordId(int id)
{
    for (int i = 0; i < m_popWidgetList.count(); i++) {
        popupItemWidget *popupWidget = m_popWidgetList.at(i);
        if (popupWidget->getPopupWidgetId() == id && popupWidget->isPopupWidgetEnable()) {
            popupWidget->closePopupWidget(id);
            break;
        }
    }
    return;
}

/* 调用锁屏接口实现在锁屏显示窗口 */
void topTransparentWidget::initScreenNotifications()
{
    QDBusInterface *iface = new QDBusInterface(SSWND_DBUS_SERVICE,
                                               SSWND_DBUS_PATH,
                                               SSWND_DBUS_INTERFACE);
    if (iface && iface->isValid()) {
        QDBusReply<int> reply =  iface->call("RegisteSubWnd", this->winId());
        if(reply.isValid()) {
            qDebug() << "Result :"<<reply.value();
        } else {
            qDebug() << "Result invalid！！";
        }
    }
}

/* 毛玻璃效果 */
void topTransparentWidget::TransformGroundGlassAreaSlots(const QVariant &value)
{
    if (m_popWidgetList.count() == 1) {
        this->setProperty("blurRegion", QRegion(QRect(1, 1, 1, 1)));
    }
}

/* 将等待队列中的消息添加到topWidget中 */
void topTransparentWidget::addWaittingPopupWidgetSlots()
{
    if (m_pWaitingQueue.count() != 0) {
        AddPopupItemWidget(m_pWaitingQueue.takeAt(0));
    }
    return;
}

/* 任务栏位置或高度改变槽函数 */
void topTransparentWidget::panelSiteSlots(QString key)
{
    if (key == "panelposition" || key == "panelsize") {
        m_ipanelPosition = m_pPanelSetting->get("panelposition").toInt();
        m_ipanelHeight = m_pPanelSetting->get("panelsize").toInt();
        setNotifyPopWidgetSite();
    }
    return;
}

/* 通知与侧边栏联动动画
 * 参数：
 *      time:动画持续时间
 *      distance：移动距离
 */
void topTransparentWidget::animationActionSlots(const uint time, const int distance)
{
    /* 获取任务栏位置 */
    int panelPosition = 0;
    if (m_pPanelSetting && m_pPanelSetting->keys().contains("panelposition")) {
        panelPosition = m_pPanelSetting->get("panelposition").toInt();
    } else {
        return;
    }
    if (panelPosition != topTransparentWidget::PanelLeft) {
        QPoint startPoint;
        QPoint endPoint;
        startPoint = m_point;
        endPoint = m_point - QPoint(distance, 0);
        m_point = endPoint; //记录当前结束位置，作为下一次的开始位置
        m_posAnimation->stop();
        m_posAnimation->setDuration(time);
        m_posAnimation->setStartValue(startPoint);
        m_posAnimation->setEndValue(endPoint);
        m_posAnimation->start();
    }
}
