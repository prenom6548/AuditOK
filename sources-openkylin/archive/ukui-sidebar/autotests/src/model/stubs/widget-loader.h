/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef AUTOTEST_WIDGET_LOADER_H
#define AUTOTEST_WIDGET_LOADER_H

#include <QObject>
#include <QStringList>

#include "widget-metadata.h"

namespace UkuiQuick {

class Widget;

struct WidgetLoaderStubState
{
    bool containerLoadSucceeds = true;
    bool containerItemLoadSucceeds = true;
    bool widgetLoadSucceeds = true;
    int loadCalls = 0;
    int addPathCalls = 0;
    int setFilterCalls = 0;
    QStringList loadedIds;
    QStringList searchPaths;
    WidgetMetadata::Hosts lastHosts = WidgetMetadata::Undefined;
};

WidgetLoaderStubState &widgetLoaderStubState();
void resetWidgetLoaderStubState();

class WidgetLoader : public QObject
{
    Q_OBJECT
public:
    explicit WidgetLoader(QObject *parent = nullptr);

    Widget *loadWidget(const QString &id);
    void addWidgetSearchPath(const QString &path);
    void setShowInFilter(WidgetMetadata::Hosts hosts);
};

} // namespace UkuiQuick

#endif // AUTOTEST_WIDGET_LOADER_H
