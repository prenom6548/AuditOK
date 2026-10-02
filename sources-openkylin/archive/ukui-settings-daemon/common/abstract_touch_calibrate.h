/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ABSTRACTTOUCHCALIBRATE_H
#define ABSTRACTTOUCHCALIBRATE_H

#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QX11Info>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QDebug>



#include "clib-syslog.h"
//添加product 识别，但对于同型号触摸屏product id和 name 完全一致，无法区分
struct Product
{
    int32_t vid = 0;
    int32_t pid = 0;
    bool isValid = false; //有效性，是否需要有效匹配product

    bool operator ==(const Product& other) {
        return (this->vid == other.vid && this->pid == other.pid);
    }

    bool operator !=(const Product& other) {
        return (this->vid != other.vid || this->pid != other.pid);
    }
};

struct TouchDevice
{
    QString name;
    QString node;
    int id;
    int width = 0;
    int height = 0;
    bool isMapped = false; //已映射
    Product product;
    QVariant calibrateName;
};

struct ScreenInfo
{
    QString name;
    int width = 0;
    int height = 0;
    bool isMapped = false;
};

struct TouchConfig
{
    QString sTouchName;     //触摸屏的名称
    QString sTouchSerial;   //触摸屏的序列号
    QString sMonitorName;   //显示器的名称
    Product product;      //配置存储的ProductId
};//配置文件中记录的映射关系信息

typedef QSharedPointer<TouchDevice> TouchDevicePtr;
typedef QList<TouchDevicePtr> TouchDeviceList;

typedef QSharedPointer<ScreenInfo> ScreenInfoPtr;
typedef QMap<QString, ScreenInfoPtr> ScreenInfoMap;

typedef QSharedPointer<TouchConfig> TouchConfigPtr;
typedef QList<TouchConfigPtr> TouchConfigList;

class AbstractTouchCalibrate: public QObject
{
    Q_OBJECT
public:
    explicit AbstractTouchCalibrate(QObject *parent = nullptr);
    ~AbstractTouchCalibrate();

    /**
     * @brief 获取已连接屏幕信息
     */
    virtual void getScreenList() = 0;
    /**
     * @brief 获取触摸设备信息
     */
    virtual void getTouchDeviceList() = 0;
    /**
     * @brief 获取屏幕映射信息
     */
    void getTouchConfigure();

    /**
     * @brief touchscreen 映射
     */
    virtual void calibrateTouchScreen() = 0;
    /**
     * @brief tablet 映射
     */
    void calibrateTablet();

    /**
     * @brief checkMatch
     * @param output_width
     * @param output_height
     * @param input_width
     * @param input_height
     * @return 尺寸比较
     */
    bool checkMatch(double output_width, double output_height, double input_width, double input_height);

    /**
     * @brief 获取触摸设备尺寸
     * @param node
     * @param width
     * @param height
     */
    void getTouchSize(const char *node, int &width, int &height);

    /**
     * @brief calibrateDevice
     * @param id
     * @param output
     */
    virtual void calibrateDevice(QVariant id, const QString &output) = 0;
    /**
     * @brief autoMaticMapping 触摸笔，触摸屏，非配置部分映射
     * @param touchMap 触摸设备
     * @param screenMap 屏幕信息
     */
     void autoMaticMapping(TouchDeviceList& touchList, ScreenInfoMap& screenMap, bool isTouchScreen = true);

protected:
    Display *m_display;
    const QString m_touchConfigPath = QDir::homePath() + "/.config/touchcfg.ini";
    ScreenInfoMap m_screenInfoMap;
    TouchDeviceList m_touchScreenList;
    TouchDeviceList m_tabletList;
    TouchConfigList m_touchConfigList;

    /***wlcom service***/
    const QString C_WLCOM_SERVICE = "com.kylin.Wlcom";
    const QString C_WLCOM_INPUT_PATH = "/com/kylin/Wlcom/Input";
    const QString C_WLCOM_INPUT_INTERFACE = "com.kylin.Wlcom.Input";
};

#endif // ABSTRACTTOUCHCALIBRATE_H
