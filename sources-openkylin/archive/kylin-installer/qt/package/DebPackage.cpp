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
#include "DebPackage.h"

DebPackage::DebPackage(QString debPath):Package(debPath)
{
    initPackageInfo();
    initPackageInstallStatus();
    printInfo();
}

QString DebPackage::getDebInfoFromDeb(QString debPath, QString type)
{

    QStorageInfo SD;
    SD.setPath("/");
    SD.refresh();
    int disk_space = SD.bytesAvailable()/1024/1024;
    if (disk_space <=10)
    {
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",QObject::tr("Insufficient disk space!!!!"),"");
        msgBox->exec();
        msgBox->setContextMenuPolicy(Qt::NoContextMenu);
        exit(0);
    }

    debPath = QDir::toNativeSeparators(debPath);

    // fixme: 单元测试时，路径对也会无法生成对象
    QApt::DebFile debfile(debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }

}

QString DebPackage::getAppCnName(QString appName)
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/uksc/uksc.db");
    if (!db.open()) {
        qDebug()<<"db open failed!";
    }
    /**************************使用QSqlQuery操作数据库**************************/
    QSqlQuery query;	//执行操作类对象

    bool ret = query.exec(QString("SELECT * FROM application WHERE app_name IS '%1'").arg(appName));	//执行
    qDebug()<<"ret:"<<ret;
    while(query.next()) {
        qDebug()<<query.size();
        qDebug()<<query.value(4).toString();
        return query.value(4).toString();
    }
    db.close();
    return "";
}


void DebPackage::initPackageInfo()
{
    if (getDebInfoFromDeb(this->m_debPath, "Package") == "The software package is damaged")
    {
        this->m_status = PackageEnum::Status::DAMAGED;
        return;
    }
    this->m_name = getDebInfoFromDeb(this->m_debPath, "Package").toLower();
    this->m_version = getDebInfoFromDeb(this->m_debPath, "Version");
    if (this->m_name.isEmpty() || this->m_version.isEmpty()) return;
    this->m_architecture = getDebInfoFromDeb(this->m_debPath,"Architecture");
    this->m_zhName = getAppCnName(this->m_name);
    this->m_md5Sum = PackageUtils::getMd5Sum(this->m_debPath);
}

void DebPackage::initPackageInstallStatus()
{
    if (this->m_name.isEmpty() || this->m_version.isEmpty())
        return;

    QString debpathstring   = this->m_debPath;
    QString current_version = getDebInfoFromDeb(debpathstring, "Version");
    QString current_name    = getDebInfoFromDeb(debpathstring, "Package").toLower();
    QString Architecture    = getDebInfoFromDeb(debpathstring, "Architecture");

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

