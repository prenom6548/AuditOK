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

#include "imageoperationbase.h"
#include "../include/common.h"
#include <QStack>
#include <QDebug>

ImageOperationBase::ImageOperationBase(QObject *parent) : QObject(parent)
{

}
void ImageOperationBase::ImageOP(){
    qDebug() << "do nothing!";
}
QImage ImageOperationBase::imageCopy(QImage *origin, QStack<QImage> *stackImage){
    QImage tmp = origin->copy();
    stackImage->push(tmp);
    return tmp;
}
void ImageOperationBase::interuptOp(){
    isInterupt = true;
}
bool ImageOperationBase::getIsInterrupt(){
    return isInterupt;
}
