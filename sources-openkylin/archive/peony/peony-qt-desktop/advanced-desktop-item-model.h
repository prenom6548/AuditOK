/*
 * Peony-Qt
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
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#ifndef DESKTOPITEMMODEL_H
#define DESKTOPITEMMODEL_H

#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QQueue>
#include <QPoint>
#include <memory>
#include <QStandardItem>
#include <QSet>

#include "user-dir-manager.h"
//#include "desktop-icon-view.h"
class AdvancedDesktopIconView;
namespace Peony {

class FileEnumerator;
class FileInfo;
class FileWatcher;
class FileOperationInfo;

class AdvancedDesktopIconItem : public QStandardItem
{
public:
    explicit AdvancedDesktopIconItem(const QString &text);
    explicit AdvancedDesktopIconItem(const QString &text, bool fileCreate);
    void setData(const QVariant &value, int role = Qt::UserRole + 1) override;
};

struct IconData {
    int screenId;
    QPoint position;
    QModelIndex modelIndex;
    QString uri;

    bool operator<(const IconData &other) const {
        if (screenId == other.screenId) {
            if (position.x() == other.position.x()) {
                return position.y() < other.position.y();
            }
            return position.x() < other.position.x();
        }
        return screenId < other.screenId;
    }
};

class AdvancedDesktopItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit AdvancedDesktopItemModel(QObject *parent = nullptr);
    ~AdvancedDesktopItemModel() override;
    QHash<int, QByteArray> roleNames() const override;
    const QModelIndex indexFromUri(const QString &uri);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant getEmblems(const QModelIndex &index) const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QMimeData *mimeData(const QModelIndexList& indexes) const override;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    void updateMetaInfo(QStandardItem *item);
    void getAllRestoreInfo();
    void saveExtendItemInfo();
    void resetExtendItemInfo(QSet<int> idList);
    void clearExtendItemPos(bool saveId = false);
    void saveExtendItemInfo(int id);
    std::shared_ptr<FileInfo> getFileInfo(const QModelIndex& index) const;

    void updateTooltips(const QString& uri);

Q_SIGNALS:
    void requestClearIndexWidget(const QStringList &uris = QStringList());
    void refreshed();
    void prepareRefresh();
    void emitFinish();
    void sig_relayoutItems();
    void sig_fileRemoved(const QString &uri);
    void sig_fileCreated(int id, const QString &uri);
    void selectUri(const QString &uri);
    void idChanged(int id, int oldId, const QStringList &uris);
    void invalidateFilterRequest();
    void setInfoPos(const QString &uri);
    void dragFromOtherViewAutoLayout(const QStringList &uris);
public Q_SLOTS:
    void refresh();
    void setdata(int i, const QVariant &value, int role = Qt::EditRole);
protected Q_SLOTS:
    void onEnumerateFinished(bool successed);
    void fileCreated(const QString &uri);
    void pendingQuery(const QString &uri);

private:
    FileEnumerator *m_enumerator;
    QList<std::shared_ptr<FileInfo>> m_files;
    QList<std::shared_ptr<FileInfo>> m_querying_files;
    std::shared_ptr<FileWatcher> m_trash_watcher;
    std::shared_ptr<FileWatcher> m_desktop_watcher;
    std::shared_ptr<FileWatcher> m_thumbnail_watcher; //just handle the thumbnail created.

    QStringList m_items_need_relayout;
    UserdirManager * m_dir_manager;

    std::shared_ptr<FileInfo> m_desktop_info;
    std::shared_ptr<FileOperationInfo> m_renaming_operation_info;

    QPair<QString, QPoint> m_renaming_file_pos;

    bool m_showFileExtension = true;

    QSet<QString> m_pending_query_uris;
    QTimer *m_pending_query_timer = nullptr;

    mutable QSet<QString> m_pendingTooltips;
    QString m_renameingUri;
private:
    void refreshInternal();

    QVariant getTooltipForIndex(const QModelIndex &index) const;
    void setupTooltipConnections();
};

}

#endif // DESKTOPITEMMODEL_H
