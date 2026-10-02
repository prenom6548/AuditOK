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

#include "diroptutils.h"

#include <QMapIterator>
#include <QFileInfo>
#include <QFileInfoList>
#include <QMap>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>

DirOptUtils::DirOptUtils()
{
    mMap.clear();
}

DirOptUtils::~DirOptUtils()
{

}

void DirOptUtils::testPrintMap()
{
    QMapIterator<QString, QString> iterator(mMap);
    while (iterator.hasNext()) {
        iterator.next();
        qDebug() << iterator.key() << ":" << iterator.value();
    }
}

QStringList DirOptUtils::getFileListUnderDir1(const QString &dirPath)
{
    //最外层目录
    QStringList fileList;
    mdirPath = dirPath;
    QDir dir(dirPath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs| QDir::Hidden);
#ifndef UNIT_TEST
    foreach (auto fileInfo, fileInfoList) {
        if(fileInfo.isDir())
        {
            getFileListUnderDir2(fileInfo.absoluteFilePath());
        }

        if(fileInfo.isFile())
        {
            if (0 == fileInfo.fileName().compare("md5sum.txt") || 0 == fileInfo.fileName().compare(".md5sum.txt")){

            } else  {
                // 计算文件md5值并写入map中
                QString md5output = calculateMd5(fileInfo.absoluteFilePath());
                QString fileRelativePath = fileInfo.absoluteFilePath();
                fileRelativePath = "." + fileRelativePath.mid(mdirPath.length()); //绝对路径变相对路径
                mMap.insert(fileRelativePath, md5output);
                qDebug() << __FUNCTION__ << __LINE__ << mdirPath << "absolute path=[" << fileInfo.absoluteFilePath() << "]  relativePath: [" << fileRelativePath << "]";
            }
        }
    }
#endif
    return fileList;
}

QStringList DirOptUtils::getFileListUnderDir2(const QString &dirPath)
{
    //里层目录
    QStringList fileList;
    QDir dir(dirPath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden);
#ifndef UNIT_TEST
    foreach (auto fileInfo, fileInfoList) {
        if(fileInfo.isDir())
        {
            getFileListUnderDir2(fileInfo.absoluteFilePath());
        }

        if(fileInfo.isFile())
        {
            // 计算文件md5值并写入map中
            QString md5output = calculateMd5(fileInfo.absoluteFilePath());
            QString fileRelativePath = fileInfo.absoluteFilePath();
            fileRelativePath = "." + fileRelativePath.mid(mdirPath.length()); //绝对路径变相对路径
            mMap.insert(fileRelativePath, md5output);
            qDebug() << __FUNCTION__ << __LINE__ << mdirPath << "absolute path=[" << fileInfo.absoluteFilePath() << "]  relativePath: [" << fileRelativePath << "]";
        }
    }
#endif
    return fileList;
}


QString DirOptUtils::calculateMd5(const QString &path)
{
    /* 打开文件 */
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"文件打开错误";
        return QString();
    }

    QCryptographicHash md5_hash(QCryptographicHash::Md5);
    md5_hash.reset();

    /* 文本读取 */
    while(!file.atEnd())
    {
        md5_hash.addData(file.readLine());
    }

    return md5_hash.result().toHex();
}


//在指定目录下生成md5sum.txt文件
bool DirOptUtils::generateCheckFile()
{
    // 1 生成校验文件.md5sum.txt文件
    QDir tempDir;
    QString fileName = ".md5sum.txt";
    if (!tempDir.exists(mdirPath)) {
        qDebug() << "dir: [" << mdirPath << "] is not exist";
        return false;
    }
#ifndef UNIT_TEST
    QFile tempFile;
    tempDir.setCurrent(mdirPath);
    if (tempFile.exists(fileName)) {
        qDebug() << QString("dir：[%1] has exist, so we need delete it.").arg(tempDir.path());
        // 删除它
        tempFile.remove(fileName);
    }

    tempFile.setFileName(fileName);

    if (tempFile.open(QIODevice::ReadWrite | QIODevice::Text)) {

        QMapIterator<QString, QString> iterator(mMap);
        while (iterator.hasNext()) {
            iterator.next();
            QString path = iterator.key();
            QString md5 = iterator.value();
            QString output = md5 + "  " + path + "\n";
            QTextStream out(&tempFile);
            out << output;
        }
        tempFile.close();
        qDebug() << "写完成";
    } else {
        qDebug() << "failed to open file";
        return false;
    }
#endif
    return true;
}
