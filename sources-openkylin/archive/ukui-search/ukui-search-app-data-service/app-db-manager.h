/*
 *
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
 *
 */
#ifndef APPDBMANAGER_H
#define APPDBMANAGER_H
#include <QDir>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QApplication>
#include <QDateTime>
#include <QMutex>
#include <QSettings>
#include <QThread>
#include <QDBusInterface>
#include <QDBusVariant>
#include "pending-app-info-queue.h"
#include "file-system-watcher.h"
#include "application-property.h"

#define APP_DATABASE_PATH QDir::homePath()+"/.config/org.ukui/ukui-search/appdata/"
#define APP_DATABASE_NAME "app-info.db"
#define CONNECTION_NAME QLatin1String("ukss-appdb-connection")
/**
 * changelog(1.2 to 1.3): Add the START_UP_WMCLASS field in order to find desktop file path by the windowClassClass.
 * changelog(1.3 to 1.4): Add the KEYWORDS & LOCAL_KEYWORDS field in order to find application conveniently.
 * changelog(1.4 to 1.5): add the judgment of OnlyShowIn field ,update DONT_DISPLAY field.
 * changelog(1.5 to 1.6): add the judgment of X-Ubuntu-Gettext-Domain field ,update NAME_CN field and the fields that pinyin related.
 */

static const QString APP_DATABASE_VERSION = QStringLiteral("1.6");

namespace UkuiSearch {
class ApplicationDirWatcherHelper;
/**
 * @brief The AppDBManager class
 * 功能：1.遍历并且监听desktop文件目录，建立并且维护应用信息数据库。
 *      2.监听应用安装，打开事件，收藏等事件，更新数据库
 */

class AppDBManager : public QThread
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface","org.ukui.search.appDBManager")

    enum APP_LOCK_STATE{
       APP_UNLOCK = 0,
       APP_LOCK
    };

public:
    static AppDBManager *getInstance();

    //刷新数据库数据
    void refreshAllData2DB(const QStringList &appPaths, bool dbVersionNeedUpdate = false);

    //获取desktop文件的md5值
    QString getAppDesktopMd5(const QString &desktopfd);

    bool startTransaction();
    bool startCommit();

    bool handleDBItemInsert(const QString &desktopFilePath);
    bool handleDBItemUpdate(const QString &desktopFilePath);
    bool handleDBItemDelete(const QString &desktopFilePath);

    bool handleLocaleDataUpdate(const QString &desktopFilePath);
    bool handleLaunchTimesUpdate(const QString &desktopFilePath, int num = 1);
    bool handleFavoritesStateUpdate(const QString &desktopFilePath, const uint num);
    bool handleTopStateUpdate(const QString &desktopFilePath, const uint num);
    bool handleLockStateUpdate(const QString &desktopFilePath, int num);
    bool handleDataBaseRefresh(const QStringList &appPaths, bool dbVersionNeedUpdate);
    bool handleSdkSymlink(const QString &symlinkPath);

    bool handleValueSet(const ApplicationInfoMap &appInfoMap);

public Q_SLOTS:
    //通过pid查找对应的desktop文件
    QString tranPidToDesktopFp(uint pid);
    QString desktopFilePathFromName(const QString &desktopFileName);
    QString tranWinIdToDesktopFilePath(const QDBusVariant &id);

    //对数据库单条所有信息进行增删改
    void insertDBItem(const QString &desktopfd);
    void updateDBItem(const QString &desktopfd);
    void deleteDBItem(const QString &desktopfd);

    //对数据库某字段进行update
    void updateLocaleData(const QString &desktopFilePath);
    void updateLaunchTimes(const QString &desktopFilePath);
    void updateFavoritesState(const QString &desktopFilePath, uint num = 1);
    void updateTopState(const QString &desktopFilePath, uint num = 1);
    void updateSdkSymLink(const QString &symLinkPath);

    void setValue(const ApplicationInfoMap &infos2BSet);

private Q_SLOTS:
    void handleAppLaunched(QString desktopFilePath);

protected:
    void run() override;

private:
    explicit AppDBManager(QObject *parent = nullptr);
    ~AppDBManager();

    //加载指定路径path中的所有desktop文件路径到infolist中
    void loadDesktopFilePaths(QString path, QFileInfoList &infolist);

    //数据库查找指定字段不存在则添加到最后并设置初始值
    bool addItem2BackIfNotExist(QString itemName, QString itemDataType, QVariant defult = QVariant());

    //链接数据库
    bool openDataBase(QSqlDatabase &database, bool memory = false);
    //刷新数据库
    void refreshDataBase();
    //关闭数据库，断开链接
    void closeDataBase();
    //修复数据库路径拼接错误
    bool repairDBDesktopPath();

    //创建数据库字段
    void buildAppInfoDB();

    //初始化fileSystemWatcher
    void initFileSystemWatcher(const QStringList& appDirs);

    QSqlDatabase saveToFileDb(bool needSave);
    bool loadFromFileDb();

    static QMutex s_mutex;
    bool m_localeChanged;
    bool m_dbVersionNeedUpdate = false;

    QSettings *m_lastLocaleNameQsettings = nullptr;
    QSettings *m_dbVersionQsettings = nullptr;

    QDBusInterface *m_processManagerInterface = nullptr;

    QStringList m_appPaths;
    QSqlDatabase m_database;
    FileSystemWatcher *m_watcher = nullptr;

    ApplicationDirWatcherHelper *m_helper = nullptr;

    QHash<QString, QString> m_symLinks;

    //数据库当前所有字段
    QMap<QString, QString> m_namesOfAppinfoTable = {
        {"DESKTOP_FILE_PATH", "TEXT"},
        {"MODIFYED_TIME", "TEXT"},
        {"INSERT_TIME","TEXT"},
        {"LOCAL_NAME", "TEXT"},
        {"NAME_EN", "TEXT"},
        {"NAME_ZH", "TEXT"},
        {"PINYIN_NAME", "TEXT"},
        {"FIRST_LETTER_OF_PINYIN", "TEXT"},
        {"FIRST_LETTER_ALL", "TEXT"},
        {"ICON", "TEXT"},
        {"TYPE", "TEXT"},
        {"CATEGORY", "TEXT"},
        {"EXEC", "TEXT"},
        {"COMMENT", "TEXT"},
        {"MD5", "TEXT"},
        {"LAUNCH_TIMES", "INT"},
        {"FAVORITES", "INT"},
        {"LAUNCHED", "INT"},
        {"TOP", "INT"},
        {"LOCK", "INT"},
        {"DONT_DISPLAY", "INT"},
        {"AUTO_START", "INT"},
        {"START_UP_WMCLASS", "TEXT"},
        {"KEYWORDS", "TEXT"},
        {"LOCAL_KEYWORDS", "TEXT"}
    };

    //应用黑名单
    QStringList m_excludedDesktopfiles = {
        "/usr/share/applications/software-properties-livepatch.desktop",
        "/usr/share/applications/mate-color-select.desktop",
        "/usr/share/applications/blueman-adapters.desktop",
        "/usr/share/applications/blueman-manager.desktop",
        "/usr/share/applications/mate-user-guide.desktop",
        "/usr/share/applications/nm-connection-editor.desktop",
        "/usr/share/applications/debian-uxterm.desktop",
        "/usr/share/applications/debian-xterm.desktop",
        "/usr/share/applications/im-config.desktop",
        "/usr/share/applications/fcitx.desktop",
        "/usr/share/applications/fcitx-configtool.desktop",
        "/usr/share/applications/onboard-settings.desktop",
        "/usr/share/applications/info.desktop",
        "/usr/share/applications/ukui-power-preferences.desktop",
        "/usr/share/applications/ukui-power-statistics.desktop",
        "/usr/share/applications/software-properties-drivers.desktop",
        "/usr/share/applications/software-properties-gtk.desktop",
        "/usr/share/applications/gnome-session-properties.desktop",
        "/usr/share/applications/org.gnome.font-viewer.desktop",
        "/usr/share/applications/xdiagnose.desktop",
        "/usr/share/applications/gnome-language-selector.desktop",
        "/usr/share/applications/mate-notification-properties.desktop",
        "/usr/share/applications/transmission-gtk.desktop",
        "/usr/share/applications/mpv.desktop",
        "/usr/share/applications/system-config-printer.desktop",
        "/usr/share/applications/org.gnome.DejaDup.desktop",
        "/usr/share/applications/yelp.desktop",
        "/usr/share/applications/peony-computer.desktop",
        "/usr/share/applications/peony-home.desktop",
        "/usr/share/applications/peony-trash.desktop",

        //v10
        "/usr/share/applications/mate-about.desktop",
        "/usr/share/applications/time.desktop",
        "/usr/share/applications/network.desktop",
        "/usr/share/applications/shares.desktop",
        "/usr/share/applications/mate-power-statistics.desktop",
        "/usr/share/applications/display-im6.desktop",
        "/usr/share/applications/display-im6.q16.desktop",
        "/usr/share/applications/openjdk-8-policytool.desktop",
        "/usr/share/applications/kylin-io-monitor.desktop",
        "/usr/share/applications/wps-office-uninstall.desktop",

        //原本额外排除的目录，不知道额外的原因，有可能之后有问题--bjj20220621
        "/usr/share/applications/screensavers"
    };

Q_SIGNALS:
    //操作数据库
    void appDBItemUpdate(const ApplicationInfoMap&);
    void appDBItemUpdateAll(const QString&);
    void appDBItemAdd(const QString&);
    void appDBItemDelete(const QString&);
    void finishHandleAppDB();

    //定时器操作
    void startTimer();
    void maxProcessTimerStart();
    void stopTimer();
};

class ApplicationDirWatcherHelper : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationDirWatcherHelper(FileSystemWatcher *watcher,QObject *parent = nullptr);
    ~ApplicationDirWatcherHelper();
    void addPath(const QString& path);
Q_SIGNALS:
    void desktopFilePathAdded(const QString&);
private:
    FileSystemWatcher *m_watcher = nullptr;
    QMap<QString, FileSystemWatcher *> m_watcherMap;
};
}

#endif // APPDBMANAGER_H
