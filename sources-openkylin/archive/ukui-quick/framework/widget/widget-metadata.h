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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_WIDGET_METADATA_H
#define UKUI_QUICK_WIDGET_METADATA_H

#include <QDir>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QSharedDataPointer>

#include "types.h"

namespace UkuiQuick {
class WidgetMetadataPrivate;
class WidgetMetadata
{
public:
    enum Host
    {
        Undefined = 0x00000000,
        /**
         *@brief 用于panel类型container，如ukui-panel
         */
        Panel = 0x00000001,
        /**
         *@brief 用于sidebar类型container,如ukui-sidebar中的通知中心等从屏幕侧边划入的面板
         */
        SideBar = 0x00000002,
        /**
         *@brief 用于desktop类型container
         */
        Desktop = 0x00000004,
        /**
         *@brief 可以加载到taskManager的图标上
         */
        TaskManager = 0x00000008,
        /**
         *@brief 用于加载到快捷操作面板上，作为对应快捷操作的二级菜单
         */
        Shortcut = 0x00000010,
        /**
         * @brief 特殊类型，桌面壁纸插件
         */
        Wallpaper = 0x00000020,
        /**
         * @brief  特殊类型，桌面布局插件
         */
        DesktopView = 0x00000040,
        /**
         * @brief  特殊类型，全局搜索插件
         */
        Search = 0x00000080,
        /**
         *@brief 用于所有类型container
         */
        All = Panel | SideBar | Desktop | TaskManager | Shortcut,
    };

    Q_DECLARE_FLAGS(Hosts, Host);

    WidgetMetadata();
    explicit WidgetMetadata(const QString &root);
    WidgetMetadata(const WidgetMetadata &other);
    WidgetMetadata &operator=(const WidgetMetadata &other);
    bool operator==(const WidgetMetadata &other) const;
    ~WidgetMetadata();

    bool isValid() const;
    const QDir &root() const;

    QString id() const;
    QString icon() const;
    QString name() const;
    QString tooltip() const;
    QString version() const;
    QString website() const;
    QString bugReport() const;
    QString description() const;
    QVariantList authors() const;
    QVariantMap contents() const;
    Hosts showIn() const;
    Types::WidgetType widgetType() const;
    QStringList applicationId() const;
    QString thumbnail() const;

private:
    QSharedDataPointer<WidgetMetadataPrivate> d;
};

} // UkuiQuick
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::WidgetMetadata::Hosts)
Q_DECLARE_TYPEINFO(UkuiQuick::WidgetMetadata, Q_MOVABLE_TYPE);
#endif //UKUI_QUICK_WIDGET_METADATA_H
