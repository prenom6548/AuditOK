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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kloopplaypage.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedLayout>
#include <QTimer>

namespace kdk
{

class Q_DECL_HIDDEN CircleButton : public QPushButton
{
    Q_OBJECT
public:
    enum ButtonType {
        CircleType,
        NormalType
    };

    CircleButton(QWidget *parent = nullptr);

    void setButtonType(ButtonType type);

protected:
    void paintEvent(QPaintEvent *event);

private:
    ButtonType m_type;
};

class Q_DECL_HIDDEN Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);

Q_SIGNALS:
    void leftClick();

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
};

class Q_DECL_HIDDEN KLoopPlayPagePrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KLoopPlayPage)
public:
    KLoopPlayPagePrivate(KLoopPlayPage *parent);

    void updateWidget(int i);
    void updateButton();
    void updateShowPosition();

private:
    KLoopPlayPage *q_ptr;
    Widget *m_pWidget1;
    Widget *m_pWidget2;
    Widget *m_pWidget3;
    QWidget *m_pScrollWidget;
    QHBoxLayout *m_pWidgetLayout;
    QHBoxLayout *m_pButtonLayout;
    QVBoxLayout *m_pLayout;
    QList<QWidget *> m_WidgetList;
    CircleButton *m_pCircleButton;
    QList<CircleButton *> m_ButtonList;
    QScrollArea *m_pScrollArea;
    int m_index;
    int m_spacing;
    int m_timer;
    QTimer *m_pTimer;
};

KLoopPlayPagePrivate::KLoopPlayPagePrivate(KLoopPlayPage *parent)
    : q_ptr(parent)
    , m_index(0)
    , m_spacing(24)
    , m_timer(3000)
{
    Q_Q(KLoopPlayPage);
    setParent(parent);
    m_pLayout = new QVBoxLayout(q);
    m_pLayout->setSpacing(16);

    m_pScrollWidget = new QWidget();
    m_pScrollWidget->setContentsMargins(0, 0, 0, 0);

    m_pScrollArea = new QScrollArea(q);
    m_pScrollArea->setFrameStyle(QFrame::NoFrame);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScrollArea->setWidget(m_pScrollWidget);

    m_pWidgetLayout = new QHBoxLayout(m_pScrollWidget);
    m_pWidgetLayout->setSpacing(0);

    m_pButtonLayout = new QHBoxLayout(q);
    m_pButtonLayout->setSpacing(6);

    m_pLayout->addWidget(m_pScrollArea);
    m_pLayout->addLayout(m_pButtonLayout);

    m_pTimer = new QTimer(q);
    m_pTimer->start(m_timer);
    connect(m_pTimer, &QTimer::timeout, this, [=] {
        q->setCurrentWidget(q->currentWidget() + 1);
    });
}

void KLoopPlayPagePrivate::updateWidget(int i)
{
    Q_Q(KLoopPlayPage);
    if (m_WidgetList.count() < 3)
        return;

    QLayoutItem *child;
    while ((child = m_pWidgetLayout->takeAt(0)) != 0) {
        // 删除Stretch(弹簧)等布局
        if (child->spacerItem()) {
            m_pWidgetLayout->removeItem(child);
            continue;
        }
        // 删除布局
        m_pWidgetLayout->removeWidget(child->widget());
        child->widget()->setParent(nullptr);
        delete child;
        child = nullptr;
    }

    int prev;
    int next;
    if (i == 0) {
        prev = m_WidgetList.count() - 1;
        next = i + 1;
    } else if (i == m_WidgetList.count() - 1) {
        prev = i - 1;
        next = 0;
    } else {
        prev = i - 1;
        next = i + 1;
    }

    m_pWidget1 = new Widget();
    m_pWidget2 = new Widget();
    m_pWidget3 = new Widget();

    m_pWidgetLayout->addWidget(m_pWidget1);
    m_pWidgetLayout->addSpacing(m_spacing);
    m_pWidgetLayout->addWidget(m_pWidget2);
    m_pWidgetLayout->addSpacing(m_spacing);
    m_pWidgetLayout->addWidget(m_pWidget3);
    m_pWidgetLayout->setContentsMargins(8, 8, 8, 18);

    QHBoxLayout *w1layout = new QHBoxLayout(m_pWidget1);
    w1layout->setContentsMargins(0, 0, 0, 0);
    w1layout->setSpacing(0);
    w1layout->addWidget(m_WidgetList.at(prev));

    QHBoxLayout *w2layout = new QHBoxLayout(m_pWidget2);
    w2layout->setContentsMargins(0, 0, 0, 0);
    w2layout->setSpacing(0);
    w2layout->addWidget(m_WidgetList.at(i));

    QHBoxLayout *w3layout = new QHBoxLayout(m_pWidget3);
    w3layout->setContentsMargins(0, 0, 0, 0);
    w3layout->setSpacing(0);
    w3layout->addWidget(m_WidgetList.at(next));

    updateShowPosition();

    connect(m_pWidget1, &Widget::leftClick, q, [=] {
        q->setCurrentWidget(prev);
        m_pTimer->start(m_timer);
        emit q->leftClick();
    });
    connect(m_pWidget2, &Widget::leftClick, q, [=] {
        emit q->centerClick();
    });
    connect(m_pWidget3, &Widget::leftClick, q, [=] {
        q->setCurrentWidget(next);
        m_pTimer->start(m_timer);
        emit q->rightClick();
    });
    updateButton();
}

void KLoopPlayPagePrivate::updateButton()
{
    Q_Q(KLoopPlayPage);
    m_ButtonList.clear();
    QLayoutItem *child;
    while ((child = m_pButtonLayout->takeAt(0)) != 0) {
        // 删除Stretch(弹簧)等布局
        if (child->spacerItem()) {
            m_pButtonLayout->removeItem(child);
            continue;
        }
        // 删除布局
        m_pButtonLayout->removeWidget(child->widget());
        child->widget()->setParent(nullptr);
        delete child;
        child = nullptr;
    }
    m_pButtonLayout->addStretch();
    m_pButtonLayout->setSpacing(8);
    for (int i = 0; i < m_WidgetList.count(); i++) {
        CircleButton *circleButton = new CircleButton();
        m_ButtonList.append(circleButton);
        m_pCircleButton = new CircleButton();
        if (i == q->currentWidget()) {
            m_pCircleButton->setFixedSize(12, 6);
            m_pCircleButton->setButtonType(CircleButton::NormalType);
        } else {
            m_pCircleButton->setFixedSize(6, 6);
            m_pCircleButton->setButtonType(CircleButton::CircleType);
        }
        m_pButtonLayout->addWidget(m_pCircleButton);
        connect(m_pCircleButton, &CircleButton::clicked, this, [=] {
            q->setCurrentWidget(i);
            m_pTimer->start(m_timer);
        });
    }
    m_pButtonLayout->addStretch();
}

void KLoopPlayPagePrivate::updateShowPosition()
{
    Q_Q(KLoopPlayPage);
    int viewportWidth = m_pScrollArea->viewport()->width();
    int scrollValue = (m_pScrollWidget->width() - viewportWidth) / 2;
    if (scrollValue < 0)
        scrollValue = 0;
    m_pScrollArea->horizontalScrollBar()->setValue(scrollValue);
}

KLoopPlayPage::KLoopPlayPage(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KLoopPlayPagePrivate(this))
{
    Q_D(KLoopPlayPage);
}

void KLoopPlayPage::addWidget(QWidget *widget)
{
    Q_D(KLoopPlayPage);
    if (!d->m_WidgetList.contains(widget))
        d->m_WidgetList.append(widget);
    d->updateWidget(d->m_index);
}

void KLoopPlayPage::removeWidget(QWidget *widget)
{
    Q_D(KLoopPlayPage);
    if (d->m_WidgetList.contains(widget))
        d->m_WidgetList.removeOne(widget);
    d->updateWidget(d->m_index);
}

void KLoopPlayPage::setCurrentWidget(int index)
{
    Q_D(KLoopPlayPage);
    if (index >= d->m_WidgetList.count())
        d->m_index = 0;
    else if (index < 0)
        d->m_index = d->m_WidgetList.count() - 1;
    else
        d->m_index = index;
    d->updateWidget(d->m_index);
}

int KLoopPlayPage::currentWidget()
{
    Q_D(KLoopPlayPage);
    return d->m_index;
}

void KLoopPlayPage::setWidgetSpacing(int spacing)
{
    Q_D(KLoopPlayPage);
    d->m_spacing = spacing;
}

void KLoopPlayPage::setLoopTime(int time)
{
    Q_D(KLoopPlayPage);
    d->m_timer = time;
    d->m_pTimer->start(time);
}

void KLoopPlayPage::resizeEvent(QResizeEvent *event)
{
    Q_D(KLoopPlayPage);
    QWidget::resizeEvent(event);
    d->updateShowPosition();
}

void KLoopPlayPage::paintEvent(QPaintEvent *event)
{
    Q_D(KLoopPlayPage);
    QWidget::paintEvent(event);
}

void KLoopPlayPage::showEvent(QShowEvent *event)
{
    Q_D(KLoopPlayPage);
    QWidget::showEvent(event);
    d->updateShowPosition();
}

Widget::Widget(QWidget *parent)
{
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit leftClick();
    }
    return QWidget::mousePressEvent(event);
}

void Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

CircleButton::CircleButton(QWidget *parent)
{
    m_type = ButtonType::CircleType;
}

void CircleButton::setButtonType(CircleButton::ButtonType type)
{
    m_type = type;
    update();
}

void CircleButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setPen(Qt::NoPen);

    QColor color(18, 18, 18);
    color.setAlphaF(0.2);
    painter.setBrush(color);

    switch (m_type) {
    case ButtonType::CircleType:
        painter.drawEllipse(rect());
        break;
    case ButtonType::NormalType:
        painter.drawRoundedRect(rect(), 3, 3);
        break;
    default:
        break;
    }
}

}

#include "kloopplaypage.moc"
#include "moc_kloopplaypage.cpp"
