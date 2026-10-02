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
#include <QQuickItem>
#include <QScreen>
#include <QSignalSpy>
#include <QtTest>

#include "views/content-window.h"

using namespace UkuiQuick;

class content_window_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndNoopSetters();
    void contentOwnershipAndSizingFollowItem();
    void positioningUsesScreenGeometryAndMargins();
    void noPositionLeavesWindowPositionUnchanged();
    void centerFamilyPositionsUseExpectedCoordinates();
    void legacySidePositionsUseExpectedCoordinates();
    void centerPositionUsesExpectedCoordinates();
    void replacingContentDetachesPreviousItem();
    void settingContentToNullDetachesPreviousItem();
};

void content_window_test::defaultsAndNoopSetters()
{
    ContentWindow window;
    QSignalSpy useAvailableSpy(&window, &ContentWindow::useAvailableGeometryChanged);
    QSignalSpy positionSpy(&window, &ContentWindow::positionChanged);
    QSignalSpy screenSpy(&window, &ContentWindow::screenChanged);

    QVERIFY(window.skipTaskBar());
    QVERIFY(window.skipSwitcher());
    QVERIFY(window.margin() != nullptr);
    QCOMPARE(window.content(), nullptr);
    QCOMPARE(window.getPos(), Types::NoPosition);
    QVERIFY(window.useAvailableGeometry());

    window.setUseAvailableGeometry(true);
    window.setPos(Types::NoPosition);
    window.setWindowScreen(window.getScreen());

    QCOMPARE(useAvailableSpy.count(), 0);
    QCOMPARE(positionSpy.count(), 0);
    QCOMPARE(screenSpy.count(), 0);
}

void content_window_test::contentOwnershipAndSizingFollowItem()
{
    ContentWindow window;
    QQuickItem content;
    QSignalSpy contentSpy(&window, &ContentWindow::contentChanged);

    content.setWidth(120);
    content.setHeight(45);

    window.setContent(&content);

    QCOMPARE(window.content(), &content);
    QCOMPARE(content.parentItem(), window.contentItem());
    QCOMPARE(window.width(), 120);
    QCOMPARE(window.height(), 45);
    QCOMPARE(contentSpy.count(), 1);

    content.setWidth(150);
    content.setHeight(60);
    QCOMPARE(window.width(), 150);
    QCOMPARE(window.height(), 60);

    window.setContent(&content);
    QCOMPARE(contentSpy.count(), 1);
}

void content_window_test::positioningUsesScreenGeometryAndMargins()
{
    ContentWindow window;
    QQuickItem content;
    QScreen *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    content.setWidth(100);
    content.setHeight(50);

    window.setContent(&content);
    window.setWindowScreen(screen);
    window.setUseAvailableGeometry(false);
    window.margin()->setLeft(10);
    window.margin()->setTop(20);
    window.margin()->setRight(30);
    window.margin()->setBottom(40);

    const QRect geometry = screen->geometry();

    window.setPos(Types::TopLeft);
    QCOMPARE(window.windowPosition(), QPoint(geometry.left() + 10, geometry.top() + 20));

    window.setPos(Types::BottomRight);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.right() - window.width() - 30,
                    geometry.bottom() - window.height() - 40));

    window.setPos(Types::BottomCenter);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + geometry.width() / 2 - window.width() / 2,
                    geometry.bottom() - window.height() - 40));

    window.reLocated();
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + geometry.width() / 2 - window.width() / 2,
                    geometry.bottom() - window.height() - 40));
}

void content_window_test::noPositionLeavesWindowPositionUnchanged()
{
    ContentWindow window;
    QQuickItem content;

    content.setWidth(100);
    content.setHeight(50);
    window.setContent(&content);
    window.setWindowPosition(QPoint(12, 34));

    window.setPos(Types::NoPosition);
    window.reLocated();

    QCOMPARE(window.windowPosition(), QPoint(12, 34));
}

void content_window_test::centerFamilyPositionsUseExpectedCoordinates()
{
    ContentWindow window;
    QQuickItem content;
    QScreen *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    content.setWidth(80);
    content.setHeight(40);
    window.setContent(&content);
    window.setWindowScreen(screen);
    window.setUseAvailableGeometry(false);
    window.margin()->setLeft(10);
    window.margin()->setTop(12);
    window.margin()->setRight(14);
    window.margin()->setBottom(16);

    const QRect geometry = screen->geometry();

    window.setPos(Types::LeftCenter);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + 10,
                    geometry.top() + geometry.height() / 2 - window.height() / 2));

    window.setPos(Types::TopCenter);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + geometry.width() / 2 - window.width() / 2,
                    geometry.top() + 12));

    window.setPos(Types::RightCenter);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.right() - window.width() - 14,
                    geometry.top() + geometry.height() / 2 - window.height() / 2));
}

void content_window_test::legacySidePositionsUseExpectedCoordinates()
{
    ContentWindow window;
    QQuickItem content;
    QScreen *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    content.setWidth(80);
    content.setHeight(40);
    window.setContent(&content);
    window.setWindowScreen(screen);
    window.setUseAvailableGeometry(false);
    window.margin()->setLeft(10);
    window.margin()->setTop(12);
    window.margin()->setRight(14);
    window.margin()->setBottom(16);

    const QRect geometry = screen->geometry();

    window.setPos(Types::TopRight);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.right() - window.width() - 14,
                    geometry.top() + 12));

    window.setPos(Types::BottomLeft);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + 10,
                    geometry.bottom() - window.height() - 16));

    window.setPos(Types::Left);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + 10,
                    geometry.top() + geometry.height() / 2 - window.height() / 2));

    window.setPos(Types::Top);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + geometry.width() / 2 - window.width() / 2,
                    geometry.top() + 12));

    window.setPos(Types::Right);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.right() - window.width() - 14,
                    geometry.top() + geometry.height() / 2 - window.height() / 2));

    window.setPos(Types::Bottom);
    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + geometry.width() / 2 - window.width() / 2,
                    geometry.bottom() - window.height() - 16));
}

void content_window_test::centerPositionUsesExpectedCoordinates()
{
    ContentWindow window;
    QQuickItem content;
    QScreen *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    content.setWidth(80);
    content.setHeight(40);
    window.setContent(&content);
    window.setWindowScreen(screen);
    window.setUseAvailableGeometry(false);

    const QRect geometry = screen->geometry();
    window.setPos(Types::Center);

    QCOMPARE(window.windowPosition(),
             QPoint(geometry.left() + geometry.width() / 2 - window.width() / 2,
                    geometry.top() + geometry.height() / 2 - window.height() / 2));
}

void content_window_test::replacingContentDetachesPreviousItem()
{
    ContentWindow window;
    QQuickItem first;
    QQuickItem second;

    first.setWidth(80);
    first.setHeight(30);
    second.setWidth(60);
    second.setHeight(20);

    window.setContent(&first);
    QCOMPARE(first.parentItem(), window.contentItem());

    window.setContent(&second);
    QCOMPARE(first.parentItem(), nullptr);
    QCOMPARE(second.parentItem(), window.contentItem());
    QCOMPARE(window.width(), 60);
    QCOMPARE(window.height(), 20);
}

void content_window_test::settingContentToNullDetachesPreviousItem()
{
    ContentWindow window;
    QQuickItem content;
    QSignalSpy contentSpy(&window, &ContentWindow::contentChanged);

    content.setWidth(80);
    content.setHeight(30);
    window.setContent(&content);
    QCOMPARE(content.parentItem(), window.contentItem());

    window.setContent(nullptr);
    QCOMPARE(window.content(), nullptr);
    QCOMPARE(content.parentItem(), nullptr);
    QCOMPARE(contentSpy.count(), 2);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    content_window_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-content-window.moc"
