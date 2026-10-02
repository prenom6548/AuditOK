/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 */

#ifndef CUSTOMSIDERBARSCROLLAREA_H
#define CUSTOMSIDERBARSCROLLAREA_H

#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>

class CustomSiderBarScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    explicit CustomSiderBarScrollArea(QWidget *parent = nullptr);
    ~CustomSiderBarScrollArea();

    void updateScrollBarStyle(int themeColor);
};

#endif // CUSTOMSIDERBARSCROLLAREA_H
