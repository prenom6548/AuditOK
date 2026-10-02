/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "shortcut.h"
#include <QWidget>

Shortcut::Shortcut(QString displayName, QString iconName, QColor color, QWidget *parent) : QWidget(parent)
{
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_layout = new QVBoxLayout(this);

    m_button = new ShortcutButton(this);
    m_button->setIconName(iconName);
    m_button->setColor(color);
    connect(m_button, &ShortcutButton::clicked, this, &Shortcut::clicked);
    m_nameLabel = new QLabel(displayName, this);

    m_layout->addWidget(m_button, Qt::AlignHCenter);
    m_layout->addWidget(m_nameLabel, Qt::AlignHCenter);
}

void Shortcut::statusChanged(UkuiShortcut::StatusInfo info)
{
    m_button->setIconName(info.getIcon());
    m_button->setColor(info.getColor());
    m_nameLabel->setText(info.getName());
}
