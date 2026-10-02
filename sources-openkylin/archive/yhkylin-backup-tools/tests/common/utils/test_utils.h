#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../common/utils.h"

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试用例
    void testStringBySize_data();
    void testStringBySize();
    
    void testSizeByString_data();
    void testSizeByString();
    
    void testIsDirExists();
    
    void testCreateUuid();
};

#endif // TEST_UTILS_H