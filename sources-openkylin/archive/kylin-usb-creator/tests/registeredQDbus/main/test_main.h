#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <QObject>
#include <QtTest/QtTest>

class TestMain : public QObject
{
    Q_OBJECT

public:
    TestMain();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMainFunction();
};

#endif // TEST_MAIN_H