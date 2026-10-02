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

#ifndef MAIN_FRAME_UTILS_H_
#define MAIN_FRAME_UTILS_H_

#include <QMap>
#include <QVector>
#include <QString>
#include <QJsonObject>

class ToolUtils
{
public:
    ToolUtils();
    ~ToolUtils();

    //读取配置文件
    QMap<QString, QVector<QString>> getConfigFile();
    //写入配置文件
    bool writeConfigFile(bool isOn, QMap<QString, QVector<QString>> configMap);
    //读取开关
    bool getInnerCheckSetting();
    //判断Ip地址是否合法
    static bool isIP(QString ip);
    //判断地址是否合法
    static bool isWeb(QString web);

private:
    QString getConfigPath();
};

#endif
