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

#include <QApplication>
#include <QCursor>
#include <QSignalSpy>
#include <QToolTip>
#include <QWidget>
#include <QtTest>

#include "tooltip-proxy.h"

using namespace UkuiQuick;

class tooltip_proxy_test : public QObject
{
    Q_OBJECT

private slots:
    void textPropertyEmitsOnlyOnChange();
    void defaultShowUsesCursorFallback();
    void explicitPointOverloadsUseRequestedArea();
    void attachedPropertiesCreateParentedToolTip();

private:
    QWidget *tooltipLabel() const;
};

QWidget *tooltip_proxy_test::tooltipLabel() const
{
    const auto widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (widget && widget->objectName() == QStringLiteral("qtooltip_label")) {
            return widget;
        }
    }

    return nullptr;
}

void tooltip_proxy_test::textPropertyEmitsOnlyOnChange()
{
    ToolTip tooltip;
    QSignalSpy textChangedSpy(&tooltip, &ToolTip::textChanged);

    tooltip.setText(QStringLiteral("hello"));
    tooltip.setText(QStringLiteral("hello"));
    tooltip.setText(QStringLiteral("world"));

    QCOMPARE(tooltip.text(), QStringLiteral("world"));
    QCOMPARE(textChangedSpy.count(), 2);
}

void tooltip_proxy_test::defaultShowUsesCursorFallback()
{
    ToolTip tooltip;
    QCursor::setPos(140, 260);

    tooltip.setText(QStringLiteral("cursor-tip"));
    tooltip.show(QPoint());
    QCoreApplication::processEvents();

    QWidget *label = tooltipLabel();
    QVERIFY(label != nullptr);
    const QPoint expectedTopLeft = label->geometry().topLeft();
    QVERIFY2(expectedTopLeft.y() > 100, "Null-point fallback should place tooltip near cursor");

    tooltip.setText(QStringLiteral("cursor-tip-default"));
    tooltip.show();
    QCoreApplication::processEvents();

    label = tooltipLabel();
    QVERIFY(label != nullptr);
    QCOMPARE(QToolTip::text(), QStringLiteral("cursor-tip-default"));
    QCOMPARE(label->geometry().topLeft(), expectedTopLeft);
}

void tooltip_proxy_test::explicitPointOverloadsUseRequestedArea()
{
    ToolTip tooltip;
    QCursor::setPos(500, 500);

    tooltip.setText(QStringLiteral("pointf-tip"));
    tooltip.show(QPointF(30.5, 40.5));
    QCoreApplication::processEvents();

    QWidget *label = tooltipLabel();
    QVERIFY(label != nullptr);
    QCOMPARE(QToolTip::text(), QStringLiteral("pointf-tip"));
    const QPoint pointFTopLeft = label->geometry().topLeft();
    QVERIFY(pointFTopLeft.x() < 150);
    QVERIFY(pointFTopLeft.y() < 150);

    tooltip.setText(QStringLiteral("point-tip"));
    tooltip.show(QPoint(220, 260));
    QCoreApplication::processEvents();

    label = tooltipLabel();
    QVERIFY(label != nullptr);
    QCOMPARE(QToolTip::text(), QStringLiteral("point-tip"));
    const QPoint pointTopLeft = label->geometry().topLeft();
    QVERIFY(pointTopLeft.x() > pointFTopLeft.x() + 100);
    QVERIFY(pointTopLeft.y() > pointFTopLeft.y() + 100);

    tooltip.setText(QStringLiteral("int-tip"));
    tooltip.show(360, 390);
    QCoreApplication::processEvents();

    label = tooltipLabel();
    QVERIFY(label != nullptr);
    QCOMPARE(QToolTip::text(), QStringLiteral("int-tip"));
    const QPoint intTopLeft = label->geometry().topLeft();
    QVERIFY(intTopLeft.x() > pointTopLeft.x() + 100);
    QVERIFY(intTopLeft.y() > pointTopLeft.y() + 100);

    tooltip.hide();
    QCoreApplication::processEvents();
}

void tooltip_proxy_test::attachedPropertiesCreateParentedToolTip()
{
    QObject owner;
    ToolTip *attached = ToolTipAttached::qmlAttachedProperties(&owner);

    QVERIFY(attached != nullptr);
    QCOMPARE(attached->parent(), &owner);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);
    tooltip_proxy_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-tooltip-proxy.moc"
