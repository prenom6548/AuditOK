/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef SCREENSAVER_PLUGIN_H
#define SCREENSAVER_PLUGIN_H

#include <QWidget>

class ScreensaverPlugin
{
public:
    virtual ~ScreensaverPlugin() {}
    //插件实例的名称
    virtual QString name() const = 0;

    //创建UI的实例
    virtual QWidget *createWidget(bool isScreensaver, QWidget *parent) = 0;

    //获得插件的展示名称
    virtual QString displayName() const = 0;
};

//定义了在QT系统中该接口的全局唯一的ID
//实现该SDK的插件也要定义相同的ID
//接口的ID中包含了版本信息,通过该ID我们可以区别不同版本的SDK和插件
// Q_DECLARE_INTERFACE宏将类型和ID关联起来,这样QT就可以验证加载的插件是否可以转换成MyPluginInterface类型
#define interface_iid "org.ukui.screensaver.screensaver-default1.0.0"
Q_DECLARE_INTERFACE(ScreensaverPlugin, interface_iid)

#endif // FILTER_H
