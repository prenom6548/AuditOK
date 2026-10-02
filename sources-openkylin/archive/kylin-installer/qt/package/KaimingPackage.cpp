/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "KaimingPackage.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

KaimingPackage::KaimingPackage(QString debPath):Package(debPath, packageType::TPYE_KAIMING)
{
    initPackageInfo();
    initPackageInstallStatus();
    printInfo();
}

void KaimingPackage::initPackageInfo()
{
    QProcess        process;
    QJsonDocument   document;
    QJsonArray      layerArray;
    QJsonObject     infoObject;

    QFileInfo fileInfo(m_debPath);
    m_pkgFileName = fileInfo.fileName();
    qDebug() << "KaimingPackage::initPackageInfo:: m_pkgFileName=" << m_pkgFileName;

    process.start("/opt/kaiming-tools/bin/kaiming", QStringList() << "print-meta" << m_debPath);
    if (!process.waitForFinished(3 * 1000)) {
        qDebug() << "KaimingPackage::initPackageInfo: Failed to get meta data, "  \
                 << "[" << "/opt/kaiming-tools/bin/kaiming" << "print-meta" << m_debPath << "]" \
                 << process.error() << process.errorString();
        return;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qDebug() << "Kaiming process failed: exitCode ="
                << process.exitCode()
                << "exitStatus=" << process.exitStatus()
                << "stderr=" << process.readAllStandardError();
        return;
    }

    document = QJsonDocument::fromJson(process.readAll());
    if (document.isNull() || !document.isObject()) {
        qDebug()<<"KaimingPackage::initPackageInfo::document is null";
        return;
    }

    layerArray = document.object().value("layers").toArray();
    if (layerArray.isEmpty()) {
        qDebug()<<"KaimingPackage::initPackageInfo::layer is empty";
        return;
    }

    infoObject      = layerArray.at(0).toObject().value("info").toObject();
    this->m_name    = infoObject.value("id").toString();
    this->m_zhName  = infoObject.value("name").toString();
    this->m_version = infoObject.value("version").toString();
    this->m_md5Sum  = PackageUtils::getMd5SumPart(this->m_debPath);
}

void KaimingPackage::initPackageInstallStatus()
{
    if (this->m_name.isEmpty() || this->m_version.isEmpty())
        return;

    QString current_name    = this->m_pkgFileName;
    QString current_version = "";

    QDBusInterface *updaterInterface = new QDBusInterface(
                                                KYLIN_OS_DBUS_SERVICE_NAME,
                                                KYLIN_OS_DBUS_PATH,
                                                KYLIN_OS_DBUS_INTERFACE_NAME,
                                                QDBusConnection::systemBus());
    if (!updaterInterface->isValid()) {
        qDebug() << "connect to updater dbus failed";
        return ;
    }

    QDBusReply<int> reply = updaterInterface->call(QDBus::Block, "PackageInstallStatus", current_name, current_version);
    if (reply.isValid()) {
        if (reply.value() == 0) {
            this->m_status = PackageEnum::Status::INSTALLED;
            this->m_installedVersion = current_version;
        } else
            this->m_status = PackageEnum::Status::UNINSTALLED;

        qDebug() << "PackageInstallStatus method return value:" << reply.value();
    } else {
        qDebug() << "PackageInstallStatus method called failed ";
        this->m_status = PackageEnum::Status::UNINSTALLED;
    }
}
