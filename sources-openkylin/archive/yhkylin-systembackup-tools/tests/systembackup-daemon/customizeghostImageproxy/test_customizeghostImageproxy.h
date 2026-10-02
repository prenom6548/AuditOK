#ifndef TEST_CUSTOMIZEGHOSTIMAGEPROXY_H
#define TEST_CUSTOMIZEGHOSTIMAGEPROXY_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/customizeghostImageproxy.h"

struct BackupWrapper;

class TestCustomizeGhostImageProxy : public QObject
{
    Q_OBJECT

public:
    TestCustomizeGhostImageProxy() : m_proxy(nullptr) {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testCheckEnvEx_emitsCheckResult();
    void testCheckEnvEx_nonExistentUuid_emitsGhostCannotFindOrMountFail();
    void testCheckEnvEx_fakeData_srcDirEmpty_emitsGhostSrcDirNotExist();
    void testCheckEnvEx_fakeData_dstImgNotExist_emitsGhostSrcDirNotExist();
    void testCheckEnvEx_fakeData_successOrCapacityOrMountFail();
    void testDoWorkEx();
    void testCancelEx_emitsStartCancelAndCancelSuccess();
    void testCancelEx_calledTwice_bothEmitSignals();

    void testDoGhostImage_afterCheckEnvSuccess_runsWithoutCrash();
    void testCheckDestDirExists_notFinished_prefixDirExists_returnsTrue();
    void testCheckDestDirExists_prefixDirEmpty_thenForce_emitsWorkResultFalse();

private:
    void setGhostBackupWrapper(BackupWrapper &w, const QString &uuid, const QString &prefixPath);
    /** 在 basePath 下写入 backuplist.xml，包含一个 BackupPoint（uuid、Comment、PrefixDestPath=basePath） */
    bool writeGhostBackupListXml(const QString &basePath, const QString &uuid, const QString &comment = QStringLiteral("test-ghost"));

    CustomizeGhostImageProxy *m_proxy;
};

#endif // TEST_CUSTOMIZEGHOSTIMAGEPROXY_H
