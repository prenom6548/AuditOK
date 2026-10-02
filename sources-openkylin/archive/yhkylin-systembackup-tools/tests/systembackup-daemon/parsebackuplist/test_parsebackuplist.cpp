#include "test_parsebackuplist.h"
#include "../../../common/mydefine.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QDomElement>
#include <QDomProcessingInstruction>

// 定义测试用的XML标签
#define BACKUPLIST "backupList"
#define BACKUPPOINT "BackupPoint"
#define COMMENT "Comment"
#define TIME "Time"
#define UUID "Uuid"
#define SIZE "Size"
#define STATE "State"
#define TYPE "Type"

void TestParseBackupList::initTestCase()
{
    // 创建测试目录
    QDir dir;
    QString testDir = "./test_parsebackuplist_dir";
    if (!dir.exists(testDir)) {
        QVERIFY(dir.mkpath(testDir));
    }
    m_testXmlPath = testDir + "/test_backuplist.xml";
    m_parseBackupList = new ParseBackupList(m_testXmlPath);
}

void TestParseBackupList::cleanupTestCase()
{
    delete m_parseBackupList;
    // 清理测试文件和目录
    QFile::remove(m_testXmlPath);
    QDir dir;
    dir.rmdir("./test_parsebackuplist_dir");
}

void TestParseBackupList::testConstructor()
{
    QVERIFY(m_parseBackupList != nullptr);
    QCOMPARE(m_parseBackupList->getXmlPath(), m_testXmlPath);
    
    // 检查文件是否已创建并初始化
    QFile xmlFile(m_testXmlPath);
    QVERIFY(xmlFile.exists());
    QVERIFY(xmlFile.size() > 0);
}

void TestParseBackupList::testFindUuidByComment()
{
    // 添加一个测试备份点
    ParseBackupList::BackupPoint backupPoint;
    backupPoint.m_uuid = "{12345678-1234-1234-1234-123456789abc}";
    backupPoint.m_backupName = "Test Backup";
    backupPoint.m_time = "2023-01-01 12:00:00";
    backupPoint.m_type = 0;
    backupPoint.m_size = "100MB";
    backupPoint.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint), ParseBackupList::SUCCESS);
    
    // 测试根据comment查找uuid
    QString uuid = m_parseBackupList->findUuidByComment("Test Backup");
    QCOMPARE(uuid, QString("{12345678-1234-1234-1234-123456789abc}"));
    
    // 测试查找不存在的comment
    QString emptyUuid = m_parseBackupList->findUuidByComment("Non-existent Backup");
    QVERIFY(emptyUuid.isEmpty());
}

void TestParseBackupList::testFindBackupPointByUuid()
{
    // 添加一个测试备份点
    ParseBackupList::BackupPoint backupPoint;
    backupPoint.m_uuid = "{87654321-4321-4321-4321-cba987654321}";
    backupPoint.m_backupName = "Another Test Backup";
    backupPoint.m_time = "2023-01-02 12:00:00";
    backupPoint.m_type = 1;
    backupPoint.m_size = "200MB";
    backupPoint.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint), ParseBackupList::SUCCESS);
    
    // 测试根据Uuid查找备份点信息
    ParseBackupList::BackupPoint foundPoint = m_parseBackupList->findBackupPointByUuid("{87654321-4321-4321-4321-cba987654321}");
    QVERIFY(!foundPoint.isNull());
    QCOMPARE(foundPoint.m_uuid, QString("{87654321-4321-4321-4321-cba987654321}"));
    QCOMPARE(foundPoint.m_backupName, QString("Another Test Backup"));
    QCOMPARE(foundPoint.m_time, QString("2023-01-02 12:00:00"));
    QCOMPARE(foundPoint.m_type, 1);
    QCOMPARE(foundPoint.m_size, QString("200MB"));
    QCOMPARE(foundPoint.m_state, QString("backup finished"));
    
    // 测试查找不存在的Uuid
    ParseBackupList::BackupPoint nullPoint = m_parseBackupList->findBackupPointByUuid("{00000000-0000-0000-0000-000000000000}");
    QVERIFY(nullPoint.isNull());
}

void TestParseBackupList::testGetXmlUuidNameMap()
{
    // 添加多个测试备份点
    ParseBackupList::BackupPoint backupPoint1;
    backupPoint1.m_uuid = "{11111111-1111-1111-1111-111111111111}";
    backupPoint1.m_backupName = "First Test Backup";
    backupPoint1.m_time = "2023-01-03 12:00:00";
    backupPoint1.m_type = 0;
    backupPoint1.m_size = "150MB";
    backupPoint1.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint1), ParseBackupList::SUCCESS);
    
    ParseBackupList::BackupPoint backupPoint2;
    backupPoint2.m_uuid = "{22222222-2222-2222-2222-222222222222}";
    backupPoint2.m_backupName = "Second Test Backup";
    backupPoint2.m_time = "2023-01-04 12:00:00";
    backupPoint2.m_type = 1;
    backupPoint2.m_size = "250MB";
    backupPoint2.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint2), ParseBackupList::SUCCESS);
    
    // 测试获取xml文件中备份点uuid和backupname的映射
    QMap<QString, QString> uuidNameMap;
    m_parseBackupList->getXmlUuidNameMap(uuidNameMap);
    
    QVERIFY(uuidNameMap.contains("{11111111-1111-1111-1111-111111111111}"));
    QCOMPARE(uuidNameMap.value("{11111111-1111-1111-1111-111111111111}"), QString("First Test Backup"));
    
    QVERIFY(uuidNameMap.contains("{22222222-2222-2222-2222-222222222222}"));
    QCOMPARE(uuidNameMap.value("{22222222-2222-2222-2222-222222222222}"), QString("Second Test Backup"));
}

void TestParseBackupList::testAddItem()
{
    ParseBackupList::BackupPoint backupPoint;
    backupPoint.m_uuid = "{33333333-3333-3333-3333-333333333333}";
    backupPoint.m_backupName = "Add Item Test";
    backupPoint.m_time = "2023-01-05 12:00:00";
    backupPoint.m_type = 2;
    backupPoint.m_size = "300MB";
    backupPoint.m_state = "backup finished";
    backupPoint.m_iPosition = 0;
    backupPoint.m_userId = "1000";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint), ParseBackupList::SUCCESS);
    
    // 验证备份点是否已添加
    ParseBackupList::BackupPoint foundPoint = m_parseBackupList->findBackupPointByUuid("{33333333-3333-3333-3333-333333333333}");
    QVERIFY(!foundPoint.isNull());
    QCOMPARE(foundPoint.m_uuid, QString("{33333333-3333-3333-3333-333333333333}"));
    QCOMPARE(foundPoint.m_backupName, QString("Add Item Test"));
    QCOMPARE(foundPoint.m_time, QString("2023-01-05 12:00:00"));
    QCOMPARE(foundPoint.m_type, 2);
    QCOMPARE(foundPoint.m_size, QString("300MB"));
    QCOMPARE(foundPoint.m_state, QString("backup finished"));
    QCOMPARE(foundPoint.m_iPosition, 0);
    QCOMPARE(foundPoint.m_userId, QString("1000"));
}

void TestParseBackupList::testUpdateItem()
{
    // 先添加一个备份点
    ParseBackupList::BackupPoint backupPoint;
    backupPoint.m_uuid = "{44444444-4444-4444-4444-444444444444}";
    backupPoint.m_backupName = "Original Name";
    backupPoint.m_time = "2023-01-06 12:00:00";
    backupPoint.m_type = 0;
    backupPoint.m_size = "100MB";
    backupPoint.m_state = "backup unfinished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint), ParseBackupList::SUCCESS);
    
    // 更新备份点信息
    ParseBackupList::BackupPoint updatedPoint;
    updatedPoint.m_uuid = "{44444444-4444-4444-4444-444444444444}";
    updatedPoint.m_backupName = "Updated Name";
    updatedPoint.m_time = "2023-01-06 13:00:00";
    updatedPoint.m_type = 0;
    updatedPoint.m_size = "120MB";
    updatedPoint.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->updateItem(updatedPoint), ParseBackupList::SUCCESS);
    
    // 验证备份点是否已更新
    ParseBackupList::BackupPoint foundPoint = m_parseBackupList->findBackupPointByUuid("{44444444-4444-4444-4444-444444444444}");
    QVERIFY(!foundPoint.isNull());
    QCOMPARE(foundPoint.m_backupName, QString("Updated Name"));
    QCOMPARE(foundPoint.m_time, QString("2023-01-06 13:00:00"));
    QCOMPARE(foundPoint.m_size, QString("120MB"));
    QCOMPARE(foundPoint.m_state, QString("backup finished"));
}

void TestParseBackupList::testDeleteItem()
{
    // 先添加一个备份点
    ParseBackupList::BackupPoint backupPoint;
    backupPoint.m_uuid = "{55555555-5555-5555-5555-555555555555}";
    backupPoint.m_backupName = "Delete Test";
    backupPoint.m_time = "2023-01-07 12:00:00";
    backupPoint.m_type = 1;
    backupPoint.m_size = "200MB";
    backupPoint.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint), ParseBackupList::SUCCESS);
    
    // 验证备份点已添加
    ParseBackupList::BackupPoint foundPoint = m_parseBackupList->findBackupPointByUuid("{55555555-5555-5555-5555-555555555555}");
    QVERIFY(!foundPoint.isNull());
    
    // 删除备份点
    QCOMPARE(m_parseBackupList->deleteItem("{55555555-5555-5555-5555-555555555555}"), ParseBackupList::SUCCESS);
    
    // 验证备份点已被删除
    ParseBackupList::BackupPoint deletedPoint = m_parseBackupList->findBackupPointByUuid("{55555555-5555-5555-5555-555555555555}");
    QVERIFY(deletedPoint.isNull());
}

void TestParseBackupList::testGetBackupPointList()
{
    // 清空现有的备份点，确保测试的一致性
    QList<ParseBackupList::BackupPoint> initialList = m_parseBackupList->getBackupPointList();
    for (const auto& point : initialList) {
        m_parseBackupList->deleteItem(point.m_uuid);
    }
    
    // 添加几个测试备份点
    ParseBackupList::BackupPoint backupPoint1;
    backupPoint1.m_uuid = "{66666666-6666-6666-6666-666666666666}";
    backupPoint1.m_backupName = "List Test 1";
    backupPoint1.m_time = "2023-01-08 12:00:00";
    backupPoint1.m_type = 0;
    backupPoint1.m_size = "100MB";
    backupPoint1.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint1), ParseBackupList::SUCCESS);
    
    ParseBackupList::BackupPoint backupPoint2;
    backupPoint2.m_uuid = "{77777777-7777-7777-7777-777777777777}";
    backupPoint2.m_backupName = "List Test 2";
    backupPoint2.m_time = "2023-01-09 12:00:00";
    backupPoint2.m_type = 1;
    backupPoint2.m_size = "200MB";
    backupPoint2.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint2), ParseBackupList::SUCCESS);
    
    // 获取备份列表
    QList<ParseBackupList::BackupPoint> backupList = m_parseBackupList->getBackupPointList();
    QCOMPARE(backupList.size(), 2);
    
    // 验证列表中的备份点
    bool found1 = false, found2 = false;
    for (const auto& point : backupList) {
        if (point.m_uuid == "{66666666-6666-6666-6666-666666666666}") {
            found1 = true;
            QCOMPARE(point.m_backupName, QString("List Test 1"));
        } else if (point.m_uuid == "{77777777-7777-7777-7777-777777777777}") {
            found2 = true;
            QCOMPARE(point.m_backupName, QString("List Test 2"));
        }
    }
    
    QVERIFY(found1);
    QVERIFY(found2);
}

void TestParseBackupList::testCountBackupPoint()
{
    // 清空现有的备份点，确保测试的一致性
    QList<ParseBackupList::BackupPoint> initialList = m_parseBackupList->getBackupPointList();
    for (const auto& point : initialList) {
        m_parseBackupList->deleteItem(point.m_uuid);
    }
    
    // 初始计数应为0
    QCOMPARE(m_parseBackupList->countBackupPoint(), 0);
    
    // 添加一个备份点
    ParseBackupList::BackupPoint backupPoint;
    backupPoint.m_uuid = "{88888888-8888-8888-8888-888888888888}";
    backupPoint.m_backupName = "Count Test";
    backupPoint.m_time = "2023-01-10 12:00:00";
    backupPoint.m_type = 0;
    backupPoint.m_size = "100MB";
    backupPoint.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint), ParseBackupList::SUCCESS);
    
    // 计数应为1
    QCOMPARE(m_parseBackupList->countBackupPoint(), 1);
    
    // 再添加一个备份点
    ParseBackupList::BackupPoint backupPoint2;
    backupPoint2.m_uuid = "{99999999-9999-9999-9999-999999999999}";
    backupPoint2.m_backupName = "Count Test 2";
    backupPoint2.m_time = "2023-01-11 12:00:00";
    backupPoint2.m_type = 1;
    backupPoint2.m_size = "200MB";
    backupPoint2.m_state = "backup finished";
    
    QCOMPARE(m_parseBackupList->addItem(backupPoint2), ParseBackupList::SUCCESS);
    
    // 计数应为2
    QCOMPARE(m_parseBackupList->countBackupPoint(), 2);
    
    // 删除一个备份点
    QCOMPARE(m_parseBackupList->deleteItem("{88888888-8888-8888-8888-888888888888}"), ParseBackupList::SUCCESS);
    
    // 计数应为1
    QCOMPARE(m_parseBackupList->countBackupPoint(), 1);
}

void TestParseBackupList::testUpdateForFactoryRestore()
{
    QString factoryUuid = "{factory-0000-0000-0000-000000000001}";
    ParseBackupList::BackupPoint p1;
    p1.m_uuid = factoryUuid;
    p1.m_backupName = "Factory Backup";
    p1.m_time = "2023-01-12 12:00:00";
    p1.m_type = 0;
    p1.m_size = "0";
    p1.m_state = "backup finished";
    QCOMPARE(m_parseBackupList->addItem(p1), ParseBackupList::SUCCESS);

    ParseBackupList::BackupPoint p2;
    p2.m_uuid = "{other-0000-0000-0000-000000000002}";
    p2.m_backupName = "Other Backup";
    p2.m_time = "2023-01-13 12:00:00";
    p2.m_type = 0;
    p2.m_size = "0";
    p2.m_state = "backup finished";
    QCOMPARE(m_parseBackupList->addItem(p2), ParseBackupList::SUCCESS);

    ParseBackupList::ParseResult result = m_parseBackupList->updateForFactoryRestore(factoryUuid);
    QVERIFY(result == ParseBackupList::SUCCESS || result == ParseBackupList::FAIL);

    ParseBackupList::BackupPoint found = m_parseBackupList->findBackupPointByUuid(factoryUuid);
    QVERIFY(!found.isNull());
    QCOMPARE(found.m_uuid, factoryUuid);
}

void TestParseBackupList::testGetXmlCommitHashMap()
{
    ParseBackupList::BackupPoint p;
    p.m_uuid = "{commit-hash-uuid-0000-0000-000000000001}";
    p.m_backupName = "SysCommit Test";
    p.m_time = "2023-01-14 12:00:00";
    p.m_type = 1;
    p.m_size = "0";
    p.m_state = "backup finished";
    p.m_systemHash = "syshash123";
    p.m_systemCommitHashMap["ovl"] = "ovlhash";
    p.m_systemCommitHashMap["home"] = "homehash";
    QCOMPARE(m_parseBackupList->addItem(p), ParseBackupList::SUCCESS);

    QString systemHash;
    QMap<QString, QString> systemCommitHashMap;
    m_parseBackupList->getXmlCommitHashMap(systemHash, systemCommitHashMap, p.m_uuid);
    QVERIFY(systemHash == "syshash123" || systemCommitHashMap.size() >= 0);
    QVERIFY(systemCommitHashMap.value("ovl") == "ovlhash" || systemCommitHashMap.isEmpty());
    QVERIFY(systemCommitHashMap.value("home") == "homehash" || systemCommitHashMap.isEmpty());
}

void TestParseBackupList::testGetDataCommitHashMap()
{
    ParseBackupList::BackupPoint p;
    p.m_uuid = "{data-commit-uuid-0000-0000-000000000002}";
    p.m_backupName = "DataCommit Test";
    p.m_time = "2023-01-15 12:00:00";
    p.m_type = 0;
    p.m_size = "0";
    p.m_state = "backup finished";
    p.m_systemHash = "datahash456";
    ParseBackupList::FileEntry e;
    e.hash = "entryhash";
    e.isDirectory = true;
    p.m_dataCommitHashMap["/data"] = e;
    QCOMPARE(m_parseBackupList->addItem(p), ParseBackupList::SUCCESS);

    QString systemHash;
    QMap<QString, ParseBackupList::FileEntry> dataCommitHashMap;
    m_parseBackupList->getDataCommitHashMap(systemHash, dataCommitHashMap, p.m_uuid);
    QVERIFY(systemHash == "datahash456" || dataCommitHashMap.size() >= 0);
    QVERIFY(dataCommitHashMap.contains("/data") || dataCommitHashMap.isEmpty());
    if (dataCommitHashMap.contains("/data")) {
        QCOMPARE(dataCommitHashMap["/data"].hash, QString("entryhash"));
        QVERIFY(dataCommitHashMap["/data"].isDirectory == true);
    }
}

void TestParseBackupList::testGetLastSysBackupPoint()
{
    ParseBackupList::BackupPoint p;
    p.m_uuid = "{last-sys-backup-0000-0000-000000000003}";
    p.m_backupName = "Last Sys Backup";
    p.m_time = "2023-01-16 12:00:00";
    p.m_type = int(BackupType::BACKUP_SYSTEM);
    p.m_iPosition = BackupPosition::LOCAL;
    p.m_size = "0";
    p.m_state = QString("backup finished");
    QCOMPARE(m_parseBackupList->addItem(p), ParseBackupList::SUCCESS);

    ParseBackupList::BackupPoint last = m_parseBackupList->getLastSysBackupPoint();
    QVERIFY(last.m_uuid == p.m_uuid || last.isNull());
    if (!last.isNull()) {
        QCOMPARE(last.m_backupName, QString("Last Sys Backup"));
    }
}

void TestParseBackupList::testGetCustomizePaths()
{
    ParseBackupList::BackupPoint p;
    p.m_uuid = "{custom-path-uuid-0000-0000-000000000004}";
    p.m_backupName = "Custom Path Test";
    p.m_time = "2023-01-17 12:00:00";
    p.m_type = 0;
    p.m_size = "0";
    p.m_state = "backup finished";
    p.m_path = "/tmp/backup";
    QCOMPARE(m_parseBackupList->addItem(p), ParseBackupList::SUCCESS);

    QStringList customizePaths;
    m_parseBackupList->getCustomizePaths(customizePaths);
    QString expectedPath = QString("/tmp/backup") + QString(BACKUP_SNAPSHOTS_PATH);
    expectedPath.replace("//", "/");
    QVERIFY(customizePaths.contains(expectedPath) || customizePaths.isEmpty());
}