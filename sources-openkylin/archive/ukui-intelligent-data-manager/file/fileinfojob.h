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

#ifndef FILEINFOJOB_H
#define FILEINFOJOB_H

#include <QObject>
#include <QRunnable>

#include "fileinfo.h"

class Condition;
class QFileInfo;
typedef struct _GFile                         GFile; /* Dummy typedef */
typedef struct _GFileInfo                     GFileInfo;

namespace UKUI {
namespace IDM {
class FileInfoJobPrivate;

class FileInfoJob : public QObject, public QRunnable
{
    Q_OBJECT
public:
    static FileInfoJob *fromUris(const QStringList &uris, bool addWatchByDefault = true);
    static FileInfoJob *fromPaths(const QStringList &paths, bool addWatchByDefault = true);
    static FileInfoJob *fromInfos(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, bool addWatchByDefault = true);
    static bool infoMatchCondtions(const QFileInfo &fileinfo, const QList<QSharedPointer<Condition> > &andConditions, const QList<QSharedPointer<Condition> > &orConditions);
    static bool queryInfoSyncWithConditions(QSharedPointer<FileInfo> &info, const QList<QSharedPointer<Condition>> &andConditions, const QList<QSharedPointer<Condition>> &orConditions);

    ~FileInfoJob();

    void start();
    void cancel();

    /*!
     * \brief setConditions
     * 设置搜索过滤条件
     * \param conditions
     */
    void setConditions(const QList<QSharedPointer<Condition>> &conditions);

    void setSpaceId(int spaceId);

    void run() override;

    QList<QSharedPointer<FileInfo>> getInfos();

signals:
    void started();
    void finished();
    void cancelled();
    void errored(const QString &errorMessage);
    void infoUpdated(const QSharedPointer<FileInfo> &info);

public slots:
    void handleCancelWithSpaceId(int spaceId);

private:
    static void updateInfoWithGFileInfo(QSharedPointer<FileInfo> &info, GFile *gfile, GFileInfo *gfileinfo);
    static bool infoMatchCondtions(QSharedPointer<FileInfo> &info, const QList<QSharedPointer<Condition> > &andConditions, const QList<QSharedPointer<Condition> > &orConditions);

    explicit FileInfoJob(const QStringList &args, bool isUriFormat, bool addWatchByDefault = true, QObject *parent = nullptr);
    FileInfoJob();

    FileInfoJobPrivate *priv;
};

#endif // FILEINFOJOB_H

}
}
