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

#include "loadimage.h"
#include <QDebug>
#include <QPixmap>
#include <QThread>
#include "../saneobject.h"
LoadImage::LoadImage(QObject *parent) : QObject(parent)
{
    m_flag = false;
}
void LoadImage::setFlag(bool res){
    m_flag = res;
}
void LoadImage::loadImageToWidget(QString loadPath, QString save_path, QImage *img, QLabel *container, double scale, bool rotationFlag, QSize labSize){
    if(m_flag){
        m_flag = false;
        return;
    }
    QFileInfo loadPathInfo(loadPath);
    QString newLoadPath = loadPathInfo.absolutePath() + "/" + loadPathInfo.baseName() + ".jpg";

    img->load(newLoadPath);


    double proportion;
    {
        if (img->isNull()) {
            qDebug() << "image is null.";
            return;
        }
        double labWidth = labSize.width();
        double labHeight = labSize.height();

        double imgWidth = img->width();
        double imgHeight = img->height();

        if(rotationFlag){
            double tmp = imgWidth;
            imgWidth = imgHeight;
            imgHeight = tmp;
        }


        if (! qFuzzyCompare(scale, 1.0)) {
            proportion = scale;
        } else {
            if ((labWidth / imgWidth) <= (labHeight / imgHeight)) {
                proportion = labWidth / imgWidth;
            } else {
                proportion = labHeight / imgHeight;
            }
        }

        QPixmap pixmap = QPixmap::fromImage(*img);

        QPixmap fitpixmap =pixmap.scaled(img->size()*proportion, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        container->setPixmap(fitpixmap);
        container->setFixedSize(QSize(fitpixmap.size()));
        container->setScaledContents(true);
        container->setAlignment(Qt::AlignCenter | Qt::AlignVCenter | Qt::AlignHCenter);
    }
    QImage image = *img;
    qDebug() << &image << " " << img;
    if(m_flag){
        m_flag = false;
        return;
    }
    emit finished(save_path, proportion, image);

    if (g_sane_object->ocrFlag != 0) {
        g_user_signal->toolbarOcrOperationStart();
    }
}



