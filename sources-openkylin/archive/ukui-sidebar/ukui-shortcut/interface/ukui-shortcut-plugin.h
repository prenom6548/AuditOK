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

#ifndef UKUI_SIDEBAR_UKUI_SHORTCUT_PLUGIN_H
#define UKUI_SIDEBAR_UKUI_SHORTCUT_PLUGIN_H

#define UKUI_SHORTCUT_PLUGIN_IFACE_IID         "org.ukui.shortcut.UkuiShortcutPluginIface"
#define UKUI_SHORTCUT_PLUGIN_IFACE_TYPE        "UKUI_SHORT_CUT"
/**
 * changelog:1.0.2->1.1.0: UkuiShortcut中增加setAddedState接口
 * changelog:1.0.1->1.0.2: PluginMetaData中增加widgetId信息
 */
#define UKUI_SHORTCUT_PLUGIN_IFACE_VERSION     "1.1.0"

#include <QtPlugin>
#include <QStringList>

#include "ukui-shortcut.h"

namespace UkuiShortcut {

class UkuiShortcutPlugin : public QObject
{
    Q_OBJECT
public:
    explicit UkuiShortcutPlugin(QObject *parent = nullptr);
    ~UkuiShortcutPlugin() override;

    /**
     * 插件唯一id名称
     * @return 插件id
     */
    virtual QString pluginId() = 0;

    /**
     *  快捷按钮的翻译文件名称
     *  eg: 自动旋转按钮的翻译文件： auto-rotation-shortcut_zh_CN.qm，那么此方法只需要返回 "auto-rotation-shortcut" 即可。
     *  翻译文件会在插件被加载前加载。
     *
     * @return 可以返回多个翻译文件名称
     */
    virtual QStringList translations() = 0;

    /**
     * 创建快捷按钮
     * @return Shortcut*
     */
    virtual Shortcut *createShortcut() = 0;
};

} // Shortcut

Q_DECLARE_INTERFACE(UkuiShortcut::UkuiShortcutPlugin, UKUI_SHORTCUT_PLUGIN_IFACE_IID)

#endif //UKUI_SIDEBAR_UKUI_SHORTCUT_PLUGIN_H
