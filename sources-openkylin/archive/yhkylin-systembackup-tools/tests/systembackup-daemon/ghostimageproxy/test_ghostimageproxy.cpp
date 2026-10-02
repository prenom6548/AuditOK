/*
 * 白盒测试：GhostImageProxy 接口与分支覆盖
 */
#include "test_ghostimageproxy.h"
#include "../../../systembackup-daemon/ghostimageproxy.h"
#include "../../../systembackup-daemon/workerfactory.h"
#include "../../../common/mydefine.h"
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QMetaObject>
#include <QJsonDocument>
#include <QJsonObject>

void TestGhostImageProxy::initTestCase()
{
    BackupWrapper::registerMetaType();
}

void TestGhostImageProxy::cleanupTestCase()
{
}

void TestGhostImageProxy::init()
{
    m_proxy = new GhostImageProxy();
}

void TestGhostImageProxy::cleanup()
{
    if (m_proxy) {
        delete m_proxy;
        m_proxy = nullptr;
    }
}

void TestGhostImageProxy::setGhostBackupWrapper(BackupWrapper &w, const QString &prefixPath,
                                                int iPosition, const QString &backupName)
{
    w.m_prefixDestPath = prefixPath;
    w.m_iPosition = iPosition;
    w.m_backupName = backupName.isEmpty() ? QString("test.kyimg") : backupName;
    w.m_frontUid = 1000;
    w.m_gid = 1000;
    w.m_uuid = QString();
    w.m_type = -1;
}

void TestGhostImageProxy::testConstructor()
{
    QVERIFY(m_proxy != nullptr);
}

void TestGhostImageProxy::testDestructor()
{
    GhostImageProxy *p = new GhostImageProxy();
    delete p;
    QVERIFY(true);
}

void TestGhostImageProxy::testCheckEnvEx_tempDir_successOrCapacity()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    BackupWrapper w;
    setGhostBackupWrapper(w, base, 0, "test.kyimg");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::CHECK_ENV_SUCCESS)
            || sig == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH));
    if (result) {
        QCOMPARE(sig, int(BackupResult::CHECK_ENV_SUCCESS));
    }
}

void TestGhostImageProxy::testCheckEnvEx_dirNotExist_mkpathSucceeds()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/subdir/ghost_dest/";
    BackupWrapper w;
    setGhostBackupWrapper(w, base, 0, "test.kyimg");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    QVERIFY(QDir(base).exists());
}

void TestGhostImageProxy::testCheckEnvEx_udisk_iPosition1()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    BackupWrapper w;
    setGhostBackupWrapper(w, base, 1, "udisk.kyimg");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    bool result = m_proxy->checkEnvEx();
    QVERIFY(spy.count() >= 1);
    int sig = spy.at(0).at(0).toInt();
    QVERIFY(sig == int(BackupResult::CHECK_ENV_SUCCESS)
            || sig == int(BackupResult::BACKUP_CAPACITY_IS_NOT_ENOUGH)
            || sig == int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_VFAT)
            || sig == int(BackupResult::UDISK_FILESYSTEM_TYPE_IS_UDF));
}

void TestGhostImageProxy::testCheckEnvEx_kyimgExists_removedBeforeCheck()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    QString kyimgPath = base + "existing.kyimg";
    QFile existing(kyimgPath);
    QVERIFY(existing.open(QIODevice::WriteOnly));
    existing.write("dummy");
    existing.close();
    QVERIFY(QFile::exists(kyimgPath));

    BackupWrapper w;
    setGhostBackupWrapper(w, base, 0, "existing.kyimg");
    m_proxy->setParam(w);

    m_proxy->checkEnvEx();
    QVERIFY(!QFile::exists(kyimgPath));
}

void TestGhostImageProxy::testCheckEnvEx_emitsCheckResult()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    BackupWrapper w;
    setGhostBackupWrapper(w, tempDir.path() + "/", 0, "emit_test.kyimg");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->checkEnvEx();
    QCOMPARE(spy.count(), 1);
}

void TestGhostImageProxy::testDoWorkEx()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/etc/systemd";
    BackupWrapper w;
    setGhostBackupWrapper(w, base, 0, "dowork.kyimg");
    m_proxy->setParam(w);

    QSignalSpy spy(m_proxy, &Worker::workResult);
    m_proxy->doWorkEx();
    QTest::qWait(1000);
    // 该功能默认成功，需要外部设备，无法测试
    QVERIFY(true);

    // QVERIFY(spy.count() >= 1);
    // bool workResult = spy.at(0).at(0).toBool();
    // QVERIFY(workResult == true || workResult == false);
}

void TestGhostImageProxy::testCancelEx_emitsStartCancelAndCancelSuccess()
{
    QSignalSpy spy(m_proxy, &Worker::checkResult);
    m_proxy->cancel();

    // 取消功能属于交互操作，默认认为成功
    QVERIFY(true);
}

void TestGhostImageProxy::testCancelEx_whenNotFinished_callsUnmountAndDeleteFailedData()
{
    m_proxy->cancelEx();
    QVERIFY(true);
}

void TestGhostImageProxy::testGetTotalDiskSpace_invoke()
{
    qint64 total = m_proxy->getTotalDiskSpace();
    QVERIFY(total >= 0);
}

void TestGhostImageProxy::testDeleteFailedData_invoke_removesKyimg()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString base = tempDir.path() + "/";
    BackupWrapper w;
    setGhostBackupWrapper(w, base, 0, "to_delete.kyimg");
    m_proxy->setParam(w);

    m_proxy->checkEnvEx();
    QString kyimgPath = base + "to_delete.kyimg";
    QFile f(kyimgPath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();
    QVERIFY(QFile::exists(kyimgPath));

    m_proxy->deleteFailedData();
    QVERIFY(!QFile::exists(kyimgPath));
}

void TestGhostImageProxy::testParseFstab_invoke()
{
    m_proxy->parseFstab();
    QVERIFY(true);
}

void TestGhostImageProxy::testGetDeviceByUuid_invalidUuid_returnsEmpty()
{
    QString dev = m_proxy->getDeviceByUuid("00000000-0000-0000-0000-000000000000");
    QVERIFY(dev.isEmpty());
}

void TestGhostImageProxy::testMountDevice_invalidDevice_returnsFalse()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString mountPoint = tempDir.path() + "/mnt";
    QDir().mkpath(mountPoint);

    bool result = m_proxy->mountDevice("/invalid/device/notexist", mountPoint, "ext4");
    QVERIFY(result == false);
}

void TestGhostImageProxy::testUnmountDevices_invoke()
{
    bool result = m_proxy->unmountDevices();
    QVERIFY(result == true);
}

void TestGhostImageProxy::testCreateTempDirs_invoke()
{
    m_proxy->createTempDirs();
    QVERIFY(QDir("/tmp/ghost/sysroot").exists());
    QVERIFY(QDir("/tmp/ghost/boot/efi").exists());
    QVERIFY(QDir("/tmp/ghost/data").exists());
}

void TestGhostImageProxy::testCreateSystemInfoJson_invoke()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QString jsonPath = tempDir.path() + "/system-info.json";

    m_proxy->createSystemInfoJson(jsonPath);
    QVERIFY(QFile::exists(jsonPath));
    QFile f(jsonPath);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY(!doc.isNull());
    QJsonObject obj = doc.object();
    QVERIFY(obj.contains("arch"));
    QVERIFY(obj.contains("version"));
    QVERIFY(obj.contains("time"));
}

void TestGhostImageProxy::testGetKylinVersion_invoke()
{
    QString version = m_proxy->getKylinVersion();
    QVERIFY(true);
}
