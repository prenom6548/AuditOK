/*
 * 白盒测试：CalcWorker 公共接口（不测私有函数）
 */
#include "test_calcworker.h"
#include "../../../systembackup-daemon/calcworker.h"
#include "../../../common/mydefine.h"
#include <QtTest/QtTest>
#include <QThread>

void TestCalcWorker::initTestCase()
{
    g_userName = "testuser";
}

void TestCalcWorker::cleanupTestCase()
{
}

void TestCalcWorker::init()
{
    m_calcWorker = new CalcWorker();
}

void TestCalcWorker::cleanup()
{
    if (m_calcWorker) {
        delete m_calcWorker;
        m_calcWorker = nullptr;
    }
}

void TestCalcWorker::testConstructor()
{
    QVERIFY(m_calcWorker != nullptr);
}

void TestCalcWorker::testDestructor()
{
    CalcWorker *p = new CalcWorker();
    delete p;
    QVERIFY(true);
}

void TestCalcWorker::testCalcSizeForHome_returnsValue()
{
    qint64 size = m_calcWorker->calcSizeForHome();
    QVERIFY(size == -1 || size >= 0);
}

void TestCalcWorker::testDoWork_startsWithoutCrash()
{
    QThread thread;
    CalcWorker worker;
    worker.moveToThread(&thread);
    connect(&thread, &QThread::started, &worker, &CalcWorker::doWork);
    thread.start();
    QTest::qWait(1500);
    thread.quit();
    if (!thread.wait(500)) {
        thread.terminate();
        thread.wait(200);
    }
    QVERIFY(true);
}
