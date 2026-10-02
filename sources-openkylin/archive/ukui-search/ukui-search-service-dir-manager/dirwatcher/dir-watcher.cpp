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
#include "dir-watcher.h"

#include <QDebug>
#include <QDir>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QThread>
#include <fstab.h>
#include <QMutexLocker>
#include "config.h"
#include "fileindexserviceadaptor.h"
#include "volume-manager.h"

#define CURRENT_INDEXABLE_DIR_SETTINGS QDir::homePath() + "/.config/org.ukui/ukui-search/ukui-search-current-indexable-dir.conf"
#define INDEXABLE_DIR_VALUE "IndexableDir"
#define DEFAULT_INDEXABLE_DIR QDir::homePath()

static std::once_flag flag;
static DirWatcher *global_instance = nullptr;

DirWatcher::DirWatcher(QObject *parent) : QObject(parent)
{
    //兼容旧版配置
    Config::self()->processCompatibleCache();

    connect(VolumeManager::self(), &VolumeManager::MountAdded, this, &DirWatcher::mountAdded);
    connect(VolumeManager::self(), &VolumeManager::UDiskMounted, this, &DirWatcher::onUDiskMounted);
    connect(VolumeManager::self(), &VolumeManager::UDiskUnmounted, this, &DirWatcher::onUDiskUnmounted);
    new FileindexAdaptor(this);
}

DirWatcher::~DirWatcher()
{

}

void DirWatcher::refreshExtraDirs()
{
    QStringList before = extraDirs();
    QStringList after = extraDirsOfPaths(Config::self()->searchDirs());

    auto dirs2BAdded = QSet<QString>(after.begin(), after.end()) - QSet<QString>(before.begin(), before.end());
    auto dirs2BRemoved = QSet<QString>(before.begin(), before.end()) - QSet<QString>(after.begin(), after.end());

    if (dirs2BAdded.isEmpty() && dirs2BRemoved.isEmpty()) {
        return;
    }

    Config::self()->setExtraDirs(after);

    for (const QString& addPath : dirs2BAdded) {
        Q_EMIT this->appendIndexItem(addPath, {});
        qDebug() << "Add extra dir:" << addPath;
    }

    for (const QString& removePath : dirs2BRemoved) {
        Q_EMIT this->removeIndexItem(removePath);
        qDebug() << "Remove extra dir:" << removePath;
    }
}

void DirWatcher::onUDiskMounted(const QString &mountPoint)
{
    qDebug() << "UDisk mounted" << mountPoint;
    refreshExtraDirs();
}

void DirWatcher::onUDiskUnmounted(const QString &mountPoint) {
    qDebug() << "UDisk unmounted" << mountPoint;
    refreshExtraDirs();
}

DirWatcher *DirWatcher::getDirWatcher()
{
    std::call_once(flag, [ & ] {
        global_instance = new DirWatcher();
    });
    return global_instance;
}

QStringList DirWatcher::currentIndexableDir()
{
    return currentSearchDirs();
}


QStringList DirWatcher::currentBlackListOfIndex()
{
    return currentBlackList();
}

/**
 * @brief DirWatcher::blackListOfDir 根据传入目录返回当前目录下的所有不可搜索目录，没有则返回空列表
 * @param dirPath 要搜索的目录
 */
QStringList DirWatcher::blackListOfDir(const QString &dirPath)
{
    return SearchDir::blackListOfDir(dirPath);
}

QStringList DirWatcher::extraDirs()
{
    return Config::self()->extraDirs();
}

QStringList DirWatcher::extraDirsOfPaths(const QStringList &paths)
{
    QStringList normalizedPaths;
    normalizedPaths.reserve(paths.size());
    for (const QString &path : paths) {
        normalizedPaths.append(QDir::cleanPath(path));
    }

    // 提前计算黑名单,避免重复计算
    QStringList globalBlackList = Config::self()->globalBlackList();
    QStringList blackDirs;
    QStringList searchDirs = Config::self()->searchDirs();
    // 使用QSet对比提高效率，防止内容相同顺序不同时遍历较慢
    if (QSet(normalizedPaths.begin(), normalizedPaths.end()) == QSet(searchDirs.begin(), searchDirs.end())) {
        blackDirs = Config::self()->blackDirs();
    } else {
        for (const auto& path : normalizedPaths) {
            blackDirs.append(blackListOfDir(path));
        }
    }

    QStringList extraDirs;
    for (const auto& uDisk : VolumeManager::self()->uDiskVolumes()) {
        for (const auto& mountPoint : uDisk.mountPoints()) {
            // 检查是否在索引目录下
            bool needCheck(false);
            for (const auto &indexDir: normalizedPaths) {
                if (mountPoint.startsWith(indexDir + "/") || indexDir == "/") {
                    needCheck = true;
                    break;
                }
            }

            if (!needCheck) {
                continue;
            }

            // 检查是否在全局黑名单下
            bool underGlobalBlacklist(false);
            for (const auto &blackPath: blackDirs) {
                if (mountPoint.startsWith(blackPath + "/") || blackPath == mountPoint) {
                    if (globalBlackList.contains(blackPath)) {
                        underGlobalBlacklist = true;
                        break;
                    }
                }
            }

            if (underGlobalBlacklist) {
                extraDirs << mountPoint;
                break;
            }
        }
    }
    return extraDirs;
}

void DirWatcher::initDbusService()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerObject("/org/ukui/search/fileindex", this, QDBusConnection::ExportAdaptors)){
        qWarning() << "ukui-search-fileindex dbus register object failed reason:" << sessionBus.lastError();
    }
}

QStringList DirWatcher::currentSearchableDir()
{
    return currentSearchDirs();
}

QStringList DirWatcher::searchableDirForSearchApplication()
{
    return currentSearchDirs();
}

int DirWatcher::appendIndexableListItem(const QString &path)
{
    return appendSearchDir(path);
    /* code:
     * 0: successful
     * 1: path or its parent dir has been added
     * 2: path is or under blacklist
     * 3: path is in repeat mounted devices and another path which is in the same device has been indexed
     * 4: another path which is in the same device has been indexed
     * 5: path is not exists
     */
}

bool DirWatcher::removeIndexableListItem(const QString &path)
{
    removeSearchDir(path);
    return true;
}

int DirWatcher::appendSearchDir(const QString &path)
{
    SearchDir dir(path);
    if (dir.error() == SearchDir::Successful) {
        Q_EMIT this->appendIndexItem(dir.getPath(), dir.getBlackList());
        qDebug() << "Add search dir:" << dir.getPath() << "blacklist:" << dir.getBlackList();

        //要添加已索引目录的父目录，先添加索引，再同步配置文件，从而使得下次读取配置文件时不会将子目录排除掉
        for (const QString &searchDir : Config::self()->searchDirs()) {
            if (searchDir.startsWith(dir.getPath() + "/") || dir.getPath() == "/") {
                Config::self()->removeDir(searchDir);
            }
        }

        dir.generateBlackList();
        Config::self()->addDir(dir);
        refreshExtraDirs();
    }
    qWarning() << dir.getPath() << dir.errorString();
    return dir.error();
}

void DirWatcher::removeSearchDir(const QString &path)
{
    const QString normalizedPath = QDir::cleanPath(path);
    Config::self()->removeDir(normalizedPath);
    Q_EMIT this->removeIndexItem(normalizedPath);
    refreshExtraDirs();
    return;
}

QStringList DirWatcher::currentSearchDirs()
{
    return Config::self()->searchDirs();
}

QStringList DirWatcher::currentBlackList()
{
    return Config::self()->blackDirs();
}

QStringList DirWatcher::blockDirsForUser()
{
    return Config::self()->blockDirsForUser();
}

int DirWatcher::addBlockDirOfUser(const QString &dir)
{
    const QString normalizedDir = QDir::cleanPath(dir);
    int res = Config::self()->addBlockDirOfUser(normalizedDir);
    if (res == SearchDir::Successful) {
        Q_EMIT this->blockItemAdded(normalizedDir);
    }
    return res;
}

void DirWatcher::removeBlockDirOfUser(const QString &dir)
{
    const QString normalizedDir = QDir::cleanPath(dir);
    Config::self()->removeBlockDirOfUser(normalizedDir);
    Q_EMIT this->blockItemRemoved(normalizedDir);
}

bool DirWatcher::showInUkcc(const QString &dir) {
    return Config::self()->showInUkcc(QDir::cleanPath(dir));
}
