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
#include "KylinDaqCommon.h" 

qlonglong KylinDaqCommon::secToMsec(qlonglong sec)
{
    return sec*gs_secToMsec;
}

KylinDaqCommon::FunctionState KylinDaqCommon::getValidConfInt(const QJsonObject &conf ,const QJsonObject &defaultConf, const QString &key, qlonglong &value)
{
    if(defaultConf.contains(key)
       && defaultConf[key].isObject()
       && defaultConf[key].toObject().contains(gs_defaultStr))
    {
        QJsonObject defaultConfKeyObj = defaultConf[key].toObject();
        if(conf.contains(key)
           && defaultConfKeyObj.contains(gs_maxStr)
           && defaultConfKeyObj.contains(gs_minStr))
        {
            value = compareValue(conf[key].toVariant().toLongLong()
                         ,defaultConfKeyObj[gs_maxStr].toVariant().toLongLong()
                         ,defaultConfKeyObj[gs_minStr].toVariant().toLongLong()
                         ,defaultConfKeyObj[gs_defaultStr].toVariant().toLongLong());
        }
        else {
           value = defaultConfKeyObj[gs_defaultStr].toVariant().toLongLong();
        }
    }
    else {
       return KylinDaqCommon::FunctionFail;
    }
    return KylinDaqCommon::FunctionSuccess;
}

KylinDaqCommon::FunctionState KylinDaqCommon::getValidConfStrArray(const QJsonObject &conf ,const QJsonObject &defaultConf, const QString &key, QList<QString> &value)
{
    if(defaultConf.contains(key)
       && defaultConf[key].isObject()
       && defaultConf[key].toObject().contains(gs_defaultStr)
       && defaultConf[key].toObject()[gs_defaultStr].isArray()
       && checkJsonStrArray(defaultConf[key].toObject()[gs_defaultStr].toArray()) == KylinDaqCommon::FunctionSuccess)
    {
        QJsonArray jsonStrArray;

        if(conf.contains(key)
           && conf[key].isArray()
           && checkJsonStrArray(conf[key].toArray()) == KylinDaqCommon::FunctionSuccess)
        {
            jsonStrArray = conf[key].toArray();
        }
        else
        {
            jsonStrArray = defaultConf[key].toObject()[gs_defaultStr].toArray();
        }
        addValidConfStrArray(jsonStrArray, value);
    }
    else {
       return KylinDaqCommon::FunctionFail;
    }
    return KylinDaqCommon::FunctionSuccess;
}

KylinDaqCommon::FunctionState KylinDaqCommon::checkJsonStrArray(const QJsonArray &jsonStrArray)
{
    for(auto iter=jsonStrArray.begin();iter != jsonStrArray.end();iter++)
    {
        if (!(*iter).isString())
        {
            return KylinDaqCommon::FunctionFail;
        }
    }
    return KylinDaqCommon::FunctionSuccess;
}

KylinDaqCommon::FunctionState KylinDaqCommon::addValidConfStrArray(const QJsonArray &jsonStrArray, QList<QString> &strArray)
{
    strArray.clear();
    for(auto iter=jsonStrArray.begin();iter != jsonStrArray.end();iter++)
    {
        strArray.append((*iter).toString());
    }
    return KylinDaqCommon::FunctionSuccess;
}

KylinDaqCommon::FunctionState KylinDaqCommon::setValidConfInt(const QJsonObject &conf, const QJsonObject &defaultConf, const QString &moduleName, const QString &key, qlonglong &value)
{
    if(defaultConf.contains(moduleName)
        && defaultConf[moduleName].isObject()
    )
    {
        QJsonObject moduleConf;
        QJsonObject moduleDefaultConf = defaultConf[moduleName].toObject();
        if(conf.contains(moduleName)
            && conf[moduleName].isObject())
        {
            moduleConf = conf[moduleName].toObject();
        }
        if(getValidConfInt(moduleConf,moduleDefaultConf,key,value) == KylinDaqCommon::FunctionSuccess)
        {
            qInfo()<<"module="<<moduleName<<",property="<<key<<":"<<value;
            return KylinDaqCommon::FunctionSuccess;
        }
        else
        {
            qCritical()<<"module="<<moduleName<<",property="<<key<<":read conf fail";
            return KylinDaqCommon::FunctionFail;
        }
    }
    else
    {
        qCritical()<<"module="<<moduleName<<":update property fail";
        return KylinDaqCommon::FunctionFail;
    }
}

KylinDaqCommon::FunctionState KylinDaqCommon::setValidConfIntCmpMax(const QJsonObject &conf, const QJsonObject &defaultConf, const QString &moduleName, const QString &key, const qlonglong &waitCmpValue,qlonglong &value)
{
    if(defaultConf.contains(moduleName)
        && defaultConf[moduleName].isObject()
    )
    {
        QJsonObject moduleConf;
        QJsonObject moduleDefaultConf = defaultConf[moduleName].toObject();
        if(conf.contains(moduleName)
            && conf[moduleName].isObject())
        {
            moduleConf = conf[moduleName].toObject();
        }
        if(getValidConfInt(moduleConf,moduleDefaultConf,key,value) == KylinDaqCommon::FunctionSuccess)
        {
            if(waitCmpValue > value)
            {
                value = waitCmpValue;
            }
            qInfo()<<"module="<<moduleName<<",property="<<key<<":"<<value;
            return KylinDaqCommon::FunctionSuccess;
        }
        else
        {
            qCritical()<<"module="<<moduleName<<",property="<<key<<":read conf fail";
            return KylinDaqCommon::FunctionFail;
        }
    }
    else
    {
        qCritical()<<"module="<<moduleName<<":update property fail";
        return KylinDaqCommon::FunctionFail;
    }
}

KylinDaqCommon::FunctionState KylinDaqCommon::setValidConfStrArray(const QJsonObject &conf, const QJsonObject &defaultConf, const QString &moduleName, const QString &key, QList<QString> &value)
{
    if(defaultConf.contains(moduleName)
        && defaultConf[moduleName].isObject()
    )
    {
        QJsonObject moduleConf;
        QJsonObject moduleDefaultConf = defaultConf[moduleName].toObject();
        if(conf.contains(moduleName)
            && conf[moduleName].isObject())
        {
            moduleConf = conf[moduleName].toObject();
        }
        if(getValidConfStrArray(moduleConf,moduleDefaultConf,key,value) == KylinDaqCommon::FunctionSuccess)
        {
            qInfo()<<"module="<<moduleName<<",property="<<key<<":"<<value;
            return KylinDaqCommon::FunctionSuccess;
        }
        else
        {
            qCritical()<<"module="<<moduleName<<",property="<<key<<":read conf fail";
            return KylinDaqCommon::FunctionFail;
        }
    }
    else
    {
        qCritical()<<"module="<<moduleName<<":update property fail";
        return KylinDaqCommon::FunctionFail;
    }
}



void KylinDaqCommon::setValidMaxLines(qlonglong maxLines)
{
    gs_maxLines = maxLines;
}
