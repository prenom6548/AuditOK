#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "pluginmanage/search-plugin-manager.h"
#undef private
#include "search-plugin-manager-stubs/search-plugin-test-stub.h"

using namespace UkuiSearch;

class SearchPluginManagerWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanup();
    void defaultConstructionRegistersEnabledPluginsInOrder();
    void initOrderSettingsRefreshesStaleConfigVersion();
    void registerPluginRespectsDisabledBuiltInConfig();
    void registerPluginRejectsDuplicateAndUnknownConfig();
    void registerExternalPluginCreatesConfigAndMovesWebLast();
    void registerExternalPluginRespectsDisabledExistingConfig();
    void registerExternalPluginRejectsDuplicateRegistration();
    void unregisterAndReregisterBuiltInPlugin();
    void unregisterExternalPluginByPathUpdatesConfig();
    void changePluginPosCoversInvalidUnchangedAndShiftedCases();
    void changePluginPosCoversAliasesAndInvalidStoredOrder();
    void stringFacadeCoversContentWebExternalAndRegisteredBranches();
    void pointerFacadeCoversExternalPathAndMissingBranches();
    void reRegisterPluginStringCoversBuiltInAndUnknownBranches();
    void reRegisterPluginStringCoversExternalFixtureBranches();
    void reRegisterPluginPointerRejectsMissingConfig();
    void singletonAndDestructorAreCovered();

private:
    static QString settingsFile();
    static QStringList pluginNames(const QList<PluginInfo> &infos);
    static PluginInfo findInfo(const QList<PluginInfo> &infos, const QString &name);

    QTemporaryDir m_configHome;
};

void SearchPluginManagerWhiteboxTest::initTestCase()
{
    QVERIFY(m_configHome.isValid());
    qputenv("HOME", m_configHome.path().toUtf8());
    qputenv("XDG_CONFIG_HOME", (m_configHome.path() + QStringLiteral("/.config")).toUtf8());
}

void SearchPluginManagerWhiteboxTest::cleanup()
{
    QFile::remove(settingsFile());
}

QString SearchPluginManagerWhiteboxTest::settingsFile()
{
    return QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-search/ukui-search-plugin-order.conf");
}

namespace {
QString fixturePath(const QString &name)
{
    return QStringLiteral(SEARCH_PLUGIN_MANAGER_FIXTURE_DIR) + QLatin1Char('/') + name;
}
}

QStringList SearchPluginManagerWhiteboxTest::pluginNames(const QList<PluginInfo> &infos)
{
    QStringList names;
    for (const PluginInfo &info : infos) {
        names << info.name();
    }
    return names;
}

PluginInfo SearchPluginManagerWhiteboxTest::findInfo(const QList<PluginInfo> &infos, const QString &name)
{
    for (const PluginInfo &info : infos) {
        if (info.name() == name) {
            return info;
        }
    }
    return PluginInfo(QString(), false, 0, false, false);
}

void SearchPluginManagerWhiteboxTest::defaultConstructionRegistersEnabledPluginsInOrder()
{
    auto *manager = new SearchPluginManager;
    const QList<PluginInfo> infos = manager->getPluginIds();

    QCOMPARE(infos.size(), 8);
    QVERIFY(pluginNames(infos).contains(QStringLiteral("Applications Search")));
    QVERIFY(pluginNames(infos).contains(QStringLiteral("Web Page")));
    QCOMPARE(findInfo(infos, QStringLiteral("Ai Search")).order(), 1);
    QCOMPARE(findInfo(infos, QStringLiteral("Web Page")).order(), 8);
    QVERIFY(findInfo(infos, QStringLiteral("Web Page")).isFixed());
    QVERIFY(manager->getPlugin(QStringLiteral("Applications Search")) != nullptr);
    QCOMPARE(manager->getPlugin(QStringLiteral("Not Registered")), nullptr);
}

void SearchPluginManagerWhiteboxTest::initOrderSettingsRefreshesStaleConfigVersion()
{
    QSettings settings(settingsFile(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("PluginsInfo"));
    settings.setValue(QStringLiteral("Version"), QStringLiteral("0.9"));
    settings.beginGroup(QStringLiteral("Web Page"));
    settings.setValue(QStringLiteral("Order"), 3);
    settings.setValue(QStringLiteral("Enable"), false);
    settings.setValue(QStringLiteral("External"), true);
    settings.setValue(QStringLiteral("FixPos"), false);
    settings.endGroup();
    settings.endGroup();
    settings.sync();

    auto *manager = new SearchPluginManager;
    const QList<PluginInfo> infos = manager->getPluginIds();
    const PluginInfo web = findInfo(infos, QStringLiteral("Web Page"));

    QVERIFY(manager->getPlugin(QStringLiteral("Web Page")) != nullptr);
    QVERIFY(web.isEnable());
    QVERIFY(web.isFixed());
    QVERIFY(!web.isExternal());
    QCOMPARE(web.order(), 8);

    QSettings refreshed(settingsFile(), QSettings::IniFormat);
    refreshed.beginGroup(QStringLiteral("PluginsInfo"));
    QCOMPARE(refreshed.value(QStringLiteral("Version")).toString(), QStringLiteral("1.0"));
    refreshed.endGroup();

}

void SearchPluginManagerWhiteboxTest::registerPluginRespectsDisabledBuiltInConfig()
{
    auto *manager = new SearchPluginManager;

    QVERIFY(manager->unregisterPlugin(QStringLiteral("Settings")));
    QVERIFY(!manager->registerPlugin(new SearchPluginTestStub(QStringLiteral("Settings Search"))));
    QVERIFY(manager->getPlugin(QStringLiteral("Settings Search")) == nullptr);

}

void SearchPluginManagerWhiteboxTest::registerPluginRejectsDuplicateAndUnknownConfig()
{
    auto *manager = new SearchPluginManager;

    QVERIFY(!manager->registerPlugin(new SearchPluginTestStub(QStringLiteral("Applications Search"))));
    QVERIFY(!manager->registerPlugin(new SearchPluginTestStub(QStringLiteral("Missing Search"))));
    QVERIFY(manager->getPlugin(QStringLiteral("Missing Search")) == nullptr);

}

void SearchPluginManagerWhiteboxTest::registerExternalPluginCreatesConfigAndMovesWebLast()
{
    auto *manager = new SearchPluginManager;
    const QString path = m_configHome.path() + QStringLiteral("/libexternal-search.so");
    auto *plugin = new SearchPluginTestStub(QStringLiteral("External Search"));

    QVERIFY(manager->registerExternalPlugin(plugin, path));
    QCOMPARE(manager->getPlugin(QStringLiteral("External Search")), plugin);

    const QList<PluginInfo> infos = manager->getPluginIds();
    const PluginInfo external = findInfo(infos, QStringLiteral("External Search"));
    const PluginInfo web = findInfo(infos, QStringLiteral("Web Page"));
    QVERIFY(external.isEnable());
    QVERIFY(external.isExternal());
    QCOMPARE(external.path(), path);
    QVERIFY(external.order() > 0);
    QVERIFY(web.order() > external.order());

}

void SearchPluginManagerWhiteboxTest::registerExternalPluginRespectsDisabledExistingConfig()
{
    QSettings settings(settingsFile(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("PluginsInfo"));
    settings.beginGroup(QStringLiteral("Dormant Search"));
    settings.setValue(QStringLiteral("Order"), 4);
    settings.setValue(QStringLiteral("Enable"), false);
    settings.setValue(QStringLiteral("External"), true);
    settings.setValue(QStringLiteral("FixPos"), false);
    settings.setValue(QStringLiteral("Path"), m_configHome.path() + QStringLiteral("/dormant.so"));
    settings.endGroup();
    settings.endGroup();
    settings.sync();

    auto *manager = new SearchPluginManager;
    const QString path = m_configHome.path() + QStringLiteral("/dormant.so");
    auto *plugin = new SearchPluginTestStub(QStringLiteral("Dormant Search"));

    QVERIFY(!manager->registerExternalPlugin(plugin, path));
    QCOMPARE(manager->getPlugin(QStringLiteral("Dormant Search")), nullptr);

    QSettings stored(settingsFile(), QSettings::IniFormat);
    stored.beginGroup(QStringLiteral("ExternalPlugins"));
    QCOMPARE(stored.value(path).toString(), QStringLiteral("Dormant Search"));
    stored.endGroup();

}

void SearchPluginManagerWhiteboxTest::registerExternalPluginRejectsDuplicateRegistration()
{
    auto *manager = new SearchPluginManager;
    const QString path = m_configHome.path() + QStringLiteral("/libduplicate.so");

    QVERIFY(manager->registerExternalPlugin(new SearchPluginTestStub(QStringLiteral("Duplicate External Search")), path));
    QVERIFY(!manager->registerExternalPlugin(new SearchPluginTestStub(QStringLiteral("Duplicate External Search")), path));

}

void SearchPluginManagerWhiteboxTest::unregisterAndReregisterBuiltInPlugin()
{
    auto *manager = new SearchPluginManager;
    QSignalSpy unregisteredSpy(manager, &SearchPluginManager::unregistered);
    QSignalSpy reregisteredSpy(manager, &SearchPluginManager::reRegistered);

    QVERIFY(manager->unregisterPlugin(QStringLiteral("Note")));
    QCOMPARE(unregisteredSpy.count(), 1);
    QVERIFY(!findInfo(manager->getPluginIds(), QStringLiteral("Note Search")).isEnable());

    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Note")));

    QVERIFY(manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("Note Search")), false));
    QCOMPARE(reregisteredSpy.count(), 1);
    QVERIFY(manager->getPlugin(QStringLiteral("Note Search")) != nullptr);

    QVERIFY(!manager->unregisterPlugin(QStringLiteral("DefinitelyMissing")));

}

void SearchPluginManagerWhiteboxTest::unregisterExternalPluginByPathUpdatesConfig()
{
    auto *manager = new SearchPluginManager;
    QSignalSpy unregisteredSpy(manager, &SearchPluginManager::unregistered);
    const QString path = m_configHome.path() + QStringLiteral("/libexternal-removable.so");

    QVERIFY(manager->registerExternalPlugin(new SearchPluginTestStub(QStringLiteral("Removable Search")), path));
    QVERIFY(manager->unregisterPlugin(path));
    QCOMPARE(unregisteredSpy.count(), 1);
    QVERIFY(!findInfo(manager->getPluginIds(), QStringLiteral("Removable Search")).isEnable());
    QVERIFY(manager->getPlugin(QStringLiteral("Removable Search")) == nullptr);

}

void SearchPluginManagerWhiteboxTest::changePluginPosCoversInvalidUnchangedAndShiftedCases()
{
    auto *manager = new SearchPluginManager;
    QSignalSpy changedSpy(manager, &SearchPluginManager::changePos);

    QVERIFY(!manager->changePluginPos(QStringLiteral("File"), 0));
    QVERIFY(!manager->changePluginPos(QStringLiteral("Missing"), 2));
    QVERIFY(!manager->changePluginPos(QStringLiteral("File"), findInfo(manager->getPluginIds(), QStringLiteral("File Search")).order()));

    QVERIFY(!manager->changePluginPos(QStringLiteral("File"), 2));
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.takeFirst().at(0).toString(), QStringLiteral("File Search"));

    const QList<PluginInfo> infos = manager->getPluginIds();
    QCOMPARE(findInfo(infos, QStringLiteral("File Search")).order(), 2);
    QVERIFY(findInfo(infos, QStringLiteral("Applications Search")).order() > 2);

}

void SearchPluginManagerWhiteboxTest::changePluginPosCoversAliasesAndInvalidStoredOrder()
{
    auto *manager = new SearchPluginManager;
    QSignalSpy changedSpy(manager, &SearchPluginManager::changePos);

    QVERIFY(!manager->changePluginPos(QStringLiteral("Web"), 2));
    QVERIFY(!manager->changePluginPos(QStringLiteral("Content"), 6));
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0).at(0).toString(), QStringLiteral("Web Page"));

    QSettings broken(settingsFile(), QSettings::IniFormat);
    broken.beginGroup(QStringLiteral("PluginsInfo"));
    broken.beginGroup(QStringLiteral("Applications Search"));
    broken.setValue(QStringLiteral("Order"), 0);
    broken.endGroup();
    broken.endGroup();
    broken.sync();

    QVERIFY(!manager->changePluginPos(QStringLiteral("Applications"), 3));

}

void SearchPluginManagerWhiteboxTest::stringFacadeCoversContentWebExternalAndRegisteredBranches()
{
    auto *manager = new SearchPluginManager;
    QSignalSpy unregisteredSpy(manager, &SearchPluginManager::unregistered);
    QSignalSpy reregisteredSpy(manager, &SearchPluginManager::reRegistered);
    QSignalSpy changedSpy(manager, &SearchPluginManager::changePos);

    QVERIFY(manager->unregisterPlugin(QStringLiteral("Web")));
    QCOMPARE(unregisteredSpy.count(), 1);
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Web")));
    QVERIFY(manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("Web Page")), false));
    QVERIFY(manager->getPlugin(QStringLiteral("Web Page")) != nullptr);
    QCOMPARE(reregisteredSpy.count(), 1);

    QVERIFY(manager->unregisterPlugin(QStringLiteral("Content")));
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Content")));
    QVERIFY(manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("File Content Search")), false));
    QVERIFY(manager->getPlugin(QStringLiteral("File Content Search")) != nullptr);
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Content")));

    const QString path = m_configHome.path() + QStringLiteral("/libexternal-string.so");
    QVERIFY(manager->registerExternalPlugin(new SearchPluginTestStub(QStringLiteral("String External Search")), path));
    QVERIFY(!manager->changePluginPos(path, 2));
    QVERIFY(changedSpy.count() >= 1);
    QVERIFY(manager->unregisterPlugin(path));
    QVERIFY(manager->m_map.find(QStringLiteral("String External Search")) == manager->m_map.end());
    QVERIFY(!manager->reRegisterPlugin(path));
    QVERIFY(!manager->changePluginPos(path, 3));

}

void SearchPluginManagerWhiteboxTest::pointerFacadeCoversExternalPathAndMissingBranches()
{
    auto *manager = new SearchPluginManager;
    QSignalSpy unregisteredSpy(manager, &SearchPluginManager::unregistered);
    QSignalSpy reregisteredSpy(manager, &SearchPluginManager::reRegistered);

    QVERIFY(!manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("Pointer External Search")), true));

    const QString path = m_configHome.path() + QStringLiteral("/libpointer-external.so");
    QVERIFY(manager->registerExternalPlugin(new SearchPluginTestStub(QStringLiteral("Pointer External Search")), path));
    QVERIFY(!manager->unregisterPlugin(new SearchPluginTestStub(QStringLiteral("Pointer External Search")), true, QString()));
    QCOMPARE(unregisteredSpy.count(), 1);
    QVERIFY(!findInfo(manager->getPluginIds(), QStringLiteral("Pointer External Search")).isEnable());
    QVERIFY(manager->m_map.find(QStringLiteral("Pointer External Search")) == manager->m_map.end());

    QVERIFY(manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("Pointer External Search")), true, path));
    QCOMPARE(reregisteredSpy.count(), 1);
    QVERIFY(manager->getPlugin(QStringLiteral("Pointer External Search")) != nullptr);
    QVERIFY(!manager->unregisterPlugin(new SearchPluginTestStub(QStringLiteral("Not Registered Search")), false));

    QVERIFY(!manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("Pointer External Search")), true, path));
    QVERIFY(!manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("No Path External Search")), true));

    QVERIFY(manager->unregisterPlugin(new SearchPluginTestStub(QStringLiteral("Pointer External Search")), true, path));
    QVERIFY(manager->getPlugin(QStringLiteral("Pointer External Search")) == nullptr);

}

void SearchPluginManagerWhiteboxTest::reRegisterPluginStringCoversBuiltInAndUnknownBranches()
{
    auto *manager = new SearchPluginManager;

    QVERIFY(manager->unregisterPlugin(QStringLiteral("Applications")));
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Applications")));
    QVERIFY(manager->unregisterPlugin(QStringLiteral("Settings")));
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Settings")));
    QVERIFY(manager->unregisterPlugin(QStringLiteral("Dir")));
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("Dir")));
    QVERIFY(manager->unregisterPlugin(QStringLiteral("File")));
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("File")));
    QVERIFY(!manager->reRegisterPlugin(QStringLiteral("DefinitelyUnknown")));

}

void SearchPluginManagerWhiteboxTest::reRegisterPluginStringCoversExternalFixtureBranches()
{
    auto *manager = new SearchPluginManager;

    const QString validPath = fixturePath(QStringLiteral("fixture-search-plugin.so"));
    const QString wrongVersionPath = fixturePath(QStringLiteral("fixture-wrong-version-search-plugin.so"));
    const QString unsupportedPath = fixturePath(QStringLiteral("fixture-unsupported-plugin.so"));

    QVERIFY(QFileInfo::exists(validPath));
    QVERIFY(QFileInfo::exists(wrongVersionPath));
    QVERIFY(QFileInfo::exists(unsupportedPath));

    QVERIFY(manager->registerExternalPlugin(new SearchPluginTestStub(QStringLiteral("Fixture Search Plugin")), validPath));
    QVERIFY(manager->unregisterPlugin(validPath));
    QVERIFY(manager->reRegisterPlugin(validPath));
    QVERIFY(manager->getPlugin(QStringLiteral("Fixture Search Plugin")) != nullptr);
    QVERIFY(manager->unregisterPlugin(validPath));

    QSettings settings(settingsFile(), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("PluginsInfo"));
    settings.beginGroup(QStringLiteral("Wrong Version Search Plugin"));
    settings.setValue(QStringLiteral("Order"), 9);
    settings.setValue(QStringLiteral("Enable"), true);
    settings.setValue(QStringLiteral("External"), true);
    settings.setValue(QStringLiteral("FixPos"), false);
    settings.setValue(QStringLiteral("Path"), wrongVersionPath);
    settings.endGroup();
    settings.beginGroup(QStringLiteral("Unsupported Plugin"));
    settings.setValue(QStringLiteral("Order"), 10);
    settings.setValue(QStringLiteral("Enable"), true);
    settings.setValue(QStringLiteral("External"), true);
    settings.setValue(QStringLiteral("FixPos"), false);
    settings.setValue(QStringLiteral("Path"), unsupportedPath);
    settings.endGroup();
    settings.endGroup();
    settings.beginGroup(QStringLiteral("ExternalPlugins"));
    settings.setValue(wrongVersionPath, QStringLiteral("Wrong Version Search Plugin"));
    settings.setValue(unsupportedPath, QStringLiteral("Unsupported Plugin"));
    settings.endGroup();
    settings.sync();

    QVERIFY(!manager->reRegisterPlugin(wrongVersionPath));
    QVERIFY(manager->getPlugin(QStringLiteral("Wrong Version Search Plugin")) == nullptr);
    QVERIFY(!manager->reRegisterPlugin(unsupportedPath));
    QVERIFY(manager->getPlugin(QStringLiteral("Unsupported Plugin")) == nullptr);
}

void SearchPluginManagerWhiteboxTest::reRegisterPluginPointerRejectsMissingConfig()
{
    auto *manager = new SearchPluginManager;

    QVERIFY(!manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("Ghost Search")), false));

}

void SearchPluginManagerWhiteboxTest::singletonAndDestructorAreCovered()
{
    SearchPluginManager *first = SearchPluginManager::getInstance();
    SearchPluginManager *second = SearchPluginManager::getInstance();
    QCOMPARE(first, second);

    // Also test destructor with registered plugins
    auto *manager = new SearchPluginManager;
    manager->reRegisterPlugin(new SearchPluginTestStub(QStringLiteral("DestructTest")), false);
    delete manager;
}

QTEST_MAIN(SearchPluginManagerWhiteboxTest)

#include "search-plugin-manager-whitebox-test.moc"
