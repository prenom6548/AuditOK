/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "workerobject.h"

#include "xmlhandle.h"
#include <QMetaType>
#include <QDebug>
#include <QBuffer>
#include <QCryptographicHash>


WorkerObject::WorkerObject(QObject *parent)
    : QThread(parent)
{
    //自定义结构提，线程传递时无法放入列表，注册解决
    qRegisterMetaType<QMap<QString, QMap<QString, QString>>>("QMap<QString, QMap<QString, QString>>");
    filenameList.clear();
}

WorkerObject::~WorkerObject()
{
    delete xmlHandleObj;
    xmlHandleObj = nullptr;
}

void WorkerObject::run(){
    //构建xmlhandle对象
    xmlHandleObj = new XmlHandle();

    //解析壁纸数据，如果本地xml文件不存在则自动构建
    xmlHandleObj->init();

    //获取壁纸数据
    wallpaperinfosMap = xmlHandleObj->requireXmlData();
    QByteArray allData = "";
    //压缩壁纸
    QSize IMAGE_SIZE(166, 110);
    QMap<QString, QMap<QString, QString> >::iterator iters = wallpaperinfosMap.begin();
    for (int row = 0; iters != wallpaperinfosMap.end(); iters++, row++){
        //跳过xml的头部信息
        if (QString(iters.key()) == "head")
            continue;

        //跳过被删除的壁纸
        QMap<QString, QString> wpMap = (QMap<QString, QString>)iters.value();
        QString delstatus = QString(wpMap.find("deleted").value());
        if (delstatus == "true")
            continue;

        QString filename = QString(iters.key());
        QFile file(filename);
        if(file.open(QIODevice::ReadOnly)) {//打开文件（只读
            uchar *mapped = file.map(0, file.size());
            if (mapped) {
                QByteArray content = QByteArray::fromRawData(reinterpret_cast<const char*>(mapped), file.size());
                allData += content;
                file.unmap(mapped);
            }
            file.close();
        }
        filenameList.append(filename);
    }
    QByteArray hashDataWrite = QCryptographicHash::hash(allData, QCryptographicHash::Md5);
    QFile hashFile;
    hashFile.setFileName(LOCAL_WALLPAPER_HASH_PATH);
    if (hashFile.open(QIODevice::ReadOnly)) {
        QByteArray hashDataRead = hashFile.readAll();
        hashFile.close();
        if (hashDataRead.compare(hashDataWrite) == 0) { //和已保存的一样
            QDir dir(LOCAL_WALLPAPER_PATH);
            QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
            if ((fileInfoList.size() - 1) == filenameList.size()) { //文件数量也一致
                QStringList dataList = {};
                for (int i = 0; i < filenameList.size(); ++i) {
                    QString filename = filenameList.at(i);
                    QString filanemaTemp = filename;
                    filanemaTemp.replace("/", "-");
                    QString loadFilename = LOCAL_WALLPAPER_PATH + QString::number(i) + filanemaTemp;
                    dataList.append(loadFilename);
                }
                Q_EMIT wallpaperReaded(dataList, filenameList);
                return;
            }
        }
    }

    //未保存正确的缩略壁纸，发送原壁纸
    Q_EMIT wallpaperReaded(filenameList, filenameList);

    //保存正确的缩略壁纸
    QDir dir;
    dir.setPath(LOCAL_WALLPAPER_PATH);
    if (dir.exists()) {
        dir.removeRecursively();
        qDebug()<<"removeRecursively:"<<LOCAL_WALLPAPER_PATH;
    }

    if (dir.mkpath(LOCAL_WALLPAPER_PATH)) {
        qDebug()<<"mkpath:"<<LOCAL_WALLPAPER_PATH<<"  success.";
        //写hash
        if (hashFile.open(QIODevice::WriteOnly)) {
            hashFile.write(hashDataWrite);
            hashFile.close();
        }
    } else {
        qDebug()<<"mkpath "<<LOCAL_WALLPAPER_PATH<<"  failed.";
    }
    QStringList dataList = {};
    for (int i = 0; i < filenameList.size(); ++i) {
        QString filename = filenameList.at(i);
        QString filanemaTemp = filename;
        filanemaTemp.replace("/", "-");
        QString loadFilename = LOCAL_WALLPAPER_PATH + QString::number(i) + filanemaTemp;
        dataList.append(loadFilename);
    }
    Q_EMIT wallpaperReaded(dataList, filenameList);
    return;
}
