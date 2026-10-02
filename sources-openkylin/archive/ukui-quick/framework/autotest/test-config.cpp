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

#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "config.h"

using namespace UkuiQuick;

namespace {

bool writeJson(const QString &fileName, const QJsonObject &object)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        return false;
    }

    file.write(QJsonDocument(object).toJson());
    return true;
}

QJsonObject readJson(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return {};
    }

    return QJsonDocument::fromJson(file.readAll()).object();
}

}

class config_test : public QObject
{
    Q_OBJECT

private slots:
    void missingFileIsCreatedWithEmptyObject();
    void fileBackedConfigReadsWritesAndRemovesKeys();
    void mapBackedConfigStoresValuesAndEmitsSignals();
    void childGroupsAreExtractedAddedAndRemoved();
};

void config_test::missingFileIsCreatedWithEmptyObject()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileName = dir.filePath(QStringLiteral("nested/config.json"));

    Config config(fileName);

    QVERIFY(config.isRoot());
    QVERIFY(config.isNull());
    QVERIFY(QFile::exists(fileName));
    QCOMPARE(readJson(fileName), QJsonObject());
}

void config_test::fileBackedConfigReadsWritesAndRemovesKeys()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileName = dir.filePath(QStringLiteral("config.json"));
    QVERIFY(writeJson(fileName, {
        {QStringLiteral("enabled"), true},
        {QStringLiteral("count"), 7},
        {QStringLiteral("name"), QStringLiteral("panel")}
    }));

    Config config(fileName);
    QSignalSpy changedSpy(&config, &Config::configChanged);

    QCOMPARE(config.keys().size(), 3);
    QVERIFY(config.contains(QStringLiteral("enabled")));
    QCOMPARE(config.getValue(QStringLiteral("count")).toInt(), 7);
    QCOMPARE(config.getValue(QStringLiteral("name")).toString(), QStringLiteral("panel"));

    config.setValue(QStringLiteral("count"), 8);
    config.removeKey(QStringLiteral("enabled"));
    config.forceSync();
    QCoreApplication::sendPostedEvents(&config, QEvent::UpdateRequest);

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.takeFirst().at(0).toString(), QStringLiteral("count"));
    const QJsonObject saved = readJson(fileName);
    QCOMPARE(saved.value(QStringLiteral("count")).toInt(), 8);
    QCOMPARE(saved.value(QStringLiteral("name")).toString(), QStringLiteral("panel"));
    QVERIFY(!saved.contains(QStringLiteral("enabled")));
}

void config_test::mapBackedConfigStoresValuesAndEmitsSignals()
{
    Config config(QVariantMap {
        {QStringLiteral("name"), QStringLiteral("dock")},
        {QStringLiteral("size"), 48}
    }, static_cast<QObject *>(nullptr));
    QSignalSpy changedSpy(&config, &Config::configChanged);

    QVERIFY(config.isRoot());
    QVERIFY(!config.isNull());
    QCOMPARE(config.data().value(QStringLiteral("name")).toString(), QStringLiteral("dock"));

    config.setValue(QStringLiteral("size"), 64);
    config.setValue(QStringLiteral("newKey"), QStringLiteral("newValue"));

    QCOMPARE(config.getValue(QStringLiteral("size")).toInt(), 64);
    QCOMPARE(config.getValue(QStringLiteral("newKey")).toString(), QStringLiteral("newValue"));
    QCOMPARE(changedSpy.count(), 2);
}

void config_test::childGroupsAreExtractedAddedAndRemoved()
{
    QVariantMap firstChild {
        {QStringLiteral("id"), QStringLiteral("alpha")},
        {QStringLiteral("label"), QStringLiteral("Alpha")}
    };
    QVariantMap secondChild {
        {QStringLiteral("id"), QStringLiteral("beta")},
        {QStringLiteral("label"), QStringLiteral("Beta")}
    };
    Config config(QVariantMap {
        {QStringLiteral("items"), QVariantList { firstChild }}
    }, static_cast<QObject *>(nullptr));
    QSignalSpy addedSpy(&config, &Config::childAdded);
    QSignalSpy removedSpy(&config, &Config::childRemoved);

    config.addGroupInfo(QStringLiteral("items"), QStringLiteral("id"));

    QCOMPARE(config.groupInfo().value(QStringLiteral("items")), QStringLiteral("id"));
    QCOMPARE(config.numberOfChildren(QStringLiteral("items")), 1);
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.first().at(0).toString(), QStringLiteral("items"));
    QCOMPARE(addedSpy.first().at(1).toString(), QStringLiteral("alpha"));
    Config *alpha = config.child(QStringLiteral("items"), QStringLiteral("alpha"));
    QVERIFY(alpha != nullptr);
    QVERIFY(!alpha->isRoot());
    QCOMPARE(alpha->group(), QStringLiteral("items"));
    QCOMPARE(alpha->id().toString(), QStringLiteral("alpha"));
    QCOMPARE(alpha->getValue(QStringLiteral("label")).toString(), QStringLiteral("Alpha"));

    config.addChild(QStringLiteral("items"), secondChild);
    config.addChild(QStringLiteral("items"), secondChild);

    QCOMPARE(config.numberOfChildren(QStringLiteral("items")), 2);
    QCOMPARE(addedSpy.count(), 2);
    QCOMPARE(addedSpy.at(1).at(0).toString(), QStringLiteral("items"));
    QCOMPARE(addedSpy.at(1).at(1).toString(), QStringLiteral("beta"));

    config.removeChild(QStringLiteral("items"), QStringLiteral("beta"));
    config.removeChild(QStringLiteral("items"), QStringLiteral("missing"));

    QCOMPARE(config.numberOfChildren(QStringLiteral("items")), 1);
    QCOMPARE(config.child(QStringLiteral("items"), QStringLiteral("beta")), nullptr);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.first().at(0).toString(), QStringLiteral("items"));
    QCOMPARE(removedSpy.first().at(1).toString(), QStringLiteral("beta"));

    config.setValue(QStringLiteral("items"), QVariantList { secondChild });
    QCOMPARE(config.numberOfChildren(QStringLiteral("items")), 1);
}

QTEST_GUILESS_MAIN(config_test)

#include "test-config.moc"
