/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "../../../src/data-entity.h"
#include "../../../src/extension/context-menu-extension.h"
#include "../../../src/extension/menu/app-menu-plugin.h"
#include "../../../src/libappdata/app-category-model.h"
#include "../../../src/libappdata/app-database-interface.h"
#include "../../../src/libappdata/basic-app-filter-model.h"
#include "../../../src/libappdata/recently-installed-model.h"
#include "../../../src/utils/app-manager.h"
#include "../../../src/utils/security-function-control.h"
#include "../../libappdata/fakes/settings.h"

using namespace UkuiMenu;

namespace {

QString g_fixedAppId;
int g_fixedTop = -1;
QString g_updatedLaunchedAppId;
bool g_updatedLaunchedState = false;
bool g_recentInstall = false;
bool g_disableUninstallAction = false;
QString g_lastLaunchedBinary;
QString g_lastLaunchedArgs;

DataEntity createAppData(const QString &appId)
{
    DataEntity data;
    data.setType(DataType::Normal);
    data.setId(appId);
    return data;
}

QAction *findActionByText(const QList<QAction *> &actions, const QString &text)
{
    for (QAction *action : actions) {
        if (action && action->text() == text) {
            return action;
        }
    }

    return nullptr;
}

}

namespace UkuiMenu {

AppDatabaseInterface::AppDatabaseInterface(QObject *parent)
    : QObject(parent)
{
}

DataEntityVector AppDatabaseInterface::apps() const
{
    return {};
}

bool AppDatabaseInterface::getApp(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

void AppDatabaseInterface::fixAppToTop(const QString &appid, int index) const
{
    g_fixedAppId = appid;
    g_fixedTop = index;
}

void AppDatabaseInterface::fixAppToFavorite(const QString &appid, int index) const
{
    Q_UNUSED(appid);
    Q_UNUSED(index);
}

void AppDatabaseInterface::updateApLaunchedState(const QString &appid, bool state) const
{
    g_updatedLaunchedAppId = appid;
    g_updatedLaunchedState = state;
}

BasicAppFilterModel *BasicAppFilterModel::instance()
{
    static BasicAppFilterModel instance;
    return &instance;
}

BasicAppFilterModel::BasicAppFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

const AppDatabaseInterface *BasicAppFilterModel::databaseInterface() const
{
    static AppDatabaseInterface databaseInterface;
    return &databaseInterface;
}

DataEntity BasicAppFilterModel::appOfIndex(int row) const
{
    Q_UNUSED(row);
    return {};
}

int BasicAppFilterModel::indexOfApp(const QString &appid) const
{
    Q_UNUSED(appid);
    return -1;
}

bool BasicAppFilterModel::getAppById(const QString &appid, DataEntity &app) const
{
    Q_UNUSED(appid);
    Q_UNUSED(app);
    return false;
}

QVariant BasicAppFilterModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);
    return {};
}

bool BasicAppFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

bool BasicAppFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
}

RecentlyInstalledModel::RecentlyInstalledModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool RecentlyInstalledModel::event(QEvent *event)
{
    return QObject::event(event);
}

QVariant RecentlyInstalledModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index);
    Q_UNUSED(role);
    return {};
}

void RecentlyInstalledModel::setMode(AppCategoryModel::Mode mode)
{
    Q_UNUSED(mode);
}

bool RecentlyInstalledModel::isRecentInstall(const DataEntity &data)
{
    Q_UNUSED(data);
    return g_recentInstall;
}

bool RecentlyInstalledModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_row);
    Q_UNUSED(source_parent);
    return true;
}

bool RecentlyInstalledModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    Q_UNUSED(source_left);
    Q_UNUSED(source_right);
    return false;
}

qreal RecentlyInstalledModel::getDegree(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 0;
}

void RecentlyInstalledModel::executeInvalidate()
{
}

SecurityFunctionControl *SecurityFunctionControl::instance()
{
    static SecurityFunctionControl instance;
    return &instance;
}

SecurityFunctionControl::SecurityFunctionControl(QObject *parent)
    : QObject(parent)
{
}

bool SecurityFunctionControl::disable() const
{
    return false;
}

bool SecurityFunctionControl::disableMouseLeftButton() const
{
    return false;
}

bool SecurityFunctionControl::disableMouseRightButton() const
{
    return false;
}

bool SecurityFunctionControl::disableMouseHover() const
{
    return false;
}

QStringList SecurityFunctionControl::appWhiteList() const
{
    return {};
}

QStringList SecurityFunctionControl::appBlackList() const
{
    return {};
}

bool SecurityFunctionControl::disableUninstallAction() const
{
    return g_disableUninstallAction;
}

bool SecurityFunctionControl::windowMaximize() const
{
    return true;
}

bool SecurityFunctionControl::showFoldButton() const
{
    return true;
}

QString SecurityFunctionControl::powerButtonSoleOperation() const
{
    return {};
}

bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const
{
    Q_UNUSED(desktop);
    return true;
}

void SecurityFunctionControl::onFunctionControlChanged(const QString &name, bool exit)
{
    Q_UNUSED(name);
    Q_UNUSED(exit);
}

AppManager *AppManager::instance()
{
    static AppManager instance;
    return &instance;
}

AppManager::AppManager(QObject *parent)
    : QObject(parent)
{
}

void AppManager::launchApp(const QString &appid)
{
    Q_UNUSED(appid);
}

void AppManager::launchBinaryApp(const QString &app, const QString &args)
{
    g_lastLaunchedBinary = app;
    g_lastLaunchedArgs = args;
}

void AppManager::launchAppWithArguments(const QString &appid, const QStringList &args, const QString &applicationName)
{
    Q_UNUSED(appid);
    Q_UNUSED(args);
    Q_UNUSED(applicationName);
}

void AppManager::changeFavoriteState(const QString &appid, bool isFavorite)
{
    Q_UNUSED(appid);
    Q_UNUSED(isFavorite);
}

QStringList AppManager::parseDesktopFile(const QString &desktopFilePath)
{
    Q_UNUSED(desktopFilePath);
    return {};
}

}

class appMenuPlugin_test : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void test_pluginMetadataAndFactories();
    void test_contextMenu_returnsEmptyForInvalidInputs();
    void test_appListLocation_addsTopDesktopAndUninstallActions();
    void test_topAction_triggered_updatesDatabase();
    void test_recentInstallAction_triggered_updatesLaunchedState();
    void test_uninstallAction_triggered_launchesUninstaller();
    void test_systemApp_omitsUninstallAction();
    void test_desktopAction_copiesShortcutWhenTargetMissing();
    void test_desktopAction_isDisabledWhenTargetExists();
};

void appMenuPlugin_test::init()
{
    g_fixedAppId.clear();
    g_fixedTop = -1;
    g_updatedLaunchedAppId.clear();
    g_updatedLaunchedState = false;
    g_recentInstall = false;
    g_disableUninstallAction = false;
    GlobalSetting::instance()->setSystemApps(QStringList{});
    g_lastLaunchedBinary.clear();
    g_lastLaunchedArgs.clear();
}

void appMenuPlugin_test::test_pluginMetadataAndFactories()
{
    AppMenuPlugin plugin;

    QCOMPARE(plugin.id(), QStringLiteral("app-menu"));
    QCOMPARE(plugin.createWidgetExtension(), nullptr);

    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QVERIFY(extension);
    QCOMPARE(extension->index(), -1);
}

void appMenuPlugin_test::test_contextMenu_returnsEmptyForInvalidInputs()
{
    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());

    DataEntity folderData;
    folderData.setType(DataType::Folder);
    QMenu menu;

    QVERIFY(extension->actions(folderData, nullptr, MenuInfo::AppList, QStringLiteral("all")).isEmpty());
    QVERIFY(extension->actions(folderData, &menu, MenuInfo::AppList, QStringLiteral("all")).isEmpty());
}

void appMenuPlugin_test::test_appListLocation_addsTopDesktopAndUninstallActions()
{
    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(QStringLiteral("/tmp/test-app.desktop"));
    data.setTop(0);

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::AppList, QStringLiteral("all"));
    int separatorCount = 0;
    for (QAction *action : actions) {
        if (action && action->isSeparator()) {
            ++separatorCount;
        }
    }

    QCOMPARE(actions.size(), 4);
    QVERIFY(findActionByText(actions, tr("Fixed to all applications")));
    QVERIFY(findActionByText(actions, tr("Send to desktop shortcuts")));
    QVERIFY(findActionByText(actions, tr("Uninstall")));
    QCOMPARE(separatorCount, 1);
}

void appMenuPlugin_test::test_topAction_triggered_updatesDatabase()
{
    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(QStringLiteral("/tmp/fixed-app.desktop"));
    data.setTop(3);

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::AppList, QStringLiteral("all"));
    QAction *topAction = findActionByText(actions, tr("Unfixed from all applications"));

    QVERIFY(topAction);
    topAction->trigger();

    QCOMPARE(g_fixedAppId, QStringLiteral("/tmp/fixed-app.desktop"));
    QCOMPARE(g_fixedTop, 3);
}

void appMenuPlugin_test::test_recentInstallAction_triggered_updatesLaunchedState()
{
    g_recentInstall = true;
    g_disableUninstallAction = true;

    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(QStringLiteral("/tmp/recent-app.desktop"));

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::Extension, QString());
    QAction *clearAction = findActionByText(actions, tr("Clear recent installation mark"));

    QVERIFY(clearAction);
    clearAction->trigger();

    QCOMPARE(g_updatedLaunchedAppId, QStringLiteral("/tmp/recent-app.desktop"));
    QCOMPARE(g_updatedLaunchedState, true);
}

void appMenuPlugin_test::test_uninstallAction_triggered_launchesUninstaller()
{
    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(QStringLiteral("/tmp/remove-app.desktop"));

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::Extension, QString());
    QAction *uninstallAction = findActionByText(actions, tr("Uninstall"));

    QVERIFY(uninstallAction);
    uninstallAction->trigger();

    QCOMPARE(g_lastLaunchedBinary, QStringLiteral("kylin-uninstaller"));
    QCOMPARE(g_lastLaunchedArgs, QStringLiteral("/tmp/remove-app.desktop"));
}

void appMenuPlugin_test::test_systemApp_omitsUninstallAction()
{
    GlobalSetting::instance()->setSystemApps(QStringList{QStringLiteral("/tmp/system.desktop")});

    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(QStringLiteral("/tmp/system.desktop"));

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::Extension, QString());

    QCOMPARE(findActionByText(actions, tr("Uninstall")), nullptr);
}

void appMenuPlugin_test::test_desktopAction_copiesShortcutWhenTargetMissing()
{
    QTemporaryDir homeDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(qputenv("HOME", QFile::encodeName(homeDir.path())));
    QStandardPaths::setTestModeEnabled(true);

    const QString desktopDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QVERIFY(!desktopDir.isEmpty());
    QVERIFY(QDir().mkpath(desktopDir));

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    const QString appId = sourceDir.path() + QStringLiteral("/copied.desktop");

    QFile sourceFile(appId);
    QVERIFY(sourceFile.open(QIODevice::WriteOnly));
    sourceFile.write("[Desktop Entry]\nName=Copied\n");
    sourceFile.close();

    QFile::remove(desktopDir + QStringLiteral("/copied.desktop"));

    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(appId);

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::Extension, QString());
    QAction *desktopAction = findActionByText(actions, tr("Send to desktop shortcuts"));

    QVERIFY(desktopAction);
    QVERIFY(desktopAction->isEnabled());
    desktopAction->trigger();

    const QString targetPath = desktopDir + QStringLiteral("/copied.desktop");
    QFile targetFile(targetPath);
    QVERIFY(targetFile.exists());
    QCOMPARE(targetFile.permissions() & (QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther),
             QFile::Permissions(QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther));
}

void appMenuPlugin_test::test_desktopAction_isDisabledWhenTargetExists()
{
    QTemporaryDir homeDir;
    QVERIFY(homeDir.isValid());
    QVERIFY(qputenv("HOME", QFile::encodeName(homeDir.path())));
    QStandardPaths::setTestModeEnabled(true);

    const QString desktopDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QVERIFY(!desktopDir.isEmpty());
    QVERIFY(QDir().mkpath(desktopDir));

    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());
    const QString appId = sourceDir.path() + QStringLiteral("/existing.desktop");

    QFile sourceFile(appId);
    QVERIFY(sourceFile.open(QIODevice::WriteOnly));
    sourceFile.write("[Desktop Entry]\nName=Existing\n");
    sourceFile.close();

    QFile targetFile(desktopDir + QStringLiteral("/existing.desktop"));
    QVERIFY(targetFile.open(QIODevice::WriteOnly));
    targetFile.write("existing");
    targetFile.close();

    AppMenuPlugin plugin;
    QScopedPointer<ContextMenuExtension> extension(plugin.createContextMenuExtension());
    QMenu menu;
    DataEntity data = createAppData(appId);

    const QList<QAction *> actions = extension->actions(data, &menu, MenuInfo::Extension, QString());
    QAction *desktopAction = findActionByText(actions, tr("Send to desktop shortcuts"));

    QVERIFY(desktopAction);
    QCOMPARE(desktopAction->isEnabled(), false);
}

QTEST_MAIN(appMenuPlugin_test)

#include "app-menu-plugin-test.moc"
