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

#include "createofd.h"

#include <QUuid>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <zip.h>

QString OFD::m_OFD = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                     "<ofd:OFD xmlns:ofd=\"http://www.ofdspec.org/2016\" Version=\"1.1\" DocType=\"OFD\">"
                     "<ofd:DocBody>"
                     "<ofd:DocInfo>"
                     "<ofd:DocID>%1</ofd:DocID>"
                     "<ofd:Title>%2</ofd:Title>"
                     "<ofd:Creator>%3</ofd:Creator>"
                     "<ofd:CreationDate>%4</ofd:CreationDate>"
                     "</ofd:DocInfo>"
                     "<ofd:DocRoot>Doc/doc.xml</ofd:DocRoot>"
                     "</ofd:DocBody>"
                     "</ofd:OFD>";
QString OFD::m_OFDDoc = "<ofd:Document xmlns:ofd=\"http://www.ofdspec.org/2016\">"
                        "<ofd:Pages>"
                        "%1"
                        "</ofd:Pages>"
                        "<ofd:CommonData>"
                        "<ofd:MaxUnitID>%2</ofd:MaxUnitID>"
                        "<ofd:PublicRes>PublicRes.xml</ofd:PublicRes>"
                        "</ofd:CommonData>"
                        "</ofd:Document>";
QString OFD::m_OFDContent = "<ofd:Page xmlns:ofd=\"http://www.ofdspec.org/2016\">"
                            "<ofd:Area>"
                            "<ofd:PhysicalBox>0 0 210 297</ofd:PhysicalBox>"
                            "</ofd:Area>"
                            "<ofd:Content>"
                            "<ofd:Layer ID=\"%1\">"
                            "<ofd:ImageObject ID=\"%2\" CTM=\"210 0 0 297 0 0\" Boundary=\"0 0 210 297\" ResourceID=\"%3\"/>"
                            "</ofd:Layer>"
                            "</ofd:Content>"
                            "</ofd:Page>";
QString OFD::m_OFDPublicRes = "<ofd:Res xmlns:ofd=\"http://www.ofdspec.org/2016\"  BaseLoc=\"Res\">"
                              "<ofd:MultiMedias>"
                              "%1"
                              "</ofd:MultiMedias>"
                              "</ofd:Res>";
QString OFD::m_docTitle = "扫描文件";
QString OFD::m_docCreator = "麒麟扫描";
zip *OFD::m_zipHandle = nullptr;
int OFD::m_docID = 0;
QList<char *> OFD::m_imageData;

bool OFD::createOFD(QString fileName, QStringList imageList)
{
    //创建文档id
    QString uuid = QUuid::createUuid().toString();
    uuid = uuid.mid(1);
    uuid.chop(1);
    uuid.replace("-","");
    uuid = uuid.toUpper();

    //创建ZIP压缩包
    int err;
    m_zipHandle = zip_open(fileName.toLocal8Bit().data(), ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (m_zipHandle == nullptr) {
        qDebug()<<"zip_open failed :" << err;
        return false;
    }

    //添加文件
    m_docID = 0;
    clearImageData();
    QByteArray data;
    QString pages;
    QString multiMedias;
    for (int i = 1 ; i <= imageList.length() ; i++) {
        QString orig = imageList.at(i - 1);
        QString suffix = QFileInfo(orig).suffix();

        QString index = QString::number(i);

        QString page = getMaxIDAndAdd();
        pages += QString("<ofd:Page BaseLoc=\"Pages/%1/Content.xml\" ID=\"%2\"/>").arg(index , page);

        QString resourceID = getMaxIDAndAdd();
        data = m_OFDContent.arg(getMaxIDAndAdd() , getMaxIDAndAdd() , resourceID).toLocal8Bit();

        addFileFromData(QString("Doc/Pages/%1/Content.xml").arg(i) , data);

        multiMedias += QString("<ofd:MultiMedia ID=\"%1\" Type=\"Image\"><ofd:MediaFile>%2.%3</ofd:MediaFile></ofd:MultiMedia>").arg(resourceID , index , suffix);

        addFileFromFile(QString("Doc/Res/%1.%2").arg(index,suffix) , orig);
    }

    data = m_OFD.arg(uuid,m_docTitle,m_docCreator,QDateTime::currentDateTime().toString("yyyy-MM-dd")).toLocal8Bit();
    addFileFromData("OFD.xml",data);

    data = m_OFDDoc.arg(pages , getMaxIDAndAdd()).toLocal8Bit();
    addFileFromData("Doc/doc.xml",data);

    data = m_OFDPublicRes.arg(multiMedias).toLocal8Bit();
    addFileFromData("Doc/PublicRes.xml",data);

    //关闭ZIP句柄
    if (zip_close(m_zipHandle) == -1) {
        qDebug()<<"zip_close failed";
        return false;
    }
    clearImageData();
    return true;
}

void OFD::setOFDTitle(QString value)
{
    m_docTitle = value;
}

void OFD::setOFDCreator(QString value)
{
    m_docCreator = value;
}

void OFD::addFileFromData(QString fileName, QByteArray data)
{
    char *fileData = (char*)malloc(data.size());
    memcpy(fileData, data.data(), data.size());
    m_imageData.append(fileData);
    struct zip_source *source = zip_source_buffer(m_zipHandle, fileData, data.size(), 0);
    if (source == nullptr) {
        qDebug()<<"zip_source_buffer failed :" << fileName;
        return;
    }
    if (zip_file_add(m_zipHandle, fileName.toLocal8Bit().data(), source, ZIP_FL_OVERWRITE) < 0) {
        qDebug()<<"zip_file_add failed :" << fileName;
        return;
    }
}

void OFD::addFileFromFile(QString fileName, QString origFileName)
{
    QFile file(origFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug()<<"QFile::open failed :" << origFileName;
        return;
    }
    QByteArray data = file.readAll();
    addFileFromData(fileName, data);
    file.close();
}

QString OFD::getMaxIDAndAdd()
{
    m_docID++;
    return QString::number(m_docID);
}

void OFD::clearImageData()
{
    for (char* img : m_imageData) {
        if (img) {
            free(img);
        }
    }
    m_imageData.clear();
}


