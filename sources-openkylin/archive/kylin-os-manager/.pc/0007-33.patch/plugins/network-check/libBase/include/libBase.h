/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef LIBBASE_H
#define LIBBASE_H

#include <iostream>
#include <memory>
#include <string.h>
#include <QtPlugin>
#include "return_code.h"
#include "../Observer_Notifier/notifier.h"
struct InnerNetCheck
{
    bool isInnerCheck = false;
    uchar ipNum = 0;
    uchar webNum = 0;
    QList<QString> ip = {"", "", "", "", ""};
    QList<QString> web = {"", "", "", "", ""};
    void ipClear()
    {
        ipNum = 0;
        ip = {"", "", "", "", ""};
    }
    void webClear()
    {
        webNum = 0;
        web = {"", "", "", "", ""};
    }
    void clear()
    {
        isInnerCheck = false;
        ipNum = 0;
        webNum = 0;
        ip = {"", "", "", "", ""};
        web = {"", "", "", "", ""};
    }
    void operator=(InnerNetCheck &other)
    {
        isInnerCheck = other.isInnerCheck;
        ipNum = other.ipNum;
        webNum = other.webNum;
        for (int i = 0; i < 5; ++i) {
            ip[i] = other.ip[i];
            web[i] = other.web[i];
        }
    }
};
class KeyForCheck
{

public:
    KeyForCheck();

    std::string getKey();

private:
    std::string key;
};

#define LibBaseInterfaceIID "org.kyNW.PluginInterface"
class LibBase : public Notifier
{
public:
    LibBase();
    virtual ~LibBase();
    void setPluginType(CheckType type)
    {
        m_pluginType = type;
    }
    CheckType getPluginType() const
    {
        return m_pluginType;
    }
    void setCheckKey(std::string keyStr)
    {
        m_checkKey = keyStr;
    }
    void setProjectName(std::string projectNameStr)
    {
        m_projectName = projectNameStr;
    }
    KeyForCheck *getKeyInfoPtr()
    {
        return m_keyInfo;
    }
    //返回so用于验证的key
    std::string getKYdynamicLibKey() const
    {
        return m_checkKey;
    };
    //返回检测项名称
    std::string getCheckProjectName() const
    {
        return m_projectName;
    };
    //返回检测结果是否OK
    virtual CHECKRESULT getCheckResult() = 0;
    //开始检测
    virtual void startChecking(InnerNetCheck &checkSettings) = 0;
    virtual void setInit() = 0;

private:
    std::string m_checkKey = "";

    KeyForCheck *m_keyInfo = nullptr;
    CheckType m_pluginType = CheckType::UNKNOWN;
    std::string m_projectName = "";
    std::string m_checkDetail = "";
    std::string m_failedInfo = "";
};
Q_DECLARE_INTERFACE(LibBase, LibBaseInterfaceIID)
#endif // LIBBASE_H
