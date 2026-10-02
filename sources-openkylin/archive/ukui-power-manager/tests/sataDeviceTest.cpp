#include <gtest/gtest.h>
#include <QTemporaryDir>
#include "../registeredQDbus/dev-power-control/satadevice.h"

class SataDeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(m_tempDir->isValid());
        setupMockDevice();
        m_device = new SataDevice();
    }

    void TearDown() override {
        if (m_device) {
            delete m_device;
            m_device = nullptr;
        }
        m_tempDir.reset();
    }

    void setupMockDevice() {
        QString basePath = m_tempDir->path();
        QDir dir;
        
        // 创建SATA设备目录结构
        for(int i = 0; i < 2; i++) {
            QString devicePath = basePath + "/sys/class/scsi_host/host" + QString::number(i);
            dir.mkpath(devicePath);
            
            // 创建电源管理策略文件
            createMockFile(devicePath + "/link_power_management_policy", "min_power");
        }
    }

    void createMockFile(const QString &path, const QString &content) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
        }
    }

    QScopedPointer<QTemporaryDir> m_tempDir;
    SataDevice* m_device{nullptr};
};

// 测试设备数量
TEST_F(SataDeviceTest, GetDeviceNumTest) {
    int count = m_device->getPCIDeviceNum();
    EXPECT_GE(count, 0);
}



// 测试电源状态
TEST_F(SataDeviceTest, PowerStateTest) {
    if (m_device->getPCIDeviceNum() > 0) {
        // 测试当前状态
        EXPECT_NE(m_device->getCurrentPowerStat(0), "False");
        
        // 测试默认状态
        EXPECT_NE(m_device->getDefaultPowerStat(0), "False");
        
        // 测试设置状态
        EXPECT_TRUE(m_device->setPowerStat(0, "min_power"));
        EXPECT_TRUE(m_device->setPowerStat(0, "max_performance"));
        EXPECT_TRUE(m_device->setPowerStat(0, "default"));
        EXPECT_FALSE(m_device->setPowerStat(0, "invalid"));
    }
}

// 测试设置所有设备电源状态
TEST_F(SataDeviceTest, SetAllDevicePowerStatTest) {
    EXPECT_TRUE(m_device->setAllDevicePowerStat("min_power"));
    EXPECT_TRUE(m_device->setAllDevicePowerStat("max_performance"));
    EXPECT_TRUE(m_device->setAllDevicePowerStat("default"));
    EXPECT_FALSE(m_device->setAllDevicePowerStat("invalid"));
}