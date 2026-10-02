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

#ifndef FILEITEMMODEL_H
#define FILEITEMMODEL_H

#include <QAbstractTableModel>
#include "../file/fileinfo.h"
#include "../file/fileinfomanager.h"

class QTimer;

class FileItemModel : public QAbstractTableModel
{
    friend class ModelController;
    friend class ModelTest;
    Q_OBJECT

public:
    explicit FileItemModel(QObject *parent = nullptr); // adapt for qml, also used in recent view
    explicit FileItemModel(int spaceId, const QString &spaceName, QList<QSharedPointer<UKUI::IDM::FileInfo>> infos, QObject *parent = nullptr);
    ~FileItemModel();

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void startLoadRecentFiles();
    Q_INVOKABLE void startLoadFiles();

    Q_INVOKABLE int getSpaceId() const;
    Q_INVOKABLE QString getSpaceName() const;

    void handleFileEvent(UKUI::IDM::FileInfoManager::FileEventType type, const QString &arg1, const QString &arg2);

    bool isUserAddedFile(const QString &path);
    bool isUserAddedFile(int index);

    quint64 getLastAddedFileCount() const;
    quint64 getLastAddedFileTimeMsec() const;

    QList<QSharedPointer<UKUI::IDM::FileInfo> > insertFiles(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, bool isAutoAdded);

Q_SIGNALS:
    void filesLoaded();
    void spaceNameChanged();
    void spacesStatusDescriptionChanged();

    void recentFilesChanged();

    void reportError(const QString &uri);

public Q_SLOTS:
    void handleFilesAdded(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &addInfos, int spaceId);
    void handleFilesChangedForConditionsChanged(bool needReload, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &resultFiles, int spaceId);
    void handleFilesReloaded(const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos, int spaceId);

    void handleRecentFileChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    void startCountAddedFiles();

    void initRecentTimer();

    void removeFile(const QString &filePath);

Q_SIGNALS:
    void requestStart();

private:
    int spaceId = -1;

    QString spaceName;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> userAddedInfos;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;

    QTimer *addedFileTimer = nullptr;
    QTimer *recentFileTimer = nullptr;
    quint64 lastAddedFileTimeMsec = 0;
    quint64 lastAddedFileCount = 0;

    quint64 tmpFileCount = 0;
};

#endif // FILEITEMMODEL_H
