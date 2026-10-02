#ifndef TEST_DATABACKUPPROXY_H
#define TEST_DATABACKUPPROXY_H

#include <QObject>
#include <QtTest>

class DataBackupProxy;
struct BackupWrapper;

class TestDataBackupProxy : public QObject
{
    Q_OBJECT

public:
    TestDataBackupProxy();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testLoadExistsBackupPoint();
    void testConstructor();
    void testDestructor();
    void testCheckEnvEx_tempDir_iPosition0();
    void testCheckEnvEx_udiskNonExt4_emitsSignal();
    void testCheckEnvEx_dirNotExist_mkpathSucceeds();
    void testDoWorkEx();
    void testCancelEx_emitsCancelSignals();
    void testCheckEnvEx_emptyBackupPaths_calcSizeStillRuns();


    void testGetRsyncArgs_allScenes();
    void testDoMksqushfs();
    void testRemoveAutoBackupPoint();
    void testExtraDataPaths();
    void testExtraDataBackup();
    void testCalcSizeForExtraBackup();

private:
    void setDataBackupWrapper(BackupWrapper& w, const QString& prefixPath,
                              int iPosition, const QStringList& backupPaths);

    DataBackupProxy* m_dataBackupProxy;
};

#endif // TEST_DATABACKUPPROXY_H
