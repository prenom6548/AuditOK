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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>

#include "status-info.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace UkuiShortcut;

class StatusInfoTest : public QObject
{
    Q_OBJECT

private slots:
    void test_defaultValues();
    void test_settersGettersAndJson();
    void test_menuRoundTrip();
    void test_copyConstructorAndAssignment();
};

void StatusInfoTest::test_defaultValues()
{
    StatusInfo info;

    QCOMPARE(info.getInfoType(), StatusInfo::Normal);
    QCOMPARE(info.getName(), QString());
    QCOMPARE(info.getStatusName(), QString());
    QCOMPARE(info.getWidgetId(), QString());
    QCOMPARE(info.getIcon(), QString());
    QCOMPARE(info.getToolTip(), QString());
    QCOMPARE(info.getColor(), Color::BaseColor);
    QCOMPARE(info.getValue(), 0);
    QCOMPARE(info.getMaxValue(), 100);
    QCOMPARE(info.getMenu().size(), 0);
    QCOMPARE(info.isDisabled(), false);
}

void StatusInfoTest::test_settersGettersAndJson()
{
    StatusInfo info(StatusInfo::MenuInfo);

    info.setInfoType(StatusInfo::Normal);
    info.setDisable(true);
    info.setName(QStringLiteral("Network"));
    info.setStatusName(QStringLiteral("Connected"));
    info.setWidgetId(QStringLiteral("org.ukui.shortcut.network"));
    info.setIcon(QStringLiteral("network-wireless-symbolic"));
    info.setToolTip(QStringLiteral("Network tooltip"));
    info.setColor(QColor(Qt::red));
    info.setValue(42);
    info.setMaxValue(150);

    QCOMPARE(info.getInfoType(), StatusInfo::Normal);
    QCOMPARE(info.isDisabled(), true);
    QCOMPARE(info.getName(), QStringLiteral("Network"));
    QCOMPARE(info.getStatusName(), QStringLiteral("Connected"));
    QCOMPARE(info.getWidgetId(), QStringLiteral("org.ukui.shortcut.network"));
    QCOMPARE(info.getIcon(), QStringLiteral("network-wireless-symbolic"));
    QCOMPARE(info.getToolTip(), QStringLiteral("Network tooltip"));
    QCOMPARE(info.getColor(), Color::BaseColor);
    QCOMPARE(info.getValue(), 42);
    QCOMPARE(info.getMaxValue(), 150);

    info.setColor(Color::Danger);
    info.setDisable(false);
    QCOMPARE(info.getColor(), Color::Danger);
    QCOMPARE(info.isDisabled(), false);

    const QJsonObject object = QJsonDocument::fromJson(info.toJsonString().toUtf8()).object();
    QCOMPARE(object.value(QStringLiteral("infoType")).toInt(), static_cast<int>(StatusInfo::Normal));
    QCOMPARE(object.value(QString::number(ValueType::Disabled)).toBool(), false);
    QCOMPARE(object.value(QString::number(ValueType::Name)).toString(), QStringLiteral("Network"));
    QCOMPARE(object.value(QString::number(ValueType::StatusName)).toString(), QStringLiteral("Connected"));
    QCOMPARE(object.value(QString::number(ValueType::WidgetId)).toString(), QStringLiteral("org.ukui.shortcut.network"));
    QCOMPARE(object.value(QString::number(ValueType::Icon)).toString(), QStringLiteral("network-wireless-symbolic"));
    QCOMPARE(object.value(QString::number(ValueType::ToolTip)).toString(), QStringLiteral("Network tooltip"));
    QCOMPARE(object.value(QString::number(ValueType::Color)).toInt(), static_cast<int>(Color::Danger));
    QCOMPARE(object.value(QString::number(ValueType::Value)).toInt(), 42);
    QCOMPARE(object.value(QString::number(ValueType::MaxValue)).toInt(), 150);
}

void StatusInfoTest::test_menuRoundTrip()
{
    StatusInfo info(StatusInfo::MenuInfo);
    const QList<StatusInfo::MenuItem> menu = {
        {QStringLiteral("icon-a"), QStringLiteral("Text A"), QStringLiteral("status-a")},
        {QStringLiteral("icon-b"), QStringLiteral("Text B"), QStringLiteral("status-b")}
    };

    info.setMenu(menu);

    const QList<StatusInfo::MenuItem> result = info.getMenu();
    QCOMPARE(result.size(), 2);
    QCOMPARE(result.at(0).icon, QStringLiteral("icon-a"));
    QCOMPARE(result.at(0).text, QStringLiteral("Text A"));
    QCOMPARE(result.at(0).statusIcon, QStringLiteral("status-a"));
    QCOMPARE(result.at(1).icon, QStringLiteral("icon-b"));
    QCOMPARE(result.at(1).text, QStringLiteral("Text B"));
    QCOMPARE(result.at(1).statusIcon, QStringLiteral("status-b"));

    const QJsonArray array = QJsonDocument::fromJson(info.toJsonString().toUtf8())
                                 .object()
                                 .value(QString::number(ValueType::Menu))
                                 .toArray();
    QCOMPARE(array.size(), 2);
    QCOMPARE(array.at(0).toObject().value(QString::number(ValueType::MenuItemIcon)).toString(),
             QStringLiteral("icon-a"));
    QCOMPARE(array.at(0).toObject().value(QString::number(ValueType::MenuItemText)).toString(),
             QStringLiteral("Text A"));
    QCOMPARE(array.at(0).toObject().value(QString::number(ValueType::MenuItemStatusIcon)).toString(),
             QStringLiteral("status-a"));

    info.setMenu({});
    QCOMPARE(info.getMenu().size(), 0);
    QCOMPARE(QJsonDocument::fromJson(info.toJsonString().toUtf8())
                 .object()
                 .value(QString::number(ValueType::Menu))
                 .toArray()
                 .size(),
             0);
}

void StatusInfoTest::test_copyConstructorAndAssignment()
{
    StatusInfo source(StatusInfo::MenuInfo);
    source.setName(QStringLiteral("source"));
    source.setStatusName(QStringLiteral("ready"));
    source.setWidgetId(QStringLiteral("widget-id"));
    source.setIcon(QStringLiteral("icon"));
    source.setToolTip(QStringLiteral("tooltip"));
    source.setColor(Color::Warning);
    source.setValue(10);
    source.setMaxValue(20);
    source.setDisable(true);
    source.setMenu({{QStringLiteral("icon"), QStringLiteral("menu"), QStringLiteral("status")}});

    StatusInfo copied(source);
    QCOMPARE(copied.getInfoType(), StatusInfo::MenuInfo);
    QCOMPARE(copied.getName(), QStringLiteral("source"));
    QCOMPARE(copied.getStatusName(), QStringLiteral("ready"));
    QCOMPARE(copied.getWidgetId(), QStringLiteral("widget-id"));
    QCOMPARE(copied.getIcon(), QStringLiteral("icon"));
    QCOMPARE(copied.getToolTip(), QStringLiteral("tooltip"));
    QCOMPARE(copied.getColor(), Color::Warning);
    QCOMPARE(copied.getValue(), 10);
    QCOMPARE(copied.getMaxValue(), 20);
    QCOMPARE(copied.isDisabled(), true);
    QCOMPARE(copied.getMenu().size(), 1);

    source.setName(QStringLiteral("changed"));
    QCOMPARE(copied.getName(), QStringLiteral("source"));

    StatusInfo assigned;
    assigned = source;
    QCOMPARE(assigned.getInfoType(), StatusInfo::MenuInfo);
    QCOMPARE(assigned.getName(), QStringLiteral("changed"));
    QCOMPARE(assigned.getStatusName(), QStringLiteral("ready"));
    QCOMPARE(assigned.getWidgetId(), QStringLiteral("widget-id"));
    QCOMPARE(assigned.getIcon(), QStringLiteral("icon"));
    QCOMPARE(assigned.getToolTip(), QStringLiteral("tooltip"));
    QCOMPARE(assigned.getColor(), Color::Warning);
    QCOMPARE(assigned.getValue(), 10);
    QCOMPARE(assigned.getMaxValue(), 20);
    QCOMPARE(assigned.isDisabled(), true);
    QCOMPARE(assigned.getMenu().at(0).text, QStringLiteral("menu"));
}

QTEST_MAIN(StatusInfoTest)
#include "status-info-test.moc"
