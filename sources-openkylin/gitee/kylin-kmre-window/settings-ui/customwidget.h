/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#pragma once

#include <QPushButton>
#include <QLabel>
#include "utils.h"

template <class T>
class CustomWidget: public T
{
public:
    explicit CustomWidget(const QString& text, uint32_t maxLen, int width, int height, QWidget* parent = nullptr)
        : T(parent) {
        auto [elided, elideText] = kmre::utils::getElideText(text, maxLen);
        T::setText(elideText);
        if (elided) {
            T::setToolTip(text);
        }
        
        if ((width > 0) && (height > 0)) {
            T::setFixedSize(width, height);
        }
        else if (width > 0) {
            T::setFixedWidth(width);
        }
        else if (height > 0) {
            T::setFixedHeight(height);
        }
    }
};