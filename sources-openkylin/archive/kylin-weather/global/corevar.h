/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef COREVAR_H
#define COREVAR_H

#include <QString>
#include <QGSettings>
#include <QDir>
#include <QSize>
#include <QPixmap>
#include <QStandardPaths>
#include <QDebug>
#include <unistd.h>
#include <QTranslator>

//当前城市
struct CurrentWeather
{
    QString id = "101010100";   //城市id
    QString city = "N/A";       //城市名字
    QString updatetime = "N/A"; //更新时间--服务器时间
    QString cond_code = "999";  //实况天气状况代码 	默认999
    QString cond_txt = "N/A";   //实况天气状况描述 晴
    QString hum = "N/A";        //相对湿度 40
    QString tmp = "N/A";        //温度，默认单位：摄氏度℃ 	21
    QString wind_dir = "N/A";   //风向 	西北
    QString wind_sc = "N/A";    //风力 3 (0:静风  1:1级风)
    QString serveTime = "N/A";  //天气信息的更新时间
};
//本地城市列表
struct LocationData
{
    QString id;                //城市id
    QString city_en;           //城市/县区-英文
    QString city;              //城市/县区-中文
    QString country_en;        //国家-中文
    QString country;           //国家-中文
    QString province_en;       //省-英文
    QString province;          //省-中文
    QString admin_district_en; //市-英文
    QString admin_district;    //市-中文
    QString shorthand;         //首字母缩写
};
//每一个收藏列表item的信息
struct CollectItemData
{
    QString cityId = "";
    QString cityName = "";
    QString cond_code = "999"; //实况天气状况代码--转换为icon
    QString tmp = "-";         //温度
};
//每一个搜索列表item的信息
struct ItemData
{
    QString cityId = "";
    QString cityName = "";
};
//某一天的详细信息
struct oneDay
{
    QString cond_code_d = "999";  //白天天气状况代码 	默认999
    QString cond_code_n = "999";  //晚间天气状况代码 	默认999
    QString cond_txt_d = "N/A";   //白天天气状况描述 	晴
    QString cond_txt_n = "N/A";   //晚间天气状况描述 	晴
    QString forcast_date = "N/A"; //预报日期 	2013-12-30
    QString wind_dir = "N/A";     //风向 	西北风
    QString wind_sc = "N/A";      //风力 	1-2 (级风)
    QString date = "N/A";         //日期
    QString dateTime = "N/A";     //当前城市信息更新日期
    QString tmp_max = "N/A";      //最高气温
    QString tmp_min = "N/A";      //最低气温
};

//预测天气
struct ForecastWeather
{
    oneDay first;
    oneDay second;
    oneDay third;
    oneDay fourth;
    oneDay fifth;
    oneDay sixth;
    oneDay seventh;
};
// QList<oneDay> ForecastWeather;
//逐小时天气
struct Hourly
{
    QString cond_code = "999"; //	"100" 默认999
    QString cond_txt = "N/A";  //	"晴"
    QString time = "N/A";      //	"2021-09-24 16:00"
    QString tmp = "N/A";       //	"36"
};

//生活指数
struct LifeStyle
{
    QString air_brf = "N/A";   //空气指数
    QString cw_brf = "N/A";    //洗车指数
    QString drsg_brf = "N/A";  //穿衣指数
    QString flu_brf = "N/A";   //感冒指数
    QString sport_brf = "N/A"; //运动指数
    QString uv_brf = "N/A";    //紫外线指数
};
// QMap <QString,ItemData>SearchList;
Q_DECLARE_METATYPE(CurrentWeather)  // QVarant 注册自定义类型
Q_DECLARE_METATYPE(LocationData)    // QVarant 注册自定义类型
Q_DECLARE_METATYPE(ForecastWeather) // QVarant 注册自定义类型
Q_DECLARE_METATYPE(LifeStyle)       // QVarant 注册自定义类型
Q_DECLARE_METATYPE(ItemData)        // QVarant 注册自定义类型
Q_DECLARE_METATYPE(CollectItemData) // QVarant 注册自定义类型
Q_DECLARE_METATYPE(oneDay)          // QVarant 注册自定义类型
Q_DECLARE_METATYPE(Hourly)          // QVarant 注册自定义类型


enum CityType {
    None = 0,
    Location,
    Collection,
    Other,
};

class CoreVar : public QObject
{

public:
    static const QString PROGRAM_NAME;                  //项目名称
    static const QString WEATHER_DBUS_SERVICENAME;      // DBUS服务名称
    static const QString WEATHER_DBUS_PARH;             // DBUS路径
    static const QString WEATHER_DBUS_INTERFACE;        // DBUS接口
    static const QString WEATHER_GSETTINGS_SERVICENAME; // QGSettings服务名称
    static const QString PCONLINEURL;                   //高德定位url---得到ip
    static const QString IPIP;                          // ipip定位url---得到ip
    static QString URLHEAD;                             //除ip外完整定位url
    //    static const QString URLKEY;                        // url的key--需要申请
    static const QString SERVICEREQUEST;             //服务器url
    static const QString SERVICEREQUEST_COLLECT;     //服务器收藏城市url
    static const QMap<QString, QString> SUPPORT_CMD; //支持的命令列表

    static const int REFRESH_RATE; //刷新间隔


    static const QString API_TYPE;
    static const int SEARCHCITYNUM;                                     //搜索城市列表2页的数量
    static const QSize CITYITEM;                                        //城市item的大小
    static void setSettings(const QString &key, const QVariant &value); //写入配置文件
    static QVariant getSettings(const QString &key);                    //读取配置文件
    static const QStringList WEATHER_CODE_TMP;                          //临时用--逐小时
    static const QString DEFAULT_CITYID;
    static const int DEFAULT_TIMEOUT; //刷新间隔

    // 标题栏搜索列表
    static int TITLE_SEARCH_LIST_NUM; // 搜索城市，列表个数

    // 当前城市信息
    static bool GET_CITY_INFO;               // 得到城市信息（默认为false）
    static CityType CURRENT_CITY_TYPE;       // 当前城市的类别（定位1/收藏2/其他3）
    static QString CURRENT_CITY_ID;          // 当前城市的城市ID
    static QString CURRENT_CITY_NAME;        // 当前城市的城市名称
    static QString LOCATION_CITY_ID;         // 定位城市的ID
    static QStringList CURRENT_COLLECT_LIST; // 收藏城市的ID列表

    // 当前是否为标题栏搜索城市
    static bool CURRENT_TITLE_SEARCH;
    //网络状态
    static bool HAS_NETWORK;

private:
    static QGSettings *m_settings;                 //配置文件对象
    static QGSettings *getSettings();              //初始化gsettings
    static QMap<QString, QString> getSupportCmd(); //创建支持的命令列表
    static QStringList temList();                  //临时用--逐小时
};
#endif // VARIABLE_H
