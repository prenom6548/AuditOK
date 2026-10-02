/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef GAMMAMANAGERHELPER_H
#define GAMMAMANAGERHELPER_H

#include <QTime>

#include <QtMath>
#include <QDebug>
#include <QSizeF>
#include <QString>
#include <QX11Info>
#include <QObject>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QScreen>

#include "clib-syslog.h"
#include "gamma-manager-define.h"
#include "gamma-manager-adaptor.h"
#include "rgb-gamma-table.h"
#include "usd_base_class.h"
#include "QGSettings/qgsettings.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <math.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
#ifdef __cplusplus
}
#endif

#ifdef Bool
#undef Bool
#endif

typedef struct _OutputInfo{
    QString name;
    QString vender;
    bool connectState;
    bool isPrimary;
    uint targetTemp;
    uint lastTemp;
    uint rtTemp;
    double targetBrightness;
    double lastBrightness;
    double rtBrightness;
}OutputInfo;

struct WeatherInfo {
    int tmp;           // 温度
    int cond_code;     // 天气状况代码
    QString cond_txt;  // 天气状况描述
    int fl;            // 体感温度
    int hum;           // 湿度
    float pcpn;        // 降水量
    int pres;          // 大气压强
    int vis;           // 能见度
    int wind_deg;      // 风向角度
    QString wind_dir;  // 风向
    int wind_sc;       // 风力等级
    float wind_spd;    // 风速
    int cloud;         // 云量
    QString sunrise;   // 日出时间
    QString sunset;    // 日落时间
    QString id;        // 城市ID
    QString location;  // 地点名称
};

#if 0
struct outputMsg{
    bool enbale;
    int width;
    int height;
    int refresh;
    double scale;
    int transform;
    int vrr_policy;
    int lx;
    int ly;
    int brightness;
    int color_temp;
};

struct outputStruct{
    QString outputName;
    outputMsg outputInfo;
};

typedef QList<outputStruct> outputStructList;
Q_DECLARE_METATYPE(outputStruct)
Q_DECLARE_METATYPE(outputStructList)


inline QDBusArgument &operator << (QDBusArgument &argument, const outputStruct &output)
{
    argument.beginStructure();
    argument << output.outputName;
    argument << output.outputInfo.enbale;
    argument << output.outputInfo.width;
    argument << output.outputInfo.height;
    argument << output.outputInfo.refresh;
    argument << output.outputInfo.scale;
    argument << output.outputInfo.transform;
    argument << output.outputInfo.vrr_policy;
    argument << output.outputInfo.lx;
    argument << output.outputInfo.ly;
    argument << output.outputInfo.brightness;
    argument << output.outputInfo.color_temp;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, outputStruct &output) {
    argument.beginStructure();
    argument >> output.outputName;
    argument >> output.outputInfo.enbale;
    argument >> output.outputInfo.width;
    argument >> output.outputInfo.height;
    argument >> output.outputInfo.refresh;
    argument >> output.outputInfo.scale;
    argument >> output.outputInfo.transform;
    argument >> output.outputInfo.vrr_policy;
    argument >> output.outputInfo.lx;
    argument >> output.outputInfo.ly;
    argument >> output.outputInfo.brightness;
    argument >>  output.outputInfo.color_temp;
    argument.endStructure();
    return argument;
}
#else

struct OutputStruct{
    QString outputName;
    QString outputInfo;
};

typedef QList<OutputStruct> OutputStructList;
Q_DECLARE_METATYPE(OutputStruct)
Q_DECLARE_METATYPE(OutputStructList)



inline QDBusArgument &operator << (QDBusArgument &argument, const OutputStruct &output)
{
    argument.beginStructure();
    argument << output.outputName;
    argument << output.outputInfo;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, OutputStruct &output) {
    argument.beginStructure();
    argument >> output.outputName;
    argument >> output.outputInfo;
    argument.endStructure();
    return argument;
}


#endif
class GmHelper: public QObject
{
    Q_OBJECT
public:
    GmHelper(QObject *parent = nullptr);
    ~GmHelper();

    /**
     * @brief getLonAndLatByJson 根据服务器地址，提取经纬度信息
     * @param url
     * @param bytes
     * @param psize
     * @return
     */
    bool getLonAndLatByJson(QString url, QByteArray bytes, QSizeF& psize);

    /**
     * @brief getSunriseSunset 计算指定日期日出日落
     * @param rtDate
     * @param location
     * @param SunriseSunset
     * @return
     */
    bool getSunriseSunset(QDateTime &rtDate,QSizeF &location, QSizeF &SunriseSunset);

    /**
     * @brief getRtSunriseSunset 计算当前日期的日出日落
     * @param location
     * @param SunriseSunset
     * @return
     */
    bool getRtSunriseSunset(QSizeF &location, QSizeF &SunriseSunset);

    /**
     * @brief getRgbWithTemperature 色温值转化gamma所需的RGB
     * @param temp
     * @param result
     * @return
     */
    bool getRgbWithTemperature(double temp, ColorRGB &result);


    /**
     * @brief getTempInterpolate
     * @param svalue
     * @param bvalue
     * @param value
     * @return
     */
    uint getTempInterpolate(const double svalue, const double bvalue, double value);

    /**
     * @brief getTemperatureWithRgb
     * @param red
     * @param green
     * @param blue
     * @return
     */
    uint getTemperatureWithRgb(const double red, const double green, const double blue);

    /**
     * @brief setAllOutputsBrightness
     * @param brightness
     */
    void setAllOutputsBrightness(const uint brightness);

    /**
     * @brief setBrightness
     * @param outputName
     * @param brightness
     */
    void setBrightness(const QString outputName, const double brightness);

    /**
     * @brief getAllOutputsInfo
     * @return
     */
    OutputGammaInfoList getAllOutputsInfo();

    /**
     * @brief getOutputInfo 为设置单个屏幕的亮度，外部方法先获取outputinfo，然后逐个显示设置targetVaue，并根据target与运行次数设置rtvalue
     * @return
     */
    QList<OutputInfo>& getOutputInfo();

    /**
     * @brief setTemperature 设置gamma，便利outputinfo的rtvalue与
     * @param temp
     */
    bool setGammaWithTemp(const uint rtTemp);

    /**
     * @brief freeScreenResource
     */
    void freeScreenResource();


    /**
     * @brief setPrimaryBrightness
     */
    bool setPrimaryBrightness(int brightness);


    /**
     * @brief getPrimaryBrightness
     */
    int getPrimaryBrightness();

    /**
         * @brief enablePrimarySignal
         * @param state
         * @return
         */
    void enablePrimarySignal(int state);

    /**
     * @brief recheckPrimary
     */
    void recheckPrimary();


    /**
     * @brief initOutput
     * @return
     */
    QList<OutputInfo> initOutput();


    /**
     * @brief sendPrimaryChanged
     * @param brightness
     */
    void sendPrimaryChanged(int brightness, QString signalName);

    /**
     * @brief updateOutputInfo 由helper区分显示服务器。
     * @param name
     * @param state
     */
    void updateOutputInfo(QString outputName, int state, int colorTemp);

    /**
     * @brief getTemperatureInitValue
     * @return
     */
    int getTemperatureInitValue();

    /**
     * @brief getGlobalBrightness
     * @param outputName
     * @return
     */
    int getGlobalBrightness(QString outputName);
private Q_SLOTS:
    /**
     * @brief doWlcomInitTimeOut wlcom 初始化定时器，xrandr不一定比这个组件先启动。并且存在不接入屏幕启动的情况
     */
    void doWlcomInitTimeOut();
private:

    /**
     * @brief getRgbInterpolate
     * @param p1
     * @param p2
     * @param index
     * @param result
     */
    void getRgbInterpolate(const ColorRGB &p1, const ColorRGB &p2, double index, ColorRGB &result);

    /**
     * @brief getLonAndLatMozilla
     * @param jsonBytes
     * @param psize
     * @return
     * //{"location": {"lat": 39.9075, "lng": 116.3972}, "accuracy": 49000.0}
     */
    bool getLonAndLatMozilla(QByteArray jsonBytes, QSizeF& psize);

     /**
     * @brief getLonAndLatIPAPI
     * @param jsonBytes
     * @param psize
     * @return
     *  //{"status":"success","country":"China","countryCode":"CN","region":"TJ","regionName":"Tianjin","city":"Tianjin","zip":"","lat":39.3434,"lon":117.362,"timezone":"Asia/Shanghai","isp":"ASN for TIANJIN Provincial Net of CT","org":"Chinanet TJ","as":"AS17638 ASN for TIANJIN Provincial Net of CT","query":"123.150.8.42"}
     */
    bool getLonAndLatIPAPI(QByteArray jsonBytes, QSizeF& psize);

    /**
     * @brief getLonAndLatIPAPI
     * @param jsonBytes
     * @param psize
     * @return
     */
    bool getSunriseAndSunsetWithWeatherUrl(QByteArray jsonBytes, QSizeF& psize);

    /**
     * @brief sendEnableChanged
     * @param brightness
     */
    void sendEnableChanged(int state);

    /*X11*/
    /**
     * @brief initOutput
     * @return
     */
    QList<OutputInfo> initX11Output();

    /**
     * @brief getAllOutputsInfo
     * @return
     */
    OutputGammaInfoList getX11AllOutputsInfo();



    /**
     * @brief setX11PrimaryBrightness
     */
    bool setX11PrimaryBrightness(int brightness);


    /**
     * @brief setTemperature 设置gamma，便利outputinfo的rtvalue与
     * @param temp
     */
    bool setX11GammaWithTemp(const uint rtTemp);

    /**
     * @brief updateXOutputInfo 更新x11的output
     */
    void updateXOutputInfo(QString outputName, int state, int colorTemp);


    /**
     * @brief getPrimaryBrightness
     */
    int getX11PrimaryBrightness();
    /*wlcom*/
    /**
     * @brief initOutput
     * @return
     */
    QList<OutputInfo> initWlComOutput();


    /**
     * @brief getAllOutputsInfo
     * @return
     */
    OutputGammaInfoList getWlComAllOutputsInfo();


    /**
     * @brief setX11PrimaryBrightness
     */
    bool setWlcomPrimaryBrightness(int brightness);


    /**
     * @brief setTemperature 设置gamma，便利outputinfo的rtvalue与
     * @param temp
     */
    bool setWlcomTemp(const uint rtTemp);

    /**
     * @brief updateWlcomOutputInfo 更新wlcom显示器
     * @param name
     * @param state
     */
    void updateWlcomOutputInfo(QString outputName, int state, int colorTemp);

    /**
     * @brief getWlcomPrimaryBrightness
     */
    int getWlcomPrimaryBrightness();

    /**
     * @brief doScreensParamChanged
     * @param param
     */
    void doScreensParamChanged(QString param);

    /**
     * @brief getScreenInfo 从xrandr组件中解析出所有已链接屏幕名称以及主屏名称
     * @param param
     */
    void getScreenInfo(const QString &param, QStringList &outputList, QString &primaryOutput);


    /**
     * @brief syncBrightnessToConfig 把亮度存入配置。
     */
    void syncBrightnessToConfig();

    /**
     * @brief syncBrightnessFromConfig 把亮度读取出来。
     */
    void syncBrightnessFromConfig();


Q_SIGNALS:
    void outputInitSuccess();
private:
    int normalizeBrightness(int userValue);
    int denormalizeBrightness(int wlcomValue);
    QString getPrimaryScreenNameFormXrandr();

    /** xx:xx:xx to xx.xx
     * @brief timeToDouble
     * @param timeStr
     * @return
     */
    double timeToDouble(QString timeStr, bool &isOk);
private:
    const int c_minBrightness = 30;
    const int c_maxBrightness = 100;
    const double c_convertFactor = 0.7;

private:
    double deg2rad (double degrees);
    double  rad2deg (double radians);
    QString  getVendor(QString outputTypeName);
    XRRScreenResources  *m_pScreenRes = nullptr;
    QList<OutputInfo> m_outputList;
    uint m_brightness;
    uint m_temperature;
    bool m_powerManagerSetable = false;
    ColorRGB m_colorRGB;
    bool m_sendPrimaryBrightnessSignal = true;

    QTimer *m_wlComInitTimer = nullptr;

    QDBusInterface *m_wlComItr;
    QDBusInterface *m_xrandrInterface;
    QMutex  m_outputListMutex;
};

#endif // GAMMAMANAGERHELPER_H
