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
#include "dataparser.h"
#include <QDateTime>

DataParser::DataParser() {}

void DataParser::dataAnalysis(QByteArray dataBa)
{
    qDebug() << " ##### Function Positioning ##### 处理返回的网络数据 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //数据异常
    QJsonParseError jsonParserErr;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(dataBa, &jsonParserErr);
    if (jsonParserErr.error != QJsonParseError::NoError) { // Json type error
        ErrorReact::getInstance()->networkFeedBack(false, jsonParserErr.errorString());
        return;
    }
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        ErrorReact::getInstance()->networkFeedBack(false, "Json null or empty!");
        return;
    }
    // jsonObject数据为空
    QJsonObject jsonObject = jsonDocument.object();
    if (jsonObject.isEmpty() || jsonObject.size() == 0) {
        ErrorReact::getInstance()->networkFeedBack(false, "Json object null or empty!");
        return;
    }
    //无KylinWeather
    if (!jsonObject.contains("KylinWeather")) {
        //        ErrorReact::getInstance()->networkFeedBack(false,"服务器数据没有字段");
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
        return;
    }

    QJsonObject mainObj = jsonObject.value("KylinWeather").toObject();
    if (mainObj.isEmpty() || mainObj.size() == 0) {
        ErrorReact::getInstance()->networkFeedBack(false, "Json object null or empty!");
        return;
    }

    //解析当前城市天气状况和一周预报天气
    if (mainObj.contains("weather")) {
        QJsonObject weatherObj = mainObj.value("weather").toObject();
        if (!weatherObj.isEmpty() && weatherObj.size() > 0) {
            m_currentWeather = currentWeather(weatherObj);
            forcastWeather(weatherObj, m_currentWeather);
        }
    } else {
        success = false;
    }

    //解析生活指数信息
    if (mainObj.contains("lifestyle")) {
        QJsonObject lifestyleObj = mainObj.value("lifestyle").toObject();
        lifeStyle(lifestyleObj);
    }

    //解析逐小时天气信息
    if (mainObj.contains("hourly")) {
        QJsonObject hourlyObj = mainObj.value("hourly").toObject();
        hourlist = hourlyWeather(hourlyObj);
    } else {
        success = false;
    }

}
//处理返回的网络数据
void DataParser::dataReply(QByteArray dataBa)
{
    dataAnalysis(dataBa);
    if (success){
        emit showCurrent(m_currentWeather);
        QVariant var;
        var.setValue<QList<Hourly>>(hourlist);
        emit showHourly(var);
        //将当前城市信息存入配置文件
        if (hourlist.length() > 0) {
            QStringList serveTimeList = m_currentWeather.serveTime.split(' ');
            if (hourlist.first().time.length() > 1 && serveTimeList.length() > 0) {
                m_currentWeather.serveTime = serveTimeList.first() + " " + hourlist.first().time.mid(0);
            }
        }
        setGsettings(m_currentWeather);
    } else {
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
    }
}

void DataParser::dataReplyL(QByteArray dataBa)
{
    dataAnalysis(dataBa);
    if (success){
        emit showLeftCity(m_currentWeather);
    } else {
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
    }
    return;
}

void DataParser::dataReplyR(QByteArray dataBa)
{
    dataAnalysis(dataBa);
    if (success){
        emit showRightCity(m_currentWeather);
    } else {
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
    }
    return;
}

void DataParser::dataReplyColl(QByteArray dataBa)
{
    QJsonParseError jsonParserErr;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(dataBa, &jsonParserErr);
    if (jsonParserErr.error != QJsonParseError::NoError) { // Json type error
        ErrorReact::getInstance()->networkFeedBack(false, jsonParserErr.errorString());
        return;
    }
    if (jsonDocument.isNull() || jsonDocument.isEmpty()) {
        ErrorReact::getInstance()->networkFeedBack(false, "Json null or empty!");
        return;
    }

    QJsonObject jsonObject = jsonDocument.object();
    if (jsonObject.isEmpty() || jsonObject.size() == 0) {
        ErrorReact::getInstance()->networkFeedBack(false, "Json object null or empty!");
        return;
    }
    if (jsonObject.contains("KylinWeather")) {
        QJsonObject mainObj = jsonObject.value("KylinWeather").toObject();
        if (mainObj.isEmpty() || mainObj.size() == 0) {
            return;
        }
        if (mainObj.contains("weather")) {
            QString weather_msg = mainObj.value("weather").toString();
            if (weather_msg != "") {
                collectWeather(weather_msg);
            }
        }
    }
}
//解析生活指数信息
void DataParser::lifeStyle(QJsonObject lifestyleObj)
{
    LifeStyle m_lifestyle;
    if (!lifestyleObj.isEmpty() && lifestyleObj.size() > 0) {
        m_lifestyle.air_brf = lifestyleObj.value("air_brf").toString();
        m_lifestyle.cw_brf = lifestyleObj.value("cw_brf").toString();
        m_lifestyle.drsg_brf = lifestyleObj.value("drsg_brf").toString();
        m_lifestyle.flu_brf = lifestyleObj.value("flu_brf").toString();
        m_lifestyle.sport_brf = lifestyleObj.value("sport_brf").toString();
        m_lifestyle.uv_brf = lifestyleObj.value("uv_brf").toString();
    } else {
        m_lifestyle.air_brf = "N/A";
        m_lifestyle.cw_brf = "N/A";
        m_lifestyle.drsg_brf = "N/A";
        m_lifestyle.flu_brf = "N/A";
        m_lifestyle.sport_brf = "N/A";
        m_lifestyle.uv_brf = "N/A";
        //        ErrorReact::getInstance()->networkFeedBack(false,"服务器返回数据为空或不全");
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
    }
    emit showLifeStyle(m_lifestyle);
}
//解析当前城市信息
CurrentWeather DataParser::currentWeather(QJsonObject weatherObj)
{
    CurrentWeather currentWeather;
    qDebug() << " ##### Function Positioning ##### 解析当前城市信息 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //当前城市id
    QString id_msg = weatherObj.value("id").toString();
    if (id_msg != "") {
        currentWeather.id = id_msg;
    }
    //当前城市名字
    QString location_msg = weatherObj.value("location").toString();
    if (location_msg != "") {
        currentWeather.city = location_msg;
    }
    //当前城市需要显示的具体信息
    QString now_msg = weatherObj.value("now").toString();
    if (now_msg != "" && now_msg.contains(",", Qt::CaseInsensitive)) {
        QStringList strList = now_msg.split(",");
        QJsonObject m_json;
        foreach (QString str, strList) {
            if (str != "") {
                m_json.insert(str.split("=").at(0), str.split("=").at(1));
            }
        }
        currentWeather.tmp = m_json.value("tmp").toString();
        currentWeather.wind_sc = m_json.value("wind_sc").toString();
        currentWeather.cond_txt = m_json.value("cond_txt").toString();
        currentWeather.hum = m_json.value("hum").toString();
        currentWeather.cond_code = m_json.value("cond_code").toString();
        currentWeather.wind_dir = m_json.value("wind_dir").toString();
        currentWeather.serveTime = weatherObj.value("update_time").toString();
//        currentWeather.serveTime = QDateTime::currentDateTime().toString("hh:mm");
        qDebug() << " ##### Function Positioning ##### 解析当前城市信息 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    } else {

        currentWeather.tmp = "N/A";
        currentWeather.wind_sc = "N/A";
        currentWeather.cond_txt = "N/A";
        currentWeather.hum = "N/A";
        currentWeather.cond_code = "999";
        currentWeather.wind_dir = "N/A";
        currentWeather.serveTime = "N/A";
        //        ErrorReact::getInstance()->networkFeedBack(false,"服务器返回数据为空或不全");
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
        qDebug() << " ##### Function Positioning ##### 解析当前城市信息 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    }
    return currentWeather;
}
//解析预报天气信息
void DataParser::forcastWeather(QJsonObject weatherObj, CurrentWeather currentWeather)
{
    ForecastWeather forecastWeather;
    QStringList strList;
    int i = 0;
    //处理预报天气
    QString forecast_msg = weatherObj.value("forecast").toString();
    if (forecast_msg != "" && forecast_msg.contains(",", Qt::CaseInsensitive)) {
        strList = forecast_msg.split(";");
        foreach (QString strDay, strList) {
            QStringList strListDaySub;
            QJsonObject m_json;
            if (strDay != "") {
                strListDaySub = strDay.split(",");

                foreach (QString str, strListDaySub) {
                    if (str != "") {
                        m_json.insert(str.split("=").at(0), str.split("=").at(1));
                    }
                }
                QString dateTime = currentWeather.serveTime.split(" ").first();
                setCollectValue(i, m_json, forecastWeather, dateTime);
                i++;
            }
        }
    } else {
        //        ErrorReact::getInstance()->networkFeedBack(false,"服务器返回数据为空或不全");
        ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
    }
    emit showForcast(forecastWeather);
}

void DataParser::setGsettings(CurrentWeather currentWeather)
{
    //存配置文件
    QString weatherNow = "";
    weatherNow.append(currentWeather.serveTime + ","); //时间
    weatherNow.append(currentWeather.id + ",");        //省市编码
    weatherNow.append(currentWeather.city + ",");      //城市名称
    weatherNow.append(currentWeather.cond_txt + ",");  //天气情况
    weatherNow.append(currentWeather.hum + "%,");      //湿度
    weatherNow.append(currentWeather.tmp + "°,");      //温度
    weatherNow.append(currentWeather.wind_dir + ",");  //风向
    weatherNow.append(currentWeather.wind_sc + "级,"); //风力
    //    weatherNow.append(weatherObj.value("admin_area").toString()+",");//省份----暂时不需要
    weatherNow.append(currentWeather.cond_code + ",");
    CoreVar::setSettings("weather", weatherNow); //写入配置文件
}

void DataParser::collectWeather(QString weatherList)
{
    QList<CollectItemData> collectItemList;
    if (weatherList == "")
        return;
    QStringList strList = weatherList.split(";");
    QString weatherStr = "";
    for (int i = 0; i <= strList.length() - 1; i++) {
        weatherStr = strList.at(i);
        CollectItemData collectItemData;
        if (!weatherStr.isEmpty() && weatherStr.contains(",", Qt::CaseInsensitive)) {
            QJsonObject m_json;
            if (!weatherStr.isEmpty()) {
                QStringList eachKeyList = weatherStr.split(",");
                foreach (QString strKey, eachKeyList) {
                    if (!strKey.isEmpty()) {
                        //等号左边为键，右边为值
                        m_json.insert(strKey.split("=").at(0), strKey.split("=").at(1));
                    }
                }
            }
            collectItemData.tmp = m_json.value("tmp").toString() + QString("°");
            collectItemData.cond_code = m_json.value("cond_code").toString();
            collectItemData.cityId = m_json.value("id").toString();
            collectItemData.cityName = m_json.value("location").toString();
        } else {
            collectItemData.tmp = "-";
            collectItemData.cond_code = "999";
            collectItemData.cityId = "-";
            collectItemData.cityName = "-";
            //        ErrorReact::getInstance()->networkFeedBack(false,"服务器返回数据为空或不全");
            //返回的数据中带了一个空数据，所以将这一项去掉
            if (i != strList.length() - 1) {
                ErrorReact::getInstance()->networkFeedBack(false, tr("Data is empty"));
            }
        }
        collectItemList.append(collectItemData);
    }
    emit showCollectCode(collectItemList);
}

QList<Hourly> DataParser::hourlyWeather(QJsonObject hourlyObj)
{
    QList<Hourly> hourlist;
    if (hourlyObj.contains(QStringLiteral("hourly"))) {
        QJsonValue arrayValue = hourlyObj.value(QStringLiteral("hourly"));
        if (arrayValue.isArray()) {
            QJsonArray array = arrayValue.toArray();
            if (array.size() != 24) {
                ErrorReact::getInstance()->networkFeedBack(false, tr("hourly != 24"));
                return hourlist;
            }
            for (int i = 0; i < array.size(); i++) {
                QJsonValue jsonArray = array.at(i);
                QJsonObject m_json = jsonArray.toObject();
                Hourly tmp_hour;
                if (!(m_json.contains("icon") && m_json.contains("fxTime") && m_json.contains("temp"))) {
                    ErrorReact::getInstance()->networkFeedBack(false, tr("hourly key is wrong"));
                    return hourlist;
                }
                if (m_json.value("icon").toString() == "" || m_json.value("fxTime").toString() == ""
                    || m_json.value("temp").toString() == "") {
                    tmp_hour.cond_code = "999";
                    tmp_hour.time = "-";
                    tmp_hour.tmp = "-";
                } else {
                    tmp_hour.cond_code = m_json.value("icon").toString();
                    //                    tmp_hour.cond_txt = m_json.value("text").toString();
                    tmp_hour.time = m_json.value("fxTime").toString();
                    QString timeFormat = tmp_hour.time.mid(tmp_hour.time.indexOf("T"), 6);
                    timeFormat = timeFormat.replace("T", "");
                    tmp_hour.time = timeFormat;
                    tmp_hour.tmp = m_json.value("temp").toString() + QString("°");
                }
                hourlist.append(tmp_hour);
            }
        }
    }
    return hourlist;
}

void DataParser::setCollectValue(int i, const QJsonObject &json, ForecastWeather &forecastweather,
                                 const QString &dateTime)
{
    switch (i) {
    case 0:
        forecastweather.first.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.first.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.first.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.first.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.first.wind_dir = json.value("wind_dir").toString();
        forecastweather.first.wind_sc = json.value("wind_sc").toString();
        forecastweather.first.tmp_max = json.value("tmp_max").toString();
        forecastweather.first.tmp_min = json.value("tmp_min").toString();
        forecastweather.first.date = json.value("date").toString();
        forecastweather.first.dateTime = dateTime;
    case 1:
        forecastweather.second.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.second.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.second.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.second.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.second.wind_dir = json.value("wind_dir").toString();
        forecastweather.second.wind_sc = json.value("wind_sc").toString();
        forecastweather.second.tmp_max = json.value("tmp_max").toString();
        forecastweather.second.tmp_min = json.value("tmp_min").toString();
        forecastweather.second.date = json.value("date").toString();
        forecastweather.second.dateTime = dateTime;
    case 2:
        forecastweather.third.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.third.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.third.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.third.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.third.wind_dir = json.value("wind_dir").toString();
        forecastweather.third.wind_sc = json.value("wind_sc").toString();
        forecastweather.third.tmp_max = json.value("tmp_max").toString();
        forecastweather.third.tmp_min = json.value("tmp_min").toString();
        forecastweather.third.date = json.value("date").toString();
        forecastweather.third.dateTime = dateTime;
    case 3:
        forecastweather.fourth.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.fourth.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.fourth.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.fourth.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.fourth.wind_dir = json.value("wind_dir").toString();
        forecastweather.fourth.wind_sc = json.value("wind_sc").toString();
        forecastweather.fourth.tmp_max = json.value("tmp_max").toString();
        forecastweather.fourth.tmp_min = json.value("tmp_min").toString();
        forecastweather.fourth.date = json.value("date").toString();
        forecastweather.fourth.dateTime = dateTime;
    case 4:
        forecastweather.fifth.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.fifth.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.fifth.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.fifth.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.fifth.wind_dir = json.value("wind_dir").toString();
        forecastweather.fifth.wind_sc = json.value("wind_sc").toString();
        forecastweather.fifth.tmp_max = json.value("tmp_max").toString();
        forecastweather.fifth.tmp_min = json.value("tmp_min").toString();
        forecastweather.fifth.date = json.value("date").toString();
        forecastweather.fifth.dateTime = dateTime;
    case 5:
        forecastweather.sixth.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.sixth.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.sixth.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.sixth.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.sixth.wind_dir = json.value("wind_dir").toString();
        forecastweather.sixth.wind_sc = json.value("wind_sc").toString();
        forecastweather.sixth.tmp_max = json.value("tmp_max").toString();
        forecastweather.sixth.tmp_min = json.value("tmp_min").toString();
        forecastweather.sixth.date = json.value("date").toString();
        forecastweather.sixth.dateTime = dateTime;
    case 6:
        forecastweather.seventh.cond_txt_d = json.value("cond_txt_d").toString();
        forecastweather.seventh.cond_txt_n = json.value("cond_txt_n").toString();
        forecastweather.seventh.cond_code_d = json.value("cond_code_d").toString();
        forecastweather.seventh.cond_code_n = json.value("cond_code_n").toString();
        forecastweather.seventh.wind_dir = json.value("wind_dir").toString();
        forecastweather.seventh.wind_sc = json.value("wind_sc").toString();
        forecastweather.seventh.tmp_max = json.value("tmp_max").toString();
        forecastweather.seventh.tmp_min = json.value("tmp_min").toString();
        forecastweather.seventh.date = json.value("date").toString();
        forecastweather.seventh.dateTime = dateTime;
        break;
    default:
        break;
    }
}
