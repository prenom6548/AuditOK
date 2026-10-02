/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */

#include "window-thumbnail-item.h"
#include "wlcom-thumbnail.h"
using namespace UkuiThumbnail;

WindowThumbnailItem::WindowThumbnailItem(QQuickItem* parent)
    : ThumbnailItem(parent)
{
}

UkuiThumbnail::Thumbnail* WindowThumbnailItem::createThumbnail()
{
    auto* thumbnail = new WlcomThumbnail(this);
    thumbnail->setType(WlcomThumbnail::Type::Toplevel);
    return thumbnail;
}
