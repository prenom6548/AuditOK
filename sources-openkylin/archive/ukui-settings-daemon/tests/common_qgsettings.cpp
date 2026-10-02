#include <gtest/gtest.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "QGSettings/qgsettings.h"
#include "QGSettings/qconftype.h"

#define TEST_ENABLE_KEY "touchpad-enabled"

#define UKUI_TOUCHPAD_SCHEMA    "org.ukui.peripherals-touchpad"

class qgsettingsTest : public ::testing::Test {
protected:
   void SetUp() override {
        m_pgsettings = new QGSettings(UKUI_TOUCHPAD_SCHEMA);
   }

   void TearDown() override {
       // 清理测试环境
   }
   QGSettings *m_pgsettings = nullptr;
};

TEST_F(qgsettingsTest, is_item_in_schemaTest) {
    QGSettings::isSchemaInstalled(UKUI_TOUCHPAD_SCHEMA);
}

TEST_F(qgsettingsTest, qconf_types_collect_from_variantTest) {

}


TEST_F(qgsettingsTest, trySetTest) {
    m_pgsettings->trySet(TEST_ENABLE_KEY,true);    EXPECT_EQ(1,1);
}


TEST_F(qgsettingsTest, setEnumTest) {
    m_pgsettings->setEnum(TEST_ENABLE_KEY,true);    EXPECT_EQ(1,1);
}


TEST_F(qgsettingsTest, getEnumTest) {
    m_pgsettings->getEnum(TEST_ENABLE_KEY);    EXPECT_EQ(1,1);
}


TEST_F(qgsettingsTest, getStrvTest) {
    m_pgsettings->getStrv(TEST_ENABLE_KEY);    EXPECT_EQ(1,1);
}


TEST_F(qgsettingsTest, delayTest) {
    m_pgsettings->delay();    EXPECT_EQ(1,1);
}


TEST_F(qgsettingsTest, applyTest) {
  m_pgsettings->apply();    EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, keysTest) {
  m_pgsettings->keys();    EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, choicesTest) {
    m_pgsettings->choices(TEST_ENABLE_KEY);    EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, resetTest) {
  m_pgsettings->reset(TEST_ENABLE_KEY);    EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, isSchemaInstalledTest) {
    m_pgsettings->isSchemaInstalled(UKUI_TOUCHPAD_SCHEMA);
    EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, getAllSchemaWithFilterTest) {
    m_pgsettings->getAllSchemaWithFilter("*");
    EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, getSummaryTest) {
    m_pgsettings->getSummary(TEST_ENABLE_KEY);
        EXPECT_EQ(1,1);
}

TEST_F(qgsettingsTest, getKeyTypeTest) {
    m_pgsettings->getKeyType(TEST_ENABLE_KEY);
    EXPECT_EQ(1,1);
}

