#ifndef TEST_SYSTEMDBUSREGISTER_H
#define TEST_SYSTEMDBUSREGISTER_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include "../../../registeredQDbus/systemdbusregister.h"

class TestSystemDbusRegister : public QObject
{
    Q_OBJECT

public:
    TestSystemDbusRegister();

private slots:
       // 测试生命周期管理
    void initTestCase();          // 测试用例初始化
    void cleanupTestCase();       // 测试用例清理
    
    // 基础功能测试
    void testConstructor();       // 测试构造函数
    
    // 静态函数测试
    void testIsRunning_data();    // isRunning测试数据
    void testIsRunning();         // 测试isRunning函数
    
    // 设备管理函数测试
    void testMountDevice_data();  // mountDevice测试数据
    void testMountDevice();       // 测试mountDevice函数
    
    void testUnmountPartition_data();  // unmountPartition测试数据
    void testUnmountPartition();       // 测试unmountPartition函数
    
    void testUmountPath_data();   // umountPath测试数据
    void testUmountPath();        // 测试umountPath函数
    
    // 数据处理函数测试
    void testQStringToJsonArray_data();  // QStringToJsonArray测试数据
    void testQStringToJsonArray();       // 测试QStringToJsonArray函数
    
    void testFindPartitions_data();      // findPartitions测试数据
    void testFindPartitions();           // 测试findPartitions函数
    
    // 新增测试函数
    void testGetMountPoint();            // 测试getmountpoint函数
    void testIsMakingSuccess();          // 测试isMakingSucess函数
    void testMakeExit();                 // 测试MakeExit槽函数
    void testSignalEmissions();          // 测试信号发射
    
    // 安全性测试
    void testCommandInjectionPrevention(); // 测试命令注入防护
    
    // 路径处理测试
    void testDevicePathParsing();        // 测试设备路径解析逻辑
    void testPathValidation_data();      // 路径验证测试数据
    void testPathValidation();           // 测试路径验证逻辑
    
    // JSON处理测试
    void testJsonParsingEdgeCases();     // 测试JSON解析边缘情况
    
    // 进程管理测试
    void testProcessManagement();        // 测试进程管理功能

private:
    SystemDbusRegister* m_systemDbusRegister;
};

#endif // TEST_SYSTEMDBUSREGISTER_H
