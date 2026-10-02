#ifndef TEST_MYTHREAD_H
#define TEST_MYTHREAD_H

#include <QObject>
#include <QtTest/QtTest>

class MyThread;

class TestMyThread : public QObject
{
    Q_OBJECT

public:
    TestMyThread() {}
    ~TestMyThread() {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testDestructor();
    void testCancelWorkSignal();

private:
    MyThread *m_thread;
};

#endif // TEST_MYTHREAD_H