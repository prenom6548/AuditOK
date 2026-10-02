/*
 * Unit tests for ukui-style-conf-settings.cpp (offscreen, no event loop).
 */

#include <QtTest>
#include <glib.h>

#include "settings/ukui-style-conf-settings.h"

extern QString qtify_name(const char *name);
extern char *unqtify_name(const QString &name);

class TestUkuiStyleConfSettings : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQtifyAndUnqtifyName();
    void testSettingsLifecycle();
    void testMultipleSchemaInstances();
    void testKeysAndGetWhenBackendAvailable();
};

void TestUkuiStyleConfSettings::testQtifyAndUnqtifyName()
{
    QCOMPARE(qtify_name("hello-world"), QStringLiteral("helloWorld"));
    QCOMPARE(qtify_name("style-name"), QStringLiteral("styleName"));
    QCOMPARE(qtify_name(""), QString());

    char *ckey = unqtify_name(QStringLiteral("widgetThemeName"));
    QVERIFY(ckey != nullptr);
    QCOMPARE(QString::fromUtf8(ckey), QStringLiteral("widget-theme-name"));
    g_free(ckey);

    char *empty = unqtify_name(QString());
    QVERIFY(empty != nullptr);
    QCOMPARE(QString::fromUtf8(empty), QString());
    g_free(empty);
}

void TestUkuiStyleConfSettings::testSettingsLifecycle()
{
    const QByteArray schemaId("org.ukui.style");
    const QByteArray badSchema("no.such.schema.id");

    (void)ukuiStyleConfSettings::isSettingsAvailable(schemaId);
    QVERIFY(!ukuiStyleConfSettings::isSettingsAvailable(badSchema));

    ukuiStyleConfSettings settings(schemaId);
    // trySet/reset guard null KSettings; avoid keys()/get() when CI has no kysdk-conf backend.
    settings.reset(QStringLiteral("styleName"));
    settings.set(QStringLiteral("styleName"), QVariant(QStringLiteral("ukui")));
    const bool ok = settings.trySet(QStringLiteral("styleName"), QVariant(QStringLiteral("ukui-config")));
    Q_UNUSED(ok);
}

void TestUkuiStyleConfSettings::testMultipleSchemaInstances()
{
    ukuiStyleConfSettings a(QByteArray("org.ukui.style"));
    ukuiStyleConfSettings b(QByteArray("org.ukui.peripherals-mouse"));
    a.reset(QStringLiteral("styleName"));
    b.reset(QStringLiteral("cursorTheme"));
    a.set(QStringLiteral("iconThemeName"), QVariant(QStringLiteral("ukui-icon-theme-default")));
    b.trySet(QStringLiteral("cursorTheme"), QVariant(QStringLiteral("default")));
}

void TestUkuiStyleConfSettings::testKeysAndGetWhenBackendAvailable()
{
    const QByteArray schemaId("org.ukui.style");
    if (!ukuiStyleConfSettings::isSettingsAvailable(schemaId))
        QSKIP("kysdk-conf backend not available");

    ukuiStyleConfSettings settings(schemaId);
    const QStringList keys = settings.keys();
    if (!keys.isEmpty()) {
        (void)settings.get(keys.first());
        (void)settings.get(QStringLiteral("styleName"));
    }
    settings.trySet(QStringLiteral("menuTransparency"), QVariant(90));
    settings.reset(QStringLiteral("menuTransparency"));
}

QTEST_MAIN(TestUkuiStyleConfSettings)
#include "test_ukui_style_conf_settings.moc"
