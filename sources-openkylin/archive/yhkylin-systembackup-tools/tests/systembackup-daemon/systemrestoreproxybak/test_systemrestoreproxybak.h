#ifndef TEST_SYSTEMRESTOREPROXYBAK_H
#define TEST_SYSTEMRESTOREPROXYBAK_H

#include <QObject>
#include <QtTest/QtTest>

class SystemRestoreProxyBak;

class TestSystemRestoreProxyBak : public QObject
{
    Q_OBJECT

public:
    TestSystemRestoreProxyBak();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();
    
    // 测试析构函数
    void testDestructor();
    
    // 测试环境检测功能
    void testCheckEnvEx();
    
    // 测试任务处理功能
    void testDoWorkEx();
    
    // 测试restoreEfi功能
    void testRestoreEfi();
    
    // 测试repairEfi功能
    void testRepairEfi();
    
    // 测试rsyncEfi功能
    void testRsyncEfi();
    
    // 测试restoreSystem功能
    void testRestoreSystem();
    
    // 测试getRsyncArgs功能
    void testGetRsyncArgs();

private:
    SystemRestoreProxyBak* m_systemRestoreProxyBak;
};

#endif // TEST_SYSTEMRESTOREPROXYBAK_H