#ifndef TEST_GLOBAL_H
#define TEST_GLOBAL_H

#include <QObject>
#include <QtTest/QtTest>

class TestGlobal : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGlobalInitWithWayland();
    void testGlobalInitWithoutWayland();
    void testGlobalEnd();

};

#endif // TEST_GLOBAL_H