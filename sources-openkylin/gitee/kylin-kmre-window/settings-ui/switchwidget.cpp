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

#include "switchwidget.h"
#include "switchbutton.h"

#include <QLabel>
#include <QBoxLayout>

SwitchWidget::SwitchWidget(QFrame *parent):
    SettingsItem(parent),
    m_iconLabel(new QLabel),
    m_nameLabel(new QLabel),
    m_switchBtn(new SwitchButton)
{
    //this->setStyleSheet("QFrame:hover{background: palette(Highlight); border-radius: 4px;}");//this->setStyleSheet("QFrame:hover{background-color: rgba(55,144,250,0.30);border-radius: 4px;}");
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 0, 10, 0);

    m_iconLabel->setFixedSize(24, 24);

    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignVCenter);
    mainLayout->addWidget(m_nameLabel, 0, Qt::AlignVCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(m_switchBtn, 0, Qt::AlignVCenter);

    setFixedHeight(36);
    setLayout(mainLayout);

    connect(m_switchBtn, &SwitchButton::checkedChanged, this, &SwitchWidget::checkedChanged);
}

void SwitchWidget::setChecked(const bool checked)
{
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(checked);
    m_switchBtn->blockSignals(false);
}

void SwitchWidget::setName(const QString &name)
{
    m_name = name;
    setAccessibleName(name);
}

void SwitchWidget::setTitle(const QString& title)
{
    m_nameLabel->setText(title);
}

void SwitchWidget::setIcon(const QPixmap &icon)
{
    m_iconLabel->setPixmap(icon.scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

bool SwitchWidget::isChecked() const
{
   return m_switchBtn->isChecked();
}

void SwitchWidget::setDisabledFlag(bool b)
{
    this->setDisabled(b);
    m_switchBtn->setDisabled(b);
    m_switchBtn->setDisabledFlag(b);
}
