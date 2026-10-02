/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "system-tray-plugin.h"
#include "items-filter-model.h"
#include "item-group-model.h"
#include "security-function-control.h"
#include "control-panel-model.h"

#include <QQmlEngine>
#include <QQmlContext>

void SystemTrayPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.systemTray"));

    qmlRegisterUncreatableType<ItemGroupModel>(uri,1,0,"ItemGroupModel", "");
    qmlRegisterType<ItemGroupModel>(uri, 1, 0, "GroupModel");

    qmlRegisterSingletonType<ItemsFilterModel>(uri,1,0,"ItemModel", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return ItemsFilterModel::instance();
    });
    qmlRegisterSingletonType<SecurityFunctionControl>(uri,1,0,"FunctionControl", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        return SecurityFunctionControl::instance();
    });
    qmlRegisterSingletonType<ControlPanelModel>(uri,1,0,"ControlPanelModel", [] (QQmlEngine *, QJSEngine *) -> QObject* {
    return ControlPanelModel::self();
});
}
