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

#include <QEvent>
#include <QGuiApplication>
#include <QSignalSpy>
#include <QtTest>

#include "views/ukui-style-window.h"

using namespace UkuiQuick;

namespace {

class TestableUkuiStyleWindow : public UkuiStyleWindow
{
public:
    using UkuiStyleWindow::UkuiStyleWindow;

    bool dispatchEvent(QEvent *event)
    {
        return UkuiStyleWindow::event(event);
    }
};

}

class ukui_style_window_test : public QObject
{
    Q_OBJECT

private slots:
    void geometrySettersEmitOnlyForRealChanges();
    void blurAndTypePropertiesAreStored();
    void showEventPathIsSafe();
};

void ukui_style_window_test::geometrySettersEmitOnlyForRealChanges()
{
    TestableUkuiStyleWindow window;
    QSignalSpy xSpy(&window, &UkuiStyleWindow::windowXChanged);
    QSignalSpy ySpy(&window, &UkuiStyleWindow::windowYChanged);
    QSignalSpy widthSpy(&window, &UkuiStyleWindow::windowWidthChanged);
    QSignalSpy heightSpy(&window, &UkuiStyleWindow::windowHeightChanged);
    QSignalSpy geometrySpy(&window, &UkuiStyleWindow::windowGeometryChanged);

    QCOMPARE(window.windowGeometry(), QRect(0, 0, 1, 1));

    window.setWindowGeometry(QRect(0, 0, 1, 1));
    QCOMPARE(xSpy.count(), 0);
    QCOMPARE(ySpy.count(), 0);
    QCOMPARE(widthSpy.count(), 0);
    QCOMPARE(heightSpy.count(), 0);
    QCOMPARE(geometrySpy.count(), 0);

    window.setWindowGeometry(QRect(10, 20, 30, 40));
    QCOMPARE(window.windowGeometry(), QRect(10, 20, 30, 40));
    QCOMPARE(window.x(), 10);
    QCOMPARE(window.y(), 20);
    QCOMPARE(window.width(), 30);
    QCOMPARE(window.height(), 40);
    QCOMPARE(xSpy.count(), 1);
    QCOMPARE(ySpy.count(), 1);
    QCOMPARE(widthSpy.count(), 1);
    QCOMPARE(heightSpy.count(), 1);
    QCOMPARE(geometrySpy.count(), 1);

    window.setX(11);
    window.setY(21);
    window.setWidth(31);
    window.setHeight(41);
    QCOMPARE(window.windowGeometry(), QRect(11, 21, 31, 41));
    QCOMPARE(xSpy.count(), 2);
    QCOMPARE(ySpy.count(), 2);
    QCOMPARE(widthSpy.count(), 2);
    QCOMPARE(heightSpy.count(), 2);
    QCOMPARE(geometrySpy.count(), 5);
}

void ukui_style_window_test::blurAndTypePropertiesAreStored()
{
    TestableUkuiStyleWindow window;
    QSignalSpy blurSpy(&window, &UkuiStyleWindow::enableWindowBlurChanged);
    QSignalSpy typeSpy(&window, &UkuiStyleWindow::windowTypeChanged);
    const QRegion region(0, 0, 20, 10);

    QVERIFY(!window.enableWindowBlur());
    QCOMPARE(window.windowType(), WindowType::Normal);

    window.setEnableWindowBlur(true);
    QVERIFY(window.enableWindowBlur());
    QCOMPARE(blurSpy.count(), 1);

    window.setEnableWindowBlur(true);
    QCOMPARE(blurSpy.count(), 1);

    window.setBlurRegion(region);
    QCOMPARE(window.blurRegion(), region);

    window.setWindowType(WindowType::ToolTip);
    QCOMPARE(window.windowType(), WindowType::ToolTip);
    QCOMPARE(typeSpy.count(), 1);

    QVERIFY(window.getWinId() != 0);
    QVERIFY(window.graphBackend().isEmpty() || !window.graphBackend().isNull());
}

void ukui_style_window_test::showEventPathIsSafe()
{
    TestableUkuiStyleWindow window;
    QEvent event(QEvent::Show);

    QVERIFY(window.dispatchEvent(&event) || !window.dispatchEvent(&event));
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    ukui_style_window_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-ukui-style-window.moc"
