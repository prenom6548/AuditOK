#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include "../registeredQDbus/brightness-control/brightnessnode.h"


class BrightnessNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(m_tempDir->isValid());
        setupMockDevice();
        m_node = new BrightnessNode(50);
    }

    void TearDown() override {
        if (m_node) {
            delete m_node;
            m_node = nullptr;
        }
        m_tempDir.reset();
    }

    void setupMockDevice() {
        QString basePath = m_tempDir->path() + "/sys/class/backlight/intel_backlight";
        QDir dir;
        ASSERT_TRUE(dir.mkpath(basePath));
        
        createMockFile(basePath + "/max_brightness", "100");
        createMockFile(basePath + "/brightness", "50");
        createMockFile(basePath + "/type", "firmware");
    }

    void createMockFile(const QString &path, const QString &content) {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write(content.toUtf8());
        file.close();
    }

    QScopedPointer<QTemporaryDir> m_tempDir;
    BrightnessNode* m_node{nullptr};
};
// 测试能否设置亮度
TEST_F(BrightnessNodeTest, CanSetBrightnessTest) {
    EXPECT_TRUE(m_node->canSetBrightness());
}

// 测试亮度百分比设置和获取
TEST_F(BrightnessNodeTest, BrightnessPercentTest) {
    // 正常范围
    m_node->setBrightnessPercent(75);
    EXPECT_EQ(m_node->getBrightnessPercent(), 75);

    // 小于最小值
    m_node->setBrightnessPercent(1);
    EXPECT_EQ(m_node->getBrightnessPercent(), 1);

    // 大于最大值
    m_node->setBrightnessPercent(150);
    EXPECT_EQ(m_node->getBrightnessPercent(), 100);
}

// 测试最大亮度获取
TEST_F(BrightnessNodeTest, GetMaxBrightnessTest) {
    EXPECT_EQ(m_node->getMaxBrightness(), 100);
}

// 测试亮度降低功能
TEST_F(BrightnessNodeTest, ReduceMaxBrightnessTest) {
    int originalMax = m_node->getMaxBrightness();
    
    // 启用亮度降低
    m_node->setReduceMaxBrightness(true);
    m_node->setBrightnessPercent(100);
    EXPECT_EQ(m_node->getBrightnessPercent(), 100);
    
    // 禁用亮度降低
    m_node->setReduceMaxBrightness(false);
    m_node->setBrightnessPercent(100);
    EXPECT_EQ(m_node->getBrightnessPercent(), 100);
}

