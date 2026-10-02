#include "gtest/gtest.h"
#include <QGSettings>
#include <QTemporaryDir>
#include <QFile>
#include <QDebug>
#include "../registeredQDbus/dev-power-control/pcidevice.h"


class PciDeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        m_tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(m_tempDir->isValid());
        
        // 设置环境变量指向模拟目录
        m_originalPath = qgetenv("PCI_DEVICE_PATH");
        qputenv("PCI_DEVICE_PATH", m_tempDir->path().toUtf8() + "/sys/bus/pci/devices/");
        
        // 创建完整的模拟环境
        setupMockEnvironment();
        
        // 初始化设备对象
        m_device = new PciDevice();
    }

    void TearDown() override {
        if (m_device) {
            delete m_device;
            m_device = nullptr;
        }
        
        // 恢复环境变量
        if (m_originalPath.isEmpty()) {
            qunsetenv("PCI_DEVICE_PATH");
        } else {
            qputenv("PCI_DEVICE_PATH", m_originalPath);
        }
        
        m_tempDir.reset();
    }

    void setupMockEnvironment() {
        QString basePath = m_tempDir->path();
        QDir dir;
        
        // 创建完整的PCI设备目录结构
        QString devicePath = basePath + "/sys/bus/pci/devices/0000:00:00.0";
        dir.mkpath(devicePath + "/power");

        // 创建必要的文件和内容
        createMockFile(devicePath + "/vendor", "0x8086");
        createMockFile(devicePath + "/device", "0x1234");
        createMockFile(devicePath + "/power/control", "auto");
        createMockFile(devicePath + "/power/runtime_active_time", "1000");
        createMockFile(devicePath + "/power/runtime_suspended_time", "500");
    }

    void createMockFile(const QString &path, const QString &content) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
        }
    }

    QScopedPointer<QTemporaryDir> m_tempDir;
    PciDevice* m_device{nullptr};
    QByteArray m_originalPath;
};

// 基本测试用例
TEST_F(PciDeviceTest, BasicTest) {
    ASSERT_NE(m_device, nullptr);
    EXPECT_GE(m_device->getPCIDeviceNum(), 0);
}

// 设备数量测试
TEST_F(PciDeviceTest, DeviceCountTest) {
    int count = m_device->getPCIDeviceNum();
    EXPECT_GE(count, 0);
    qDebug() << "Device count:" << count;
}

// 测试电源状态设置
TEST_F(PciDeviceTest, PowerStateTest) {
    EXPECT_TRUE(m_device->setAllDevicePowerStat("auto"));
    EXPECT_FALSE(m_device->setAllDevicePowerStat("invalid"));
}

// 测试设备初始化
TEST_F(PciDeviceTest, Constructor) {
    EXPECT_NE(m_device, nullptr);
}

// 测试设备数量获取
TEST_F(PciDeviceTest, GetPCIDeviceNum) {
    EXPECT_GE(m_device->getPCIDeviceNum(), 0);
}

// 测试设置所有设备电源状态
TEST_F(PciDeviceTest, SetAllDevicePowerStat) {
    // 测试无效参数
    EXPECT_FALSE(m_device->setAllDevicePowerStat("invalid"));
    
    // 测试有效参数
    EXPECT_TRUE(m_device->setAllDevicePowerStat("auto"));
    EXPECT_TRUE(m_device->setAllDevicePowerStat("on"));
    EXPECT_TRUE(m_device->setAllDevicePowerStat("default"));
}
