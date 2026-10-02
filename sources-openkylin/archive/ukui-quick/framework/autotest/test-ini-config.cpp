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

#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "ini-config.h"

using namespace UkuiQuick;

class ini_config_test : public QObject
{
    Q_OBJECT

private slots:
    void valuesAreStoredReadAndSynced();
};

void ini_config_test::valuesAreStoredReadAndSynced()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileName = dir.filePath(QStringLiteral("settings.conf"));
    IniConfig config(fileName);
    QSignalSpy changedSpy(&config, &IniConfig::configChanged);

    QVERIFY(config.keys().isEmpty());
    QVERIFY(config.settings() != nullptr);

    config.setValue(QStringLiteral("General/enabled"), true);
    config.setValue(QStringLiteral("General/count"), 12);
    config.forceSync();

    QCOMPARE(changedSpy.count(), 2);
    QCOMPARE(config.getValue(QStringLiteral("General/enabled")).toBool(), true);
    QCOMPARE(config.getValue(QStringLiteral("General/count")).toInt(), 12);
    QVERIFY(config.keys().contains(QStringLiteral("General/enabled")));
    QVERIFY(config.keys().contains(QStringLiteral("General/count")));

    QSettings settings(fileName, QSettings::IniFormat);
    QCOMPARE(settings.value(QStringLiteral("General/enabled")).toBool(), true);
    QCOMPARE(settings.value(QStringLiteral("General/count")).toInt(), 12);
}

QTEST_GUILESS_MAIN(ini_config_test)

#include "test-ini-config.moc"
