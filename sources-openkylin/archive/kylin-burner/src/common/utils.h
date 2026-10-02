/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef UTILS_H
#define UTILS_H

#include <QString>

class SystemInfo
{
public:
    // 操作系统
    static QString m_os;
    // 架构
    static QString m_arch;
    // 显示协议
    static bool m_isWayland;
};


/**
 * @brief 工具类
 */
class Utils
{
public:
    /**
     * @brief getArch
     * @return 架构，arch命令的结果，如：x86_64
     */
    static QString getArch();

    /**
     * @brief getOs
     * @return 操作系统名字, 如：
     *         Kylin-Desktop V10-SP1
     *         Build 20210407
     */
    static QString getOs();

    /**
     * @brief isWayland
     * @return 判断显示协议是否是wayland
     */
    static bool isWayland();


    /**
     * @brief 初始化系统信息
     */
    static void initSystemInfo();


private:
    static QString m_burnerVersion;

};

class MyMessageBox
{
public:
    static bool showIsoReplaceMessageBox(QString fileName);
};

#endif // UTILS_H
