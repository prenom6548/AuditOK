#include "test_functypeconverter.h"
#include "../../../kybackup/functypeconverter.h"

void TestFuncTypeConverter::initTestCase()
{
    m_converter = new FuncTypeConverter();
}

void TestFuncTypeConverter::cleanupTestCase()
{
    delete m_converter;
}

// 测试keyCodeToKeyString方法的数据驱动测试
void TestFuncTypeConverter::testKeyCodeToKeyString_data()
{
    QTest::addColumn<int>("code");
    QTest::addColumn<QString>("expected");

    QTest::newRow("BACKUP_SYSTEM") << static_cast<int>(FuncTypeConverter::BACKUP_SYSTEM) << QString("BACKUP_SYSTEM");
    QTest::newRow("RESTORE_SYSTEM") << static_cast<int>(FuncTypeConverter::RESTORE_SYSTEM) << QString("RESTORE_SYSTEM");
    QTest::newRow("BACKUP_DATA") << static_cast<int>(FuncTypeConverter::BACKUP_DATA) << QString("BACKUP_DATA");
    QTest::newRow("RESTORE_DATA") << static_cast<int>(FuncTypeConverter::RESTORE_DATA) << QString("RESTORE_DATA");
    QTest::newRow("OPERATION_LOG") << static_cast<int>(FuncTypeConverter::OPERATION_LOG) << QString("OPERATION_LOG");
    QTest::newRow("GHOST_IMAGE") << static_cast<int>(FuncTypeConverter::GHOST_IMAGE) << QString("GHOST_IMAGE");
    QTest::newRow("TOTALMODULES") << static_cast<int>(FuncTypeConverter::TOTALMODULES) << QString("TOTALMODULES");
    QTest::newRow("Invalid code") << -1 << QString(); // 无效代码应返回空字符串
}

void TestFuncTypeConverter::testKeyCodeToKeyString()
{
    QFETCH(int, code);
    QFETCH(QString, expected);

    QString result = m_converter->keyCodeToKeyString(code);
    QCOMPARE(result, expected);
}

// 测试keystringTokeycode方法的数据驱动测试
void TestFuncTypeConverter::testKeystringTokeycode_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("expected");

    QTest::newRow("BACKUP_SYSTEM") << QString("BACKUP_SYSTEM") << static_cast<int>(FuncTypeConverter::BACKUP_SYSTEM);
    QTest::newRow("RESTORE_SYSTEM") << QString("RESTORE_SYSTEM") << static_cast<int>(FuncTypeConverter::RESTORE_SYSTEM);
    QTest::newRow("BACKUP_DATA") << QString("BACKUP_DATA") << static_cast<int>(FuncTypeConverter::BACKUP_DATA);
    QTest::newRow("RESTORE_DATA") << QString("RESTORE_DATA") << static_cast<int>(FuncTypeConverter::RESTORE_DATA);
    QTest::newRow("OPERATION_LOG") << QString("OPERATION_LOG") << static_cast<int>(FuncTypeConverter::OPERATION_LOG);
    QTest::newRow("GHOST_IMAGE") << QString("GHOST_IMAGE") << static_cast<int>(FuncTypeConverter::GHOST_IMAGE);
    QTest::newRow("TOTALMODULES") << QString("TOTALMODULES") << static_cast<int>(FuncTypeConverter::TOTALMODULES);
    QTest::newRow("Invalid string") << QString("INVALID") << -1; // 无效字符串应返回-1
}

void TestFuncTypeConverter::testKeystringTokeycode()
{
    QFETCH(QString, string);
    QFETCH(int, expected);

    int result = m_converter->keystringTokeycode(string);
    QCOMPARE(result, expected);
}

// 测试funcTypeToFuncName方法的数据驱动测试
void TestFuncTypeConverter::testFuncTypeToFuncName_data()
{
    QTest::addColumn<int>("funcType");
    QTest::addColumn<QString>("expected");

    QTest::newRow("BACKUP_SYSTEM") << static_cast<int>(FuncTypeConverter::BACKUP_SYSTEM) << QString("System Backup");
    QTest::newRow("RESTORE_SYSTEM") << static_cast<int>(FuncTypeConverter::RESTORE_SYSTEM) << QString("System Recovery");
    QTest::newRow("BACKUP_DATA") << static_cast<int>(FuncTypeConverter::BACKUP_DATA) << QString("Data Backup");
    QTest::newRow("RESTORE_DATA") << static_cast<int>(FuncTypeConverter::RESTORE_DATA) << QString("Data Recovery");
    QTest::newRow("OPERATION_LOG") << static_cast<int>(FuncTypeConverter::OPERATION_LOG) << QString("Log Records");
    QTest::newRow("GHOST_IMAGE") << static_cast<int>(FuncTypeConverter::GHOST_IMAGE) << QString("Ghost Image");
    QTest::newRow("Invalid type") << -1 << QString(); // 无效类型应返回空字符串
}

void TestFuncTypeConverter::testFuncTypeToFuncName()
{
    QFETCH(int, funcType);
    QFETCH(QString, expected);

    QString result = m_converter->funcTypeToFuncName(funcType);
    QCOMPARE(result, expected);
}

// 测试funcTypeToThemeIconName方法的数据驱动测试
void TestFuncTypeConverter::testFuncTypeToThemeIconName_data()
{
    QTest::addColumn<int>("funcType");
    QTest::addColumn<QString>("expected");

    QTest::newRow("BACKUP_SYSTEM") << static_cast<int>(FuncTypeConverter::BACKUP_SYSTEM) << QString("ukui-bf-system-backup-symbolic");
    QTest::newRow("RESTORE_SYSTEM") << static_cast<int>(FuncTypeConverter::RESTORE_SYSTEM) << QString("ukui-bf-system-restore-symbolic");
    QTest::newRow("BACKUP_DATA") << static_cast<int>(FuncTypeConverter::BACKUP_DATA) << QString("ukui-bf-data-backup-symbolic");
    QTest::newRow("RESTORE_DATA") << static_cast<int>(FuncTypeConverter::RESTORE_DATA) << QString("ukui-bf-data-restore-symbolic");
    QTest::newRow("OPERATION_LOG") << static_cast<int>(FuncTypeConverter::OPERATION_LOG) << QString("ukui-catalogue-symbolic");
    QTest::newRow("GHOST_IMAGE") << static_cast<int>(FuncTypeConverter::GHOST_IMAGE) << QString("ukui-bf-ghost-mirror-symbolic");
    QTest::newRow("Invalid type") << -1 << QString(); // 无效类型应返回空字符串
}

void TestFuncTypeConverter::testFuncTypeToThemeIconName()
{
    QFETCH(int, funcType);
    QFETCH(QString, expected);

    QString result = m_converter->funcTypeToThemeIconName(funcType);
    QCOMPARE(result, expected);
}

// 测试funcTypeToWidgetClassName方法的数据驱动测试
void TestFuncTypeConverter::testFuncTypeToWidgetClassName_data()
{
    QTest::addColumn<int>("funcType");
    QTest::addColumn<QString>("expected");

    QTest::newRow("BACKUP_SYSTEM") << static_cast<int>(FuncTypeConverter::BACKUP_SYSTEM) << QString("SystemBackup");
    QTest::newRow("RESTORE_SYSTEM") << static_cast<int>(FuncTypeConverter::RESTORE_SYSTEM) << QString("SystemRestore");
    QTest::newRow("BACKUP_DATA") << static_cast<int>(FuncTypeConverter::BACKUP_DATA) << QString("DataBackup");
    QTest::newRow("RESTORE_DATA") << static_cast<int>(FuncTypeConverter::RESTORE_DATA) << QString("DataRestore");
    QTest::newRow("OPERATION_LOG") << static_cast<int>(FuncTypeConverter::OPERATION_LOG) << QString("OperationLog");
    QTest::newRow("GHOST_IMAGE") << static_cast<int>(FuncTypeConverter::GHOST_IMAGE) << QString("GhostImage");
    QTest::newRow("Invalid type") << -1 << QString(); // 无效类型应返回空字符串
}

void TestFuncTypeConverter::testFuncTypeToWidgetClassName()
{
    QFETCH(int, funcType);
    QFETCH(QString, expected);

    QString result = m_converter->funcTypeToWidgetClassName(funcType);
    QCOMPARE(result, expected);
}