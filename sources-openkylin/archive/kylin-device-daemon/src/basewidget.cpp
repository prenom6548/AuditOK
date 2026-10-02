/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "basewidget.h"
#include "config/xatom-helper.h"
#include "fdframe.h"

#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>
#include <QLabel>
#include <QTimer>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>

#define THEME_UKFD_TRANS "org.ukui.control-center.personalise"

baseWidget::baseWidget(QWidget *parent) : QWidget(parent)
{
//    setWindowFlags(Qt::FramelessWindowHint); //Qt::ToolTip设置新窗口是一个提示窗口，没有标题栏和窗口边框,设置后独立与窗口存在
//    setWindowFlags(Qt::Tool);   //设置为tool会导致窗口没有焦点

//   注释掉透明度设置，固定背景透明度，完全透明。
//   initOpacityGSettings();
    m_curTransOpacity = 0;
    installEventFilter(this);

//    根据新版设计说明取消底框上的文字
//    QLabel *tiplabel = new QLabel(this);
//    tiplabel->move(16,16);
//    tiplabel->setFixedWidth(200);
//    tiplabel->setText(tr("removable storage device"));
//    tiplabel->show();

//    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFocusPolicy(Qt::NoFocus);
//    setProperty("useSystemStyleBlur", true);  //毛玻璃效果


    //窗管协议,隐藏上方标题栏
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

#ifdef KY_SDK_WAYLANDHELPER
    // 初始化 UkuiWindowHelper
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    //m_windowHelper->setBlurEffect(QRegion());
    //m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::Notification);
#endif

    initUi();

    m_hideTimer = new QTimer(this);
    connect(m_hideTimer, &QTimer::timeout, [=](){
        qDebug() << "[baseWidget::connect] hide timer timeout";
        this->hide();
        m_hideTimer->stop();
    });
}

baseWidget::~baseWidget()
{
    if (m_gsTransOpacity) {
        delete m_gsTransOpacity;
        m_gsTransOpacity = nullptr;
    }
#ifdef KY_SDK_WAYLANDHELPER
    if (m_windowHelper) {
        delete m_windowHelper;
        m_windowHelper = nullptr;
    }
#endif
}

void baseWidget::initOpacityGSettings()
{
    const QByteArray idtrans(THEME_UKFD_TRANS);
    if(QGSettings::isSchemaInstalled(idtrans)) {
        m_gsTransOpacity = new QGSettings(idtrans);
    }

    if (!m_gsTransOpacity) {
        m_curTransOpacity = 1;
        return;
    }

    connect(m_gsTransOpacity, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "transparency") {
            QStringList keys = m_gsTransOpacity->keys();
            if (keys.contains("transparency")) {
                m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
                repaint();
            }
        }
    });

    QStringList keys = m_gsTransOpacity->keys();
    if(keys.contains("transparency")) {
        m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
    }
}

void baseWidget::setPanelPosition(int pos)
{
    m_panelPosition = pos;
}

void baseWidget::showBaseWindow(){
    updateSize();

    //不在任务栏处显示窗口图标
    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
        const KWindowInfo info(this->winId(), NET::WMState);
        if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
            KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
        }
    }

    kdk::UkuiStyleHelper::self()->removeHeader(this);
    kdk::WindowManager::setSkipTaskBar(this->windowHandle(), true);
    kdk::WindowManager::setSkipSwitcher(this->windowHandle(), true);

    // // 自动设置滑动动效
    // setupSlideAnimation();
    this->show();
}

void baseWidget::paintEvent(QPaintEvent * event)
{
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(m_curTransOpacity);  //设置不透明度
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRoundedRect(this->rect(), 12, 12);
    path.setFillRule(Qt::WindingFill);   //填充规则
    painter.setBrush(this->palette().color(QPalette::Base));
    painter.setPen(Qt::transparent);

    painter.drawPath(path); //绘制底色
    //QFrame::paintEvent(event);
}

bool baseWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this)
    {
        if(event->type() == QEvent::Enter) {
           qDebug() << "[baseWidget::eventFilter] Mouse enter, stopping hide timer";
           if(m_hideTimer->isActive()) {
              m_hideTimer->stop();
           }
           return false;

        } else if (event->type() == QEvent::WindowDeactivate)  {

            qDebug() << "[baseWidget::eventFilter] Window deactivated, hiding widget";
            this->hide();
            return false;
        } /* else if (event->type() == QEvent::Leave) {
            qDebug() << "[baseWidget::eventFilter] Mouse leave, starting hide timer";
            if(m_hideTimer->isActive())
                m_hideTimer->stop();
            m_hideTimer->start(2000);
            return false;
        } */
    }

    if (!isActiveWindow())
    {
        activateWindow();
    }

    return false;


}

void baseWidget::setMaxHeight(int height)
{
    m_maxHeight = height;
    updateSize();
}

void baseWidget::updateSize()
{
    m_contentWidget->adjustSize();
    int contentHeight = m_contentWidget->height();

    if (contentHeight <= m_maxHeight) {
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->setFixedHeight(contentHeight);
        mainWindowVboxLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        this->setFixedHeight(m_maxHeight);
        mainWindowVboxLayout->setContentsMargins(0, 0, 4, 0);
    }
}

void baseWidget::initUi()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        m_maxHeight = screen->availableGeometry().height() * 0.8;
    } else {
        m_maxHeight = 600;
    }

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setAttribute(Qt::WA_TranslucentBackground);
    m_scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical {"
        "    background: transparent;"
        "    width: 4px;"
        "    margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(255, 255, 255, 0.3);"
        "    border-radius: 2px;"
        "    min-height: 30px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: transparent;"
        "}"
    );

    m_contentWidget = new QWidget();
    m_contentWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_contentWidget->setMinimumWidth(384);

    mainWindowVboxLayout = new QVBoxLayout(m_contentWidget);
    mainWindowVboxLayout->setAlignment(Qt::AlignCenter|Qt::AlignBottom);
    mainWindowVboxLayout->setContentsMargins(0,0,4,0);
    mainWindowVboxLayout->setSpacing(7);

    m_scrollArea->setWidget(m_contentWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(m_scrollArea);
    this->setLayout(mainLayout);
    this->adjustSize();
}

void baseWidget::setupWindowEffects()
{
    qDebug() << "UkuiWindowHelper, panelPosition:" << m_panelPosition;
#ifdef KY_SDK_WAYLANDHELPER
    if (!m_windowHelper) return;
    switch (m_panelPosition) { //划入动效
        case 0: m_windowHelper->setSlideEffect(UkuiWindowHelper::Position(3), 0, true); break;   // 从下往上滑出
        case 1: m_windowHelper->setSlideEffect(UkuiWindowHelper::Position(1), 0, true); break;   // 从上往下滑出
        case 2: m_windowHelper->setSlideEffect(UkuiWindowHelper::Position(0), 0, true); break;  // 从左往右滑出
        case 3: m_windowHelper->setSlideEffect(UkuiWindowHelper::Position(2), 0, true); break; // 从右往左滑出
        default: m_windowHelper->setSlideEffect(UkuiWindowHelper::Position(3), 0, true); break;
    }
    //毛玻璃
    //窗管setBlurEffect接口和setSlideEffect接口，必须是先后顺序，否则会影响 划入动画
    // bug#442945 【交互体验捉虫攻关】【U盘管理工具】插入U盘，点击任务栏U盘图标，动效不是从下到上弹出
    m_windowHelper->setBlurEffect(QRegion()); 
#endif
}
