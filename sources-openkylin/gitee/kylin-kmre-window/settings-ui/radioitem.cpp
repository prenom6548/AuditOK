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

#include "radioitem.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QEvent>
#include <QToolTip>

RadioItem::RadioItem(QFrame *parent)
    : SettingsItem(parent)
    , m_titleLabel(new QLabel)
    , m_buttonGroup(new QButtonGroup(this))
    , m_btn1(new QRadioButton)
    , m_btn2(new QRadioButton)
{
    m_titleLabel->setWordWrap(true);

    m_buttonGroup->setExclusive(true);// 设置互斥
    m_buttonGroup->addButton(m_btn1);
    m_buttonGroup->addButton(m_btn2);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(30, 5, 30, 5);
    layout->setSpacing(0);
    layout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    layout->addSpacing(30);
    layout->addWidget(m_btn1, 0, Qt::AlignVCenter);
    layout->addSpacing(10);
    layout->addWidget(m_btn2, 0, Qt::AlignVCenter);
    layout->addStretch();

    setLayout(layout);
    setFixedHeight(36);

    m_btn1->installEventFilter(this);
    m_btn2->installEventFilter(this);
    m_btn1->setFocusPolicy(Qt::NoFocus);
    m_btn2->setFocusPolicy(Qt::NoFocus);

    connect(m_buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
}

const QString RadioItem::currentText() const
{
    QList<QAbstractButton*> list = m_buttonGroup->buttons();
    foreach (QAbstractButton *btn, list) {
        if (btn->isChecked()) {
            return btn->text();
        }
    }
}

void RadioItem::onButtonClicked(QAbstractButton *button)
{
    QList<QAbstractButton*> list = m_buttonGroup->buttons();
    foreach (QAbstractButton *btn, list) {
        if (btn->isChecked()) {
            //emit this->sendSelectedMode(btn->text());
        }
    }
}

void RadioItem::setTitle(const QString &title, const QString &name1, bool support1, const QString &name2, bool support2)
{
    m_titleLabel->setText(title);
    m_name1 = name1;
    m_name2 = name2;
    m_btn1->setEnabled(support1);
    m_btn1->setText(name1);
    m_btn2->setEnabled(support2);
    m_btn2->setText(name2);
}

void RadioItem::setTip(const QString &tip1, const QString &tip2)
{
    m_tip1 = tip1;
    m_tip2 = tip2;
}

void RadioItem::setDefaultValue(const QString &value)
{
    blockSignals(true);
    if (value == m_name1) {
        m_btn1->setChecked(true);
    }
    else if (value == m_name2) {
        m_btn2->setChecked(true);
    }
    blockSignals(false);
}

bool RadioItem::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == m_btn1 && event->type() == QEvent::HoverMove) && (m_btn1->geometry().contains(this->mapFromGlobal(QCursor::pos())))) {
        QToolTip::showText(QCursor::pos(), m_tip1);
    }
    else if((watched == m_btn2 && event->type() == QEvent::HoverMove) && m_btn2->geometry().contains(this->mapFromGlobal(QCursor::pos()))) {
        QToolTip::showText(QCursor::pos(), m_tip2);
    }

    return QWidget::eventFilter(watched, event);
}
