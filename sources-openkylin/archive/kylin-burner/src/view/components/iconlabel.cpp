/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "iconlabel.h"
#include <QIcon>

IConLabel::IConLabel(QWidget *parent) :
    ClickLabel(parent)
{
    setAlignment(Qt::AlignCenter);
}

IConLabel::~IConLabel()
{}

// 应用场景：设置跟随主题风格图标（主题中也可能不存在该图标），不跟随主体风格：寻光、启典、合印等图标风格变化而变换，不随深浅背景色变化
void IConLabel::setThemeIconSchema(const QString &schema, const QString &schemaDefault, QSize size)
{
    m_iconTheme = schema;
    m_iconThemeDefault = schemaDefault;
    m_iconSize = size;

    on_themeIconChanged();
}

void IConLabel::on_themeIconChanged()
{
    QIcon titleIcon = QIcon::fromTheme(m_iconTheme, QIcon(m_iconThemeDefault));
    setPixmap(titleIcon.pixmap(titleIcon.actualSize(m_iconSize)));
}

