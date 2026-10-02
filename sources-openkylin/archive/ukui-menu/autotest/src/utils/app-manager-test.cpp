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
#include <QCoreApplication>
#include <QTimer>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>

#define private public
#include "../../../src/utils/app-manager.h"
#undef private

#include "../../../src/libappdata/app-database-interface.h"
#include "../../../src/libappdata/basic-app-filter-model.h"

using namespace UkuiMenu;

QString g_fixedFavoriteAppId;
int g_fixedFavoriteIndex = -1;
QString g_updatedLaunchedAppId;
bool g_updatedLaunchedState = false;
QString g_lastDetachedProgram;
QStringList g_lastDetachedArguments;
QString g_lastDetachedWorkingDirectory;
int g_startDetachedCalls = 0;

class TestFakeProcess
{
public:
    static bool startDetached(const QString &program, const QStringList &arguments)
    {
        g_lastDetachedProgram = program;
        g_lastDetachedArguments = arguments;
        g_lastDetachedWorkingDirectory.clear();
        ++g_startDetachedCalls;
        return true;
    }

    static bool startDetached(const QString &program, const QStringList &arguments, const QString &workingDirectory)
    {
        g_lastDetachedProgram = program;
        g_lastDetachedArguments = arguments;
        g_lastDetachedWorkingDirectory = workingDirectory;
        ++g_startDetachedCalls;
        return true;
    }
};

class TestFakePendingCall
{
};

class TestFakeDBusError
{
public:
    QString message() const
    {
        return QStringLiteral("fake-dbus-error");
    }
};

class TestFakePendingCallWatcher : public QObject
{
    Q_OBJECT
public:
    explicit TestFakePendingCallWatcher(const TestFakePendingCall &, QObject *parent = nullptr)
        : QObject(parent)
    {
        QTimer::singleShot(0, this, [this] {
            Q_EMIT finished(this);
        });
    }

    bool isError() const
    {
        return true;
    }

Q_SIGNALS:
    void finished(TestFakePendingCallWatcher *self);
};

template<typename... Ts>
class TestFakePendingReply
{
public:
    TestFakePendingReply(const TestFakePendingCallWatcher &watcher)
    {
        Q_UNUSED(watcher);
    }

    TestFakePendingReply &operator=(const TestFakePendingCallWatcher &watcher)
    {
        Q_UNUSED(watcher);
        return *this;
    }

    TestFakeDBusError error() const
    {
        return {};
    }
};

class TestFakeDBusConnection
{
public:
    static TestFakeDBusConnection sessionBus()
    {
        return {};
    }

    TestFakePendingCall asyncCall(const QDBusMessage &message) const
    {
        Q_UNUSED(message);
        return {};
    }
};

namespace {

QString createExecutableScript(const QString &dirPath, const QString &fileName)
{
    const QString scriptPath = dirPath + QLatin1Char('/') + fileName;
    QFile file(scriptPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write("#!/bin/sh\n");
        file.write("exit 0\n");
        file.close();
        QFile::setPermissions(scriptPath, QFile::permissions(scriptPath)
                                          | QFileDevice::ExeOwner
                                          | QFileDevice::ExeGroup
                                          | QFileDevice::ExeOther);
    }
    return scriptPath;
}

QString createDesktopFile(const QString &dirPath, const QString &execLine)
{
    const QString desktopPath = dirPath + QLatin1String("/sample.desktop");
    QFile file(desktopPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write("[Desktop Entry]\n");
        file.write(QString("Exec=%1\n").arg(execLine).toUtf8());
        file.close();
    }
    return desktopPath;
}

}

#define QDBusConnection TestFakeDBusConnection
#define QDBusPendingCall TestFakePendingCall
#define QDBusPendingCallWatcher TestFakePendingCallWatcher
#define QDBusPendingReply TestFakePendingReply
#define QProcess TestFakeProcess
#define private public
#include "../../../src/utils/app-manager.cpp"
#undef private
#undef QProcess
#undef QDBusPendingReply
#undef QDBusPendingCallWatcher
#undef QDBusPendingCall
#undef QDBusConnection

namespace UkuiMenu {

AppDatabaseInterface::AppDatabaseInterface(QObject *parent)
    : QObject(parent)
{
}

DataEntityVector AppDatabaseInterface::apps() const
{
    return {};
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    Q_UNUSED(appid);
    Q_UNUSED(index);
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    g_fixedFavoriteAppId = appid;
    g_fixedFavoriteIndex = index;
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    g_updatedLaunchedAppId = appid;
    g_updatedLaunchedState = state;
}

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel instance;
    return &instance;
}

BasicAppFilterModel::BasicAppFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const
{
    static AppDatabaseInterface databaseInterface;
    return &databaseInterface;
}

DataEntity BasicAppFilterModel::appOfIndex(int row) const
{
    Q_UNUSED(row);
    return {};
}

int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    Q_UNUSED(appid);
    return -1;
}

bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

QVariant BasicAppFilterModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);
    return {};
}

bool BasicAppFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

bool BasicAppFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
}

}

class appManager_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_parseDesktopFile_filtersDesktopPlaceholders();
    void test_parseDesktopFile_handlesEmptyExec();
    void test_parseDesktopFile_handlesMissingDesktopFile();
    void test_launchApp_ignoresEmptyId();
    void test_launchApp_emitsHideAndUpdatesLaunchState();
    void test_launchApp_fallsBackToDesktopExecWhenDbusFails();
    void test_launchBinaryApp_ignoresEmptyBinary();
    void test_launchBinaryApp_fallsBackToDetachedProcessWhenDbusFails();
    void test_launchAppWithArguments_ignoresEmptyId();
    void test_launchAppWithArguments_emitsHideForValidId();
    void test_launchAppWithArguments_fallsBackToDetachedProcessWhenDbusFails();
    void test_changeFavoriteState_updatesDatabase();
};

void appManager_test::init()
{
    g_fixedFavoriteAppId.clear();
    g_fixedFavoriteIndex = -1;
    g_updatedLaunchedAppId.clear();
    g_updatedLaunchedState = false;
    g_lastDetachedProgram.clear();
    g_lastDetachedArguments.clear();
    g_lastDetachedWorkingDirectory.clear();
    g_startDetachedCalls = 0;
}

void appManager_test::test_parseDesktopFile_filtersDesktopPlaceholders()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString execPath = createExecutableScript(dir.path(), QStringLiteral("launcher.sh"));
    const QString desktopPath = createDesktopFile(dir.path(),
                                                  QStringLiteral("%1 --flag %u --name demo %F").arg(execPath));

    const QStringList parsed = AppManager::parseDesktopFile(desktopPath);

    QCOMPARE(parsed.size(), 4);
    QCOMPARE(parsed.at(0), execPath);
    QCOMPARE(parsed.at(1), QStringLiteral("--flag"));
    QCOMPARE(parsed.at(2), QStringLiteral("--name"));
    QCOMPARE(parsed.at(3), QStringLiteral("demo"));
}

void appManager_test::test_parseDesktopFile_handlesEmptyExec()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString desktopPath = dir.path() + QLatin1String("/empty.desktop");
    QFile file(desktopPath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("[Desktop Entry]\n");
    file.write("Name=OnlyName\n");
    file.close();

    const QStringList parsed = AppManager::parseDesktopFile(desktopPath);

    QCOMPARE(parsed.size(), 1);
    QCOMPARE(parsed.first(), QString());
}

void appManager_test::test_parseDesktopFile_handlesMissingDesktopFile()
{
    const QStringList parsed = AppManager::parseDesktopFile(QStringLiteral("/tmp/not-exist.desktop"));

    QCOMPARE(parsed.size(), 1);
    QCOMPARE(parsed.first(), QString());
}

void appManager_test::test_launchApp_ignoresEmptyId()
{
    QSignalSpy spy(AppManager::instance(), &AppManager::request);

    AppManager::instance()->launchApp(QString());

    QCOMPARE(spy.count(), 0);
    QVERIFY(g_updatedLaunchedAppId.isEmpty());
}

void appManager_test::test_launchApp_emitsHideAndUpdatesLaunchState()
{
    QSignalSpy spy(AppManager::instance(), &AppManager::request);

    AppManager::instance()->launchApp(QStringLiteral("/tmp/test.desktop"));

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(g_updatedLaunchedAppId, QStringLiteral("/tmp/test.desktop"));
    QCOMPARE(g_updatedLaunchedState, true);
}

void appManager_test::test_launchApp_fallsBackToDesktopExecWhenDbusFails()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString execPath = createExecutableScript(dir.path(), QStringLiteral("fallback-launcher.sh"));
    const QString desktopPath = createDesktopFile(dir.path(),
                                                  QStringLiteral("%1 --safe %u").arg(execPath));

    AppManager::instance()->launchApp(desktopPath);

    QTRY_VERIFY(g_startDetachedCalls >= 1);
    QCOMPARE(g_lastDetachedProgram, execPath);
    QCOMPARE(g_lastDetachedArguments, QStringList({ QStringLiteral("--safe") }));
}

void appManager_test::test_launchBinaryApp_ignoresEmptyBinary()
{
    AppManager::instance()->launchBinaryApp(QString(), QStringLiteral("--flag"));

    QVERIFY(g_updatedLaunchedAppId.isEmpty());
}

void appManager_test::test_launchBinaryApp_fallsBackToDetachedProcessWhenDbusFails()
{
    AppManager::instance()->launchBinaryApp(QStringLiteral("/usr/bin/demo-binary"), QStringLiteral("--flag"));

    QTRY_VERIFY(g_startDetachedCalls >= 1);
    QCOMPARE(g_lastDetachedProgram, QStringLiteral("/usr/bin/demo-binary"));
    QCOMPARE(g_lastDetachedArguments, QStringList({ QStringLiteral("--flag") }));
    QCOMPARE(g_lastDetachedWorkingDirectory, QString());
}

void appManager_test::test_launchAppWithArguments_ignoresEmptyId()
{
    QSignalSpy spy(AppManager::instance(), &AppManager::request);

    AppManager::instance()->launchAppWithArguments(QString(), { QStringLiteral("--verbose") }, QStringLiteral("demo"));

    QCOMPARE(spy.count(), 0);
    QVERIFY(g_updatedLaunchedAppId.isEmpty());
}

void appManager_test::test_launchAppWithArguments_emitsHideForValidId()
{
    QSignalSpy spy(AppManager::instance(), &AppManager::request);

    AppManager::instance()->launchAppWithArguments(QStringLiteral("/tmp/demo.desktop"),
                                                   { QStringLiteral("--verbose"), QStringLiteral("--safe") },
                                                   QStringLiteral("demo-app"));

    QTRY_COMPARE(spy.count(), 1);
}

void appManager_test::test_launchAppWithArguments_fallsBackToDetachedProcessWhenDbusFails()
{
    AppManager::instance()->launchAppWithArguments(QStringLiteral("/tmp/demo.desktop"),
                                                   { QStringLiteral("--verbose"), QStringLiteral("--safe") },
                                                   QStringLiteral("demo-app"));

    QTRY_VERIFY(g_startDetachedCalls >= 1);
    QCOMPARE(g_lastDetachedProgram, QStringLiteral("demo-app"));
    QCOMPARE(g_lastDetachedArguments, QStringList({ QStringLiteral("--verbose"), QStringLiteral("--safe") }));
}

void appManager_test::test_changeFavoriteState_updatesDatabase()
{
    AppManager::instance()->changeFavoriteState(QStringLiteral("org.demo.app"), true);
    QCOMPARE(g_fixedFavoriteAppId, QStringLiteral("org.demo.app"));
    QCOMPARE(g_fixedFavoriteIndex, 1);

    AppManager::instance()->changeFavoriteState(QStringLiteral("org.demo.app"), false);
    QCOMPARE(g_fixedFavoriteIndex, 0);
}

QTEST_MAIN(appManager_test)

#include "app-manager-test.moc"
