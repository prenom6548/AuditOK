#include "app-db-manager.h"

using namespace UkuiSearch;

namespace {
int insertCalls = 0;
int updateCalls = 0;
int deleteCalls = 0;
int localeCalls = 0;
int launchCalls = 0;
int favoritesCalls = 0;
int topCalls = 0;
int refreshCalls = 0;
int symlinkCalls = 0;
int valueSetCalls = 0;
}

QMutex AppDBManager::s_mutex;

extern "C" void resetAppDBManagerStubCalls()
{
    insertCalls = 0;
    updateCalls = 0;
    deleteCalls = 0;
    localeCalls = 0;
    launchCalls = 0;
    favoritesCalls = 0;
    topCalls = 0;
    refreshCalls = 0;
    symlinkCalls = 0;
    valueSetCalls = 0;
}

extern "C" int appDBManagerStubInsertCalls() { return insertCalls; }
extern "C" int appDBManagerStubUpdateCalls() { return updateCalls; }
extern "C" int appDBManagerStubDeleteCalls() { return deleteCalls; }
extern "C" int appDBManagerStubLocaleCalls() { return localeCalls; }
extern "C" int appDBManagerStubLaunchCalls() { return launchCalls; }
extern "C" int appDBManagerStubFavoritesCalls() { return favoritesCalls; }
extern "C" int appDBManagerStubTopCalls() { return topCalls; }
extern "C" int appDBManagerStubRefreshCalls() { return refreshCalls; }
extern "C" int appDBManagerStubSymlinkCalls() { return symlinkCalls; }
extern "C" int appDBManagerStubValueSetCalls() { return valueSetCalls; }

AppDBManager *AppDBManager::getInstance()
{
    static AppDBManager *instance = new AppDBManager;
    return instance;
}

bool AppDBManager::handleDBItemInsert(const QString &)
{
    insertCalls++;
    return true;
}

bool AppDBManager::handleDBItemUpdate(const QString &)
{
    updateCalls++;
    return true;
}

bool AppDBManager::handleDBItemDelete(const QString &)
{
    deleteCalls++;
    return true;
}

bool AppDBManager::handleLocaleDataUpdate(const QString &)
{
    localeCalls++;
    return true;
}

bool AppDBManager::handleLaunchTimesUpdate(const QString &, int)
{
    launchCalls++;
    return true;
}

bool AppDBManager::handleFavoritesStateUpdate(const QString &, const uint)
{
    favoritesCalls++;
    return true;
}

bool AppDBManager::handleTopStateUpdate(const QString &, const uint)
{
    topCalls++;
    return true;
}

bool AppDBManager::handleDataBaseRefresh(const QStringList &, bool)
{
    refreshCalls++;
    return true;
}

bool AppDBManager::handleSdkSymlink(const QString &)
{
    symlinkCalls++;
    return true;
}

bool AppDBManager::handleValueSet(const ApplicationInfoMap)
{
    valueSetCalls++;
    return true;
}

QString AppDBManager::tranPidToDesktopFp(uint)
{
    return {};
}

QString AppDBManager::desktopFilePathFromName(const QString &)
{
    return {};
}

QString AppDBManager::tranWinIdToDesktopFilePath(const QDBusVariant &)
{
    return {};
}

void AppDBManager::insertDBItem(const QString &)
{
}

void AppDBManager::updateDBItem(const QString &)
{
}

void AppDBManager::deleteDBItem(const QString &)
{
}

void AppDBManager::updateLocaleData(const QString &)
{
}

void AppDBManager::updateLaunchTimes(const QString &)
{
}

void AppDBManager::updateFavoritesState(const QString &, uint)
{
}

void AppDBManager::updateTopState(const QString &, uint)
{
}

void AppDBManager::updateSdkSymLink(const QString &)
{
}

void AppDBManager::setValue(const ApplicationInfoMap &)
{
}

void AppDBManager::handleAppLaunched(QString)
{
}

void AppDBManager::run()
{
}

AppDBManager::AppDBManager(QObject *parent)
    : QThread(parent)
{
}

AppDBManager::~AppDBManager()
{
}

ApplicationDirWatcherHelper::ApplicationDirWatcherHelper(FileSystemWatcher *watcher, QObject *parent)
    : QObject(parent),
      m_watcher(watcher)
{
}

ApplicationDirWatcherHelper::~ApplicationDirWatcherHelper()
{
}

void ApplicationDirWatcherHelper::addPath(const QString &)
{
}
