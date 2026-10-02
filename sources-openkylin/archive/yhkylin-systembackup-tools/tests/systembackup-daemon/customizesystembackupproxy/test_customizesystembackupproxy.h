#ifndef TEST_CUSTOMIZESYSTEMBACKUPPROXY_H
#define TEST_CUSTOMIZESYSTEMBACKUPPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/customizesystembackupproxy.h"

struct BackupWrapper;

class TestCustomizeSystemBackupProxy : public QObject
{
    Q_OBJECT

public:
    TestCustomizeSystemBackupProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();

    // void testCheckEnvEx_emitsCheckResult();
    // void testCheckEnvEx_pathInjection_emitsDuErr();
    // void testCheckEnvEx_emptyPrefixDestPath_mkpath();
    void testDoWorkEx();
    // 白盒：直接测试私有 doPrepare / doMksqushfs
    void testDoPrepare_createsSnapshotAndXml();
    void testDoMksqushfs_basic();
    // void testCancelEx_emitsStartCancelAndCancelSuccess();

private:
    void setSystemBackupWrapper(BackupWrapper &w, const QString &prefixPath);

    CustomizeSystemBackupProxy *m_proxy;
};

#endif // TEST_CUSTOMIZESYSTEMBACKUPPROXY_H
