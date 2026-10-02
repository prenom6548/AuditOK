/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
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
#ifndef BAISCINFOCOLLECTION_H
#define BAISCINFOCOLLECTION_H

#include <QObject>
#include <QString>
#include <unistd.h>
#include "InfoCollectionBase.h"

typedef struct
{
    QString kernelVersion;
    QString gccVersion;
    QString glibcVersi;
    QString gdbVersion;
    QString qt5Version;
    QString qt4Version;
    QString python3Version;
    QString python2Version;
    QString javaVersion;
    QString grubVersion;
    QString cupsVersion;
    QString libjbigVersion;
    QString libpngVersion;
    QString libjpeg;
    QString kylinDaqVersion;
}BasicInfo;

class BasicInfoCollection : public InfoCollectionBase
{
    Q_OBJECT
private:
    /**
     * @brief 获取内核版本命令
     * 
     */
    QString kernelVersionCmd;
    /**
     * @brief 获取gcc版本命令
     * 
     */
    QString gccVersionCmd;
    /**
     * @brief 获取glibc版本命令
     * 
     */
    QString glibcVersiCmd;
    /**
     * @brief 获取gdb版本命令
     * 
     */
    QString gdbVersionCmd;
    /**
     * @brief 获取qt5版本命令
     * 
     */
    QString qt5VersionCmd;
    /**
     * @brief 获取qt4版本命令
     * 
     */
    QString qt4VersionCmd;
    /**
     * @brief 获取python3版本命令
     * 
     */
    QString python3VersionCmd;
    /**
     * @brief 获取python2版本命令
     * 
     */
    QString python2VersionCmd;
    /**
     * @brief 获取java版本命令
     * 
     */
    QString javaVersionCmd;
    /**
     * @brief 获取grub版本命令
     * 
     */
    QString grubVersionCmd;
    /**
     * @brief 获取cups版本命令
     * 
     */
    QString cupsVersionCmd;
    /**
     * @brief 获取libjbig版本命令
     * 
     */
    QString libjbigVersionCmd;
    /**
     * @brief 获取libpng版本命令
     * 
     */
    QString libpngVersionCmd;
    /**
     * @brief 获取libjpeg版本命令
     * 
     */
    QString libjpegCmd;
    QString kylinDaqVersionCmd;
public:
    /**
     * @brief Construct a new Basic Info Collection object
     * 
     * @param parent 
     */
    explicit BasicInfoCollection(QObject *parent = nullptr);
    /**
     * @brief 获得信息函数
     * 
     * @param basicInfo 基础信息
     */
    void getInfo(BasicInfo &basicInfo);

private:
signals:

};

#endif // BAISCINFOCOLLECTION_H
