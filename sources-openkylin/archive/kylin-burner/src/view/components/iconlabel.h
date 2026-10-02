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


#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include "clicklabel.h"

#include <QSize>

class IConLabel : public ClickLabel
{
    Q_OBJECT

public:
    explicit IConLabel(QWidget *parent = nullptr);
    ~IConLabel();

    // 应用场景四：设置跟随主题风格图标（主题中也可能不存在该图标），随主体风格：寻光、启典、合印等图标风格变化而变换，不随深浅背景色变化
    void setThemeIconSchema(const QString &schema, const QString &schemaDefault, QSize size = QSize(24, 24));

private slots:
    void on_themeIconChanged();

private:
    // 主题图标
    QString m_iconTheme;
    QString m_iconThemeDefault;
    // 主题图标大小
    QSize m_iconSize;
};

#endif // PIXMAPLABEL_H
