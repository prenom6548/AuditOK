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

#define private public

#include "../../../src/utils/sidebar-button-utils.h"

#include "fakes/app-manager.h"

#include <QMenu>
#include <QPushButton>

namespace {
QString g_lastLaunchAppId;
QStringList g_lastLaunchArgs;
QString g_lastApplicationName;
int g_launchWithArgumentsCalls = 0;
int g_binaryLaunchCalls = 0;
int g_popupCalls = 0;
QPoint g_lastPopupPos;
}

class TestUserMenu : public QMenu
{
    Q_OBJECT
public:
    explicit TestUserMenu(QWidget *parent = nullptr)
        : QMenu(parent)
    {
    }

    void addMiniButton(QAction *action, const QString &btnText, const QSize &btnSize)
    {
        Q_UNUSED(action)
        Q_UNUSED(btnText)
        Q_UNUSED(btnSize)
        ++m_miniButtonCount;
    }

    void popup(const QPoint &pos)
    {
        ++g_popupCalls;
        g_lastPopupPos = pos;
        deleteLater();
    }

    int m_miniButtonCount {0};

Q_SIGNALS:
    void miniButtonClicked(QAction *action, QPushButton *button);
};

namespace UkuiMenu {

AppManager *AppManager::instance()
{
    static AppManager instance;
    return &instance;
}

void AppManager::launchApp(const QString &appid)
{
    Q_UNUSED(appid);
}

void AppManager::launchBinaryApp(const QString &app, const QString &args)
{
    Q_UNUSED(app);
    Q_UNUSED(args);
    ++g_binaryLaunchCalls;
}

void AppManager::launchAppWithArguments(const QString &appid, const QStringList &args, const QString &applicationName)
{
    g_lastLaunchAppId = appid;
    g_lastLaunchArgs = args;
    g_lastApplicationName = applicationName;
    ++g_launchWithArgumentsCalls;
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

#define UserMenu TestUserMenu
#define USERMENU_H
#define UKUI_MENU_APP_MANAGER_H
#include "../../../src/utils/sidebar-button-utils.cpp"
#undef USERMENU_H
#undef UserMenu

#undef private

#include <QtTest>
#include <QSignalSpy>

using namespace UkuiMenu;

class sidebarButtonUtils_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_gettersFallbackWithoutCurrentUser();
    void test_gettersUseCurrentUserAndDefaultIconRule();
    void test_defineModule_registersType();
    void test_openHelpers_delegateToAppManager();
    void test_openUsersInfoMenu_buildsMenuAndPopsUp();
    void test_getUserObjectPath_fallsBackToAutomaticLoginUsers();
    void test_onUserAddedAndRemoved_manageUserList();
};

void sidebarButtonUtils_test::init()
{
    g_lastLaunchAppId.clear();
    g_lastLaunchArgs.clear();
    g_lastApplicationName.clear();
    g_launchWithArgumentsCalls = 0;
    g_binaryLaunchCalls = 0;
    g_popupCalls = 0;
    g_lastPopupPos = QPoint();
}

void sidebarButtonUtils_test::test_gettersFallbackWithoutCurrentUser()
{
    SidebarButtonUtils utils;
    utils.m_currentUserInfo.reset();

    QCOMPARE(utils.getUsername(), QString());
    QCOMPARE(utils.getRealName(), QString());
    QCOMPARE(utils.getIconFile(), QStringLiteral(":/res/icon/default-community-image.png"));
}

void sidebarButtonUtils_test::test_gettersUseCurrentUserAndDefaultIconRule()
{
    SidebarButtonUtils utils;
    auto current = std::make_shared<UserInfoItem>(QStringLiteral("/current"));
    current->m_userName = QStringLiteral("current-user");
    current->m_realName = QStringLiteral("Current User");
    current->m_iconFile = QStringLiteral("/tmp/.face");
    utils.m_currentUserInfo = current;

    QCOMPARE(utils.getUsername(), QStringLiteral("current-user"));
    QCOMPARE(utils.getRealName(), QStringLiteral("Current User"));
    QCOMPARE(utils.getIconFile(), QStringLiteral(":/res/icon/default-community-image.png"));

    current->m_iconFile = QStringLiteral("/tmp/avatar.png");
    QCOMPARE(utils.getIconFile(), QStringLiteral("/tmp/avatar.png"));
}

void sidebarButtonUtils_test::test_defineModule_registersType()
{
    SidebarButtonUtils::defineModule("Ukui.Menu.Sidebar.Test", 1, 0);
    QVERIFY(true);
}

void sidebarButtonUtils_test::test_openHelpers_delegateToAppManager()
{
    SidebarButtonUtils utils;

    utils.openUserCenter();
    QCOMPARE(g_lastLaunchAppId, QStringLiteral("/usr/share/applications/ukui-control-center.desktop"));
    QCOMPARE(g_lastLaunchArgs, QStringList({ QStringLiteral("-m"), QStringLiteral("Userinfo") }));
    QCOMPARE(g_lastApplicationName, QStringLiteral("ukui-control-center"));

    utils.openPeonyComputer();
    QCOMPARE(g_lastLaunchAppId, QStringLiteral("/usr/share/applications/peony-computer.desktop"));
    QCOMPARE(g_lastLaunchArgs, QStringList());
    QCOMPARE(g_lastApplicationName, QStringLiteral("/usr/bin/peony computer:///"));

    utils.openControlCenter();
    QCOMPARE(g_lastLaunchAppId, QStringLiteral("/usr/share/applications/ukui-control-center.desktop"));
    QCOMPARE(g_lastLaunchArgs, QStringList());
    QCOMPARE(g_lastApplicationName, QStringLiteral("ukui-control-center"));
    QCOMPARE(g_launchWithArgumentsCalls, 3);
}

void sidebarButtonUtils_test::test_openUsersInfoMenu_buildsMenuAndPopsUp()
{
    SidebarButtonUtils utils;
    auto current = std::make_shared<UserInfoItem>(QStringLiteral("/current"));
    current->m_userName = QString(qgetenv("USER"));
    current->m_realName = QStringLiteral("Current User");
    current->m_isCurrentUser = true;
    current->m_isLogged = true;
    utils.m_currentUserInfo = current;
    utils.m_userInfoItemList.append(current);

    utils.openUsersInfoMenu(120, 240, false);
    QCoreApplication::processEvents();

    QCOMPARE(g_popupCalls, 1);
    QCOMPARE(g_lastPopupPos.x(), 120);
    QVERIFY(g_lastPopupPos.y() <= 240);
    QCOMPARE(g_binaryLaunchCalls, 0);
}

void sidebarButtonUtils_test::test_getUserObjectPath_fallsBackToAutomaticLoginUsers()
{
    SidebarButtonUtils utils;
    utils.m_systemUserIFace = new QDBusInterface(QStringLiteral("org.ukui.Menu.NonexistentAccounts"),
                                                 QStringLiteral("/org/ukui/Menu/NonexistentAccounts"),
                                                 QStringLiteral("org.ukui.Menu.NonexistentAccounts"),
                                                 QDBusConnection::systemBus(),
                                                 &utils);

    const QStringList users = utils.getUserObjectPath();

    QCOMPARE(users, QStringList());
}

void sidebarButtonUtils_test::test_onUserAddedAndRemoved_manageUserList()
{
    SidebarButtonUtils utils;
    QSignalSpy spy(&utils, &SidebarButtonUtils::userInfoChanged);
    const int initialSize = utils.m_userInfoItemList.size();

    utils.onUserAdded(QDBusObjectPath(QStringLiteral("/user/current")));
    utils.onUserAdded(QDBusObjectPath(QStringLiteral("/user/other")));

    QCOMPARE(utils.m_userInfoItemList.size(), initialSize + 2);
    QCOMPARE(utils.m_userInfoItemList.at(utils.m_userInfoItemList.size() - 2)->objPath(), QStringLiteral("/user/current"));
    QCOMPARE(utils.m_userInfoItemList.at(utils.m_userInfoItemList.size() - 1)->objPath(), QStringLiteral("/user/other"));

    emit utils.m_userInfoItemList.at(utils.m_userInfoItemList.size() - 2)->userInfoChanged();
    QCOMPARE(spy.count(), 1);

    utils.onUserRemoved(QDBusObjectPath(QStringLiteral("/user/missing")));
    QCOMPARE(utils.m_userInfoItemList.size(), initialSize + 2);

    utils.onUserRemoved(QDBusObjectPath(QStringLiteral("/user/current")));
    QCOMPARE(utils.m_userInfoItemList.size(), initialSize + 1);

    bool hasCurrentUser = false;
    bool hasOtherUser = false;
    for (const auto &item : qAsConst(utils.m_userInfoItemList)) {
        if (item->objPath() == QStringLiteral("/user/current")) {
            hasCurrentUser = true;
        }
        if (item->objPath() == QStringLiteral("/user/other")) {
            hasOtherUser = true;
        }
    }

    QCOMPARE(hasCurrentUser, false);
    QCOMPARE(hasOtherUser, true);
}

QTEST_MAIN(sidebarButtonUtils_test)

#include "sidebar-button-utils-test.moc"
