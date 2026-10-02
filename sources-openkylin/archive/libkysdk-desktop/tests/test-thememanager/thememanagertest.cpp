#include <QtTest/QtTest>
#include "../../kysdk-thememanager/src/kthememanager.h"

class KThemeManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void testGetUkccGlobalTheme();
    void testGetSupportSystemStyle();
    void testGetSystemThemeColor();
    void testGetSystemFont();
    void testGetSystemFontSize();
    void testSetSystemStyle();
    void testSetSystemFont();
    void testSetSystemFontSize();
    void testSetSystemThemeColor();
    void testSetPanelSpecialEffect();
    void testSetTransparency();

private:
    kdk::KThemeManager themeManager;
};

void KThemeManagerTest::testGetUkccGlobalTheme()
{
    QString theme = themeManager.getUkccGlobalTheme();
    QVERIFY(!theme.isEmpty());
}

void KThemeManagerTest::testGetSupportSystemStyle()
{
    QStringList styles = themeManager.getSupportSystemStyle();
    QVERIFY(!styles.isEmpty());
}

void KThemeManagerTest::testGetSystemThemeColor()
{
    QString color = themeManager.getSystemThemeColor();
    QVERIFY(!color.isEmpty());
}

void KThemeManagerTest::testGetSystemFont()
{
    QString font = themeManager.getSystemFont();
    QVERIFY(!font.isEmpty());
}

void KThemeManagerTest::testGetSystemFontSize()
{
    int fontSize = themeManager.getSystemFontSize();
    QVERIFY(fontSize > 0);
}

void KThemeManagerTest::testSetSystemStyle()
{
    bool success = themeManager.setSystemStyle("ukui-default");
    QVERIFY(success);
}

void KThemeManagerTest::testSetSystemFont()
{
    bool success = themeManager.setSystemFont("Noto Sans CJK SC");
    QVERIFY(success);
}

void KThemeManagerTest::testSetSystemFontSize()
{
    bool success = themeManager.setSystemFontSize(12);
    QVERIFY(success);
}

void KThemeManagerTest::testSetSystemThemeColor()
{
    bool success = themeManager.setSystemThemeColor("daybreakBlue");
    QVERIFY(success);
}

void KThemeManagerTest::testSetPanelSpecialEffect()
{
    bool success = themeManager.setPanelSpecialEffect(true);
    QVERIFY(success);
}

void KThemeManagerTest::testSetTransparency()
{
    bool success = themeManager.setTransparency(0.70);
    QVERIFY(success);
}

QTEST_APPLESS_MAIN(KThemeManagerTest)

#include "thememanagertest.moc"