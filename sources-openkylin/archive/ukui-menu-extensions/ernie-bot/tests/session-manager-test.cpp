#include <QtTest>
#include <QSignalSpy>
#include <QSettings>
#include <QTemporaryDir>

#define private public
#include "../src/session-manager.h"
#undef private
#include "../src/session.h"

void setChatStubErrorForTest(const QString &error);

namespace {

void useTemporaryHome(QTemporaryDir &temporaryDir)
{
    QVERIFY(temporaryDir.isValid());
    qputenv("HOME", temporaryDir.path().toUtf8());
}

void flushEvents()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

}

class sessionManager_test : public QObject
{
    Q_OBJECT

private slots:
    void test_SessionManager();
};

void sessionManager_test::test_SessionManager()
{
    QTemporaryDir temporaryHome;
    useTemporaryHome(temporaryHome);

    SessionManager manager;
    QCOMPARE(manager.apiReady(), false);
    QCOMPARE(manager.newSession(), -1);
    QVERIFY(manager.sessionIds().isEmpty());
    QCOMPARE(manager.getSession(0), nullptr);
    QCOMPARE(manager.sessionIds(), QVariantList());

    manager.setSessionKeys(QString(), "secret");
    manager.setSessionKeys("key", QString());
    manager.setSessionKeys(QString(), QString());
    QCOMPARE(manager.apiReady(), false);

    QSignalSpy readySpy(&manager, &SessionManager::apiReadyChanged);
    QSignalSpy apiErrorSpy(&manager, &SessionManager::apiError);
    manager.checkApiReady();
    QCOMPARE(manager.apiReady(), false);

    setChatStubErrorForTest("bad credentials");
    manager.setSessionKeys("error-key", "error-secret");
    QCOMPARE(manager.apiReady(), false);
    QCOMPARE(apiErrorSpy.count(), 1);
    QCOMPARE(apiErrorSpy.takeFirst().at(0).toString(), QString("bad credentials"));
    manager.checkApiReady();
    QCOMPARE(apiErrorSpy.count(), 0);
    flushEvents();

    QSignalSpy addedSpy(&manager, &SessionManager::sessionAdded);
    manager.setSessionKeys("key", "secret");
    QCOMPARE(manager.apiReady(), true);
    QCOMPARE(readySpy.count(), 1);
    flushEvents();
    manager.checkApiReady();
    QCOMPARE(manager.apiReady(), true);
    manager.setSessionKeys("ignored", "ignored");
    QCOMPARE(manager.apiReady(), true);

    const int firstSessionId = manager.newSession();
    QCOMPARE(firstSessionId, 0);
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(manager.sessionIds(), QVariantList({0}));
    QVERIFY(manager.getSession(firstSessionId) != nullptr);
    QCOMPARE(manager.getSession(firstSessionId)->sessionIsReady(), true);

    const int secondSessionId = manager.newSession();
    QCOMPARE(secondSessionId, 1);
    QCOMPARE(manager.sessionIds(), QVariantList({0, 1}));

    QSignalSpy removedSpy(&manager, &SessionManager::sessionRemoved);
    manager.exitSessionApi();
    QCOMPARE(manager.apiReady(), false);
    QCOMPARE(removedSpy.count(), 2);
    QVERIFY(manager.sessionIds().isEmpty());
    QCOMPARE(manager.getSession(firstSessionId), nullptr);
    manager.m_sessions.insert(99, nullptr);
    manager.exitSessionApi();
    QCOMPARE(removedSpy.count(), 2);

    QTemporaryDir savedHome;
    useTemporaryHome(savedHome);
    const QString settingsPath(savedHome.filePath(".config/ukui-menu/ai-assistant/ai-assistant.conf"));
    QVERIFY(QDir().mkpath(QFileInfo(settingsPath).absolutePath()));
    QSettings settings(settingsPath, QSettings::IniFormat);
    settings.beginGroup("Api Setting");
    settings.setValue("key", "saved-key");
    settings.setValue("secret", "saved-secret");
    settings.endGroup();
    settings.sync();

    SessionManager savedManager;
    QSignalSpy savedReadySpy(&savedManager, &SessionManager::apiReadyChanged);
    savedManager.checkApiReady();
    QCOMPARE(savedManager.apiReady(), true);
    QCOMPARE(savedReadySpy.count(), 1);
    QCOMPARE(savedManager.newSession(), 0);
}

QTEST_MAIN(sessionManager_test)

#include "session-manager-test.moc"
