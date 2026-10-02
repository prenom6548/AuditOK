#ifndef TEST_SYSTEMRESTOREWORKER_H
#define TEST_SYSTEMRESTOREWORKER_H

#include <QObject>
#include <QtTest/QtTest>

class SystemRestoreWorker;

class TestSystemRestoreWorker : public QObject
{
    Q_OBJECT

public:
    TestSystemRestoreWorker();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数
    void testConstructor();
    
    // 测试plymountMode函数
    void testPlymountMode();
    
    // 测试plymountMessage函数
    void testPlymountMessage();
    
    // 测试rsyncFinished函数
    void testRsyncFinished();
    
    // 测试rsyncBackupSelf函数
    void testRsyncBackupSelf();
    
    // 测试rsyncAgain函数
    void testRsyncAgain();
    
    // 测试restartSystemAfterOneMinute函数
    void testRestartSystemAfterOneMinute();

private:
    SystemRestoreWorker* m_systemRestoreWorker;
};

#endif // TEST_SYSTEMRESTOREWORKER_H