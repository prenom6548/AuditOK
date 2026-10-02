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
#include "parmscontroller.h"
#include "themeController.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QTimer>
#include <QVariant>
#include "kysdk-ukuiwindowhelper/ukuiwindowhelper.h"
#include "accessinfohelper.h"

namespace kdk
{

class KWindowButtonBarPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KWindowButtonBar)

public:
    KWindowButtonBarPrivate(KWindowButtonBar *parent);

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
    QColor m_pixColor;
    QRect m_splitScreenRect;
    bool m_followMode;
    QTimer *m_enterTimer;
    bool m_splitScreen;
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
    d->m_pmaximumBtn->installEventFilter(this);


    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=]() {
        setFixedHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_IconbarHeight));
        d->m_pMenuBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pmaximumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pCloseBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        d->m_pMinimumBtn->setFixedSize(Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize), Parmscontroller::parm(Parmscontroller::Parm::PM_WindowButtonBarSize));
        updateGeometry();
        repaint();
    });
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

    if (watched == d->m_pmaximumBtn)
    {
        switch (event->type())
        {
        case QEvent::Enter:
        {
            d->m_splitScreen = false;
            d->m_enterTimer->start();
            break;
        }
        case QEvent::Leave:
        {
            d->m_splitScreen = false;
            d->m_enterTimer->stop();
            break;
        }
        }
    }
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
    , m_splitScreen(false)
{
    Q_Q(KWindowButtonBar);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(8);
    hLayout->setContentsMargins(0, 0, 0, 0);

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

    hLayout->setAlignment(Qt::AlignRight);
    hLayout->setContentsMargins(0, 0, 4, 0);
    hLayout->setSpacing(2);
//    hLayout->addStretch();
    hLayout->addWidget(m_pMenuBtn);
    hLayout->addWidget(m_pMinimumBtn);
    hLayout->addWidget(m_pmaximumBtn);
    hLayout->addWidget(m_pCloseBtn);
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

    m_pUkuiWindowHelper = new UkuiWindowHelper(q->window());
    m_enterTimer = new QTimer(q);
    m_enterTimer->setSingleShot(true);
    m_enterTimer->setInterval(500);

    changeTheme();
    connect(m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            changeTheme();
        }
    });
    connect(m_pmaximumBtn, &QPushButton::clicked, this, [=]() {
        if (m_maximumButtonState == Maximum)
            m_maximumButtonState = Restore;
        else
            m_maximumButtonState = Maximum;
        changeTheme();
    });
    connect(m_enterTimer, &QTimer::timeout, this, [=]{
        if (!m_splitScreen) {
            m_splitScreen = true;
            QPoint pos = m_pmaximumBtn->mapTo(m_pmaximumBtn->window(), QPoint(0, 0));
            m_pUkuiWindowHelper->showTileFlyout(QRect(pos, m_pmaximumBtn->size()));
        }
    });
    setParent(parent);

    KDK_ALL_INFO_FORMAT(m_pMenuBtn,"");
    KDK_ALL_INFO_FORMAT(m_pMinimumBtn,"");
    KDK_ALL_INFO_FORMAT(m_pmaximumBtn,"");
    KDK_ALL_INFO_FORMAT(m_pCloseBtn,"");
}

void KWindowButtonBarPrivate::changeTheme()
{
    Q_Q(KWindowButtonBar);
    initThemeStyle();
    if(m_maximumButtonState == Maximum)
    {
        m_pmaximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    }
    else
    {
        m_pmaximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    }
}

}
#include "kwindowbuttonbar.moc"
#include "moc_kwindowbuttonbar.cpp"
