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

#include "karrowtabbar.h"
#include "kborderlessbutton.h"
#include "knativetabbar.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QProxyStyle>

namespace kdk
{
class Q_DECL_HIDDEN ArrowTabBarStyle : public QProxyStyle
{
    Q_OBJECT

public:
    ArrowTabBarStyle(QStyle *style = nullptr);

    QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const override;
};

class Q_DECL_HIDDEN KArrowTabBarPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KArrowTabBar)

public:
    KArrowTabBarPrivate(KArrowTabBar *parent);

    void currentChangeUpdateBtnStatus(int index);

    void updateBtnVisible();

private:
    KArrowTabBar *q_ptr;
    KBorderlessButton *m_pLeftButton;
    KBorderlessButton *m_pRightButton;
    KNativeTabbar *m_pTabBar;
    QHBoxLayout *m_pHLayout;
};

KArrowTabBar::KArrowTabBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KArrowTabBarPrivate(this))
{
    Q_D(KArrowTabBar);
    connect(this, &KArrowTabBar::sizeChange, this, [=] {
        d->updateBtnVisible();
    });
}

KNativeTabbar *KArrowTabBar::tabbar()
{
    Q_D(KArrowTabBar);
    return d->m_pTabBar;
}

void KArrowTabBar::resizeEvent(QResizeEvent *event)
{
    Q_D(KArrowTabBar);
    QWidget::resizeEvent(event);
    emit sizeChange();
}

KArrowTabBarPrivate::KArrowTabBarPrivate(KArrowTabBar *parent)
    : q_ptr(parent)
{
    Q_Q(KArrowTabBar);
    setParent(parent);
    m_pHLayout = new QHBoxLayout(q);
    m_pTabBar = new KNativeTabbar();
    ArrowTabBarStyle *style = new ArrowTabBarStyle();
    m_pTabBar->setStyle(style);
    m_pLeftButton = new KBorderlessButton();
    m_pLeftButton->setIcon(QIcon::fromTheme("go-previous.symbolic"));
    m_pRightButton = new KBorderlessButton();
    m_pRightButton->setIcon(QIcon::fromTheme("go-next.symbolic"));

    m_pHLayout->addWidget(m_pLeftButton);
    m_pHLayout->addWidget(m_pTabBar);
    m_pHLayout->addWidget(m_pRightButton);

    connect(m_pTabBar, &KNativeTabbar::currentChanged, this, [=](int index) {
        currentChangeUpdateBtnStatus(index);
    });

    connect(m_pLeftButton, &KBorderlessButton::clicked, this, [=] {
        m_pTabBar->leftScrollTabs();
    });
    connect(m_pRightButton, &KBorderlessButton::clicked, this, [=] {
        m_pTabBar->rightScrollTabs();
    });
    connect(m_pTabBar, &KNativeTabbar::leftBStatus, q, [=](bool flag) {
        m_pLeftButton->setEnabled(flag);
    });
    connect(m_pTabBar, &KNativeTabbar::rightBStatus, q, [=](bool flag) {
        m_pRightButton->setEnabled(flag);
    });
}

void KArrowTabBarPrivate::currentChangeUpdateBtnStatus(int index)
{
    Q_Q(KArrowTabBar);
    if (index == 0) {
        m_pLeftButton->setEnabled(false);
        if (!m_pRightButton->isEnabled())
            m_pRightButton->setEnabled(true);
    } else if (index == m_pTabBar->count() - 1) {
        m_pRightButton->setEnabled(false);
        if (!m_pLeftButton->isEnabled())
            m_pLeftButton->setEnabled(true);
    } else {
        if (!m_pLeftButton->isEnabled())
            m_pLeftButton->setEnabled(true);
        if (!m_pRightButton->isEnabled())
            m_pRightButton->setEnabled(true);
    }
}

void KArrowTabBarPrivate::updateBtnVisible()
{
    Q_Q(KArrowTabBar);
    int wid = 0;
    for (int i = 0; i < q->tabbar()->count(); i++)
        wid += q->tabbar()->tabRect(i).width();
    if (q->rect().width() < wid) {
        if (m_pTabBar->usesScrollButtons()) {
            m_pLeftButton->setVisible(true);
            m_pRightButton->setVisible(true);
        } else {
            m_pLeftButton->setVisible(false);
            m_pRightButton->setVisible(false);
        }
    } else {
        m_pLeftButton->setVisible(false);
        m_pRightButton->setVisible(false);
    }
}

ArrowTabBarStyle::ArrowTabBarStyle(QStyle *style)
{
}

QRect ArrowTabBarStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rc;
    switch (element) {
    case QStyle::SE_TabBarScrollLeftButton:
        rc = option->rect;
        rc.setWidth(0);
        rc.setHeight(0);
        break;
    case QStyle::SE_TabBarScrollRightButton:
        rc = option->rect;
        rc.setWidth(0);
        rc.setHeight(0);
        break;
    default:
        break;
    }
    return rc;
}

}

#include "karrowtabbar.moc"
#include "moc_karrowtabbar.cpp"
