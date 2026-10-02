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
#ifndef COMMONINFOCOLLECTION_H
#define COMMONINFOCOLLECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDebug>
#include "KylinDaqCommon.h" 
#include "InfoCollectionBase.h"
#include "NetworkInfoCollection.h"
#include "RegisterNumberInfoCollection.h"


typedef struct
{
    QString mainBoardSerialNumber;
    QString systemVersion;
    QString systemSerialNumber;
    QString systemUuid;
    QString kernelVersion;
    QString processorModel;
    QString graphicsCardModel;
    QString networkMacAddress;
    QString networkType;
    QString hardDiskSerialNumber;
    QString systemManufacturer;
    QString systemProductName;
    QString machineId;
    QString registerNumber;
    QString dpkgVersion;
    QString kysecVersion;
    QString kysecAuthVersion;
    QString kysecDaemonVersion;
    QString kysecUtilsVersion;
    QString libkysecVersion;
    QString osVersion;
    QString osReleaseId;
    QString osBuildInfo;
    QString osTreeVersion;
    QString osPatchVersion;
    QString osSerialNumber;
    QString systemArchitecture;
    QString systemLanguage;
}CommonInfo;

class CommonInfoCollection : public InfoCollectionBase
{
    Q_OBJECT
private: 
    /**
     * @brief 获取主板序列号命令
     * 
     */
    QString mainBoardSerialNumberCmd;
    /**
     * @brief 获取系统版本命令
     * 
     */
    QString systemVersionCmd;
    /**
     * @brief 获取系统序列号命令
     * 
     */
    QString systemSerialNumberCmd;
    /**
     * @brief 获取uuid命令
     * 
     */
    QString systemUuidCmd;
    /**
     * @brief 获取内核版本命令
     * 
     */
    QString kernelVersionCmd;
    /**
     * @brief 获取处理器型号命令
     * 
     */
    QString processorModelCmd;
    /**
     * @brief 获取显卡型号命令
     * 
     */
    QString graphicsCardModelCmd;
    /**
     * @brief 获取硬盘序列号命令
     * 
     */
    QString hardDiskSerialNumberCmd;
    /**
     * @brief 获取网卡mac地址命令
     * 
     */
    QString networkMacAddressCmd;
    /**
     * @brief 获取网卡类型命令
     * 
     */
    QString networkTypeCmd;
    /**
      * @brief 获取系统厂商
      *
      */
    QString systemManufacturerCmd;
    /**
      * @brief 获取系统厂商
      *
      */
    QString systemProductNameCmd;
    /**
     * @brief 获取系统机器id
     * 
     */
    QString machineIdCmd;
    /**
     * @brief 网卡信息收集类
     */
    QScopedPointer<NetworkInfoCollection> networkInfoCollection;
    /**
     * @brief 注册码
     */
    QScopedPointer<RegisterNumberInfoCollection> registerNumberInfoCollection;
    QString dpkgVersionCmd;
    QString kysecVersionCmd;
    QString kysecAuthVersionCmd;
    QString kysecDaemonVersionCmd;
    QString kysecUtilsVersionCmd;
    QString libkysecVersionCmd;
    QString osVersionCmd;
    QString osReleaseIdCmd;
    QString osBuildInfoCmd;
    QString systemArchitectureCmd;
    QString systemLanguageCmd;
public:
    /**
     * @brief Construct a new Common Info Collection object
     * 
     * @param parent 
     */
    explicit CommonInfoCollection(QObject *parent = nullptr);
    /**
     * @brief 获取公共信息
     * 
     * @param commonInfo 公共信息
     */
    void getInfo(CommonInfo &commonInfo);

private:
    /**
     * @brief 获得硬盘序列号
     * 
     * @param serialNumber 硬盘序列号
     */
    void getHardDiskSerialNumber(QString &serialNumber);
    /**
     * @brief 获取OStree版本号
     */
    QString getOstreeVersion();
    /**
     * @brief 获取系统补丁版本号
     */
    QString getOsPatchVersion();
    /**
     * @brief 获取服务序列号
     */
    QString getOsSerialNumber();

    /**
     *  @brief 获取显卡信息
    */
    QString getGraphicsCardModel();


signals:

};

#endif // COMMONINFOCOLLECTION_H
