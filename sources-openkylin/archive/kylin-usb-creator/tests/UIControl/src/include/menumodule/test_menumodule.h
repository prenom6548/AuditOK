#ifndef TEST_MENUMODULE_H
#define TEST_MENUMODULE_H

#include <QObject>
#include <QtTest>
#include "../../../UIControl/src/include/menumodule.h"

class TestMenuModule : public QObject
{
    Q_OBJECT

public:
    TestMenuModule();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 构造函数测试
    void testConstructor();
    void testConstructorWithWayland();
    void testConstructorWithDBusFailure();
    void testConstructorWithDBusReplyType1();
    void testConstructorWithDBusReplyType2();
    void testConstructorWithXDGSessionTypeWayland();
    void testConstructorWithXDGSessionTypeNotWayland();
    void testConstructorWithDBusConnectSuccess();

    // initAction函数测试
    void testInitAction();
    void testInitActionWithThemeMenu();
    void testInitActionUIComponents();

    // fontSizeInit函数测试
    void testFontSizeInit();
    void testFontSizeInitWithSchema();
    void testFontSizeInitWithoutSchema();
    void testFontSizeInitWithValidSize();
    void testFontSizeInitWithInvalidSize();
    void testFontSizeChangedSignal();

    // themeUpdate函数测试
    void testThemeUpdate();
    void testThemeUpdateOld();

    // setStyleByThemeGsetting函数测试
    void testSetStyleByThemeGsetting();
    void testSetStyleByThemeGsettingDark();
    void testSetStyleByThemeGsettingLight();
    void testSetStyleByThemeGsettingBlack();

    // initGsetting函数测试
    void testInitGsetting();
    void testInitGsettingOld();
    void testInitGsettingWithSchema();
    void testInitGsettingWithoutSchema();
    void testInitGsettingWithSchemaNotInstalled();

    // refreshThemeBySystemConf函数测试
    void testRefreshThemeBySystemConf();
    void testRefreshThemeBySystemConfOld();
    void testRefreshThemeBySystemConfDark();
    void testRefreshThemeBySystemConfLight();
    void testRefreshThemeBySystemConfBlack();

    // setThemeDark/Light函数测试
    void testSetThemeDark();
    void testSetThemeLight();

    // triggerMenu函数测试
    void testTriggerMenu();
    void testTriggerMenuQuit();
    void testTriggerMenuQuitActual();
    void testTriggerMenuAbout();
    void testTriggerMenuHelp();
    void testTriggerMenuOther();

    // triggerThemeMenu函数测试
    void testTriggerThemeMenuOld();
    void testTriggerThemeMenuLight();
    void testTriggerThemeMenuDark();
    void testTriggerThemeMenuAuto();
    void testTriggerThemeMenuWithNullGSetting();

    // aboutAction/helpAction函数测试
    void testAboutAction();
    void testHelpAction();
    void testHelpActionWithIpcDbusNull();
    void testHelpActionWithDaemonRunning();
    void testHelpActionWithDaemonNotRunning();

    // uiModeChange函数测试
    void testUiModeChange();
    void testUiModeChangeTabletModeTrue();
    void testUiModeChangeTabletModeFalse();
    void testUiModeChangeWithAboutWindow();
    void testUiModeChangeWithoutAboutWindow();
    void testUiModeChangeWithAboutWindowNotNull();
    void testUiModeChangeWithAboutWindowMinimized();

    // limitTextEditTextCount函数测试
    void testLimitTextEditTextCount();
    void testLimitTextEditTextCountWithinLimit();
    void testLimitTextEditTextCountExceedLimit();
    void testLimitTextEditTextCountExactLimit();
    void testLimitTextEditTextCountEmptyText();
    void testLimitTextEditTextCountWithSpecialCharacters();
    void testLimitTextEditTextCountWithUnicodeCharacters();
    void testLimitTextEditTextCountWithVeryLongText();
    void testSlotLimitTextEditTextCount();
    void testSlotLimitTextEditTextCountWithText();

    // dealSetFontSize函数测试
    void testDealSetFontSize();
    void testDealSetFontSizePositive();
    void testDealSetFontSizeZero();
    void testDealSetFontSizeNegative();

    // dealSystemGsettingChange函数测试
    void testDealSystemGsettingChange();
    void testDealSystemGsettingChangeOld();
    void testDealSystemGsettingChangeStyleName();
    void testDealSystemGsettingChangeStyleNameWithSignal();
    void testDealSystemGsettingChangeIcon();
    void testDealSystemGsettingChangeIconNameWithSignal();
    void testDealSystemGsettingChangeIconWithSignal();
    void testDealSystemGsettingChangeOther();
    void testDealSystemGsettingChangeBothBranches();
    void testDealSystemGsettingChangeBothConditions();

    // keyPressEvent函数测试
    void testKeyPressEvent();
    void testKeyPressEventF1();
    void testKeyPressEventOther();

    // 其他函数测试
    void testIsDarkTheme();
    void testSetThemeStyle();
    void testSuccessbackAction();
    void testInitAbout();
    void testFontSizeInitOld();
    void testSetStyleByThemeGsettingOld();
    void testInitSuccessBackDialog();

private:
    menuModule* m_menuModule;
};

#endif // TEST_MENUMODULE_H