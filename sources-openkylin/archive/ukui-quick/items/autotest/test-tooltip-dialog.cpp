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
#include <QHoverEvent>
#include <QScreen>
#include <QtTest>

#include "views/tooltip-dialog.h"

using namespace UkuiQuick;

namespace {

class TestableTooltipDialog : public TooltipDialog
{
public:
    using TooltipDialog::TooltipDialog;
    using TooltipDialog::event;
    using TooltipDialog::hideEvent;
    using TooltipDialog::popupPosition;
    using TooltipDialog::showEvent;
};

}

class tooltip_dialog_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndSetters();
    void showHideDismissAndKeepaliveControlTimer();
    void eventBranchesUpdateTimerAndFlags();
    void popupPositionIsKeptInsideScreenWhenFollowingCursor();
};

void tooltip_dialog_test::defaultsAndSetters()
{
    TestableTooltipDialog dialog;

    QCOMPARE(dialog.location(), Dialog::Floating);
    QCOMPARE(dialog.type(), WindowType::ToolTip);
    QVERIFY(dialog.enableWindowBlur());
    QVERIFY(dialog.owner() == nullptr);
    QVERIFY(!dialog.interactive());
    QCOMPARE(dialog.hideTimeout(), 4000);
    QVERIFY(dialog.posFollowCursor());
    QCOMPARE(dialog.margin(), 0);

    dialog.setInteractive(true);
    dialog.setOwner(this);
    dialog.setHideTimeout(120);
    dialog.setPosFollowCursor(false);
    dialog.setMargin(8);
    QMetaObject::invokeMethod(&dialog, "valueChanged", Q_ARG(QVariant, QVariant(QPoint(5, 6))));

    QVERIFY(dialog.interactive());
    QCOMPARE(dialog.owner(), this);
    QCOMPARE(dialog.hideTimeout(), 120);
    QVERIFY(!dialog.posFollowCursor());
    QCOMPARE(dialog.margin(), 8);
    QCOMPARE(dialog.position(), QPoint(5, 6));
}

void tooltip_dialog_test::showHideDismissAndKeepaliveControlTimer()
{
    TestableTooltipDialog dialog;
    auto *timer = dialog.findChild<QTimer *>();
    QVERIFY(timer != nullptr);

    dialog.setHideTimeout(50);

    QShowEvent showEvent;
    dialog.showEvent(&showEvent);
    QVERIFY(timer->isActive());
    QCOMPARE(timer->interval(), 50);

    dialog.dismiss();
    QVERIFY(timer->isActive());
    QCOMPARE(timer->interval(), 2);

    dialog.keepalive();
    QVERIFY(timer->isActive());
    QCOMPARE(timer->interval(), 50);

    QHideEvent hideEvent;
    dialog.hideEvent(&hideEvent);
    QVERIFY(!timer->isActive());
}

void tooltip_dialog_test::eventBranchesUpdateTimerAndFlags()
{
    TestableTooltipDialog dialog;
    auto *timer = dialog.findChild<QTimer *>();
    QVERIFY(timer != nullptr);

    dialog.setHideTimeout(80);
    dialog.setInteractive(true);
    dialog.dismiss();
    QVERIFY(timer->isActive());

    QEvent enterEvent(QEvent::Enter);
    dialog.event(&enterEvent);
    QVERIFY(!timer->isActive());

    QEvent resizeEvent(QEvent::Resize);
    dialog.event(&resizeEvent);
    QEvent moveEvent(QEvent::Move);
    dialog.event(&moveEvent);
    QEvent leaveEvent(QEvent::Leave);
    dialog.event(&leaveEvent);
    QVERIFY(timer->isActive());
    QCOMPARE(timer->interval(), 80);

    QEvent mouseMoveEvent(QEvent::MouseMove);
    dialog.event(&mouseMoveEvent);
    dialog.event(&leaveEvent);
    QVERIFY(timer->isActive());
    QCOMPARE(timer->interval(), 4);

    QVERIFY(dialog.flags().testFlag(Qt::ToolTip));
    QVERIFY(dialog.flags().testFlag(Qt::WindowDoesNotAcceptFocus));
    QVERIFY(dialog.flags().testFlag(Qt::WindowStaysOnTopHint));
}

void tooltip_dialog_test::popupPositionIsKeptInsideScreenWhenFollowingCursor()
{
    TestableTooltipDialog dialog;
    QScreen *screen = QGuiApplication::primaryScreen();
    QVERIFY(screen != nullptr);

    dialog.resize(40, 20);
    dialog.setPosFollowCursor(true);

    const QPoint pos = dialog.popupPosition(nullptr, QSize(40, 20));
    const QRect geometry = screen->geometry();

    QVERIFY(pos.x() >= geometry.left());
    QVERIFY(pos.y() >= geometry.top());
    QVERIFY(pos.x() + dialog.width() <= geometry.right() + 1);
    QVERIFY(pos.y() + dialog.height() <= geometry.bottom() + 1);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    tooltip_dialog_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-tooltip-dialog.moc"
