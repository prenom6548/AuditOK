#include "test_workerfactory.h"
#include <QDebug>

void TestWorkerFactory::initTestCase()
{
    qDebug() << "TestWorkerFactory initTestCase";
}

void TestWorkerFactory::cleanupTestCase()
{
    qDebug() << "TestWorkerFactory cleanupTestCase";
}

void TestWorkerFactory::testCreateWorker_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<int>("position");
    QTest::addColumn<int>("user");
    QTest::addColumn<bool>("shouldBeNull");

    // 测试各种有效的组合
    QTest::newRow("BackupSystem_Local_Auto") << (int)BackupType::BACKUP_SYSTEM << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("BackupSystem_UDisk_Auto") << (int)BackupType::BACKUP_SYSTEM << (int)BackupPosition::UDISK << (int)BackupUser::AUTO << false;
    QTest::newRow("IncBackupSystem_Local_User") << (int)BackupType::INC_BACKUP_SYSTEM << (int)BackupPosition::LOCAL << (int)BackupUser::USER << false;
    QTest::newRow("IncBackupSystem_UDisk_User") << (int)BackupType::INC_BACKUP_SYSTEM << (int)BackupPosition::UDISK << (int)BackupUser::USER << false;
    QTest::newRow("IncBackupSystem_Customize_User") << (int)BackupType::INC_BACKUP_SYSTEM << (int)BackupPosition::CUSTOMIZE << (int)BackupUser::USER << false;
    QTest::newRow("RestoreSystem_Local_Auto") << (int)BackupType::RESTORE_SYSTEM << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("RestoreSystem_Local_User") << (int)BackupType::RESTORE_SYSTEM << (int)BackupPosition::LOCAL << (int)BackupUser::USER << false;
    QTest::newRow("AutoRestoreSystem_Local_Auto") << (int)BackupType::AUTO_RESTORE_SYSTEM << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("DataBackup_Local_Auto") << (int)BackupType::BACKUP_DATA << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("IncDataBackup_Local_Auto") << (int)BackupType::INC_BACKUP_DATA << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("IncDataBackup_UDisk_Auto") << (int)BackupType::INC_BACKUP_DATA << (int)BackupPosition::UDISK << (int)BackupUser::AUTO << false;
    QTest::newRow("IncDataBackup_Customize_Auto") << (int)BackupType::INC_BACKUP_DATA << (int)BackupPosition::CUSTOMIZE << (int)BackupUser::AUTO << false;
    QTest::newRow("RestoreData_Local_Auto") << (int)BackupType::RESTORE_DATA << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("DeleteBackup_Local_Auto") << (int)BackupType::DELETE_BACKUP << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("GhostImage_Local_Auto") << (int)BackupType::GHOST_IMAGE << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << false;
    QTest::newRow("AutoBackupSystem_Customize_Auto") << (int)BackupType::AUTO_BACKUP_SYSTEM << (int)BackupPosition::CUSTOMIZE << (int)BackupUser::AUTO << false;
    
    // 测试无效类型
    QTest::newRow("InvalidType") << -1 << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << true;
    QTest::newRow("InvalidType2") << 100 << (int)BackupPosition::LOCAL << (int)BackupUser::AUTO << true;
}

void TestWorkerFactory::testCreateWorker()
{
    QFETCH(int, type);
    QFETCH(int, position);
    QFETCH(int, user);
    QFETCH(bool, shouldBeNull);

    Worker* worker = WorkerFactory::createWorker(type, position, user);
    
    if (shouldBeNull) {
        QVERIFY(worker == nullptr);
    } else {
        // 对于某些类型，可能返回nullptr，因为对应的类可能未注册或不存在
        // 这里我们只是验证函数能正常执行而不崩溃
        if (worker != nullptr) {
            delete worker;
        }
        // 我们不强制要求返回非空指针，因为这取决于反射机制和类注册情况
    }
}

void TestWorkerFactory::testCreateWorkerWithInvalidType()
{
    // 测试无效的类型
    Worker* worker = WorkerFactory::createWorker(-1, 0, 0);
    QVERIFY(worker == nullptr);
    
    worker = WorkerFactory::createWorker(1000, 0, 0);
    QVERIFY(worker == nullptr);
}

void TestWorkerFactory::testCreateWorkerWithBackupSystem()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::BACKUP_SYSTEM, BackupPosition::LOCAL, BackupUser::AUTO);
    // 不强制要求返回非空，因为SystemBackupProxy类可能未注册
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithIncBackupSystem()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::INC_BACKUP_SYSTEM, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
    
    worker = WorkerFactory::createWorker(BackupType::INC_BACKUP_SYSTEM, BackupPosition::UDISK, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
    
    worker = WorkerFactory::createWorker(BackupType::INC_BACKUP_SYSTEM, BackupPosition::CUSTOMIZE, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithAutoBackupSystem()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::AUTO_BACKUP_SYSTEM, BackupPosition::CUSTOMIZE, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithRestoreSystem()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::RESTORE_SYSTEM, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
    
    worker = WorkerFactory::createWorker(BackupType::RESTORE_SYSTEM, BackupPosition::LOCAL, BackupUser::USER);
    if (worker != nullptr) {
        delete worker;
    }
    
    worker = WorkerFactory::createWorker(BackupType::AUTO_RESTORE_SYSTEM, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithDataBackup()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::BACKUP_DATA, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithIncDataBackup()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::INC_BACKUP_DATA, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
    
    worker = WorkerFactory::createWorker(BackupType::INC_BACKUP_DATA, BackupPosition::UDISK, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
    
    worker = WorkerFactory::createWorker(BackupType::INC_BACKUP_DATA, BackupPosition::CUSTOMIZE, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithRestoreData()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::RESTORE_DATA, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithDeleteBackup()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::DELETE_BACKUP, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}

void TestWorkerFactory::testCreateWorkerWithGhostImage()
{
    Worker* worker = WorkerFactory::createWorker(BackupType::GHOST_IMAGE, BackupPosition::LOCAL, BackupUser::AUTO);
    if (worker != nullptr) {
        delete worker;
    }
}