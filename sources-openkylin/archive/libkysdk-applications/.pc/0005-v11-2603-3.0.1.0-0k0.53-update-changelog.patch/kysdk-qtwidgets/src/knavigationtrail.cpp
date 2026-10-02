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

#include "knavigationtrail.h"
#include "kborderlessbutton.h"
#include "kcrumb.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>

namespace kdk
{

class Q_DECL_HIDDEN KNavigationTrailPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KNavigationTrail)

public:
    KNavigationTrailPrivate(KNavigationTrail *parent);

    void updateLayout();

private:
    KNavigationTrail *q_ptr;
    KBorderlessButton *m_pLeftButton;
    KBorderlessButton *m_pRightButton;
    KCrumb *m_pCrumb;
    QHBoxLayout *m_pHLayout;
    ButtonPosition m_position;
};

KNavigationTrail::KNavigationTrail(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KNavigationTrailPrivate(this))
{
    Q_D(KNavigationTrail);
}

KCrumb *KNavigationTrail::crumb()
{
    Q_D(KNavigationTrail);
    return d->m_pCrumb;
}

void KNavigationTrail::setPosition(ButtonPosition type)
{
    Q_D(KNavigationTrail);
    d->m_position = type;
    d->updateLayout();
}

void KNavigationTrail::resizeEvent(QResizeEvent *event)
{
    Q_D(KNavigationTrail);
    QWidget::resizeEvent(event);
    emit sizeChange();
}

KNavigationTrailPrivate::KNavigationTrailPrivate(KNavigationTrail *parent)
    : q_ptr(parent)
{
    Q_Q(KNavigationTrail);
    m_position = ButtonPosition::LeftOrRight;
    m_pHLayout = new QHBoxLayout(q);
    m_pCrumb = new KCrumb();
    m_pLeftButton = new KBorderlessButton();
    m_pLeftButton->setIcon(QIcon::fromTheme("go-previous.symbolic"));
    m_pRightButton = new KBorderlessButton();
    m_pRightButton->setIcon(QIcon::fromTheme("go-next.symbolic"));

    m_pHLayout->addWidget(m_pLeftButton);
    m_pHLayout->addWidget(m_pRightButton);
    m_pHLayout->addWidget(m_pCrumb);

    connect(m_pCrumb, &KCrumb::currentChanged, this, [=](int index) {
        if (index == 0) {
            m_pLeftButton->setEnabled(false);
            if (!m_pRightButton->isEnabled())
                m_pRightButton->setEnabled(true);
        } else if (index == m_pCrumb->count() - 1) {
            m_pRightButton->setEnabled(false);
            if (!m_pLeftButton->isEnabled())
                m_pLeftButton->setEnabled(true);
        } else {
            if (!m_pLeftButton->isEnabled())
                m_pLeftButton->setEnabled(true);
            if (!m_pRightButton->isEnabled())
                m_pRightButton->setEnabled(true);
        }
    });

    connect(m_pLeftButton, &KBorderlessButton::clicked, this, [=] {
        //        m_pCrumb->leftScroll();
        m_pCrumb->setCurrentIndex(m_pCrumb->currentIndex() - 1);
    });
    connect(m_pRightButton, &KBorderlessButton::clicked, this, [=] {
        //        m_pCrumb->rightScroll();
        m_pCrumb->setCurrentIndex(m_pCrumb->currentIndex() + 1);
    });
}

void KNavigationTrailPrivate::updateLayout()
{
    Q_Q(KNavigationTrail);
    QLayoutItem *child;
    while ((child = m_pHLayout->takeAt(0)) != 0) {
        // 删除Stretch(弹簧)等布局
        if (child->spacerItem()) {
            m_pHLayout->removeItem(child);
            continue;
        }
        // 删除布局
        m_pHLayout->removeWidget(child->widget());
        child->widget()->setParent(nullptr);
        delete child;
        child = nullptr;
    }

    switch (m_position) {
    case ButtonPosition::LeftPosition:
        m_pHLayout->addWidget(m_pLeftButton);
        m_pHLayout->addWidget(m_pRightButton);
        m_pHLayout->addWidget(m_pCrumb);
        break;
    case ButtonPosition::LeftOrRight:
        m_pHLayout->addWidget(m_pLeftButton);
        m_pHLayout->addWidget(m_pCrumb);
        m_pHLayout->addWidget(m_pRightButton);
        break;
    case ButtonPosition::RightPosition:
        m_pHLayout->addWidget(m_pCrumb);
        m_pHLayout->addWidget(m_pLeftButton);
        m_pHLayout->addWidget(m_pRightButton);
        break;
    default:
        break;
    }
}

}

#include "knavigationtrail.moc"
#include "moc_knavigationtrail.cpp"
