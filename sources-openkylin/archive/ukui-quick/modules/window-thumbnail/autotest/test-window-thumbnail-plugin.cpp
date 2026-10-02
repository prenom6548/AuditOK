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
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QCoreApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtQml>
#include <QtTest>

#include "window-thumbnail-config.h"
#include "window-thumbnail-plugin.h"

namespace {

constexpr auto ThumbnailUri = "org.ukui.windowThumbnail";

}

class window_thumbnail_plugin_test : public QObject
{
    Q_OBJECT

private slots:
    void registerTypesExposesExpectedQmlTypes();
    void initializeEngineProvidesConfigContextProperty();
};

void window_thumbnail_plugin_test::registerTypesExposesExpectedQmlTypes()
{
    WindowThumbnailPlugin plugin;

    plugin.registerTypes(ThumbnailUri);

    QVERIFY(qmlTypeId(ThumbnailUri, 1, 0, "XWindowThumbnail") >= 0);
    QVERIFY(qmlTypeId(ThumbnailUri, 1, 0, "WindowThumbnailMprisModel") >= 0);
    QVERIFY(qmlTypeId(ThumbnailUri, 1, 0, "WindowThumbnailItem") >= 0);
    QVERIFY(qmlTypeId(ThumbnailUri, 1, 0, "WorkspaceThumbnailItem") >= 0);
    QVERIFY(qmlTypeId(ThumbnailUri, 1, 0, "MprisProperties") >= 0);
}

void window_thumbnail_plugin_test::initializeEngineProvidesConfigContextProperty()
{
    QQmlEngine engine;
    WindowThumbnailPlugin plugin;

    plugin.initializeEngine(&engine, ThumbnailUri);

    const QVariant property = engine.rootContext()->contextProperty(QStringLiteral("windowThumbnailConfig"));
    auto *config = qobject_cast<WindowThumbnailConfig *>(property.value<QObject *>());
    QVERIFY(config != nullptr);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    window_thumbnail_plugin_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-window-thumbnail-plugin.moc"
