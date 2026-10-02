/*
 * libkysdk-base's Library
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#ifndef KYLIN_IMAGE_CODEC_KYLINIMAGECODEC_GLOBAL_H_
#define KYLIN_IMAGE_CODEC_KYLINIMAGECODEC_GLOBAL_H_

#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <gif_lib.h>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <QDebug>
#include <QFileInfo>
#include <QHash>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QSvgGenerator>
#include <QSvgRenderer>
#include <QtCore/qglobal.h>

/**
 * @brief kabase模块应用通用函数
 * @since 1.2.0
 */

using namespace cv;

#if defined(KYLINIMAGECODEC_LIBRARY)
#define KYLINIMAGECODEC_EXPORT Q_DECL_EXPORT
#else
#define KYLINIMAGECODEC_EXPORT Q_DECL_IMPORT
#endif

#endif // KYLINIMAGECODEC_GLOBAL_H
