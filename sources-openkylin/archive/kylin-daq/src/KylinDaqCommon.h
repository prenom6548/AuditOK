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
#ifndef KYLINDAQCOMMON_H
#define KYLINDAQCOMMON_H
#include <QObject>
#include <QJsonObject>
#include <QVariant>
#include <QVector>
#include <QJsonArray>
#include <QtCore/QtCore>


template <typename A> A compareValue(const A inputValue,const A maxValue, const A minValue, const A defaultValue);
template <typename A> A compareValue(const A inputValue,const A maxValue, const A minValue, const A defaultValue)
{
    if(inputValue > maxValue )
    {
        return maxValue;
    }
    else if( inputValue < minValue)
    {
        return minValue;
    }
    else
    {
        return inputValue;
    }

}

class KylinDaqCommon
{
public:
    static constexpr const char* gs_confFileName= "kylinDaqConf.json";
#ifdef UNITTEST
    static constexpr const char* gs_cacheFolderPath= "../../cache/";
#else
    static constexpr const char* gs_cacheFolderPath= "/var/cache/kylin-daq/";
#endif
    static constexpr const char* gs_defaultConfFilePath = ":conf/conf/defaultConf.json";

    static constexpr const char* gs_dataBaseName = "data.db";
    static constexpr const char* gs_connectionName = "cacheManager";

    static constexpr const char* gs_maxStr = "Max";
    static constexpr const char* gs_defaultStr = "Default";
    static constexpr const char* gs_minStr = "Min";
    static constexpr const qlonglong gs_secToMsec = 1000;

    static constexpr const char *gs_informationKey = "information";
    static constexpr const char *gs_sourceKey = "source";
    static constexpr const char *gs_packageNameKey = "packageName";
    static constexpr const char *gs_messageTypeKey = "messageType";
    static constexpr const char *gs_tidKey = "tid";
    static constexpr const char *gs_parentTidKey = "parentTid";
    static constexpr const char *gs_sourceVersionKey = "version";
    static constexpr const char *gs_sourceVersionValue = "1.1.0";
    static constexpr const char *gs_stateOperatorKey = "dataOperator";
    static constexpr const char *gs_stateOperatorKeyAddValue= "add";
    static constexpr const char *gs_stateOperatorKeyUpdateValue = "update";

    /**
     * @brief json对象中information对象的createTimeStamp键对应值格式
     * 
     */
    static constexpr const char *gs_timeFormat = "yyyy-MM-dd HH:mm:ss.zzz";
    static constexpr const char *gs_timeFormat2 = "yyyy-MM-dd HH:mm:ss";
    /**
     * @brief json对象中information对象的createTimeStamp键
     * 
     */
    static constexpr const char *gs_createTimeStampKey = "createTimeStamp";
    /**
     * @brief json对象中information对象的createTimeStamp键对应的时区
     * 
     */
    static constexpr int32_t gs_timezone = 8*3600;// 时区为东八区
    enum UploadState{
        UploadDisable = 0,
        UploadEnable = 1
    };
    enum FunctionState{
        FunctionSuccess = 0,
        FunctionFail = -1
    };
    /**
     * UploadStateMessageCreated: 状态被创建
     * UploadStateMessageUpdated: 状态被更新
     * UploadStateMessageUploaed: 状态信息被上传
    */
    enum UploadStateEnum{
        UploadStateMessageNull = 0,
        UploadStateMessageCreated = 1,
        UploadStateMessageUpdated = 2,
        UploadStateMessageUploaded = 3
    };
    typedef struct {
        QString packageInfo;
        QString uploadedMessage;
        QString tids;
        QString createTimeStamp;
        KylinDaqCommon::UploadStateEnum uploadState;
    }UploadStateMessageInfo_t;
    enum UploadTypeEnum{
        OnlyState = 0,
        StateAndMessage = 1,
        StateAndTwoMessage = 2
    };


    qlonglong gs_maxLines = 0;

    static qlonglong secToMsec(qlonglong sec);
    void setValidMaxLines(qlonglong maxLines);
    static FunctionState setValidConfInt(const QJsonObject &conf, const QJsonObject &defaultConf, const QString &moduleName, const QString &key, qlonglong &value);
    static FunctionState setValidConfIntCmpMax(const QJsonObject &conf, const QJsonObject &defaultConf, const QString &moduleName, const QString &key, const qlonglong &waitCmpValue,qlonglong &value);
    static FunctionState setValidConfStrArray(const QJsonObject &conf, const QJsonObject &defaultConf, const QString &moduleName, const QString &key, QList<QString> &value);
private:
    static FunctionState getValidConfInt(const QJsonObject &conf ,const QJsonObject &defaultConf, const QString &key, qlonglong &value);
    static FunctionState getValidConfStrArray(const QJsonObject &conf ,const QJsonObject &defaultConf, const QString &key, QList<QString> &value);
    static FunctionState checkJsonStrArray(const QJsonArray &jsonStrArray);
    static FunctionState addValidConfStrArray(const QJsonArray &jsonStrArray, QList<QString> &strArray);

};
#endif
