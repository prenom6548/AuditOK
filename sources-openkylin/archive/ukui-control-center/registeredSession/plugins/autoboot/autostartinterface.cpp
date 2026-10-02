/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "autostartinterface.h"
#include <QDir>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDebug>
#include <QSettings>

#include "../../universalinterface.h"

#define LOCAL_CONFIG_DIR           "/.config/autostart/"
#define SYSTEM_CONFIG_DIR          "/etc/xdg/autostart/"
#define USR_CONFIG_DIR             "/usr/share/applications/"
#define UKCC_AUTOAPP_KEY           "autoapp-list"

AutoStartInterface::AutoStartInterface(QObject *parent) : UKUI::ServiceObject(parent)
{
    mUkccGsettings = UniversalInterface::self()->ukccGsettings();
    mStyleGsettings = UniversalInterface::self()->ukuiStyleGsettings();

    if (mUkccGsettings) {
        connect(mUkccGsettings, &QGSettings::changed, [=](const QString &key){
            emit changed(key);
        });
        if (mUkccGsettings->keys().contains("autoappList")) {
            m_blackList = mUkccGsettings->get("autoapp-list").toStringList();
        }
    }
    if (mStyleGsettings)
        connect(mStyleGsettings, &QGSettings::changed, [=](const QString &key){
            emit changed(key);
        });
    mSysServiceMap = getServiceMap();
    mAllAppMap = getAllAppMap();
}

AutoApp AutoStartInterface::getAppInfo(const QString &filepath)
{
    AutoApp app;
    QFileInfo file = QFileInfo(filepath);
    QString mainSection = "Desktop Entry";
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(filepath, KConfig::NoGlobals);
    KConfigGroup cfg(sharedConfig, mainSection);
    app.bname = file.fileName();
    app.name = cfg.readEntry("Name");
    app.icon = cfg.readEntry("Icon");
    if (app.bname == "sogouImeService.desktop")
       app.icon = "/opt/sogouimebs/files/share/resources/skin/logo/logo.png";
    app.hidden = cfg.readEntry("Hidden", false);
    return app;
}

QStringList AutoStartInterface::getAppList()
{
    QStringList list;
    if (mUkccGsettings->keys().contains("autoappList"))
        list = mUkccGsettings->get(UKCC_AUTOAPP_KEY).toStringList();
    return list;
}

QMap <QString, QVariant> AutoStartInterface::getStatusMap()
{
    QMap <QString, QVariant> map;
    for (QMap<QString, AutoApp>::iterator it = mAllAppMap.begin(); it != mAllAppMap.end(); it++)
        map.insert(it.key(), QVariant::fromValue(it.value()));
    return map;
}

QMap <QString , AutoApp> AutoStartInterface::getLocalAppMap()
{
    QDir localdir(QString(QDir::homePath()+LOCAL_CONFIG_DIR).toUtf8());
    QStringList filters;
    filters<<QString("*.desktop");
    localdir.setFilter(QDir::Files | QDir::NoSymLinks);
    localdir.setNameFilters(filters);

    QMap <QString , AutoApp> map;
    for (uint i = 0; i < localdir.count(); i++) {
        QString file_name = localdir[i];  // 文件名称
        AutoApp app;
        app = setInformation(QDir::homePath() + LOCAL_CONFIG_DIR + file_name);
        app.position = Pos::LOCALPOS;
        map.insert(app.bname, app);
    }
    return map;

}

AutoApp AutoStartInterface::setInformation(QString filepath)
{
    AutoApp app;
    QSettings* desktopFile = new QSettings(filepath, QSettings::IniFormat);
    QString icon, only_showin, not_show_in;
    if (desktopFile) {
        desktopFile->setIniCodec("utf-8");

        QFileInfo file = QFileInfo(filepath);
        app.bname = file.fileName();
        app.path = filepath;
        app.exec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
        app.icon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
        app.hidden = desktopFile->value(QString("Desktop Entry/Hidden")).toBool();
        app.no_display = desktopFile->value(QString("Desktop Entry/NoDisplay")).toBool();
        only_showin = desktopFile->value(QString("Desktop Entry/OnlyShowIn")).toString();
        not_show_in = desktopFile->value(QString("Desktop Entry/NotShowIn")).toString();
        bool mshow = true;
        if (app.bname == "sogouImeService.desktop") {
            icon = "/opt/sogouimebs/files/share/resources/skin/logo/logo.png";
        }
        if (only_showin != nullptr) {
            if (!only_showin.contains("UKUI")) {
                mshow = false;
            }
        }
        if (not_show_in != nullptr) {
            if (not_show_in.contains("UKUI")) {
                mshow = false;
            }
        }
        app.shown = mshow;
        delete desktopFile;
        desktopFile = nullptr;
    }
    //通过glib库函数获取Name字段，防止特殊情况（含有字段X-Ubuntu-Gettext-Domain）
    GKeyFile *keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, filepath.toLatin1().data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);
        return app;
    }
    app.name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                            G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    g_key_file_free(keyfile);

    return app;

}

QMap <QString , AutoApp> AutoStartInterface::getServiceMap()
{
    qDBusRegisterMetaType<AutoApp>();
    QDir systemdir(QString(SYSTEM_CONFIG_DIR).toUtf8());
    QStringList filters;
    filters<<QString("*.desktop");
    systemdir.setFilter(QDir::Files | QDir::NoSymLinks);
    systemdir.setNameFilters(filters);
    QMap <QString , AutoApp> map;
    for (uint32_t i = 0; i < systemdir.count(); i++) {
        QString file_name = systemdir[i];
        if (m_blackList.contains(file_name)) {
            qDebug() << Q_FUNC_INFO << "hit blacklist, skip";
            continue;
        }
        AutoApp app;
        app = setInformation(SYSTEM_CONFIG_DIR + file_name);
        app.position = SYSTEMPOS;
        if (app.no_display || app.name.isEmpty() || !app.shown) {
            continue;
        }
        qDebug() << Q_FUNC_INFO << "add system app" << file_name << app.bname << m_blackList;
        map.insert(app.bname, app);
    }
    return map;
}

QMap<QString, AutoApp> AutoStartInterface::getAllAppMap()
{
    QMap<QString, AutoApp> map;
    QMap<QString, AutoApp>::iterator it = mSysServiceMap.begin();
    for (; it != mSysServiceMap.end(); it++) {
        map.insert(it.key(), it.value());
    }

    QMap<QString, AutoApp> localMap = getLocalAppMap();
    QMap<QString, AutoApp>::iterator localit = localMap.begin();
    for (; localit != localMap.end(); localit++) {
        if (map.contains(localit.key())) {
            QMap<QString, AutoApp>::iterator updateit = map.find(localit.key());
            if (localit.value().hidden != updateit.value().hidden) {
                updateit.value().hidden = localit.value().hidden;
                updateit.value().path = localit.value().path;
                if (mSysServiceMap.contains(localit.key())) {
                    updateit.value().position = Pos::ALLPOS;
                }
            }
        } else {
            map.insert(localit.key(), localit.value());
        }
    }
    return map;
}

bool AutoStartInterface::deleteLocalFile(const QString &filename)
{
    const QString safeName = sanitizeAutostartFilename(filename);
    if (safeName.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "reject unsafe filename" << filename;
        return false;
    }

    const QString localDir = QDir::homePath() + LOCAL_CONFIG_DIR;
    const QString dstpath = localDir + safeName;
    if (!isPathInsideDir(dstpath, localDir) || !QDir().exists(dstpath)) {
        qDebug() << Q_FUNC_INFO << "filename is empty or dstpath not exist" << safeName << dstpath;
        return false;
    }

    QMap<QString, AutoApp>::iterator updateit = mAllAppMap.find(safeName);
    if (updateit == mAllAppMap.end()) {
         qCritical() << "mAllAppMap Data Error when delete local file";
    } else {
        if (updateit.value().position == LOCALPOS) {
            mAllAppMap.remove(safeName);
        } else if (updateit.value().position == ALLPOS) {
            //当系统应用的自启动开关打开时，将其属性变为SYSTEMPOS，将其从~/.config/autostart目录下删除
            QMap<QString, AutoApp>::iterator appit = mSysServiceMap.find(safeName);
            if (appit == mSysServiceMap.end()) {
                qDebug() << "appMaps Data Error when delete local file";
            } else {
                updateit.value().hidden = appit.value().hidden;
                updateit.value().path = appit.value().path;
            }
            updateit.value().position = SYSTEMPOS;
        }
    }

    if (QFile::remove(dstpath)) {
        qDebug() << Q_FUNC_INFO << "remove file suc" << dstpath;
        return true;
    }
    return false;
}

bool AutoStartInterface::copyFiletoLocal(const QString &filename)
{
    const QString safeName = sanitizeAutostartFilename(filename);
    if (safeName.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "reject unsafe filename" << filename;
        return false;
    }

    QMap<QString, AutoApp>::iterator it = mSysServiceMap.find(safeName);
    if (it == mSysServiceMap.end()) {
        qWarning() << Q_FUNC_INFO << "filename not in system map" << safeName;
        return false;
    }

    const QString srcPath = it.value().path;
    const QString localDir = QDir::homePath() + LOCAL_CONFIG_DIR;
    const QString dstPath = localDir + safeName;
    const bool srcOk = isPathInsideDir(srcPath, QString(SYSTEM_CONFIG_DIR))
            || isPathInsideDir(srcPath, QString(USR_CONFIG_DIR));
    if (!isPathInsideDir(dstPath, localDir) || !srcOk) {
        qWarning() << Q_FUNC_INFO << "reject path outside allowed dirs" << srcPath << dstPath;
        return false;
    }

    qDebug() << Q_FUNC_INFO << "copyFiletoLocal srcPath" << srcPath << "dstPath" << dstPath;
    if (!QFile::exists(dstPath)) {
        if (!QFile::copy(srcPath, dstPath)) {
            return false;
        }
    }

    //将复制的文件权限改为可读写
    QFile(dstPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    AutoApp addapp;
    addapp = setInformation(dstPath);
    addapp.position = ALLPOS;

    // localappMaps.insert(addapp.bname, addapp);

    QMap<QString, AutoApp>::iterator updateit = mAllAppMap.find(safeName);
    if (updateit != mAllAppMap.end()) {
        updateit.value().position = ALLPOS;
        updateit.value().path = dstPath;
    }

    return true;
}

gboolean AutoStartInterface::keyFileToTile(GKeyFile *keyfile, const gchar *path)
{
    GError *werror;
    gchar *data;
    gsize length;
    gboolean res;

    werror = NULL;
    data = g_key_file_to_data(keyfile, &length, &werror);

    if (werror)
        return FALSE;

    res = g_file_set_contents(path, data, length, &werror);
    g_free(data);

    if (werror)
        return FALSE;

    return res;
}

bool AutoStartInterface::setAppHiddenInfo(const QString &filename, bool hidden)
{
    const QString safeName = sanitizeAutostartFilename(filename);
    if (safeName.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "reject unsafe filename" << filename;
        return false;
    }

    const QString localDir = QDir::homePath() + LOCAL_CONFIG_DIR;
    const QString dstpath = localDir + safeName;
    if (!isPathInsideDir(dstpath, localDir) || !QDir().exists(dstpath)) {
        qDebug() << Q_FUNC_INFO << "filename is empty or dstpath not exist, skip";
        return false;
    }
    GKeyFile *keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, dstpath.toUtf8().data(), G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
        g_key_file_free(keyfile);
        qDebug() << Q_FUNC_INFO << "load file failed, skip" << dstpath;
        return false;
    }
    g_key_file_set_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                           G_KEY_FILE_DESKTOP_KEY_HIDDEN, hidden);
    if (!keyFileToTile(keyfile, dstpath.toUtf8().data())) {
        qDebug() << Q_FUNC_INFO << "save file failed, skip" << dstpath;
        return false;
    }

    g_key_file_free(keyfile);
    QMap<QString, AutoApp>::iterator updateit = mAllAppMap.find(safeName);
    if (updateit != mAllAppMap.end())
        updateit.value().hidden = hidden;
    return true;
}

bool AutoStartInterface::addAutobootApp(const QString &filename)
{
    AutoApp app = setInformation(filename);
    app.position = Pos::LOCALPOS;

    QMap<QString, AutoApp>::iterator it = mAllAppMap.begin();
    for (; it != mAllAppMap.end(); it++) {
        if (it.value().name == app.name) {
            qDebug() << Q_FUNC_INFO << "app already in list, skip" << app.name;
            return false;
        }
    }
    qDebug() << Q_FUNC_INFO << "add autoboot" << filename;
    QFileInfo file = QFileInfo(filename);
    QString mFileName = file.fileName();
    QString filepath = QDir::homePath() + LOCAL_CONFIG_DIR + mFileName;
    if (!QFile::exists(filepath)) {
        if (!QFile::copy(filename, filepath)) {
            qDebug() << Q_FUNC_INFO << "add autoboot app copy failed";
            return false;
        }
    }
    QFile(filepath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    it = mAllAppMap.find(mFileName);
    if (it == mAllAppMap.end()) {
        AutoApp app = setInformation(filepath);
        app.position = Pos::LOCALPOS;
        mAllAppMap[mFileName] = app;
    }
    return true;
}

bool AutoStartInterface::setAutobootAllBlack() {
    mSysServiceMap = getServiceMap();
    mAllAppMap = getAllAppMap();
    for (QMap<QString, AutoApp>::iterator it = mAllAppMap.begin(); it != mAllAppMap.end(); it++) {
        if (saveAppStatus(it.key(), false)) {
            qDebug() << Q_FUNC_INFO << "autoboot set desktop" << it.key() << "to not autoboot suc";
        } else {
            qDebug() << Q_FUNC_INFO << "autoboot set desktop" << it.key() << "to not autoboot failed";
        }
    }
    qDebug() << Q_FUNC_INFO << "setAutobootAllBlack before emit changed";
    emit changed("boot");
    return true;
}

bool AutoStartInterface::setAutobootBlacklist(QStringList blacklist) {
    mSysServiceMap = getServiceMap();
    mAllAppMap = getAllAppMap();
    foreach (const QString& desktop, blacklist) {
        if (saveAppStatus(desktop, false)) {
            qDebug() << Q_FUNC_INFO << "autoboot set desktop" << desktop << "to not autoboot suc";
        } else {
            qDebug() << Q_FUNC_INFO << "autoboot set desktop" << desktop << "to not autoboot failed";
        }
    }
    qDebug() << Q_FUNC_INFO << "setAutobootBlacklist before emit changed";
    emit changed("boot");
    return true;
}

bool AutoStartInterface::setAutobootWhitelist(QStringList whitelist) {
    mSysServiceMap = getServiceMap();
    mAllAppMap = getAllAppMap();
    for (QMap<QString, AutoApp>::iterator it = mAllAppMap.begin(); it != mAllAppMap.end(); it++) {
        QString desktopName = it.key();
        if (!whitelist.contains(desktopName)) {
            qDebug() << Q_FUNC_INFO << "setAutobootWhitelist desktop not in whitelist set autoboot false" << desktopName;
            if (saveAppStatus(it.key(), false)) {
                qDebug() << Q_FUNC_INFO << "autoboot set desktop" << it.key() << "to not autoboot suc";
            }
        }
    }
    foreach (const QString& desktopName, whitelist) {
        if (saveAppStatus(desktopName, true)) {
            qDebug() << Q_FUNC_INFO << "autoboot set desktop" << desktopName << "to autoboot suc";
        } else {
            qDebug() << Q_FUNC_INFO << "autoboot set desktop" << desktopName << "to autoboot failed";
            const QString desktops = QString::fromLocal8Bit(qgetenv("XDG_DATA_DIRS"));
            qDebug() << Q_FUNC_INFO << "XDG_DATA_DIRS" << desktops;
            QString absolutePath;
            const auto list = desktops.split(QLatin1Char(':'));
            for (const QString &desktop : list) {
                QStringList pathlist = desktop.split(":");
                qDebug() << Q_FUNC_INFO << "ukcc info" << pathlist << desktop;
                for (QString path : pathlist) {
                    QString desktopDirStr = path + "/applications/";
                    QDir desktopDir = QDir(desktopDirStr);
                    if (!desktopDir.exists()) {
                        qDebug() << Q_FUNC_INFO << "desktopDir not exist, skip" << desktopDirStr;
                        continue;
                    }
                    foreach (QString filename, desktopDir.entryList(QDir::Files | QDir::Dirs)) {
                        QString desktopPath = desktopDirStr + filename;
                        QFileInfo fileInfo(desktopPath);
                        if(fileInfo.isDir() || !filename.endsWith("desktop")) {
                            qDebug() << Q_FUNC_INFO << "not desktop file, skip" << desktopPath;
                            continue;
                        }
                        if (desktopName == filename) {
                            absolutePath = desktopPath;
                            qDebug() << Q_FUNC_INFO << "autoboot desktop" << desktop << "find absolute path" << absolutePath;
                            break;
                        }
                    }
                    if (!absolutePath.isEmpty()) {
                        break;
                    }
                }
                if (!absolutePath.isEmpty()) {
                    break;
                }
            }
            qDebug() << Q_FUNC_INFO << "autoboot desktop" << desktopName << "absolute path" << absolutePath;
            if (!absolutePath.isEmpty() && addAutobootApp(absolutePath)) {
                qDebug() << Q_FUNC_INFO << "autoboot addAutobootApp suc" << desktopName << absolutePath;
            } else {
                qDebug() << Q_FUNC_INFO << "autoboot addAutobootApp failed" << desktopName;
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "setAutobootWhitelist before emit changed";
    emit changed("boot");
    return true;
}

bool AutoStartInterface::saveAppStatus(const QString &filename, bool status)
{
    QMap<QString, AutoApp>::iterator it = mAllAppMap.find(filename);
    if (it == mAllAppMap.end()) {
        return false;
    }
    if (status) { // 开启开机启动
        if (it.value().position == SYSTEMPOS) {
        } else if (it.value().position == ALLPOS) {
            QMap<QString, AutoApp>::iterator appit = mSysServiceMap.find(filename);
            if (!appit.value().hidden) { // 直接删除
               deleteLocalFile(filename);
                // 更新状态
                QMap<QString, AutoApp>::iterator updateit = mAllAppMap.find(filename);
                if (updateit != mAllAppMap.end()) {
                    updateit.value().hidden = false;
                    updateit.value().position = SYSTEMPOS;
                    //路径改为/etc/xdg/autostart
                    updateit.value().path = appit.value().path;
                } else {
                    qDebug() << Q_FUNC_INFO << "Update status failed when start autostart";
                }
            }
        } else if (it.value().position == LOCALPOS) {// 改值("hidden"字段->false)
            setAppHiddenInfo(filename, false);
        }
    } else { // 关闭
        if (it.value().position == SYSTEMPOS) { // 复制后改值，将对应应用的desktop文件从/etc/xdg/autostart目录下复制到~/.config/autostart目录下
            if (copyFiletoLocal(filename)) {
                setAppHiddenInfo(filename, true);
            }
        }  else if (it.value().position == ALLPOS) {// 正常逻辑不应存在该情况,预防处理
            QMap<QString, AutoApp>::iterator appit = mSysServiceMap.find(filename);
            if (appit.value().hidden) {
                deleteLocalFile(filename);
            }
        } else if (it.value().position == LOCALPOS) {// 改值
            setAppHiddenInfo(filename, true);
        }
    }
    return true;
}

bool AutoStartInterface::setApplist(const QStringList &list)
{
    if (mUkccGsettings->keys().contains("autoappList"))
        mUkccGsettings->set(UKCC_AUTOAPP_KEY, list);
    return true;
}

bool AutoStartInterface::getDisplayStatus(const QString &filename)
{
    QString mainSection = "Desktop Entry";
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(filename, KConfig::NoGlobals);
    KConfigGroup cfg(sharedConfig, mainSection);
    return cfg.readEntry("NoDisplay", false);
}

QString AutoStartInterface::sanitizeAutostartFilename(const QString &filename)
{
    if (filename.isEmpty() || filename.contains(QChar('\0'))) {
        return QString();
    }
    // 拒绝带路径分隔符或父目录引用的参数，只接受纯文件名
    if (filename.contains('/') || filename.contains('\\') || filename.contains("..")) {
        return QString();
    }
    const QString baseName = QFileInfo(filename).fileName();
    if (baseName.isEmpty() || baseName != filename || baseName == "." || baseName == "..") {
        return QString();
    }
    return baseName;
}

bool AutoStartInterface::isPathInsideDir(const QString &filePath, const QString &dirPath)
{
    const QString absFile = QFileInfo(filePath).absoluteFilePath();
    QString absDir = QDir(dirPath).absolutePath();
    if (!absDir.endsWith(QLatin1Char('/'))) {
        absDir.append(QLatin1Char('/'));
    }
    // 目标文件可能尚不存在，用 absoluteFilePath；已存在时再校验 canonical 防 symlink 逃逸
    if (!absFile.startsWith(absDir)) {
        return false;
    }
    const QFileInfo fi(filePath);
    if (fi.exists()) {
        const QString canonFile = fi.canonicalFilePath();
        const QString canonDir = QDir(dirPath).canonicalPath() + QLatin1Char('/');
        if (canonFile.isEmpty() || !canonFile.startsWith(canonDir)) {
            return false;
        }
    }
    return true;
}

const QDBusArgument &operator<<(QDBusArgument &argument, const AutoApp &app)
{
    argument.beginStructure();
    argument << app.bname;
    argument << app.icon;
    argument << app.name;
    argument << app.hidden;
    argument << app.position;

    argument << app.path;
    argument << app.no_display;
    argument << app.shown;
    argument << app.comment;
    argument << app.exec;
    argument << app.description;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AutoApp &app)
{
    argument.beginStructure();
    argument >> app.bname;
    argument >> app.icon;
    argument >> app.name;
    argument >> app.hidden;
    argument >> app.position;

    argument >> app.path;
    argument >> app.no_display;
    argument >> app.shown;
    argument >> app.comment;
    argument >> app.exec;
    argument >> app.description;
    argument.endStructure();
    return argument;
}
