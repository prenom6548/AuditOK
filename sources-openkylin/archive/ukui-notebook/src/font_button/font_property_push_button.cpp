/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include <QVariant>
#include "font_property_push_button.h"

FontPropertyPushButton::FontPropertyPushButton(QWidget *parent)
    : ResizablePushButton(parent)
{
    setTabletModeButtonSize(QSize(36, 36), QSize(48, 48));
    setProperty("isWindowButton", 0x1);
    setProperty("useIconHighlightEffect", 0x2);
    setCheckable(true);
    setFlat(true);
}

FontPropertyPushButton::~FontPropertyPushButton()
{
}
