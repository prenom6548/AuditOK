/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef ICON_H
#define ICON_H

#include <QObject>
#include <QQuickImageProvider>
#include <QIcon>
#include <QUrl>
#include <QPixmap>
#include <QString>

namespace UKUIQQC2Style {

class Icon : public QObject
{
    Q_OBJECT

public:
    explicit Icon(QObject *parent = nullptr);

    enum Mode { Normal, Disabled, Hovered, Selected, Highlight, HighDisabled,
                NormalWidthThemeColor, HoveredWidthThemeColor, ClickedWidthThemeColor, DisableWidthThemeColor};

};
}
#endif // IMAGEPROVIDER_H
