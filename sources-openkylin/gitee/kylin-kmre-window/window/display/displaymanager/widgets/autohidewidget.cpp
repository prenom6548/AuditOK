/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "autohidewidget.h"
#include "common.h"
#include "utils.h"
#include "kmreenv.h"
#include "preferences.h"
#include "sessionsettings.h"

#include <QApplication>
#include <QPushButton>
#include <QTimer>
#include <QEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QStyleOption>
#include <QHelpEvent>
#include <sys/syslog.h>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

AutohideWidget::AutohideWidget(QString pkgName, QWidget * parent)
	: QWidget(parent)
    , mPkgName(pkgName)
    , m_turnedOn(false)
    , m_autoHide(false)
    , m_useAnimation(true)
	, spacing(0)
    , m_percWidth(100)
    , m_activationArea(Bottom)
    , m_timer(0)
#if QT_VERSION >= 0x040600
    , m_animation(0)
#endif
    , m_width(450)
{
//    this->setStyleSheet("QWidget{background-color:transparent;}");
    this->setBackgroundRole(QPalette::Base);//this->setBackgroundRole(QPalette::Window);
    this->setAutoFillBackground(true);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setLayoutDirection(Qt::LeftToRight);
    this->setFixedHeight(DEFAULT_TITLEBAR_HEIGHT);
    this->setFocusPolicy(Qt::NoFocus);//this->setAttribute(Qt::WA_ShowWithoutActivating, true);
    this->setMouseTracking(true);

#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || \
    defined(__i386) || defined(__i386__) || defined(__i686) || defined(__i686__)
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        this->setAttribute(Qt::WA_NativeWindow);
    }
#endif

	parent->installEventFilter(this);
	installFilter(parent);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkUnderMouse()));
    m_timer->setInterval(3000);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(0);
	layout->setMargin(0);
	setLayout(layout);

    this->initButtons();
    this->updateUi();
    this->initConnections();
    //this->adjustSize();
}

AutohideWidget::~AutohideWidget()
{
#if QT_VERSION >= 0x040600
    if (m_animation) delete m_animation;
#endif
}

void AutohideWidget::initButtons()
{
    QWidget *w = new QWidget;
    QHBoxLayout *m_centralLayout = new QHBoxLayout(w);
    m_centralLayout->setContentsMargins(8, 0, 8, 0);
    m_centralLayout->setSpacing(BUTTON_SPACE);

    m_backBtn = new QPushButton;
    m_backBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_backBtn->setFlat(true);
    m_backBtn->setToolTip(tr("Back"));
    m_backBtn->setProperty("useIconHighlightEffect", 0x2);
    m_backBtn->setProperty("isWindowButton", 0x01);
    m_backBtn->setIcon(QIcon::fromTheme("go-previous-symbolic"));

    m_gameKeyBtn = new QPushButton;
    m_gameKeyBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_gameKeyBtn->setFlat(true);
    m_gameKeyBtn->setToolTip(tr("Game Key"));
    m_gameKeyBtn->setProperty("useIconHighlightEffect", 0x2);
    m_gameKeyBtn->setProperty("isWindowButton", 0x01);
    m_gameKeyBtn->setIcon(QIcon::fromTheme("document-properties-symbolic"));

    m_unFullscreenBtn = new QPushButton;
    m_unFullscreenBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_unFullscreenBtn->setFlat(true);
    m_unFullscreenBtn->setToolTip(tr("UnFullscreen"));
    m_unFullscreenBtn->setProperty("useIconHighlightEffect", 0x2);
    m_unFullscreenBtn->setProperty("isWindowButton", 0x01);
    m_unFullscreenBtn->setIcon(QIcon::fromTheme("view-restore-symbolic"));

//    m_minBtn = new QPushButton;
//    m_minBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
//    m_minBtn->setFlat(true);
//    m_minBtn->setToolTip(tr("Minimize"));
//    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
//    m_minBtn->setProperty("isWindowButton", 0x01);
//    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

    m_closeBtn = new QPushButton;
    m_closeBtn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("Close"));
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    m_centralLayout->addWidget(m_backBtn);
    m_centralLayout->addStretch();
    m_centralLayout->addWidget(m_gameKeyBtn);
    m_centralLayout->addWidget(m_unFullscreenBtn);
//    m_centralLayout->addWidget(m_minBtn);
    m_centralLayout->addWidget(m_closeBtn);

    m_backBtn->setFocusPolicy(Qt::NoFocus);
    m_gameKeyBtn->setFocusPolicy(Qt::NoFocus);
    m_unFullscreenBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);

    layout()->addWidget(w);
}

void AutohideWidget::initConnections()
{
    connect(m_backBtn, &QPushButton::clicked, this, &AutohideWidget::sigBack);
    connect(m_gameKeyBtn, &QPushButton::clicked, this, [=] () {
        emit this->requestSettings(QPoint(m_gameKeyBtn->geometry().center()), true);
    });
    connect(m_unFullscreenBtn, &QPushButton::clicked, this, &AutohideWidget::sigExitFullscreen);
//    connect(m_minBtn, &QPushButton::clicked, this, [=] () {
//        emit this->sigMiniSize();
//    });
    connect(m_closeBtn, &QPushButton::clicked, this, &AutohideWidget::sigClose);
}

void AutohideWidget::updateUi()
{
    KmreConfig::Feature *feature = KmreConfig::Feature::getInstance();

    m_backBtn->setVisible(feature->isEnabled(mPkgName, "back"));
    m_gameKeyBtn->setVisible(feature->isEnabled(mPkgName, "game_key"));
    m_unFullscreenBtn->setVisible(feature->isEnabled(mPkgName, "full"));
    m_closeBtn->setVisible(feature->isEnabled(mPkgName, "close"));
}

void AutohideWidget::enableGameAction(bool b)
{
    m_gameKeyBtn->setVisible(KmreConfig::Feature::getInstance()->isEnabled(mPkgName, "game_key") && b);
}

void AutohideWidget::setHideDelay(int ms)
{
    m_timer->setInterval(ms);
}

int AutohideWidget::hideDelay()
{
    return m_timer->interval();
}

void AutohideWidget::installFilter(QObject *o)
{
	QObjectList children = o->children();
	for (int n=0; n < children.count(); n++) {
		if (children[n]->isWidgetType()) {
			QWidget *w = static_cast<QWidget *>(children[n]);
			w->setMouseTracking(true);
			w->installEventFilter(this);
			installFilter(children[n]);
		}
	}
}

void AutohideWidget::activate()
{
    m_turnedOn = true;
    m_timer->start();
}

void AutohideWidget::deactivate()
{
    m_turnedOn = false;
    m_timer->stop();
	hide();
}

void AutohideWidget::show()
{
	resizeAndMove();

    QWidget::raise();// avoid to override by main window
    if (m_useAnimation) {
		showAnimated();
    }
    else {
		QWidget::show();
	}

    // Restart m_timer
    if (m_timer->isActive()) m_timer->start();
}

void AutohideWidget::setAutoHide(bool b)
{
    m_autoHide = b;
}

void AutohideWidget::checkUnderMouse()
{
    if (m_autoHide) {
        if ((isVisible()) && (!underMouse()))  {
            hide();
            emit sigForceRedraw();//请求刷新一次数据，避免静态画面下隐藏的标题栏不消失
        }
    }
}

void AutohideWidget::resizeAndMove()
{
    QWidget *widget = parentWidget();
    /*int w = widget->width() * m_percWidth / 100;
	int h = height();
    resize(w, h);*/

    /*int x = (widget->width() - width() ) / 2;
    int y = widget->height() - height() - spacing;
    move(x, y);*/

    /*int w = widget->width();
    int h = height();
    resize(w, h);
    move(0, 0);*/

    int w = m_width;
    int h = height();
    resize(w, h);
    int x = (widget->width() - width()) / 2;
    int y = 0;
    move(x, y);
}

bool AutohideWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (m_turnedOn) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            emit sigExitFullscreen();
        }
		else if (event->type() == QEvent::MouseMove) {
			if (!isVisible()) {
                if (m_activationArea == Anywhere) {
					show();
                }
                else {
                    QMouseEvent *mouse_event = dynamic_cast<QMouseEvent*>(event);
                    QWidget *parent = parentWidget();
					QPoint p = parent->mapFromGlobal(mouse_event->globalPos());
//					if (p.y() > (parent->height() - height() - spacing)) {
//						show();
//					}
                    if (p.y() <= 0) {
                        show();
                    }
//                    if (p.y() < height()) {
//                        show();
//                    }
                }
			}
		}
	}

	return QWidget::eventFilter(obj, event);
}

void AutohideWidget::showAnimated()
{
#if QT_VERSION >= 0x040600
    if (!m_animation) {
        m_animation = new QPropertyAnimation(this, "pos");
	}

    QPoint initial_position = QPoint(pos().x(), 0/*parentWidget()->size().height()*/);
	QPoint final_position = pos();
	move(initial_position);

	QWidget::show();

    m_animation->setDuration(300);
    m_animation->setEasingCurve(QEasingCurve::OutBounce);
    m_animation->setEndValue(final_position);
    m_animation->setStartValue(initial_position);
    m_animation->start();
#else
	QWidget::show();
#endif
}

