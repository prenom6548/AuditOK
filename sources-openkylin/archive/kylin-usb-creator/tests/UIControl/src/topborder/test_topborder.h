#ifndef TEST_TOPBORDER_H
#define TEST_TOPBORDER_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include "../../../../UIControl/src/topborder.h"

class TestTopBorder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testPaintEvent();
};

#endif // TEST_TOPBORDER_H