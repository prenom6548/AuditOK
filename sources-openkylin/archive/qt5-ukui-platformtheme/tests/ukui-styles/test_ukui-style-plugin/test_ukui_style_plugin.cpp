/*
 * Coverage for ukui-style-plugin.cpp via QPluginLoader (no source changes).
 * Offscreen: only instantiate plugin and call create(); no settings signal pumping.
 */

#include <QtTest>
#include <QPluginLoader>
#include <QStyle>
#include <QStylePlugin>
#include <QStyleFactory>
#include <QMetaObject>
#include <QPalette>
#include <QApplication>

#include "settings/ukui-style-settings.h"
#include "settings/black-list.h"

#ifndef UKUI_STYLE_PLUGIN_PATH
#define UKUI_STYLE_PLUGIN_PATH ""
#endif

class TestUkuiStylePlugin : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLoadPluginAndCreateStyles();
    void testCreateWithSkipTheme();
    void testCreateVariousStyleKeys();
    void testPluginBlackListCreatePath();
    void testPluginMetaObjectSlots();
    void testPluginCreateAllFactoryKeys();
    void testPluginTableModeWithUkuiStyle();
    void testPluginSystemPaletteBranches();
};

void TestUkuiStylePlugin::testLoadPluginAndCreateStyles()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QVERIFY2(!pluginPath.isEmpty(), "UKUI_STYLE_PLUGIN_PATH not defined at build time");

    QPluginLoader loader(pluginPath);
    QVERIFY2(loader.load(), qPrintable(loader.errorString()));

    auto *plugin = qobject_cast<QStylePlugin *>(loader.instance());
    QVERIFY(plugin != nullptr);

    const QStringList keys = QStringList({
        QStringLiteral("ukui-config"),
        QStringLiteral("ukui"),
        QStringLiteral("fusion"),
    });
    for (const QString &key : keys) {
        QStyle *style = plugin->create(key);
        if (style)
            delete style;
    }

    QStyle *configStyle = plugin->create(QStringLiteral("ukui-config"));
    QVERIFY(configStyle != nullptr);
    delete configStyle;

    if (QStyleFactory::keys().contains(QStringLiteral("Fusion"), Qt::CaseInsensitive)) {
        QStyle *fusion = plugin->create(QStringLiteral("Fusion"));
        if (fusion)
            delete fusion;
    }
}

void TestUkuiStylePlugin::testCreateWithSkipTheme()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    auto *plugin = qobject_cast<QStylePlugin *>(loader.instance());
    QVERIFY(plugin != nullptr);

    qApp->setProperty("skipUKUITheme", true);
    QStyle *skipped = plugin->create(QStringLiteral("ukui-config"));
    QVERIFY(skipped != nullptr);
    delete skipped;
    qApp->setProperty("skipUKUITheme", false);

    Q_UNUSED(UKUIStyleSettings::globalInstance());
}

void TestUkuiStylePlugin::testCreateVariousStyleKeys()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    auto *plugin = qobject_cast<QStylePlugin *>(loader.instance());
    QVERIFY(plugin != nullptr);

    const QStringList keys = {
        QStringLiteral("ukui-config"),
        QStringLiteral("ukui"),
        QStringLiteral("ukui-light"),
        QStringLiteral("ukui-dark"),
        QStringLiteral("Fusion"),
        QStringLiteral("Windows"),
    };
    for (const QString &key : keys) {
        if (QStyle *style = plugin->create(key))
            delete style;
    }

    qApp->setApplicationName(QStringLiteral("kylin-software-center.py"));
    if (QStyle *blacklisted = plugin->create(QStringLiteral("ukui-config")))
        delete blacklisted;
    qApp->setApplicationName(QStringLiteral("test_ukui_style_plugin"));
}

void TestUkuiStylePlugin::testPluginBlackListCreatePath()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    auto *plugin = qobject_cast<QStylePlugin *>(loader.instance());
    QVERIFY(plugin != nullptr);
    QVERIFY(!blackAppList().isEmpty());

    const QString savedName = qApp->applicationName();
    for (const QString &appName : blackAppList().mid(0, 5)) {
        qApp->setApplicationName(appName);
        if (QStyle *style = plugin->create(QStringLiteral("ukui-config")))
            delete style;
    }
    qApp->setApplicationName(savedName);

    qApp->setProperty("skipUKUITheme", true);
    if (QStyle *skipped = plugin->create(QStringLiteral("ukui")))
        delete skipped;
    qApp->setProperty("skipUKUITheme", false);
}

void TestUkuiStylePlugin::testPluginMetaObjectSlots()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    QObject *pluginObj = loader.instance();
    QVERIFY(pluginObj != nullptr);

    QMetaObject::invokeMethod(pluginObj, "onSystemPaletteChanged", Qt::DirectConnection);
    QMetaObject::invokeMethod(pluginObj, "tableModeChanged", Qt::DirectConnection, Q_ARG(bool, true));
    QMetaObject::invokeMethod(pluginObj, "tableModeChanged", Qt::DirectConnection, Q_ARG(bool, false));
}

void TestUkuiStylePlugin::testPluginCreateAllFactoryKeys()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    auto *plugin = qobject_cast<QStylePlugin *>(loader.instance());
    QVERIFY(plugin != nullptr);

    qApp->setProperty("customStyleName", QStringLiteral("ukui-light"));
    if (QStyle *light = plugin->create(QStringLiteral("ukui-config")))
        delete light;
    qApp->setProperty("customStyleName", QStringLiteral("ukui-dark"));
    if (QStyle *dark = plugin->create(QStringLiteral("ukui-config")))
        delete dark;
    qApp->setProperty("customStyleName", QVariant());

    for (const QString &key : QStyleFactory::keys()) {
        if (QStyle *style = plugin->create(key))
            delete style;
    }
}

void TestUkuiStylePlugin::testPluginTableModeWithUkuiStyle()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    QObject *pluginObj = loader.instance();
    QVERIFY(pluginObj != nullptr);

    qApp->setProperty("customStyleName", QStringLiteral("ukui-light"));
    QMetaObject::invokeMethod(pluginObj, "tableModeChanged", Qt::DirectConnection, Q_ARG(bool, true));
    qApp->setProperty("customStyleName", QStringLiteral("ukui-dark"));
    QMetaObject::invokeMethod(pluginObj, "tableModeChanged", Qt::DirectConnection, Q_ARG(bool, true));
    qApp->setProperty("customStyleName", QVariant());

    const QString savedName = qApp->applicationName();
    if (!blackAppList().isEmpty()) {
        qApp->setApplicationName(blackAppList().first());
        QMetaObject::invokeMethod(pluginObj, "tableModeChanged", Qt::DirectConnection, Q_ARG(bool, true));
    }
    qApp->setApplicationName(savedName);
}

void TestUkuiStylePlugin::testPluginSystemPaletteBranches()
{
    const QString pluginPath = QString::fromUtf8(UKUI_STYLE_PLUGIN_PATH);
    QPluginLoader loader(pluginPath);
    if (!loader.load())
        QSKIP(qPrintable(loader.errorString()));

    QObject *pluginObj = loader.instance();
    QVERIFY(pluginObj != nullptr);

    UKUIStyleSettings *settings = UKUIStyleSettings::globalInstance();
    if (!settings)
        QSKIP("UKUIStyleSettings unavailable");

    const QVariant savedUsePalette = settings->get("useSystemPalette");
    const QVariant savedPalette = settings->get("systemPalette");

    settings->set("useSystemPalette", true);
    settings->set("systemPalette", QVariant());
    QMetaObject::invokeMethod(pluginObj, "onSystemPaletteChanged", Qt::DirectConnection);

    QPalette appPalette = qApp->palette();
    settings->set("systemPalette", QVariant::fromValue(appPalette));
    QMetaObject::invokeMethod(pluginObj, "onSystemPaletteChanged", Qt::DirectConnection);

    settings->set("useSystemPalette", false);
    QMetaObject::invokeMethod(pluginObj, "onSystemPaletteChanged", Qt::DirectConnection);

    settings->set("useSystemPalette", savedUsePalette);
    if (savedPalette.isValid())
        settings->set("systemPalette", savedPalette);
}

QTEST_MAIN(TestUkuiStylePlugin)
#include "test_ukui_style_plugin.moc"
