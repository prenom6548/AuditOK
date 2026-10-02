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
#include <QWindow>

#include "wm-interface.h"

using namespace UkuiQuick;

class TestWMInterface : public WMInterface
{
public:
    explicit TestWMInterface(QWindow *window)
        : WMInterface(window)
    {
    }

    void setWindowType(WindowType::Type type) override { Q_UNUSED(type); }
    void setSkipTaskBar(bool skip = true) override { Q_UNUSED(skip); }
    void setSkipSwitcher(bool skip = true) override { Q_UNUSED(skip); }
    void setRemoveTitleBar(bool remove = true) override { Q_UNUSED(remove); }
    void setPanelTakesFocus(bool takesFocus = true) override { Q_UNUSED(takesFocus); }
    void setPanelAutoHide(bool autoHide = false) override { Q_UNUSED(autoHide); }
    QScreen *currentScreen() override { return QGuiApplication::primaryScreen(); }
};

class wm_interface_test : public QObject
{
    Q_OBJECT

private slots:
    void isWaylandReflectsQtPlatformName();
    void windowAndPositionAreForwarded();
    void decorationComponentsDefaultImplementationIsNoop();
};

void wm_interface_test::isWaylandReflectsQtPlatformName()
{
    QCOMPARE(WMInterface::isWayland(), QGuiApplication::platformName().startsWith(QStringLiteral("wayland")));
}

void wm_interface_test::windowAndPositionAreForwarded()
{
    QWindow window;
    TestWMInterface wm(&window);

    QCOMPARE(wm.window(), &window);

    const QPoint expectedPosition(12, 34);
    wm.setPosition(expectedPosition);
    QCOMPARE(wm.position(), expectedPosition);
}

void wm_interface_test::decorationComponentsDefaultImplementationIsNoop()
{
    QWindow window;
    TestWMInterface wm(&window);

    wm.setDecorationComponents(Decoration::RoundCorner | Decoration::Border | Decoration::Shadow);
    QCOMPARE(wm.window(), &window);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("GSETTINGS_BACKEND", "memory");

    QGuiApplication app(argc, argv);
    wm_interface_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-wm-interface.moc"
