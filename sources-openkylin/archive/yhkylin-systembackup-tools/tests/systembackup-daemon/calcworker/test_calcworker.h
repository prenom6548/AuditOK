#ifndef TEST_CALCWORKER_H
#define TEST_CALCWORKER_H

#include <QObject>
#include <QtTest/QtTest>

class CalcWorker;

class TestCalcWorker : public QObject
{
    Q_OBJECT

public:
    TestCalcWorker() : m_calcWorker(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testCalcSizeForHome_returnsValue();
    void testDoWork_startsWithoutCrash();

private:
    CalcWorker *m_calcWorker;
};

#endif // TEST_CALCWORKER_H
