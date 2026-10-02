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

#include <QSignalSpy>
#include <QtTest>

#include "config-property-map.h"

using namespace UkuiQuick;

class TestableConfigPropertyMap : public ConfigPropertyMap
{
public:
    explicit TestableConfigPropertyMap(Config *config) : ConfigPropertyMap(config) {}
    using ConfigPropertyMap::updateValue;
};

class config_property_map_test : public QObject
{
    Q_OBJECT

private slots:
    void constructorLoadsExistingConfigKeys();
    void configChangesUpdatePropertyMap();
    void qmlSideUpdateWritesBackToConfig();
};

void config_property_map_test::constructorLoadsExistingConfigKeys()
{
    Config config(QVariantMap {
        {QStringLiteral("name"), QStringLiteral("launcher")},
        {QStringLiteral("size"), 32}
    }, static_cast<QObject *>(nullptr));

    TestableConfigPropertyMap map(&config);

    QCOMPARE(map.value(QStringLiteral("name")).toString(), QStringLiteral("launcher"));
    QCOMPARE(map.value(QStringLiteral("size")).toInt(), 32);
}

void config_property_map_test::configChangesUpdatePropertyMap()
{
    Config config(QVariantMap {
        {QStringLiteral("name"), QStringLiteral("launcher")}
    }, static_cast<QObject *>(nullptr));
    TestableConfigPropertyMap map(&config);

    config.setValue(QStringLiteral("name"), QStringLiteral("panel"));
    config.setValue(QStringLiteral("enabled"), true);

    QCOMPARE(map.value(QStringLiteral("name")).toString(), QStringLiteral("panel"));
    QCOMPARE(map.value(QStringLiteral("enabled")).toBool(), true);
}

void config_property_map_test::qmlSideUpdateWritesBackToConfig()
{
    Config config(QVariantMap {
        {QStringLiteral("count"), 1}
    }, static_cast<QObject *>(nullptr));
    TestableConfigPropertyMap map(&config);
    QSignalSpy changedSpy(&config, &Config::configChanged);

    QCOMPARE(map.updateValue(QStringLiteral("count"), 2).toInt(), 2);

    QCOMPARE(config.getValue(QStringLiteral("count")).toInt(), 2);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.first().at(0).toString(), QStringLiteral("count"));
}

QTEST_GUILESS_MAIN(config_property_map_test)

#include "test-config-property-map.moc"
