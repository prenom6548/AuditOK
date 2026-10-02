#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../src/lock-dialog/backenddbushelper.h"
#include "../../src/common/definetypes.h"
#include "../../src/agreementinfo.h"
#include "stubext.h"

using namespace stub_ext;

class BackendDbusHelperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
        QString sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
        m_pBackendDbusHelperDbus = new BackendDbusHelper(sessionDbus, SS_DBUS_PATH, QDBusConnection::sessionBus());
        if (!m_pBackendDbusHelperDbus->isValid()) {
            delete m_pBackendDbusHelperDbus;
            m_pBackendDbusHelperDbus = new BackendDbusHelper(SS_DBUS_SERVICE, SS_DBUS_PATH, QDBusConnection::sessionBus());
        }
    }

    static void TearDownTestSuite()
    {
        delete m_pBackendDbusHelperDbus;
        m_pBackendDbusHelperDbus = nullptr;
    }

    static BackendDbusHelper *m_pBackendDbusHelperDbus;
};

BackendDbusHelper *BackendDbusHelperTest::m_pBackendDbusHelperDbus = nullptr;

TEST_F(BackendDbusHelperTest, getUsersInfo)
{
    QList<UserInfoPtr> value = m_pBackendDbusHelperDbus->getUsersInfo();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, getAgreementInfo)
{
    AgreementInfoPtr value = m_pBackendDbusHelperDbus->getAgreementInfo();
//    ASSERT_EQ(value, NULL);
}

TEST_F(BackendDbusHelperTest, getSessionsInfo)
{
    QList<QString> value = m_pBackendDbusHelperDbus->getSessionsInfo();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, getBatteryArgs)
{
    QStringList value = m_pBackendDbusHelperDbus->getBatteryArgs();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, getSleepLockcheck)
{
    QStringList value = m_pBackendDbusHelperDbus->getSleepLockcheck();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, isSessionActive)
{
    bool value = m_pBackendDbusHelperDbus->isSessionActive();
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, switchToUser)
{
    int value = m_pBackendDbusHelperDbus->switchToUser(getenv("USER"));
    ASSERT_EQ(value, 0);
}

TEST_F(BackendDbusHelperTest, setCurrentUser)
{
    bool value = m_pBackendDbusHelperDbus->setCurrentUser(getenv("USER"));
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, setPowerManager)
{
    bool value = m_pBackendDbusHelperDbus->setPowerManager("LockScreen");
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, sendPassword)
{
    QByteArray decryptText;
    bool value = m_pBackendDbusHelperDbus->sendPassword(getenv("USER"), decryptText);
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, getBatteryIconName)
{
    QString value = m_pBackendDbusHelperDbus->getBatteryIconName();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, getIsBattery)
{
    bool value = m_pBackendDbusHelperDbus->getIsBattery();
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, getCurTabletMode)
{
    bool value = m_pBackendDbusHelperDbus->getCurTabletMode();
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, lockStateChanged)
{
    bool value = m_pBackendDbusHelperDbus->lockStateChanged(false, false);
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, pamIsInAuthentication)
{
    m_pBackendDbusHelperDbus->startSession();
    m_pBackendDbusHelperDbus->pamAuthenticate(getenv("USER"));
    m_pBackendDbusHelperDbus->bioStartAuth(getuid(), 32);
    m_pBackendDbusHelperDbus->bioStopAuth();
    m_pBackendDbusHelperDbus->pamRespond("");
    m_pBackendDbusHelperDbus->pamAuthenticateCancel();
    bool value = m_pBackendDbusHelperDbus->pamIsInAuthentication();
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, pamIsAuthenticated)
{
    bool value = m_pBackendDbusHelperDbus->pamIsAuthenticated();
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, pamAuthenticateUser)
{
    QString value = m_pBackendDbusHelperDbus->pamAuthenticateUser();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, bioGetAvailableDevices)
{
    QList<DeviceInfo> value = m_pBackendDbusHelperDbus->bioGetAvailableDevices(101);
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, bioGetDisabledDevices)
{
    QList<int> value = m_pBackendDbusHelperDbus->bioGetDisabledDevices(101);
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, bioGetBioAuthState)
{
    int value = m_pBackendDbusHelperDbus->bioGetBioAuthState();
    ASSERT_EQ(value, 0);
}

TEST_F(BackendDbusHelperTest, bioGetCurBioInfo)
{
    DeviceInfo bioValue1 = m_pBackendDbusHelperDbus->bioGetCurBioInfo();
    DeviceInfo bioValue2 = m_pBackendDbusHelperDbus->bioFindDeviceById(getuid(), 101);
    DeviceInfo bioValue3 = m_pBackendDbusHelperDbus->bioFindDeviceByName(getuid(), "uru4000");
    QString value = m_pBackendDbusHelperDbus->bioGetDefaultDevice(getuid(), getenv("USER"));
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, getCurrentSession)
{
    QString value = m_pBackendDbusHelperDbus->getCurrentSession();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, setCurrentSession)
{
    bool value = m_pBackendDbusHelperDbus->setCurrentSession("testSession");
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, onUpdateInformation)
{
    QList<int> list{LOCK_CMD_ID_USERINFO_CHANGED, LOCK_CMD_ID_CURRENT_USER_CHANGED,
                   LOCK_CMD_ID_CURRENT_SESSION_CHANGED, LOCK_CMD_ID_LOGIN1_REQ_LOCK,
                   LOCK_CMD_ID_LOGIN1_REQ_UNLOCK, LOCK_CMD_ID_LOGIN1_PREPAREFORSLEEP,
                   LOCK_CMD_ID_LOGIN1_SESSION_ACTIVE_CHANGED, LOCK_CMD_ID_GSETTINGS_LOCKSCREEN_CONF_CHANGED,
                   LOCK_CMD_ID_GSETTINGS_SCREENSAVER_CONF_CHANGED, LOCK_CMD_ID_GSETTINGS_POWERMANAGER_CONF_CHANGED,
                   LOCK_CMD_ID_GSETTINGS_MATEBG_CONF_CHANGED, LOCK_CMD_ID_GSETTINGS_UKCCPLUGINS_CONF_CHANGED,
                   LOCK_CMD_ID_GSETTINGS_THEMESTYLE_CONF_CHANGED, LOCK_CMD_ID_GSETTINGS_SESSION_CONF_CHANGED,
                   LOCK_CMD_ID_LOGIN1_BLOCKINHIBITED_CHANGED, LOCK_CMD_ID_GSETTINGS_KEYBOARD_CONF_CHANGED,
                   LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE, LOCK_CMD_ID_PAMAUTH_SHOWPROMPT, LOCK_CMD_ID_PAMAUTH_AUTH_COMPLETED,
                   LOCK_CMD_ID_GSETTINGS_USD_MEDIAKEYS_CONF_CHANGED, LOCK_CMD_ID_GSETTINGS_USD_MEDIA_STATE_KEYS_CONF_CHANGED,
                   LOCK_CMD_ID_UPOWER_BATTERY_STATUS, LOCK_CMD_ID_UPOWER_BATTERY, LOCK_CMD_ID_BIOAUTH_SERVICE_STATUS_CHANGED,
                   LOCK_CMD_ID_BIOAUTH_DEVICE_CHANGED, LOCK_CMD_ID_BIOAUTH_SHOW_MESSAGE, LOCK_CMD_ID_BIOAUTH_AUTHSTATE_CHANGED,
                   LOCK_CMD_ID_BIOAUTH_FRAME_DATA, LOCK_CMD_ID_BIOAUTH_COMPLETE, LOCK_CMD_ID_TABLET_MODE_CHANGED};
    for (int i = 0; i < list.count(); i++) {
        QJsonObject retObj;
        retObj["CmdId"] = list.at(i);
        m_pBackendDbusHelperDbus->onUpdateInformation(QString(QJsonDocument(retObj).toJson()));
    }
}

TEST_F(BackendDbusHelperTest, usdExternalDoAction)
{
    bool value = m_pBackendDbusHelperDbus->usdExternalDoAction(VOLUME_DOWN_KEY);
    m_pBackendDbusHelperDbus->usdExternalDoAction(VOLUME_UP_KEY);
    ASSERT_EQ(value, true);
}

TEST_F(BackendDbusHelperTest, getDefaultAuthUser)
{
    QString value = m_pBackendDbusHelperDbus->getDefaultAuthUser();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, getCurrentUser)
{
    QString value = m_pBackendDbusHelperDbus->getCurrentUser();
    ASSERT_EQ(value.isEmpty(), true);
}

TEST_F(BackendDbusHelperTest, getScreenSaverConf)
{
    bool value = m_pBackendDbusHelperDbus->getScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE).toBool();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, setScreenSaverConf)
{
    bool oldvalue = m_pBackendDbusHelperDbus->getScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE).toBool();
    bool value = m_pBackendDbusHelperDbus->setScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE, !oldvalue);
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE, oldvalue);
}

TEST_F(BackendDbusHelperTest, getLockScreenConf)
{
    bool value = m_pBackendDbusHelperDbus->getLockScreenConf(KEY_LOCK_ENABLED).toBool();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, setLockScreenConf)
{
    bool oldvalue = m_pBackendDbusHelperDbus->getLockScreenConf(KEY_LOCK_ENABLED).toBool();
    bool value = m_pBackendDbusHelperDbus->setLockScreenConf(KEY_LOCK_ENABLED, !oldvalue);
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setLockScreenConf(KEY_LOCK_ENABLED, oldvalue);
}

TEST_F(BackendDbusHelperTest, getShutdownLockcheck)
{
    QStringList value = m_pBackendDbusHelperDbus->getShutdownLockcheck();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, getSaverThemes)
{
    QStringList value = m_pBackendDbusHelperDbus->getSaverThemes();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerConf)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerConf(KEY_LOCK_BLANKSCREEN).toBool();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, setPowerManagerConf)
{
    bool oldvalue = m_pBackendDbusHelperDbus->getPowerManagerConf(KEY_LOCK_BLANKSCREEN).toBool();
    bool value = m_pBackendDbusHelperDbus->setPowerManagerConf(KEY_LOCK_BLANKSCREEN, !oldvalue);
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setPowerManagerConf(KEY_LOCK_BLANKSCREEN, oldvalue);
}

TEST_F(BackendDbusHelperTest, getMateBgConf)
{
    QString value = m_pBackendDbusHelperDbus->getMateBgConf(KEY_PICTURE_FILENAME).toString();
    ASSERT_EQ(value.isEmpty(), false);
}

TEST_F(BackendDbusHelperTest, setMateBgConf)
{
    QString oldvalue = m_pBackendDbusHelperDbus->getMateBgConf(KEY_PICTURE_FILENAME).toString();
    return ;

    bool value = m_pBackendDbusHelperDbus->setMateBgConf("picture-filename", "sss");
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setMateBgConf("picture-filename", oldvalue);
}

TEST_F(BackendDbusHelperTest, getUkccPluginsConf)
{
    QString value = m_pBackendDbusHelperDbus->getUkccPluginsConf(KEY_DATE).toString();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, setUkccPluginsConf)
{
    QString oldvalue = m_pBackendDbusHelperDbus->getUkccPluginsConf(KEY_DATE).toString();
    bool value = m_pBackendDbusHelperDbus->setUkccPluginsConf(KEY_DATE, "sss");
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setUkccPluginsConf(KEY_DATE, oldvalue);
}

TEST_F(BackendDbusHelperTest, getThemeStyleConf)
{
    QString value = m_pBackendDbusHelperDbus->getThemeStyleConf(KEY_THEME_COLOR).toString();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, setThemeStyleConf)
{
    QString oldvalue = m_pBackendDbusHelperDbus->getThemeStyleConf(KEY_THEME_COLOR).toString();
    bool value = m_pBackendDbusHelperDbus->setThemeStyleConf(KEY_THEME_COLOR, "sss");
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setThemeStyleConf(KEY_THEME_COLOR, oldvalue);
}

TEST_F(BackendDbusHelperTest, getSessionConf)
{
    int value = m_pBackendDbusHelperDbus->getSessionConf(KEY_SESSION_IDLE).toInt();
    ASSERT_EQ(value, 60);
}

TEST_F(BackendDbusHelperTest, setSessionConf)
{
    int oldvalue = m_pBackendDbusHelperDbus->getSessionConf(KEY_SESSION_IDLE).toInt();
    bool value = m_pBackendDbusHelperDbus->setSessionConf(KEY_SESSION_IDLE, 30);
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setSessionConf(KEY_SESSION_IDLE, oldvalue);
}

TEST_F(BackendDbusHelperTest, getKeyboardConf)
{
    bool value = m_pBackendDbusHelperDbus->getKeyboardConf(KEY_CAPSLOCK_STATUS).toBool();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, setKeyboardConf)
{
    bool oldvalue = m_pBackendDbusHelperDbus->getKeyboardConf(KEY_CAPSLOCK_STATUS).toBool();
    bool value = m_pBackendDbusHelperDbus->setKeyboardConf(KEY_CAPSLOCK_STATUS, !oldvalue);
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setKeyboardConf(KEY_CAPSLOCK_STATUS, oldvalue);
}

TEST_F(BackendDbusHelperTest, getUsdMediaStateKeys)
{
    int value = m_pBackendDbusHelperDbus->getUsdMediaStateKeys(KEY_RFKILL_STATE).toInt();
    ASSERT_EQ(value, 60);
}

TEST_F(BackendDbusHelperTest, setUsdMediaStateKeys)
{
    int oldvalue = m_pBackendDbusHelperDbus->getUsdMediaStateKeys(KEY_RFKILL_STATE).toInt();
    bool value = m_pBackendDbusHelperDbus->setUsdMediaStateKeys(KEY_RFKILL_STATE, 0);
    ASSERT_EQ(value, false);
    m_pBackendDbusHelperDbus->setUsdMediaStateKeys(KEY_RFKILL_STATE, oldvalue);
}

TEST_F(BackendDbusHelperTest, getUsdMediaKeys)
{
    QString value = m_pBackendDbusHelperDbus->getUsdMediaKeys(KEY_WINDOW_SCREENSHOT).toString();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanSuspend)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanSuspend();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanReboot)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanReboot();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanPowerOff)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanPowerOff();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanSwitchUser)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanSwitchUser();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanHibernate)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanHibernate();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanLogout)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanLogout();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPowerManagerCanLockScreen)
{
    bool value = m_pBackendDbusHelperDbus->getPowerManagerCanLockScreen();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, checkSystemUpgrade)
{
    bool value = m_pBackendDbusHelperDbus->checkSystemUpgrade();
    ASSERT_EQ(value, false);
}

TEST_F(BackendDbusHelperTest, getPublicEncrypt)
{
    QString value = m_pBackendDbusHelperDbus->getPublicEncrypt();
    ASSERT_EQ(value.isEmpty(), false);
}
