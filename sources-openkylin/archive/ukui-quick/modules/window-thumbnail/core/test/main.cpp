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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: sundagao <sundagao@kylinos.cn>
 */

//
// Created by sundagao on 2026/3/5.
//
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QDebug>
#include <QQmlContext>
#include "../window-thumbnail-item.h"
#include "../workspace-thumbnail-item.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("window-thumbnail-item-test");
    QGuiApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("WindowThumbnailItem Test");
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("uuid", "Window UUID", "uuid", ""));
    parser.addOption(QCommandLineOption("output", "Output ID (for workspace)", "outputId", ""));
    parser.addOption(QCommandLineOption("type", "Test type: window or workspace", "type", "window"));
    parser.process(app);

    QString uuid = parser.value("uuid");

    qmlRegisterType<UkuiThumbnail::WindowThumbnailItem>("org.ukui.windowThumbnail.test", 1, 0, "WindowThumbnailItem");
    qmlRegisterType<UkuiThumbnail::WorkspaceThumbnailItem>("org.ukui.windowThumbnail.test", 1, 0, "WorkspaceThumbnailItem");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("testUuid", uuid);
    engine.rootContext()->setContextProperty("testOutputId", parser.value("output"));
    engine.rootContext()->setContextProperty("testType", parser.value("type"));

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}