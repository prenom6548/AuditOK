#ifndef TEST_MYMOUNTPROXY_H
#define TEST_MYMOUNTPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/mymountproxy.h"

// Mock类声明
class MockMountBackupProcess : public MountBackupProcess {
public:
    MockMountBackupProcess(QObject* parent = nullptr) : MountBackupProcess(parent) {}
    
    // Mock方法
    virtual bool Do(const QString& diskUuid) {
        // 记录调用参数
        lastDiskUuid = diskUuid;
        // 根据测试需要返回不同结果
        return doResult;
    }
    
    virtual bool umountBackupPartition() {
        umountCalled = true;
        return umountResult;
    }
    
    // Mock控制变量
    bool doResult = true;
    bool umountResult = true;
    bool umountCalled = false;
    QString lastDiskUuid;
};

class TestMyMountProxy : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testMountBackupPartition_Success();
    void testMountBackupPartition_NoUuid();
    void testMountBackupPartition_DoFail();
    void testMountBackupPartition_GenerateExcludeFail();
    void testUmountBackupPartition();
    void testRemountPartitionsForCasperMode_NotCasper();
    void testRemountPartitionsForCasperMode_Success();
    void testRemountPartitionsForCasperMode_MountFail();

private:
    MockMountBackupProcess* mockProcess;
};

#endif // TEST_MYMOUNTPROXY_H