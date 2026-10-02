/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "filecheck.h"

FileCheck::FileCheck(QObject *parent) : QObject(parent)
{

}

bool FileCheck::isLegalIconFile(const QString &filePath)
{
    QSvgRenderer renderer(filePath);
    QFileInfo fileinfo(filePath);
    if(fileinfo.suffix() != "svg"){
        QMessageBox::information(nullptr, tr("error"), tr("The file is not a svg"));
        return false;
    }
    if (renderer.isValid()){
        QSize svgSize = renderer.defaultSize();
        if (svgSize.width() == 256 && svgSize.height() == 256 ){
            if(svgSize.width() == svgSize.height()){
                return true;
            }else{
                if(FileProcess::g_isBatchImport == false){
                    QMessageBox::information(nullptr, tr("error"), tr("the icon is not a square icon"));
                }

                return false;
            }
        }else{
            if(FileProcess::g_isBatchImport == false){
                QMessageBox::information(nullptr, tr("error"), tr("The icon size must be 256x256"));
            }

            return false;
        }
    }

    return false;
}

bool FileCheck::isLegalCursorFile(const QString &filePath)
{
    QSvgRenderer renderer(filePath);
    QFileInfo fileinfo(filePath);
    if(fileinfo.suffix() != "svg"){
        QMessageBox::information(nullptr, tr("error"), tr("The file is not a svg"));
        return false;
    }
    if (renderer.isValid()){
        QSize svgSize = renderer.defaultSize();
        if (svgSize.width() == 96 && svgSize.height() == 96 ){
            if(svgSize.width() == svgSize.height()){
                return true;
            }else{
                if(FileProcess::g_isBatchImport == false){
                    QMessageBox::information(nullptr, tr("error"), tr("the icon is not a square icon"));
                }
                return false;
            }
        }else{
            if(FileProcess::g_isBatchImport == false){
                QMessageBox::information(nullptr, tr("error"), tr("The icon size must be 96x96"));
            }

            return false;
        }
    }

    return false;
}

bool FileCheck::isLegalWallPaperFile(const QString& filePath, const QString& type)
{
    QFileInfo fileInfo(filePath);
    QImage image(filePath);
    QSize imageSize = image.size();
    qint64 fileSize = fileInfo.size();

    if (type == "wallpaper") {
        if(fileInfo.suffix()!="jpg"){
            QMessageBox::information(nullptr, tr("error"), tr("The file is not a jpg"));
            return false;
        }
        if (imageSize.width() != 3840 || imageSize.height() != 2160){
            QMessageBox::information(nullptr, tr("error"), tr("The wallpaper size must be 3840x2160"));
            return false;
        }
        qint64 maxSize = 10 * 1024 * 1024; // 10 MB
        if (fileSize > maxSize){
            QMessageBox::information(nullptr, tr("error"), tr("The file size cannot exceed 10MB"));
            return false;
        }
    } else if (type == "cover") {
        if(fileInfo.suffix()!="png"){
            QMessageBox::information(nullptr, tr("error"), tr("The file is not a png"));
            return false;
        }
        if (imageSize.width() != 1640 || imageSize.height() != 1080){
            QMessageBox::information(nullptr, tr("error"), tr("Cover size must be 1640x1080"));
            return false;
        }
        qint64 maxSize = 5 * 1024 * 1024; // 5 MB
        if (fileSize > maxSize){
            QMessageBox::information(nullptr, tr("error"), tr("The file size cannot exceed 5MB"));
            return false;
        }
    } else if (type == "grub") {
        if(fileInfo.suffix()!="png"){
            QMessageBox::information(nullptr, tr("error"), tr("The file is not a png"));
            return false;
        }
        if (imageSize.width() != 3840 || imageSize.height() != 2160){
            QMessageBox::information(nullptr, tr("error"), tr("The wallpaper size must be 3840x2160"));
            return false;
        }
        qint64 maxSize = 10 * 1024 * 1024; // 10 MB
        if (fileSize > maxSize){
            QMessageBox::information(nullptr, tr("error"), tr("The file size cannot exceed 10MB"));
            return false;
        }
    }

    return true;
}

bool FileCheck::isLegalMP4File(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || !fileInfo.isFile() || fileInfo.suffix().toLower() != "mp4") {
        QMessageBox::information(nullptr, tr("error"), tr("The file is not a mp4"));
        return false;
    }

    qint64 fileSize = fileInfo.size(); // 文件大小，单位为 Bytes
    qint64 maxSize = 1024 * 1024 *10;

    if (fileSize > maxSize) {
        QMessageBox::information(nullptr, tr("error"), tr("The file size cannot exceed 10MB"));
        return false;
    }

    return true;
}

bool FileCheck::isLegalPlymouthPicFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || !fileInfo.isFile() || fileInfo.suffix().toLower() != "png") {
        QMessageBox::information(nullptr, tr("error"), tr("The file is not a png"));
        return false;
    }

    qint64 fileSize = fileInfo.size(); // 文件大小，单位为 Bytes
    qint64 maxSize = 1024 * 20;

    if (fileSize > maxSize) {
        QMessageBox::information(nullptr, tr("error"), tr("The file size cannot exceed 20KB"));
        return false;
    }

    return true;
}

bool FileCheck::isLegalThemeName(const QString &filename)
{
    //判断是否是黑名单里面的主题包名
    QFile blacklistFile(":/resource/blacklist.txt");
     if (!blacklistFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug() << "Error opening blacklist file.";
     }
     QString inputText = filename;

     QTextStream in(&blacklistFile);
     while (!in.atEnd()) {
         QString line = in.readLine().trimmed();
         if (inputText == line) {
             qDebug() << "error";
             return false;
         }
     }
     return true;
}

QStringList FileCheck::isLegalBatchIcon(const QString &filePath, const QMap<QString, QString> *map)
{
    QStringList foundFiles; // 用于存储符合条件的文件路径
    QDir dir(filePath);
    if (!dir.exists()) {
        qDebug() << "Directory does not exist:" << filePath;
        return foundFiles;
    }

    QStringList filters;
    filters << "*.svg";
    dir.setNameFilters(filters);

    QFileInfoList fileInfoList = dir.entryInfoList();

    qDebug() << "Keys in the map:";
    for (auto it = map->begin(); it != map->end(); ++it) {
//        qDebug() << it.key();
    }

    // 打印在文件夹里存在且在map中的key中存在的文件名
    qDebug() << "Files found in directory and also present in map:";
    for (const QFileInfo &fileInfo : fileInfoList) {
        QString fileName = fileInfo.fileName().split('.').first();
        if (map->contains(fileName)) {
            foundFiles.append(fileInfo.filePath());
        }
    }

    return foundFiles;
}

