#include <gtest/gtest.h>
#include <QTemporaryDir>
#include "../registeredQDbus/dev-power-control/devicepowerinfo.h"


class DevicePowerInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(m_tempDir->isValid());
        setupMockDevice();
        
        // 验证测试环境
        QDir dir(m_powerPath);
        ASSERT_TRUE(dir.exists()) << "Power path does not exist: " << m_powerPath.toStdString();
    }

    void setupMockDevice() {
        QString basePath = m_tempDir->path();
        m_devicePath = basePath + "/sys/bus/pci/devices/0000:00:00.0";
        m_powerPath = m_devicePath + "/power";
        
        QDir dir;
        ASSERT_TRUE(dir.mkpath(m_powerPath));
        
        // 创建并验证控制文件
        createMockFile(m_powerPath + "/control", "auto");
        QFile controlFile(m_powerPath + "/control");
        ASSERT_TRUE(controlFile.exists());
    }

    void createMockFile(const QString &path, const QString &content) {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text))
            << "Failed to create file: " << path.toStdString();
        file.write(content.toUtf8());
        file.close();
    }

    QScopedPointer<QTemporaryDir> m_tempDir;
    QString m_devicePath;
    QString m_powerPath;
};

TEST_F(DevicePowerInfoTest, SetDevicePowerStatTest) {
    DevicePowerInfo device("pci", "Test Device", m_devicePath, m_powerPath, nullptr);
    
    // 验证设备路径
    ASSERT_TRUE(QFile::exists(m_powerPath + "/control"))
        << "Control file does not exist: " << (m_powerPath + "/control").toStdString();
    
    // 测试有效状态
    EXPECT_TRUE(device.setDevicePowerStat("auto")) << "Failed to set power state to auto";
    EXPECT_TRUE(device.setDevicePowerStat("on")) << "Failed to set power state to on";
    EXPECT_TRUE(device.setDevicePowerStat("default")) << "Failed to set power state to default";

    // 测试无效状态
    EXPECT_FALSE(device.setDevicePowerStat("invalid"));

    // 测试无效路径
    DevicePowerInfo invalidDevice("pci", "Test Device", "/invalid/path", "/invalid/power", nullptr);
    EXPECT_FALSE(invalidDevice.setDevicePowerStat("auto"));
}