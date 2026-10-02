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

#ifndef POPUPITEMWIDGET_H
#define POPUPITEMWIDGET_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QHBoxLayout>
#include <QUrl>
#include <QIcon>
#include <QScreen>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QFontMetrics>
#include <QProcess>
#include <QGSettings>
#include <QTranslator>
#include <QToolButton>
#include <QDBusReply>
#include <QDBusInterface>
#include "notifyreceiveinfo.h"
#include "platformInfo.h"
#include "popupitemwidget.h"
#include <QFileInfo>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QFile>
#include <KApplicationTrader>
#define SYSTEM_DESKTOP_PATH      "/etc/xdg/autostart/"

//#include "toptransparentwidget.h"

#define UKUI_TRANSPARENCY_SETTING_PATH "org.ukui.control-center.personalise"
#define UKUI_TRANSPARENCY_SETTING_KEY "transparency"
#define UKUI_STYLE_SETTING_PATH "org.ukui.style"
#define UKUI_STYLE_FONT_SIZE_SETTING_KEY "system-font-size"

#define DEFAULT_ACTION "default"
#define URLS_ACTION "x-kde-urls"

#define ORG_UKUI_STYLE "org.ukui.style"
#define ICON_THEME_NAME "iconThemeName"

static const QString SidebarNotifyDBusService = "org.ukui.Sidebar";
static const QString SidebarNotifyDBusPath = "/org/ukui/Sidebar";
static const QString SidebarNotifyDBusInterface = "org.ukui.Sidebar.notify";

class popupItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit popupItemWidget(QWidget *parent = nullptr, NotifyReceiveInfo *entryInfo = 0);
    ~popupItemWidget();

    QString getAppName();
    uint getPopupWidgetId();
    NotifyReceiveInfo* getPopupNotifyInfo();
    void addNotifyInfo(NotifyReceiveInfo *entryInfo); //添加一条消息到popupWidget弹窗
    bool isPopupWidgetEnable(); //弹窗是否可用
    void closePopupWidget(int id); //应用通过ID关闭弹窗
    void replaceWidgetDate(NotifyReceiveInfo *replaceInfo); // 替换 popupWidget 内容

    enum ClosedReason
    {
        TIMEOUT = 1,
        CLOSEDBYUSER = 2,
        CLOSEDBYAPP = 3,
        UNDEfINED = 4,
        HANDLEDBYUSER = 20
    };

private:
    void initUiLayout();            // 初始化UI
    void initIconWidgetlayout();    // 初始化图标区域布局
    void initInfoWidgetLayout();    // 初始化中间UI布局
    void initLabelSizeInfo();       // 初始化所有label信息
    void initOperationButton();     // 初始化操作区按钮
    void initFoldWidgetLayout();    // 初始化折叠提示label
    void initCloseButtonWidget();   // 初始化关闭按钮界面
    void initTimer();               // 初始化显示时长定时器
    void initTransparencySetting(); // 初始化控制面板gsetting值
    void setWidgetAttribute();      // 设置窗口属性
    void initWidgetAnimations();    // 初始化显示和消失动画
    void widgetOutAnimation();      // 开启窗口退出动画
    void initScreenLockConnect();   // 链接锁屏信号
    void initScreenUnLockConnect();   // 链接解锁信号

    bool getScreenLockStates();
    bool getScreenAppNotify(QString appName);
    bool getShowDetail(QString appName);



    void systemThemeChanges();
    void updataWidgetData();    //更新 popupWidget 显示内容

    void updataTimeoutTimer(NotifyReceiveInfo *notifyInfo); // 更新超时定时器
    void showToSidebar();   //将弹窗里所有消息收纳到侧边栏
    void showToSidebar(NotifyReceiveInfo *notifyInfo);    //（超时信息）收起一条消息到侧边栏
    void initTranslation();


    bool containsMouse() const;                                     // 判断鼠标是否在当前弹窗中
    void convertToImage();                                          // 将路径中的数据转换成图片
    void updateBodyAndSummaryText();  //更新主题和正文
    void updateButtonAction(); //更新按钮动作
    void updateFoldTextAction(); //更新折叠通知数量

    void getAppinfo();

    QString SetFormatBody(QString text, QLabel *label);             // 超过label长度，则显示...
    QString judgeBlankLine(QStringList list);                       // 判断第一行是否有空行
    QString setButtonStringBody(QString text, QToolButton *button); // 设置button字体...
    QMap <QString, QString> appInfoMap; // 储存Name和desktopEntryName键值对,key为Name， value为desktopEntryName
    QStringList existList; // 保存已存在的应用，防止重复

    void clearAllActionButton();                 // 删除掉所有动作按钮
    bool substringSposition(QString formatBody, QStringList list);
    void jumpThroughSidebar(const QString &urlStr); //链接跳转由侧边栏完成
    bool jumpThroughAppManager(const QString &urlStr);//应用管理代理Url跳转
    void setFontStyle(QLabel *label, uint fontSize, qreal fontOpacity); //设置字体样式
    void setFontStyle(QLabel *label, qreal fontOpacity); //设置字体样式
    void setPopupWidgetSize(); //设置窗口大小

Q_SIGNALS:

    void outAnimationFinishSignal(QWidget *w, int id); //退出动画完成信号
    void clickedNotifyBody(QWidget *w, int id);        //鼠标点击消息体信号
    void clickedDefaultAction();     //点击消息体,有默认动作发出该信号
    void animationAction(const QVariant &value);

    void actionInvokedSignal(uint id, QString actionKey); //跳转关闭信号
    void notificationClosedSignal(uint id, uint closeReson); //其他原因关闭信号

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;

private:
    QHBoxLayout *m_pMainHBoxLayout = nullptr;
    QHBoxLayout *m_pOperationButtonWidgetLayout = nullptr;
    QVBoxLayout *m_pIconWidgetLayout = nullptr;
    QVBoxLayout *m_pLeftVBoxLayout = nullptr;
    QVBoxLayout *m_pCloseWidgetLayout = nullptr;
    QVBoxLayout *m_pSummaryLabelWidgetLayout = nullptr;
    QVBoxLayout *m_pBodyLabelWidgetLayout = nullptr;
    QVBoxLayout *m_pFoldLabelWidgetLayout = nullptr;

    QLabel *m_pIconLabel = nullptr;
    QLabel *m_pTextBodyLabel = nullptr;
    QLabel *m_pScreenBodyLabel = nullptr;
    QLabel *m_pSummaryLabel = nullptr;
    QLabel *m_pTipLabel = nullptr;
    QLabel *m_pFoldLabel = nullptr;

    QPushButton *m_pCloseButton = nullptr;

    QPropertyAnimation *m_pMoveAnimation = nullptr;

    QWidget *m_pMainWidget = nullptr;
    QWidget *m_pIconWidget = nullptr;
    QWidget *m_pInfoAreaWidget = nullptr;
    QWidget *m_pCloseButtonWidget = nullptr;
    QWidget *m_pOperationWidget = nullptr;
    QWidget *m_pSummaryLabelWidget = nullptr;
    QWidget *m_pBodyLabelWidget = nullptr;
    QWidget *m_pFoldLabelWidget = nullptr;

    QList<QToolButton *> *m_pListButton = nullptr; //记录弹窗中的按钮，清空布局时使用
    QGSettings *m_pFontStyleGsetting = nullptr;
    QGSettings *m_pTransparencyGsetting = nullptr;
    QGSettings *m_pThemeGsetting = nullptr;
    int m_iStyleFontSize;
    float m_fTransparencyValue;
    bool bodyIsTwoLine = false;

    QString m_appName;
    uint m_popupWidgetId;
    /* m_popupWidgetEnable 是弹窗可用标志位，在弹窗发出退出信号后需要置 false。
    在弹窗退出到弹窗被销毁的需要经过一段动画时间，这期间防止弹窗被使用*/
    bool m_popupWidgetEnable = true;
    QList<NotifyReceiveInfo*> m_notifyInfoList; //保存该弹窗的所有消息内容
    NotifyReceiveInfo *m_currentNotifyInfo = nullptr; //当前正在显示的通知
    NotifyReceiveInfo *m_foldNotifyInfo = nullptr; //折叠通知的信息类：“*条通知”
    QTimer *m_notifyTimeout = nullptr; //单条消息超时定时器
    QTimer *m_maximumTimeout = nullptr; //单条消息最大超时时间定时器
    bool m_flodStatus = false;
    QPropertyAnimation *m_pOutAnimation = nullptr;

private slots:
    void closeButtonSlots();
    void screenLock();
    void screenUnLock();
};

#endif // POPUPITEMWIDGET_H
