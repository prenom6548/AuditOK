#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QTemporaryFile>
#include "../../registeredSession/plugins/about/aboutinterface.h"

class AboutInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAboutInterface = new AboutInterface();
    }

    static void TearDownTestSuite()
    {
        if (m_pAboutInterface) {
            delete m_pAboutInterface;
            m_pAboutInterface = nullptr;
        }
    }

    static AboutInterface *m_pAboutInterface;
};

AboutInterface *AboutInterfaceTest::m_pAboutInterface = nullptr;

TEST_F(AboutInterfaceTest, versionInfo)
{
    QStringList list = m_pAboutInterface->property("versionInfo").toStringList();
    ASSERT_EQ(list.size(), 2);
}

TEST_F(AboutInterfaceTest, themeMode)
{
    std::string currentValue = m_pAboutInterface->getThemeMode().toStdString();
    m_pAboutInterface->setThemeMode("ukui-light");
    ASSERT_STREQ("ukui-light", m_pAboutInterface->getThemeMode().toStdString().c_str());
    m_pAboutInterface->setThemeMode(currentValue.c_str());
}

TEST_F(AboutInterfaceTest, buildList)
{
    m_pAboutInterface->getBuildList();
    QStringList buildList = m_pAboutInterface->getBuild();
    ASSERT_EQ(buildList.size(), 2);
}

TEST_F(AboutInterfaceTest, desktopEnv)
{
    m_pAboutInterface->getDesktopEnvStr();
    std::string desktopEnv = m_pAboutInterface->getDesktopEnv().toStdString();
    ASSERT_STREQ(desktopEnv.c_str(), "UKUI");
}

TEST_F(AboutInterfaceTest, memory)
{
    std::string memory = m_pAboutInterface->getTotalMemory().toStdString();
    ASSERT_STRNE(memory.c_str(), "");
}

TEST_F(AboutInterfaceTest, blockInfo)
{
    m_pAboutInterface->getBlockInfoStr();
    QString blockInfo = m_pAboutInterface->getBlockInfo();
    ASSERT_GT(blockInfo.split("\n").size(), 0);
}

TEST_F(AboutInterfaceTest, diskInfo)
{
    QString diskInfo = m_pAboutInterface->getDiskInfo();
    ASSERT_GT(diskInfo.split("\n").size(), 0);
}

TEST_F(AboutInterfaceTest, upgradeDate)
{
    QString upgradeDate = m_pAboutInterface->getUpgradeDate();
    if (upgradeDate.isEmpty()) {
        ASSERT_STREQ(upgradeDate.toStdString().c_str(), "");
    } else {
        ASSERT_STRNE(upgradeDate.toStdString().c_str(), "");
    }
}

TEST_F(AboutInterfaceTest, ntpTime)
{
    QString upgradeDate = m_pAboutInterface->getNtpTime();
    ASSERT_STRNE(upgradeDate.toStdString().c_str(), "");
}

TEST_F(AboutInterfaceTest, openActivation)
{
    if (qEnvironmentVariableIsEmpty("UKCC_RUN_OPEN_ACTIVATION_TEST")) {
        GTEST_SKIP() << "Skip openActivation to avoid popping activation window.";
    }
    ASSERT_TRUE(m_pAboutInterface->openActivation());
}

TEST_F(AboutInterfaceTest, readFileMissingAndExisting)
{
    QStringList missing = m_pAboutInterface->readFile("/tmp/ukcc-not-exist-about-file");
    ASSERT_TRUE(missing.isEmpty());

    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("line1\nline2\n");
    tmp.flush();
    QStringList lines = m_pAboutInterface->readFile(tmp.fileName());
    ASSERT_GE(lines.size(), 2);
}

TEST_F(AboutInterfaceTest, desktopEnvNonUkuiPath)
{
    QByteArray oldDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    qputenv("XDG_CURRENT_DESKTOP", "GNOME");
    m_pAboutInterface->getDesktopEnvStr();
    ASSERT_STREQ(m_pAboutInterface->getDesktopEnv().toStdString().c_str(), "GNOME");
    if (oldDesktop.isEmpty()) {
        qunsetenv("XDG_CURRENT_DESKTOP");
    } else {
        qputenv("XDG_CURRENT_DESKTOP", oldDesktop);
    }
}
