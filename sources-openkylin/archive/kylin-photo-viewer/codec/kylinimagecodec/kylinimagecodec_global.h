/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef KYLINIMAGECODEC_GLOBAL_H
#define KYLINIMAGECODEC_GLOBAL_H

#include <QtCore/qglobal.h>

#include <QDebug>

#include <QFileInfo>
#include <QPainter>
#include <QMovie>
#include <QPixmap>
#include <QHash>
#include <vector>
#include <QStringList>
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
#include <gif_lib.h>
#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <cairo/cairo-svg.h>
#define signals rsvg_signals
#include <librsvg/rsvg.h>
#undef signals
#define SAVE_QUAITY_VALUE 100

#if defined(KYLINIMAGECODEC_LIBRARY)
#define KYLINIMAGECODEC_EXPORT Q_DECL_EXPORT
#else
#define KYLINIMAGECODEC_EXPORT Q_DECL_IMPORT
#endif

#endif // KYLINIMAGECODEC_GLOBAL_H
