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

#include "imageoperationrectify.h"
#include "../rectify.h"
#include <QDebug>
#include <QTime>
ImageOperationRectify::ImageOperationRectify(QImage *origin, QStack<QImage> *stackImage)
{
    m_origin = origin;
    m_stackImage = stackImage;
    isInterupt = false;
}

void ImageOperationRectify::ImageOP(){
    QImage imgCopy = imageCopy(m_origin, m_stackImage);
    int res = 0;

    res = ImageRectify(m_origin);
    if(res == 0){
        qDebug() << "rectify success!";
    }else{
        isInterupt = true;
        qDebug() << "rectify exit!";
    }
    emit finished();
}
