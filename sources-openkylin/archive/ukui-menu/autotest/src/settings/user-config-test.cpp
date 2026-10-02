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

#define private public
#include "../../../src/settings/user-config.h"
#undef private

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace UkuiMenu;

namespace {

QString configDirPath()
{
    return UserConfig::configFilePath;
}

QString configFilePath()
{
    return UserConfig::configFilePath + UserConfig::configFileName;
}

QString configDirAsFilePath()
{
    QString path = UserConfig::configFilePath;
    if (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    return path;
}

QJsonDocument readJsonDocument(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    return QJsonDocument::fromJson(file.readAll());
}

}

class userConfig_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void test_instance_returnsSingleton();
    void test_firstStartup_createsConfigFile();
    void test_firstStartup_handlesDirectoryCreationFailure();
    void test_initPreInstalledLists_andSync_writeJson();
    void test_readExistingConfig_populatesPreInstalledApps();
    void test_readConfigFile_handlesMissingFileAndMixedObjects();
    void test_invalidConfig_isIgnored();
    void test_addAndRemovePreInstalledApp_updatesStateAndFile();
    void test_writeConfigFile_returnsWhenTargetIsDirectory();
};

void userConfig_test::initTestCase()
{
    QFile::remove(configDirAsFilePath());
    QDir(configDirPath()).removeRecursively();
}

void userConfig_test::init()
{
    QFile::remove(configDirAsFilePath());
    QDir(configDirPath()).removeRecursively();
    QDir().mkpath(configDirPath());
}

void userConfig_test::test_instance_returnsSingleton()
{
    UserConfig *first = UserConfig::instance();
    UserConfig *second = UserConfig::instance();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
}

void userConfig_test::test_firstStartup_createsConfigFile()
{
    QDir(configDirPath()).removeRecursively();

    UserConfig config;

    QCOMPARE(config.isFirstStartUp(), true);
    QVERIFY(QDir(configDirPath()).exists());
    QVERIFY(QFile::exists(configFilePath()));
    QCOMPARE(config.preInstalledApps().isEmpty(), true);
}

void userConfig_test::test_firstStartup_handlesDirectoryCreationFailure()
{
    QDir(configDirPath()).removeRecursively();
    QVERIFY(QDir().mkpath(QFileInfo(configDirAsFilePath()).absolutePath()));

    QFile blocker(configDirAsFilePath());
    QVERIFY(blocker.open(QIODevice::WriteOnly));
    blocker.close();

    UserConfig config;

    QCOMPARE(config.isFirstStartUp(), true);
    QVERIFY(!QFile::exists(configFilePath()));
}

void userConfig_test::test_initPreInstalledLists_andSync_writeJson()
{
    UserConfig config;

    config.initPreInstalledLists({QStringLiteral("app.a"), QStringLiteral("app.b"), QStringLiteral("app.a")});

    QCOMPARE(config.preInstalledApps().size(), 2);
    QVERIFY(config.isPreInstalledApps(QStringLiteral("app.a")));
    QVERIFY(config.isPreInstalledApps(QStringLiteral("app.b")));

    const QJsonDocument doc = readJsonDocument(configFilePath());
    QVERIFY(doc.isArray());
    const QJsonArray root = doc.array();
    QCOMPARE(root.size(), 1);
    const QJsonArray apps = root.at(0).toObject().value(QStringLiteral("PreInstalledApps")).toArray();
    QCOMPARE(apps.size(), 2);
}

void userConfig_test::test_readExistingConfig_populatesPreInstalledApps()
{
    QFile file(configFilePath());
    QVERIFY(file.open(QIODevice::WriteOnly));

    QJsonArray root;
    QJsonObject object;
    object.insert(QStringLiteral("PreInstalledApps"),
                  QJsonArray({QStringLiteral("alpha.desktop"), QStringLiteral("beta.desktop")}));
    root.append(object);
    file.write(QJsonDocument(root).toJson());
    file.close();

    UserConfig config;

    QCOMPARE(config.isFirstStartUp(), false);
    QCOMPARE(config.preInstalledApps().size(), 2);
    QVERIFY(config.isPreInstalledApps(QStringLiteral("alpha.desktop")));
    QVERIFY(config.isPreInstalledApps(QStringLiteral("beta.desktop")));
}

void userConfig_test::test_readConfigFile_handlesMissingFileAndMixedObjects()
{
    UserConfig config;
    config.addPreInstalledApp(QStringLiteral("stale.desktop"));

    QFile::remove(configFilePath());
    config.readConfigFile();
    QVERIFY(config.preInstalledApps().isEmpty());

    QFile file(configFilePath());
    QVERIFY(file.open(QIODevice::WriteOnly));

    QJsonArray root;
    root.append(QJsonObject{{QStringLiteral("Ignored"), QJsonArray{QStringLiteral("ignored.desktop")}}});
    root.append(QJsonObject{{QStringLiteral("PreInstalledApps"), QJsonArray{
        QStringLiteral("alpha.desktop"),
        QStringLiteral("beta.desktop"),
        QStringLiteral("alpha.desktop")
    }}});
    root.append(QJsonObject{{QStringLiteral("PreInstalledApps"), QStringLiteral("not-array")}});
    file.write(QJsonDocument(root).toJson());
    file.close();

    config.readConfigFile();
    QCOMPARE(config.preInstalledApps().size(), 2);
    QVERIFY(config.isPreInstalledApps(QStringLiteral("alpha.desktop")));
    QVERIFY(config.isPreInstalledApps(QStringLiteral("beta.desktop")));
}

void userConfig_test::test_invalidConfig_isIgnored()
{
    QFile file(configFilePath());
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{invalid json");
    file.close();

    UserConfig config;

    QCOMPARE(config.isFirstStartUp(), false);
    QCOMPARE(config.preInstalledApps().isEmpty(), true);
}

void userConfig_test::test_addAndRemovePreInstalledApp_updatesStateAndFile()
{
    UserConfig config;

    config.addPreInstalledApp(QStringLiteral("gamma.desktop"));
    QVERIFY(config.isPreInstalledApps(QStringLiteral("gamma.desktop")));

    config.sync();
    QVERIFY(readJsonDocument(configFilePath()).isArray());

    config.removePreInstalledApp(QStringLiteral("gamma.desktop"));
    QCOMPARE(config.isPreInstalledApps(QStringLiteral("gamma.desktop")), false);

    const QJsonDocument doc = readJsonDocument(configFilePath());
    QVERIFY(doc.isArray());
    const QJsonArray apps = doc.array().at(0).toObject().value(QStringLiteral("PreInstalledApps")).toArray();
    QCOMPARE(apps.isEmpty(), true);
}

void userConfig_test::test_writeConfigFile_returnsWhenTargetIsDirectory()
{
    QFile::remove(configFilePath());
    QVERIFY(QDir().mkpath(configFilePath()));

    UserConfig config;
    config.addPreInstalledApp(QStringLiteral("blocked.desktop"));
    config.writeConfigFile();

    QVERIFY(QFileInfo(configFilePath()).isDir());
}

QTEST_MAIN(userConfig_test)

#include "user-config-test.moc"
