/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_WIDGETINTERFACE_H
#define UKUI_QUICK_WIDGETINTERFACE_H
#define WidgetInterface_iid "org.ukui.quick.widgetInterface"
#define WIDGET_INTERFACE_VERSION "1.0.0"
#include <QAction>
#include <QList>
#include <QObject>
namespace UkuiQuick
{
/*!
    \class UkuiQuick::WidgetInterface
    \inmodule UkuiQuickFramework
    \inheaderfile widget-interface.h
    \brief 插件接口.

    WidgetInterface 是一个插件接口定义, 插件可以选择实现这个接口以实现某些特性功能, 如增加自定义右键菜单项、
    向 QML 暴露 feature object 等。
    这是一个可选接口, 但可以脱离 qml UI 单独加载, 即可以实现一个不包含 UI 的插件, 仅包含一个动态库的插件.
*/
class WidgetInterface
{
public:
    WidgetInterface()= default;

/*!
    \fn virtual QList<QAction *> WidgetInterface::actions()

    返回插件希望追加到 widget 上下文操作中的动作列表.
 */
    virtual QList<QAction *> actions()
    {
        return {};
    };
/*!
    \fn virtual QList<QObject *> WidgetInterface::featureProviders()

    返回插件侧注册的 feature provider 列表。
    列表中的对象如果实现了 WidgetFeatureProvider 接口, 将被 WidgetBridge 聚合后暴露给 QML.

    当多个 provider 声明同一个 feature key 时, 插件侧 provider 的优先级高于宿主侧 provider;
    同一来源内部, 列表中靠后的 provider 优先级更高.

    \sa UkuiQuick::WidgetFeatureProvider, UkuiQuick::WidgetBridge
 */
    virtual QList<QObject *> featureProviders()
    {
        return {};
    }

};
}
Q_DECLARE_INTERFACE(UkuiQuick::WidgetInterface, WidgetInterface_iid)
Q_DECLARE_METATYPE(UkuiQuick::WidgetInterface *);

#endif //UKUI_QUICK_WIDGETINTERFACE_H
