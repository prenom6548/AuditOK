#ifndef TEST_WORKERFACTORY_H
#define TEST_WORKERFACTORY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/workerfactory.h"

class TestWorkerFactory : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试 createWorker 函数的各种情况
    void testCreateWorker_data();
    void testCreateWorker();
    
    // 测试边界条件
    void testCreateWorkerWithInvalidType();
    
    // 测试各种备份类型的创建
    void testCreateWorkerWithBackupSystem();
    void testCreateWorkerWithIncBackupSystem();
    void testCreateWorkerWithAutoBackupSystem();
    void testCreateWorkerWithRestoreSystem();
    void testCreateWorkerWithDataBackup();
    void testCreateWorkerWithIncDataBackup();
    void testCreateWorkerWithRestoreData();
    void testCreateWorkerWithDeleteBackup();
    void testCreateWorkerWithGhostImage();
};

#endif // TEST_WORKERFACTORY_H