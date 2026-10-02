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
#include "corevar.h"

//项目名称
const QString CoreVar::PROGRAM_NAME = QString("kylin-weather");
// DBUS服务名称
const QString CoreVar::WEATHER_DBUS_SERVICENAME = QString("org.ukui.kylin_weather");
// DBUS路径
const QString CoreVar::WEATHER_DBUS_PARH = QString("/");
// DBUS接口
const QString CoreVar::WEATHER_DBUS_INTERFACE = QString("kylin_weather.commands");
//高德定位url
const QString CoreVar::PCONLINEURL = QString("http://whois.pconline.com.cn/");
// ipip定位url
const QString CoreVar::IPIP = QString("http://myip.ipip.net");
//除ip外完整定位url
QString CoreVar::URLHEAD = QString("http://restapi.amap.com/v5/");
// url的key--需要申请
// const QString CoreVar::URLKEY = QString("faf8faad4c9c070879bebe615eddeff4");
//服务器url
const QString CoreVar::SERVICEREQUEST =
    QString("http://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/%1/");
//收藏城市
const QString CoreVar::SERVICEREQUEST_COLLECT =
    QString("http://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_simple_s6/?cityids=");

//网络状态
bool CoreVar::HAS_NETWORK = false;

//服务器url -intel
// const QString CoreVar::SERVICEREQUEST = QString("http://edu.kylinos.cn:8001/weather/api/3.0/heweather_data_s6/%1/");
//收藏城市
// const QString CoreVar::SERVICEREQUEST_COLLECT =
// QString("http://edu.kylinos.cn:8001/weather/api/3.0/heweather_simple_s6/?cityids=");

// QGSettings服务名称
const QString CoreVar::WEATHER_GSETTINGS_SERVICENAME = QString("org.kylin-weather.settings");
// GSettings
QGSettings *CoreVar::m_settings = CoreVar::getSettings();
//支持的命令列表
const QMap<QString, QString> CoreVar::SUPPORT_CMD = CoreVar::getSupportCmd();
const int CoreVar::REFRESH_RATE = 15;               //刷新间隔
const int CoreVar::SEARCHCITYNUM = 42;              //搜索城市列表2页的数量
const QSize CoreVar::CITYITEM = QSize(156, 36);     //城市item的大小
const QString CoreVar::API_TYPE = QString("$api$"); //是否为外部作为API调用的标识
const QStringList CoreVar::WEATHER_CODE_TMP = CoreVar::temList();
const QString CoreVar::DEFAULT_CITYID = QString("101010100");
const int CoreVar::DEFAULT_TIMEOUT = 30000; // 30s

bool CoreVar::GET_CITY_INFO = false;                       // 得到城市信息（默认为false）
CityType CoreVar::CURRENT_CITY_TYPE = CityType::None;     // 当前城市的类别（定位1/收藏2/其他3）
QString CoreVar::CURRENT_CITY_ID = "";                     // 当前城市的城市ID
QString CoreVar::CURRENT_CITY_NAME = "";                   // 当前城市的城市名称
QString CoreVar::LOCATION_CITY_ID = QString("101010100");  // 定位城市的ID
QStringList CoreVar::CURRENT_COLLECT_LIST;                 // 收藏城市的ID列表

// 当前是否为标题栏搜索城市
bool CoreVar::CURRENT_TITLE_SEARCH = false;

// 标题栏搜索列表
int CoreVar::TITLE_SEARCH_LIST_NUM = 1;

QGSettings *CoreVar::getSettings()
{
    QGSettings *mysetting = nullptr;
    if (QGSettings::isSchemaInstalled(WEATHER_GSETTINGS_SERVICENAME.toLocal8Bit())) {
        mysetting = new QGSettings(WEATHER_GSETTINGS_SERVICENAME.toLocal8Bit());
    }
    return mysetting;
}

QMap<QString, QString> CoreVar::getSupportCmd()
{
    QMap<QString, QString> cmds;
    cmds.insert("-changeCity", "切换城市");
    return cmds;
}

QStringList CoreVar::temList()
{
    QStringList tmp;
    tmp << "100"
        << "101"
        << "102"
        << "103"
        << "104"
        << "150"
        << "153"
        << "154"
        << "300"
        << "301"
        << "302"
        << "303"
        << "304"
        << "305"
        << "306"
        << "307"
        << "308"
        << "309"
        << "310"
        << "311"
        << "312"
        << "313"
        << "314"
        << "315"
        << "316"
        << "317"
        << "318"
        << "399"
        << "350"
        << "351"
        << "400"
        << "401"
        << "402"
        << "403"
        << "404"
        << "405"
        << "406"
        << "407"
        << "408"
        << "409"
        << "410"
        << "499"
        << "456"
        << "457"
        << "500"
        << "501"
        << "502"
        << "503"
        << "504"
        << "507"
        << "508"
        << "509"
        << "510"
        << "511"
        << "512"
        << "513"
        << "514"
        << "515"
        << "900"
        << "901"
        << "101";
    return tmp;
}


//写入配置文件
void CoreVar::setSettings(const QString &key, const QVariant &value)
{
    m_settings->set(key, value);
}

//读取配置文件
QVariant CoreVar::getSettings(const QString &key)
{
    QVariant setting;
    setting = m_settings->get(key);
    return setting;
}
