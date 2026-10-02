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

#include <QGuiApplication>
#include <QApplication>
#include <QQuickView>
#include <QList>

#include "widget.h"
#include "widget-loader.h"
#include "widget-item.h"
#include "config-loader.h"
#include "widget-container.h"
#include "island-view.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto view = new UkuiQuick::IslandView("ukui-quick-frameworktest", "ukui-quick-frameworktest1");
    const QString defaultViewId = QStringLiteral("org.ukui.quick.frameworktest");
    view->resize(600, 100);
    view->setColor(QColor(Qt::transparent));
    UkuiQuick::WidgetContainer::widgetLoader().addWidgetSearchPath(QStringLiteral(":/"));
    view->loadMainView(defaultViewId);

    UkuiQuick::WidgetLoader loader;
    view->mainView()->addWidget("org.ukui.menu.starter", 0);
    view->mainView()->addWidget("org.ukui.panel.taskView", 1);
    view->mainView()->addWidget("org.ukui.testWidget", 2);
    view->show();

    return QGuiApplication::exec();
}
