/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#include "settingscrollcontent.h"

#include <QResizeEvent>
#include <QScroller>
#include <QScrollBar>
#include <QScrollArea>
#include <QApplication>
#include <QWheelEvent>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QHBoxLayout>

SettingScrollContent::SettingScrollContent(QWidget *parent)
    : QWidget(parent),
      m_content(nullptr),
      m_speedPeriod(2)
{
//    this->setStyleSheet("QWidget{background:palette(Base);}");//QPalette::Base    QPalette::Window        border-radius:6px;

    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setContentsMargins(30, 2, 24, 40);

    m_title = new QLabel;
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
//    m_title->setVisible(false);
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->installEventFilter(this);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    m_scrollArea->setContentsMargins(0, 0, 0, 0);
    m_scrollArea->viewport()->installEventFilter(this);
    QScroller::grabGesture(m_scrollArea, QScroller::LeftMouseButtonGesture);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(m_title);
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addLayout(titleLayout);
    centralLayout->addWidget(m_scrollArea);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    this->setLayout(centralLayout);
    this->initAnimation();
}

void SettingScrollContent::initAnimation()
{
    m_scrollAnimation = new QPropertyAnimation(m_scrollArea->verticalScrollBar(), "value");
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuint);
    m_scrollAnimation->setDuration(1500);

    m_wheelAnimation = new QPropertyAnimation(m_scrollArea->verticalScrollBar(), "value");
    m_wheelAnimation->setEasingCurve(QEasingCurve::OutQuint);
    m_wheelAnimation->setDuration(1500);

    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, [=] {
        m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuint);
        m_scrollAnimation->setDuration(1500);
    });

    connect(m_wheelAnimation, &QPropertyAnimation::finished, this, [=] {
        m_wheelAnimation->setEasingCurve(QEasingCurve::OutQuint);
        m_wheelAnimation->setDuration(1500);
    });
}



QWidget *SettingScrollContent::setContent(QWidget * const widget)
{
    QWidget *lastWidget = m_content;
    if (lastWidget) {
        lastWidget->removeEventFilter(this);
    }

    m_content = widget;
    m_content->installEventFilter(this);
    m_scrollArea->setWidget(m_content);

    return lastWidget;
}

bool SettingScrollContent::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_content) {
        return false;
    }

    if (m_content && watched == m_scrollArea && event->type() == QEvent::Resize) {
        m_content->setFixedWidth(static_cast<QResizeEvent *>(event)->size().width());
    }

    if (m_content && watched == m_scrollArea->viewport() && event->type() == QEvent::Wheel) {
        const QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
        QWheelEvent *newEvent =  new QWheelEvent(wheel->pos(), wheel->delta(), wheel->buttons(), wheel->modifiers(), wheel->orientation());
        qApp->postEvent(this, newEvent);
        return true;
    }

    if (watched == m_content && event->type() == QEvent::LayoutRequest) {
        if (m_content->hasHeightForWidth()) {
            m_content->setMinimumHeight(m_content->heightForWidth(m_content->width()));
        }
        else {
            m_content->setFixedHeight(m_content->layout()->sizeHint().height());
        }
    }

    return false;
}

void SettingScrollContent::stopScroll()
{
    m_speedPeriod = 2;
    m_scrollAnimation->stop();
    m_wheelAnimation->stop();
}

void SettingScrollContent::wheelEvent(QWheelEvent *e)
{
    int offset = - e->delta();

    if (m_wheelAnimation->state() == QPropertyAnimation::Running) {
        m_speedPeriod += 0.02;
    }
    else {
        m_speedPeriod = 2;
    }

    m_scrollAnimation->stop();
    m_wheelAnimation->stop();
    m_wheelAnimation->setStartValue(m_scrollArea->verticalScrollBar()->value());
    m_wheelAnimation->setEndValue(m_scrollArea->verticalScrollBar()->value() + offset * qMin(int(m_speedPeriod), 14));
    m_wheelAnimation->start();
}

void SettingScrollContent::mousePressEvent(QMouseEvent *e)
{
    stopScroll();
    QWidget::mousePressEvent(e);
}
