#ifndef TEST_MYDUSIZETOOL_H
#define TEST_MYDUSIZETOOL_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../common/mydusizetool.h"

class TestMyDuSizeTool : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testDoWithExcludePaths_data();
    void testDoWithExcludePaths();
    void testDoWithoutExcludePaths_data();
    void testDoWithoutExcludePaths();
    void testSize();
};

#endif // TEST_MYDUSIZETOOL_H