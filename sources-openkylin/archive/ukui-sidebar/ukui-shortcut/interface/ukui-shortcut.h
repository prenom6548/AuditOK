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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *          hxf <hewenfei@kylinos.cn>
 *
 */
#ifndef UKUISHORTCUTPLUGIN_H
#define UKUISHORTCUTPLUGIN_H

#define UKUI_SHORTCUT_PLUGIN_IFACE_IID         "org.ukui.shortcut.UkuiShortcutPluginIface"

#include <QObject>
#include <QMap>

#include "ukui-shortcut_global.h"
#include "plugin-common-data.h"
#include "status-info.h"

namespace UkuiShortcut {

class UKUISHORTCUT_EXPORT Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = nullptr) : QObject(parent) {}
    //插件标识
    virtual QString pluginId() = 0;
    //插件类型（样式）, 需要指定不同模式下的按钮
    virtual QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() = 0;
    //实现响应前端用户操作
    virtual void active(PluginMetaType::Action action) = 0;
    //进度条样式需要重写该函数
    virtual void setValue(int value) {Q_UNUSED(value)}
    //返回插件当前的状态信息
    virtual const StatusInfo currentStatus() = 0;
    //插件是否显示
    virtual bool isEnable() { return true; }
    //此插件是否被添加到快捷面板中
    virtual void setAddedState(bool added) {Q_UNUSED(added)}

Q_SIGNALS:
    void statusChanged(const StatusInfo &info);
    void enableStatusChanged(bool isEnable);
};

}
Q_DECLARE_INTERFACE(UkuiShortcut::Shortcut, UKUI_SHORTCUT_PLUGIN_IFACE_IID)
#endif // UKUISHORTCUTPLUGIN_H
