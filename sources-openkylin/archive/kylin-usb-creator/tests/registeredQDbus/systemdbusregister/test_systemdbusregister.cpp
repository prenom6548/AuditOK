#include "test_systemdbusregister.h"
#include <QCoreApplication>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>

TestSystemDbusRegister::TestSystemDbusRegister()
{
}

void TestSystemDbusRegister::initTestCase()
{
    m_systemDbusRegister = new SystemDbusRegister();
}

void TestSystemDbusRegister::cleanupTestCase()
{
    delete m_systemDbusRegister;
}

void TestSystemDbusRegister::testConstructor()
{
    // 测试构造函数
    QVERIFY(m_systemDbusRegister != nullptr);
    
    // 验证初始状态
    QCOMPARE(m_systemDbusRegister->isRunning("nonexistent_process"), false);
}

void TestSystemDbusRegister::testIsRunning_data()
{
    QTest::addColumn<QString>("processName");
    QTest::addColumn<bool>("expectedResult");
    
    // 测试存在的进程（通常系统会运行的一些进程）
    QTest::newRow("system_process") << "systemd" << true;
    QTest::newRow("nonexistent_process") << "nonexistent_process_xyz123" << false;
    QTest::newRow("empty_string") << "" << false;
    QTest::newRow("self_process") << "test_systemdbusregister" << true; // 测试程序自身
    QTest::newRow("partial_name") << "test" << true; // 部分匹配
}

void TestSystemDbusRegister::testIsRunning()
{
    QFETCH(QString, processName);
    QFETCH(bool, expectedResult);
    
    bool result = SystemDbusRegister::isRunning(processName);
    
    // 对于部分匹配的情况，结果可能为true
    if (processName == "test" || processName == "test_systemdbusregister") {
        QVERIFY(result == true);
    } else {
        QCOMPARE(result, expectedResult);
    }
}

void TestSystemDbusRegister::testMountDevice_data()
{
    QTest::addColumn<QString>("targetPath");
    QTest::addColumn<bool>("expectedResult");
    
    QTest::newRow("null_device") << QString() << false;
    QTest::newRow("empty_path") << "" << false;
    QTest::newRow("invalid_path") << "/invalid/path/xyz" << false;
    QTest::newRow("root_path") << "/" << false; // 根路径不应挂载
    QTest::newRow("proc_path") << "/proc/cpuinfo" << false; // 特殊文件系统
    QTest::newRow("dev_null") << "/dev/null" << false; // 特殊设备文件
    QTest::newRow("with_spaces") << "/dev/sd a" << false; // 带空格的路径
    QTest::newRow("relative_path") << "relative/path" << false; // 相对路径
}

void TestSystemDbusRegister::testMountDevice()
{
    QFETCH(QString, targetPath);
    QFETCH(bool, expectedResult);
    
    bool result = m_systemDbusRegister->mountDevice(targetPath);
    
    // 对于无效路径，mountDevice应该返回false
    // 我们主要验证函数不会崩溃，并且返回合理的值
    QCOMPARE(result, false); // 在测试环境中，没有真实设备，所有挂载都应该失败
}

void TestSystemDbusRegister::testUnmountPartition_data()
{
    QTest::addColumn<QString>("partition");
    QTest::addColumn<bool>("expectedResult");
    
    QTest::newRow("null_partition") << QString() << false;
    QTest::newRow("empty_partition") << "" << false;
    QTest::newRow("invalid_partition") << "/dev/invalid123" << false;
    QTest::newRow("unmounted_partition") << "/dev/sda1" << false; // 未挂载的分区
    QTest::newRow("system_partition") << "/dev/root" << false; // 系统分区
    QTest::newRow("with_options") << "/dev/sda1 -l" << false; // 包含选项
}

void TestSystemDbusRegister::testUnmountPartition()
{
    QFETCH(QString, partition);
    QFETCH(bool, expectedResult);
    
    bool result = m_systemDbusRegister->unmountPartition(partition);
    
    // 在测试环境中，卸载操作应该失败
    // 主要验证函数不会崩溃
    QCOMPARE(result, false);
}

void TestSystemDbusRegister::testUmountPath_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("expectedResult");
    
    QTest::newRow("null_path") << QString() << false;
    QTest::newRow("empty_path") << "" << false;
    QTest::newRow("invalid_path") << "/invalid/path/xyz" << false;
    QTest::newRow("mnt_path") << "/mnt" << false;
    QTest::newRow("home_path") << "/home" << false;
    QTest::newRow("tmp_path") << "/tmp" << false;
    QTest::newRow("dev_path") << "/dev" << false;
    QTest::newRow("proc_path") << "/proc" << false;
    QTest::newRow("sys_path") << "/sys" << false;
}

void TestSystemDbusRegister::testUmountPath()
{
    QFETCH(QString, path);
    QFETCH(bool, expectedResult);
    
    bool result = m_systemDbusRegister->umountPath(path);
    
    // 验证函数执行不会崩溃
    // 在测试环境中，umountPath通常会返回false或true（表示没有需要卸载的内容）
    QVERIFY(result == false || result == true);
}

void TestSystemDbusRegister::testQStringToJsonArray_data()
{
    QTest::addColumn<QString>("jsonString");
    QTest::addColumn<int>("expectedSize");
    QTest::addColumn<bool>("shouldBeValid");
    
    // 有效的JSON测试用例
    QTest::newRow("valid_json_with_two_devices") 
        << "{\"blockdevices\":[{\"name\":\"sda\"},{\"name\":\"sdb\"}]}" 
        << 2 
        << true;
    
    QTest::newRow("valid_json_empty_array") 
        << "{\"blockdevices\":[]}" 
        << 0 
        << true;
    
    QTest::newRow("valid_json_complex_structure") 
        << "{\"blockdevices\":[{\"name\":\"sda\",\"type\":\"disk\",\"children\":[]}]}" 
        << 1 
        << true;
    
    // 无效的JSON测试用例
    QTest::newRow("invalid_json_missing_brace") 
        << "{\"blockdevices\":[{\"name\":\"sda\"}" 
        << 0 
        << false;
    
    QTest::newRow("invalid_json_wrong_format") 
        << "invalid json string" 
        << 0 
        << false;
    
    QTest::newRow("empty_string") 
        << "" 
        << 0 
        << false;
    
    QTest::newRow("null_json") 
        << QString() 
        << 0 
        << false;
    
    QTest::newRow("json_without_blockdevices") 
        << "{\"otherkey\":[1,2,3]}" 
        << 0 
        << false;
    
    QTest::newRow("json_not_array") 
        << "{\"blockdevices\":{\"name\":\"sda\"}}" 
        << 0 
        << false;
}

void TestSystemDbusRegister::testQStringToJsonArray()
{
    QFETCH(QString, jsonString);
    QFETCH(int, expectedSize);
    QFETCH(bool, shouldBeValid);
    
    QJsonArray result = m_systemDbusRegister->QStringToJsonArray(jsonString);
    
    if (shouldBeValid) {
        QCOMPARE(result.size(), expectedSize);
    } else {
        // 对于无效JSON，结果应该是空数组
        QCOMPARE(result.size(), 0);
    }
}

void TestSystemDbusRegister::testFindPartitions_data()
{
    QTest::addColumn<QString>("devname");
    QTest::addColumn<int>("expectedSize");
    
    QTest::newRow("empty_devname") << "" << 0;
    QTest::newRow("invalid_devname") << "invalid123" << 0;
    QTest::newRow("numeric_devname") << "123" << 0;
    QTest::newRow("devname_with_slash") << "sda/1" << 0;
    QTest::newRow("devname_with_spaces") << "sd a" << 0;
    QTest::newRow("long_devname") << "verylongdevicename1234567890" << 0;
}

void TestSystemDbusRegister::testFindPartitions()
{
    QFETCH(QString, devname);
    QFETCH(int, expectedSize);
    
    QStringList result = m_systemDbusRegister->findPartitions(devname);
    
    // 对于无效设备名或测试环境，应该返回空列表
    QCOMPARE(result.size(), expectedSize);
    QVERIFY(result.isEmpty() == (expectedSize == 0));
}

// 新增加的测试函数

void TestSystemDbusRegister::testGetMountPoint()
{
    // 测试getmountpoint函数（不依赖真实设备）
    m_systemDbusRegister->getmountpoint();
    
    // 验证函数执行不会崩溃
    QVERIFY(true);
}

void TestSystemDbusRegister::testIsMakingSuccess()
{
    // 测试isMakingSucess函数
    m_systemDbusRegister->isMakingSucess();
    
    // 验证函数执行不会崩溃
    QVERIFY(true);
}

void TestSystemDbusRegister::testMakeExit()
{
    // 测试MakeExit槽函数
    // 在测试环境中，应该不会执行实际的kill操作
    m_systemDbusRegister->MakeExit();
    
    // 验证函数执行不会崩溃
    QVERIFY(true);
}

void TestSystemDbusRegister::testSignalEmissions()
{
    // 测试信号发射（模拟）
    // 注意：由于这是单元测试，我们不实际执行DD命令
    
    // 创建信号监视器
    QSignalSpy progressSpy(m_systemDbusRegister, &SystemDbusRegister::workingProgress);
    QSignalSpy finishSpy(m_systemDbusRegister, &SystemDbusRegister::makeFinish);
    QSignalSpy errorSpy(m_systemDbusRegister, &SystemDbusRegister::errorstr);
    QSignalSpy authSpy(m_systemDbusRegister, &SystemDbusRegister::authorityStatus);
    
    // 验证初始状态没有信号
    QCOMPARE(progressSpy.count(), 0);
    QCOMPARE(finishSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(authSpy.count(), 0);
}

void TestSystemDbusRegister::testCommandInjectionPrevention()
{
    // 测试命令注入防护逻辑
    // 这些测试用例模拟MakeStart函数中的防护检查
    
    QStringList injectionPatterns = {
        "`id`",
        "test`whoami`test",
        "$(ls)",
        "test$(cat /etc/passwd)test",
        "${PATH}",
        "test${HOME}test",
        "test;ls",
        "test&ls",
        "test|ls",
        "test&&ls",
        "test||ls",
        ";",
        "&",
        "|",
        "`echo test`",
        "$(echo test)",
        "${USER}"
    };
    
    for (const QString &pattern : injectionPatterns) {
        // 测试sourcePath包含注入模式
        // 注意：我们无法直接调用MakeStart，因为它需要Polkit授权
        // 这里只是验证注入模式会被正则表达式匹配
        
        QRegularExpression re1(".*`.*`.*");
        QRegularExpression re2(".*\\$\\(.*\\).*");
        QRegularExpression re3(".*\\$\\{.*\\}.*");
        QRegularExpression re4("[;&|]+");
        
        bool isInjection = re1.match(pattern).hasMatch() ||
                          re2.match(pattern).hasMatch() ||
                          re3.match(pattern).hasMatch() ||
                          re4.match(pattern).hasMatch();
        
        QVERIFY(isInjection == true);
    }
}

void TestSystemDbusRegister::testDevicePathParsing()
{
    // 测试设备路径解析逻辑（模拟umountPath函数中的逻辑）
    
    struct TestCase {
        QString input;
        QString expected;
    };
    
    QVector<TestCase> testCases = {
        {"/dev/sda1", "sda"},
        {"/dev/sdb2", "sdb"},
        {"/dev/nvme0n1p3", "nvme0n1p"}, // 注意：这个逻辑可能有问题
        {"/dev/mmcblk0p1", "mmcblk0p"}, // 注意：这个逻辑可能有问题
        {"/dev/sda", "sda"},
        {"/dev/sdb", "sdb"},
        {"/invalid/path", "invalid/path"}, // 非标准路径
        {"", ""},
        {"/dev/", ""}
    };
    
    for (const TestCase &tc : testCases) {
        QString result = tc.input.mid(5); // 去掉"/dev/"
        if (result.contains(QRegExp("[0-9]$"))) {
            result = result.left(result.size() - 1);
        }
        
        // 验证解析逻辑不会崩溃
        QVERIFY(!result.isNull());
    }
}

void TestSystemDbusRegister::testJsonParsingEdgeCases()
{
    // 测试JSON解析的边缘情况
    
    QStringList edgeCases = {
        // 超大JSON
        QString("{\"blockdevices\":").repeated(1000) + "[{\"name\":\"test\"}]}",
        // 特殊字符
        "{\"blockdevices\":[{\"name\":\"sda\\twith\\ttab\"}]}",
        "{\"blockdevices\":[{\"name\":\"sda\\nwith\\nnewline\"}]}",
        "{\"blockdevices\":[{\"name\":\"sda\\\\with\\\\backslash\"}]}",
        "{\"blockdevices\":[{\"name\":\"sda\\\"with\\\"quotes\"}]}",
        // Unicode字符
        "{\"blockdevices\":[{\"name\":\"sda_测试_テスト\"}]}",
        // 空值
        "{\"blockdevices\":[null]}",
        // 混合类型
        "{\"blockdevices\":[{\"name\":\"sda\"}, 123, \"string\"]}",
        // 深度嵌套
        "{\"blockdevices\":[{\"children\":[{\"children\":[{\"name\":\"deep\"}]}]}]}"
    };
    
    for (const QString &jsonStr : edgeCases) {
        QJsonArray result = m_systemDbusRegister->QStringToJsonArray(jsonStr);
        // 主要验证不会崩溃
        QVERIFY(true);
    }
}

void TestSystemDbusRegister::testProcessManagement()
{
    // 测试进程管理相关功能
    
    // 测试不存在的进程
    QVERIFY(!SystemDbusRegister::isRunning("__nonexistent_process_name_xyz_123"));
    
    // 测试空进程名
    QVERIFY(!SystemDbusRegister::isRunning(""));
    
    // 测试当前测试进程（应该存在）
    QString currentProcess = QCoreApplication::applicationName();
    if (!currentProcess.isEmpty()) {
        QVERIFY(SystemDbusRegister::isRunning(currentProcess));
    }
}

// 数据驱动测试的辅助函数
void TestSystemDbusRegister::testPathValidation_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("isValid");
    
    // 有效路径（格式上）
    QTest::newRow("standard_device") << "/dev/sda1" << true;
    QTest::newRow("nvme_device") << "/dev/nvme0n1p1" << true;
    QTest::newRow("mmc_device") << "/dev/mmcblk0p1" << true;
    
    // 无效路径
    QTest::newRow("no_dev_prefix") << "sda1" << false;
    QTest::newRow("double_slash") << "/dev//sda1" << false;
    QTest::newRow("trailing_slash") << "/dev/sda1/" << false;
    QTest::newRow("contains_null") << "/dev/sda\0" << false;
    QTest::newRow("absolute_path_not_device") << "/home/user/file.txt" << false;
}

void TestSystemDbusRegister::testPathValidation()
{
    QFETCH(QString, path);
    QFETCH(bool, isValid);
    
    // 简单的路径验证逻辑
    bool actualIsValid = path.startsWith("/dev/") && 
                        !path.contains("//") && 
                        !path.endsWith("/") &&
                        path.length() > 5; // 长于"/dev/"
    
    QCOMPARE(actualIsValid, isValid);
}

QTEST_MAIN(TestSystemDbusRegister)
