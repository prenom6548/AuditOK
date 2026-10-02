/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "radiobuttonwidget.h"

#include <QDebug>
#include <QKeyEvent>
#include <algorithm>

RadioButtonWidget::RadioButtonWidget(const QString title,  QWidget *parent, UkccFrame::BorderRadiusStyle style)
    : UkccFrame(parent, style, true)
{
    m_settingName = new FixLabel(title, this);
    m_buttonGroup = new QButtonGroup(this);
    init();
    m_settingName->setAccessibleName(title);
    this->setAccessibleName(title);
}

RadioButtonWidget::~RadioButtonWidget()
{
}

void RadioButtonWidget::init()
{
    mainLayout = new QHBoxLayout();
    radioLayout = new QHBoxLayout();
    radioLayout->setContentsMargins(0, 0, 0, 0);
    radioLayout->setSpacing(80);
    mainLayout->setContentsMargins(16, 0, 16, 0);
    m_settingName->setFixedWidth(214);
    mainLayout->addWidget(m_settingName);
    mainLayout->setSpacing(16);
    mainLayout->addLayout(radioLayout);
    this->setLayout(mainLayout);
    connect(m_buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &RadioButtonWidget::buttonClicked);
}


void RadioButtonWidget::setTitle(const QString &title)
{
    if (m_settingName) {
        m_settingName->setText(title);
    }
    this->setAccessibleName(title);
}

QString RadioButtonWidget::title() const
{
    if (m_settingName) {
        return m_settingName->text();
    }
    return QString();
}

void RadioButtonWidget::setTitleFixedWidth(int w)
{
    m_settingName->setFixedWidth(w);
}

void RadioButtonWidget::addButton(QRadioButton *button, int id, bool addStretch)
{
    // 互斥交给 QButtonGroup；关闭 autoExclusive，避免方向键移焦时连带选中
    button->setAutoExclusive(false);
    button->installEventFilter(this);
    connect(button, &QAbstractButton::toggled, this, [this](bool) {
        refreshGroupKeyboardNav();
    });

    m_buttonGroup->addButton(button, id);
    radioLayout->addWidget(button, 0, Qt::AlignLeft | Qt::AlignVCenter);
    if (addStretch) {
        radioLayout->addStretch();
    }
    refreshGroupKeyboardNav();
}

QHBoxLayout *RadioButtonWidget::mainHLayout() const
{
    return mainLayout;
}

void RadioButtonWidget::setButtonSpacing(int spacing)
{
    radioLayout->setSpacing(spacing);
}

QButtonGroup *RadioButtonWidget::buttonGroup() const
{
    return m_buttonGroup;
}

void RadioButtonWidget::refreshGroupKeyboardNav()
{
    const QList<QAbstractButton *> buttons = m_buttonGroup->buttons();
    if (buttons.isEmpty()) {
        return;
    }

    QAbstractButton *checked = m_buttonGroup->checkedButton();
    for (QAbstractButton *b : buttons) {
        if (!b) {
            continue;
        }
        b->setAutoExclusive(false);
        const bool tabStop = checked ? (b == checked) : (b == buttons.first());
        b->setFocusPolicy(tabStop ? Qt::StrongFocus : Qt::ClickFocus);
    }

    // 行标题 buddy 跟随当前选中（无选中则首项），读屏 LABELLED_BY 对齐
    if (m_settingName) {
        m_settingName->setBuddy(checked ? checked : buttons.first());
    }
}

void RadioButtonWidget::moveFocusInGroup(QRadioButton *from, int key)
{
    if (!from) {
        return;
    }

    QList<QRadioButton *> peers;
    for (QAbstractButton *b : m_buttonGroup->buttons()) {
        auto *rb = qobject_cast<QRadioButton *>(b);
        if (rb && rb->isVisible() && rb->isEnabled()) {
            peers.append(rb);
        }
    }
    if (peers.size() < 2) {
        return;
    }

    std::sort(peers.begin(), peers.end(), [](QRadioButton *a, QRadioButton *b) {
        const QPoint pa = a->mapToGlobal(QPoint(0, 0));
        const QPoint pb = b->mapToGlobal(QPoint(0, 0));
        if (pa.y() != pb.y()) {
            return pa.y() < pb.y();
        }
        return pa.x() < pb.x();
    });

    const int idx = peers.indexOf(from);
    if (idx < 0) {
        return;
    }

    int next = idx;
    if (key == Qt::Key_Left || key == Qt::Key_Up) {
        next = (idx - 1 + peers.size()) % peers.size();
    } else if (key == Qt::Key_Right || key == Qt::Key_Down) {
        next = (idx + 1) % peers.size();
    } else {
        return;
    }

    peers.at(next)->setFocus(Qt::TabFocusReason);
}

bool RadioButtonWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *btn = qobject_cast<QRadioButton *>(watched);
        if (btn && m_buttonGroup->buttons().contains(btn)) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
                // 与 ThemeButton 一致：方向键只切换焦点，空格/回车或鼠标再选中
                moveFocusInGroup(btn, keyEvent->key());
                return true;
            default:
                break;
            }
        }
    }
    return UkccFrame::eventFilter(watched, event);
}
