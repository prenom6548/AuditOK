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

#ifndef UKUI_QUICK_WIDGET_LOADER_H
#define UKUI_QUICK_WIDGET_LOADER_H

#include <QObject>
#include <QString>

#include "widget-metadata.h"

namespace UkuiQuick {

class Widget;
class WidgetLoaderPrivate;

class WidgetLoader : public QObject
{
    Q_OBJECT
public:
    explicit WidgetLoader(QObject *parent = nullptr);
    ~WidgetLoader() override;

    Widget *loadWidget(const QString &id);
    WidgetMetadata loadMetadata(const QString &id);

    void addWidgetSearchPath(const QString &path);
    void setShowInFilter(WidgetMetadata::Hosts hosts);

    static WidgetLoader &globalLoader();

    /**
     * @return 获取支持当前host的并且ApplicationId包含 @param applicationId 的所有插件id
     */
    QStringList widgets(const QString &applicationId = "");

    /**
     *
     * @return 获取所有已安装插件的元数据列表
     */
    QList<WidgetMetadata> widgetsMetadata();

private:
    WidgetLoaderPrivate *d {nullptr};
};

} // UkuiQuick

#endif //UKUI_QUICK_WIDGET_LOADER_H
