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
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <QStringList>
#include <QDebug>
#include <QProcess>
#include <QByteArray>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>
#include <QFileInfo>
#include "common.h"
#include "libkysysinfo.h"
#include "libkypackages.h"
#include "device_information.h"


CheckRes checkCommand(const QStringList &command)
{
    if (command.isEmpty())
        return MAINWINDOW;
    else if(command.at(0).compare("debug")==0)
        return DEBUG;
    else if(command.at(0).compare("show")==0)
        return MAINWINDOW;
    else if (command.size() != 2) {
        return CHECK_FALSE;
    }
    else if (command.at(0).compare("add")==0
          || command.at(0).compare("remove")==0
          || command.at(0).compare("add_uri")==0 ) {

        return POPWINDOW;
    }
    return CHECK_FALSE;
}

QString getRetFromCommand(const QStringList &command)
{
    QString res;
    execCommand(command, &res, nullptr, nullptr, nullptr);
    return res;
}

bool execCommand(const QStringList &command, QString *res, QString *standardError, int *exitCode, int *msec)
{
    QProcess proc;
    QStringList options;
    options << "-c"<< command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);

    int mescs = 30000;
    if (msec) {
        mescs = *msec;
    }
    
    if (!proc.waitForFinished(mescs))
        return false;
    if (res) {
        *res = QString(proc.readAll());
        if((*res).right(1) == "\n")
            (*res).chop(1);
    }
    if (standardError) {
        *standardError = QString(proc.readAllStandardError());
        if((*standardError).right(1) == "\n")
            (*standardError).chop(1);
    }
    if (exitCode)
        *exitCode = proc.exitCode();
    proc.close();
    return true;
}

void runCommand(const QStringList &command)
{
    if (command.isEmpty()) {
        return;
    }
    QProcess proc;
    QStringList options = command;
    options.removeAt(0);
    proc.closeWriteChannel();
    proc.startDetached(command[0], options);

    return ;
}

QString getUserName()
{
    uid_t uid;
    uid_t NO_UID = -1;
    struct passwd *pw;

    errno = 0;
    uid = geteuid ();
    if (uid == -1 && errno) {
        qDebug () << "get uid error";
        return "";
    }
    pw = getpwuid (uid);
    if (!pw) {
        qDebug ("cannot find name for user ID %lu", (unsigned long int) uid);
        return "";
    }
    return QString(pw->pw_name);
}

bool isAdministratorUser()
{
    static gid_t list[100];
    int n_gp = getgroups(100,list);
    struct group *gp;
    for(int i = 0;i < n_gp; i++) {
        gp = getgrgid(list[i]);
        if (!gp) {
            continue;
        }
        if(QString(gp->gr_name) == "lpadmin") {
            return true;
        }
    }
    return false;
}

void specialDeviceCheck(QString &vendor, QString &model)
{
    vendor = vendor.trimmed();
    model = model.trimmed();

    QMap<QString, QString>::const_iterator iter = mfgConvertMap.begin();
    while (iter != mfgConvertMap.end())
    {
        if (iter.key().compare(vendor, Qt::CaseInsensitive) == 0) {
            vendor = iter.value();
            break;
        }
        iter++;
    }
    if (model.indexOf("HP") == 0) {
        model.remove(0, 2);
        if (model.at(0) == " ") {
            model.remove(0, 1);
        }
    }

    if (model.contains(" series")) {
        model.remove(" series");
    }
}

QString devicePathCheck(const QString &path)
{
    QString ppath = path;
    if (ppath.contains("unbind@"))
        ppath.remove("unbind@");
    if (ppath.contains("bind@"))
        ppath.remove("bind@");
    return ppath;
}

bool ipIsValid(const QString &ipAddr)
{
    QRegExp rx2("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
    if (!rx2.exactMatch(ipAddr)) {
        return false;
    } else {
        QStringList pieces = ipAddr.split(".");
        bool ok = true;
        for (int i = 0; i < 4; i++) {
            int num = pieces.value(i).toInt(&ok, 10); //合理性判断
            if (!ok) {
                return false;
            }
            if (num > 255) {
                return false;
            }
        }
    }
    return true;
}

bool printerNameIsLegal(const QString &printerName)
{
    if (printerName.isEmpty()) {
        return false;
    }

    if (printerName.contains(" ")
     || printerName.contains("/")
     || printerName.contains("\\")
     || printerName.contains("'")
     || printerName.contains('"')
     || printerName.contains("?")
     || printerName.contains("#")
     || printerName.contains("<")
     || printerName.contains(">")
     || printerName.toLocal8Bit().size() > 127) {

        return false;
    }
    return true;
}

bool processCheck(const QString &processName) {
    QString cmd = QString("pgrep -f \"^%1\"").arg(processName);
    QString res = getRetFromCommand(cmd.split(" "));
    if (res != "" && res.toInt() > 0) {
        return true;
    }
    return false;
}

QString getPackageVersion(const QString &packageName)
{
    QString res{QString()};
    if (packageName.isEmpty()) {
        return res;
    }
    
    char *version = kdk_package_get_version(QTC(packageName));
    res = QString(version);
    free(version);
    qDebug() << "kdk_package_get_version is:" << packageName << " " << res;
    return res;
}

QString getDebVersion(const QString &debName)
{
    QString deb(debName);
    if (!deb.contains("_")) {
        return "";
    }

    deb.remove(0, deb.indexOf("_") + 1);

    if (!deb.contains("_")) {
        return "";
    }

    QString debVersion = deb.left(deb.indexOf("_"));
    return debVersion;
}

bool packageIsInstalled(const QString &name, const QString &version)
{
    // brscan5chn_1.2.5-0_amd64.deb
    QString packageVersion = getPackageVersion(name);
    if (packageVersion.isEmpty()) {
        return false;
    }

    if (version.isEmpty()) {
        return false;
    }
    if (version != packageVersion) {
        return false;
    }
    return true;
}

bool packageListNeedInstall(const QList<PackageInfo> &list)
{
    if (list.isEmpty()) {
        return false;
    }

    int needInstall = false;
    for (int i = 0; i < list.size(); i++) {
        if (packageIsInstalled(list[i].name, list[i].version) == false) {
            needInstall = true;
            break;
        }
    }
    return needInstall;
}


QString encodeUrl(const QString &in) {
    return QUrl::toPercentEncoding(in);
}

QString decodeUrl(const QString &in) {
    return QUrl::fromPercentEncoding(in.toLocal8Bit());
}

void delay_msec(unsigned int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}

bool isV11version()
{
    if (QString(kdk_system_get_major_version()) == "V11") {
        return true;
    } else {
        return false;
    }
    return false;
}

bool isDebFile(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return (fileInfo.suffix().toLower() == "deb");
}

bool isFile(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.isFile();  // 返回true如果是文件且存在
}
