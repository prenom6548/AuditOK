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
/*** 
 * @Author: pengcheng
 * @Date: 2021-06-24 10:27:50
 * @LastEditTime: 2021-09-17 14:44:01
 * @LastEditors: pengcheng
 * @Description: 
 * @FilePath: /kylin-daq/src/utils.h
 */
#ifndef UTILS_H
#define UTILS_H
#include <QtCore/QString>
#include <QtCore/QDir>

//开机关机时间
typedef struct TimeInformation
{
    QString bootTime;//开机时间
    QString shutdownTime;//关机时间
}TimeInfo_t;

//操作系统信息
typedef struct OperatingSystemInformation
{
    QString systemVersion = "";//操作系统版本
    QString serialNumber;//操作系统序列号
    QString activationState;//操作系统系统是否激活状态标志
}OperatingSystemInfo_t;

//整机信息
typedef struct MachineInformation
{
    QString manufacturers;//整机制造商
    QString model;//整机型号
}MachineInfo_t;

//处理器信息
typedef struct ProcessorInformation
{
    QString manufacturers;//处理器制造商
    QString model;//处理器型号
    QString maxSpeed;//处理其最大主频
}ProcessorInfo_t;

//主板和BOIS信息
typedef struct MainboardInformation
{
    QString manufacturers;//主板制造商
    QString model;//主板型号
    QString biosManufacturers;//bios制造商
    QString biosVersion;//bios版本
}MainboardInfo_t;

//硬盘信息
typedef struct HardDiskInformation
{
    QString manufacturers;//硬盘制造商
    QString model;//硬盘型号
    QString serialNumber;//硬盘序列号
    QString capacity;//硬盘容量
}HardDiskInfo_t;

//网卡信息
typedef struct NetworkInformation
{
    QString model;//网卡型号 product
    QString driver;//网卡驱动 dirver
    QString type;//网卡类型 description
    QString macAddress;//mac地址 serial
}NetworkCardInfo_t;

//显卡信息
typedef struct GraphicsCardInformation
{
    QString manufacturers;//显卡制造商 vendor
    QString model;//显卡型号 product
    QString driver;//显卡驱动 dirver
}GraphicsCardInfo_t;

//  公共信息
//typedef struct CommonInformation
//{
//    QString mainboardModel;
//    QString networkMacAddress;
//    QString processorModel;
//    QString graphicsCardModel;
//    QString operatingSystemVersion;
//    QString operatingSystemSerialNumber;
//    // TODO：添加网络类型
//    QString networkType;
//    QString hardDiskSerialNumber;
//    QString kernelVersion;
//    QString uid;
//};


typedef QList<HardDiskInfo_t> HardDiskInfoList_t;
typedef QList<NetworkCardInfo_t> NetworkCardInfoList_t;
typedef QList<GraphicsCardInfo_t> GraphicsCardInfoList_t;

typedef struct DeviceInformation
{
    TimeInfo_t          timeInfo;
    OperatingSystemInfo_t        operatingOperatingSystemInfo;
    MachineInfo_t       machineInfo;
    ProcessorInfo_t     processorInfo;
    MainboardInfo_t     mainboardInfo;
    HardDiskInfoList_t  hardDiskInfoList;
    NetworkCardInfoList_t   networkCardInfoList;
    GraphicsCardInfoList_t   graphicsCardInfoList;
}DeviceInfo_t;

#endif // UTILS_H
