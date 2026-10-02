#ifndef TEST_MYDEFINE_H
#define TEST_MYDEFINE_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../common/mydefine.h"

class TestMyDefine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testBackupWrapperSerialization();
    void testBackupWrapperDeserialization();
    void testRegisterMetaType();
    void testGlobalVariable();
};

#endif // TEST_MYDEFINE_H