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
 *
 */

#ifndef BATTERYICONHELPER_H
#define BATTERYICONHELPER_H

#include <QIcon>
#include <QString>

// 根据电量与低电量阈值，对 battery-level-* 主题图标应用红/橙/绿规则
QIcon buildBatteryLevelIcon(const QString &iconName,
                            double batteryPercentage,
                            const QString &context = QString());

#endif // BATTERYICONHELPER_H
