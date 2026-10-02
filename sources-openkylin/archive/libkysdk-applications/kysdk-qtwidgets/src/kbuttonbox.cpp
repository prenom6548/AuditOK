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

#include "kbuttonbox.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QButtonGroup>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStyleOptionButton>
#include <QStylePainter>
#include "accessinfohelper.h"

namespace kdk
{
class KButtonBoxPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KButtonBox)
public:
    KButtonBoxPrivate(KButtonBox *parent);
    void updateBorderRadius();
    void updateButtonList();

protected:
    void changeTheme();

private:
    KButtonBox *q_ptr;
    QButtonGroup *m_buttonGroup;
    QBoxLayout *m_layout;
    int m_radius;
    bool m_isCheckable;
    bool m_shadowFlag;
    QList<KPushButton *> m_buttonList;
    QGraphicsDropShadowEffect *m_pShadow_effect;
};

KButtonBoxPrivate::KButtonBoxPrivate(KButtonBox *parent)
    : q_ptr(parent)
{
    Q_Q(KButtonBox);
    setParent(parent);
    m_buttonGroup = new QButtonGroup(q);
    q->connect(m_buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), q, SIGNAL(buttonClicked(QAbstractButton *)));
    q->connect(m_buttonGroup, SIGNAL(buttonPressed(QAbstractButton *)), q, SIGNAL(buttonPressed(QAbstractButton *)));
    q->connect(m_buttonGroup, SIGNAL(buttonReleased(QAbstractButton *)), q, SIGNAL(buttonReleased(QAbstractButton *)));
    q->connect(m_buttonGroup, SIGNAL(buttonToggled(QAbstractButton *, bool)), q, SIGNAL(buttonToggled(QAbstractButton *, bool)));
    q->connect(m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            changeTheme();
        }
    });

    m_layout = new QHBoxLayout(q);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (m_radius == -1)
        m_radius = 6;
    m_isCheckable = false;
    m_shadowFlag = false;
    m_pShadow_effect = new QGraphicsDropShadowEffect();

    KDK_OBJ_INFO_FORMAT(m_buttonGroup);
    KDK_OBJ_INFO_FORMAT(m_pShadow_effect);
}

void KButtonBoxPrivate::updateBorderRadius()
{
    Q_Q(KButtonBox);
    QList<KPushButton *> buttonList = q->buttonList();
    switch (q->orientation()) {
    case Qt::Horizontal: {
        for (int i = 0; i < buttonList.count(); i++) {
            KPushButton *button = buttonList.at(i);
            if (i == 0) {
                Q_ASSERT(button);
                button->setBorderRadius(m_radius, m_radius, 0, 0);
            } else if (i == buttonList.count() - 1) {
                Q_ASSERT(button);
                button->setBorderRadius(0, 0, m_radius, m_radius);
            } else {
                Q_ASSERT(button);
                button->setBorderRadius(0);
            }
        }
        break;
    }
    case Qt::Vertical: {
        for (int i = 0; i < buttonList.count(); i++) {
            KPushButton *button = buttonList.at(i);
            if (i == 0) {
                Q_ASSERT(button);
                button->setBorderRadius(0, m_radius, m_radius, 0);
            } else if (i == buttonList.count() - 1) {
                Q_ASSERT(button);
                button->setBorderRadius(m_radius, 0, 0, m_radius);
            } else {
                Q_ASSERT(button);
                button->setBorderRadius(0);
            }
        }
        break;
    }
    default:
        break;
    }
    q->update();
}

void KButtonBoxPrivate::updateButtonList()
{
    Q_Q(KButtonBox);

    QColor shadowColor;
    if (ThemeController::themeMode() == LightTheme)
        shadowColor = ThemeController::getCustomColorFromDT("kblack");
    else
        shadowColor = ThemeController::getCustomColorFromDT("kwhite");
    shadowColor.setAlphaF(0.1);
    for (QAbstractButton *button : m_buttonGroup->buttons()) {
        m_buttonGroup->removeButton(button);
        m_layout->removeWidget(button);
    }
    for (int i = 0; i < m_buttonList.count(); ++i) {
        KPushButton *button = m_buttonList.at(i);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_layout->insertWidget(i, button);
        m_buttonGroup->addButton(button, i);
        button->setCheckable(m_isCheckable);
        if (m_shadowFlag) {
            m_pShadow_effect->setOffset(0, 2);
            m_pShadow_effect->setColor(shadowColor);
            m_pShadow_effect->setBlurRadius(4);
            q->setGraphicsEffect(m_pShadow_effect);
        }
    }
    updateBorderRadius();
}

void KButtonBoxPrivate::changeTheme()
{
    Q_Q(KButtonBox);
    initThemeStyle();

    if (ThemeController::themeMode() == LightTheme) {
        QColor shadowColor(ThemeController::getCustomColorFromDT("kblack"));
        shadowColor.setAlphaF(0.1);
        if (m_shadowFlag) {
            m_pShadow_effect->setOffset(0, 2);
            m_pShadow_effect->setColor(shadowColor);
            m_pShadow_effect->setBlurRadius(4);
            q->setGraphicsEffect(m_pShadow_effect);
        }
    } else {
        QColor shadowColor(255, 255, 255);
        shadowColor.setAlphaF(0.1);
        if (m_shadowFlag) {
            m_pShadow_effect->setOffset(0, 2);
            m_pShadow_effect->setColor(shadowColor);
            m_pShadow_effect->setBlurRadius(4);
            q->setGraphicsEffect(m_pShadow_effect);
        }
    }
}

KButtonBox::KButtonBox(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KButtonBoxPrivate(this))
{
    Q_D(KButtonBox);
}

KButtonBox::~KButtonBox()
{
}

Qt::Orientation KButtonBox::orientation()
{
    Q_D(KButtonBox);
    QBoxLayout::Direction layoutDirection = d->m_layout->direction();
    if (layoutDirection == QBoxLayout::LeftToRight || layoutDirection == QBoxLayout::RightToLeft) {
        return Qt::Horizontal;
    }
    return Qt::Vertical;
}

void KButtonBox::setOrientation(Qt::Orientation orientation)
{
    Q_D(KButtonBox);
    if (orientation == Qt::Vertical) {
        d->m_layout->setDirection(QBoxLayout::TopToBottom);
        d->updateBorderRadius();
        return;
    }
    d->m_layout->setDirection(QBoxLayout::LeftToRight);
    d->updateBorderRadius();
}

void KButtonBox::addButton(KPushButton *button, int i)
{
    Q_D(KButtonBox);
    if (i < -1)
        return;

    if (i == -1 || i >= d->m_buttonList.count()) {
        d->m_buttonList.append(button);
    } else {
        d->m_buttonList.insert(i, button);
    }

    button->show();

    setButtonList(d->m_buttonList);
}

void KButtonBox::removeButton(KPushButton *button)
{
    Q_D(KButtonBox);
    if (d->m_buttonList.contains(button)) {
        d->m_buttonList.removeAll(button);
        button->hide();
    }
    setButtonList(d->m_buttonList);
}

void KButtonBox::removeButton(int i)
{
    Q_D(KButtonBox);
    if (i < 0 || i >= d->m_buttonList.count())
        return;
    auto button = d->m_buttonList.at(i);
    if (button)
        button->hide();
    d->m_buttonList.removeAt(i);
    setButtonList(d->m_buttonList);
}

void KButtonBox::setButtonList(const QList<KPushButton *> &list)
{
    Q_D(KButtonBox);
    d->m_buttonList = list;
    d->updateButtonList();
}

QList<KPushButton *> KButtonBox::buttonList()
{
    Q_D(KButtonBox);
    return d->m_buttonList;
}

void KButtonBox::setBorderRadius(int radius)
{
    Q_D(KButtonBox);
    d->m_radius = radius;
    update();
}

int KButtonBox::borderRadius()
{
    Q_D(KButtonBox);
    return d->m_radius;
}

void KButtonBox::setId(KPushButton *button, int id)
{
    Q_D(KButtonBox);
    d->m_buttonGroup->setId(button, id);
}

int KButtonBox::id(KPushButton *button)
{
    Q_D(KButtonBox);
    return d->m_buttonGroup->id(button);
}

KPushButton *KButtonBox::checkedButton()
{
    Q_D(KButtonBox);
    KPushButton *button = dynamic_cast<KPushButton *>(d->m_buttonGroup->checkedButton());
    return button;
}

KPushButton *KButtonBox::button(int id)
{
    Q_D(KButtonBox);
    KPushButton *button = dynamic_cast<KPushButton *>(d->m_buttonGroup->button(id));
    return button;
}

int KButtonBox::checkedId()
{
    Q_D(KButtonBox);
    return d->m_buttonGroup->checkedId();
}

void KButtonBox::setExclusive(bool exclusive)
{
    Q_D(KButtonBox);
    d->m_buttonGroup->setExclusive(exclusive);
}

bool KButtonBox::exclusive()
{
    Q_D(KButtonBox);
    return d->m_buttonGroup->exclusive();
}

void KButtonBox::setCheckable(bool flag)
{
    Q_D(KButtonBox);
    d->m_isCheckable = flag;
    QList<KPushButton *> list = buttonList();
    for (int i = 0; i < list.count(); ++i) {
        KPushButton *button = list.at(i);
        button->setCheckable(d->m_isCheckable);
    }
    update();
}

bool KButtonBox::isCheckable()
{
    Q_D(KButtonBox);
    return d->m_isCheckable;
}

void KButtonBox::setShadow(bool flag)
{
    Q_D(KButtonBox);
    QColor shadowColor(0, 0, 0);
    shadowColor.setAlphaF(0.1);
    d->m_shadowFlag = flag;
    if (flag) {
        d->updateButtonList();
    } else {
        for (int i = 0; i < d->m_buttonList.count(); ++i) {
            KPushButton *button = d->m_buttonList.at(i);
            d->m_pShadow_effect->setOffset(0, 2);
            d->m_pShadow_effect->setColor(Qt::NoBrush);
            d->m_pShadow_effect->setBlurRadius(4);
            button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
            setGraphicsEffect(d->m_pShadow_effect);
        }
    }
}

bool KButtonBox::shadow()
{
    Q_D(KButtonBox);
    return d->m_shadowFlag;
}

}
#include "kbuttonbox.moc"
#include "moc_kbuttonbox.cpp"
