#ifndef TEST_KYNETLOADRATETHREAD_H
#define TEST_KYNETLOADRATETHREAD_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QDebug>

#include "kynetloadratethread.h"

class TestKyNetLoadRateThread : public QObject
{
    Q_OBJECT
public:
    explicit TestKyNetLoadRateThread(QObject *parent = nullptr);
    ~TestKyNetLoadRateThread();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_getNetSpeed_beforeStart();
    void test_getNetSpeed_afterStart();
    void test_getNetSpeed_loopbackDevice();
    void test_signal_loadRateUpdate();

private:
    KyNetLoadRateThread *m_netLoadRateThread;
};

#endif // TEST_KYNETLOADRATETHREAD_H
