/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 * Copyright (C) 2012-2014 Daniel Vrátil <dvratil@redhat.com>
 * Copyright (C) 2012 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef USDBASECLASS_H
#define USDBASECLASS_H
#include <QFile>
#include <QObject>
#include <QMetaEnum>
#include "usd_global_define.h"

extern "C" {
#include "clib-syslog.h"
}

//lenovo N70Z
#define EC_TOUCHPADSTATE            "/sys/devices/platform/lenovo_ec/touchpad"  //触摸板状态节点
#define PERFORMANCE_MODE            "/sys/devices/platform/lenovo_ec/mode"  //性能模式节点
#define GRAPHICS_CARD_UEVENT        "/sys/class/drm/card0/device/uevent"
#define GRAPHICS_CARD_UEVENT1       "/var/lib/lightdm-data/kylin/usd/drm"
#define BRIGHTNESS_NODE_PATH        "/sys/class/backlight/"
class UsdBaseClass: public QObject
{
    Q_OBJECT

public:
    UsdBaseClass();
    ~UsdBaseClass();
    enum eScreenMode {
        firstScreenMode = 0,
        cloneScreenMode,
        extendScreenMode,
        secondScreenMode,
        showKDS,
        ERROR = 0x99};

    Q_ENUM(eScreenMode)

    enum BatteryState {
        Connected = 1,
        Disconnect};

    Q_ENUM(BatteryState)

    enum PowerMode {
        PerformanceMode = 0,
        AutoMode,
        EcoMode};

    Q_ENUM(PowerMode)

    enum eOsdType {
        keyboardIcon = 0,
        mediaIcon,
        mediaBar};

    Q_ENUM(eOsdType)

    static bool isTablet();

    static bool isTableWithDBus();

    static bool isMasterSP1();

    static bool is9X0();

    static bool isWayland();

    static bool isWlcom();

    static bool isXcb();

    static bool isNotebook();

    static bool isUseXEventAsShutKey();

    static bool readPowerOffConfig();

    static bool isPowerOff();

    static bool isJJW7200();

    static int getDPI();

    static double getScaleWithSize(int heightmm, int widthmm, int height, int width);

    static double getScale(double scaling);

    static double getScoreScale(double scaling);

    static void writeUserConfigToLightDM(QString group, QString key, QVariant value, QString userName = "");

    static QVariant readUserConfigToLightDM(QString group, QString key, QString userName = "");

    static bool isWaylandWithKscreen();

    /*
     * 针对EC自己控制的硬件设备，usd先获取控制权是否在EC，然后读取状态进行状态显示。
    */
    static bool brightnessControlByHardware(int &step);

    static bool flightModeControlByHardware(int &mode);

    static bool touchpadControlByHardware(int &state);

    static bool powerModeControlByHardware(int &mode);

    static QVariant readInfoFromFile(QString filePath);

    static QString readHashFromFile(QString filePath);

    static bool isVirt();

    static int getConnectOutputCount();

    /**
     * @brief getConnectOutputHash
     * @return
     */
    static QString getConnectOutputHash();

    /**
     * @brief getConnectOutputHash
     * @return
     */
    static QString getOutputHash(QString name);

    /**
     * @brief getConnectOutputHashWithPort 获取已链接的hash。
     * @return
     */
    static QString getConnectOutputHashWithPort();

    /**
     * @brief parseVendor
     * @param data
     * @return
     */
    static QString parseVender(const QByteArray &rawData);

    static QString getOutputVender(const QString outputTypeName);

    /**
     * @brief writeGlobalConfig
     * @param path,全局配置下的路径
     * @param fileName，全局配置以及路径下的文件名
     * @param info，内容
     * @return
     */
    static bool writeGlobalConfig(QString path, QString fileName, QByteArray info);

    /**
     * @brief writeGlobalConfig
     * @param path,全局配置下的路径
     * @return
     */
    static QByteArray readGlobalConfig(QString path);

    static QByteArray readGlobalConfigWithDbus(QString path);

    static bool globalConfigExists(QString path);

    static bool globalConfigExistsWithDbus(QString path);

    static double getDisplayScale();

    static double getPreferredScale(double widthMM, double heightMM, int resolutionWidth, int resolutionHeight, double scale = 0);

    static bool isSupportGamma();

    static bool isSupportGammaWithLspci();

    static bool inTrialMode();

    static QString getFileHash(QString filePath);

    static QString updateSecurityConfig(QString filePath);

    /**
     * @brief clearSecurityConfig
     * @return 清空安全管控配置文件，退域。
     */
    static QString clearSecurityConfig();
    /**
     * @brief getSecurityGsettingsNode
     * @return
     */
    static const QStringList getSecurityGsettingsNode();

    /**
     * @brief peekDir
     * @param dirPath
     * @param permission
     * @return
     */
    static bool peekDir(QString dirPath, QFileDevice::Permission permission);

    static bool setAttributesToAppendOnly(QString path);

    static bool unsetAttributesToAppendOnly(QString path);

    static const QStringList get41NodeSchema();

    static bool upmSupportAdjustBrightness();

    static QMap<QString,QString> getPnpMap();

    static void printTimeWithMsg(QString msg, QString fun, int line);
private:
    static QString readAllText(QString filePath);
    static bool isSupportGammaWithPciID(QString ueventInfo);
    //com.kylin.Wlcom /com/kylin/Wlcom/Renderer com.kylin.Wlcom.Renderer IsHardwareRendering
    static bool isSupportGammaWithRender();
    static double getNativeScaleFactor(double widthMM, double heightMM, int resolutionWidth, int resolutionHeight);
    static double getPreferredScaleFactor(double nativeDpi, int resolutionWidth, int resolutionHeight);
};


#if 0

参考：
https://blog.csdn.net/s1_mple/article/details/145780031
https://blog.csdn.net/s1_mple/article/details/145780031
原数据包
00ffffffffffff0030aec26501010101
2a1e01046c301b782e2755a3544f9e27
115054bfef0081809500a9c0b300714f
818a010101011a3680a070381f403020
3500dc0c1100001a000000ff00553536
31394842380a20202020000000fd0032
4b1e5311000a202020202020000000fc
        004c454e204c4932323234410a2000d9

解包：
包头：00 ff ff ff ff ff ff 00
----------------------------------
Vendor&Porduct：30 ae  c2 65 01 01 01 01 2a 1e
ID Manufacturer Name（厂商编号）：30 ae
30 = 0011 0000 1010 1110
组合就是：
0
01100 = （12）L
00101 = （5）E
01110  =（14）N
（00001）=A
（11010）=Z
-------------------------------------
ID product Code（型号）：C2 65
------------------------------------
ID SerialNumaber：01 01 01 01
---------------------------------
week of manufacture ： 2A 1E
2A=42周，不用则写FF
1E=30 ：生产年份 = 1990+30
--------------------------------------------------------------
12~13字节EDID Structure Version & Revision: 2 Bytes : 01 04
edid版本
-------------------------------------------------------------------
14字节：Basic Display Parameters and Features: 5 Bytes：6c 30 1b 78 2e
Video Input Definition: 1 Byte 6C
0110 1101
高位标识0 标识模拟，1标识数字
--------------------------------------------
15~16字节：Horizontal and Vertical Screen Size or Aspect Ratio: 2 Bytes 30 1b
48cm，27cm
与之对应的是第66-68字节最大显示画面，如dc0c11
---------------------------------------------
Display Transfer Characteristics (GAMMA): 1 Byte:78
-----------------------------------
Feature Support: 1 Byte 2E
---------------------------------------

--------------------------------------------------------------------

#endif
class EdidParser: public QObject
{
    Q_OBJECT
public:
    EdidParser(const char *data, int length){
        quint16 serialValue = 0;
        serialValue = (unsigned char)data[EDID_OFFSET_PNPID] << 8 | (unsigned char)data[EDID_OFFSET_PNPID+1];
        m_pnpId.resize(3);
        m_pnpId[0] = ((serialValue >> 10) & 0x1F) + '@';
        m_pnpId[1] = ((serialValue >> 5) & 0x1F) + '@';
        m_pnpId[2] = ((serialValue >> 0) & 0x1F) + '@';

        if (data[0x13] == 0x04) {
            USD_LOG(LOG_DEBUG,"edid version:1.4");
        } else {
            USD_LOG(LOG_DEBUG,"edid version:2.0");
        }
        for (int i = 0; i < EDID_BYTE_DESCRIPTOR_COUNT; i++) {
            int descData = 0x36 + i * EDID_BYTE_DESCRIPTOR_SIZE;

            if ((unsigned char)data[descData+3] == DESCRIPTOR_DISPLAY_PRODUCT_NAME) {
                int len = 8;
                for (int k = 0; k < 15; k++) {
                    if ((unsigned char)data[descData+5+k] == 0x0a) {
                        len = k;
                    }
                }
                m_model = QString::fromLatin1(data+descData+5, len);
                break;
            }
        }
        if (m_model.isEmpty()) {
            m_model = "0x"+QString("%1%2").arg(data[0x0a],2,16,QLatin1Char('0')).toUpper().arg(data[0x0a],2,16,QLatin1Char('0')).toUpper();
        }
        USD_LOG_SHOW_PARAMS(m_model.toLatin1().data());
        USD_LOG_SHOW_PARAMS(m_pnpId.toLatin1().data());
    }
    ~EdidParser(){}
    QString getPnpId() {
        return m_pnpId;
    }

    QString getSerialId() {
        return m_model;
    }
private:
    QString edidParseString(const char *data) const
    {
        /* this is always 13 bytes, but we can't guarantee it's null
         * terminated or not junk. */
        auto text = QString::fromLatin1(reinterpret_cast<const char *>(data), 13).simplified();

        for (int i = 0; i < text.length(); ++i) {
            if (!text.at(i).isPrint()) {
                text[i] = QLatin1Char('-');
            }
        }
        return text;
    }
public:
    QString m_monitorName;
    QString m_serialNumber;
    QString m_model = "";
    QString m_pnpId = "";
    QString m_eisaId;
};

#endif // USDBASECLASS_H
