/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "ktoolbar.h"
#include "klineframe.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QPainter>

namespace kdk
{

class KToolBarPrivate : public QObject, public ThemeController
{
    Q_DECLARE_PUBLIC(KToolBar)
    Q_OBJECT
public:
    KToolBarPrivate(KToolBar *parent);

    void init();
    void updateButtonLayout();
    void clearButtonLayout();
    void getSelectList(KToolButton *button, bool checked);

private:
    KToolBar *q_ptr;
    QBoxLayout *m_pLayout;
    QList<KToolButton *> m_pButtonList;
    QList<KToolButton *> m_pCheckList;
    int m_radius;
    bool m_exclusive;
    bool m_isCheckable;
    bool m_isLineVisible;
    bool m_isBkgColorEnable;
};

KToolBar::KToolBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KToolBarPrivate(this))
{
}

Qt::Orientation KToolBar::orientation()
{
    Q_D(KToolBar);
    QBoxLayout::Direction layoutDirection = d->m_pLayout->direction();
    if (layoutDirection == QBoxLayout::LeftToRight || layoutDirection == QBoxLayout::RightToLeft) {
        return Qt::Horizontal;
    }
    return Qt::Vertical;
}

void KToolBar::setOrientation(Qt::Orientation orientation)
{
    Q_D(KToolBar);
    if (orientation == Qt::Vertical)
        d->m_pLayout->setDirection(QBoxLayout::TopToBottom);
    else
        d->m_pLayout->setDirection(QBoxLayout::LeftToRight);
    d->updateButtonLayout();
}

KToolButton *KToolBar::addButton(QIcon icon)
{
    Q_D(KToolBar);
    KToolButton *button = new KToolButton(this);
    button->setType(KToolButtonType::Frameless);
    button->setIcon(icon);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->m_pButtonList.append(button);
    d->updateButtonLayout();
    return button;
}

KToolButton *KToolBar::addButton(QString string)
{
    Q_D(KToolBar);
    KToolButton *button = new KToolButton(this);
    button->setType(KToolButtonType::Frameless);
    button->setText(string);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->m_pButtonList.append(button);
    d->updateButtonLayout();
    return button;
}

QList<KToolButton *> KToolBar::addButton(QList<QIcon> list)
{
    Q_D(KToolBar);
    QList<KToolButton *> btnList;
    for (int i = 0; i < list.count(); i++) {
        KToolButton *button = new KToolButton(this);
        button->setType(KToolButtonType::Frameless);
        button->setIcon(list.at(i));
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        d->m_pButtonList.append(button);
        btnList.append(button);
    }
    d->updateButtonLayout();
    return btnList;
}

QList<KToolButton *> KToolBar::addButton(QList<QString> list)
{
    Q_D(KToolBar);
    QList<KToolButton *> btnList;
    for (int i = 0; i < list.count(); i++) {
        KToolButton *button = new KToolButton(this);
        button->setType(KToolButtonType::Frameless);
        button->setText(list.at(i));
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        d->m_pButtonList.append(button);
        btnList.append(button);
    }
    d->updateButtonLayout();
    return btnList;
}

// void KToolBar::addButton(KToolButton *button)
//{
//     Q_D(KToolBar);
//     if(!d->m_pButtonList.contains(button))
//     {
//         d->m_pButtonList.append(button);
//         d->updateButtonLayout();
//     }
// }

// void KToolBar::addButton(QList<KToolButton *> list)
//{
//     Q_D(KToolBar);
//     for(int i = 0 ; i < list.count() ; i++)
//     {
//         if(!d->m_pButtonList.contains(list.at(i)))
//         {
//             d->m_pButtonList.append(list.at(i));
//         }
//     }
//     d->updateButtonLayout();
// }

QList<KToolButton *> KToolBar::buttonList()
{
    Q_D(KToolBar);
    return d->m_pButtonList;
}

void KToolBar::removeButton(KToolButton *button)
{
    Q_D(KToolBar);
    if (d->m_pButtonList.contains(button)) {
        d->m_pButtonList.removeAll(button);
    }
    d->updateButtonLayout();
}

void KToolBar::removeButton(int id)
{
    Q_D(KToolBar);
    if (id < 0 || id >= d->m_pButtonList.count())
        return;
    d->m_pButtonList.removeAt(id);
    d->updateButtonLayout();
}

void KToolBar::removeButton(QList<KToolButton *> list)
{
    Q_D(KToolBar);
    for (int i = 0; i < list.count(); i++) {
        if (d->m_pButtonList.contains(list.at(i))) {
            d->m_pButtonList.removeAll(list.at(i));
        }
    }
    d->updateButtonLayout();
}

void KToolBar::setBorderRadius(int radius)
{
    Q_D(KToolBar);
    d->m_radius = radius;
}

int KToolBar::borderRadius()
{
    Q_D(KToolBar);
    return d->m_radius;
}

void KToolBar::setExclusive(bool exclusive)
{
    Q_D(KToolBar);
    d->m_exclusive = exclusive;
}

bool KToolBar::exclusive()
{
    Q_D(KToolBar);
    return d->m_exclusive;
}

void KToolBar::setCheckable(bool flag)
{
    Q_D(KToolBar);
    d->m_isCheckable = flag;
    QList<KToolButton *> list = buttonList();
    for (int i = 0; i < list.count(); ++i) {
        KToolButton *button = list.at(i);
        button->setCheckable(d->m_isCheckable);
    }
    update();
}

bool KToolBar::isCheckable()
{
    Q_D(KToolBar);
    return d->m_isCheckable;
}

QList<KToolButton *> KToolBar::checkedButton()
{
    Q_D(KToolBar);
    return d->m_pCheckList;
}

void KToolBar::setLineVisible(bool flag)
{
    Q_D(KToolBar);
    d->m_isLineVisible = flag;
    d->updateButtonLayout();
}

bool KToolBar::isLineVisible()
{
    Q_D(KToolBar);
    return d->m_isLineVisible;
}

void KToolBar::setBackgroundColorEnabled(bool flag)
{
    Q_D(KToolBar);
    d->m_isBkgColorEnable = flag;
    d->updateButtonLayout();
}

bool KToolBar::backgroundColorEnabled()
{
    Q_D(KToolBar);
    return d->m_isBkgColorEnable;
}

void KToolBar::paintEvent(QPaintEvent *event)
{
    Q_D(KToolBar);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (d->m_isBkgColorEnable)
        painter.setBrush(ThemeController::getPaletteColorFromDT("button"));
    else
        painter.setBrush(Qt::transparent);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(this->rect(), d->m_radius, d->m_radius);
    QWidget::paintEvent(event);
}

KToolBarPrivate::KToolBarPrivate(KToolBar *parent)
    : q_ptr(parent)
{
    Q_Q(KToolBar);
    init();
}

void KToolBarPrivate::init()
{
    Q_Q(KToolBar);
    m_pLayout = new QHBoxLayout(q);
    m_pLayout->setMargin(0);
    m_pLayout->setSpacing(0);
    m_pLayout->setDirection(QBoxLayout::LeftToRight);
    m_radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (m_radius == -1)
        m_radius = 6;
    m_exclusive = false;
    m_isCheckable = false;
    m_isLineVisible = false;
    m_isBkgColorEnable = false;
}

void KToolBarPrivate::updateButtonLayout()
{
    Q_Q(KToolBar);
    clearButtonLayout();
    for (int i = 0; i < m_pButtonList.count(); i++) {
        if (m_isBkgColorEnable)
            m_pButtonList.at(i)->setBackgroundColorEnabled(true);
        else
            m_pButtonList.at(i)->setBackgroundColorEnabled(false);

        m_pButtonList.at(i)->setBorderRadius(m_radius, m_radius, m_radius, m_radius);

        m_pLayout->addWidget(m_pButtonList.at(i));
        q->setCheckable(q->isCheckable());

        if (m_pButtonList.at(i)->menu())
            m_pButtonList.at(i)->setArrow(true);

        connect(m_pButtonList.at(i), &KToolButton::checkedChanged, this, [=](bool checked) {
            getSelectList(m_pButtonList.at(i), checked);
        });

        if (i != m_pButtonList.count() - 1 && m_isLineVisible) {
            if (q->orientation() == Qt::Vertical) {
                KHLineFrame *lineFrame = new KHLineFrame(q);
                lineFrame->setFollowPalette(false);
                QPalette pale;
                QColor color(0, 0, 0);
                color.setAlphaF(0.1);
                pale.setColor(QPalette::Window, color);
                lineFrame->setPalette(pale);
                m_pLayout->addWidget(lineFrame);
            } else {
                KVLineFrame *lineFrame = new KVLineFrame(q);
                lineFrame->setFollowPalette(false);
                QPalette pale;
                QColor color(0, 0, 0);
                color.setAlphaF(0.1);
                pale.setColor(QPalette::Window, color);
                lineFrame->setPalette(pale);
                m_pLayout->addWidget(lineFrame);
            }
        }
    }
}

void KToolBarPrivate::clearButtonLayout()
{
    for (int i = 0; i < m_pButtonList.count(); i++) {
        disconnect(m_pButtonList.at(i), SIGNAL(checkedChanged(bool)), 0, 0);
    }

    QLayoutItem *child;
    while ((child = m_pLayout->takeAt(0)) != 0) {
        if (child->spacerItem()) {
            m_pLayout->removeItem(child);
            continue;
        }
        m_pLayout->removeWidget(child->widget());
        child->widget()->setParent(nullptr);
        delete child;
        child = nullptr;
    }
}

void KToolBarPrivate::getSelectList(KToolButton *button, bool checked)
{
    m_pCheckList.clear();
    if (m_exclusive) {
        if (checked) {
            for (int i = 0; i < m_pButtonList.count(); i++) {

                if (button == m_pButtonList.at(i)) {
                    button->setChecked(checked);
                } else
                    m_pButtonList.at(i)->setChecked(false);
            }

        } else {
            button->setChecked(checked);
        }
    } else {
        for (int i = 0; i < m_pButtonList.count(); i++) {
            if (button == m_pButtonList.at(i)) {
                button->setChecked(checked);
            } else
                m_pButtonList.at(i)->setChecked(m_pButtonList.at(i)->isChecked());
        }
    }

    for (int i = 0; i < m_pButtonList.count(); i++) {
        if (m_pButtonList.at(i)->isChecked())
            m_pCheckList.append(m_pButtonList.at(i));
    }
}

}

#include "ktoolbar.moc"
#include "moc_ktoolbar.cpp"
