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
#ifndef UTILS_H
#define UTILS_H

#include "global_utils.h"

/**
 * @brief checkIslivecd 检查是否为试用模式，试用模式直接退出进程
 */
void checkIslivecd();

/**
 * @brief KillFocusOfKydroid 通知Kydroid组件取消焦点
 */
void KillFocusOfKydroid();

/**
 * @brief 判断大写键状态
 * @return true: 大写锁定
 */
bool checkCapsLockState();

bool ispicture(QString filepath);

double getDefaultFontSize();

double getUserFontSize(QString userName);

QString getUserThemeColor(QString userName);

#endif // UTILS_H
