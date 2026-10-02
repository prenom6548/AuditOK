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

#include "popupitemwidget.h"
#include <QPainterPath>
#include <QTranslator>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include "commondefinition.h"

#define KYLIN_APP_MANAGER_NAME         "com.kylin.AppManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/AppManager"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.AppManager"
#define KYLIN_APP_MANAGER_METHOD       "LaunchDefaultAppWithUrl"
#define TOOLBUTTON_LEFT_RIGHT_PADDING  26

#define UKUI_CLOCK_NAME         "org.kylin.dbus.ukuiclock"
#define UKUI_CLOCK_PATH         "/org/kylin/dbus/ukuiclock/noticeShowAgain"
#define UKUI_CLOCK_INTERFACE    "org.kylin.dbus.ukuiclock"
#define UKUI_CLOCK_METHODS      "noticeShowAgain"

popupItemWidget::popupItemWidget(QWidget *parent, NotifyReceiveInfo *entryInfo)
    : QWidget(parent)
    , m_pIconLabel(new QLabel(this))
    , m_pSummaryLabel(new QLabel(this))
    , m_pFoldLabel(new QLabel(this))
    , m_pTextBodyLabel(new QLabel(this))
    , m_pScreenBodyLabel(new QLabel(this))
    , m_pCloseButton(new QPushButton(this))
    , m_pListButton(new QList<QToolButton *>)
    , m_appName(entryInfo->appName())
    , m_popupWidgetId(entryInfo->id().toUInt())
    , m_currentNotifyInfo(entryInfo)
{
    m_notifyInfoList.append(entryInfo);

    //初始化翻译文件、初始化窗口属性、初始化透明度跟随主题
    /* 初始化翻译文件 */
    initTranslation();
    /* 设置窗口属性 */
    setWidgetAttribute();
    /* 监听主题变化 */
    systemThemeChanges();
    /* 初始化控制面板透明度gsetting值 */
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        initTransparencySetting();
    }

    getAppinfo();
    //初始化UI和动画
    /* 初始化图标 */
    initIconWidgetlayout();
    /* 初始化锁屏信号连接 */
    initScreenLockConnect();
    initScreenUnLockConnect();
    /* 初始化主题、正文label */
    initLabelSizeInfo();
    /* 初始化操作按钮 */
    initOperationButton();
    /* 初始化折叠提示label */
    initFoldWidgetLayout();
    /* 初始化中间部分UI */
    initInfoWidgetLayout();
    /* 初始化关闭按钮 */
    initCloseButtonWidget();
    /* 初始化整体UI */
    initUiLayout();
    /* 初始化定时器 */
    initTimer();
    /* 初始化动画 */
    initWidgetAnimations();

    //从 m_notifyInfoList 中获取通知消息，显示该弹窗的第一条通知
    updataWidgetData();
}

popupItemWidget::~popupItemWidget()
{
    if (m_pTransparencyGsetting) {
        delete m_pTransparencyGsetting;
        m_pTransparencyGsetting = NULL;
    }
    if (m_pThemeGsetting) {
        delete m_pThemeGsetting;
        m_pThemeGsetting = NULL;
    }
    if (m_pOutAnimation) {
        delete m_pOutAnimation;
        m_pOutAnimation = NULL;
    }
    if (m_pMoveAnimation) {
        delete m_pMoveAnimation;
        m_pMoveAnimation = NULL;
    }
}

/* 监听主题变化 */
void popupItemWidget::systemThemeChanges()
{
    const QByteArray styleId(ORG_UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(styleId)) {
        m_pThemeGsetting = new QGSettings(styleId);
        connect(m_pThemeGsetting, &QGSettings::changed, this, [=](const QString &key) {
            if (key == ICON_THEME_NAME) {
                convertToImage(); //主题变化更新图标
            }
        });
    }
}

/* 更新 popupWidget 显示内容 */
void popupItemWidget::updataWidgetData()
{
    m_notifyTimeout->stop();
    m_maximumTimeout->stop();

    convertToImage();
    updateBodyAndSummaryText();
    updateButtonAction();
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        updateFoldTextAction();
    }

    if (PLATFORM::g_platformType == PLATFORM::Intel) {
        setPopupWidgetSize();
    }
    //更新超时定时器
    updataTimeoutTimer(m_currentNotifyInfo);

    return;
}

/* 替换弹窗的内容 */
void popupItemWidget::replaceWidgetDate(NotifyReceiveInfo *replaceInfo)
{
    m_notifyTimeout->stop();
    m_maximumTimeout->stop();

    m_notifyInfoList[0] = replaceInfo;
    m_currentNotifyInfo = replaceInfo;
    updataWidgetData();
    return;
}

/* 更新超时定时器 */
void popupItemWidget::updataTimeoutTimer(NotifyReceiveInfo *notifyInfo)
{
    m_notifyTimeout->setSingleShot(true);
    m_maximumTimeout->setSingleShot(true);

    if (notifyInfo->timeout().toInt() == -1) {  //默认驻留时间 3 秒
        m_notifyTimeout->setInterval(g_defaultTimer * 1000);
        m_maximumTimeout->setInterval(g_quitTimer * 1000);
    } else if (notifyInfo->timeout().toInt() == 0) {  //弹窗常驻
        m_notifyTimeout->blockSignals(true);
        m_maximumTimeout->blockSignals(true);
    } else {                                    //应用自定义驻留时间
        m_notifyTimeout->setInterval(notifyInfo->timeout().toInt());
        m_maximumTimeout->setInterval(g_quitTimer * 1000);
    }
    m_notifyTimeout->start();
    m_maximumTimeout->start();
}

/* 将弹窗里所有消息收纳到侧边栏内 */
void popupItemWidget::showToSidebar()
{
    for(int i=0; i<m_notifyInfoList.size(); ++i) {
        showToSidebar(m_notifyInfoList.at(i));
    }
}

/* 超时信息收纳到侧边栏 */
void popupItemWidget::showToSidebar(NotifyReceiveInfo *notifyInfo)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.ukui.Sidebar",
                                                      "/org/ukui/Sidebar/notification",
                                                      "org.ukui.Sidebar.notification",
                                                      "sidebarNotification");
    //发送的数据：appName, appIcon, summary, body, urlStr,  actions
    msg << notifyInfo->appName() << notifyInfo->appIcon() << notifyInfo->summary() << notifyInfo->bodyText()
        << notifyInfo->bodyUrl() << notifyInfo->defaultActions(); //填充要发送的数据
    qInfo() << "Stow to sidebar:" << msg;
    QDBusPendingCall async = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    delete watcher;
    watcher = NULL;
}

/* 初始化整体UI布局 */
void popupItemWidget::initUiLayout()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        this->setContentsMargins(0, 0, 0, 0);
        m_pMainWidget = new QWidget(this);
        m_pMainWidget->setContentsMargins(0, 0, 0, 0);
        m_pMainHBoxLayout = new QHBoxLayout(m_pMainWidget);
        m_pMainHBoxLayout->setContentsMargins(0, 0, 0, 0);
        m_pMainHBoxLayout->setSpacing(0);
        m_pMainHBoxLayout->addWidget(m_pIconWidget);
        m_pMainHBoxLayout->addItem(new QSpacerItem(12, 0));
        m_pMainHBoxLayout->addWidget(m_pInfoAreaWidget);
        m_pMainHBoxLayout->addWidget(m_pCloseButtonWidget);
        m_pMainWidget->setFixedSize(MASTER::popupWidgetSize);
        m_pMainWidget->setLayout(m_pMainHBoxLayout);

        this->setContentsMargins(0,0,0,0);
        this->setFixedSize(m_pMainWidget->size());
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        this->setContentsMargins(0, 0, 0, 0);
        m_pMainWidget = new QWidget(this);
        m_pMainWidget->setContentsMargins(0, 0, 0, 0);
        m_pMainHBoxLayout = new QHBoxLayout(m_pMainWidget);
        m_pMainHBoxLayout = new QHBoxLayout(this);
        m_pMainHBoxLayout->setContentsMargins(0, 0, 0, 0);
        m_pMainHBoxLayout->addItem(new QSpacerItem(16, 0, QSizePolicy::Fixed));
        m_pMainHBoxLayout->addWidget(m_pIconWidget);
        m_pMainHBoxLayout->addItem(new QSpacerItem(8, 0, QSizePolicy::Fixed));
        m_pMainHBoxLayout->addWidget(m_pInfoAreaWidget);
        m_pMainHBoxLayout->addItem(new QSpacerItem(2, 0, QSizePolicy::Expanding));
        m_pMainHBoxLayout->addWidget(m_pCloseButtonWidget);
        m_pMainHBoxLayout->addItem(new QSpacerItem(16, 0, QSizePolicy::Fixed));
        m_pMainWidget->setFixedSize(INTER::popupWidgetSize);
        m_pMainWidget->setLayout(m_pMainHBoxLayout);
#if 0
        //修改UI时打开，可以看到每个组件的边框
        this->setStyleSheet(QString::fromUtf8("border:1px solid blue"));
#endif
    }
}

/* 初始化图标 */
void popupItemWidget::initIconWidgetlayout()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        m_pIconWidget = new QWidget(this);
        m_pIconWidget->setFixedWidth(MASTER::iconWidgetWidth);
        m_pIconWidget->setContentsMargins(0, 0, 0, 0);
        m_pIconWidgetLayout = new QVBoxLayout(m_pIconWidget);
        m_pIconWidgetLayout->setContentsMargins(6, 0, 0, 0);
        m_pIconWidgetLayout->setSpacing(0);
        /* 初始化图标label大小 */
        m_pIconLabel->setFixedSize(MASTER::iconLabelSize);
        m_pIconWidgetLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed));
        m_pIconWidgetLayout->addWidget(m_pIconLabel);
        m_pIconWidgetLayout->addItem(new QSpacerItem(10, 200, QSizePolicy::Expanding));
        m_pIconWidget->setLayout(m_pIconWidgetLayout);
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        m_pIconWidget = new QWidget(this);
        m_pIconWidget->setFixedWidth(INTER::iconWidgetWidth);
        m_pIconWidget->setContentsMargins(0, 0, 0, 0);
        m_pIconWidgetLayout = new QVBoxLayout(m_pIconWidget);
        m_pIconWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pIconWidgetLayout->setSpacing(0);
        /* 初始化图标label大小 */
        m_pIconLabel->setFixedSize(INTER::iconLabelSize);
        m_pIconLabel->setMargin(0);
        m_pIconLabel->setContentsMargins(0, 0, 0, 0);
        m_pIconWidgetLayout->addItem(new QSpacerItem(10, 18, QSizePolicy::Fixed));
        m_pIconWidgetLayout->addWidget(m_pIconLabel);
        m_pIconWidgetLayout->addItem(new QSpacerItem(10, 153, QSizePolicy::Expanding));
        m_pIconWidget->setLayout(m_pIconWidgetLayout);
    }
}

/* 初始化中间部分布局 */
void popupItemWidget::initInfoWidgetLayout()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        m_pInfoAreaWidget = new QWidget(this);
        m_pInfoAreaWidget->setContentsMargins(0, 0, 0, 0);
        m_pLeftVBoxLayout = new QVBoxLayout(m_pInfoAreaWidget);
        m_pLeftVBoxLayout->setContentsMargins(0, 0, 0, 0);
        m_pLeftVBoxLayout->setSpacing(0);
        m_pLeftVBoxLayout->addWidget(m_pSummaryLabelWidget);
        m_pLeftVBoxLayout->addItem(new QSpacerItem(10, 0, QSizePolicy::Minimum));
        m_pLeftVBoxLayout->addWidget(m_pBodyLabelWidget);
        m_pLeftVBoxLayout->addWidget(m_pOperationWidget);
        m_pLeftVBoxLayout->addWidget(m_pFoldLabelWidget);
        m_pLeftVBoxLayout->addItem(new QSpacerItem(10, 5, QSizePolicy::Expanding));
        m_pInfoAreaWidget->setLayout(m_pLeftVBoxLayout);
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        m_pInfoAreaWidget = new QWidget(this);
        m_pInfoAreaWidget->setContentsMargins(0, 0, 0, 0);
        m_pInfoAreaWidget->setFixedWidth(INTER::popupWidgetInfoAreaWidth);
        m_pLeftVBoxLayout = new QVBoxLayout(m_pInfoAreaWidget);
        m_pLeftVBoxLayout->setContentsMargins(0, 0, 0, 0);
        m_pLeftVBoxLayout->setSpacing(0);
        m_pLeftVBoxLayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Fixed));
        m_pLeftVBoxLayout->addWidget(m_pSummaryLabelWidget);
        if (!m_currentNotifyInfo->bodyText().isEmpty()) {
            m_pLeftVBoxLayout->addItem(new QSpacerItem(10, 4, QSizePolicy::Fixed));
            m_pLeftVBoxLayout->addWidget(m_pBodyLabelWidget);
        }
        if (!m_currentNotifyInfo->actions().isEmpty()) {
            m_pLeftVBoxLayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Fixed));
            m_pLeftVBoxLayout->addWidget(m_pOperationWidget);
        }
        m_pLeftVBoxLayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Fixed));
        m_pInfoAreaWidget->setLayout(m_pLeftVBoxLayout);
    }
}

/* 初始化关闭按钮 */
void popupItemWidget::initCloseButtonWidget()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        m_pCloseButtonWidget = new QWidget(this);
        m_pCloseButtonWidget->setFixedWidth(MASTER::closeBtnWidgetWidth);
        m_pCloseButtonWidget->setContentsMargins(0, 0, 0, 0);
        m_pCloseWidgetLayout = new QVBoxLayout(m_pCloseButtonWidget);
        m_pCloseWidgetLayout->setSpacing(0);
        m_pCloseWidgetLayout->setContentsMargins(0, 4, 4, 0);

        QIcon closeButtonIcon = QIcon::fromTheme("window-close-symbolic");
        m_pCloseButton->setProperty("isWindowButton", 0x2);
        m_pCloseButton->setProperty("useIconHighlightEffect", 0x8);
        m_pCloseButton->setFlat(true); //按钮边框不凸起
        m_pCloseButton->setIcon(closeButtonIcon);
        m_pCloseButton->setIconSize(MASTER::closeBtnIconSize);
        m_pCloseButton->setFixedSize(MASTER::closeBtnSize);
        connect(m_pCloseButton, &QPushButton::clicked, this, &popupItemWidget::closeButtonSlots);
        m_pCloseWidgetLayout->addWidget(m_pCloseButton);
        m_pCloseWidgetLayout->addItem(new QSpacerItem(10, 110, QSizePolicy::Expanding));
        m_pCloseButtonWidget->setLayout(m_pCloseWidgetLayout);
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        m_pCloseButtonWidget = new QWidget(this);
        m_pCloseButtonWidget->setFixedWidth(INTER::closeBtnWidgetWidth);
        m_pCloseButtonWidget->setContentsMargins(0, 0, 0, 0);
        m_pCloseWidgetLayout = new QVBoxLayout(m_pCloseButtonWidget);
        m_pCloseWidgetLayout->setSpacing(0);
        m_pCloseWidgetLayout->setContentsMargins(0, 0, 0, 0);
        if (m_currentNotifyInfo->timeout() != "0") {
            QIcon closeButtonIcon = QIcon::fromTheme("window-close-symbolic");
            m_pCloseButton->setIcon(closeButtonIcon);
            m_pCloseButton->setIconSize(INTER::closeBtnIconSize);
            m_pCloseButton->setFixedSize(INTER::closeBtnSize);
            m_pCloseWidgetLayout->addItem(new QSpacerItem(10, 18));
            connect(m_pCloseButton, &QPushButton::clicked, this, &popupItemWidget::closeButtonSlots);
            m_pCloseWidgetLayout->addWidget(m_pCloseButton);
        } else { //强通知消息不显示关闭图标
            m_pCloseButtonWidget->setHidden(true);
            m_pCloseButton->setHidden(true);
        }
        m_pCloseWidgetLayout->addItem(new QSpacerItem(10, 130, QSizePolicy::Expanding));
        m_pCloseButtonWidget->setLayout(m_pCloseWidgetLayout);
    }
}

/* 初始化主题、正文label */
void popupItemWidget::initLabelSizeInfo()
{
    const QByteArray id(UKUI_STYLE_SETTING_PATH);
    if (QGSettings::isSchemaInstalled(id)) {
        m_pFontStyleGsetting = new QGSettings(id);
        if (m_pFontStyleGsetting->keys().contains(UKUI_STYLE_FONT_SIZE_SETTING_KEY)) {
            m_iStyleFontSize = m_pFontStyleGsetting->get(UKUI_STYLE_FONT_SIZE_SETTING_KEY).toInt();
        }
    }
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        /* 标题Label 布局*/
        m_pSummaryLabelWidget = new QWidget(this);
        m_pSummaryLabelWidget->setContentsMargins(0, 0, 0, 0);
        m_pSummaryLabelWidgetLayout = new QVBoxLayout(m_pSummaryLabelWidget);
        m_pSummaryLabelWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pSummaryLabelWidgetLayout->setSpacing(0);
        m_pSummaryLabel->setFixedWidth(MASTER::summaryLabelWidth);
        m_pSummaryLabel->setAlignment(Qt::AlignVCenter);
        m_pSummaryLabelWidgetLayout->addWidget(m_pSummaryLabel);
        m_pSummaryLabelWidget->setLayout(m_pSummaryLabelWidgetLayout);
        /* 正文Label 布局 */
        m_pBodyLabelWidget = new QWidget(this);
        m_pBodyLabelWidget->adjustSize();
        m_pBodyLabelWidget->setContentsMargins(0, 0, 0, 0);
        m_pBodyLabelWidgetLayout = new QVBoxLayout(m_pBodyLabelWidget);
        m_pBodyLabelWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pBodyLabelWidgetLayout->setSpacing(0);
        m_pTextBodyLabel->setFixedWidth(MASTER::bodyLabelWidth);
        m_pTextBodyLabel->setAlignment(Qt::AlignVCenter);
        m_pScreenBodyLabel->setFixedWidth(MASTER::bodyLabelWidth);
        m_pScreenBodyLabel->setAlignment(Qt::AlignVCenter);
        m_pScreenBodyLabel->setText(tr("From ") + m_currentNotifyInfo->appName() + tr(" one notification"));
        m_pScreenBodyLabel->setVisible(false);

        m_pBodyLabelWidgetLayout->addWidget(m_pTextBodyLabel);
        m_pBodyLabelWidgetLayout->addWidget(m_pScreenBodyLabel);
        m_pBodyLabelWidget->setLayout(m_pBodyLabelWidgetLayout);

        // 检测当前是否为锁屏状态
        if (getScreenLockStates()) {
            // 检测当前应用是否允许锁屏展示
            if (getScreenAppNotify(appInfoMap[m_currentNotifyInfo->appName()])) {
                // 检测锁屏状态消息是否允许展示消息内容
                if (!getShowDetail(appInfoMap[m_currentNotifyInfo->appName()])) {
                    m_pTextBodyLabel->setVisible(false);
                    m_pScreenBodyLabel->setVisible(true);
                }
            }
        }

    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        /* 提示label和主题label 布局 */
        m_pTipLabel = new QLabel(this);
        m_pTipLabel->setFixedHeight(INTER::appNameLabelHeight);
        m_pTipLabel->setMargin(0);
        m_pTipLabel->setContentsMargins(0, 0, 0, 0);
        setFontStyle(m_pTipLabel, INTER::appNameFontSize, INTER::appNameFontOpacity);
        if(m_currentNotifyInfo->appName().isEmpty()) {
            m_pTipLabel->setText(tr("Tips"));
        } else {
            m_pTipLabel->setText(m_currentNotifyInfo->appName());
        }

        m_pSummaryLabel->setFixedHeight(INTER::summaryLabelHeight);
        m_pSummaryLabel->setMargin(0);
        m_pSummaryLabel->setContentsMargins(0, 0, 0, 0);
        setFontStyle(m_pSummaryLabel, INTER::summaryFontSize, INTER::summaryFontOpacity);
        m_pSummaryLabel->setAlignment(Qt::AlignVCenter);

        m_pSummaryLabelWidget = new QWidget(this);
        m_pSummaryLabelWidget->setFixedWidth(INTER::summaryLabelWidgetWidth);
        m_pSummaryLabelWidget->setContentsMargins(0, 0, 0, 0);
        m_pSummaryLabelWidgetLayout = new QVBoxLayout(m_pSummaryLabelWidget);
        m_pSummaryLabelWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pSummaryLabelWidgetLayout->setSpacing(0);
        m_pSummaryLabelWidgetLayout->addWidget(m_pTipLabel);
        m_pSummaryLabelWidgetLayout->addItem(new QSpacerItem(10, 8, QSizePolicy::Fixed));
        m_pSummaryLabelWidgetLayout->addWidget(m_pSummaryLabel);
        m_pSummaryLabelWidget->setLayout(m_pSummaryLabelWidgetLayout);

        /* 正文Label 布局 */
        m_pBodyLabelWidget = new QWidget(this);
        m_pBodyLabelWidget->setFixedWidth(INTER::bodyLabelWidgetWidth);
        m_pBodyLabelWidget->setContentsMargins(0, 0, 0, 0);
        m_pBodyLabelWidgetLayout = new QVBoxLayout(m_pBodyLabelWidget);
        m_pBodyLabelWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pBodyLabelWidgetLayout->setSpacing(0);
        m_pTextBodyLabel->setFixedWidth(INTER::bodyLabelWidth);
        m_pTextBodyLabel->setMaximumHeight(INTER::bodyLabelHeight);
        m_pTextBodyLabel->setAlignment(Qt::AlignVCenter);
        setFontStyle(m_pTextBodyLabel, INTER::bodyFontSize, INTER::bodyFontOpacity);
        m_pBodyLabelWidgetLayout->addWidget(m_pTextBodyLabel);
        m_pBodyLabelWidget->setLayout(m_pBodyLabelWidgetLayout);
    }
}

/* 初始化操作按钮 */
void popupItemWidget::initOperationButton()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        m_pOperationWidget = new QWidget(this);
        m_pOperationWidget->setContentsMargins(0, 0, 0, 0);
        m_pOperationWidget->setFixedHeight(MASTER::operationWidgetHeight);
        m_pOperationButtonWidgetLayout = new QHBoxLayout(m_pOperationWidget);
        m_pOperationButtonWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pOperationButtonWidgetLayout->setSpacing(0);
        m_pOperationButtonWidgetLayout->addItem(new QSpacerItem(372, 20, QSizePolicy::Expanding));
        m_pOperationWidget->setLayout(m_pOperationButtonWidgetLayout);
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        m_pOperationWidget = new QWidget(this);
        m_pOperationWidget->setContentsMargins(0, 0, 0, 0);
        m_pOperationButtonWidgetLayout = new QHBoxLayout(m_pOperationWidget);
        m_pOperationButtonWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pOperationButtonWidgetLayout->setSpacing(0);
        m_pOperationWidget->setLayout(m_pOperationButtonWidgetLayout);
    }
}

/* 初始化折叠提示label */
void popupItemWidget::initFoldWidgetLayout()
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        m_pFoldLabelWidget = new QWidget(this);
        m_pFoldLabelWidget->setContentsMargins(0, 0, 0, 0);
        m_pFoldLabelWidget->setFixedHeight(MASTER::foldWidgetHeight);
        m_pFoldLabelWidgetLayout = new QVBoxLayout(m_pFoldLabelWidget);
        m_pFoldLabelWidgetLayout->setContentsMargins(0, 0, 0, 0);
        m_pFoldLabelWidgetLayout->setSpacing(0);
        m_pFoldLabel->setFixedWidth(MASTER::foldLabelWidth);
        m_pFoldLabel->setAlignment(Qt::AlignVCenter);
        m_pFoldLabelWidgetLayout->addWidget(m_pFoldLabel);
        m_pFoldLabelWidgetLayout->addItem(new QSpacerItem(372, 16, QSizePolicy::Expanding));
        m_pFoldLabelWidget->setLayout(m_pFoldLabelWidgetLayout);

    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {

    }
}

/* 初始化显示时长定时器,绑定超时信号，更新弹窗内容 */
void popupItemWidget::initTimer()
{
    m_notifyTimeout = new QTimer(this);
    m_notifyTimeout->setSingleShot(true);
    connect(m_notifyTimeout, &QTimer::timeout, this, [=](){
        //鼠标悬浮在弹窗上时，不退出弹窗，重新计时
        if(containsMouse()) {
            m_notifyTimeout->stop();
            m_notifyTimeout->start();
        } else {
            m_maximumTimeout->stop();
            showToSidebar();
            //弹窗退出信号
            m_popupWidgetEnable = false;
            emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::TIMEOUT);

            widgetOutAnimation();
        }
    });


    m_maximumTimeout = new QTimer(this);
    m_maximumTimeout->setSingleShot(true);
    connect(m_maximumTimeout, &QTimer::timeout, this, [=](){
        m_notifyTimeout->stop();
        showToSidebar();
        //弹窗退出信号
        m_popupWidgetEnable = false;
        emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::TIMEOUT);
        widgetOutAnimation();
    });

}

/* 初始化控制面板透明度gsetting值 */
void popupItemWidget::initTransparencySetting()
{
    QByteArray id(UKUI_TRANSPARENCY_SETTING_PATH);
    if (QGSettings::isSchemaInstalled(id)) {
        m_pTransparencyGsetting = new QGSettings(id);
        if (m_pTransparencyGsetting->keys().contains(UKUI_TRANSPARENCY_SETTING_KEY)) {
            m_fTransparencyValue = m_pTransparencyGsetting->get(UKUI_TRANSPARENCY_SETTING_KEY).toDouble();
        }
        connect(m_pTransparencyGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == UKUI_TRANSPARENCY_SETTING_KEY) {
                m_fTransparencyValue = m_pTransparencyGsetting->get(UKUI_TRANSPARENCY_SETTING_KEY).toDouble();
                update();
            }
        });
    }
    return;
}

/* 设置窗口属性 */
void popupItemWidget::setWidgetAttribute()
{
    this->setWindowFlags(
        Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint); //无边框、禁止拖动、禁止改变大小、不受窗管管理
    this->setAttribute(Qt::WA_TranslucentBackground);                    //透明背景

    return;
}

/* 初始化显示和消失动画 */
void popupItemWidget::initWidgetAnimations()
{
    m_pOutAnimation = new QPropertyAnimation(this, "geometry");
    connect(m_pOutAnimation, &QPropertyAnimation::finished, this, [=]() {
        emit outAnimationFinishSignal(this, m_currentNotifyInfo->id().toInt()); //隐藏动画完成信号
    });

    connect(m_pOutAnimation,&QPropertyAnimation::valueChanged,this,&popupItemWidget::animationAction);

    m_pOutAnimation->setDuration(MASTER::outAnimationDuration);
    m_pOutAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_pMoveAnimation = new QPropertyAnimation(this, "geometry");
    m_pMoveAnimation->setDuration(MASTER::moveAnimationDuration);
    m_pMoveAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_pMoveAnimation->setStartValue(QRect(this->width() - 1, 0, this->width(), 60));
    m_pMoveAnimation->setEndValue(QRect(0, 0, this->width(), 60));
}

/* 判断鼠标是否位于弹窗内 */
bool popupItemWidget::containsMouse() const
{
    QRect rectToGlobal = QRect(mapToGlobal(rect().topLeft()), mapToGlobal(rect().bottomRight()));
    return rectToGlobal.contains(QCursor::pos());
}

/* 通知图标转换 */
void popupItemWidget::convertToImage()
{
    QString iconPath = m_currentNotifyInfo->appIcon();
    const qreal pixelRatio = qApp->primaryScreen()->devicePixelRatio();
    QPixmap pixmap;

    if (PLATFORM::g_platformType == PLATFORM::V101) {
        if (iconPath.startsWith("data:image/")) {
            QStringList strs = iconPath.split("base64,");
            if (strs.length() == 2) {
                QByteArray data = QByteArray::fromBase64(strs.at(1).toLatin1());
                pixmap.loadFromData(data);
            }
        }
    }

    pixmap = QIcon::fromTheme(iconPath).pixmap(QSize(45, 45));
    if (pixmap.isNull()) {
        QString iconUrl;
        const QUrl url(iconPath);
        iconUrl = url.isLocalFile() ? url.toLocalFile() : url.url();

        if (PLATFORM::g_platformType == PLATFORM::V101) {
            if (iconPath.contains("file://")) {
                int length = iconPath.length();
                iconPath = iconPath.mid(7, length);
            }
            const QIcon &icon = QIcon::fromTheme(iconPath, QIcon::fromTheme("application-x-desktop"));
            pixmap = icon.pixmap(width() * pixelRatio, height() * pixelRatio);
        } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
            const QIcon &icon = QIcon::fromTheme(iconPath, QIcon::fromTheme("dialog-information"));
            pixmap = icon.pixmap(QSize(45, 45));
        }
    }

    if (!pixmap.isNull()) {
        pixmap = pixmap.scaled(
            m_pIconLabel->width() * pixelRatio,
            m_pIconLabel->height() * pixelRatio,
            Qt::KeepAspectRatioByExpanding,
            Qt::SmoothTransformation);

        pixmap.setDevicePixelRatio(pixelRatio);
    }

    m_pIconLabel->setPixmap(pixmap);
    return;
}

/* 更新主题和正文 */
void popupItemWidget::updateBodyAndSummaryText()
{
    if (!m_currentNotifyInfo->summary().isEmpty() && m_currentNotifyInfo->bodyText().isEmpty()) {
        m_pBodyLabelWidget->setVisible(false);
        m_pMainWidget->setFixedSize(MASTER::popupWidgetBodyNoExsitSize);

        disconnect(this, &popupItemWidget::clickedNotifyBody, this, 0);
        if(!m_currentNotifyInfo->bodyUrl().isEmpty()) {
            connect(this, &popupItemWidget::clickedNotifyBody, this, [=](QWidget *w, int id) {
                m_notifyTimeout->stop();
                m_maximumTimeout->stop();
                m_popupWidgetEnable = false;
                //折叠状态下，将折叠的消息收纳到侧边栏内
                if(m_flodStatus) {
                    m_notifyInfoList.takeLast();
                    showToSidebar();
                }
                emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::HANDLEDBYUSER);

                bool ret = jumpThroughAppManager(m_currentNotifyInfo->bodyUrl());
                if(!ret) {
                    jumpThroughSidebar(m_currentNotifyInfo->bodyUrl()); //通过侧边栏代理跳转Dbus接口完成跳转
                }
                widgetOutAnimation();
            });
        }

        //更新主题，监听字体变化
        m_pSummaryLabel->setText(SetFormatBody(m_currentNotifyInfo->summary(), m_pSummaryLabel));
        m_pSummaryLabel->setToolTip(m_currentNotifyInfo->summary());
        connect(m_pFontStyleGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == "systemFontSize") {
                m_pSummaryLabel->setText(SetFormatBody(m_currentNotifyInfo->summary(), m_pSummaryLabel));
            }
        });

    } else if (!m_currentNotifyInfo->summary().isEmpty() && !m_currentNotifyInfo->bodyText().isEmpty()){
        m_pBodyLabelWidget->setVisible(true);
        m_pMainWidget->setFixedSize(MASTER::popupWidgetBodyExsitSize);

        //绑定 URL链接 跳转动作
        disconnect(this, &popupItemWidget::clickedNotifyBody, this, 0);
        if(!m_currentNotifyInfo->bodyUrl().isEmpty()) {
            connect(this, &popupItemWidget::clickedNotifyBody, this, [=](QWidget *w, int id) {
                m_notifyTimeout->stop();
                m_maximumTimeout->stop();
                m_popupWidgetEnable = false;
                //折叠状态下，将折叠的消息收纳到侧边栏内
                if(m_flodStatus) {
                    m_notifyInfoList.takeLast();
                    showToSidebar();
                }
                emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::HANDLEDBYUSER);

                bool ret = jumpThroughAppManager(m_currentNotifyInfo->bodyUrl());
                if(!ret) {
                    jumpThroughSidebar(m_currentNotifyInfo->bodyUrl()); //通过侧边栏代理跳转Dbus接口完成跳转
                }
                widgetOutAnimation();
            });
        }

        //更新主题和正文，设置文本显示样式，监听字体变化
        m_pTextBodyLabel->setText(SetFormatBody(m_currentNotifyInfo->bodyText(), m_pTextBodyLabel));
        QFont bodyFont;
        bodyFont.setBold(true);
        m_pTextBodyLabel->setFont(bodyFont);
        m_pTextBodyLabel->setToolTip(m_currentNotifyInfo->bodyText());
        m_pSummaryLabel->setText(SetFormatBody(m_currentNotifyInfo->summary(), m_pSummaryLabel));
        m_pSummaryLabel->setToolTip(m_currentNotifyInfo->summary());
        connect(m_pFontStyleGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == "systemFontSize") {
                m_pTextBodyLabel->setText(SetFormatBody(m_currentNotifyInfo->bodyText(), m_pTextBodyLabel));
                m_pSummaryLabel->setText(SetFormatBody(m_currentNotifyInfo->summary(), m_pSummaryLabel));
            }
        });
    }
}

/* 更新按钮动作 */
void popupItemWidget::updateButtonAction()
{
    //更新默认跳转动作
    disconnect(this, &popupItemWidget::clickedDefaultAction, this, 0);
    if(!m_currentNotifyInfo->defaultActions().isEmpty()) {
        connect(this, &popupItemWidget::clickedDefaultAction, this, [=]() {
            m_notifyTimeout->stop();
            m_maximumTimeout->stop();
            QProcess::startDetached(m_currentNotifyInfo->defaultActions());
            m_popupWidgetEnable = false;
            //折叠状态下，将折叠的消息收纳到侧边栏内
            if(m_flodStatus) {
                m_notifyInfoList.takeLast();
                showToSidebar();
            }
            emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::HANDLEDBYUSER);
            widgetOutAnimation();
        });
    }

    //更新按钮，绑定按钮跳转
    clearAllActionButton(); //清空布局中按钮
    if (m_currentNotifyInfo->buttonActions().count() == 0) {
        m_pOperationWidget->setVisible(false);
        if(PLATFORM::g_platformType == PLATFORM::V101) {
            m_pMainWidget->setFixedSize(MASTER::popupWidgetActionNoExsitSize);
        }
    } else {
        m_pOperationWidget->setVisible(true);
        if (PLATFORM::g_platformType == PLATFORM::Intel) {
            /* 针对护眼中心页面跳转做的临时跳转方案（支持dbus跳转后会删除）：使用Dbus信号，两个动作对应两个信号 */
            if (m_currentNotifyInfo->appName() == "护眼中心" && m_currentNotifyInfo->summary() == "休息提醒") {
                qDebug() << "护眼中心页面跳转 特殊处理";
                for(int i = 0; i < m_currentNotifyInfo->buttonActions().count(); i++) {
                    QString dbusMethod;
                    if(m_currentNotifyInfo->buttonActions().at(i).buttonAction == "action1" && m_currentNotifyInfo->buttonActions().at(i).buttonName == "暂不休息") {
                        dbusMethod = "NoRest";
                    } else if(m_currentNotifyInfo->buttonActions().at(i).buttonAction == "action2" && m_currentNotifyInfo->buttonActions().at(i).buttonName == "我知道了") {
                        dbusMethod = "GotIt";
                    } else {
                        break;
                    }
                    //实例化动作按钮，布局
                    QToolButton *button = new QToolButton(this);
                    button->setText(m_currentNotifyInfo->buttonActions().at(i).buttonName);
                    button->setProperty("isImportant",true);
                    button->setAutoRaise(true);
                    m_pListButton->append(button);
                    button->setFixedHeight(INTER::actionBtnHeight);
                    m_pOperationButtonWidgetLayout->addWidget(button, Qt::AlignRight);
                    m_pOperationButtonWidgetLayout->setSpacing(10);
                    //绑定按钮动作
                    connect(button, &QToolButton::clicked, this, [=](){
                        m_notifyTimeout->stop();
                        m_maximumTimeout->stop();
                        QDBusMessage message = QDBusMessage::createSignal("/org/ukui/notify/eyeprotection",
                                                                          "org.ukui.notify.eyeprotection",
                                                                          dbusMethod);
                        bool ret = QDBusConnection::sessionBus().send(message);
                        if (ret)
                            qDebug() << "护眼中心按钮动作"<<dbusMethod;
                        else
                            qDebug() << "护眼中心按钮动作-信号发送失败";

                        m_popupWidgetEnable = false;
                        emit actionInvokedSignal(m_currentNotifyInfo->id().toInt(), m_currentNotifyInfo->buttonActions().at(i).buttonName);
                        widgetOutAnimation();
                    });
                }
            }
            return;
        }
        else if(PLATFORM::g_platformType == PLATFORM::V101) {
            m_pMainWidget->setFixedSize(MASTER::popupWidgetActionExsitSize);
            for(int i = 0; i < m_currentNotifyInfo->buttonActions().count(); i++) {
                //实例化动作按钮，布局
                QToolButton *button = new QToolButton(this);
                button->setFixedSize(MASTER::actionBtnWidth ,MASTER::actionBtnHeight);
                button->setText(setButtonStringBody(m_currentNotifyInfo->buttonActions().at(i).buttonAction, button));
                connect(m_pFontStyleGsetting, &QGSettings::changed, this, [=](QString key) {
                    if (key == "systemFontSize") {
                        button->setText(setButtonStringBody(m_currentNotifyInfo->buttonActions().at(i).buttonAction, button));
                    }
                });
                m_pListButton->append(button);
                m_pOperationButtonWidgetLayout->addWidget(button, Qt::AlignRight);
                m_pOperationButtonWidgetLayout->setSpacing(10);

                //绑定按钮动作
                connect(button, &QToolButton::clicked, this, [=](){
                    m_notifyTimeout->stop();
                    m_maximumTimeout->stop();
                    if(m_currentNotifyInfo->appName().contains("Alarm") && m_currentNotifyInfo->buttonActions().at(i).buttonName == UKUI_CLOCK_METHODS) {
                        QDBusInterface interface(UKUI_CLOCK_NAME,
                                                 UKUI_CLOCK_PATH,
                                                 UKUI_CLOCK_INTERFACE,
                                                 QDBusConnection::sessionBus());
                        QList<QVariant> args;
                        QString str = "{id:"+m_currentNotifyInfo->id()+"}";
                        args<<str;
                        QDBusMessage response = interface.callWithArgumentList(QDBus::AutoDetect,UKUI_CLOCK_METHODS,args);
                        if (response.type() == QDBusMessage::ReplyMessage &&
                            response.arguments().takeFirst().toUInt() == 0) {
                            qDebug() << "Alarm clock to remind later，Successfully";
                        } else{
                            qDebug() << "Alarm clock to remind later，fail";
                        }
                    } else {
                        if (QProcess::startDetached(m_currentNotifyInfo->buttonActions().at(i).buttonName)) {
                            qInfo() << "Button action successful";
                        } else {
                            qCritical() << "Button action failed";
                        }
                    }
                    m_popupWidgetEnable = false;
                    //折叠状态下，将折叠的消息收纳到侧边栏内
                    if(m_flodStatus) {
                        m_notifyInfoList.takeLast();
                        showToSidebar();
                    }
                    emit actionInvokedSignal(m_currentNotifyInfo->id().toInt(), m_currentNotifyInfo->buttonActions().at(i).buttonName);
                    widgetOutAnimation();
                });
            }
        }
    }
}

/* 更新折叠数量文本 */
void popupItemWidget::updateFoldTextAction()
{
    if(m_flodStatus) {
        m_pFoldLabelWidget->setVisible(true);
        QString str = tr("also") + QString::number(m_notifyInfoList.size()-1) + tr("notice"); //5 more notifications
        setFontStyle(m_pFoldLabel, MASTER::foldLabelFontOpacity);
        m_pFoldLabel->setText(SetFormatBody(str, m_pFoldLabel));
        m_pFoldLabel->setToolTip(str);
        connect(m_pFontStyleGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == "systemFontSize" || key == "styleName") {
                setFontStyle(m_pFoldLabel, MASTER::foldLabelFontOpacity);
                m_pFoldLabel->setText(SetFormatBody(str, m_pFoldLabel));
            }
        });
        m_pMainWidget->setFixedSize(m_pMainWidget->width(),
                                    m_pMainWidget->height() + MASTER::foldWidgetHeight);
        //设置最外层窗口大小
        if(m_notifyInfoList.size()>2) {
            this->setFixedSize(m_pMainWidget->size().width(),
                               m_pMainWidget->size().height()+MASTER::popupWidgetRadius*2);
        } else {
            this->setFixedSize(m_pMainWidget->size().width(),
                               m_pMainWidget->size().height()+MASTER::popupWidgetRadius);
        }
    } else {
        m_pFoldLabelWidget->setVisible(false);
        this->setFixedSize(m_pMainWidget->size());
    }
}

// 预加载应用名称
void popupItemWidget::getAppinfo()
{
    QStringList whitelist;
    whitelist.append("kylin-screenshot.desktop");
    whitelist.append("peony.desktop");
    whitelist.append("kylin-device-daemon.desktop");
    whitelist.append("ukui-powermanagement-daemon.desktop");
    whitelist.append("kylin-system-update.desktop");
    whitelist.append("ukui-bluetooth.desktop");

    for (QString desktopName : whitelist) {
        QFileInfo file(SYSTEM_DESKTOP_PATH + desktopName);
        if (file.exists()) {
            QString mainSection = QStringLiteral("Desktop Entry");
            KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(SYSTEM_DESKTOP_PATH + desktopName, KConfig::NoGlobals);
            KConfigGroup cfg(sharedConfig, mainSection);
            QString name = cfg.readEntry("Name");
            QString fileName = desktopName.left(desktopName.indexOf(QLatin1Char('.')));
            if (existList.contains(name))
                continue;
            existList.append(name);
            appInfoMap[name] = fileName;
        }
    }

    KApplicationTrader::query([=](const KService::Ptr &service) {
        if (service->exec().isEmpty() || service->noDisplay())
            return false;

        QString appname, desktopEntryName;
        appname = service->name();
        desktopEntryName = service->desktopEntryName();

        if (existList.contains(appname))
            return false;
        existList.append(appname);
        appInfoMap[appname] = desktopEntryName;
        return true;
    });
    existList.clear();
}

/* 设置...字样 */
QString popupItemWidget::SetFormatBody(QString text, QLabel *label)
{
    QFontMetrics fontMetrics(label->font());
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        QFontMetrics fontMetricsTmp(label->font());
        fontMetrics = fontMetricsTmp;
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) { // Intel固定14号字体
        QFont bodyFont;
        bodyFont.setPixelSize(INTER::bodyFontSize);
        QFontMetrics fontMetricsTmp(bodyFont);
        fontMetrics = fontMetricsTmp;
    }

    int LableWidth = label->width();
    int fontSize = fontMetrics.width(text);
    QString formatBody = text;
    if (fontSize > LableWidth) {
        QStringList list = formatBody.split("\n",QString::SkipEmptyParts);
        if (PLATFORM::g_platformType == PLATFORM::V101) { // V10sp1 通知正文显示1行
            formatBody = formatBody.simplified(); //删除通知正文中的 '\t'，'\n', '\v', '\f', '\r', and ' ',尽可能多的显示正文内容
            formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, LableWidth - 10);
            return formatBody;
        } else if (PLATFORM::g_platformType == PLATFORM::Intel) { // Intel通知可显示两行
            if (list.size() >= 2) {
                QString firstText;
                QString secondText;
                if (fontMetrics.width(list.at(0)) > (LableWidth)) {
                    firstText = fontMetrics.elidedText(list.at(0), Qt::ElideRight, LableWidth - 10);
                } else {
                    firstText = list.at(0);
                }
                if (fontMetrics.width(list.at(1)) > (LableWidth)) {
                    secondText = fontMetrics.elidedText(list.at(1), Qt::ElideRight, LableWidth - 10);
                } else {
                    secondText = list.at(1);
                }
                formatBody = firstText + "\n" + secondText;
                bodyIsTwoLine = true;
            } else {
                //只有一行内容时如果内容过长则换行显示，超出第二行的部分显示...
                QString strDisplay;
                //遍历正文字符串，在大于qlabel宽度的字符前加空格
                int j = 1;
                for (int i = 0; i < formatBody.length(); i++) {
                    strDisplay += QString(formatBody.at(i));
                    uint fontSize = fontMetrics.width(strDisplay);
                    if (fontSize > (j * (LableWidth - INTER::bodyFontSize))) {
                        strDisplay += "\n ";
                        j++;
                    }
                }
                //超过2行末尾显示省略号
                formatBody = fontMetrics.elidedText(strDisplay, Qt::ElideRight, LableWidth * 2 - 10);
                bodyIsTwoLine = true;
                return formatBody;
            }
        }
    } else {
        formatBody = formatBody.simplified();
        return formatBody;
    }
    return formatBody;
}

/* 去除掉空行、空格，返回首行信息 */
QString popupItemWidget::judgeBlankLine(QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        dest = dest.trimmed(); //删除字符串中的空格
        if (dest.size() != 0) {
            return list.at(i);
        }
    }
    return list.at(0);
}

/* 设置button字体长度，超出button时，设置... */
QString popupItemWidget::setButtonStringBody(QString text, QToolButton *button)
{
    if(button) {
        QFontMetrics fontMetrics(button->font());
        int availableWidth = button->size().width() - TOOLBUTTON_LEFT_RIGHT_PADDING;
        int fontSize = fontMetrics.width(text);
        QString formatBody = text;
        if (fontSize > availableWidth) {
            button->setToolTip(text);
            formatBody = fontMetrics.elidedText(formatBody, Qt::ElideRight, availableWidth - 10);
            return formatBody;
        } else {
            button->setToolTip(QString());
            return formatBody;
        }
    }
    return text;
}

/* 判断当前子串位置，后面是否还有子串 */
bool popupItemWidget::substringSposition(QString formatBody, QStringList list)
{
    int tmp = list.count();
    for (int i = 0; i < tmp; i++) {
        QString dest = list.at(i);
        if (dest == formatBody && i == tmp - 1) {
            return true;
        }
    }
    return false;
}

/* 通过侧边栏代理url链接跳转 */
void popupItemWidget::jumpThroughSidebar(const QString &urlStr)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.ukui.Sidebar",
                                                      "/org/ukui/Sidebar/notification",
                                                      "org.ukui.Sidebar.notification",
                                                      "proxyNotificationJump");
    msg << urlStr;
    QDBusConnection::sessionBus().asyncCall(msg);
}

/* 通过应用管理器完成URL跳转 */
bool popupItemWidget::jumpThroughAppManager(const QString &urlStr)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                      KYLIN_APP_MANAGER_PATH,
                                                      KYLIN_APP_MANAGER_INTERFACE,
                                                      KYLIN_APP_MANAGER_METHOD);
    msg << urlStr;
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        return response.arguments().takeFirst().toBool();
    } else {
        return false;
    }
}

/* 设置字体样式：大小和透明度 */
void popupItemWidget::setFontStyle(QLabel *label, uint fontSize, qreal fontOpacity)
{
    //设置字体透明度
    QColor color = label->palette().windowText().color();
    color.setAlpha(255 * fontOpacity);
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    label->setPalette(palette);
    //设置字体大小
    QFont appNameFont;
    appNameFont.setPixelSize(fontSize);
    label->setFont(appNameFont);
}

void popupItemWidget::setFontStyle(QLabel *label, qreal fontOpacity)
{
    //设置字体透明度
    QPalette palette = label->palette();
    QStyleOption opt;
    opt.init(label);
    QColor color = opt.palette.color(QPalette::Text);
    color.setAlphaF(fontOpacity);
    palette.setBrush(QPalette::WindowText, color);
    label->setPalette(palette);
}

/* 设置弹窗大小 */
void popupItemWidget::setPopupWidgetSize()
{
    if (m_currentNotifyInfo->bodyText().isEmpty() && m_currentNotifyInfo->actions().isEmpty()) {
        //无正文、无按钮
        this->setFixedSize(INTER::popupWidgetNoBodySize);
    } else if (!m_currentNotifyInfo->bodyText().isEmpty() && !bodyIsTwoLine && m_currentNotifyInfo->actions().isEmpty()) {
        //单行正文、无按钮
        this->setFixedSize(INTER::popupWidgetNormalSize);
    } else if (!m_currentNotifyInfo->bodyText().isEmpty() && bodyIsTwoLine && m_currentNotifyInfo->actions().isEmpty()) {
        //两行正文、无按钮
        this->setFixedSize(INTER::popupWidgetTwoLineBodySize);
    } else if (!m_currentNotifyInfo->bodyText().isEmpty() && !bodyIsTwoLine && !m_currentNotifyInfo->actions().isEmpty()) {
        //单行正文、按钮
        this->setFixedSize(INTER::popupWidgetOneLineBodyActionSize);
    } else if (!m_currentNotifyInfo->bodyText().isEmpty() && bodyIsTwoLine && !m_currentNotifyInfo->actions().isEmpty()) {
        //两行正文、按钮
        this->setFixedSize(INTER::popupWidgetTwoLineBodyActionSize);
    } else {
        this->setFixedSize(INTER::popupWidgetNormalSize);
    }
}

/* 清空原布局中的按钮 */
void popupItemWidget::clearAllActionButton()
{
    for (QToolButton *button : *m_pListButton) {
        m_pOperationButtonWidgetLayout->removeWidget(button);
        button->setVisible(false);
    }
    m_pListButton->clear();
    return;
}

/* 重新绘制背景色和窗口阴影 */
void popupItemWidget::paintEvent(QPaintEvent *event)
{
    if (PLATFORM::g_platformType == PLATFORM::V101) {
        //绘制背景
        QStyleOption opt;
        opt.init(this);
//        QColor baseColor = opt.palette.color(QPalette::Base);
        QPainter p(this);
        /* 获取当前剪贴板中字体的颜色，作为背景色；
         * 白字体 --> 黑背景
         * 黑字体 --> 白字体
         */
        p.setBrush(opt.palette.color(QPalette::Base));
        // 此处注释原因为: 毛玻璃绘制问题，无法分区域绘制，折中方案取消毛玻璃与透明度,后续有方案再放开
//        p.setOpacity(m_fTransparencyValue);
        p.setPen(Qt::NoPen);

        // 画背景
        p.setRenderHint(QPainter::Antialiasing); //反锯齿
        p.drawRoundedRect(m_pMainWidget->x(), m_pMainWidget->y(), m_pMainWidget->width(), m_pMainWidget->height(), MASTER::popupWidgetRadius, MASTER::popupWidgetRadius);

        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        //绘制阴影
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true); //渲染，抗锯齿
        QColor color = m_pMainWidget->palette().shadow().color();
        QPainterPath shadowPath;
        shadowPath.setFillRule(Qt::WindingFill);
        shadowPath.addRoundedRect(0, 0,m_pMainWidget->width(),m_pMainWidget->height() , MASTER::popupWidgetRadius, MASTER::popupWidgetRadius);
        painter.setPen(color);
        painter.drawPath(shadowPath);


        // 多层效果
        qreal radius = MASTER::popupWidgetRadius;
        int pathOneOffset = 12;
        int pathTwoOffset = 14;
        QColor colorOne = opt.palette.color(QPalette::Base);
        colorOne.setAlphaF(0.65-(1-m_fTransparencyValue)/2);
        QBrush brachOne(colorOne);
        QColor colorTwo = opt.palette.color(QPalette::Base);
        colorTwo.setAlphaF(0.35-(1-m_fTransparencyValue)/4);
        QBrush brachTwo(colorTwo);

        if(m_notifyInfoList.size() >= 2) {
            /* 第一层 */
            QPainter painterOne(this);
            painterOne.setRenderHint(QPainter::Antialiasing); //反锯齿、
            QPainterPath pathOne;
            pathOne.setFillRule( Qt::WindingFill );
            //中间矩形
            QRect info_rect(m_pMainWidget->x()+radius+pathOneOffset,
                            m_pMainWidget->y()+m_pMainWidget->height(),
                            m_pMainWidget->width()-radius*2-pathOneOffset*2,
                            radius);
            pathOne.addRect (info_rect);
            //左侧圆弧
            painterOne.setPen(QPen(Qt::transparent,4,Qt::SolidLine));
            painterOne.setBrush(brachOne);
            QRect rect(m_pMainWidget->x()+pathOneOffset,
                       m_pMainWidget->y()+m_pMainWidget->height()-radius,
                       radius*2,radius*2);
            painterOne.drawPie(rect,270*16,-90*16);
            //右侧圆弧
            painterOne.setBrush(brachOne);
            QRect rect2(m_pMainWidget->x()+m_pMainWidget->width()-radius*2-pathOneOffset,
                        m_pMainWidget->y()+m_pMainWidget->height()-radius,
                        radius*2,radius*2);
            painterOne.drawPie(rect2, 0*16, -90*16);
            painterOne.fillPath(pathOne,  brachOne);
        }
        if(m_notifyInfoList.size() > 2) {
            /* 第二层 */
            QPainter painterTwo(this);
            painterTwo.setRenderHint(QPainter::Antialiasing); //反锯齿、
            QPainterPath pathTwo;
            pathTwo.setFillRule( Qt::WindingFill );
            //中间矩形
            QRect info_rect2(m_pMainWidget->x()+radius+pathTwoOffset*2,
                            m_pMainWidget->y()+m_pMainWidget->height()+radius,
                            m_pMainWidget->width()-radius*2-pathTwoOffset*4,
                            radius);
            pathTwo.addRect (info_rect2);
            //左侧圆弧
            painterTwo.setPen(QPen(Qt::transparent,4,Qt::SolidLine));
            painterTwo.setBrush(brachTwo);
            QRect rect3(m_pMainWidget->x()+pathTwoOffset*2,
                       m_pMainWidget->y()+m_pMainWidget->height(),
                       radius*2,radius*2);
            painterTwo.drawPie(rect3,270*16,-90*16);
            //右侧圆弧
            painterTwo.setBrush(brachTwo);
            QRect rect4(m_pMainWidget->x()+m_pMainWidget->width()-radius*2-pathTwoOffset*2,
                        m_pMainWidget->y()+m_pMainWidget->height(),
                        radius*2,radius*2);
            painterTwo.drawPie(rect4, 0*16, -90*16);
            painterTwo.fillPath(pathTwo,  brachTwo);
        }
        QWidget::paintEvent(event);
    } else if (PLATFORM::g_platformType == PLATFORM::Intel) {
        //绘制背景
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        /* 获取当前剪贴板中字体的颜色，作为背景色；
         * 白字体 --> 黑背景
         * 黑字体 --> 白字体
         */
        p.setBrush(opt.palette.color(QPalette::Active, QPalette::Base));
        p.setOpacity(1);
        p.setPen(Qt::NoPen);
        QPainterPath path;
        opt.rect.adjust(0, 0, 0, 0);
        path.addRoundedRect(opt.rect, INTER::popupWidgetRadius, INTER::popupWidgetRadius);
        p.setRenderHint(QPainter::Antialiasing); //反锯齿
        p.drawRoundedRect(opt.rect, INTER::popupWidgetRadius, INTER::popupWidgetRadius);
        //绘制阴影
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true); //渲染，抗锯齿
        QColor color = QColor(0, 0, 0, 255 * 0.16); // intel使用的3.0主题没有shadow色，暂时使用固定值
        QPainterPath shadowPath;
        shadowPath.setFillRule(Qt::WindingFill);
        shadowPath.addRoundedRect(0, 0,this->width(),this->height(), INTER::popupWidgetRadius, INTER::popupWidgetRadius);
        painter.setPen(color);
        painter.drawPath(shadowPath);

        QWidget::paintEvent(event);
    }
}

void popupItemWidget::mousePressEvent(QMouseEvent *event)
{
    // 点击消息体时，有默认动作则执行默认动作，无默认动作执行body字段中的链接
    if(!m_currentNotifyInfo->defaultActions().isEmpty()) {
        emit clickedDefaultAction();
    } else if(!m_currentNotifyInfo->bodyUrl().isEmpty()) {
        emit clickedNotifyBody(this, m_currentNotifyInfo->id().toInt());  //通过侧边栏处理URL跳转
    } else {
        if(m_currentNotifyInfo->timeout() != "0") { //非常驻应用点击消息体消失
            m_popupWidgetEnable = false;
            //折叠状态下，将折叠的消息收纳到侧边栏内
            if(m_flodStatus) {
                m_notifyInfoList.takeLast();
                showToSidebar();
            }
            emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::HANDLEDBYUSER);
            widgetOutAnimation();

        }
    }
    QWidget::mousePressEvent(event);
    return;
}

void popupItemWidget::showEvent(QShowEvent *event)
{
    QTimer::singleShot(1, this, [=] {
        raise(); //窗口置顶
    });
    m_maximumTimeout->start();
    QWidget::showEvent(event);
    return;
}

void popupItemWidget::hideEvent(QHideEvent *event)
{
    m_maximumTimeout->stop();
    QWidget::hideEvent(event);
    return;
}

/* 关闭按钮槽函数 */
void popupItemWidget::closeButtonSlots()
{
    m_notifyTimeout->stop();
    m_maximumTimeout->stop();
    m_popupWidgetEnable = false;

    //折叠状态下，将折叠的消息收纳到侧边栏内
    if(m_flodStatus) {
        m_notifyInfoList.takeLast();
        showToSidebar();
    }

    emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::CLOSEDBYUSER);
    widgetOutAnimation();
    return;
}

void popupItemWidget::screenLock()
{
    m_maximumTimeout->stop();
    showToSidebar();
    //弹窗退出信号
    m_popupWidgetEnable = false;
    emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::TIMEOUT);

    widgetOutAnimation();
}

void popupItemWidget::screenUnLock()
{
    // 检测当前是否为锁屏状态
    if (!getScreenLockStates()) {
        // 检测锁屏状态消息是否允许展示消息内容
        if (!getShowDetail(appInfoMap[m_currentNotifyInfo->appName()])) {
            m_pTextBodyLabel->setVisible(true);
            m_pScreenBodyLabel->setVisible(false);
        }
    }
}

/* 初始化翻译文件 */
void popupItemWidget::initTranslation()
{
    QTranslator translator(this);
    if (translator.load(
            QLocale(),
            QLatin1String("ukui-notification-daemon"),
            QLatin1String("_"),
            QLatin1String("/usr/share/ukui-notification-daemon"))) {
        QApplication::installTranslator(&translator);
    } else
        qDebug() << "cannot load translator " << QLocale::system().name() << ".qm!";
}

QString popupItemWidget::getAppName()
{
    return m_appName;
}

uint popupItemWidget::getPopupWidgetId()
{
    return m_popupWidgetId;
}

NotifyReceiveInfo *popupItemWidget::getPopupNotifyInfo()
{
    return m_currentNotifyInfo;
}

void popupItemWidget::addNotifyInfo(NotifyReceiveInfo *entryInfo)
{
    m_notifyInfoList.append(entryInfo);
    m_currentNotifyInfo = entryInfo;
    // 同一弹窗的通知超过2条，显示折叠数量和折叠效果
    if(m_notifyInfoList.size() > 1) {
        m_flodStatus = true;
        updataWidgetData();
    }
}

bool popupItemWidget::isPopupWidgetEnable()
{
    return m_popupWidgetEnable;
}

/* 开启弹窗退出动画 */
void popupItemWidget::widgetOutAnimation()
{
    m_pOutAnimation->setStartValue(this->geometry());
    m_pOutAnimation->setEndValue(QRect(this->geometry().x() + this->width() + 10,
                                             this->geometry().y(),
                                             this->width(),
                                             this->height()));

    m_pOutAnimation->start();
}

void popupItemWidget::initScreenLockConnect()
{
    QDBusConnection::sessionBus().connect("org.ukui.ScreenSaver",
                                          "/",
                                          "org.ukui.ScreenSaver",
                                          "lock", this, SLOT(screenLock()));
}

void popupItemWidget::initScreenUnLockConnect()
{
    QDBusConnection::sessionBus().connect("org.ukui.ScreenSaver",
                                          "/",
                                          "org.ukui.ScreenSaver",
                                          "unlock", this, SLOT(screenUnLock()));
}

bool popupItemWidget::getScreenLockStates()
{
    QDBusInterface m_quickOperationInterface("org.ukui.ScreenSaver",
                                             "/",
                                             "org.ukui.ScreenSaver",
                                             QDBusConnection::sessionBus());

    QDBusReply<bool> m_bool = m_quickOperationInterface.call("GetLockState");

    return m_bool;
}

bool popupItemWidget::getScreenAppNotify(QString appName)
{
    if (appName.isEmpty()) {
        return false;
    }
    // 初始化控制面板对于通知开关读取
    const QByteArray id_3("org.ukui.control-center.noticeorigin");
    if (QGSettings::isSchemaInstalled(id_3)) {
        QString dynamicPath = QString("%1%2/").arg("/org/ukui/control-center/noticeorigin/").arg(QString(appName));
        const QByteArray id_4(dynamicPath.toUtf8().data());
        QGSettings *m_pAppNotifyGseting = new QGSettings(id_3, id_4, this);
        bool status = m_pAppNotifyGseting->get("showOnScreenlock").toBool();
        delete m_pAppNotifyGseting;
        m_pAppNotifyGseting = NULL;
        return status;
    } else {
        return false;
    }
}

bool popupItemWidget::getShowDetail(QString appName)
{
    if (appName.isEmpty()) {
        return false;
    }
    // 初始化控制面板对于通知开关读取
    const QByteArray id_3("org.ukui.control-center.noticeorigin");
    if (QGSettings::isSchemaInstalled(id_3)) {
        QString dynamicPath = QString("%1%2/").arg("/org/ukui/control-center/noticeorigin/").arg(QString(appName));
        const QByteArray id_4(dynamicPath.toUtf8().data());
        QGSettings *m_pAppNotifyGseting = new QGSettings(id_3, id_4, this);
        bool status = m_pAppNotifyGseting->get("show-detail").toBool();
        delete m_pAppNotifyGseting;
        m_pAppNotifyGseting = NULL;
        return status;
    } else {
        return false;
    }
}

/* 应用主动关闭弹窗 */
void popupItemWidget::closePopupWidget(int id)
{
    Q_UNUSED(id);
    m_notifyTimeout->stop();
    m_maximumTimeout->stop();
    //将所有消息收纳到侧边栏内
    showToSidebar();
    //发射弹窗退出信号
    m_popupWidgetEnable = false;
    emit notificationClosedSignal(m_currentNotifyInfo->id().toInt(), ClosedReason::CLOSEDBYAPP);

    widgetOutAnimation();
}
