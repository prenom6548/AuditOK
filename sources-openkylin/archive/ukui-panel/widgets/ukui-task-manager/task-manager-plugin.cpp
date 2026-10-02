/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#include "task-manager-plugin.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <window-manager.h>

#include "ukui-task-manager.h"
#include "actions.h"
#include "task-manager-filter-model.h"
#include "widget-delegate.h"

void TaskManagerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.panel.taskManager"));

    qmlRegisterSingletonType<TaskManager::UkuiTaskManager>(uri, 1, 0, "TaskManager", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return &TaskManager::UkuiTaskManager::self();
    });

    qmlRegisterType<TaskManager::TaskManagerFilterModel>(uri, 1, 0, "TaskManagerFilterModel");
    qmlRegisterType<TaskManager::WidgetDelegate>(uri, 1, 0, "WidgetDelegate");
    qmlRegisterUncreatableType<TaskManager::Action>(uri, 1, 0, "Action", "Only enumeration variables are required");

    qRegisterMetaType<TaskManager::Action::Type>("TaskManager::Action::Type");
    qRegisterMetaType<TaskManager::Actions>("TaskManager::Actions");
}

void TaskManagerPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
    Q_UNUSED(engine);
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.panel.taskManager"));

    //workaround bug#470839 这个构造中有roundTrip，可能会触发屏幕改变信号，导致触发任务栏新增或删除，如果上面的qmlRegisterSingletonType中初始化可能会导致循环调用
    //这里确保UkuiQuick::WindowManager和TaskManager::UkuiTaskManager提前初始化好并且在主线程
    UkuiQuick::WindowManager::self();
    TaskManager::UkuiTaskManager::self();
}
