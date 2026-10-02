#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QtTest/QTest>
#include "../registeredQDbus/dev-power-control/i2cdevice.h"

class I2cDeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(m_tempDir->isValid());
        
        setupMockDevice();
        m_device = new I2cDevice();
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
        
        QString devicePath = basePath + "/sys/bus/i2c/devices/i2c-0";
        dir.mkpath(devicePath + "/power");

        createMockFile(devicePath + "/power/control", "auto");
        createMockFile(devicePath + "/name", "i2c-test-device");
    }

    void createMockFile(const QString &path, const QString &content) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
        }
    }

    QScopedPointer<QTemporaryDir> m_tempDir;
    I2cDevice* m_device{nullptr};
};

// 测试设置所有设备电源状态
TEST_F(I2cDeviceTest, SetAllDevicePowerStatTest) {
    EXPECT_FALSE(m_device->setAllDevicePowerStat("invalid"));
    
    EXPECT_TRUE(m_device->setAllDevicePowerStat("auto"));
    EXPECT_TRUE(m_device->setAllDevicePowerStat("on"));
    EXPECT_TRUE(m_device->setAllDevicePowerStat("default"));
}

// 测试打印设备信息
TEST_F(I2cDeviceTest, PrintDeviceInfoTest) {
    testing::internal::CaptureStdout();
    m_device->printAllDeviceInfo();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_TRUE(output.find("i2c device") != std::string::npos);
}