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
#include "flip.h"

QImage Flip::processing(const QImage &image, const QVariant &args)
{
    int type = args.toInt();
    switch (type) {
    case FlipWay::clockwise:
        return rotateRight(image);
    case FlipWay::counterclockwise:
        return rotateLeft(image);
    case FlipWay::horizontal:
        return flipH(image);
    case FlipWay::vertical:
        return flipV(image);
    }

    return image;
}

QImage Flip::rotateRight(const QImage &image)
{  
    QMatrix rotatematrix;
    rotatematrix.rotate(90);
    QImage dst = image.transformed(rotatematrix, Qt::SmoothTransformation);
    return dst;
}

QImage Flip::rotateLeft(const QImage &image)
{
    QMatrix rotatematrix;
    rotatematrix.rotate(-90);
    QImage dst = image.transformed(rotatematrix, Qt::SmoothTransformation);
    return dst;
}

QImage Flip::flipH(const QImage &image)
{
    QImage dst = image.mirrored(true, false);
    return dst;
}

QImage Flip::flipV(const QImage &image)
{
    QImage dst = image.mirrored(!true, !false);
    return dst;
}
