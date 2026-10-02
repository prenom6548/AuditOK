/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef GLOBAL_UTILS_H
#define GLOBAL_UTILS_H

#include <QString>
#include "definetypes.h"

/**
 * @brief isGreeterMode 是否为欢迎界面模式，区分桌面锁屏
 * @return true 是，false 否
 */
bool isGreeterMode();

/**
 * @brief isCurUserSelf 是否为当前用户自己
 * @param QString 用户名
 * @return true 是，false 否
 */
bool isCurUserSelf(QString strUserName);

/**
 * @brief getHostCloudPlatform 获取当前云平台环境
 * @return 云环境标识
 */
QString getHostCloudPlatform();

bool isCommunity();

/**
 * @brief isOpenkylin 是否为ok环境
 * @return true 是，否则不是
 */
bool isOpenkylin();

QString getDefaultFace();

void setFcitxExport();

void startFcitx();

void runCommand(const QString &command);

// 检查某进程名的进程是否在运行
bool isProcessRunning(const QString &procName);

#endif // GLOBAL_UTILS_H
