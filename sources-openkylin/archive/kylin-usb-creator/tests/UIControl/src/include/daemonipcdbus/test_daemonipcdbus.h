#ifndef TEST_DAEMONIPCDBUS_H
#define TEST_DAEMONIPCDBUS_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include "../../../../../UIControl/src/include/daemonipcdbus.h"

// Forward declarations for mock classes
class MockDBusConnectionInterface;
class MockDBusConnection;
class MockDBusReply;

class TestDaemonIpcDbus : public QObject
{
    Q_OBJECT

public:
    TestDaemonIpcDbus();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDaemonIsNotRunning();
    void testDaemonIsNotRunning_data();
    void testShowGuide();
    void testShowGuide_data();
    void testDaemonIsNotRunningWithDisconnectedBus();
    void testShowGuideWithReplyMessage();

private:
    DaemonIpcDbus* m_daemonIpcDbus;
};

#endif // TEST_DAEMONIPCDBUS_H