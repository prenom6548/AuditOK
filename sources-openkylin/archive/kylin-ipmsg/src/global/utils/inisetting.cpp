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
#include "inisetting.h"
#include <QDebug>
// #include "view/localinfo/localinfo.h"
#include "globalutils.h"

/*创建或打开配置文件*/
IniSettings::IniSettings(const QString &fileName)
{
    m_confPath = QString(getenv("HOME")) + "/.config/ipmsg/" + fileName;
    m_setting = new QSettings(m_confPath, QSettings::IniFormat);
    m_setting->setObjectName("kylin-ipmsg_inisetting_QSettings_setting");
    m_setting->setIniCodec("utf8");

    initSettings();
}

IniSettings::~IniSettings()
{
    delete m_setting;
}

// 生成静态实例
IniSettings *IniSettings::getInstance()
{
    static IniSettings *instance = nullptr;

    if (instance == nullptr) {
#ifndef TEST_MODE
        try {
            QString fileName = "Config.ini";
            instance = new IniSettings(fileName);
            instance->setObjectName("kylin-ipmsg_inisetting_IniSettings_instance");
        } catch (const std::runtime_error &re) {
            qDebug() << "runtime_error:" << re.what();
        }
#else
        QString fileName = "Config.ini";
        instance = new IniSettings(fileName);
        instance->setObjectName("kylin-ipmsg_inisetting_IniSettings_instance");
#endif
    }

    return instance;
}

QString IniSettings::path() const
{
    return m_setting->fileName();
}

/*写入配置项（group：组，key：键名，value：键值）*/
void IniSettings::setValue(const QString &group, const QString &key, const QVariant &value)
{
    m_setting->beginGroup(group);
    m_setting->setValue(key, value);
    m_setting->endGroup();
}

/*读配置项*/
QVariant IniSettings::value(const QString &group, const QString &key, const QVariant &defaultValue)
{
    m_setting->beginGroup(group);
    QVariant value = m_setting->value(key, defaultValue);
    m_setting->endGroup();
    return value;
}

/*查看配置项中，组中是否是有键值*/
bool IniSettings::containts(const QString &group, const QString &key)
{
    m_setting->beginGroup(group);
    bool bcontain = m_setting->contains(key);
    m_setting->endGroup();
    return bcontain;
}

//移除节点(包括其中所有的键值)
void IniSettings::removeNode(const QString &section)
{
    m_setting->remove(section);
}

/*移除某个组中的某个键值*/
void IniSettings::removeKey(const QString &group, const QString &key)
{
    m_setting->beginGroup(group);
    m_setting->remove(key);
    m_setting->endGroup();
}

//出于效率的原因，setValue 不会立既写入, 要立既写入可以用sync() 函数。
void IniSettings::sync()
{
    m_setting->sync();
}

//将用户名、主机名称、用户昵称、IP、Mac、uuid、端口存到配置文件Config.ini文件中
// void IniSettings::iniWriteValue(const QString &groupLocalHost, const QString &v_LocalName, const QString &v_NickName,
// const QString &v_IPAddress, const QVariant &v_MacAddress, const QString &v_uuid, const QVariant &v_port)
// {
// IniSettings *m_settings;
// m_settings = new IniSettings("Config.ini");
// //以主机名称为组名
// m_settings->setValue(groupLocalHost,"LocalName",v_LocalName);//存入用户名
// m_settings->setValue(groupLocalHost,"NickName",v_NickName);//存入昵称
// m_settings->setValue(groupLocalHost,"IPAddress",v_IPAddress);//存入IP地址
// m_settings->setValue(groupLocalHost,"MacAddress",v_MacAddress);//存入Mac地址
// m_settings->setValue(groupLocalHost,"uuid",v_uuid);//存入uuid
// m_settings->setValue(groupLocalHost,"port",v_port);//存入端口

// qDebug()<<"写入配置文件";
// }

// 初始化配置文件
void IniSettings::initSettings()
{
    QFileInfo fileInfo(m_confPath);
    if (!fileInfo.exists()) {
        this->setValue(groupName, nicknameKey, "");
        this->setValue(groupName, uuidKey, "");
        this->setValue(groupName, portKey, "");
        this->setValue(groupName, filePathKey, "");
    }
    this->sync();
}

// 判断是否存在本机昵称
bool IniSettings::isExistNickname()
{
    return this->containts(groupName, nicknameKey);
}

// 获取本机昵称
QString IniSettings::getLocalNickname()
{
    return this->value(groupName, nicknameKey, "").toString();
}

// 设置本机昵称
void IniSettings::setLocalNickname(QString nickname)
{
    this->setValue(groupName, nicknameKey, nickname);
    this->sync();
}

// 获取本机端口号
int IniSettings::getLocalTcpPort()
{
    return this->value(groupName, portKey).toInt();
}

// 设置本机端口号
void IniSettings::setLocalTcpPort(int port)
{
    this->setValue(groupName, portKey, port);
    this->sync();
}

// 获取本机uuid
QString IniSettings::getLocalUuid()
{
    return this->value(groupName, uuidKey).toString();
}

// 设置本机uuid
void IniSettings::setLocalUuid(QString uuid)
{
    this->setValue(groupName, uuidKey, uuid);
    this->sync();
}

// 获取文件保存路径
QString IniSettings::getFilePath()
{
    return this->value(groupName, filePathKey).toString();
}

// 设置文件保存路径
void IniSettings::setFilePath(QString filePath)
{
    this->setValue(groupName, filePathKey, filePath);
    this->sync();
}
