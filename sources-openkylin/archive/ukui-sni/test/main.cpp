/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>

#include "status-notifier-host.h"
#include "status-notifier-model.h"
#include "item-icon-provider.h"
#include "icon-item.h"
#include "tray-app.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.addImageProvider("itemIcon", new ItemIconProvider());
    qmlRegisterType<IconItem>("IconItemModule",1,0,"IconItem");

    TrayApp trayApp;
    trayApp.init(app.applicationPid());

    UkuiSni::StatusNotifierHost::self()->registerHost();
    QQmlContext *qmlCtx = engine.rootContext();
    statusNotifierModel itemModel;
    qmlCtx->setContextProperty("itemModel", &itemModel);

    const QUrl url(QStringLiteral("qrc:/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    qWarning() << QDir::currentPath();

    return app.exec();
}
