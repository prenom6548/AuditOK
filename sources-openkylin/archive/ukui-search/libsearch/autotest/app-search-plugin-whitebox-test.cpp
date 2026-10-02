#include <QtTest>

#include <QDir>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <memory>

#define private public
#include "appsearch/app-search-plugin.h"
#undef private

#include "app-search-plugin-stubs/security-control-config.h"
#include "app-search-plugin-stubs/global-settings.h"
#include "app-search-plugin-stubs/data-collecter.h"
#include "plugininterface/action-transmiter.h"

using namespace UkuiSearch;

namespace {

class FakeAppLauncher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.ProcessManager.AppLauncher")

public:
    QString lastPath;
    QString lastDesktopFile;
    QStringList lastArguments;
    int launchAppCalls = 0;
    int launchWithArgumentsCalls = 0;

public Q_SLOTS:
    void LaunchApp(const QString &path)
    {
        lastPath = path;
        ++launchAppCalls;
    }

    void LaunchAppWithArguments(const QString &desktopFile, const QStringList &arguments)
    {
        lastDesktopFile = desktopFile;
        lastArguments = arguments;
        ++launchWithArgumentsCalls;
    }
};

class FakeTaskManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.taskManager")

public:
    bool quickLauncherExists = false;
    QStringList addedPaths;

public Q_SLOTS:
    bool checkQuickLauncher(const QString &path) const
    {
        Q_UNUSED(path)
        return quickLauncherExists;
    }

    QDBusVariant addQuickLauncher(const QString &path)
    {
        addedPaths.append(path);
        return QDBusVariant(path);
    }
};

class FakeSoftwareCenter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.utiliface")

public:
    QStringList shownKeywords;

public Q_SLOTS:
    bool show_search_result(const QString &keyword)
    {
        shownKeywords.append(keyword);
        return true;
    }
};

class ScopedDbusRegistration
{
public:
    ScopedDbusRegistration(const QString &service,
                           const QString &path,
                           QObject *object,
                           const QDBusConnection &connection = QDBusConnection::sessionBus())
        : m_service(service),
          m_path(path),
          m_connection(connection)
    {
        m_serviceRegistered = m_connection.registerService(service);
        m_objectRegistered = m_connection.registerObject(path, object, QDBusConnection::ExportAllSlots);
    }

    ~ScopedDbusRegistration()
    {
        if (m_objectRegistered) {
            m_connection.unregisterObject(m_path);
        }
        if (m_serviceRegistered) {
            m_connection.unregisterService(m_service);
        }
    }

    bool isRegistered() const
    {
        return m_serviceRegistered && m_objectRegistered;
    }

private:
    QString m_service;
    QString m_path;
    QDBusConnection m_connection{QString()};
    bool m_serviceRegistered = false;
    bool m_objectRegistered = false;
};

bool sessionBusAvailable()
{
    return QDBusConnection::sessionBus().isConnected();
}
}

class AppSearchPluginWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void metadataActionsAndDetailPageBranches();
    void keywordSearchAndRunConvertQueuedResults();
    void detailPageShortNameThemeSignalAndActionLabels();
    void runAppliesSecurityBlackAndWhiteLists();
    void runConvertsOnlineResultsAndOpenInstallBranches();
    void openActionsCoverSafeFailureBranches();
    void shortcutBranchesCoverExistAndFailPaths();
    void installAndOpenBranchesCoverSoftwareCenterPaths();
};

static ResultItem makeAppResult(size_t searchId, const QString &desktopPath, const QString &name, int online = 0)
{
    SearchResultPropertyMap data;
    data.insert(SearchProperty::IsOnlineApplication, online);
    data.insert(SearchProperty::ApplicationDesktopPath, desktopPath);
    data.insert(SearchProperty::ApplicationLocalName, name);
    data.insert(SearchProperty::ApplicationIconName, QStringLiteral("application-x-desktop"));
    data.insert(SearchProperty::ApplicationDescription, QStringLiteral("description"));
    data.insert(SearchProperty::ApplicationPkgName, QStringLiteral("package-name"));
    return ResultItem(searchId, desktopPath, data);
}

static bool writeDesktopFile(const QString &path, const QString &exec)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QStringLiteral("[Desktop Entry]\nType=Application\nName=Demo\nExec=%1\n").arg(exec).toLocal8Bit());
    file.close();
    return true;
}

static bool writeDesktopFile(const QString &path)
{
    return writeDesktopFile(path, QStringLiteral("/bin/true"));
}

void AppSearchPluginWhiteboxTest::metadataActionsAndDetailPageBranches()
{
    AppSearchPlugin plugin;

    QCOMPARE(plugin.pluginType(), PluginInterface::PluginType::SearchPlugin);
    QCOMPARE(plugin.name(), QStringLiteral("Applications Search"));
    QCOMPARE(plugin.description(), QStringLiteral("Applications Search"));
    QCOMPARE(plugin.getPluginName(), QStringLiteral("Applications Search"));
    QVERIFY(plugin.isEnable());
    plugin.setEnable(false);
    QVERIFY(!plugin.isEnable());
    plugin.setEnable(true);
    QVERIFY(plugin.isEnable());

    QCOMPARE(plugin.getActioninfo(0).size(), 3);
    QCOMPARE(plugin.getActioninfo(1).size(), 1);
    QVERIFY(plugin.getActioninfo(9).isEmpty());

    SearchPluginIface::ResultInfo installed;
    installed.name = QStringLiteral("Installed Application With A Very Long Name");
    installed.actionKey = QStringLiteral("/tmp/installed.desktop");
    installed.type = 0;
    QWidget *installedPage = plugin.detailPage(installed);
    QVERIFY(installedPage != nullptr);
    QVERIFY(plugin.m_actionLabel1->isVisibleTo(installedPage));
    QVERIFY(plugin.m_actionLabel2->isVisibleTo(installedPage));
    QVERIFY(plugin.m_actionLabel3->isVisibleTo(installedPage));
    QVERIFY(!plugin.m_actionLabel4->isVisibleTo(installedPage));
    QVERIFY(!plugin.m_descLabel->isVisibleTo(installedPage));

    SearchPluginIface::ResultInfo online;
    online.name = QStringLiteral("Online Application");
    online.actionKey = QStringLiteral("online-package");
    online.type = 1;
    online.description.append({QStringLiteral("Application Description:"), QStringLiteral("Online description")});
    QWidget *onlinePage = plugin.detailPage(online);
    QVERIFY(onlinePage != nullptr);
    QVERIFY(!plugin.m_actionLabel1->isVisibleTo(onlinePage));
    QVERIFY(!plugin.m_actionLabel2->isVisibleTo(onlinePage));
    QVERIFY(!plugin.m_actionLabel3->isVisibleTo(onlinePage));
    QVERIFY(plugin.m_actionLabel4->isVisibleTo(onlinePage));
    QVERIFY(plugin.m_descLabel->isVisibleTo(onlinePage));
    QCOMPARE(plugin.m_currentActionKey, QStringLiteral("online-package"));
}

void AppSearchPluginWhiteboxTest::keywordSearchAndRunConvertQueuedResults()
{
    AppSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> output;

    const size_t searchId = plugin.KeywordSearch(QStringLiteral("calculator"), &output);
    QCOMPARE(searchId, size_t(1));
    QCOMPARE(plugin.m_appSearchTask->m_keywords, QStringList{QStringLiteral("calculator")});
    QVERIFY(plugin.m_appSearchTask->m_searchOnlineApps);
    QVERIFY(plugin.m_appSearchTask->m_resultProperties.contains(SearchProperty::ApplicationDesktopPath));
    plugin.stopSearch();

    plugin.m_searchResult = &output;
    plugin.m_appSearchResults->enqueue(makeAppResult(searchId, QStringLiteral("/tmp/calc.desktop"), QStringLiteral("Calculator")));
    plugin.start();
    QTRY_VERIFY_WITH_TIMEOUT(!output.isEmpty(), 2000);
    plugin.stopSearch();

    SearchPluginIface::ResultInfo result = output.tryDequeue();
    QCOMPARE(result.searchID, searchId);
    QCOMPARE(result.actionKey, QStringLiteral("/tmp/calc.desktop"));
    QCOMPARE(result.name, QStringLiteral("Calculator"));
    QCOMPARE(result.resourceType, QStringLiteral("app"));
    QCOMPARE(result.type, 0);
    QVERIFY(output.isEmpty());

    plugin.start();
    QTRY_VERIFY_WITH_TIMEOUT(plugin.isRunning(), 2000);
    DataQueue<SearchPluginIface::ResultInfo> runningOutput;
    const size_t secondSearchId = plugin.KeywordSearch(QStringLiteral("browser"), &runningOutput);
    QCOMPARE(secondSearchId, size_t(2));
    QCOMPARE(plugin.m_appSearchTask->m_keywords, QStringList{QStringLiteral("browser")});
    plugin.stopSearch();

    QVERIFY(plugin.m_appSearchTask->m_stopped);
}

void AppSearchPluginWhiteboxTest::detailPageShortNameThemeSignalAndActionLabels()
{
    QStandardPaths::setTestModeEnabled(true);
    AppSearchPlugin plugin;
    FakeAppLauncher appLauncher;
    FakeTaskManager taskManager;
    FakeSoftwareCenter softwareCenter;
    std::unique_ptr<ScopedDbusRegistration> launcherRegistration;
    std::unique_ptr<ScopedDbusRegistration> taskManagerRegistration;
    std::unique_ptr<ScopedDbusRegistration> softwareCenterRegistration;
    if (sessionBusAvailable()) {
        launcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                             QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                             &appLauncher));
        taskManagerRegistration.reset(new ScopedDbusRegistration(QStringLiteral("org.ukui.taskManager"),
                                                                 QStringLiteral("/taskManager"),
                                                                 &taskManager));
        softwareCenterRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.kylin.softwarecenter"),
                                                                    QStringLiteral("/com/kylin/softwarecenter"),
                                                                    &softwareCenter));
    }
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString desktopFile = dir.filePath(QStringLiteral("demo.desktop"));
    QVERIFY(writeDesktopFile(desktopFile));

    SearchPluginIface::ResultInfo info;
    info.name = QStringLiteral("Demo");
    info.actionKey = desktopFile;
    info.type = 0;
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::green);
    info.icon = QIcon(pixmap);

    QWidget *page = plugin.detailPage(info);
    QVERIFY(page != nullptr);
    QCOMPARE(plugin.m_nameLabel->toolTip(), QString());
    QVERIFY(plugin.m_iconLabel->pixmap() != nullptr);

    UkuiSearch::GlobalSettings::getInstance().valueChanged(UkuiSearch::ICON_THEME_KEY, {});
    QVERIFY(plugin.m_iconLabel->pixmap() != nullptr);

    DataCollecter::calls = 0;
    plugin.m_currentActionKey = desktopFile;
    QString launchAction = plugin.m_currentActionKey;
    plugin.m_actionLabel1->actionTriggered(launchAction);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("launch"));
    if (launcherRegistration && launcherRegistration->isRegistered()) {
        QCOMPARE(appLauncher.launchAppCalls, 1);
        QCOMPARE(appLauncher.lastPath, desktopFile);
    }

    QString desktopAction = plugin.m_currentActionKey;
    plugin.m_actionLabel2->actionTriggered(desktopAction);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("addDesktopShortcut"));

    QString panelAction = plugin.m_currentActionKey;
    plugin.m_actionLabel3->actionTriggered(panelAction);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("addPanelShortcut"));
    if (taskManagerRegistration && taskManagerRegistration->isRegistered()) {
        QCOMPARE(taskManager.addedPaths.size(), 1);
        QCOMPARE(taskManager.addedPaths.constFirst(), desktopFile);
    }

    plugin.m_currentActionKey = QStringLiteral("demo-package");
    QString installAction = plugin.m_currentActionKey;
    plugin.m_actionLabel4->actionTriggered(installAction);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("installAppAction"));
    if (softwareCenterRegistration && softwareCenterRegistration->isRegistered()) {
        const QStringList expectedShownKeywords{QStringLiteral("demo-package"), QStringLiteral("demo-package")};
        QCOMPARE(softwareCenter.shownKeywords, expectedShownKeywords);
    }
    QVERIFY(DataCollecter::calls >= 4);
}

void AppSearchPluginWhiteboxTest::runAppliesSecurityBlackAndWhiteLists()
{
    AppSearchPlugin plugin;
    DataQueue<SearchPluginIface::ResultInfo> output;
    plugin.m_searchResult = &output;

    SecurityControlConfig::instance().setPolicy(true, true, false,
                                                QStringList{QStringLiteral("blocked.desktop")},
                                                {});
    plugin.m_appSearchResults->enqueue(makeAppResult(7, QStringLiteral("/tmp/blocked.desktop"), QStringLiteral("Blocked")));
    plugin.m_appSearchResults->enqueue(makeAppResult(7, QStringLiteral("/tmp/allowed.desktop"), QStringLiteral("Allowed")));
    plugin.start();
    QTRY_VERIFY_WITH_TIMEOUT(!output.isEmpty(), 2000);
    plugin.stopSearch();
    SearchPluginIface::ResultInfo allowed = output.tryDequeue();
    QCOMPARE(allowed.actionKey, QStringLiteral("/tmp/allowed.desktop"));
    QVERIFY(output.isEmpty());

    SecurityControlConfig::instance().setPolicy(true, false, true,
                                                {},
                                                QStringList{QStringLiteral("allowed.desktop")});
    plugin.m_appSearchResults->enqueue(makeAppResult(8, QStringLiteral("/tmp/other.desktop"), QStringLiteral("Other")));
    plugin.m_appSearchResults->enqueue(makeAppResult(8, QStringLiteral("/tmp/allowed.desktop"), QStringLiteral("Allowed")));
    plugin.start();
    QTRY_VERIFY_WITH_TIMEOUT(!output.isEmpty(), 2000);
    plugin.stopSearch();
    allowed = output.tryDequeue();
    QCOMPARE(allowed.actionKey, QStringLiteral("/tmp/allowed.desktop"));
    QVERIFY(output.isEmpty());

    SecurityControlConfig::instance().setPolicy(false, false, false, {}, {});
}

void AppSearchPluginWhiteboxTest::runConvertsOnlineResultsAndOpenInstallBranches()
{
    QStandardPaths::setTestModeEnabled(true);
    AppSearchPlugin plugin;
    FakeAppLauncher appLauncher;
    FakeSoftwareCenter softwareCenter;
    std::unique_ptr<ScopedDbusRegistration> launcherRegistration;
    std::unique_ptr<ScopedDbusRegistration> softwareCenterRegistration;
    if (sessionBusAvailable()) {
        launcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                             QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                             &appLauncher));
        softwareCenterRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.kylin.softwarecenter"),
                                                                    QStringLiteral("/com/kylin/softwarecenter"),
                                                                    &softwareCenter));
    }
    DataQueue<SearchPluginIface::ResultInfo> output;
    plugin.m_searchResult = &output;

    SecurityControlConfig::instance().setPolicy(true, true, true,
                                                QStringList{QStringLiteral("blocked.desktop")},
                                                QStringList{QStringLiteral("never.desktop")});
    plugin.m_appSearchResults->enqueue(makeAppResult(9, QStringLiteral("/tmp/online.desktop"), QStringLiteral("Online App"), 1));
    plugin.start();
    QTRY_VERIFY_WITH_TIMEOUT(!output.isEmpty(), 2000);
    plugin.stopSearch();

    SearchPluginIface::ResultInfo online = output.tryDequeue();
    QCOMPARE(online.searchID, size_t(9));
    QCOMPARE(online.type, 1);
    QCOMPARE(online.actionKey, QStringLiteral("package-name"));
    QCOMPARE(online.resourceType, QStringLiteral("app"));
    QVERIFY(!online.description.isEmpty());
    QVERIFY(output.isEmpty());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    qputenv("HOME", dir.path().toLocal8Bit());
    const QString desktopFile = dir.filePath(QStringLiteral("launchable.desktop"));
    QVERIFY(writeDesktopFile(desktopFile));

    const QString desktopDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QVERIFY(QDir().mkpath(desktopDir));
    const QString copiedDesktopFile = desktopDir + QLatin1Char('/') + QFileInfo(desktopFile).fileName();
    QFile::remove(copiedDesktopFile);
    QVERIFY(plugin.addDesktopShortcut(desktopFile));
    QVERIFY(QFileInfo::exists(copiedDesktopFile));

    plugin.openAction(0, desktopFile, 0);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("launch"));
    if (launcherRegistration && launcherRegistration->isRegistered()) {
        QCOMPARE(appLauncher.launchAppCalls, 1);
        QCOMPARE(appLauncher.lastPath, desktopFile);
    }
    plugin.openAction(0, QStringLiteral("package-name"), 1);
    QCOMPARE(DataCollecter::lastButton, QStringLiteral("installAppAction"));
    if (softwareCenterRegistration && softwareCenterRegistration->isRegistered()) {
        const QStringList expectedShownKeywords{QStringLiteral("package-name"), QStringLiteral("package-name")};
        QCOMPARE(softwareCenter.shownKeywords, expectedShownKeywords);
    }

    SecurityControlConfig::instance().setPolicy(false, false, false, {}, {});
}

void AppSearchPluginWhiteboxTest::openActionsCoverSafeFailureBranches()
{
    AppSearchPlugin plugin;
    FakeAppLauncher appLauncher;
    std::unique_ptr<ScopedDbusRegistration> launcherRegistration;
    if (sessionBusAvailable()) {
        launcherRegistration.reset(new ScopedDbusRegistration(QStringLiteral("com.kylin.ProcessManager"),
                                                             QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                                             &appLauncher));
    }
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString missingDesktop = dir.filePath(QStringLiteral("missing.desktop"));
    QVERIFY(!plugin.addDesktopShortcut(missingDesktop));
    QVERIFY(!plugin.addPanelShortcut(missingDesktop));
    plugin.openAction(1, missingDesktop, 0);
    plugin.openAction(2, missingDesktop, 0);

    plugin.openAction(99, missingDesktop, 0);
    plugin.openAction(0, missingDesktop, 99);

    const QString screenshotDesktop = dir.filePath(QStringLiteral("kylin-screenshot.desktop"));
    QVERIFY(writeDesktopFile(screenshotDesktop));
    QSignalSpy spy(ActionTransmiter::getInstance(), &ActionTransmiter::hideUIAction);
    plugin.launch(screenshotDesktop);
    QCOMPARE(spy.count(), 1);
    if (launcherRegistration && launcherRegistration->isRegistered()) {
        QCOMPARE(appLauncher.launchAppCalls, 1);
        QCOMPARE(appLauncher.lastPath, screenshotDesktop);
    }

    const QString failingDesktop = dir.filePath(QStringLiteral("failing.desktop"));
    QVERIFY(writeDesktopFile(failingDesktop, QStringLiteral("/definitely/missing-command")));
    plugin.openAction(0, failingDesktop, 0);

    const QByteArray oldPath = qgetenv("PATH");
    qputenv("PATH", QByteArray());
    plugin.openAction(0, QStringLiteral("missing-package"), 1);
    qputenv("PATH", oldPath);
}

void AppSearchPluginWhiteboxTest::shortcutBranchesCoverExistAndFailPaths()
{
    AppSearchPlugin plugin;
    FakeTaskManager taskManager;
    std::unique_ptr<ScopedDbusRegistration> taskManagerReg;
    if (sessionBusAvailable()) {
        taskManagerReg.reset(new ScopedDbusRegistration(QStringLiteral("org.ukui.taskManager"),
                                                        QStringLiteral("/taskManager"),
                                                        &taskManager));
    }
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString existingDesktop = dir.filePath(QStringLiteral("existing.desktop"));
    QVERIFY(writeDesktopFile(existingDesktop));
    taskManager.quickLauncherExists = true;
    plugin.addPanelShortcut(existingDesktop);

    const QString newDesktop = dir.filePath(QStringLiteral("new.desktop"));
    QVERIFY(writeDesktopFile(newDesktop));
    taskManager.quickLauncherExists = false;
    plugin.addPanelShortcut(newDesktop);
}

void AppSearchPluginWhiteboxTest::installAndOpenBranchesCoverSoftwareCenterPaths()
{
    AppSearchPlugin plugin;
    FakeSoftwareCenter softwareCenter;
    std::unique_ptr<ScopedDbusRegistration> swReg;
    if (sessionBusAvailable()) {
        swReg.reset(new ScopedDbusRegistration(QStringLiteral("com.kylin.softwarecenter"),
                                                QStringLiteral("/com/kylin/softwarecenter"),
                                                &softwareCenter));
    }
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString desktop = dir.filePath(QStringLiteral("test.desktop"));
    QVERIFY(writeDesktopFile(desktop));

    plugin.openAction(1, desktop, 0);
    plugin.openAction(2, desktop, 0);
    plugin.openAction(1, QStringLiteral("non-existent-package"), 1);
}

QTEST_MAIN(AppSearchPluginWhiteboxTest)
#include "app-search-plugin-whitebox-test.moc"
