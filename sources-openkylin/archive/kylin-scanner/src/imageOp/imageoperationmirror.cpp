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

#include "imageoperationmirror.h"
#include "../include/common.h"

ImageOperationMirror::ImageOperationMirror(QImage *origin,QStack<QImage> *stackImage)
{
    m_origin = origin;
    m_stackImage = stackImage;
}
void ImageOperationMirror::ImageOP(){
    QImage res = imageCopy(m_origin,m_stackImage);
    mirror(&res);
}
void ImageOperationMirror::mirror(QImage *img){
    img->mirrored(true, false);
    img->save(ScanningPicture);
    emit finished();
}
