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
#ifndef STATUSINFO_H
#define STATUSINFO_H
#include <memory>

#include <QList>
#include <QString>
#include <QColor>
#include <QMetaType>

#include "ukui-shortcut_global.h"
#include "value-type.h"

namespace UkuiShortcut {

class StatusInfoPrivate;

/**
 * 因为无法控制插件如何存放数据
 * 所以设计固定的函数 要求插件存放固格式的数据
 */
class UKUISHORTCUT_EXPORT StatusInfo
{
public:
    enum InfoType {
        Normal = 0, //普通信息
        MenuInfo //菜单信息
    };

    struct MenuItem {
        QString icon; //图标path (如qrc等)
        QString text;
        QString statusIcon;
    };

    explicit StatusInfo(InfoType type = Normal);
    ~StatusInfo();
    StatusInfo(const StatusInfo& si);
    StatusInfo& operator=(const StatusInfo& rhs);

    InfoType getInfoType();
    void setInfoType(UkuiShortcut::StatusInfo::InfoType infoType);

    bool isDisabled() const;
    void setDisable(bool disable);

    bool isIconClickable() const;
    void setIconClickable(bool iconClickable);

    void setName(const QString &name);
    QString getName() const;

    void setStatusName(const QString &statusName);
    QString getStatusName() const;

    void setWidgetId(const QString &widgetId);
    QString getWidgetId() const;

    void setIcon(const QString &icon);
    QString getIcon() const;

    void setToolTip(const QString &tooltip);
    QString getToolTip() const;

    /**
     * 为了统一前端显示，
     * 不再支持直接设置color，请使用枚举接口
     */
    void setColor(const QColor &color);
    void setColor(Color::ColorRole colorRole);
    Color::ColorRole getColor() const;

    void setValue(int value);
    int getValue() const;

    void setMaxValue(int maxValue);
    int getMaxValue() const;

    void setMenu(const QList<UkuiShortcut::StatusInfo::MenuItem> &menu);
    QList<UkuiShortcut::StatusInfo::MenuItem> getMenu() const;

    QString toJsonString() const;

private:
    const std::unique_ptr<StatusInfoPrivate> d;
};
}
//注册MenuItem
Q_DECLARE_METATYPE(UkuiShortcut::StatusInfo::MenuItem)

#endif // STATUSINFO_H
