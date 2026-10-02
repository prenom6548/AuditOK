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

#include "shortcut-button.h"
#include <QPalette>
#include <QVariant>
#include <QDebug>

ShortcutButton::ShortcutButton(QWidget *parent) : QPushButton(parent)
{
    setFixedSize(56, 56);
    setIconSize(QSize(24, 24));
    setProperty("isRoundButton",true);
}

void ShortcutButton::setIconName(const QString &icon)
{
    qDebug() << icon;
    this->setIcon(QIcon::fromTheme(icon));
}

void ShortcutButton::setColor(const QColor &color)
{
    QPalette pa = this->palette();
    pa.setColor(QPalette::Normal, QPalette::Button, color);
    this->setPalette(pa);
}
