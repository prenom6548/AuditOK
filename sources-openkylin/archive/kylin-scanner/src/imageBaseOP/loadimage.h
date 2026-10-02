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

#ifndef LOADIMAGE_H
#define LOADIMAGE_H

#include <QObject>
#include <QImage>
#include <QLabel>
#include <FreeImage.h>
#include <QFileInfo>

class LoadImage : public QObject
{
    Q_OBJECT
public:
    explicit LoadImage(QObject *parent = nullptr);
    void setFlag(bool);
public slots:
    void loadImageToWidget(QString loadPath, QString save_path, QImage *img, QLabel *container,double scale, bool rotationFlag,QSize labSize);
private:
    bool m_flag;

signals:
    void finished(QString, double, QImage);

};

#endif // LOADIMAGE_H
