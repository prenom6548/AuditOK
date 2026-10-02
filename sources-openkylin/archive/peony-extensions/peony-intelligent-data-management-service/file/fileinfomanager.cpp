/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "fileinfomanager.h"
#include "file/qtremoteobject/fileeventhandlerimpl.h"

#include <QWeakPointer>
#include <QString>
#include <QHash>
#include <QThread>
#include <QEventLoop>
#include <QMutex>
#include <QMutexLocker>
#include <QUrl>
#include <QtRemoteObjects/QRemoteObjectHost>

#include <kysdk/kysdk-system/libkyfilewatcher.hpp>
#include <peony-qt/file-label-model.h>

#include <QDebug>

namespace UKUI {
namespace IDM {

class FileInfoManagerPrivate
{
public:
    FileInfoManagerPrivate();
    ~FileInfoManagerPrivate();

    QHash<QString, QWeakPointer<FileInfo>> hash;
    KYSDK_FILEWATCHER::FileWatcher *watcher;

    QThread *watcherThread;
    QEventLoop *watcherLoop;

    QRemoteObjectHost *host;
    FileEventHandlerImpl *impl;

    QMutex mutex;
    QMutex watcherMutex;
};

FileInfoManagerPrivate::FileInfoManagerPrivate()
{
    watcher = new KYSDK_FILEWATCHER::FileWatcher;

    watcherThread = new QThread;

    watcherLoop = new QEventLoop;

    host = new QRemoteObjectHost;
    impl = new FileEventHandlerImpl;
}

FileInfoManagerPrivate::~FileInfoManagerPrivate()
{
    watcherThread->quit();
    watcherThread->wait();
    host->disableRemoting(impl);
    watcherLoop->quit();
    watcher->clearWatchList();
    watcher->disconnect();
    delete watcher;
    delete watcherLoop;
    delete watcherThread;
    delete impl;
    delete host;
}

}
}

using namespace UKUI::IDM;

static FileInfoManager *instance = nullptr;

FileInfoManager *FileInfoManager::globalInstance()
{
    if (!instance) {
        instance = new FileInfoManager;
    }
    return instance;
}

FileInfoManager::FileInfoManager(QObject *parent) : QObject(parent)
{
    // 在主线程中初始化标记model
    FileLabelModel::getGlobalModel();

    priv = new FileInfoManagerPrivate;

    priv->watcher->moveToThread(priv->watcherThread);
    priv->watcherLoop->moveToThread(priv->watcherThread);
    priv->host->moveToThread(priv->watcherThread);
    priv->impl->moveToThread(priv->watcherThread);
    connect(priv->watcherThread, &QThread::started, priv->watcherLoop, [=]{
        QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
        QString display(qgetenv(displayEnv.toUtf8().data()));
        QUrl nodeUrl(QStringLiteral("local:ukui-idm-") + QString(qgetenv("USER")) + display);
        priv->host->setHostUrl(nodeUrl);
        priv->host->enableRemoting(priv->impl);
        priv->watcherLoop->exec();
    });

    connect(priv->impl, &FileEventHandlerImpl::reciveClientFileEvent, this, &FileInfoManager::remoteFileEvent);

    qRegisterMetaType<FileEventType>("FileEventType");

//    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileAccessed, this, [=](QString name, QString parent){
//        Q_EMIT fileChanged(FileAccessed, name, parent);
//    });
    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileAttrChanged, this, [=](QString name, QString parent){
        Q_EMIT fileChanged(FileAttrChanged, name, parent);
    });
//    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileClosed, this, [=](QString name, QString parent){
//        Q_EMIT fileChanged(FileClosed, name, parent);
//    });
//    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileCreated, this, [=](QString name, QString parent){
//        Q_EMIT fileChanged(FileCreated, name, parent);
//    });
    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileDeleted, this, [=](QString name, QString parent){
        Q_EMIT fileChanged(FileDeleted, name, parent);
    });
    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileIgnoredAutomatic, this, [=](QString name, QString parent){
        Q_EMIT fileChanged(FileIgnoredAutomatic, name, parent);
    });
    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileModified, this, [=](QString name, QString parent){
        Q_EMIT fileChanged(FileModified, name, parent);
    });
    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileMoveIn, this, [=](QString name, QString parent){
        Q_EMIT fileChanged(FileMoveIn, name, parent);
    });
    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileMoveOut, this, [=](QString name, QString parent){
        Q_EMIT fileChanged(FileMoveOut, name, parent);
    });
//    connect(priv->watcher, &KYSDK_FILEWATCHER::FileWatcher::fileOpened, this, [=](QString name, QString parent){
//        Q_EMIT fileChanged(FileOpened, name, parent);
//    });

    priv->watcherThread->start();
}

FileInfoManager::~FileInfoManager()
{
    delete priv;
}

QSharedPointer<FileInfo> FileInfoManager::getInfoByUri(const QString &uri)
{
    QMutexLocker l(&priv->mutex);
    auto info = priv->hash.value(uri);
    return info;
}

void FileInfoManager::insertInfo(QSharedPointer<FileInfo> info)
{
    QMutexLocker l(&priv->mutex);
    priv->hash.insert(info->uri(), info);
}

void FileInfoManager::removeInfoPtrByUri(const QString &uri)
{
    QMutexLocker l(&priv->mutex);
    priv->hash.remove(uri);
}

void FileInfoManager::addFileToWatcher(const QString &url, bool *ok)
{
    if (url.isEmpty()) {
        if (ok) {
            *ok = false;
        }
        return;
    }
    QUrl tmp(url);
    QString path = tmp.toLocalFile();
    if (!path.startsWith("/")) {
        if (ok) {
            *ok = false;
            return;
        }
    }
    QMutexLocker l(&priv->watcherMutex);
    int ret = priv->watcher->addWatchTarget(path,
                                            KYSDK_FILEWATCHER::FileWatcherType::PERIODIC,
                                            KYSDK_FILEWATCHER::FileWatcherAttribute::MODIFY|KYSDK_FILEWATCHER::FileWatcherAttribute::DELETE|KYSDK_FILEWATCHER::FileWatcherAttribute::ATTRIB);
    if (ok) {
        *ok = !ret; // 为0时成功
    }
}

QStringList FileInfoManager::addFilesToWatcher(const QStringList &urls)
{
    QStringList failedFiles;
    for (auto url : urls) {
        bool ok = false;
        addFileToWatcher(url, &ok);
        if (!ok && !url.isEmpty()) {
            failedFiles << url;
        }
    }
    return failedFiles;
}

void FileInfoManager::removeFileFromWatcher(const QString &url, bool *ok)
{
    if (url.isEmpty()) {
        if (ok) {
            *ok = false;
        }
        return;
    }
    QUrl tmp(url);
    QString path = tmp.toLocalFile();
    QMutexLocker l(&priv->watcherMutex);
    int ret = priv->watcher->removeWatchTarget(path);
    if (ok) {
        *ok = !ret; // 为0时成功
    }
}

QStringList FileInfoManager::removeFilesFromWatcher(const QStringList &urls)
{
    QStringList failedFiles;
    for (auto url : urls) {
        bool ok = false;
        removeFileFromWatcher(url, &ok);
        if (!ok && !url.isEmpty()) {
            failedFiles << url;
        }
    }
    return failedFiles;
}
