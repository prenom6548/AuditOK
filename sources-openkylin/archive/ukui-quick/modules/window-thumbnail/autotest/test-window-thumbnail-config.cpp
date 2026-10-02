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

#include <QGuiApplication>
#include <QtTest>

#include "window-thumbnail-config.h"

class window_thumbnail_config_test : public QObject
{
    Q_OBJECT

private slots:
    void realTimeThumbnailIsAlwaysEnabled();
    void nonWaylandPlatformUsesXThumbnail();
};

void window_thumbnail_config_test::realTimeThumbnailIsAlwaysEnabled()
{
    QVERIFY(WindowThumbnailConfig::realTimeThumbnailEnable());
}

void window_thumbnail_config_test::nonWaylandPlatformUsesXThumbnail()
{
    WindowThumbnailConfig config;

    QVERIFY(!qGuiApp->platformName().startsWith(QStringLiteral("wayland")));
    QVERIFY(config.xThumbnailEnable());
    QVERIFY(!config.pipewireThumbnailEnable());
    QVERIFY(!config.kywlcomWindowThumbnailEnable());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    window_thumbnail_config_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-window-thumbnail-config.moc"
