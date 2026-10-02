/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <QStringList>
#include <QDebug>
#include <QProcess>
#include <QByteArray>

#include "common.h"

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
    QProcess proc;
    QStringList options;
    options << "-c"<< command.join(" ");
    proc.closeWriteChannel();
    proc.start("bash", options);
    if (!proc.waitForFinished())
        return "";
    QString res = QString(proc.readAll());
    proc.close();
    if(res.right(1) == "\n")
        res.chop(1);
    return res;
}

void runCommand(const QStringList &command)
{
    QProcess proc;
    QStringList options;
    options << "-c"<< command.join(" ");
    proc.closeWriteChannel();
    proc.startDetached("bash", options);

    return ;
}

QByteArray qstringTochar(const QString &qstr)
{
    if (qstr.isEmpty())
        return nullptr;
    QByteArray ba = qstr.toLocal8Bit();
    return ba;
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
    if (printerName.contains(" ")
     || printerName.contains("/")
     || printerName.contains("\\")
     || printerName.contains("'")
     || printerName.contains('"')
     || printerName.contains("?")
     || printerName.contains("#")
     || printerName.toLocal8Bit().size() > 127) {

        return false;
    }
    return true;
}

bool processCheck(const QString &processName) {
    QString cmd = QString("ps -ef | grep %1 | grep -v grep | awk '{print $2}'").arg(processName);
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
    QString cmd = QString("/usr/bin/dpkg-query -W -f='${Version}\\n' %1").arg(packageName);
    res = ::getRetFromCommand(cmd.split(" "));
    return  res;
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

