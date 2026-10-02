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
 */

#include <QMutexLocker>
#include <QCryptographicHash>
#include <QFile>
#include <QSqlDriver>
#include <sqlite3.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6xdg/XdgDesktopFile>
#include <QRegularExpression>
#define REG_EXP QRegularExpression
#else
#include <qt5xdg/XdgDesktopFile>
#define REG_EXP QRegExp
#endif
#include "appdatabaseadaptor.h"
#include "file-utils.h"
#include "application-property-helper.h"
#include "app-db-manager.h"
#include "app-db-utils.h"
#include "app-data-utils.h"
#include "dbus-connector.h"
#include "sqlite-database-utils.h"

#define LAST_LOCALE_NAME QDir::homePath() + "/.config/org.ukui/ukui-search/appdata/last-locale-name.conf"
#define LOCALE_NAME_VALUE "CurrentLocaleName"
#define APP_DATABASE_VERSION_CONFIG QDir::homePath() + "/.config/org.ukui/ukui-search/appdata/app-database-version.conf"
#define APP_DATABASE_VERSION_VALUE "AppDatabaseVersion"
static const QString AUTOSTART_APP_DESKTOP_PATH = "/etc/xdg/autostart";
static const QString SDK_SYMBOLIC_PREFIX = "kylin-redistribution-symbolic-";

using namespace UkuiSearch;

static AppDBManager *global_instance;
QMutex AppDBManager::s_mutex;

AppDBManager *AppDBManager::getInstance()
{
    if (!global_instance) {
        global_instance = new AppDBManager();
    }
    return global_instance;
}

AppDBManager::AppDBManager(QObject *parent) : QThread(parent)
{
    //链接数据库
    if (openDataBase(m_database, true)) {
        loadFromFileDb();
        bool needSave = !m_database.tables().contains("appInfo", Qt::CaseInsensitive);
        //建数据库
        buildAppInfoDB();
        //修复数据库中的desktop文件路径
        needSave = needSave | repairDBDesktopPath();

        //监听系统语言变化
        m_lastLocaleNameQsettings = new QSettings(LAST_LOCALE_NAME, QSettings::IniFormat);
        m_localeChanged = false;
        m_lastLocaleNameQsettings->beginGroup(LOCALE_NAME_VALUE);
        QString lastLocale = m_lastLocaleNameQsettings->value(LOCALE_NAME_VALUE).toString();
        if (QLocale::system().name().compare(lastLocale)) {
            qDebug() << "I'm going to update the locale name in conf file.";
            if (!lastLocale.isEmpty()) {
                m_localeChanged = true;
            }
            m_lastLocaleNameQsettings->setValue(LOCALE_NAME_VALUE, QLocale::system().name());
        }
        m_lastLocaleNameQsettings->endGroup();

        //检查数据库版本
        m_dbVersionQsettings = new QSettings(APP_DATABASE_VERSION_CONFIG, QSettings::IniFormat);
        m_dbVersionQsettings->beginGroup(APP_DATABASE_VERSION_VALUE);
        QString dbVersion = m_dbVersionQsettings->value(APP_DATABASE_VERSION_VALUE).toString();
        if (dbVersion.isEmpty()) {
            m_dbVersionNeedUpdate = true;
        } else if (dbVersion != APP_DATABASE_VERSION) {
            if (dbVersion.toDouble() < APP_DATABASE_VERSION.toDouble()) {
                m_dbVersionNeedUpdate = true;
            } else {
                qDebug() << "app db version old version:" << dbVersion.toDouble() << "new version:" << APP_DATABASE_VERSION.toDouble();
            }
        }
        m_dbVersionQsettings->endGroup();

        if (m_dbVersionNeedUpdate) {
            qDebug() << "app db version need update! old version:" << dbVersion.toDouble() << "new version:" << APP_DATABASE_VERSION.toDouble();
        }

        //版本号改变更新数据库字段
        if (m_dbVersionNeedUpdate) {
            for (auto iter = m_namesOfAppinfoTable.constBegin(); iter!= m_namesOfAppinfoTable.constEnd(); iter++) {
                this->addItem2BackIfNotExist(iter.key(), iter.value());
            }
        }

        //刷新应用数据
        m_appPaths = AppDataUtils::getDesktopPaths();
        needSave = needSave | handleDataBaseRefresh(m_appPaths, m_dbVersionNeedUpdate);

        auto db = saveToFileDb(needSave);
        if (db.isValid() && db.isOpen()) {
            if (m_dbVersionNeedUpdate) {
                m_dbVersionQsettings->beginGroup(APP_DATABASE_VERSION_VALUE);
                m_dbVersionQsettings->setValue(APP_DATABASE_VERSION_VALUE, APP_DATABASE_VERSION);
                m_dbVersionQsettings->endGroup();
                m_dbVersionNeedUpdate = false;
            }
            m_database.close();
            m_database = db;
        }

        //初始化FileSystemWatcher
        initFileSystemWatcher(m_appPaths);

        auto kpmConnector = new UkuiQuick::DbusConnector(QStringLiteral("com.kylin.ProcessManager"),
                                              QStringLiteral("/com/kylin/ProcessManager/AppLauncher"),
                                              QStringLiteral("com.kylin.ProcessManager.AppLauncher"),
                                              QDBusConnection::sessionBus());

        connect(kpmConnector, &UkuiQuick::DbusConnector::ready, this, [ = ](QDBusInterface *dbusInterface) {
            if (dbusInterface->isValid()) {
                m_processManagerInterface = dbusInterface;
               qDebug() << "connect appLaunched signal" << connect(m_processManagerInterface, SIGNAL(AppLaunched(QString)), this, SLOT(handleAppLaunched(QString)));
            }
            if (sender()) {
                sender()->deleteLater();
            }
        });
        kpmConnector->start();

        new AppDBManagerAdaptor(this);
    } else {
        qDebug() << "App-db-manager does nothing.";
    }
}

AppDBManager::~AppDBManager()
{
    if (m_watcher) {
        delete m_watcher;
    }
    m_watcher = nullptr;

    closeDataBase();
}

void AppDBManager::buildAppInfoDB()
{
    qDebug() << "I'm going to build app info database.";
    QSqlQuery sql(m_database);
    QString cmd = QString("CREATE TABLE IF NOT EXISTS appInfo(%1, %2, %3, %4, %5, %6, %7, %8,%9, %10, %11, %12, %13, %14, %15, %16, %17, %18, %19, %20, %21, %22, %23, %24, %25)")
//                         .arg("ID INT")//自增id
                         .arg("DESKTOP_FILE_PATH TEXT PRIMARY KEY NOT NULL")//desktop文件路径
                         .arg("MODIFYED_TIME TEXT")//YYYYMMDDHHmmSS 修改日期
                         .arg("INSERT_TIME TEXT")//YYYYMMDDHHmmSS 插入日期
                         .arg("LOCAL_NAME TEXT")//本地名称，跟随系统语言
                         .arg("NAME_EN TEXT")//应用英文名称
                         .arg("NAME_ZH TEXT")//应用中文名称
                         .arg("PINYIN_NAME TEXT")//中文拼音
                         .arg("FIRST_LETTER_OF_PINYIN TEXT")//中文拼音首字母
                         .arg("FIRST_LETTER_ALL TEXT")//拼音和英文全拼
                         .arg("ICON TEXT")//图标名称（或路径）
                         .arg("TYPE TEXT")//应用类型
                         .arg("CATEGORY TEXT")//应用分类
                         .arg("EXEC TEXT")//应用命令
                         .arg("COMMENT TEXT")//应用注释
                         .arg("MD5 TEXT")//desktop文件内容md5值
                         .arg("LAUNCH_TIMES INT")//应用打开次数, 等比例缩减
                         .arg("FAVORITES INT")//收藏顺序0:为收藏，>0的数字表示收藏顺序
                         .arg("LAUNCHED INT")//应用安装后是否打开过0:未打开过;1:打开过
                         .arg("TOP INT")//置顶顺序 0:未置顶；>0的数字表示置顶顺序
                         .arg("LOCK INT")//应用是否锁定（管控），0未锁定，1锁定
                         .arg("DONT_DISPLAY INT")//应用隐藏(NoDisplay, NotShowIn)
                         .arg("AUTO_START INT")//自启应用
                         .arg("START_UP_WMCLASS TEXT")//classname
                         .arg("KEYWORDS TEXT")//应用的关键词
                         .arg("LOCAL_KEYWORDS TEXT");//应用本地关键词，跟随系统语言
    if (!sql.exec(cmd)) {
        qWarning() << m_database.lastError() << cmd;
        return;
    }
}

void AppDBManager::initFileSystemWatcher(const QStringList& appDirs)
{
    m_watcher = new FileSystemWatcher;

    m_helper = new ApplicationDirWatcherHelper(m_watcher, this);
    for (const QString& appDir : appDirs) {
       m_helper->addPath(appDir);
    }
    connect(m_helper, &ApplicationDirWatcherHelper::desktopFilePathAdded, [ & ] (const QString& appPath) {
        this->refreshAllData2DB({appPath}, false);
    });

    connect(m_watcher, &FileSystemWatcher::created, this, [ = ] (const QString &desktopfp, bool isDir) {
        //event is IN_CREATE
        if (!isDir and desktopfp.endsWith(".desktop")) {
            QFileInfo info(desktopfp);
            if (!info.isSymLink() || !info.fileName().startsWith(SDK_SYMBOLIC_PREFIX)) {
                this->insertDBItem(desktopfp);
                return;
            }
            this->updateSdkSymLink(desktopfp);
        }
    });

    connect(m_watcher, &FileSystemWatcher::moveTo, this, [ = ] (const QString &desktopfp, bool isDir) {
        //event is IN_MOVED_TO
        if (!isDir and desktopfp.endsWith(".desktop")) {
            QStringList appPaths(desktopfp.left(desktopfp.lastIndexOf("/")));
            qDebug() << "Refresh data of " << appPaths << "because of moveto event of" << desktopfp;
            this->refreshAllData2DB(appPaths);
        }
    });

    connect(m_watcher, &FileSystemWatcher::modified, this, [ = ] (const QString &desktopfp) {
        //event is IN_MODIFY
        if (desktopfp.endsWith(".desktop")) {
            this->updateDBItem(desktopfp);
        }
    });

    connect(m_watcher, &FileSystemWatcher::moved, this, [ = ] (const QString &desktopfp, bool isDir) {
        //event is IN_MOVED_FROM
        if (!isDir) {
            if (desktopfp.endsWith(".desktop")) {
                this->deleteDBItem(desktopfp);
            }
        } else {
            //event is IN_MOVE_SELF
            qWarning() << "Dir:" << desktopfp << "has been moved to other place! Stop the watching of the desktop files in it!";
        }
    });

    connect(m_watcher, &FileSystemWatcher::deleted, this, [ = ] (const QString &desktopfp, bool isDir) {
        //event is IN_DELETE
        if (!isDir) {
            if (desktopfp.endsWith(".desktop")) {
                this->deleteDBItem(desktopfp);
            }
        } else {
            //event is IN_DELETE_SELF
            qWarning() << "Dir:" << desktopfp << "has been deleted! Stop the watching of the desktop files in it!";
        }
    });
}

void AppDBManager::loadDesktopFilePaths(QString path, QFileInfoList &infolist)
{
    QDir dir(path);
    dir.setSorting(QDir::DirsFirst);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    infolist.append(dir.entryInfoList());
}

bool AppDBManager::addItem2BackIfNotExist(QString itemName, QString itemDataType, QVariant defult)
{
    bool res(true);
    QSqlQuery sql(m_database);
    sql.setForwardOnly(true);
    QString cmd = QString("SELECT * FROM sqlite_master WHERE name = 'appInfo' AND sql like '%%1%' ").arg(itemName);
    if (!sql.exec(cmd)) {
        qWarning() << m_database.lastError() << cmd;
        res = false;
        return res;
    }
    if (sql.next()) {
        qDebug() << itemName << "is exist!";
        return res;
    }

    cmd = QString("ALTER TABLE appInfo ADD '%0' %1 ").arg(itemName)
                                                     .arg(itemDataType);
    if (defult != QVariant()) {
        //TODO 根据数据类型将初始值转化为对应格式数据
        if (itemDataType == "INT(4)") {
            cmd += QString("DEFAULT %1").arg(defult.toInt());
        } else if (itemDataType == "TEXT") {
            cmd += QString("DEFAULT %1").arg(QString(defult.toByteArray()));
        }
    }
    if (!sql.exec(cmd)) {
        qWarning() << m_database.lastError() << cmd;
        res = false;
    } else {
        qDebug() << "Add item" << itemName << "successful.";
    }
    if (!res) {
        qDebug() << "Fail to addItem2Back :" << itemName;
    }
    return res;
}

void AppDBManager::refreshAllData2DB(const QStringList &appPaths, bool dbVersionNeedUpdate)
{
    PendingAppInfo items("refreshDB", PendingAppInfo::RefreshDataBase);
    items.setPathsNeedRefreshData(appPaths);
    items.setDBUpdate(dbVersionNeedUpdate);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(items);
}

bool AppDBManager::handleLocaleDataUpdate(const QString &desktopFilePath)
{
    bool res(true);
    XdgDesktopFile desktopFile;
    desktopFile.load(desktopFilePath);
    QString localName = desktopFile.localizedValue("Name", "NULL").toString();
    QString firstLetter2All = localName;

    if (localName.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
        firstLetter2All = FileUtils::findMultiToneWords(localName).at(0);
    }


    QSqlQuery query(m_database);
    query.prepare("UPDATE appInfo SET LOCAL_NAME=:localName, FIRST_LETTER_ALL=:firstOfLetter2All, LOCAL_KEYWORDS=:localKeywords WHERE DESKTOP_FILE_PATH=:desktopFilePath");
    query.bindValue(":localName", localName);
    query.bindValue(":firstOfLetter2All", firstLetter2All);
    query.bindValue(":desktopFilePath", desktopFilePath);
    query.bindValue(":localKeywords", desktopFile.localizedValue("Keywords", "NULL").toString());

    if (!this->startTransaction()) {
        return false;
    }

    if (query.exec()) {
        if (this->startCommit()) {
            ApplicationInfoMap appInfo;
            appInfo[desktopFilePath].insert(ApplicationProperty::LocalName, QVariant(localName));
            appInfo[desktopFilePath].insert(ApplicationProperty::FirstLetterAll, QVariant(firstLetter2All));
            Q_EMIT this->appDBItemUpdate(appInfo);
            qDebug() << "Update the locale data of " << desktopFilePath;
        } else {
            res = false;
        }
    } else {
        qDebug() << "Fail to update the locale data of " << desktopFilePath;
        qWarning() << query.lastError() << query.lastQuery();
        m_database.rollback();
        res = false;
    }
    return res;
}

void AppDBManager::run()
{
    exec();
}

void AppDBManager::refreshDataBase()
{
//    if (m_database.transaction()) {
//        this->updateAllData2DB();
//        if (!m_database.commit()) {
//            qWarning() << "Failed to commit !";
//            m_database.rollback();
//        } else if (!m_dbChanged) {
//            qDebug() << "app DataBase has no changes!";
//        } else {
//            Q_EMIT this->finishHandleAppDB();
//        }
//    } else {
//        qWarning() << "Failed to start transaction mode!!!";
//    }
}

bool AppDBManager::openDataBase(QSqlDatabase &database, bool memory)
{
    if (memory) {
        database = QSqlDatabase::addDatabase("QSQLITE", "tmp");
        database.setDatabaseName(":memory:");
    } else {
        QDir dir;
        if (!dir.exists(APP_DATABASE_PATH)) {
            dir.mkpath(APP_DATABASE_PATH);
        }
        if (QSqlDatabase::contains(CONNECTION_NAME)) {
            database = QSqlDatabase::database(CONNECTION_NAME);
        } else {
            database = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
            database.setDatabaseName(APP_DATABASE_PATH + APP_DATABASE_NAME);
        }
    }

    QString errorMessage;
    const auto mode = memory ? SqliteDatabaseMode::InMemory : SqliteDatabaseMode::FileBacked;
    if (!configureSqliteDatabase(database, mode, &errorMessage)) {
        qWarning() << "Fail to open AppDataBase, because" << errorMessage;
        return false;
    }

    return true;
}

void AppDBManager::closeDataBase()
{
    m_database.close();
//    delete m_database;
    QSqlDatabase::removeDatabase(CONNECTION_NAME);
}

QString AppDBManager::getAppDesktopMd5(const QString &desktopfd)
{
    QString res;
    QFile file(desktopfd);
    file.open(QIODevice::ReadOnly);
    res = QString::fromStdString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex().toStdString());
    file.close();
    return res;
}

bool AppDBManager::startTransaction()
{
    if (m_database.transaction()) {
        return true;
    } else {
        qWarning() << "Failed to start transaction mode!!!" << m_database.lastError();
        return false;
    }
}

bool AppDBManager::startCommit()
{
    if (!m_database.commit()) {
        qWarning() << "Failed to commit !" << m_database.lastError();
        m_database.rollback();
        return false;
    } else {
        return true;
    }
}

bool AppDBManager::handleDBItemInsert(const QString &desktopFilePath)
{
    bool res(true);
    QSqlQuery sql(m_database);
    XdgDesktopFile desktopfile;
    desktopfile.load(desktopFilePath);
    QString hanzi, pinyin, firstLetterOfPinyin;
    QString localName = desktopfile.localizedValue("Name", "NULL").toString();
    QString firstLetter2All = localName;
    bool isHanzi = true;

    if (localName.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
        firstLetter2All = FileUtils::findMultiToneWords(localName).at(0);
    }

    if (desktopfile.contains("Name[zh_CN]")) {
        hanzi = desktopfile.value("Name[zh_CN]").toString();
    } else if ((QLocale::system().name() == "zh_CN") && localName.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
        hanzi = localName;
    } else {
        hanzi = desktopfile.value("Name").toString();
        if (!hanzi.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
            isHanzi = false;
        }
    }

    if (isHanzi) {
        QStringList pinyinList = FileUtils::findMultiToneWords(hanzi);
        for (int i = 0; i<pinyinList.size(); ++i) {
            if (i % 2) {
                firstLetterOfPinyin += pinyinList.at(i);
            } else {
                pinyin += pinyinList.at(i);
            }
        }
    }

    int dontDisplay = 0;
    QString onlyShowIn = desktopfile.value("OnlyShowIn").toString();
    if (desktopfile.value("NoDisplay").toString().contains("true")
    || desktopfile.value("NotShowIn").toString().contains("UKUI")
    || (!onlyShowIn.isEmpty() && !onlyShowIn.contains("UKUI"))) {
        dontDisplay = 1;
    }

    int isAutoStartApp = 0;
    if (desktopFilePath.startsWith(AUTOSTART_APP_DESKTOP_PATH + "/")) {
        isAutoStartApp = 1;
    }

    sql.prepare(QString("INSERT INTO appInfo "
                        "(DESKTOP_FILE_PATH, MODIFYED_TIME, INSERT_TIME, "
                        "LOCAL_NAME, NAME_EN, NAME_ZH, PINYIN_NAME, "
                        "FIRST_LETTER_OF_PINYIN, FIRST_LETTER_ALL, "
                        "ICON, TYPE, CATEGORY, EXEC, COMMENT, MD5, "
                        "LAUNCH_TIMES, FAVORITES, LAUNCHED, TOP, LOCK, DONT_DISPLAY, AUTO_START, START_UP_WMCLASS, KEYWORDS, LOCAL_KEYWORDS) "
                        "VALUES(:desktopFilePath, '%0', '%1', :localName, :enName, :zhName, :pinyinName, :firstLetterOfPinyin, :firstLetter2All, "
                        ":icon, :type, :categories, :exec, :comment,'%2',%3,%4,%5,%6,%7,%8,%9, :wmClass, :keywords, :localKeywords)")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(getAppDesktopMd5(desktopFilePath))
                .arg(0)
                .arg(0)
                .arg(0)
                .arg(0)
                .arg(0)
                .arg(dontDisplay)
                .arg(isAutoStartApp));
    sql.bindValue(":desktopFilePath", desktopFilePath);
    sql.bindValue(":localName", localName);
    sql.bindValue(":enName", desktopfile.value("Name").toString());
    sql.bindValue(":zhName", hanzi);
    sql.bindValue(":pinyinName", pinyin);
    sql.bindValue(":firstLetterOfPinyin", firstLetterOfPinyin);
    sql.bindValue(":firstLetter2All", firstLetter2All);
    sql.bindValue(":icon", desktopfile.value("Icon").toString());
    sql.bindValue(":type", desktopfile.value("Type").toString());
    sql.bindValue(":categories", desktopfile.value("Categories").toString());
    sql.bindValue(":exec", desktopfile.value("Exec").toString());
    sql.bindValue(":comment", desktopfile.value("Comment").toString());
    sql.bindValue(":wmClass", desktopfile.value("StartupWMClass").toString());
    sql.bindValue(":keywords", desktopfile.value("Keywords").toString());
    sql.bindValue(":localKeywords", desktopfile.localizedValue("Keywords", "NULL").toString());

    if (!this->startTransaction()) {
        return false;
    }

    if (sql.exec()) {
        if (this->startCommit()) {
            Q_EMIT this->appDBItemAdd(desktopFilePath);
            qDebug() << "app database add " << desktopFilePath << "success!";
        } else {
            res = false;
        }
    } else {
        qDebug() << "app database add " << desktopFilePath << "failed!";
        qWarning() << m_database.lastError() << sql.lastQuery();
        m_database.rollback();
        res = false;
    }
    return res;
}

bool AppDBManager::handleDBItemDelete(const QString &desktopFilePath)
{
    if (m_symLinks.contains(desktopFilePath)) {
        qDebug() << "Delete symlink " << desktopFilePath << "--" << m_symLinks[desktopFilePath];
        auto target = m_symLinks[desktopFilePath];
        m_symLinks.remove(desktopFilePath);
        return this->handleDBItemDelete(target);
    }

    if (!this->startTransaction()) {
        return false;
    }

    bool res(true);
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    QString cmd = "SELECT FAVORITES, TOP FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFilePath";
    query.prepare(cmd);
    query.bindValue(":desktopFilePath", desktopFilePath);

    //查询要删除信息的应用是否被收藏或顶置过
    if (!query.exec()) {
        res = false;
        qWarning() << m_database.lastError() << query.lastQuery();
    } else if (query.next()) {
        int favorites = query.value("FAVORITES").toInt();
        int top = query.value("TOP").toInt();
        if (favorites) {
            query.prepare(QString("UPDATE appInfo SET FAVORITES = FAVORITES -1 WHERE FAVORITES > %1").arg(favorites));
            if (query.exec()) {
                cmd = QString("SELECT DESKTOP_FILE_PATH,FAVORITES FROM APPINFO WHERE FAVORITES >= %1").arg(favorites);
                if (query.exec(cmd)) {
                    ApplicationInfoMap infos;
                    while (query.next()) {
                        if (query.value("DESKTOP_FILE_PATH").toString() == desktopFilePath) {
                            continue;
                        }
                        infos[query.value("DESKTOP_FILE_PATH").toString()].insert(ApplicationProperty::Favorites, query.value("FAVORITES"));
                    }
                    if (!infos.isEmpty()) {
                        Q_EMIT this->appDBItemUpdate(infos);
                    }
                } else {
                    qWarning() << query.lastError() << query.lastQuery();
                }
            } else {
                res = false;
                qWarning() << "I'm going to delete item in db, fail to update the FAVORITES because:" <<  query.lastError() << cmd;
            }
        }
        if (top) {
            query.prepare(QString("UPDATE appInfo SET TOP = TOP -1 WHERE TOP > %1").arg(top));
            if (query.exec()) {
                cmd = QString("SELECT DESKTOP_FILE_PATH,TOP FROM APPINFO WHERE TOP >= %1").arg(top);
                if (query.exec(cmd)) {
                    ApplicationInfoMap infos;
                    while (query.next()) {
                        if (query.value("DESKTOP_FILE_PATH").toString() == desktopFilePath) {
                            continue;
                        }
                        infos[query.value("DESKTOP_FILE_PATH").toString()].insert(ApplicationProperty::Top, query.value("TOP"));
                    }
                    if (!infos.isEmpty()) {
                        Q_EMIT this->appDBItemUpdate(infos);
                    }
                } else {
                    qWarning() << query.lastError() << query.lastQuery();
                }
            } else {
                res = false;
                qWarning() << "I'm going to delete item in db, fail to update the TOP because:" << query.lastError() << cmd;
            }
        }
    } else {
        qWarning() << "Fail to exec next, because" << query.lastError() << "while executing " << query.lastQuery();
    }

    if (!res) {
        m_database.rollback();
        return res;
    }

    //执行删除操作
    cmd = "DELETE FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFilePath";
    query.prepare(cmd);
    query.bindValue(":desktopFilePath", desktopFilePath);
    if (query.exec()) {
        if (this->startCommit()) {
            Q_EMIT this->appDBItemDelete(desktopFilePath);
            qDebug() << "app database delete " << desktopFilePath << "success!";
        } else {
            res = false;
        }
    } else {
        qDebug() << "app database delete " << desktopFilePath << "failed!";
        qWarning() << query.lastError() << cmd;
        m_database.rollback();
        res = false;
    }
    return res;
}

bool AppDBManager::handleDBItemUpdate(const QString &desktopFilePath)
{
    bool res(true);
    XdgDesktopFile desktopfile;
    desktopfile.load(desktopFilePath);

    int dontDisplay = 0;
    QString onlyShowIn = desktopfile.value("OnlyShowIn").toString();
    if (desktopfile.value("NoDisplay").toString().contains("true")
        || desktopfile.value("NotShowIn").toString().contains("UKUI")
        || (!onlyShowIn.isEmpty() && !onlyShowIn.contains("UKUI"))) {
        dontDisplay = 1;
    }

    int isAutoStartApp = 0;
    if (desktopFilePath.startsWith(AUTOSTART_APP_DESKTOP_PATH + "/")) {
        isAutoStartApp = 1;
    }

    QString hanzi, pinyin, firstLetterOfPinyin;
    QString localName = desktopfile.localizedValue("Name", "NULL").toString();
    QString firstLetter2All = localName;
    bool isHanzi = true;

    if (localName.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
        firstLetter2All = FileUtils::findMultiToneWords(localName).at(0);
    }

    if (desktopfile.contains("Name[zh_CN]")) {
        hanzi = desktopfile.value("Name[zh_CN]").toString();
    } else if ((QLocale::system().name() == "zh_CN") && localName.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
        hanzi = localName;
    } else {
        hanzi = desktopfile.value("Name").toString();
        if (!hanzi.contains(REG_EXP("[\\x4e00-\\x9fa5]+"))) {
            isHanzi = false;
        }
    }

    if (isHanzi) {
        QStringList pinyinList = FileUtils::findMultiToneWords(hanzi);
        firstLetter2All = pinyinList.at(0);
        for (int i = 0; i<pinyinList.size(); ++i) {
            if (i % 2) {
                firstLetterOfPinyin += pinyinList.at(i);
            } else {
                pinyin += pinyinList.at(i);
            }
        }
    }

    QSqlQuery sql(m_database);
    sql.prepare(QString("UPDATE appInfo SET "
                        "MODIFYED_TIME='%0',"
                        "LOCAL_NAME=:localName,"
                        "NAME_EN=:enName,"
                        "NAME_ZH=:zhName,"
                        "PINYIN_NAME=:pinyinName,"
                        "FIRST_LETTER_OF_PINYIN=:firstLetterOfPinyin,"
                        "FIRST_LETTER_ALL=:firstLetter2All,"
                        "ICON=:icon,"
                        "TYPE=:type,"
                        "CATEGORY=:categories,"
                        "EXEC=:exec,"
                        "COMMENT=:comment,"
                        "START_UP_WMCLASS=:wmClass,"
                        "KEYWORDS=:keywords,"
                        "LOCAL_KEYWORDS=:localKeywords,"
                        "MD5='%1',"
                        "DONT_DISPLAY=%2,"
                        "AUTO_START=%3 "
                        "WHERE DESKTOP_FILE_PATH=:desktopFilePath")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(getAppDesktopMd5(desktopFilePath))
                .arg(dontDisplay)
                .arg(isAutoStartApp));
    sql.bindValue(":desktopFilePath", desktopFilePath);
    sql.bindValue(":localName", localName);
    sql.bindValue(":enName", desktopfile.value("Name").toString());
    sql.bindValue(":zhName", hanzi);
    sql.bindValue(":pinyinName", pinyin);
    sql.bindValue(":firstLetterOfPinyin", firstLetterOfPinyin);
    sql.bindValue(":firstLetter2All", firstLetter2All);
    sql.bindValue(":icon", desktopfile.value("Icon").toString());
    sql.bindValue(":type", desktopfile.value("Type").toString());
    sql.bindValue(":categories", desktopfile.value("Categories").toString());
    sql.bindValue(":exec", desktopfile.value("Exec").toString());
    sql.bindValue(":comment", desktopfile.value("Comment").toString());
    sql.bindValue(":wmClass", desktopfile.value("StartupWMClass").toString());
    sql.bindValue(":keywords", desktopfile.value("Keywords").toString());
    sql.bindValue(":localKeywords", desktopfile.localizedValue("Keywords", "NULL").toString());

    if (!this->startTransaction()) {
        return false;
    }

    if (sql.exec()) {
        if (this->startCommit()) {
            Q_EMIT this->appDBItemUpdateAll(desktopFilePath);
            qDebug() << "app database update all data of" << desktopFilePath << "success!";
        } else {
            res = false;
        }
    } else {
        qDebug() << "app database update " << desktopFilePath << "failed!";
        qWarning() << m_database.lastError() << sql.lastQuery();
        m_database.rollback();
        res = false;
    }
    return res;
}

bool AppDBManager::handleLaunchTimesUpdate(const QString &desktopFilePath, int num)
{
    qDebug() << "launch times will add:" << num;
    bool res(true);
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    query.prepare("SELECT LAUNCH_TIMES FROM APPINFO WHERE DESKTOP_FILE_PATH=:desktopFilePath");
    query.bindValue(":desktopFilePath", desktopFilePath);
    if (query.exec()) {
        if (query.next()) {
            int launchTimes = query.value(0).toInt() + num;
            if (!this->startTransaction()) {
                res = false;
                return res;
            }
            query.prepare(QString("UPDATE appInfo SET MODIFYED_TIME='%0', LAUNCH_TIMES=%1, LAUNCHED=%2 WHERE DESKTOP_FILE_PATH=:desktopFilePath")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                        .arg(launchTimes)
                        .arg(1));
            query.bindValue(":desktopFilePath", desktopFilePath);
            if (!query.exec()) {
                qWarning() << "Set app launch times failed!" << m_database.lastError();
                res = false;
                m_database.rollback();
            } else {
                if (this->startCommit()) {
                    ApplicationInfoMap appInfo;
                    appInfo[desktopFilePath].insert(ApplicationProperty::LaunchTimes, QVariant(launchTimes));
                    appInfo[desktopFilePath].insert(ApplicationProperty::Launched, QVariant(1));
                    Q_EMIT this->appDBItemUpdate(appInfo);
                    qDebug() << "app database update " << desktopFilePath << "launch times: " << launchTimes << "success!";
                } else {
                    res = false;
                }
            }
        } else {
            qWarning() << "Failed to exec next!" << query.lastQuery() << query.lastError();
            res = false;
        }
    } else {
        qWarning() << "Failed to exec:" << query.lastQuery();
        res = false;
    }

    return res;
}

bool AppDBManager::handleFavoritesStateUpdate(const QString &desktopFilePath, const uint num)
{
    bool res(true);
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    QString cmd;
    ApplicationInfoMap infos;

    //更新favorites状态
    if (!this->startTransaction()) {
        return false;
    }
    cmd = QString("UPDATE APPINFO SET MODIFYED_TIME='%0', FAVORITES=%1 WHERE DESKTOP_FILE_PATH=:desktopFilePath")
                  .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                  .arg(num);
    query.prepare(cmd);
    query.bindValue(":desktopFilePath", desktopFilePath);
    if (!query.exec()) {
        qWarning() << "Set app favorites state failed!" << m_database.lastError();
        res = false;
        m_database.rollback();
    } else {
        if (this->startCommit()) {
            infos[desktopFilePath].insert(ApplicationProperty::Favorites, QVariant(num));
            Q_EMIT this->appDBItemUpdate(infos);
            qDebug() << "app database update " << desktopFilePath << "favorites state: " << num << "success!";
        } else  {
            res = false;
        }
    }
    return res;
}

bool AppDBManager::handleTopStateUpdate(const QString &desktopFilePath, const uint num)
{
    ApplicationInfoMap infos;
    const bool res = applyTopStateUpdate(m_database, desktopFilePath, num, &infos);
    if (res) {
        Q_EMIT this->appDBItemUpdate(infos);
        qDebug() << "app database update " << desktopFilePath << "top state: " << num << "success!";
    }
    return res;
}

QSqlDatabase AppDBManager::saveToFileDb(bool needSave)
{
    QSqlDatabase fileDb;
    openDataBase(fileDb, false);
    if (needSave) {
        sqlite3* to = nullptr;
        QVariant v = fileDb.driver()->handle();
        if (v.isValid() && (qstrcmp(v.typeName(), "sqlite3*") == 0)) {
            to = *static_cast<sqlite3 **>(v.data());
            if (!to) {
                qWarning() << "saveToFileDb: fileDb sqlite3 handle is null!";
                return {};
            }
        }
        v = m_database.driver()->handle();
        sqlite3* from = nullptr;
        if (v.isValid() && (qstrcmp(v.typeName(), "sqlite3*") == 0)) {
            from = *static_cast<sqlite3 **>(v.data());
            if (!from) {
                qWarning() << "saveToFileDb: m_database sqlite3 handle is null!";
                return {};
            }
        }

        sqlite3_backup* backup = sqlite3_backup_init(to, "main", from, "main");
        if (backup) {
            (void)sqlite3_backup_step(backup, -1);
            (void)sqlite3_backup_finish(backup);
        }
        qDebug() << "saveToFileDb:" << sqlite3_errmsg(to);
    }

    return fileDb;
}

bool AppDBManager::loadFromFileDb()
{
    sqlite3 *fileDatabase = nullptr;
    const QByteArray databasePath = (APP_DATABASE_PATH + APP_DATABASE_NAME).toUtf8();
    const int rc = sqlite3_open_v2(databasePath.constData(),
                                   &fileDatabase,
                                   SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                   nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "loadFromFileDb: failed to open file database:" << sqlite3_errstr(rc);
        if (fileDatabase) {
            sqlite3_close(fileDatabase);
        }
        return false;
    }

    sqlite3_busy_timeout(fileDatabase, defaultSqliteBusyTimeoutMs());

    QVariant v = m_database.driver()->handle();
    sqlite3* to = nullptr;
    if (v.isValid() && (qstrcmp(v.typeName(), "sqlite3*") == 0)) {
        to = *static_cast<sqlite3 **>(v.data());
    }
    if (!to) {
        qWarning() << "loadFromFileDb: sqlite3 handle is null!";
        sqlite3_close(fileDatabase);
        return false;
    }

    bool success = true;
    sqlite3_backup* backup = sqlite3_backup_init(to, "main", fileDatabase, "main");
    if (!backup) {
        qWarning() << "loadFromFileDb: backup init failed:" << sqlite3_errmsg(fileDatabase);
        success = false;
    } else {
        const int stepRc = sqlite3_backup_step(backup, -1);
        const int finishRc = sqlite3_backup_finish(backup);
        if (stepRc != SQLITE_DONE || finishRc != SQLITE_OK) {
            qWarning() << "loadFromFileDb: backup failed:" << sqlite3_errstr(stepRc)
                       << sqlite3_errstr(finishRc);
            success = false;
        }
    }
    qDebug() << "loadFromFileDb:" << sqlite3_errmsg(fileDatabase);
    (void)sqlite3_close(fileDatabase);
    return success;
}

bool AppDBManager::handleLockStateUpdate(const QString &desktopFilePath, int num)
{
    if (!this->startTransaction()) {
        return false;
    }
    bool res(true);
    QSqlQuery sql(m_database);
    sql.prepare(QString("UPDATE appInfo SET MODIFYED_TIME='%0', LOCK=%1 WHERE DESKTOP_FILE_PATH=:desktopFilePath")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(num));
    sql.bindValue(":desktopFilePath", desktopFilePath);
    if (!sql.exec()) {
        qWarning() << "Set app lock state failed!" << m_database.lastError();
        res = false;
        m_database.rollback();
    } else {
        if (this->startCommit()) {
            ApplicationInfoMap appInfo;
            appInfo[desktopFilePath].insert(ApplicationProperty::Lock, QVariant(num));
            Q_EMIT this->appDBItemUpdate(appInfo);
            qDebug() << "app database update " << desktopFilePath << "lock state: " << num << "success!";
        } else {
            res = false;
        }
    }
    return res;
}

bool AppDBManager::handleDataBaseRefresh(const QStringList &appPaths, bool dbVersionNeedUpdate)
{
    QMap<QString, QString> dataMap;
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    QString condition;
    for (int i = 0; i < appPaths.size(); i++) {
        condition.append("DESKTOP_FILE_PATH LIKE ? OR ");
    }
    condition = condition.left(condition.lastIndexOf(" OR "));
    query.prepare(QString("SELECT DESKTOP_FILE_PATH,MD5 FROM APPINFO WHERE %0").arg(condition));
    for (int t = 0; t < appPaths.size(); t++) {
        query.bindValue(t, appPaths.at(t) + "/%");
    }

    if (!query.exec()) {
        qWarning() << m_database.lastError() << query.lastError();
    } else {
        query.exec();
        while (query.next()) {
            dataMap.insert(query.value("DESKTOP_FILE_PATH").toString(), query.value("MD5").toString());
        }
    }

    //遍历desktop文件
    QFileInfoList infos;
    for (const QString &path : appPaths) {
        this->loadDesktopFilePaths(path, infos);
    }

    bool res(false);
    if(infos.isEmpty()) {
        return res;
    }
    XdgDesktopFile desktopfile;
    for (int i = 0; i < infos.length(); i++) {
        QFileInfo fileInfo = infos.at(i);
        QString path = fileInfo.filePath();
        //对目录递归
        if (fileInfo.isDir()) {
            loadDesktopFilePaths(path, infos);
            continue;
        }
        //排除非法路径（非desktop文件）
        if (!path.endsWith(".desktop")) {
            continue;
        }

        //记录符号链接
        if (fileInfo.isSymLink() && fileInfo.fileName().startsWith(SDK_SYMBOLIC_PREFIX)) {
            m_symLinks.insert(path, fileInfo.symLinkTarget());
            continue;
            //TODO: 做一些异常处理
        }

        desktopfile.load(path);
        //排除localized名字为空
        if (desktopfile.localizedValue("Name").toString().isEmpty()) {
            continue;
        }

        if (!dataMap.isEmpty()) {
            //数据库有记录
            if (dataMap.contains(path)) {
                if (!QString::compare(dataMap.value(path), getAppDesktopMd5(path)) && !dbVersionNeedUpdate) {
                    //判断系统语言是否改变
                    if (m_localeChanged) {
                        res = true;
                        this->handleLocaleDataUpdate(path);
                    }
                    dataMap.remove(path);
                    continue;
                } else {
                    //数据库有记录但md5值改变或数据库版本需要更新则update
                    res = true;
                    this->handleDBItemUpdate(path);
                    dataMap.remove(path);
                    continue;
                }
            } else {
                //数据库中没有记录则insert
                res = true;
                this->handleDBItemInsert(path);
                dataMap.remove(path);
                continue;
            }
        }
        //数据库为空则全部insert
        res = true;
        this->handleDBItemInsert(path);
        dataMap.remove(path);
    }

    //遍历完成后重置标志位
    m_localeChanged = false;

    //数据库冗余项直接delete
    if (!dataMap.isEmpty()) {
        for (auto i = dataMap.constBegin(); i != dataMap.constEnd(); i++) {
            res = true;
            this->handleDBItemDelete(i.key());
        }
    }
    return res;
}

bool AppDBManager::handleSdkSymlink(const QString& symlinkPath)
{
    QFileInfo info(symlinkPath);
    auto targetPath = info.symLinkTarget();
    if (!QFile::exists(targetPath)) {
        return false;
    }

    m_symLinks.insert(symlinkPath, targetPath);
    qDebug() << "Symlink add" << symlinkPath << "--" << targetPath;

    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    query.prepare("SELECT MD5 FROM APPINFO WHERE DESKTOP_FILE_PATH=:targetPath");
    query.bindValue(":targetPath", targetPath);
    if (!query.exec()) {
        qWarning() << query.lastQuery() << m_database.lastError();
        return false;
    }

    if (!query.next()) {
        qDebug() << "There's no record, insert target path: " << targetPath;
        this->handleDBItemInsert(targetPath);
        return true;
    }

    if (QString::compare(query.value("MD5").toString(), getAppDesktopMd5(targetPath))) {
        qDebug() << "Ready to update target path" << targetPath;
        this->handleDBItemUpdate(targetPath);
    }
    return true;
}

bool AppDBManager::handleValueSet(const ApplicationInfoMap &appInfoMap)
{
    ApplicationInfoMap infos2BSend;
    const bool res = applyValueSetUpdates(m_database, appInfoMap, &infos2BSend);

    if (!infos2BSend.isEmpty()) {
        Q_EMIT this->appDBItemUpdate(infos2BSend);
    }
    return res;
}

QString AppDBManager::tranPidToDesktopFp(uint pid)
{
    QString desktopFilePath;
    if (!m_processManagerInterface) {
        qWarning() << "The interface of ProcessManager is not valid";;
        return desktopFilePath;
    }
    QDBusReply<QString> reply = m_processManagerInterface->call("GetDesktopFileByPid", static_cast<int>(pid));
    if (reply.isValid()) {
        desktopFilePath = reply.value();
        qDebug() << "PID: "  << pid << "Desktop file path: " << desktopFilePath;
    } else {
        qWarning() << "Cannot find desktop file by pid:" << pid << "because of " << reply.error();
    }

    return desktopFilePath;
}

QString AppDBManager::desktopFilePathFromName(const QString &desktopFileName)
{
    QString desktopFilePath;
    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    query.prepare("SELECT DESKTOP_FILE_PATH FROM APPINFO WHERE DESKTOP_FILE_PATH LIKE :desktopFilePath");
    query.bindValue(":desktopFilePath", "%/" + desktopFileName + ".desktop");

    if (query.exec()) {
        QStringList results;
        while (query.next()) {
            desktopFilePath = query.value("DESKTOP_FILE_PATH").toString();
            results.append(desktopFilePath);
        }
        //筛选后有多个结果时进一步过滤
        if (results.size() > 1) {
            desktopFilePath.clear();
            for (const auto &appPath: m_appPaths) {
                for (const QString &path : results) {
                    if (path.startsWith(AUTOSTART_APP_DESKTOP_PATH + "/")) {
                        continue;
                    } else if (path.startsWith(appPath + "/")) {
                        desktopFilePath = path;
                        break;
                    }
                }

                if (!desktopFilePath.isEmpty()) {
                    break;
                }
            }
        }

        if (!desktopFilePath.isEmpty()) {
            qDebug() << "Desktop file path is" << desktopFilePath;
        } else {
            qWarning() << "Can not find the desktop file by Name:" << desktopFileName;
        }
    } else {
        qWarning() << "Fail to exec cmd" << query.lastQuery();
    }
    return desktopFilePath;
}

QString AppDBManager::tranWinIdToDesktopFilePath(const QDBusVariant &id)
{
    /*
    KWindowInfo info(id.variant().toULongLong(), NET::Properties(), NET::WM2AllProperties);
    QString desktopFilePath;
    if (info.valid()) {
        QString classClass = info.windowClassClass(); //the 2nd part of WM_CLASS, specified by the application writer
        QSqlQuery query(m_database);
        query.setForwardOnly(true);
        query.prepare("SELECT DESKTOP_FILE_PATH, START_UP_WMCLASS FROM APPINFO WHERE "
                      "LOWER(START_UP_WMCLASS)=:windowClassClass OR LOWER(LOCAL_NAME)=:windowClassClass");
        query.bindValue(":windowClassClass", classClass.toLower());

        if (query.exec()) {
            QMap<QString, QString> wmClassInfos;
            while (query.next()) {
                wmClassInfos[query.value("DESKTOP_FILE_PATH").toString()] = query.value("START_UP_WMCLASS").toString();
                desktopFilePath = query.value("DESKTOP_FILE_PATH").toString();
            }
            //筛选后有多个结果时进一步过滤
            if (wmClassInfos.size() > 1) {
                desktopFilePath.clear();
                for (const auto &appPath: m_appPaths) {
                    for (auto it = wmClassInfos.constBegin(); it != wmClassInfos.constEnd(); it++) {
                        if (it.key().startsWith(AUTOSTART_APP_DESKTOP_PATH)) {
                            continue;
                        }
                        if (it.key().section("/", -1) == (classClass + ".desktop") || it.value().toLower() == classClass.toLower()) {
                            if (it.key().startsWith(appPath + "/")) {
                                desktopFilePath = it.key();
                                break;
                            }
                        }
                    }

                    if (!desktopFilePath.isEmpty()) {
                        break;
                    }
                }

            }
            //没有查到
            if (desktopFilePath.isEmpty() && !classClass.isEmpty()) {
                query.prepare("SELECT DESKTOP_FILE_PATH, START_UP_WMCLASS FROM APPINFO WHERE EXEC LIKE :classClass OR DESKTOP_FILE_PATH LIKE :classClass");
                query.bindValue(":classClass", "%" + classClass + "%");

                if (query.exec()) {
                    while (query.next()) {
                        wmClassInfos[query.value("DESKTOP_FILE_PATH").toString()] = query.value("START_UP_WMCLASS").toString();
                    }

                    if (!wmClassInfos.isEmpty()) {
                        for (auto it = wmClassInfos.constBegin(); it != wmClassInfos.constEnd(); it++) {
                            if (wmClassInfos.size() > 1 && it.key().startsWith(AUTOSTART_APP_DESKTOP_PATH)) {
                                continue;
                            }
                            if (it.key().section("/", -1) == (classClass + ".desktop") || it.value().toLower() == classClass.toLower()) {
                                desktopFilePath = it.key();
                                break;
                            }
                        }
                    }
                }
            }

            if (!desktopFilePath.isEmpty()) {
                qDebug()  << "WId:" << id.variant() << "Classclass of window:" << classClass << "Desktop file path:" << desktopFilePath;
            } else {
                qWarning() << "Can not find the desktop file by windowClassClass:" << classClass;
            }
        } else {
            qWarning() << "Fail to exec cmd" << query.lastQuery();
        }

        if (desktopFilePath.isEmpty()) {
            if (!m_processManagerInterface->isValid()) {
                qWarning() << "The interface of ProcessManager is not valid";
                return desktopFilePath;
            }
            QDBusReply<QString> reply = m_processManagerInterface->call("GetDesktopFileByPid", info.pid());
            if (reply.isValid()) {
                desktopFilePath = reply.value();
                qDebug() << "PID: "  << info.pid() << "Desktop file path: " << desktopFilePath;
            } else {
                qWarning() << "Cannot find desktop file by pid:" << info.pid() << "because of " << reply.error();
            }
        }
    } else {
        qWarning() << "Cannot find desktop file by WinId:" << id.variant() << "it is invalid" << id.variant().toULongLong();
    }
    return desktopFilePath;
     */
    return {};
}

void AppDBManager::insertDBItem(const QString &desktopfd)
{
    PendingAppInfo item(desktopfd, PendingAppInfo::HandleType::Insert);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::updateDBItem(const QString &desktopfd)
{
    PendingAppInfo item(desktopfd, PendingAppInfo::HandleType::UpdateAll);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::deleteDBItem(const QString &desktopfd)
{
    PendingAppInfo item(desktopfd, PendingAppInfo::HandleType::Delete);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::updateLocaleData(const QString &desktopFilePath)
{
    PendingAppInfo item(desktopFilePath, PendingAppInfo::HandleType::UpdateLocaleData);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::updateLaunchTimes(const QString &desktopFilePath)
{
    PendingAppInfo item(desktopFilePath, PendingAppInfo::HandleType::UpdateLaunchTimes);
    item.setLaunchWillAdd(true);
    item.setLaunchTimes(1);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::updateFavoritesState(const QString &desktopFilePath, uint num)
{
    PendingAppInfo item(desktopFilePath, PendingAppInfo::HandleType::UpdateFavorites);
    item.setFavorites(num);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::updateTopState(const QString &desktopFilePath, uint num)
{
    PendingAppInfo item(desktopFilePath, PendingAppInfo::HandleType::UpdateTop);
    item.setTop(num);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::updateSdkSymLink(const QString& symLinkPath)
{
    PendingAppInfo item(symLinkPath, PendingAppInfo::HandleType::UpdateSymLink);
    PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
}

void AppDBManager::setValue(const ApplicationInfoMap &infos2BSet)
{
    for (const QString &desktopFilePath : infos2BSet.keys()) {
        PendingAppInfo item(desktopFilePath, PendingAppInfo::SetValue);
        item.setValue(infos2BSet);
        PendingAppInfoQueue::getAppInfoQueue().enqueue(item);
    }
}

void AppDBManager::handleAppLaunched(QString desktopFilePath) {
    if (!desktopFilePath.isEmpty()) {
        this->updateLaunchTimes(desktopFilePath);
    } else {
        qWarning() << "desktopFilePath is empty.";
    }
}

bool AppDBManager::repairDBDesktopPath() {
    bool repaired = false;
    const bool success = repairMalformedDesktopPaths(m_database, &repaired);
    if (!success) {
        qWarning() << "Failed to completely repair malformed desktop paths.";
    }
    return repaired;
}

ApplicationDirWatcherHelper::ApplicationDirWatcherHelper(FileSystemWatcher *watcher, QObject *parent) : QObject(parent), m_watcher(watcher)
{
}

void ApplicationDirWatcherHelper::addPath(const QString &path) {
    QDir dir(path);
    auto watcher = new FileSystemWatcher(false);
    m_watcherMap.insert(path, watcher);

    if (dir.exists() || dir.mkpath(path)) {
        m_watcher->addWatch(path);
        qDebug() << "=====desktop file's watcher add path:" << path;
        watcher->addWatch(path);
        qDebug() << "=====desktop dirs' Watcher add path:" << path;
    } else {
        while (!dir.exists()) {
            dir.setPath(dir.absolutePath().left(dir.absolutePath().lastIndexOf("/")));
            if (dir.absolutePath().isEmpty()) {
                dir.setPath("/");
                break;
            }
        }
        watcher->addWatch(dir.absolutePath());
        qDebug() << "=====desktop dirs' Watcher add path:" << dir.absolutePath();
    }

    auto handlePathAppeared = [&, watcher](const QString &path, bool isDir) {
        if (isDir) {
            QString appPath = m_watcherMap.key(watcher);
            if (appPath.startsWith(path+ "/") || appPath == path) {
                watcher->clearAll();
                QDir dir(appPath);
                //会有类似mkdir -p的一次性将子文件夹创建出来的情况，需要挨层判断
                while (!dir.exists()) {
                    dir.setPath(dir.absolutePath().left(dir.absolutePath().lastIndexOf("/")));
                }
                if (dir.absolutePath() == appPath) {
                    m_watcher->addWatch(appPath);
                    Q_EMIT this->desktopFilePathAdded(appPath);
                    qDebug() << "=====desktop file's watcher add path:" << appPath;
                }
                watcher->addWatch(dir.absolutePath());
                qDebug() << "=====desktop dirs' watcher add path:" << dir.absolutePath();
            }
        }
    };

    auto handlePathDisappeared = [&, watcher](const QString &path, bool isDir) {
        if (isDir) {
            if (m_watcherMap.keys().contains(path)) {
                QDir dir(path);
                //会有类似rm -r的一次性将子文件夹删除的情况，需要挨层判断
                while (!dir.exists()) {
                    dir.setPath(dir.absolutePath().left(dir.absolutePath().lastIndexOf("/")));
                    if (dir.absolutePath().isEmpty()) {
                        dir.setPath("/");
                        break;
                    }
                }
                watcher->addWatch(dir.absolutePath());
                qDebug() << "=====desktop dirs' watcher add path:" << dir.absolutePath();
            }
        }
    };

    connect(watcher, &FileSystemWatcher::created, this, handlePathAppeared);
    connect(watcher, &FileSystemWatcher::moveTo, this, handlePathAppeared);
    connect(watcher, &FileSystemWatcher::deleted, this, handlePathDisappeared);
    connect(watcher, &FileSystemWatcher::moved, this, handlePathDisappeared);
}

ApplicationDirWatcherHelper::~ApplicationDirWatcherHelper() {
    for (auto it = m_watcherMap.constBegin(); it != m_watcherMap.constEnd(); it++) {
        it.value()->deleteLater();
    }
}
