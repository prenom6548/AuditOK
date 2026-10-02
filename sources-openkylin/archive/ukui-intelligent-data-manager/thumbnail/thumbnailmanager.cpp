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

#include "thumbnailmanager.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include <QImage>
#include <QTimeLine>

#include <QDebug>

#include <ukui-file-metadata/extractor.h>
#include <ukui-file-metadata/mime-utils.h>
#include <ukui-file-metadata/extractor-manager.h>
#include <ukui-file-metadata/thumbnail.h>
#include <ukui-file-metadata/simple-extraction-result.h>

class ThumbnailManagerPrivate
{
public:
    ThumbnailManagerPrivate();
    ~ThumbnailManagerPrivate();

    void startThumbnail(const QString &path);

    QThread *thread;
    QMutex mutex;
    QString pathWaitForThumbnail;
    QString currentPathForThumbnailing;
    UkuiFileMetadata::ExtractorManager *manager;
    QTimeLine *timeLine;

    bool strictedSignalMode = false;
};

class ThumbnailJob : public QObject
{
    friend class ThumbnailManager;
    Q_OBJECT
public:
    ThumbnailJob(const QString &path, UkuiFileMetadata::ExtractorManager *manager);

signals:
    void thumnailFinished();

public slots:
    void startThumbnail();

private:
    QString path;
    QImage image;
    UkuiFileMetadata::ExtractorManager *manager;
};

ThumbnailManager::ThumbnailManager(QObject *parent) : QObject(parent)
{
    priv = new ThumbnailManagerPrivate;

    connect(priv->timeLine, &QTimeLine::finished, this, [=]{
        if (priv->currentPathForThumbnailing.isEmpty() && !priv->pathWaitForThumbnail.isEmpty()) {
            priv->currentPathForThumbnailing = priv->pathWaitForThumbnail;
            priv->pathWaitForThumbnail.clear();
            if (!priv->thread->isRunning()) {
                auto job = new ThumbnailJob(priv->currentPathForThumbnailing, priv->manager);
                job->moveToThread(priv->thread);
                connect(priv->thread, &QThread::started, job, &ThumbnailJob::startThumbnail);
                connect(job, &ThumbnailJob::thumnailFinished, priv->thread, &QThread::quit);
                connect(priv->thread, &QThread::finished, this, [=]{
                    QImage image = job->image;
                    if (!image.isNull()) {
                        if (priv->strictedSignalMode) {
                            if (priv->pathWaitForThumbnail.isEmpty()) {
                                Q_EMIT this->thumbnailFinished(job->path, image, nullptr);
                            } else {
                                //qDebug() << "stricted mode, not send" << job->path << job->image;
                            }
                        } else {
                            Q_EMIT this->thumbnailFinished(job->path, image, nullptr);
                        }
                    } else {
                        Q_EMIT this->thumbnailFinished(job->path, QImage(), tr("Failed to create image for %1").arg(job->path));
                    }

                    priv->currentPathForThumbnailing.clear();
                    delete job;
                    priv->thread;
                    priv->thread = new QThread;
                    if (priv->timeLine->state() == QTimeLine::NotRunning && !priv->pathWaitForThumbnail.isEmpty())
                        priv->timeLine->start();
                });
                priv->thread->start();
            } else {
                qDebug() << "a thumbnail job is running, waiting previous job finished" << priv->currentPathForThumbnailing;
            }
        } else {
            qDebug() << "a thumbnail job is running, waiting previous job finished" << priv->currentPathForThumbnailing;
        }
    });
}

ThumbnailManager::~ThumbnailManager()
{
    delete priv;
}

void ThumbnailManager::startPreview(const QString &path)
{
    priv->startThumbnail(path);
}

void ThumbnailManager::setStrictedMode(bool strictedMode)
{
    priv->strictedSignalMode = strictedMode;
}

ThumbnailManagerPrivate::ThumbnailManagerPrivate()
{
    thread = new QThread;
    manager = new UkuiFileMetadata::ExtractorManager;
    timeLine = new QTimeLine;
    timeLine->setDuration(100);
    timeLine->setUpdateInterval(100);
}

ThumbnailManagerPrivate::~ThumbnailManagerPrivate()
{
    timeLine->stop();
    thread->quit();
    delete thread;
    delete manager;
    delete timeLine;
}

void ThumbnailManagerPrivate::startThumbnail(const QString &path)
{
    if (path.isEmpty())
        return;

    QMutexLocker l(&mutex);
    if (path == currentPathForThumbnailing)
        return;

    pathWaitForThumbnail = path;
    if (timeLine->state() == QTimeLine::NotRunning)
        timeLine->start();
}

ThumbnailJob::ThumbnailJob(const QString &path, UkuiFileMetadata::ExtractorManager *manager) : path(path), manager(manager)
{

}

void ThumbnailJob::startThumbnail()
{
    QString mimeType = UkuiFileMetadata::MimeUtils::strictMimeType(path, {}).name();
    QList<UkuiFileMetadata::Extractor*> extractors = manager->fetchExtractors(mimeType);
    UkuiFileMetadata::SimpleExtractionResult result(path, mimeType, UkuiFileMetadata::ExtractionResult::Flag::ExtractThumbnail);
    result.setThumbnailRequest(UkuiFileMetadata::ThumbnailRequest(QSize(512, 512), 1));
    for(auto extractor : extractors) {
        extractor->extract(&result);
        if(result.thumbnail().isValid()) {
            image = result.thumbnail().image();
            break;
        }
    }

    Q_EMIT this->thumnailFinished();
}

#include "thumbnailmanager.moc"
