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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "config.h"
#include <QDir>
#include <mutex>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>

static const QString HOME_PATH = QDir::homePath();
static const QString OLD_BLOCK_DIR_SETTINGS = HOME_PATH + "/.config/org.ukui/ukui-search/ukui-search-block-dirs.conf";
static const QString OLD_INDEXABLE_DIR_SETTINGS = HOME_PATH + "/.config/org.ukui/ukui-search/ukui-search-current-indexable-dir.conf";
static const QString SEARCH_DIRS_SETTINGS = HOME_PATH + "/.config/org.ukui/ukui-search/ukui-search-dirs.json";
static const QString SEARCH_DIRS_SETTINGS_DIR = HOME_PATH + "/.config/org.ukui/ukui-search";
static const QString INDEXABLE_DIR_KEY = "IndexableDir"; //兼容历史版本
static const QString CONFIG_VERSION_KEY = "ConfigVersion";
/**
 * changelog: 1.0 to 1.1 新增/sysroot为全局黑名单
 */
static const QString CONFIG_VERSION = "1.1";
static const QString BLOCK_DIRS_FOR_USER_KEY = "BlockDirsForUser";
static const QString GLOBAL_BLACK_LIST_KEY = "GlobalBlackList";
static const QStringList GLOBAL_BLACK_LIST{"/proc", "/sys", "/dev", "/tmp", "/run", "/sysroot"};
static const QString GLOBAL_SETTINGS_GROUP = "GlobalSettings";
static const QString SEARCH_DIRS_GROUP = "SearchDirs";
static const QString EXTRA_DIRS_GROUP = "ExtraDirs";

static std::once_flag flag;
static Config *global_instance = nullptr;
Config *Config::self()
{
    std::call_once(flag, [ & ] {
        global_instance = new Config();
    });
    return global_instance;
}

void Config::addDir(const SearchDir &dir)
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    QJsonArray blackList;
    for(const QString& path : dir.getBlackList()) {
        blackList.append(path);
    }
    searchDirData.insert(dir.getPath(), blackList);
    m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
    save(m_settingsData);
}

QStringList Config::removeDir(const QString &path)
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    QStringList blackDirs = searchDirData.take(path).toVariant().toStringList();
    m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
    save(m_settingsData);
    return blackDirs;
}

void Config::setExtraDirs(const QStringList extraDirs)
{
    m_settingsData.insert(EXTRA_DIRS_GROUP, QJsonArray::fromStringList(extraDirs));
    save(m_settingsData);
}

int Config::addBlockDirOfUser(const QString &dir)
{
    if (!QFile::exists(dir)) {
        return SearchDir::NotExists;
    }

    QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
    QJsonArray blockDirsOfUser = globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toArray();

    for (auto it = blockDirsOfUser.begin(); it !=blockDirsOfUser.end();) {
        QString blockDir = (*it).toString();
        if (dir == blockDir || dir.startsWith(blockDir + "/") || blockDir == "/") {
            return SearchDir::Duplicated;
        }
        //有子文件夹已被添加，删除这些子文件夹
        if (blockDir.startsWith(dir + "/") || dir == "/") {
            it = blockDirsOfUser.erase(it);
        } else {
            it++;
        }
    }

    blockDirsOfUser.append(dir);
    globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirsOfUser);
    m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
    save(m_settingsData);
    return SearchDir::Successful;
}

void Config::removeBlockDirOfUser(const QString &dir)
{
    QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
    QJsonArray blockDirs = globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toArray();
    bool removeItem(false);

    for (auto it = blockDirs.begin(); it != blockDirs.end();) {
        if ((*it).toString() == dir) {
            it = blockDirs.erase(it);
            removeItem = true;
        } else {
            it++;
        }
    }
    if (removeItem) {
        globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirs);
        m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        save(m_settingsData);
    }
}

bool Config::isCompatibilityMode()
{
    return m_compatibilityMode;
}

void Config::save(const QJsonObject &jsonDocData)
{
    QDir dir;
    QString configFileDir(SEARCH_DIRS_SETTINGS_DIR);
    if (!dir.exists(configFileDir)) {
        if (!dir.mkdir(configFileDir)) {
            qWarning() << "Unable to create settings config file.";
            return;
        }
    }

    QFile settingsFile(SEARCH_DIRS_SETTINGS);
    if(!settingsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Fail to open file " << SEARCH_DIRS_SETTINGS;
    }

    if (settingsFile.write(QJsonDocument(jsonDocData).toJson()) == -1) {
        qWarning() << "Error saving configuration file.";
    }
    settingsFile.flush();
    settingsFile.close();
}

void Config::initSettingsData(QJsonObject &data)
{
    //全局配置
    QJsonObject globalSettings;

    QJsonArray blackList;
    for(const QString& path : GLOBAL_BLACK_LIST) {
        blackList.append(path);
    }
    globalSettings.insert(GLOBAL_BLACK_LIST_KEY, blackList);

    data.insert(CONFIG_VERSION_KEY, CONFIG_VERSION);
    data.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
}

void Config::initDefaultCompatibleConfig()
{
    if (m_compatibleCache.isEmpty()) {
        SearchDir dir(HOME_PATH, false);
        m_compatibleCache.append(dir);
    }
    m_compatibilityMode = true;
}

QStringList Config::searchDirs()
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    QStringList resultList;
    bool needSave = false;

    for (const QString& path : searchDirData.keys()) {
        if (!QFile::exists(path)) {
            searchDirData.remove(path);
            needSave = true;
            continue;
        }

        if (path != QDir::cleanPath(path)) {
            searchDirData.insert(QDir::cleanPath(path), searchDirData.value(path));
            searchDirData.remove(path);
            needSave = true;
        }
        resultList.append(QDir::cleanPath(path));
    }

    if (needSave) {
        m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
        save(m_settingsData);
    }

    resultList.append(extraDirs());
    return resultList;
}

QStringList Config::blackDirs()
{
    QStringList tmp;
    bool needSave = false;
    QJsonObject searchDir = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    for(const QString& path : searchDir.keys()) {
        if(QFile::exists(path)) {
            tmp.append(searchDir.value(path).toVariant().toStringList());
        } else {
            searchDir.remove(path);
            needSave = true;
        }
    }
    if(needSave) {
        m_settingsData.insert(SEARCH_DIRS_GROUP, searchDir);
        save(m_settingsData);
    }
    return tmp;
}

QStringList Config::globalBlackList()
{
    auto cleanBlackList = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject().value(GLOBAL_BLACK_LIST_KEY).toVariant().toStringList();
    if (cleanBlackList.removeDuplicates()) {
        auto globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
        globalSettings.insert(GLOBAL_BLACK_LIST_KEY, QJsonArray::fromStringList(cleanBlackList));
        m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        save(m_settingsData);
    }
    return cleanBlackList;
}

QStringList Config::blockDirsForUser()
{
    QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
    QStringList blockDirs;
    bool needSave(false);

    for (const QString& dir : globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toVariant().toStringList()) {
        if (!QFile::exists(dir)) {
            needSave = true;
            continue;
        }

        if (dir != QDir::cleanPath(dir)) {
            needSave = true;
        }
        blockDirs << QDir::cleanPath(dir);
    }

    if (needSave) {
        QJsonArray array;
        for (const QString& dir : blockDirs) {
            array << dir;
        }
        globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, array);
        m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        save(m_settingsData);
    }
    return blockDirs;
}

QStringList Config::extraDirs()
{
    bool needSave = false;
    QStringList extraDirs = m_settingsData.value(EXTRA_DIRS_GROUP).toVariant().toStringList();
    QStringList tmp = extraDirs;
    for(const QString& path : extraDirs) {
        if(!QFile::exists(path)) {
            tmp.removeAll(path);
            needSave = true;
        }
    }

    if (needSave) {
        m_settingsData.insert(EXTRA_DIRS_GROUP, QJsonArray::fromStringList(tmp));
        save(m_settingsData);
    }
    return tmp;
}

bool Config::showInUkcc(const QString &dir)
{
    if (extraDirs().contains(dir)) {
        return false;
    }
    return true;
}

Config::Config(QObject *parent) : QObject(parent)
{
    QJsonArray blockDirsForUser;
    bool oldBlockSettingsExists(false);
    if (QFile::exists(OLD_BLOCK_DIR_SETTINGS)) {
        oldBlockSettingsExists = true;
        QSettings oldBlockSettings(OLD_BLOCK_DIR_SETTINGS, QSettings::IniFormat);
        for(const QString& blockDir : oldBlockSettings.allKeys()) {
            QString wholePath = "/" + blockDir;
            if(QFile::exists(wholePath)) {
                blockDirsForUser.append(QDir::cleanPath(wholePath));
            }
        }
        QFile::remove(OLD_BLOCK_DIR_SETTINGS);
    }

    QSettings oldSettings(OLD_INDEXABLE_DIR_SETTINGS, QSettings::IniFormat);
    QFile settingsFile(SEARCH_DIRS_SETTINGS);

    if(!settingsFile.exists()) {
        qDebug() << oldSettings.value(INDEXABLE_DIR_KEY + "/" + INDEXABLE_DIR_KEY).toStringList();
        for(const QString& path : oldSettings.value(INDEXABLE_DIR_KEY + "/" + INDEXABLE_DIR_KEY).toStringList()) {
            if(QFile::exists(path)) {
                SearchDir dir(path, false);
                qDebug() << "Found old config path" << path;
                m_compatibleCache.append(dir);
            }
        }

        //老版本配置会在更新后被清空，如果allKeys为空说明已经运行过新版本，此时如果新版配置不存在则添加家目录作为默认搜索目录
        if (oldSettings.allKeys().isEmpty()) {
            initDefaultCompatibleConfig();
        }
        oldSettings.clear();
        oldSettings.sync();

        initSettingsData(m_settingsData);
        if (!blockDirsForUser.isEmpty()) {
            QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
            globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirsForUser);
            m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
        }
        save(m_settingsData);
        return;
    }

    if (!settingsFile.open(QFile::ReadOnly)) {
        qWarning() << "SettingsManagerPrivate: configuration file " << settingsFile.fileName() << "open failed !";
        return;
    }
    QByteArray byteArray = settingsFile.readAll();
    settingsFile.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "Incorrect configuration files. JSON parse error";
        initSettingsData(m_settingsData);
        initDefaultCompatibleConfig();
        save(m_settingsData);
        return;
    }
    // 读取配置文件
    m_settingsData = jsonDocument.object();
    QString version = m_settingsData.value(CONFIG_VERSION_KEY).toString();
    if (!version.isEmpty() && version != QString(CONFIG_VERSION)) {
        qWarning() << "Settings version check failed, old: " << version << " new:" << CONFIG_VERSION;
        if (version == "1.0") {
            auto globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
            auto globalBlackList = globalSettings.value(GLOBAL_BLACK_LIST_KEY).toArray();

            const auto& newPath = "/sysroot";
            if (!globalBlackList.contains(newPath)) {
                globalBlackList << newPath;
                globalSettings.insert(GLOBAL_BLACK_LIST_KEY, globalBlackList);
                m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
            }
            m_settingsData.insert(CONFIG_VERSION_KEY, CONFIG_VERSION);
            save(m_settingsData);
        }
    } else {
        if (!m_settingsData.keys().contains(CONFIG_VERSION_KEY) || !m_settingsData.keys().contains(GLOBAL_SETTINGS_GROUP)) {
            initSettingsData(m_settingsData);
            save(m_settingsData);
        }

        if (!m_settingsData.keys().contains(SEARCH_DIRS_GROUP)) {
            initDefaultCompatibleConfig();
        }

        QJsonObject globalSettings = m_settingsData.value(GLOBAL_SETTINGS_GROUP).toObject();
        //旧的配置文件存在则需要进行同步旧配置
        if (oldBlockSettingsExists) {
            globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, blockDirsForUser);
            m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
            save(m_settingsData);
        } else {
            QStringList blockDirs;
            bool needSave(false);
            bool needCheck(false);

            for (const QString& dir : globalSettings.value(BLOCK_DIRS_FOR_USER_KEY).toVariant().toStringList()) {
                if (!QFile::exists(dir)) {
                    needSave = true;
                    continue;
                }

                if (dir != QDir::cleanPath(dir)) {
                    needSave = true;
                    needCheck = true;
                }
                blockDirs << QDir::cleanPath(dir);
            }

            if (needSave) {
                if (needCheck) {
                    filterSubDirs(blockDirs);
                }
                QJsonArray array;
                for (const QString& dir : blockDirs) {
                    array << dir;
                }
                globalSettings.insert(BLOCK_DIRS_FOR_USER_KEY, array);
                m_settingsData.insert(GLOBAL_SETTINGS_GROUP, globalSettings);
                save(m_settingsData);
            }
        }
    }
    checkSearchDir();
}

void Config::checkSearchDir()
{
    QJsonObject searchDirData = m_settingsData.value(SEARCH_DIRS_GROUP).toObject();
    QStringList searchPaths;
    bool needCheck = false;

    for(const QString& path : searchDirData.keys()) {
        if(!QFile::exists(path)) {
            searchDirData.remove(path);
            continue;
        }

        QString cleanPath = QDir::cleanPath(path);
        if (path != cleanPath) {
            searchDirData.insert(cleanPath, searchDirData.value(path));
            searchDirData.remove(path);
            needCheck = true;
        }
        searchPaths.append(cleanPath);
    }

    if (needCheck) {
        QStringList removedDirs = filterSubDirs(searchPaths);
        for (const QString & removedPath : removedDirs) {
            searchDirData.remove(removedPath);
        }
        m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirData);
        save(m_settingsData);
    }
}

void Config::processCompatibleCache()
{
    if(this->isCompatibilityMode() && !m_compatibleCache.isEmpty()) {
        QJsonObject searchDirsData;
        for (SearchDir dir : m_compatibleCache) {
            dir.generateBlackList();
            if (dir.error() == SearchDir::ErrorInfo::Successful) {
                QJsonArray blackList;
                for(const QString& path : dir.getBlackList()) {
                    blackList.append(path);
                }
                searchDirsData.insert(dir.getPath(), blackList);

                m_settingsData.insert(SEARCH_DIRS_GROUP, searchDirsData);
            }
        }
        save(m_settingsData);
        m_compatibleCache.clear();
        m_compatibilityMode = false;
    }
}

QStringList Config::filterSubDirs(QStringList& paths) {
    QStringList filteredDirs;
    QStringList removedDirs;
    paths.sort(Qt::CaseInsensitive);
    for (const QString& dir : paths) {
        bool isSub = false;
        for (const QString& keptBlock : filteredDirs) {
            if (dir.startsWith(keptBlock + "/") || keptBlock == "/") {
                isSub = true;
                break;
            }
        }
        if (!isSub) {
            filteredDirs << dir;
        } else {
            removedDirs << dir;
        }
    }
    paths = filteredDirs;
    return removedDirs;
}