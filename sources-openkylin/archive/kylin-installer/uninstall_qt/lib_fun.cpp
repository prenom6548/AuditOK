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
#include <stdio.h>

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include "lib_fun.h"
#include <QProcess>

#define XML_APPLICATIONS_PATH "/opt/kre/config/applications_v2.xml"
#define CMD_DPKG "dpkg -S %s | awk -F':' '{print $1}'"

/*
 * compatible environment analysis pkgname and runtime by desktop field
 * p_desktop_path_in --> desktop ptah -> in , p_pkg_name_out --> analysis pkg_name -> out , analysis runtime -> out
 * return 0->success , -1->fail
 */
int snow_analysis_xml(const QString &p_desktop_path_in , QString &p_pkg_name_out , QString &p_runtime_out)
{
    if (p_desktop_path_in.isEmpty()) {
        qWarning() << "Error : args error";
        return -1;
    }

    QString pkg_name;
    QStringList runtime_list;
    pkg_name.clear();
    runtime_list.clear();

    QString desktop_path(p_desktop_path_in);
    QString desktop_name = desktop_path.split('/').last();

    QString p_buf;

    int index = desktop_name.indexOf("kylin-v10");
    if (index == -1) {
        qWarning() << "Error : desktop name error";
        return -1;
    }

    p_buf = desktop_name.left(index) + desktop_name.mid(index + 8);

    qDebug() << "Info : handle desktop name is [" << p_buf << "]";

    desktop_name = p_buf;

    QDomDocument doc;
    QFile file(XML_APPLICATIONS_PATH);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        qWarning() << "Error : open xml file fail";
        return -1;
    }

    if (!doc.setContent(&file)) {
        qWarning() << "Error : load xml file fail";
        return -1;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNode first = root.firstChild();

    bool found = false;
    while (!first.isNull()) {
        if (first.nodeName() == "appPkgName") {
            pkg_name = first.toElement().attribute("name");
            QDomNode second = first.firstChild();
            while (!second.isNull()) {
                if (second.nodeName() == "runtimes") {
                    QDomNode third = second.firstChild();
                    while (!third.isNull()) {
                        if (third.nodeName() == "runtime") {
                            runtime_list << third.toElement().text();
                        }
                        third = third.nextSibling();
                    }
                }
                if (second.nodeName() == "appEntrys" && second.toElement().attribute("desktop") == desktop_name) {
                    found = true;
                    break;
                }
                second = second.nextSibling();
            }
        }
        if (found) break;
        first = first.nextSibling();
    }

    if (found) {
        p_pkg_name_out = pkg_name;
        if (!runtime_list.isEmpty()) {
            p_runtime_out = runtime_list.at(0);
        }
        return 0;
    }

    qWarning() << "Error : not analysis desktop";
    return -1;
}



QString get_zhname(QString p_desktop_path_in)
{
    QSettings setting(p_desktop_path_in, QSettings::IniFormat);
    QString locale = getenv("LANG");
    setting.beginGroup("Desktop Entry");

    if (locale.contains("zh_CN")) {
        if (setting.allKeys().contains("Name[zh_CN]")) {
            return QString(setting.value("Name[zh_CN]").toString().toLatin1());
        }
        else
        {
            return QString(setting.value("Name").toString().toLatin1());
        }
    }
    return QString(setting.value("Name").toString().toLatin1());
}

QString get_namezh_util(QString desktopPath, QString appName)
{
    QSettings setting(desktopPath, QSettings::IniFormat);
    setting.beginGroup("Desktop Entry");

    QString locale = QString::fromLocal8Bit(getenv("LANG"));
    QString appNameCN;
    if (locale.contains("zh_CN")) {
        appNameCN = get_zhname_from_uksc(appName);

        if (appNameCN.isEmpty())
            appNameCN = get_zhname_from_desktop(desktopPath);
    }

    if (appNameCN.isEmpty())
        appNameCN = setting.value("Name").toString().toLatin1();

    return appNameCN;
}

QString get_zhname_from_uksc(const QString &appName)
{
    const QString connName = "qt_sql_default_connection";

    QSqlDatabase db = QSqlDatabase::contains(connName)
            ? QSqlDatabase::database(connName)
            : QSqlDatabase::addDatabase("QSQLITE", connName);

    db.setDatabaseName(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
        + "/.cache/uksc/uksc.db"
    );

    if (!db.open()) {
        qDebug() << "Failed to open DB:" << db.lastError().text();
        return QString();
    }

    QSqlQuery query(db);
    query.prepare("SELECT display_name_cn FROM application WHERE app_name = :name");
    query.bindValue(":name", appName);

    if (!query.exec()) {
        qDebug() << "Query exec failed:" << query.lastError().text();
        return QString();
    }

    if (query.next()) {
        QString ret = query.value(0).toString();
        qDebug() << "Found zh_name:" << ret;
        return ret;
    }

    qDebug() << "No zh_name found for app:" << appName;
    return QString();
}

QString get_zhname_from_desktop(QString desktopPath)
{
    QSettings setting(desktopPath, QSettings::IniFormat);
    setting.beginGroup("Desktop Entry");

    if (setting.allKeys().contains("Name[zh_CN]")) {
        return setting.value("Name[zh_CN]").toString().toLatin1();
    }

    return QString();
}

/*
 * get local pkgname by desktop
 * return 0->success , -1->fail
 */
QString snow_get_local_pkg_name(QString p_desktop_path_in)
{
    if (p_desktop_path_in.length() <= 0) {
        printf("Error : arg error\n");
        return QString(); // 返回空字符串
    }

    QFileInfo filein(p_desktop_path_in);
    QString fileffsufix = filein.suffix();
    if (!fileffsufix.contains("desktop", Qt::CaseSensitive)) {
        return QString(); // 返回空字符串
    }

    if (p_desktop_path_in.contains("TopSAP")) {
        return "topsap";
    }

    QString cmd;
    cmd.append("dpkg -S ").append(p_desktop_path_in).append(" | awk -F':' '{print $1}'");
    QString pkgname = SelectPackgeName(cmd);
    qDebug() << "cmd:::" << cmd << "SelectPackgeName(cmd):::" << pkgname;
    if (pkgname.isEmpty()) {
        // 寻找包名，只按desktop文件寻找并不完全准确。
        QFile file(p_desktop_path_in);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!file.atEnd()) {
                QByteArray line = file.readLine();
                QString str(line);
                qDebug() << "readLine:::" << str;
                if (str.contains("Exec", Qt::CaseSensitive)) {
                    QStringList list = str.split("=");
                    if (list.size() > 1) { // 检查list的大小
                        qDebug() << "list[1]:::" << list[1];
                        if (list[1].contains("/usr/bin/runWithCompatibility.sh") || list[1].contains("kare run")) {
                            pkgname = "kare";
                            return pkgname;
                        }
                        if (list[1].contains(" ", Qt::CaseSensitive)) {
                            QStringList desk_one = list[1].split(" ");
                            cmd.clear();
                            cmd.append("dpkg -S ").append(desk_one[0]).append(" | awk -F':' '{print $1}'");
                            pkgname = SelectPackgeName(cmd);
                            qDebug() << "cmd:::" << cmd << "SelectPackgeName(cmd):::" << pkgname;
                            if(pkgname.contains(" "), Qt::CaseSensitive){     //如果有多行结果，则继续进行精准匹配（匹配“可执行程序名称”或“可执行程序名称+后缀”的形式）
                                cmd.clear();
                                QString tmpStr = "\"" + desk_one[0] + "(\\.[^/]+)?$" + "\"";
                                //qDebug() << "tmpStr:::" << tmpStr;
                                cmd = QString("dpkg -S %1 | grep -E %2 | awk -F':' '{print $1}'").arg(desk_one[0], tmpStr);
                                pkgname = SelectPackgeName(cmd);
                                qDebug() << "精准匹配---cmd:::" << cmd << "SelectPackgeName(cmd):::" << pkgname;
                            }
                            return pkgname;
                        } else {
                            cmd.clear();
                            cmd.append("dpkg -S ").append(list[1].simplified()).append(" | awk -F':' '{print $1}'");
                            pkgname = SelectPackgeName(cmd);
                            qDebug() << "cmd:::" << cmd << "SelectPackgeName(cmd):::" << pkgname;
                            if(pkgname.contains(" "), Qt::CaseSensitive){     //如果有多行结果，则继续进行精准匹配（匹配“可执行程序名称”或“可执行程序名称+后缀”的形式）
                                cmd.clear();
                                QString tmpStr = "\"" + list[1].simplified() + "(\\.[^/]+)?$" + "\"";
                                //qDebug() << "tmpStr:::" << tmpStr;
                                cmd = QString("dpkg -S %1 | grep -E %2 | awk -F':' '{print $1}'").arg(list[1].simplified(), tmpStr);
                                pkgname = SelectPackgeName(cmd);
                                qDebug() << "精准匹配---cmd:::" << cmd << "SelectPackgeName(cmd):::" << pkgname;
                            }
                            return pkgname;
                        }
                    }
                }
            }
        } else {
            printf("Error: Unable to open file\n");
            return QString(); // 返回空字符串
        }
    } else {
        return pkgname;
    }

    return QString(); // 默认返回空字符串
}

QString SelectPackgeName(QString cmd)
{
    QProcess terminal;
    QStringList cmdList;
    cmdList << "-c" << cmd;
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();
    return debInfo.simplified();
}



QString getAndroidPackageName(QString desktopFilePath)
{
    if (desktopFilePath == NULL) {
        return NULL;
    }
    if(desktopFilePath.contains("TopSAP"))
    {
        return  "topsap";
    }
    QString androidFilePath(desktopFilePath);
    QString desktopFileName = androidFilePath.split('/').last();
    return QString(desktopFileName.mid(0, desktopFileName.count() - 8).toLatin1().data());

}

QString getKarePackageNameFromFile(const QString &filePath)
{
    QProcess process;
    process.start("kare",QStringList()<<"-S"<<filePath);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
    if (!error.isEmpty())
    {
        qWarning()<<"lib_fun::getKarePackageNameFromFile::error="<<error;
        return QString();
    }

    QStringList lines = output.split('\n');
    for (const QString &line: lines)
    {
        int colonIndex = line.indexOf(':');
        if (colonIndex != -1)
        {
            QString packageName = line.left(colonIndex).trimmed();
            if (!packageName.isEmpty())
            {
                return packageName;
            }
        }

    }
    qWarning() << "lib_fun::getKarePackageNameFromFile::output="<<output;
    return QString();

}

QString resolveRealFile(const QString &path)
{
    // qDebug() << "[test]: resolveRealFile path: " << path;
    QByteArray pathBytes = path.toLocal8Bit();

    QFileInfo info(path);
    if (!info.isSymLink()) {
        return path;
    }

    char buf[PATH_MAX] = {0};
    ssize_t len = readlink(pathBytes.constData(), buf, sizeof(buf) - 1);

    if (len == -1) {
        return path;
    }

    buf[len] = '\0';
    qDebug() << "[test]: resolveRealFile Real path: " << QString::fromLocal8Bit(buf);
    return QString::fromLocal8Bit(buf);
}

/*
 * remove line last ernter
 */
void remove_last_line_break(QString &src_in_out)
{
    if (src_in_out.endsWith('\n')) {
        src_in_out.chop(1);
    }
}


/*
 * get max six chinese characters from char array
 */
QString intercept_chinese_characters(QString src_in)
{
    QString dst;
    dst.clear();
    int i_len = src_in.length();
    qDebug() << "==========" << i_len;

    if (i_len > 6) {
        dst = src_in.mid(0 , 6);
    } else {
        dst = src_in;
    }

    qDebug() << "---------->" << dst;

    return dst;
}
