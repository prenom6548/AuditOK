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

#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <QObject>
#include <QPdfWriter>
#include <QImage>
#include <tiff.h>
#include <tiffio.h>
#include <FreeImage.h>
#include <QFile>
#include <QProcess>

class SaveFileBase : public QObject
{
    Q_OBJECT
public:
    explicit SaveFileBase(QObject *parent = nullptr);
    void saveFileOP(QString savePath,QImage *imgOP);
    bool imageSave(QImage *imgOP);
    void saveToPdf(QImage *img);
    int toUnicode(QString str);
    void setPdfSize(QImage *tmp, QPdfWriter *pdfWriter, QString size);
    void saveAsTiff(QImage image, QString file_name);

private:
    QString m_fileName;
    QList<QImage> m_tiffImageList;
    QStringList m_fileImages;
    QList<FIBITMAP*> m_filist;
    int m_pdfCount = 0;
    int m_ofdCount = 0;
    QStringList m_ofdName;
    QStringList m_pngName;
    QStringList m_pdfName;
signals:

};

#endif // SAVEFILE_H
