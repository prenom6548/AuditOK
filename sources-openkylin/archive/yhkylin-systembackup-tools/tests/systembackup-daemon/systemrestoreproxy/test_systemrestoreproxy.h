#ifndef TEST_SYSTEMRESTOREPROXY_H
#define TEST_SYSTEMRESTOREPROXY_H

#include <QObject>
#include <QtTest/QtTest>

class SystemRestoreProxy;

class TestSystemRestoreProxy : public QObject
{
    Q_OBJECT

public:
    TestSystemRestoreProxy();

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

private:
    SystemRestoreProxy* m_systemRestoreProxy;
};

#endif // TEST_SYSTEMRESTOREPROXY_H