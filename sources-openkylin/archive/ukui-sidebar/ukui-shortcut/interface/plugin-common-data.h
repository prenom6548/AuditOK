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
 *
 */
#ifndef STATUS_H
#define STATUS_H

#include <QObject>

namespace UkuiShortcut {

class PluginMetaType
{
    Q_GADGET
public:
    enum PluginType
    {
        Icon = 0,    //图标样式
        ProgressBar, //进度条样式
        MenuButton   //可展开菜单按钮样式
    };
    Q_DECLARE_FLAGS(PluginTypes, PluginType)
    Q_ENUM(PluginType)

    enum SystemMode {
        PC = 0,           //PC模式
        Tablet,           //平板模式
        VR = Tablet,      //VR
        AR = Tablet,      //AR
        Other = Tablet    //其他
    };
    Q_DECLARE_FLAGS(SystemModes, SystemMode)
    Q_ENUM(SystemMode)

    enum Action {
        Click = 0, //点击
        LongClick, //长按
        MenuRequest, //请求菜单
    };
    Q_DECLARE_FLAGS(Actions, Action)
    Q_ENUM(Action)

    /**
     * 当控制中心执行对应的Action前，需要执行的动作
     */
    enum PredefinedAction {
        NoAction = 0, //无动作
        Hide, //隐藏
    };
    Q_DECLARE_FLAGS(PredefinedActions, PredefinedAction)
    Q_ENUM(PredefinedAction)
};

class PluginMetaData
{
    Q_GADGET
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(int index READ index WRITE setIndex)
    Q_PROPERTY(PluginMetaType::PluginType pluginType READ pluginType WRITE setPluginType)
    Q_PROPERTY(PluginMetaType::PredefinedAction preAction READ preAction WRITE setPreAction)
public:
    PluginMetaData() = default;
    explicit PluginMetaData(bool visible) : m_visible(visible) {}
    PluginMetaData(bool visible, int index) : m_visible(visible), m_index(index) {}
    PluginMetaData(bool visible, int index, PluginMetaType::PluginType pluginType) : m_visible(visible), m_index(index), m_pluginType(pluginType) {}

    bool isVisible() const {
        return m_visible;
    }

    void setVisible(bool visible) {
        m_visible = visible;
    }

    int index() const {
        return m_index;
    }

    void setIndex(int index) {
        m_index = index;
    }

    PluginMetaType::PluginType pluginType() const {
        return m_pluginType;
    }

    void setPluginType(PluginMetaType::PluginType pluginType) {
        m_pluginType = pluginType;
    }

    PluginMetaType::PredefinedAction preAction() const
    {
        return m_preAction;
    }

    void setPreAction(PluginMetaType::PredefinedAction preAction)
    {
        m_preAction = preAction;
    }

private:
    bool m_visible = false;
    int  m_index   = -1;
    PluginMetaType::PluginType m_pluginType = PluginMetaType::PluginType::Icon;
    PluginMetaType::PredefinedAction m_preAction = PluginMetaType::NoAction;
};

}
#endif // STATUS_H
