#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "pluginmanage/plugin-manager.h"
#include "pluginmanage/search-plugin-manager.h"
#include "pluginmanage/search-task-plugin-manager.h"
#undef private

using namespace UkuiSearch;

namespace {
int g_externalSearchRegisterCalls = 0;
int g_taskPluginPathRegisterCalls = 0;
bool g_externalSearchRegisterResult = false;
}

SearchPluginManager *SearchPluginManager::getInstance()
{
    return reinterpret_cast<SearchPluginManager *>(quintptr(0x1));
}

bool SearchPluginManager::registerExternalPlugin(SearchPluginIface *plugin, const QString &path)
{
    Q_UNUSED(plugin)
    Q_UNUSED(path)
    ++g_externalSearchRegisterCalls;
    return g_externalSearchRegisterResult;
}

SearchTaskPluginManager *SearchTaskPluginManager::getInstance()
{
    return reinterpret_cast<SearchTaskPluginManager *>(quintptr(0x1));
}

void SearchTaskPluginManager::registerPluginPath(const QString &customType, const QString &pluginPath)
{
    Q_UNUSED(customType)
    Q_UNUSED(pluginPath)
    ++g_taskPluginPathRegisterCalls;
}

class PluginManagerWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void constructorHandlesEmptyAndUnsupportedPluginDirectory();
    void constructorLoadsSupportedPluginsAndRejectsWrongVersion();
    void constructorKeepsSuccessfulExternalSearchPluginRegistration();
    void singletonInitAndGetInstanceReturnSameObject();

private:
    void resetPluginDir();
};

void PluginManagerWhiteboxTest::initTestCase()
{
    resetPluginDir();
}

void PluginManagerWhiteboxTest::resetPluginDir()
{
    QDir dir(QStringLiteral(PLUGIN_INSTALL_DIRS));
    if (dir.exists()) {
        dir.removeRecursively();
    }
    QVERIFY(QDir().mkpath(QStringLiteral(PLUGIN_INSTALL_DIRS)));
}

void PluginManagerWhiteboxTest::constructorHandlesEmptyAndUnsupportedPluginDirectory()
{
    resetPluginDir();
    {
        auto *manager = new PluginManager;
        QVERIFY(manager != nullptr);
        delete manager;
    }

    QFile invalidPlugin(QStringLiteral(PLUGIN_INSTALL_DIRS) + QStringLiteral("/not-a-plugin.so"));
    QVERIFY(invalidPlugin.open(QIODevice::WriteOnly | QIODevice::Truncate));
    invalidPlugin.write("not a qt plugin");
    invalidPlugin.close();

    g_externalSearchRegisterCalls = 0;
    g_taskPluginPathRegisterCalls = 0;
    auto *manager = new PluginManager;
    QVERIFY(manager != nullptr);
    QCOMPARE(g_externalSearchRegisterCalls, 0);
    QCOMPARE(g_taskPluginPathRegisterCalls, 0);
    delete manager;
}

void PluginManagerWhiteboxTest::constructorLoadsSupportedPluginsAndRejectsWrongVersion()
{
    resetPluginDir();

    const QString fixtureDir = QStringLiteral(PLUGIN_MANAGER_FIXTURE_DIR);
    const QString pluginDir = QStringLiteral(PLUGIN_INSTALL_DIRS);
    const QStringList fixtureNames{
        QStringLiteral("fixture-search-plugin.so"),
        QStringLiteral("fixture-task-plugin.so"),
        QStringLiteral("fixture-wrong-version-search-plugin.so"),
        QStringLiteral("fixture-unsupported-plugin.so")
    };
    for (const QString &fixtureName : fixtureNames) {
        const QString fixturePath = fixtureDir + QStringLiteral("/") + fixtureName;
        const QString pluginPath = pluginDir + QStringLiteral("/") + fixtureName;
        QVERIFY(QFile::exists(fixturePath));
        QVERIFY(QFile::copy(fixturePath, pluginPath));
        QVERIFY(QFile::exists(pluginPath));
    }

    g_externalSearchRegisterCalls = 0;
    g_taskPluginPathRegisterCalls = 0;
    g_externalSearchRegisterResult = false;
    auto *manager = new PluginManager;
    QVERIFY(manager != nullptr);
    QCOMPARE(g_externalSearchRegisterCalls, 1);
    QCOMPARE(g_taskPluginPathRegisterCalls, 1);
    delete manager;
}

void PluginManagerWhiteboxTest::constructorKeepsSuccessfulExternalSearchPluginRegistration()
{
    resetPluginDir();

    const QString fixtureDir = QStringLiteral(PLUGIN_MANAGER_FIXTURE_DIR);
    const QString pluginDir = QStringLiteral(PLUGIN_INSTALL_DIRS);
    const QString fixtureName = QStringLiteral("fixture-search-plugin.so");
    const QString fixturePath = fixtureDir + QStringLiteral("/") + fixtureName;
    const QString pluginPath = pluginDir + QStringLiteral("/") + fixtureName;
    QVERIFY(QFile::exists(fixturePath));
    QVERIFY(QFile::copy(fixturePath, pluginPath));

    g_externalSearchRegisterCalls = 0;
    g_taskPluginPathRegisterCalls = 0;
    g_externalSearchRegisterResult = true;
    auto *manager = new PluginManager;
    QVERIFY(manager != nullptr);
    QCOMPARE(g_externalSearchRegisterCalls, 1);
    QCOMPARE(g_taskPluginPathRegisterCalls, 0);
    delete manager;
    g_externalSearchRegisterResult = false;
}

void PluginManagerWhiteboxTest::singletonInitAndGetInstanceReturnSameObject()
{
    resetPluginDir();
    PluginManager::init();
    PluginManager *first = PluginManager::getInstance();
    PluginManager *second = PluginManager::getInstance();
    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
}

QTEST_MAIN(PluginManagerWhiteboxTest)

#include "plugin-manager-whitebox-test.moc"
