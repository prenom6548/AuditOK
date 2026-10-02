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
#ifndef SCANNERIMAGEPROVIDER_H
#define SCANNERIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QDebug>
#include "./global/variable.h"

class ScannerImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    ScannerImageProvider(): QQuickImageProvider(QQuickImageProvider::Image){}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    //图片路径"image://name/id"
    {
        if(id == "orig"){
            return m_origImage;
        }else if(id == "scan"){
            return m_scanImage;
        }else if(id == "watermark"){
            return m_warkmarkImage;
        }else{
            return QImage();
        }
    }
    void setOrigImage(QImage image){
        m_origImage = image;
    }
    void setScanImage(QImage image){
        m_scanImage = image;
    }
    void setWarkmarkImage(QImage image){
        m_warkmarkImage = image;
    }

private:

    QImage m_origImage;
    QImage m_scanImage;
    QImage m_warkmarkImage;

};

#endif // SCANNERIMAGEPROVIDER_H
