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

#ifndef IMAGEOPERATIONBASE_H
#define IMAGEOPERATIONBASE_H

#include <QObject>
#include <QImage>

class ImageOperationBase : public QObject
{
    Q_OBJECT
public:
    explicit ImageOperationBase(QObject *parent = nullptr);    

    QImage *m_origin;
    QStack<QImage> *m_stackImage;
    bool isInterupt = false;
    QImage imageCopy(QImage *origin, QStack<QImage> *stackImage);
    bool getIsInterrupt();
    void interuptOp();
private:


signals:
    void finished();
public slots:
    virtual void ImageOP();
};

#endif // IMAGEOPERATIONBASE_H
