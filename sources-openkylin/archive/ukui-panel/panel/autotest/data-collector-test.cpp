#include "data-collector-test.h"

#include <QScopedPointer>
#include <QSignalSpy>
#include <QTest>

#define private public
#define protected public
#include "../src/data-collector.cpp"
#undef protected
#undef private

void DataCollectorTest::initTestCase()
{
    QVERIFY(DataCollectorPrivate::self() != nullptr);
}

void DataCollectorTest::cleanupTestCase()
{
    if (g_dataCollectorPrivate) {
        g_dataCollectorPrivate->quit();
        g_dataCollectorPrivate->wait();
        delete g_dataCollectorPrivate;
        g_dataCollectorPrivate = nullptr;
    }
}

void DataCollectorTest::testPositionToStr_data()
{
    QTest::addColumn<int>("position");
    QTest::addColumn<QString>("expected");

    QTest::newRow("bottom") << 0 << QStringLiteral("Bottom");
    QTest::newRow("top") << 1 << QStringLiteral("Top");
    QTest::newRow("left") << 2 << QStringLiteral("Left");
    QTest::newRow("right") << 3 << QStringLiteral("Right");
    QTest::newRow("invalid") << 99 << QString();
}

void DataCollectorTest::testPositionToStr()
{
    QFETCH(int, position);
    QFETCH(QString, expected);

    QCOMPARE(DataCollectWrapper::positionToStr(position), expected);
}

void DataCollectorTest::testSingletonAndThreadSetup()
{
    auto *collectorPrivate = DataCollectorPrivate::self();
    auto *collectorTimer = collectorPrivate->m_timer;
    auto *collectorWrapper = collectorPrivate->m_wrapper;

    QCOMPARE(DataCollectorPrivate::self(), collectorPrivate);
    QVERIFY(collectorPrivate != nullptr);
    QVERIFY(collectorTimer != nullptr);
    QVERIFY(collectorWrapper != nullptr);
    QVERIFY(collectorTimer->isSingleShot());
    QCOMPARE(collectorTimer->interval(), 1000);
    QCOMPARE(collectorWrapper->thread(), collectorPrivate);

    collectorTimer->setInterval(20);
    collectorPrivate->startRunning();

    QVERIFY(collectorPrivate->isRunning());
    QVERIFY(collectorTimer->isActive());
    QTRY_VERIFY(!collectorTimer->isActive());
}

void DataCollectorTest::testStartRunningWhileAlreadyRunning()
{
    auto *collectorPrivate = DataCollectorPrivate::self();
    auto *collectorTimer = collectorPrivate->m_timer;

    QVERIFY(collectorPrivate != nullptr);
    QVERIFY(collectorTimer != nullptr);

    collectorTimer->setInterval(80);
    collectorPrivate->startRunning();
    QVERIFY(collectorPrivate->isRunning());
    QVERIFY(collectorTimer->isActive());

    QTest::qWait(20);
    QVERIFY(collectorPrivate->isRunning());
    QVERIFY(collectorTimer->isActive());

    collectorPrivate->startRunning();
    QVERIFY(collectorTimer->isActive());
    QVERIFY(collectorTimer->remainingTime() > 0);
    QTRY_VERIFY(!collectorTimer->isActive());
}

void DataCollectorTest::testMetaObjectCoverage()
{
    auto *collectorPrivate = DataCollectorPrivate::self();
    auto *collectorTimer = collectorPrivate->m_timer;
    auto *collectorWrapper = collectorPrivate->m_wrapper;

    QVERIFY(collectorPrivate != nullptr);
    QVERIFY(collectorTimer != nullptr);
    QVERIFY(collectorWrapper != nullptr);

    QCOMPARE(QString::fromLatin1(collectorPrivate->metaObject()->className()), QStringLiteral("DataCollectorPrivate"));
    QVERIFY(collectorPrivate->qt_metacast("DataCollectorPrivate") != nullptr);
    QVERIFY(collectorPrivate->qt_metacast("QThread") != nullptr);
    QVERIFY(collectorPrivate->qt_metacast("NotARealCollector") == nullptr);

    QCOMPARE(QString::fromLatin1(collectorWrapper->metaObject()->className()), QStringLiteral("DataCollectWrapper"));
    QVERIFY(collectorWrapper->qt_metacast("DataCollectWrapper") != nullptr);
    QVERIFY(collectorWrapper->qt_metacast("QObject") != nullptr);
    QVERIFY(collectorWrapper->qt_metacast("NotARealWrapper") == nullptr);

    QSignalSpy executedSpy(collectorWrapper, SIGNAL(functiomExecuted()));
    QVERIFY(executedSpy.isValid());

    collectorTimer->setInterval(50);
    collectorPrivate->startRunning();

    QVERIFY(QMetaObject::invokeMethod(collectorPrivate,
                                      "setPanelPositionEvent",
                                      Qt::DirectConnection,
                                      Q_ARG(int, 2)));
    QVERIFY(QMetaObject::invokeMethod(collectorPrivate,
                                      "setPanelSizeEvent",
                                      Qt::DirectConnection,
                                      Q_ARG(int, 64)));
    QTRY_COMPARE(executedSpy.count(), 2);

    QSignalSpy directSpy(collectorWrapper, SIGNAL(functiomExecuted()));
    QVERIFY(directSpy.isValid());
    QVERIFY(QMetaObject::invokeMethod(collectorWrapper, "functiomExecuted", Qt::DirectConnection));
    QCOMPARE(directSpy.count(), 1);
}

void DataCollectorTest::testSetPanelPositionEvent()
{
    auto *collectorPrivate = DataCollectorPrivate::self();
    auto *collectorTimer = collectorPrivate->m_timer;
    auto *collectorWrapper = collectorPrivate->m_wrapper;
    collectorTimer->setInterval(50);

    QSignalSpy executedSpy(collectorWrapper, SIGNAL(functiomExecuted()));
    QVERIFY(executedSpy.isValid());

    DataCollector::setPanelPositionEvent(1);
    QTRY_COMPARE(executedSpy.count(), 1);

    DataCollector::setPanelPositionEvent(99);
    QTRY_COMPARE(executedSpy.count(), 2);
}

void DataCollectorTest::testSetPanelSizeEvent()
{
    auto *collectorPrivate = DataCollectorPrivate::self();
    auto *collectorTimer = collectorPrivate->m_timer;
    auto *collectorWrapper = collectorPrivate->m_wrapper;
    collectorTimer->setInterval(50);

    QSignalSpy executedSpy(collectorWrapper, SIGNAL(functiomExecuted()));
    QVERIFY(executedSpy.isValid());

    DataCollector::setPanelSizeEvent(40);
    QTRY_COMPARE(executedSpy.count(), 1);

    DataCollector::setPanelSizeEvent(72);
    QTRY_COMPARE(executedSpy.count(), 2);
}

void DataCollectorTest::testDirectWrapperMethodsEmitSignals()
{
    QObject wrapper;
    QScopedPointer<DataCollectWrapper> directWrapper(new DataCollectWrapper(&wrapper));
    QSignalSpy executedSpy(directWrapper.data(), SIGNAL(functiomExecuted()));
    QVERIFY(executedSpy.isValid());

    directWrapper->setPanelPositionEvent(0);
    directWrapper->setPanelPositionEvent(3);
    directWrapper->setPanelPositionEvent(-1);
    directWrapper->setPanelSizeEvent(0);
    directWrapper->setPanelSizeEvent(88);

    QCOMPARE(executedSpy.count(), 5);
}

void DataCollectorTest::testStandaloneDestructorHandlesNullWrapper()
{
    auto *originalCollector = g_dataCollectorPrivate;
    auto *collector = new DataCollectorPrivate();
    collector->m_wrapper = nullptr;
    delete collector;
    g_dataCollectorPrivate = originalCollector;
    QVERIFY(true);
}

void DataCollectorTest::testDeleteThroughBasePointer()
{
    auto *originalCollector = g_dataCollectorPrivate;
    QThread *thread = new DataCollectorPrivate();
    delete thread;
    g_dataCollectorPrivate = originalCollector;
    QVERIFY(true);
}

void DataCollectorTest::testZZDestroySingletonClearsInstance()
{
    QVERIFY(g_dataCollectorPrivate != nullptr);
    delete g_dataCollectorPrivate;
    QCOMPARE(g_dataCollectorPrivate, nullptr);
}

QTEST_MAIN(DataCollectorTest)
