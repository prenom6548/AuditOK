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

#ifndef INISETTING_H
#define INISETTING_H

#include <QObject>
#include <QVariant>
#include <QSettings>
#include <QFileInfo>

class IniSettings : public QObject
{
    Q_OBJECT

public:
    IniSettings(const QString &fileName); /*创建或打开配置文件*/
    ~IniSettings();

    // 生成静态实例
    static IniSettings *getInstance();

    QString path() const;

    /*写入配置项（group：组，key：键名，value：键值）*/
    void setValue(const QString &group, const QString &key, const QVariant &value);

    /*读配置项*/
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());

    /*查看配置项中，组中是否是有键值*/
    bool containts(const QString &group, const QString &key);

    //移除节点(包括其中所有的键值)
    void removeNode(const QString &section);

    /*移除某个组中的某个键值*/
    void removeKey(const QString &group, const QString &key);

    //出于效率的原因，setValue 不会立既写入, 要立既写入可以用sync() 函数。
    void sync();

    //将用户名、主机名称、用户昵称、IP、Mac、uuid、端口存到配置文件config.ini文件中
    // void iniWriteValue(const QString &groupLocalHost, const QString &v_LocalName, const QString &v_NickName, const
    // QString &v_IPAddress, const QVariant &v_MacAddress, const QString &v_uuid, const QVariant &v_port);

    // 初始化配置文件
    void initSettings();

    // 判断是否存在本机昵称
    bool isExistNickname();

    // 获取本机昵称
    QString getLocalNickname();

    // 设置本机昵称
    void setLocalNickname(QString nickname);

    // 获取本机端口号
    int getLocalTcpPort();

    // 设置本机端口号
    void setLocalTcpPort(int port);

    // 获取本机uuid
    QString getLocalUuid();

    // 设置本机uuid
    void setLocalUuid(QString uuid);

    // 获取文件保存路径
    QString getFilePath();

    // 设置文件保存路径
    void setFilePath(QString filePath);

private:
    QSettings *m_setting;

    QString m_confPath;

    const QString groupName = QString("General");
    const QString nicknameKey = QString("Nickname");
    const QString uuidKey = QString("Uuid");
    const QString portKey = QString("Port");
    const QString filePathKey = QString("FilePath");
};

#endif // INISETTING_H
