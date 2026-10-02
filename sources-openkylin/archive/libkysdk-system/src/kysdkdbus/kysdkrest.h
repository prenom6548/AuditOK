/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#ifndef KYSDKREST_H
#define KYSDKREST_H

/**
 * @file kysdkrest.h
 * @author shaozhimin (shaozhimin@kylinos.cn)
 * @brief sdk系统层屏幕分辨率信息生成dbus信号
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup libkysdk-system-dbus
 * @{
 * 
 */

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QtDebug>
#include <string.h>

#include <memory>
#include "../hardware/libkyprinter.h"       //打印

/**
 * @brief 生成屏幕分辨率信息dbus信号类
 * 主要用来生成系统当前显示屏幕,分辨率,系统支持分辨率dbus信号
 */
class KySdkRest : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kysdk.resolution")
public:
    explicit KySdkRest(QObject *parent = nullptr);

public slots:
    /**
     * @brief  获取系统当前显示屏幕,分辨率,系统支持分辨率
     * 
     * @return QStringList 当前显示屏幕,分辨率,系统支持分辨率信息列表
     */
    QStringList getSysLegalResolution() const;

};

/**
 * @brief 生成打印信息dbus信息类
 * 主要用来生成本地的所有打印机(包含可用打印机和不可用打印机)、获取本地的所有可用打印机(打印机处于idle状态)、
 * 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印、
 * 下载网络文件、打印本地文件、清除某个打印机的所有打印队列、获取当前打印机状态(状态不是实时更新)、
 * 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）、获取url内的文件名dbus信息
 */
class KySdkPrint : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.kysdk.print")
public:
    explicit KySdkPrint();
public slots:
    /**
     * @brief 获取本地的所有打印机(包含可用打印机和不可用打印机)
     * 
     * @return QStringList 地的所有打印机名称列表
     */
    QStringList getPrinterList() const;

    /**
     * @brief 获取本地的所有可用打印机(打印机处于idle状态)
     * 
     * @return QStringList 本地的所有可用打印机名称列表
     */
    QStringList getPrinterAvailableList() const;

    /**
     * @brief 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印
     * 
     * @param number_up cups属性，一张纸打印几页，如2 4等
     * @param media cups属性，纸张类型，如A4
     * @param number_up_layout cups属性，如lrtb
     * @param sides cups属性，单面如one-sided，双面如two-sided-long-edge(长边翻转),two-sided-short-edge(短边翻转)
     */
    void setPrinterOptions(int number_up, const QString media, const QString number_up_layout, const QString sides) const;
    
    /**
     * @brief 打印本地文件
     * 
     * @param printername 打印机名
     * @param filepath 文件绝对路径
     * 
     * @return int 打印作业的id
     * 
     * @notice 虽然支持多种格式的打印，但是打印除pdf之外的格式打印的效果都不够好，建议打印pdf格式的文件
     *         打印机处于stop状态函数返回0
     *         格式不对的文件可以成功创建打印任务，但是打印不出来。什么叫格式不对，举个例子，比如当前要打印日志文件a.log,
     *         然后把a.log改名叫a.pdf,这个时候a.pdf就变成了格式不对的文件，打印不出来了
     * 
     */
    int getPrinterPrintLocalFile (const QString printername, const QString filepath) const;

    /**
     * @brief 清除某个打印机的所有打印队列
     * 
     * @param printername 打印机名
     * @return int 成功返回0,失败返回-1
     */
    int getPrinterCancelAllJobs(const QString printername) const;

    /**
     * @brief 获取当前打印机状态(状态不是实时更新)
     * 
     * @param printername 打印机名
     * @return int 状态码
     */
    int getPrinterStatus(const QString printername) const;

    /**
     * @brief  获取url内的文件名
     * 
     * @param  url 路径
     * @return String 解析出来的函数名
     */
    QString getPrinterFilename(const QString url) const;

    /**
     * @brief 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）
     * 
     * @param printername 打印机名
     * @param jobid 打印作业的id
     * @return int 状态码
     */
    int getPrinterJobStatus(const QString printername, int jobid) const;

    /**
     * @brief 下载网络文件
     * 
     * @param url 要打印的url
     * @param filepath 保存的文件路径
     * 
     * @return int 打印作业的id
     */
    int getPrinterDownloadRemoteFile(const QString url, const QString filepath) const;
};

#endif // KYSDKREST_H
