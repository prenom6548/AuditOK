/*
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
 *
 */

//
// Created by hxf on 22-8-15.
//

#ifndef UKUI_SIDEBAR_COLOR_HELPER_H
#define UKUI_SIDEBAR_COLOR_HELPER_H

#include "value-type.h"

#include <QApplication>
#include <QPalette>
#include <QColor>

namespace Sidebar {

class ColorHelper : public QObject
{
    Q_OBJECT
public:
    static ColorHelper *getInstance();
    ~ColorHelper() override = default;

    // 当前界面是否为深色主题
    Q_INVOKABLE bool isDarkStyle();

    /**
     * 根据提供的颜色枚举值，返回对应的颜色
     * 用于适配主题，统一颜色显示
     * @param colorRole
     * @return
     */
    Q_INVOKABLE QColor pluginColor(UkuiShortcut::Color::ColorRole colorRole);
    Q_INVOKABLE QColor pluginColorWithCustomTransparency(UkuiShortcut::Color::ColorRole colorRole, qreal alphaF);

    Q_INVOKABLE QColor pluginColorHover(UkuiShortcut::Color::ColorRole colorRole);
    Q_INVOKABLE QColor pluginColorPressed(UkuiShortcut::Color::ColorRole colorRole);

    /**
     * 带有主题透明度的插件颜色
     * @param colorRole
     * @return
     */
    Q_INVOKABLE QColor pluginColorWithTransparency(UkuiShortcut::Color::ColorRole colorRole);

    /**
     * 判断一个颜色是否为高亮色
     * 比如：如果一个图标的背景颜色为高亮色，那么这个图标需要进行反色显示，否则会看不清图标内容
     * !!! 只能判断插件显示的颜色是否高亮色
     * @param colorRole 颜色枚举值
     * @return
     */
    Q_INVOKABLE bool isHighLightColor(UkuiShortcut::Color::ColorRole colorRole);

    Q_INVOKABLE QColor separator() const;

    // 侧边栏蒙版颜色
    Q_INVOKABLE QColor mask() const;

private:
    explicit ColorHelper(QObject *parent = nullptr);

private Q_SLOTS:
    void styleChangedSlot(const QString &key = "");

Q_SIGNALS:
    void styleColorChanged();

private:
    bool m_isDarkStyle = false;
    qreal m_transparency = 1.0;
};

} // UkuiShortcut

#endif //UKUI_SIDEBAR_COLOR_HELPER_H
