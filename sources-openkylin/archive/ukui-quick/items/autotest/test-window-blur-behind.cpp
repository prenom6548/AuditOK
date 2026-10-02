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
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest>

#include "window-blur-behind.h"

using namespace UkuiQuick;

class window_blur_behind_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndNoopSetters();
    void listPropertyHelpersManageUniqueRectangles();
    void windowAssignmentAllowsBlurUpdates();
};

void window_blur_behind_test::defaultsAndNoopSetters()
{
    WindowBlurBehind blur;
    QSignalSpy enableSpy(&blur, &WindowBlurBehind::enableChanged);
    QSignalSpy windowSpy(&blur, &WindowBlurBehind::windowChanged);

    QVERIFY(!blur.enable());
    QCOMPARE(blur.window(), nullptr);

    blur.setEnable(false);
    blur.setWindow(nullptr);

    QCOMPARE(enableSpy.count(), 0);
    QCOMPARE(windowSpy.count(), 0);

    blur.setEnable(true);
    QVERIFY(blur.enable());
    QCOMPARE(enableSpy.count(), 1);

    blur.setEnable(true);
    QCOMPARE(enableSpy.count(), 1);
}

void window_blur_behind_test::listPropertyHelpersManageUniqueRectangles()
{
    WindowBlurBehind blur;
    RectRegion first;
    RectRegion second;
    RectRegion replacement;
    QQmlListProperty<RectRegion> list = blur.rectRegions();

    WindowBlurBehind::appendRect(&list, nullptr);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 0);

    WindowBlurBehind::appendRect(&list, &first);
    WindowBlurBehind::appendRect(&list, &first);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 1);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 0), &first);

    WindowBlurBehind::appendRect(&list, &second);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 2);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 1), &second);
    QCOMPARE(WindowBlurBehind::rectAt(&list, -1), nullptr);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 2), nullptr);

    WindowBlurBehind::replaceRect(&list, 1, &replacement);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 1), &replacement);

    WindowBlurBehind::replaceRect(&list, 5, &second);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 1), &replacement);

    WindowBlurBehind::replaceRect(&list, 1, nullptr);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 1), &replacement);

    WindowBlurBehind::removeLastRect(&list);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 1);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 0), &first);

    blur.addRectRegion(&second);
    blur.addRectRegion(&second);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 2);
    QCOMPARE(WindowBlurBehind::rectAt(&list, 1), &second);

    blur.clearRectRegion();
    QCOMPARE(WindowBlurBehind::rectCount(&list), 0);

    WindowBlurBehind::appendRect(&list, &first);
    WindowBlurBehind::appendRect(&list, &second);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 2);

    WindowBlurBehind::clearRect(&list);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 0);

    WindowBlurBehind::removeLastRect(&list);
    QCOMPARE(WindowBlurBehind::rectCount(&list), 0);
}

void window_blur_behind_test::windowAssignmentAllowsBlurUpdates()
{
    WindowBlurBehind blur;
    QQuickWindow window;
    RectRegion region;
    QSignalSpy windowSpy(&blur, &WindowBlurBehind::windowChanged);

    region.setX(5);
    region.setY(6);
    region.setWidth(20);
    region.setHeight(10);
    region.setRadius(4);

    blur.setEnable(true);
    blur.setWindow(&window);
    QCOMPARE(blur.window(), &window);
    QCOMPARE(windowSpy.count(), 1);

    blur.setWindow(&window);
    QCOMPARE(windowSpy.count(), 1);

    blur.addRectRegion(&region);
    QQmlListProperty<RectRegion> list = blur.rectRegions();
    QCOMPARE(WindowBlurBehind::rectCount(&list), 1);

    blur.clearRectRegion();
    QCOMPARE(WindowBlurBehind::rectCount(&list), 0);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    window_blur_behind_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-window-blur-behind.moc"
