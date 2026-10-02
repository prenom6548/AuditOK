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
#include <QScreen>
#include <QtTest>

#include "screen-area-utils.h"

using namespace UkuiQuick;

class screen_area_utils_test : public QObject
{
    Q_OBJECT

private slots:
    void instanceIsShared();
    void availableGeometryStaysWithinScreenGeometry();
};

void screen_area_utils_test::instanceIsShared()
{
    QCOMPARE(ScreenAreaUtils::instance(), ScreenAreaUtils::instance());
}

void screen_area_utils_test::availableGeometryStaysWithinScreenGeometry()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    const QRect geometry = screen->geometry();
    const QRect availableGeometry = ScreenAreaUtils::instance()->getAvailableGeometry(screen);

    QVERIFY(geometry.isValid());
    QVERIFY(availableGeometry.isValid());
    QVERIFY(geometry.contains(availableGeometry));
    QVERIFY(availableGeometry.width() <= geometry.width());
    QVERIFY(availableGeometry.height() <= geometry.height());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    screen_area_utils_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-screen-area-utils.moc"
