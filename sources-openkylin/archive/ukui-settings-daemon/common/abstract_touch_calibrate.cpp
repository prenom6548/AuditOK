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

#include "abstract_touch_calibrate.h"
#include "usd_base_class.h"
#include <QDBusPendingCall>

extern "C"{
#include <gudev/gudev.h>
#include "clib-syslog.h"
}

AbstractTouchCalibrate::AbstractTouchCalibrate(QObject *parent)
{

}

AbstractTouchCalibrate::~AbstractTouchCalibrate()
{
    m_screenInfoMap.clear();
    m_touchScreenList.clear();
    m_tabletList.clear();
    m_touchConfigList.clear();
}

//获取触摸配置
void AbstractTouchCalibrate::getTouchConfigure()
{
    QFileInfo file(m_touchConfigPath);
    if(file.exists()) {
        QSettings *configSettings = new QSettings(m_touchConfigPath, QSettings::IniFormat);
        int mapNum = configSettings->value("/COUNT/num").toInt();
        if (mapNum < 1) {
            return;
        }
        for (int i = 0; i < mapNum ;++i) {
            QString mapName = QString("/MAP%1/%2");
            QString touchName = configSettings->value(mapName.arg(i+1).arg("name")).toString();
            if(touchName.isEmpty()) {
                continue;
            }

            QString scrname = configSettings->value(mapName.arg(i+1).arg("scrname")).toString();
            if(scrname.isEmpty()) {
                continue;
            }

            // serial 不判空，暂不关注，无法通过序列号匹配屏幕
            QString serial = configSettings->value(mapName.arg(i+1).arg("serial")).toString();

            TouchConfigPtr touchConfig(new TouchConfig);
            touchConfig->sTouchName = touchName;
            touchConfig->sMonitorName = scrname;
            touchConfig->sTouchSerial = serial;

            //读取配置中productId,如果无此配置则无效，不进行product比较
            const QStringList& productList = configSettings->value(mapName.arg(i+1).arg("productId")).toString().split(" ");
            if (productList.count() > 1) {
                touchConfig->product.isValid = true;
                touchConfig->product.vid = productList[0].toInt();
                touchConfig->product.pid = productList[1].toInt();
            }
            m_touchConfigList.append(touchConfig);
        }
        configSettings->deleteLater();
    }
}

void AbstractTouchCalibrate::calibrateTablet()
{
    //触摸笔映射，重置屏幕映射
    ScreenInfoMap::iterator it_sc = m_screenInfoMap.begin();
    while (it_sc != m_screenInfoMap.end()) {
        if (it_sc.value()->isMapped) {
            it_sc.value()->isMapped = false;
        }
        ++it_sc;
    }
    autoMaticMapping(m_tabletList, m_screenInfoMap, false);
}

//尺寸检测
bool AbstractTouchCalibrate::checkMatch(double output_width,  double output_height,
                double input_width, double input_height)
{
    double w_diff, h_diff;
    w_diff = std::abs(1 - (output_width / input_width));
    h_diff = std::abs(1 - (output_height / input_height));

    SYS_LOG(LOG_DEBUG,"w_diff--------%f,h_diff----------%f", w_diff, h_diff);
    if (w_diff < MAX_SIZE_MATCH_DIFF && h_diff < MAX_SIZE_MATCH_DIFF) {
        return true;
    }
    return false;
}

//获取触摸设备物理尺寸
void AbstractTouchCalibrate::getTouchSize(const char* node, int& width, int& height)
{
    const char *udevSubsystems[] = {"input", NULL};
    GUdevClient *udevClient = g_udev_client_new(udevSubsystems);
    if (!udevClient) {
        SYS_LOG(LOG_DEBUG, " Failed to new udev client.");
        return;
    }
    GUdevDevice *udevDevice = g_udev_client_query_by_device_file(udevClient, node);

    if (g_udev_device_has_property(udevDevice, "ID_INPUT_WIDTH_MM")) {
        width = g_udev_device_get_property_as_uint64(udevDevice, "ID_INPUT_WIDTH_MM");
    }
    if (g_udev_device_has_property(udevDevice, "ID_INPUT_HEIGHT_MM")) {
        height = g_udev_device_get_property_as_uint64(udevDevice, "ID_INPUT_HEIGHT_MM");
    }
    g_clear_object(&udevClient);
}

void AbstractTouchCalibrate::autoMaticMapping(TouchDeviceList &touchList, ScreenInfoMap &screenMap, bool isTouchScreen)
{
    //如果配置文件无法覆盖所有的屏幕组合，继续通过比较触摸设备和屏幕物理尺寸进行映射
    TouchDeviceList::iterator it_tc = touchList.begin();
    while (it_tc != touchList.end()) {
        if ((*it_tc)->isMapped) {
            ++it_tc;
            continue;
        }
        ScreenInfoMap::iterator it_sc = screenMap.begin();
        while (it_sc != screenMap.end()) {

            //屏幕映射后，不再进行映射,触摸笔无需判断，多个可映射在同一屏幕
            if ((it_sc.value()->isMapped && isTouchScreen) || (*it_tc)->isMapped) {
                ++it_sc;
                continue;
            }

            if (checkMatch(it_sc.value()->width, it_sc.value()->height, (*it_tc)->width, (*it_tc)->height)) {
                calibrateDevice((*it_tc)->calibrateName, it_sc.value()->name);
                (*it_tc)->isMapped = true;
                it_sc.value()->isMapped = true;
            }
            ++it_sc;
        }
        ++it_tc;
    }
    //通过配置文件，尺寸比较后，仍然无法映射所有触摸设备与屏幕，则对剩下的设备进行随机映射
    it_tc = touchList.begin();
    while (it_tc != touchList.end()) {
        if ((*it_tc)->isMapped) {
            ++it_tc;
            continue;
        }
        ScreenInfoMap::iterator it_sc = screenMap.begin();
        while (it_sc != screenMap.end()) {
            if (it_sc.value()->isMapped || (*it_tc)->isMapped) {
                ++it_sc;
                continue;
            }
            //此处不再比较触摸设备与屏幕大小，为映射过的设备和屏幕进行随机映射
            calibrateDevice((*it_tc)->calibrateName, it_sc.value()->name);
            ++it_sc;
        }
        ++it_tc;
    }
}


