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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QGuiApplication>
#include <QQuickView>
#include <QCommandLineParser>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QDebug>
#include <QDir>

#include "kywc-capture-thumbnail-item.h"
#include "registry.h"
#include "ukui-workspace-manager.h"
#include "kywc-output-manager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("kywc-capture-test");
    QGuiApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("KyWC Capture Manager Test");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption typeOption(
        QStringList() << "t" << "type",
        "Capture type: toplevel, output, workspace",
        "type",
        "toplevel");
    parser.addOption(typeOption);

    QCommandLineOption uuidOption(
        QStringList() << "u" << "uuid",
        "UUID of the target (toplevel, output, or workspace)",
        "uuid");
    parser.addOption(uuidOption);

    QCommandLineOption outputUuidOption(
        QStringList() << "o" << "output-uuid",
        "Output UUID (required for workspace capture)",
        "output-uuid");
    parser.addOption(outputUuidOption);

    QCommandLineOption overlayCursorOption(
        QStringList() << "c" << "overlay-cursor",
        "Overlay cursor on capture",
        "true");
    parser.addOption(overlayCursorOption);

    QCommandLineOption withoutDecorationOption(
        QStringList() << "d" << "without-decoration",
        "Capture without decoration (toplevel only)",
        "false");
    parser.addOption(withoutDecorationOption);

    QCommandLineOption widthOption(
        QStringList() << "W" << "width",
        "Preview width",
        "width",
        "400");
    parser.addOption(widthOption);

    QCommandLineOption heightOption(
        QStringList() << "H" << "height",
        "Preview height",
        "height",
        "300");
    parser.addOption(heightOption);

    parser.process(app);

    QString type = parser.value(typeOption);
    QString uuid = parser.value(uuidOption);
    QString outputUuid = parser.value(outputUuidOption);
    bool overlayCursor = parser.value(overlayCursorOption) == "true";
    bool withoutDecoration = parser.value(withoutDecorationOption) == "true";
    int width = parser.value(widthOption).toInt();
    int height = parser.value(heightOption).toInt();

    qDebug() << "=== KyWC Capture Manager Test ===";
    qDebug() << "Capture type:" << type;
    qDebug() << "Target UUID:" << uuid;
    qDebug() << "Output UUID:" << outputUuid;
    qDebug() << "Overlay cursor:" << overlayCursor;
    qDebug() << "Without decoration:" << withoutDecoration;

    qmlRegisterType<KywcCaptureThumbnailItem>("KywcCapture", 1, 0, "KywcCaptureThumbnailItem");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    QObject::connect(view.engine(), &QQmlEngine::quit, &app, &QGuiApplication::quit);

    int captureType = (type == "output") ? 1 : (type == "workspace" ? 2 : 0);

    QString qmlFilePath = "qrc:/CapturePreview.qml";
    QQmlComponent component(view.engine(), qmlFilePath);
    if (component.isError()) {
        qWarning() << "Failed to load QML component";
        qWarning() << component.errors();
        return 1;
    }

    QObject *rootObject = component.create();
    if (!rootObject) {
        qWarning() << "Failed to create QML object";
        qWarning() << component.errors();
        return 1;
    }

    rootObject->setProperty("captureType", captureType);
    rootObject->setProperty("uuid", uuid);
    rootObject->setProperty("outputUuid", outputUuid);
    rootObject->setProperty("overlayCursor", overlayCursor);
    rootObject->setProperty("withoutDecoration", withoutDecoration);
    rootObject->setProperty("active", true);
    rootObject->setProperty("previewWidth", width);
    rootObject->setProperty("previewHeight", height);

    view.setContent(QUrl::fromLocalFile(qmlFilePath), &component, rootObject);
    view.show();

    qDebug() << "Window created. Waiting for capture...";
    if (uuid.isEmpty()) {
        qWarning() << "No UUID provided. Use --uuid option to specify target.";
        qDebug() << "\nUsage examples:";
        qDebug() << "  " << argv[0] << " --type toplevel --uuid <window-uuid>";
        qDebug() << "  " << argv[0] << " --type output --uuid <output-uuid>";
        qDebug() << "  " << argv[0] << " --type workspace --uuid <workspace-uuid> --output-uuid <output-uuid>";
    }

    return QGuiApplication::exec();
}