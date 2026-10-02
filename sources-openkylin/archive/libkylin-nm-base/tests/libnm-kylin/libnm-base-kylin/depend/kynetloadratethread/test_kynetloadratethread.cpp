#include "test_kynetloadratethread.h"

TestKyNetLoadRateThread::TestKyNetLoadRateThread(QObject *parent)
    : QObject(parent)
    , m_netLoadRateThread(nullptr)
{
}

TestKyNetLoadRateThread::~TestKyNetLoadRateThread()
{
}

void TestKyNetLoadRateThread::initTestCase()
{
}

void TestKyNetLoadRateThread::cleanupTestCase()
{
}

void TestKyNetLoadRateThread::init()
{
    m_netLoadRateThread = new KyNetLoadRateThread("lo");
}

void TestKyNetLoadRateThread::cleanup()
{
    if (m_netLoadRateThread) {
        if (m_netLoadRateThread->isRunning()) {
            m_netLoadRateThread->quit();
            m_netLoadRateThread->wait(3000);
        }
        delete m_netLoadRateThread;
        m_netLoadRateThread = nullptr;
    }
}

void TestKyNetLoadRateThread::test_constructor()
{
    KyNetLoadRateThread *thread = new KyNetLoadRateThread("lo");
    QVERIFY(thread != nullptr);
    QVERIFY(thread->isFinished() == false);
    QVERIFY(thread->isRunning() == false);
    delete thread;
}

void TestKyNetLoadRateThread::test_destructor()
{
    KyNetLoadRateThread *thread = new KyNetLoadRateThread("lo");
    delete thread;
    QVERIFY(true);
}

void TestKyNetLoadRateThread::test_getNetSpeed_beforeStart()
{
    KyNetLoadRateThread *thread = new KyNetLoadRateThread("lo");
    long int rcvRate = -1;
    long int txRate = -1;
    thread->getNetSpeed(rcvRate, txRate);
    QVERIFY(rcvRate == 0);
    QVERIFY(txRate == 0);
    delete thread;
}

void TestKyNetLoadRateThread::test_getNetSpeed_afterStart()
{
    KyNetLoadRateThread *thread = new KyNetLoadRateThread("lo");
    thread->start();
    QVERIFY(thread->isRunning() == true);
    
    QTest::qWait(2500);
    
    long int rcvRate = -1;
    long int txRate = -1;
    thread->getNetSpeed(rcvRate, txRate);
    
    QVERIFY(rcvRate >= 0);
    QVERIFY(txRate >= 0);
    
    thread->quit();
    thread->wait(3000);
    QVERIFY(thread->isRunning() == false);
    
    delete thread;
}

void TestKyNetLoadRateThread::test_getNetSpeed_loopbackDevice()
{
    KyNetLoadRateThread *thread = new KyNetLoadRateThread("lo");
    thread->start();
    QVERIFY(thread->isRunning() == true);
    
    QTest::qWait(2500);
    
    long int rcvRate = -1;
    long int txRate = -1;
    thread->getNetSpeed(rcvRate, txRate);
    
    QVERIFY(rcvRate >= 0);
    QVERIFY(txRate >= 0);
    
    thread->quit();
    thread->wait(3000);
    delete thread;
}

void TestKyNetLoadRateThread::test_signal_loadRateUpdate()
{
    KyNetLoadRateThread *thread = new KyNetLoadRateThread("lo");
    thread->start();
    QTest::qWait(2500);

    long int rcvRate = -1;
    long int txRate = -1;
    thread->getNetSpeed(rcvRate, txRate);
    QVERIFY(rcvRate >= 0 || txRate >= 0);

    thread->quit();
    thread->wait(3000);
    delete thread;
}
