/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kwindowbuttonbar.h"
//#include "kbubblewidget.h"
#include "parmscontroller.h"
#include "themeController.h"
//#include <QApplication>
//#include <QDebug>
//#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
//#include <QStyleOptionButton>
//#include <QTimer>
#include <QVariant>
#include "kysdk-ukuiwindowhelper/ukuiwindowhelper.h"

namespace kdk
{
//class ScreenButton : public QPushButton
//{
//    Q_OBJECT
//public:
//    ScreenButton(QWidget *parent = nullptr);
//    void setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight);

//protected:
//    void paintEvent(QPaintEvent *event);

//private:
//    int m_topLeftRadius;
//    int m_topRightRadius;
//    int m_bottomLeftRadius;
//    int m_bottomRightRadius;
//};

//class SplitScreenWidget : public QWidget
//{
//    Q_OBJECT
//public:
//    SplitScreenWidget(QWidget *parent = nullptr);

//protected:
//    void paintEvent(QPaintEvent *event);
//};

class KWindowButtonBarPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KWindowButtonBar)

public:
    KWindowButtonBarPrivate(KWindowButtonBar *parent);
//    void initBubbleWidget();

//public Q_SLOTS:
//    void showBubbleWidget(bool check);

protected:
    void changeTheme() override;

private:
    KWindowButtonBar *q_ptr;
    KMenuButton *m_pMenuBtn;
    QPushButton *m_pMinimumBtn;
    QPushButton *m_pmaximumBtn;
    QPushButton *m_pCloseBtn;
    MaximumButtonState m_maximumButtonState;
    QWidget *m_pParentWidget;
    UkuiWindowHelper *m_pUkuiWindowHelper;
//    KBubbleWidget *m_pButtleWidget;
//    ScreenButton *m_pLeftBtn;
//    ScreenButton *m_pRightBtn;
//    ScreenButton *m_pTopLeftBtn;
//    ScreenButton *m_pTopRightBtn;
//    ScreenButton *m_pBottomLeftBtn;
//    ScreenButton *m_pBottomRightBtn;
    QColor m_pixColor;
    QRect m_splitScreenRect;
//    QTimer *m_pTimer;
    bool m_followMode;
//    bool m_mousePress;
};

KWindowButtonBar::KWindowButtonBar(QWidget *parent)
    : QFrame(parent)
    , d_ptr(new KWindowButtonBarPrivate(this))
{
    Q_D(KWindowButtonBar);
    d->m_pParentWidget = parent;
    d->m_pMenuBtn->setFocusPolicy(Qt::NoFocus);
    d->m_pMinimumBtn->setFocusPolicy(Qt::NoFocus);
    d->m_pmaximumBtn->setFocusPolicy(Qt::NoFocus);
    d->m_pCloseBtn->setFocusPolicy(Qt::NoFocus);
    setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
    //    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    d->m_pCloseBtn->installEventFilter(this);
    d->m_pmaximumBtn->installEventFilter(this);
    d->m_pMinimumBtn->installEventFilter(this);
//    d->m_pButtleWidget->installEventFilter(this);
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
        d->m_pMenuBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pmaximumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pCloseBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pMinimumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        updateGeometry();
        repaint();
    });
//    d->m_pTimer = new QTimer(this);
//    d->m_pTimer->setSingleShot(true);
//    connect(d->m_pTimer, &QTimer::timeout, this, [=] {
//        QPoint globalPos = QCursor::pos();
//        QPoint btnTopLeft = d->m_pmaximumBtn->mapToGlobal(d->m_pmaximumBtn->rect().topLeft());
//        QRect btnGlobalRect(btnTopLeft, d->m_pmaximumBtn->rect().size());

//        if (d->m_mousePress || btnGlobalRect.contains(globalPos))
//            d->m_pButtleWidget->show();
//    });
//    connect(d->m_pLeftBtn, &ScreenButton::clicked, this, [=] {
//        d->m_pButtleWidget->hide();
//    });
//    connect(d->m_pRightBtn, &ScreenButton::clicked, this, [=] {
//        d->m_pButtleWidget->hide();
//    });
//    connect(d->m_pTopLeftBtn, &ScreenButton::clicked, this, [=] {
//        d->m_pButtleWidget->hide();
//    });
//    connect(d->m_pTopRightBtn, &ScreenButton::clicked, this, [=] {
//        d->m_pButtleWidget->hide();
//    });
//    connect(d->m_pBottomLeftBtn, &ScreenButton::clicked, this, [=] {
//        d->m_pButtleWidget->hide();
//    });
//    connect(d->m_pBottomRightBtn, &ScreenButton::clicked, this, [=] {
//        d->m_pButtleWidget->hide();
//    });
}

KWindowButtonBar::~KWindowButtonBar()
{
}

QPushButton *KWindowButtonBar::minimumButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pMinimumBtn;
}

QPushButton *KWindowButtonBar::maximumButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pmaximumBtn;
}

QPushButton *KWindowButtonBar::closeButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pCloseBtn;
}

KMenuButton *KWindowButtonBar::menuButton()
{
    Q_D(KWindowButtonBar);
    return d->m_pMenuBtn;
}

MaximumButtonState KWindowButtonBar::maximumButtonState()
{
    Q_D(KWindowButtonBar);
    return d->m_maximumButtonState;
}

void KWindowButtonBar::setMaximumButtonState(MaximumButtonState state)
{
    Q_D(KWindowButtonBar);
    d->m_maximumButtonState = state;
    d->changeTheme();
}

void KWindowButtonBar::setFollowMode(bool flag)
{
    Q_D(KWindowButtonBar);
    d->m_followMode = flag;
}

bool KWindowButtonBar::followMode()
{
    Q_D(KWindowButtonBar);
    return d->m_followMode;
}

void KWindowButtonBar::setSplitScreenGeometry(QRect rect)
{
    Q_D(KWindowButtonBar);
    d->m_splitScreenRect = rect;
}

void KWindowButtonBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(KWindowButtonBar);
    if (event->button() == Qt::LeftButton && d->m_pmaximumBtn->isEnabled())
        Q_EMIT doubleClick();
}

bool KWindowButtonBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KWindowButtonBar);
    if (QEvent::WindowDeactivate == event->type()) {
        d->m_pixColor = QColor(179, 179, 179);
        auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
        auto minIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-minimize-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
        d->m_pCloseBtn->setIcon(closeIcon);
        d->m_pMinimumBtn->setIcon(minIcon);

        if (d->m_maximumButtonState == Maximum) {
            auto maxIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-maximize-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
            d->m_pmaximumBtn->setIcon(maxIcon);
            //            d->m_pmaximumBtn->setToolTip(tr("Maximize"));
        } else {
            auto maxIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-restore-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
            d->m_pmaximumBtn->setIcon(maxIcon);
            //            d->m_pmaximumBtn->setToolTip(tr("Restore"));
        }
    }
    if (QEvent::WindowActivate == event->type() || QEvent::MouseButtonPress == event->type()) {
        if (ThemeController::themeMode() == LightTheme)
            d->m_pixColor = QColor(31, 32, 34);
        else
            d->m_pixColor = QColor(255, 255, 255);
        auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
        d->m_pCloseBtn->setIcon(closeIcon);
        d->m_pMinimumBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

        if (d->m_maximumButtonState == Maximum) {
            d->m_pmaximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            //            d->m_pmaximumBtn->setToolTip(tr("Maximize"));
        } else {
            d->m_pmaximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
            //            d->m_pmaximumBtn->setToolTip(tr("Restore"));
        }
    }
    if (watched == d->m_pCloseBtn) {
        // 根据不同状态重绘icon颜色
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (isEnabled()) {
                auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton)
                    d->m_pixColor = QColor(255, 255, 255);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::Enter:
            if (isEnabled()) {
                d->m_pixColor = QColor(255, 255, 255);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::MouseButtonRelease:
            if (isEnabled()) {
                auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
                if (mouseEvent->button() == Qt::LeftButton)
                    d->m_pixColor = QColor(255, 255, 255);
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::Leave:
            if (isEnabled()) {
                if (d->m_pCloseBtn->isActiveWindow()) {
                    if (ThemeController::themeMode() == LightTheme)
                        d->m_pixColor = QColor(31, 32, 34);
                    else
                        d->m_pixColor = QColor(255, 255, 255);
                } else {
                    d->m_pixColor = QColor(179, 179, 179);
                }
                auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
                d->m_pCloseBtn->setIcon(closeIcon);
            }
            break;
        case QEvent::WindowDeactivate: {
            d->m_pixColor = QColor(179, 179, 179);
            auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), d->m_pixColor);
            d->m_pCloseBtn->setIcon(closeIcon);
        } break;
        default:
            break;
        }
    }

//    QPoint globalPos = QCursor::pos();
//    QPoint btnTopLeft = d->m_pmaximumBtn->mapToGlobal(d->m_pmaximumBtn->rect().topLeft());
//    QRect btnGlobalRect(btnTopLeft, d->m_pmaximumBtn->rect().size());
//    QRect bubbleRect = d->m_pButtleWidget->frameGeometry();

    if (watched == d->m_pmaximumBtn)
    {
        switch (event->type())
        {
        case QEvent::HoverMove:
        case QEvent::HoverEnter:
            {
            d->m_pUkuiWindowHelper->showTileFlyout(d->m_splitScreenRect);
//            QPoint point = d->m_pmaximumBtn->mapToGlobal(QPoint(0, 0));
//            d->m_pButtleWidget->move(point.x() - d->m_pButtleWidget->width() / 2 + d->m_pmaximumBtn->width() / 2, point.y() + d->m_pmaximumBtn->height());
//            //            d->m_pButtleWidget->show();
//            d->m_pTimer->start(1000);
//            break;
            }
//        case QEvent::Leave:
//            {
//            if (!btnGlobalRect.contains(globalPos) && !bubbleRect.contains(globalPos))
//                d->m_pButtleWidget->hide();
//            break;
//            }
//        case QEvent::MouseButtonPress:
//            {
//            d->m_pTimer->start(3000);
//            d->m_mousePress = true;
//            break;
//            }
//        case QEvent::MouseButtonRelease:
//            {
//            d->m_mousePress = false;
//            break;
//            }
        }
    }
//    else
//    {
//        if (watched == d->m_pButtleWidget) {
//            switch (event->type()) {
//            case QEvent::Leave:
//            case QEvent::WindowDeactivate:
//                if (d->m_pButtleWidget->isVisible())
//                    d->m_pButtleWidget->hide();
//                break;
//            default:
//                break;
//            }
//        } else {
//            switch (event->type()) {
//            case QEvent::HoverMove:
//            case QEvent::HoverEnter: {
//                if (d->m_pButtleWidget->isVisible())
//                    d->m_pButtleWidget->hide();
//            }
//            }
//        }
//    }
    return QFrame::eventFilter(watched, event);
}

QSize KWindowButtonBar::sizeHint() const
{
    auto size = QFrame::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_PushButtonHeight));
    return size;
}

KWindowButtonBarPrivate::KWindowButtonBarPrivate(KWindowButtonBar *parent)
    : q_ptr(parent)
    , m_followMode(true)
//    , m_mousePress(false)
{
    Q_Q(KWindowButtonBar);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(8);
    hLayout->setContentsMargins(0, 0, 0, 0);

    // m_maximumButtonState = Maximum;

    m_pMenuBtn = new KMenuButton(q);
    m_pMenuBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));

    m_pMinimumBtn = new QPushButton(q);
    m_pMinimumBtn->setToolTip(tr("Minimize"));
    m_pMinimumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));

    m_pmaximumBtn = new QPushButton(q);
    m_pmaximumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));

    m_pCloseBtn = new QPushButton(q);
    m_pCloseBtn->setObjectName("CloseButton");
    m_pCloseBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
    m_pCloseBtn->setToolTip(tr("Close"));

    hLayout->setContentsMargins(0, 0, 4, 0);
    hLayout->setSpacing(2);
    hLayout->addStretch();
    hLayout->addWidget(m_pMenuBtn);
    //    hLayout->addSpacing(4);
    hLayout->addWidget(m_pMinimumBtn);
    //    hLayout->addSpacing(4);
    hLayout->addWidget(m_pmaximumBtn);
    //    hLayout->addSpacing(4);
    hLayout->addWidget(m_pCloseBtn);
    //    hLayout->addSpacing(4);
    q->setLayout(hLayout);

    // 控件自己控制样式、响应主题变化
    m_pMinimumBtn->setProperty("isWindowButton", 0x1);
    m_pMinimumBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pMinimumBtn->setFlat(true);
    m_pMinimumBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

    m_pmaximumBtn->setProperty("isWindowButton", 0x1);
    m_pmaximumBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pmaximumBtn->setFlat(true);

    m_pCloseBtn->setProperty("isWindowButton", 0x02);
    m_pCloseBtn->setProperty("useIconHighlightEffect", 0x08);
    m_pCloseBtn->setFlat(true);
    m_pCloseBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

//    initBubbleWidget();
    m_pUkuiWindowHelper = new UkuiWindowHelper(q);

    changeTheme();
    connect(m_gsetting, &QGSettings::changed, this, [=]() {
        changeTheme();
    });
    connect(m_pmaximumBtn, &QPushButton::clicked, this, [=]() {
        if (m_maximumButtonState == Maximum)
            m_maximumButtonState = Restore;
        else
            m_maximumButtonState = Maximum;
        changeTheme();
    });
    setParent(parent);
}

//void KWindowButtonBarPrivate::initBubbleWidget()
//{
//    Q_Q(KWindowButtonBar);

//    int buttonRadius = 4;
//    int widgetRadius = 6;

//    m_pButtleWidget = new KBubbleWidget(q);
//    m_pButtleWidget->setWindowFlag(Qt::ToolTip);
//    m_pButtleWidget->setTailPosition(TailDirection::TopDirection);
//    m_pButtleWidget->setTailSize(QSize(16, 8));
//    m_pButtleWidget->setOpacity(0.4);
//    m_pButtleWidget->setEnableBlur(true);
//    m_pButtleWidget->setBorderRadius(widgetRadius);
//    m_pButtleWidget->setFixedSize(204, 80);
//    m_pButtleWidget->hide();

//    QHBoxLayout *layout = new QHBoxLayout(m_pButtleWidget);
//    SplitScreenWidget *widget1 = new SplitScreenWidget(m_pButtleWidget);
//    m_pLeftBtn = new ScreenButton(widget1);
//    m_pLeftBtn->setBorderRadius(buttonRadius, buttonRadius, 0, 0);
//    m_pRightBtn = new ScreenButton(widget1);
//    m_pRightBtn->setBorderRadius(0, 0, buttonRadius, buttonRadius);
//    QHBoxLayout *layout1 = new QHBoxLayout(widget1);
//    layout1->setContentsMargins(4, 4, 4, 4);
//    layout1->setSpacing(2);
//    layout1->addWidget(m_pLeftBtn);
//    layout1->addWidget(m_pRightBtn);

//    SplitScreenWidget *widget2 = new SplitScreenWidget(m_pButtleWidget);
//    m_pTopLeftBtn = new ScreenButton(widget2);
//    m_pTopLeftBtn->setBorderRadius(0, buttonRadius, 0, 0);
//    m_pTopRightBtn = new ScreenButton(widget2);
//    m_pTopRightBtn->setBorderRadius(0, 0, buttonRadius, 0);
//    m_pBottomLeftBtn = new ScreenButton(widget2);
//    m_pBottomLeftBtn->setBorderRadius(buttonRadius, 0, 0, 0);
//    m_pBottomRightBtn = new ScreenButton(widget2);
//    m_pBottomRightBtn->setBorderRadius(0, 0, 0, buttonRadius);

//    QHBoxLayout *layout2 = new QHBoxLayout();
//    layout2->setContentsMargins(0, 0, 0, 0);
//    layout2->setSpacing(2);
//    layout2->addWidget(m_pTopLeftBtn);
//    layout2->addWidget(m_pTopRightBtn);
//    QHBoxLayout *layout3 = new QHBoxLayout();
//    layout3->setContentsMargins(0, 0, 0, 0);
//    layout3->setSpacing(2);
//    layout3->addWidget(m_pBottomLeftBtn);
//    layout3->addWidget(m_pBottomRightBtn);
//    QVBoxLayout *layout4 = new QVBoxLayout(widget2);
//    layout4->setContentsMargins(4, 4, 4, 4);
//    layout4->setSpacing(2);
//    layout4->addLayout(layout2);
//    layout4->addLayout(layout3);

//    layout->setContentsMargins(8, 8, 8, 8);
//    layout->setSpacing(8);
//    layout->addWidget(widget1);
//    layout->addWidget(widget2);
//}

//void KWindowButtonBarPrivate::showBubbleWidget(bool check)
//{
//    Q_Q(KWindowButtonBar);
//    if (check)
//        m_pButtleWidget->show();
//    else
//        m_pButtleWidget->hide();
//}

void KWindowButtonBarPrivate::changeTheme()
{
    Q_Q(KWindowButtonBar);
    initThemeStyle();
    if (q->isActiveWindow()) {
        if (q->isEnabled()) {
            if (ThemeController::themeMode() == LightTheme)
                m_pixColor = QColor(31, 32, 34);
            else
                m_pixColor = QColor(255, 255, 255);
            auto closeIcon = ThemeController::drawColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize))), m_pixColor);
            m_pCloseBtn->setIcon(closeIcon);
        }
        if (m_maximumButtonState == Maximum) {
            m_pmaximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            //                m_pmaximumBtn->setToolTip(tr("Maximize"));
        } else {
            m_pmaximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
            //                m_pmaximumBtn->setToolTip(tr("Restore"));
        }
    }
}

//SplitScreenWidget::SplitScreenWidget(QWidget *parent)
//    : QWidget(parent)
//{
//}

//void SplitScreenWidget::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::HighQualityAntialiasing);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::TextAntialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.setPen(Qt::black);
//    painter.setBrush(Qt::white);
//    painter.drawRoundedRect(rect() /*.adjusted(2,2,-2,-2)*/, 6, 6);
//}

//ScreenButton::ScreenButton(QWidget *parent)
//    : QPushButton(parent)
//{
//    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    m_topLeftRadius = 4;
//    m_topRightRadius = 4;
//    m_bottomLeftRadius = 4;
//    m_bottomRightRadius = 4;
//}

//void ScreenButton::setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight)
//{
//    m_topLeftRadius = topLeft;
//    m_topRightRadius = topRight;
//    m_bottomLeftRadius = bottomLeft;
//    m_bottomRightRadius = bottomRight;
//}

//void ScreenButton::paintEvent(QPaintEvent *event)
//{
//    QStyleOptionButton option;
//    initStyleOption(&option);
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::HighQualityAntialiasing);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setRenderHint(QPainter::TextAntialiasing);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.setPen(Qt::NoPen);

//    QRect rect = this->rect();

//    QPainterPath path;
//    path.moveTo(rect.topLeft() + QPointF(m_topLeftRadius, 0));
//    path.lineTo(rect.topRight() - QPointF(m_topRightRadius, 0));
//    path.quadTo(rect.topRight(), rect.topRight() + QPointF(0, m_topRightRadius));
//    path.lineTo(rect.bottomRight() - QPointF(0, m_bottomRightRadius));
//    path.quadTo(rect.bottomRight(), rect.bottomRight() - QPointF(m_bottomRightRadius, 0));
//    path.lineTo(rect.bottomLeft() + QPointF(m_bottomLeftRadius, 0));
//    path.quadTo(rect.bottomLeft(), rect.bottomLeft() - QPointF(0, m_bottomLeftRadius));
//    path.lineTo(rect.topLeft() + QPointF(0, m_topLeftRadius));
//    path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(m_topLeftRadius, 0));

//    QColor bkgColor;
//    QString str("kcomponent-alpha-normal");
//    char *m_char = str.toLocal8Bit().data();
//    if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QColor>()) {
//        bkgColor = qApp->property(m_char).value<QColor>();
//    }

//    if (!option.state.testFlag(QStyle::State_Enabled)) {
//        bkgColor = option.palette.color(QPalette::Disabled, QPalette::WindowText);
//    } else {
//        if (option.state.testFlag(QStyle::State_MouseOver)) {
//            if (option.state.testFlag(QStyle::State_Sunken)) {
//                QString str("kcomponent-alpha-click");
//                char *m_char = str.toLocal8Bit().data();
//                if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QColor>()) {
//                    bkgColor = qApp->property(m_char).value<QColor>();
//                }
//            } else {
//                QString str("kcomponent-alpha-hover");
//                char *m_char = str.toLocal8Bit().data();
//                if (qApp->property(m_char).isValid() && qApp->property(m_char).canConvert<QColor>()) {
//                    bkgColor = qApp->property(m_char).value<QColor>();
//                }
//            }
//        }
//    }
//    painter.setBrush(bkgColor);
//    painter.drawPath(path);
//}

}
#include "kwindowbuttonbar.moc"
#include "moc_kwindowbuttonbar.cpp"
