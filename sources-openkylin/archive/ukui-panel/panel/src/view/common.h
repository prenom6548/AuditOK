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
 */

#ifndef UKUI_PANEL_COMMON_H
#define UKUI_PANEL_COMMON_H

#include <QString>

namespace UkuiPanel {
/**
 * 屏幕不可用区域
 * 一般存在于屏幕四周
 */
class UnavailableArea {
public:
    int leftWidth = 0;
    int leftStart = 0;
    int leftEnd = 0;
    int rightWidth = 0;
    int rightStart = 0;
    int rightEnd = 0;
    int topWidth = 0;
    int topStart = 0;
    int topEnd = 0;
    int bottomWidth = 0;
    int bottomStart = 0;
    int bottomEnd = 0;
};

class NumberSetting {
public:
    NumberSetting(QString k, int d, int l, int r) : key(std::move(k)), defaultValue(d), min(l), max(r)
    {}

    QString key;
    int defaultValue{0};
    int min{0};
    int max{0};
};

class WindowSuppressionState
{
public:
    bool screenLockHidden() const
    {
        return m_screenLockHidden;
    }

    void setScreenLockHidden(bool hidden)
    {
        m_screenLockHidden = hidden;
    }

    bool shellHidden() const
    {
        return m_shellHidden;
    }

    void setShellHidden(bool hidden)
    {
        m_shellHidden = hidden;
    }

    bool controlHidden() const
    {
        return m_controlHidden;
    }

    void setControlHidden(bool hidden)
    {
        m_controlHidden = hidden;
    }

    bool suppressed() const
    {
        return m_screenLockHidden || m_shellHidden || m_controlHidden;
    }

private:
    bool m_screenLockHidden = false;
    bool m_shellHidden = false;
    bool m_controlHidden = false;
};

/*
 * 配置文件版本号
 * 由于历史原因，panel的版本号曾经使用主版本CONFIG_VERSION号记录，所以将主版本号从 空 或 1.0 改为 1.1的操作在panel中进行
 * 在后续版本中，主版本号的兼容操作需要放到shell中进行，且
 * panel的版本号使用PANEL_CONFIG_VERSION记录
 * panel-next的版本号使用PANEL_NEXT_CONFIG_VERSION记录
 * panel-top-bar的版本号使用PANEL_TOP_BAR_CONFIG_VERSION记录
 */
#define CONFIG_VERSION "1.1"
#define PANEL_CONFIG_VERSION "1.0"
#define PANEL_NEXT_CONFIG_VERSION "1.0"
#define PANEL_TOP_BAR_CONFIG_VERSION "1.0"
#define QWINDOWSIZE_MAX ((1<<24)-1)
}
#endif //UKUI_PANEL_COMMON_H
