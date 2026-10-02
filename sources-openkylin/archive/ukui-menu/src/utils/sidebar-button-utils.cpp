/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "sidebar-button-utils.h"
#include "app-manager.h"
#include "user-menu.h"

#include <QProcess>
#include <QDBusReply>
#include <QQmlEngine>

#define KYLIN_ACCOUNT_INFORMATION_NAME         "org.freedesktop.Accounts"
#define KYLIN_ACCOUNT_INFORMATION_PATH         "/org/freedesktop/Accounts"
#define KYLIN_ACCOUNT_INFORMATION_INTERFACE    "org.freedesktop.Accounts"
#define DEFAULT_USER_ICON_FILE        ":/res/icon/default-community-image.png"

using namespace UkuiMenu;

SidebarButtonUtils::SidebarButtonUtils(QObject *parent) : QObject(parent)
{
    registeredAccountsDbus();
    if (m_systemUserIFace && m_systemUserIFace->isValid()) {
        initUserInfo();
    } else {
        qWarning() << "KYLIN_ACCOUNT_INFORMATION_INTERFACE" << "dbus is invalid .";
    }
}

void SidebarButtonUtils::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterType<SidebarButtonUtils>("org.ukui.menu.utils", versionMajor, versionMinor, "SidebarButtonUtils");
}

QString SidebarButtonUtils::getUsername()
{
    if (!m_currentUserInfo) {
        return QString{};
    }
    return m_currentUserInfo->userName();
}

QString SidebarButtonUtils::getRealName()
{
    if (!m_currentUserInfo) {
        return QString{};
    }
    return m_currentUserInfo->realName();
}

QString SidebarButtonUtils::getIconFile()
{
    if (!m_currentUserInfo) {
        return QString{DEFAULT_USER_ICON_FILE};
    }

    if (m_currentUserInfo->iconFile().isEmpty() || m_currentUserInfo->iconFile().endsWith("/.face")) {
        return {DEFAULT_USER_ICON_FILE};
    }

    return m_currentUserInfo->iconFile();
}

void SidebarButtonUtils::openUserCenter()
{
    QStringList args;
    args.append("-m");
    args.append("Userinfo");
    UkuiMenu::AppManager::instance()->launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", args, "ukui-control-center");
}

void SidebarButtonUtils::openPeonyComputer()
{
    UkuiMenu::AppManager::instance()->launchAppWithArguments("/usr/share/applications/peony-computer.desktop", QStringList(), "/usr/bin/peony computer:///");
}

void SidebarButtonUtils::openControlCenter()
{
    UkuiMenu::AppManager::instance()->launchAppWithArguments("/usr/share/applications/ukui-control-center.desktop", QStringList(), "ukui-control-center");
}

void SidebarButtonUtils::openUsersInfoMenu(int menuX, int menuY, bool isFullScreen)
{
    // GCOVR_EXCL_START
    auto userMenu = new UserMenu;
    userMenu->setAttribute(Qt::WA_DeleteOnClose);

    QDBusInterface sessionManager("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                      "org.gnome.SessionManager", QDBusConnection::sessionBus());
    QDBusReply<bool> reply = sessionManager.call(QStringLiteral("canSwitch"));

    for (const auto& userItem : m_userInfoItemList) {
        if (reply.isValid() && reply.value() || userItem->isCurrentUser()) {
            QAction *act = new QAction(userItem->circularIcon(), userItem->realName(), userMenu);
            connect(act, &QAction::triggered, userMenu, [this, userItem](){
                if (userItem->userName() != getUsername()) {
                    switchToUser(userItem->userName());
                }
            });
            userMenu->addAction(act);
            const bool isActveUser = userItem->isCurrentUser() && userItem->isLogged();
            act->setCheckable(isActveUser);
            act->setChecked(isActveUser);

            reply = sessionManager.call(QStringLiteral("canLogout"));
            if (isActveUser && (reply.isValid() && reply.value())) {
                userMenu->addMiniButton(act, tr("Log Out"), QSize(64, 32));
                connect(userMenu, &UserMenu::miniButtonClicked, this, [](QAction *action, QPushButton *btn) {
                   AppManager::instance()->launchBinaryApp("ukui-session-tools", "--logout");
                });
            }
        }
    }

    static QIcon userIcon = QIcon::fromTheme("user-available-symbolic");
    QAction *act = new QAction(userIcon, tr("Change account settings"), userMenu);
    connect(act, &QAction::triggered, userMenu, [this](){
        openUserCenter();
    });

    if (userMenu->actions().count() > 0) userMenu->addSeparator();
    userMenu->addAction(act);

    const QPoint popupPos = isFullScreen
        ? QPoint(menuX - userMenu->sizeHint().width(), menuY - userMenu->sizeHint().height())
        : QPoint(menuX, menuY - userMenu->sizeHint().height());

    userMenu->popup(popupPos);
}

void SidebarButtonUtils::registeredAccountsDbus()
{
    m_systemUserIFace = new QDBusInterface(KYLIN_ACCOUNT_INFORMATION_NAME,
                                           KYLIN_ACCOUNT_INFORMATION_PATH,
                                           KYLIN_ACCOUNT_INFORMATION_INTERFACE,
                                           QDBusConnection::systemBus(), this);

    QDBusConnection::systemBus().connect(KYLIN_ACCOUNT_INFORMATION_NAME,
                                         KYLIN_ACCOUNT_INFORMATION_PATH,
                                         KYLIN_ACCOUNT_INFORMATION_INTERFACE, "UserAdded",
                                         this, SLOT(onUserAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(KYLIN_ACCOUNT_INFORMATION_NAME,
                                         KYLIN_ACCOUNT_INFORMATION_PATH,
                                         KYLIN_ACCOUNT_INFORMATION_INTERFACE, "UserDeleted",
                                         this, SLOT(onUserRemoved(QDBusObjectPath)));



}

void SidebarButtonUtils::initUserInfo()
{
    QStringList objectPaths = getUserObjectPath();
    for (const QString& objectPath : objectPaths){
        std::shared_ptr<UserInfoItem> userItem(new UserInfoItem(objectPath));
        m_userInfoItemList.append(userItem);
        if (userItem->isCurrentUser() && userItem->isLogged()) {
            m_currentUserInfo = userItem;
            connect(userItem.get(), &UserInfoItem::userInfoChanged, this, &SidebarButtonUtils::userInfoChanged);
        }
    }
}

QStringList SidebarButtonUtils::getUserObjectPath()
{
    QStringList users;
    QDBusReply<QList<QDBusObjectPath> > reply = m_systemUserIFace->call("ListCachedUsers");
    if (reply.isValid()) {
        for (const QDBusObjectPath& op : reply.value()) {
            users << op.path();
        }
    }

    if (users.isEmpty()) {
        auto automaticLoginUsers = m_systemUserIFace->property("AutomaticLoginUsers").value<QList<QDBusObjectPath>>();
        for (const auto& userPath : automaticLoginUsers) {
            users << userPath.path();
        }
    }

    return users;
}

void SidebarButtonUtils::switchToUser(QString userName)
{
    QDBusInterface interface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                             "org.gnome.SessionManager", QDBusConnection::sessionBus());

    if (interface.isValid()) {
        interface.asyncCall("switchToUser", userName);
    } else {
        qCritical() << "Call 'org.gnome.SessionManager' interface's method 'switchToUser' failed!"
                    <<  QDBusConnection::sessionBus().lastError().message();
    }

    // GCOVR_EXCL_STOP
}

void SidebarButtonUtils::onUserAdded(const QDBusObjectPath &op)
{
    std::shared_ptr<UserInfoItem> userItem(new UserInfoItem(op.path()));
    connect(userItem.get(), &UserInfoItem::userInfoChanged, this, &SidebarButtonUtils::userInfoChanged);
    m_userInfoItemList.append(userItem);

}

void SidebarButtonUtils::onUserRemoved(const QDBusObjectPath &op)
{
    m_userInfoItemList.erase(
        std::remove_if(m_userInfoItemList.begin(), m_userInfoItemList.end(),
            [&op](const std::shared_ptr<UserInfoItem>& item) {
                return item->objPath() == op.path();
            }),
        m_userInfoItemList.end()
    );
}
