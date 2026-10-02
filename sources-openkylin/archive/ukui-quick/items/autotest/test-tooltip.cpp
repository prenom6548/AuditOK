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
#include <QMouseEvent>
#include <QSignalSpy>
#include <QtTest>

#include "views/tooltip.h"

using namespace UkuiQuick;

namespace {

class TestableTooltip : public Tooltip
{
public:
    using Tooltip::Tooltip;
    using Tooltip::childMouseEventFilter;
    using Tooltip::hoverEnterEvent;
    using Tooltip::hoverLeaveEvent;
    using Tooltip::mousePressEvent;
    using Tooltip::tooltipDialogInstance;
};

}

class tooltip_test : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAndPropertySignals();
    void showAndHideTooltipWithCustomItem();
    void hoverAndMousePressManageStateWithoutVisibleDialog();
};

void tooltip_test::defaultsAndPropertySignals()
{
    TestableTooltip tooltip;
    QQuickItem item;
    QSignalSpy textSpy(&tooltip, &Tooltip::mainTextChanged);
    QSignalSpy richTextSpy(&tooltip, &Tooltip::isRichTextChanged);
    QSignalSpy itemSpy(&tooltip, &Tooltip::mainItemChanged);
    QSignalSpy locationSpy(&tooltip, &Tooltip::locationChanged);
    QSignalSpy containsSpy(&tooltip, &Tooltip::containsMouseChanged);
    QSignalSpy marginSpy(&tooltip, &Tooltip::marginChanged);
    QSignalSpy interactiveSpy(&tooltip, &Tooltip::interactiveChanged);

    QCOMPARE(tooltip.location(), Dialog::Floating);
    QVERIFY(!tooltip.containsMouse());
    QVERIFY(tooltip.posFollowCursor());
    QCOMPARE(tooltip.margin(), 0);
    QVERIFY(!tooltip.property("isRichText").toBool());
    QVERIFY(tooltip.property("active").toBool());

    tooltip.setMainText(QStringLiteral("plain"));
    QCOMPARE(textSpy.count(), 1);
    QCOMPARE(richTextSpy.count(), 0);
    QVERIFY(!tooltip.property("isRichText").toBool());

    tooltip.setMainText(QStringLiteral("<b>rich</b>"));
    QCOMPARE(textSpy.count(), 2);
    QCOMPARE(richTextSpy.count(), 1);
    QVERIFY(tooltip.property("isRichText").toBool());

    tooltip.setMainItem(&item);
    QCOMPARE(itemSpy.count(), 1);
    QCOMPARE(tooltip.mainItem(), &item);

    tooltip.setLocation(Dialog::BottomEdge);
    QCOMPARE(locationSpy.count(), 1);
    QCOMPARE(tooltip.location(), Dialog::BottomEdge);

    tooltip.setContainsMouse(true);
    QCOMPARE(containsSpy.count(), 1);
    QVERIFY(tooltip.containsMouse());

    tooltip.setPosFollowCursor(false);
    QVERIFY(!tooltip.posFollowCursor());

    tooltip.setMargin(6);
    QCOMPARE(marginSpy.count(), 1);
    QCOMPARE(tooltip.margin(), 6);

    tooltip.setInteractive(true);
    QCOMPARE(interactiveSpy.count(), 1);
    QVERIFY(tooltip.property("interactive").toBool());
}

void tooltip_test::showAndHideTooltipWithCustomItem()
{
    TestableTooltip tooltip;
    QQuickItem item;
    QSignalSpy aboutToShowSpy(&tooltip, &Tooltip::aboutToShow);
    QSignalSpy visibleSpy(&tooltip, &Tooltip::toolTipVisibleChanged);

    tooltip.setMainItem(&item);
    tooltip.setMainText(QStringLiteral("custom tooltip"));
    tooltip.setLocation(Dialog::BottomEdge);
    tooltip.setPosFollowCursor(false);
    tooltip.setMargin(9);
    tooltip.setInteractive(true);
    tooltip.setTimeout(40);

    tooltip.showTooltip();

    TooltipDialog *dialog = tooltip.tooltipDialogInstance();
    QVERIFY(dialog != nullptr);
    QVERIFY(dialog->isVisible());
    QCOMPARE(dialog->owner(), &tooltip);
    QCOMPARE(dialog->location(), Dialog::BottomEdge);
    QVERIFY(!dialog->posFollowCursor());
    QCOMPARE(dialog->margin(), 9);
    QVERIFY(dialog->interactive());
    QCOMPARE(dialog->mainItem(), &item);
    QVERIFY(item.isVisible());
    QCOMPARE(aboutToShowSpy.count(), 1);
    QVERIFY(visibleSpy.count() >= 1);

    tooltip.hideImmediately();
    QVERIFY(!dialog->isVisible());
}

void tooltip_test::hoverAndMousePressManageStateWithoutVisibleDialog()
{
    TestableTooltip tooltip;
    QQuickItem child;
    QSignalSpy containsSpy(&tooltip, &Tooltip::containsMouseChanged);
    QTimer *showTimer = tooltip.findChild<QTimer *>();
    QVERIFY(showTimer != nullptr);

    tooltip.setMainText(QStringLiteral("hover tip"));

    QHoverEvent enterEvent(QEvent::HoverEnter, QPointF(2, 2), QPointF(2, 2));
    tooltip.hoverEnterEvent(&enterEvent);
    QVERIFY(tooltip.containsMouse());
    QCOMPARE(containsSpy.count(), 1);
    QVERIFY(showTimer->isActive());

    QHoverEvent leaveEvent(QEvent::HoverLeave, QPointF(2, 2), QPointF(2, 2));
    tooltip.hoverLeaveEvent(&leaveEvent);
    QVERIFY(!tooltip.containsMouse());
    QCOMPARE(containsSpy.count(), 2);
    QVERIFY(!showTimer->isActive());

    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    tooltip.mousePressEvent(&pressEvent);
    QVERIFY(!tooltip.tooltipDialogInstance()->isVisible());

    QMouseEvent childPressEvent(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(!tooltip.childMouseEventFilter(&child, &childPressEvent));
    QVERIFY(!tooltip.tooltipDialogInstance()->isVisible());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");
    qputenv("QT_QUICK_BACKEND", "software");
    qputenv("QT_FATAL_WARNINGS", "0");
    qputenv("QT_LOGGING_RULES", "qt.qpa.*=false");

    QGuiApplication app(argc, argv);
    tooltip_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-tooltip.moc"
