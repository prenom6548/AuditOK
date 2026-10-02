/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "subsystempackagemanager.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>

SubSystemPackageManager::SubSystemPackageManager(QObject *parent)
    : QObject(parent) {
    connectSignals();
}

void SubSystemPackageManager::installPackage() {
    isUpdatingCache_ = true;
    updateCache();
}

void SubSystemPackageManager::unInstallPackage(
    const QStringList &uninstallList) {
    QDBusInterface interface(serviceName_, objectPath_, interfaceName_,
                             QDBusConnection::systemBus());

    isUninstallingPkg = true;
    QDBusMessage reply = interface.call("PurgePackages", uninstallList, "", "");
}

void SubSystemPackageManager::reboot() {
    QDBusInterface interface(
        "org.gnome.SessionManager", "/org/gnome/SessionManager",
        "org.gnome.SessionManager", QDBusConnection::sessionBus());
    interface.call("reboot");
}

void SubSystemPackageManager::fixBrokenDepends() {
    qDebug() << __func__;
    QDBusInterface interface(serviceName_, objectPath_, interfaceName_,
                             QDBusConnection::systemBus());

    QDBusMessage reply = interface.call("FixBrokenDepends");
}

void SubSystemPackageManager::fixIncompleteInstall() {
    qDebug() << __func__;
    QDBusInterface interface(serviceName_, objectPath_, interfaceName_,
                             QDBusConnection::systemBus());

    QDBusMessage reply = interface.call("FixIncompleteInstall");
}

void SubSystemPackageManager::connectSignals() {
    bool updateCacheFinishedConnected = QDBusConnection::systemBus().connect(
        serviceName_, objectPath_, interfaceName_, "UpdateCacheFinished", this,
        SLOT(handleUpdateCacheFinished(bool, QStringList, QString, QString)));
    if (!updateCacheFinishedConnected) {
        qWarning() << "无法连接到 UpdateCacheFinished 信号";
    }

    bool updateCacheStatusConnected = QDBusConnection::systemBus().connect(
        serviceName_, objectPath_, interfaceName_, "UpdateCacheStatusChanged",
        this, SLOT(handleUpdateCacheStatusChanged(int, QString)));
    if (!updateCacheStatusConnected) {
        qWarning() << "无法连接到 UpdateCacheStatusChanged 信号";
    }

    bool installdebStatusConnected = QDBusConnection::systemBus().connect(
        serviceName_, objectPath_, interfaceName_, "InstalldebStatusChanged",
        this, SLOT(handleInstalldebStatusChanged(int, QString, QString)));
    if (!installdebStatusConnected) {
        qWarning() << "无法连接到 InstalldebStatusChanged 信号";
    }

    bool updateInstallFinishedConnected = QDBusConnection::systemBus().connect(
        serviceName_, objectPath_, interfaceName_, "UpdateInstallFinished",
        this,
        SLOT(handleUpdateInstallFinished(bool, QStringList, QString, QString)));
    if (!updateInstallFinishedConnected) {
        qWarning() << "无法连接到 UpdateInstallFinished 信号";
    }

    bool purgePackagesStatusConnected = QDBusConnection::systemBus().connect(
        serviceName_, objectPath_, interfaceName_, "PurgePkgStatusChanged",
        this, SLOT(handlePurgePackagesStatusChanged(int, QString, QString)));
    if (!purgePackagesStatusConnected) {
        qWarning() << "无法连接到 PurgePkgStatusChanged 信号";
    }

    bool purgePackagesFinishedConnected = QDBusConnection::systemBus().connect(
        serviceName_, objectPath_, interfaceName_,
        "PurgePackagesFinishedWithPkgname", this,
        SLOT(handlePurgePackagesFinished(bool, QString, QString, QString)));
    if (!purgePackagesFinishedConnected) {
        qWarning() << "无法连接到 PurgePackagesFinished 信号";
    }
}

void SubSystemPackageManager::updateCache() {
    QDBusInterface interface(serviceName_, objectPath_, interfaceName_,
                             QDBusConnection::systemBus());

    QDBusMessage reply = interface.call("UpdateCache");

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << __func__
                   << "Failed to call dbus method: " << reply.errorMessage();
        return;
    }

    QVariantList results = reply.arguments();
    if (results.size() >= 2) {
        int returnValue0 = results[0].toInt();
        QString returnValue1 = results[1].toString();
        // 返回0表示调用成功
        if (returnValue0) {
            qWarning() << "Failed to call dbus method: UpdateCache!"
                       << returnValue1;
        }
    }
}

void SubSystemPackageManager::handleUpdateCacheFinished(
    bool success, const QStringList &groupList, const QString &errorResult,
    const QString &errorReason) {
    qDebug() << __func__ << success << groupList << errorResult << errorReason;
    if (!isUpdatingCache_) {
        return;
    }
    QDBusInterface interface(serviceName_, objectPath_, interfaceName_,
                             QDBusConnection::systemBus());

    isUpdatingCache_ = false;
    interface.call("InstallPackages", installPackageNames_);
    isInstallingPkg_ = true;
}

void SubSystemPackageManager::handleUpdateCacheStatusChanged(
    int progress, const QString &status) {
    qDebug() << __func__ << progress << status;
}

void SubSystemPackageManager::handleInstalldebStatusChanged(
    int progress, const QString &status, const QString &details) {
    qDebug() << __func__ << progress << status << details;
    if (isInstallingPkg_) {
        emit installProgressChanged(progress);
    } else {
        for (auto pkgName : installList_) {
            if (details.contains(pkgName)) {
                emit installProgressChanged(progress);
                break;
            }
        }
    }
}

void SubSystemPackageManager::handleUpdateInstallFinished(
    bool success, const QStringList &groupList, const QString &errorResult,
    const QString &errorReason) {
    qDebug() << __func__ << success << groupList << errorResult << errorReason;
    for (auto pkgName : installPackageNames_) {
        if (!groupList.contains(pkgName)) {
            return;
        }
    }

    isInstallingPkg_ = false;
    if (success) {
        emit installSuccessed();
    } else {
        emit installFailed(errorResult, errorReason);
    }
}

void SubSystemPackageManager::handlePurgePackagesStatusChanged(
    int progress, const QString &status, const QString &details) {
    qDebug() << __func__ << progress << status << details;
    if (isUninstallingPkg) {
        emit uninstallProgressChanged(progress);
    } else {
        for (auto pkgName : uninstallPackageNames_) {
            if (details.contains(pkgName)) {
                emit uninstallProgressChanged(progress);
                break;
            }
        }
    }
}

void SubSystemPackageManager::handlePurgePackagesFinished(
    bool success, const QString &status, const QString &details,
    const QString &pkgNameListString) {
    QStringList pkgNameList = pkgNameListString.split(',');
    for (auto pkgName : pkgNameList) {
        if (!uninstallPackageNames_.contains(pkgName)) {
            continue;
        }
        qDebug() << __func__ << success << status << details << pkgNameList;
        isUninstallingPkg = false;
        success ? emit unInstallSuccessed() : emit unIntsallFailed();
        break;
    }
}
