/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "prefmanager.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProcess>
#include <QThread>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
#include <QDBusMessage>
#include <QtDBus>

#include <sys/types.h>
#include <sys/stat.h>


static const QString gamesFile = "/usr/share/kmre/games.json";
static const QString dockerIpFile = "/etc/docker/daemon.json";
static const QString warningFile = "/usr/share/kmre/warning.json";

static const QString kmreSetting = "/usr/bin/kylin-kmre-settings";
static const QString kmreWindow = "/usr/bin/kylin-kmre-window";
static const QString kmreManager = "/usr/bin/kylin-kmre-manager";

// 判断指定项是否在json文件里
bool isInJson(QString pkgName)
{
    QFile file(gamesFile);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << QString("Failed to open %1").arg(gamesFile);
            return false;
        }
        else {
            QString content = file.readAll();
            file.close();

            QJsonParseError jsonError;
            const QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
            if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
                if (jsonDocument.isObject()) {
                    QJsonObject obj = jsonDocument.object();
                    if (obj.contains("games")) {
                        QJsonValue arrayVaule = obj.value("games");
                        if (arrayVaule.isArray()) {
                            QJsonArray array = arrayVaule.toArray();
                            for (int i = 0;i<array.size();i++) {
                                QJsonValue value = array.at(i);
                                QJsonObject child = value.toObject();
                                QString pkgname = child["pkgname"].toString();
                                if (pkgName == pkgname) {
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool runCmd(const QString &cmd, const QStringList &args)
{
    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);
    // Merge stdout and stderr of subprocess with main process.
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start();
    // Wait for process to finish without timeout.
    process.waitForFinished(-1);
    return (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
}

bool runCmd(const QString &cmd, const QStringList &args, QString &output, QString &err)
{
    uint loop_num = 0;
    QProcess process;
    process.setProgram(cmd);
    process.setArguments(args);

    // try 2
    while (loop_num++ < 2) {
        process.start();
        // Wait for process to finish without timeout.
        process.waitForFinished(-1);
        output += process.readAllStandardOutput();
        err += process.readAllStandardError();

        if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            return true;
        }
        qDebug() << err;
        qDebug() << "try again!!!!!";
        QThread::sleep(1);
    }

    return false;
}

bool runCmd(const QString &cmd, const QStringList &args, QString &output)
{
    QString err;
    return runCmd(cmd, args, output, err);
}

bool addExecutable(const QString &path, bool recursive)
{
    if (recursive) {
        return runCmd("/bin/chmod", {"-R", "a+x", path});
    }
    else {
        return runCmd("/bin/chmod", {"a+x", path});
    }
}

inline int execScript(const QString &script, const QStringList &args)
{
    if (args.length() != 3) {
        return -1;
    }

    QProcess process;
    process.setProgram("/bin/sh");
    process.setArguments(QStringList() << script << args);
    process.start();
//    process.waitForStarted(-1);
    process.waitForFinished(-1);

    process.deleteLater();
    int ret = process.exitCode();

    return ret;
}

QString getFileName(const QString &filepath)
{
    const int slash_index = filepath.lastIndexOf(QDir::separator());
    if (slash_index > -1) {
        return filepath.mid(slash_index + 1);
    }
    else {
        return filepath;
    }
}

QString getFileBasename(const QString &filepath)
{
    const QString filename = getFileName(filepath);
    const int dot_index = filename.lastIndexOf(QChar('.'));
    if (dot_index > 0) {
        return filename.left(dot_index);
    }
    else {
        return filename;
    }
}

QString getFileExtname(const QString &filepath)
{
    const int dot_index = filepath.lastIndexOf(QChar('.'));
    if (dot_index > 0) {
        return filepath.mid(dot_index + 1).toLower();
    }

    return "";
}

qint64 getFileSize(const QString &filepath)
{
    QFileInfo info(filepath);
    if (info.exists()) {
        return info.size();
    }
    else {
        return 0;
    }
}

bool createDirs(const QString &dirpath)
{
    return QDir(dirpath).mkpath(".");
}

bool createParentDirs(const QString &filepath)
{
    return QFileInfo(filepath).absoluteDir().mkpath(".");
}

bool copyMode(const char *src_file, const char *dest_file)
{
    struct stat st;
    if (stat(src_file, &st) == -1) {
        return false;
    }

    const mode_t mode = st.st_mode & 0777;
    return (chmod(dest_file, mode) == 0);
}

bool copyFile(const QString &src_file, const QString &dest_file, bool overwrite)
{
    QFile dest(dest_file);
    if (dest.exists()) {
        if (overwrite) {
            if (!dest.remove()) {
                qCritical() << "Failed to remove:" << dest_file;
                return false;
            }
        }
        else {
            qCritical() << dest_file << "exists but is not overwritten";
            return false;
        }
    }

    return QFile::copy(src_file, dest_file);
}

bool copyFolder(const QString src_dir, const QString& dest_dir, bool recursive)
{
    QDirIterator::IteratorFlag iter_flag;
    if (recursive) {
        iter_flag = QDirIterator::Subdirectories;
    }
    else {
        iter_flag = QDirIterator::NoIteratorFlags;
    }

    QDirIterator iter(src_dir, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, iter_flag);
    QFileInfo src_info;
    QString dest_filepath;
    bool ok = true;
    if (!QDir(dest_dir).exists()) {
        ok = createDirs(dest_dir);
    }

    while (ok && iter.hasNext()) {
        src_info = iter.next();
        dest_filepath = iter.filePath().replace(src_dir, dest_dir);
        if (src_info.isDir()) {
            if (!QDir(dest_filepath).exists()) {
                ok = createDirs(dest_filepath);
            }
            if (ok) {
                ok = copyMode(iter.filePath().toStdString().c_str(), dest_filepath.toStdString().c_str());
            }
        }
        else if (src_info.isFile()) {
            if (QFile::exists(dest_filepath)) {
                // Remove old file first.
                QFile::remove(dest_filepath);
            }
            ok = QFile::copy(iter.filePath(), dest_filepath);
            if (ok) {
                ok = copyMode(iter.filePath().toStdString().c_str(),
                dest_filepath.toStdString().c_str());
            }
        }
        else if (src_info.isSymLink()) {
            if (QFile::exists(dest_filepath)) {
                QFile::remove(dest_filepath);
            }
            ok = QFile::link(src_info.canonicalFilePath(), dest_filepath);
        }
    }

    return ok;
}

bool readTextFile(const QString &path, QString &content)
{
    QFile file(path);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream text_stream(&file);
            content = text_stream.readAll();
            file.close();
            return true;
        }
    }

    return false;
}

bool writeTextToFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream text_stream(&file);
        text_stream << content;
        text_stream.flush();
        file.close();
        return true;
    }

    return false;
}

PrefManager::PrefManager(QObject *parent) : QObject(parent)
{

}

PrefManager::~PrefManager()
{
}


bool PrefManager::setDockerDefaultIpAddress(const QString &value)
{
    bool ret = false;
    if (value.contains("/")) {
        QFile file(dockerIpFile);
        if (!file.exists()){
            if (file.open(QIODevice::WriteOnly)) {
               file.close();
            }
        }
        if (file.open(QIODevice::ReadWrite)) {
            QJsonObject jsonObject;
            jsonObject.insert("bip", value);
            QJsonDocument jsonDoc;
            jsonDoc.setObject(jsonObject);
            file.write(jsonDoc.toJson());
            file.close();
            ret = true;
        }
    }

    return ret;
}

// 将指定项添加到json文件里
bool PrefManager::addGameToWhiteList(const QString &appName, const QString &pkgName)
{
    bool ret = false;
    if (!isInJson(pkgName)) {
        QString content;
        QFile file;
        file.setFileName(gamesFile);
        file.open(QIODevice::ReadOnly| QIODevice::Text);
        content = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
        if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            if (jsonDocument.isObject()) {
                QJsonObject obj = jsonDocument.object();
                QJsonArray array;
                if (obj.contains("games")) {
                    QJsonValue arrayVaule = obj.value("games");
                    if (arrayVaule.isArray()) {
                        array = arrayVaule.toArray();
                    }
                }

                QJsonObject addObj;
                addObj.insert("pkgname", pkgName);
                addObj.insert("appname", appName);
                array.append(addObj);

                QJsonObject objTop;
                objTop.insert("games", array);
                jsonDocument.setObject(objTop);
                file.setFileName(gamesFile);
                file.open(QIODevice::WriteOnly | QIODevice::Text);
                file.write(jsonDocument.toJson());
                file.close();
                ret = true;
            }
        }
    }
    else {
        ret = true;
    }

    return ret;
}

// 将指定项从json文件里删除
bool PrefManager::removeGameFromWhiteList(const QString &pkgName)
{
    bool ret = false;

    if (isInJson(pkgName)) {
        QString content;
        QFile file;
        file.setFileName(gamesFile);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        content = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
        if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            if (jsonDocument.isObject()) {
                QJsonObject obj = jsonDocument.object();
                if (obj.contains("games")) {
                    QJsonValue arrayVaule = obj.value("games");
                    if (arrayVaule.isArray()) {
                        QJsonArray array = arrayVaule.toArray();
                        for (int i = 0;i<array.size();i++) {
                            QJsonValue value = array.at(i);
                            QJsonObject child = value.toObject();
                            QString pkgname = child["pkgname"].toString();
                            if (pkgName == pkgname) {
                                array.removeAt(i);

                                QJsonObject objTop;
                                objTop.insert("games", array);
                                jsonDocument.setObject(objTop);
                                file.setFileName(gamesFile);
                                file.open(QIODevice::WriteOnly | QIODevice::Text);
                                file.write(jsonDocument.toJson());
                                file.close();
                                ret = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        ret = true;
    }

    return ret;
}

bool PrefManager::removeAppFromWarningList(const QString &pkgName)
{
    bool ret = false;

    if (!pkgName.isEmpty()) {
        QString content;
        QFile file;
        file.setFileName(warningFile);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        content = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
        if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            if (jsonDocument.isObject()) {
                QJsonObject obj = jsonDocument.object();
                if (obj.contains("games")) {
                    QJsonValue gamesarrayVaule = obj.value("games");
                    QJsonValue payarrayVaule = obj.value("pay");
                    QJsonArray gamesarray = gamesarrayVaule.toArray();
                    QJsonArray payarray = payarrayVaule.toArray();
                    if (gamesarrayVaule.isArray()) {
                        for (int i = 0;i<gamesarray.size();i++) {
                            QJsonValue value = gamesarray.at(i);
                            QJsonObject child = value.toObject();
                            QString pkgname = child["pkgName"].toString();
                            if (pkgName == pkgname) {
                                gamesarray.removeAt(i);
                                ret = true;

                                QJsonObject objTop;
                                objTop.insert("games", gamesarray);
                                objTop.insert("pay",payarray);
                                jsonDocument.setObject(objTop);
                                file.setFileName(warningFile);
                                file.open(QIODevice::WriteOnly | QIODevice::Text);
                                file.write(jsonDocument.toJson());
                                file.close();
                                break;
                            }
                        }
                    }
                    if (payarrayVaule.isArray()) {
                        for (int i = 0; i < payarray.size(); i++) {
                            QJsonValue value = payarray.at(i);
                            QJsonObject child = value.toObject();
                            QString pkgname = child["pkgName"].toString();
                            if (pkgName == pkgname) {
                                payarray.removeAt(i);
                                ret = true;

                                QJsonObject objTop;
                                objTop.insert("games", gamesarray);
                                objTop.insert("pay",payarray);
                                jsonDocument.setObject(objTop);
                                file.setFileName(warningFile);
                                file.open(QIODevice::WriteOnly | QIODevice::Text);
                                file.write(jsonDocument.toJson());
                                file.close();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;
}

bool PrefManager::setKmreAutoStart(const QString &checked)
{
    bool ret = false;
    const QString startapp = "/etc/xdg/autostart/startapp.desktop";
    if (checked == "false") {
        QFile desktopFp(startapp);
        if (desktopFp.exists()) {
            desktopFp.remove();
            ret = true;
        }
    }
    if (checked == "true") {
        QFile file(startapp);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n"
                "Name=startapp\n"
                "Comment=Start Kmre\n"
                "Exec=/usr/bin/startapp start_kmre_silently\n"
                "Terminal=false\n"
                "Type=Application\n"
                "StartupNotify=false\n"
                "X-GNOME-Autostart-Phase=Applications\n"
                "X-GNOME-Autostart-Notify=true\n"
                "X-GNOME-Autostart-Delay=8\n"
                "NoDisplay=true\n"
                "Categories=\n"
                << endl;
            out.flush();
            file.close();
            ret = true;
        }
    }
    return ret;
//                "X-GNOME-Autostart-Notify=" + checked + "\n"
}

bool PrefManager::setPhoneInfo(const QString &localpath, const QString &vendor, const QString &brand, const QString &name, const QString &model, const QString &equip, const QString &serialno, const QString &imei)
{
    bool ret = false;
    const QString localPath = localpath;
    QFile file(localPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream out(&file);
    QString str;
    QString allstr;
    bool hasvendor = false;
    bool hasbrand = false;
    bool hasname = false;
    bool hasmodel = false;
    bool hasequip = false;
    bool hasserialno = false;
    bool hasimei = false;
    QString vendorstr = "ro.custom.manufacturer=";
    QString brandstr = "ro.custom.brand=";
    QString namestr = "ro.custom.product=";
    QString modelstr = "ro.custom.model=";
    QString equipstr = "ro.custom.device=";
    QString serialnostr = "ro.serialno=";
    QString imeistr = "ro.custom.imei=";
    while (!out.atEnd()) {
        str = out.readLine();
        if (str.mid(0,23) == vendorstr) {
            allstr = allstr+ QString("ro.custom.manufacturer=") + vendor;
            allstr +=QString('\n');
            hasvendor = true;
        }
        else if (str.mid(0,16) == brandstr) {
            allstr = allstr+ QString("ro.custom.brand=") + brand;
            allstr +=QString('\n');
            hasbrand = true;
        }
        else if (str.mid(0,18) == namestr) {
            allstr = allstr+ QString("ro.custom.product=") + name;
            allstr +=QString('\n');
            hasname = true;
        }
        else if (str.mid(0,16) == modelstr) {
            allstr = allstr+ QString("ro.custom.model=") + model;
            allstr +=QString('\n');
            hasmodel = true;
        }
        else if (str.mid(0,17) == equipstr) {
            allstr = allstr+ QString("ro.custom.device=") + equip;
            allstr +=QString('\n');
            hasequip = true;
        }
        else if (str.mid(0,12) == serialnostr) {
            allstr = allstr+ QString("ro.serialno=") + serialno;
            allstr +=QString('\n');
            hasserialno = true;
        }
        else if (str.mid(0,15) == imeistr) {
            allstr = allstr+ QString("ro.custom.imei=") + imei;
            allstr +=QString('\n');
            hasimei = true;
        }
        else {
            allstr += str;
            allstr += QString('\n');
        }
    }
    if (!hasvendor) {
        allstr = allstr + QString("ro.custom.manufacturer=") + vendor;
        allstr += QString('\n');
    }
    if (!hasbrand) {
        allstr = allstr + QString("ro.custom.brand=") + brand;
        allstr += QString('\n');
    }
    if (!hasname) {
        allstr = allstr + QString("ro.custom.product=") + name;
        allstr += QString('\n');
    }
    if (!hasmodel) {
        allstr = allstr + QString("ro.custom.model=") + model;
        allstr += QString('\n');
    }
    if (!hasequip) {
        allstr = allstr + QString("ro.custom.device=") + equip;
        allstr += QString('\n');
    }
    if (!hasserialno) {
        allstr = allstr + QString("ro.serialno=") + serialno;
        allstr += QString('\n');
    }
    if (!hasimei) {
        allstr = allstr + QString("ro.custom.imei=") + imei;
        allstr += QString('\n');
    }
    file.close();
    file.open(QIODevice::WriteOnly |QIODevice::Text);
    if (file.isOpen()) {
        ret = true;
    }
    QTextStream in(&file);
    in << allstr;
    in.flush();
    file.close();
    return ret;
}

int PrefManager::copyLogFiles(const QString &username, const QString &userid, const QString &logPath)
{
    const QString kmreLog = "/var/log/kmre.log";
    int ret = -1;
    if (logPath.isEmpty() || logPath.isNull() || !logPath.contains("/KmreLog/")) {
        return ret;
    }

    QDir dir(logPath);
    if(!dir.exists()) {
        return ret;
    }

    QString dataPath = QString("/var/lib/kmre/kmre-%1-%2/data").arg(userid).arg(username);
    bool ok = false;

    ok = runCmd("/bin/cp", {"-a", QString("%1/klog/").arg(dataPath), logPath});
    ok = runCmd("/bin/cp", {"-a", QString("%1/anr/").arg(dataPath), logPath});
    ok = runCmd("/bin/cp", {"-a", QString("%1/system/dropbox/").arg(dataPath), logPath});
    ok = runCmd("/bin/cp", {"-a", QString("%1/tombstones/").arg(dataPath), logPath});

    if (QFileInfo(kmreLog).exists()) {
        ok = runCmd("/bin/cp", {"-a", kmreLog, logPath});
    }

    ok = runCmd("/bin/chmod", {"777", "-R", logPath});
    ok = runCmd("/bin/chmod", {"777", "-R", logPath});
    if (QFile::exists("/bin/zip")) {
        ok = runCmd("/bin/zip", {"-r", "-q", "-j", QString("%1.zip").arg(logPath), logPath});
    }
    else {
        ok = runCmd("/usr/bin/zip", {"-r", "-q", "-j", QString("%1.zip").arg(logPath), logPath});
    }
    ok = runCmd("/bin/chown", {"-hR", QString("%1:%1").arg(username), QString("%1.zip").arg(logPath)});

    Q_UNUSED(ok);

    ret = 0;
    return ret;
}

void PrefManager::quilt()
{
    QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
    exit(0);
}

