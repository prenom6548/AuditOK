/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */
#ifndef SEARCHFILEITEM_H
#define SEARCHFILEITEM_H
#include <QStandardItemModel>
#include "file-info.h"
#include "searchresultmodel.h"
#include "file-info-manager.h"
#include "file-enumerator.h"
using namespace Peony;
class SearchFileItem : public QObject
{
    friend class SearchFileItemModel;
    friend class SearchFilterProxyModel;
    friend class SearchView;
    Q_OBJECT
public:
    explicit SearchFileItem(std::shared_ptr<Peony::FileInfo> info,
                      SearchFileItem *parentItem = nullptr,
                      SearchFileItemModel *model = nullptr,
                      QObject *parent = nullptr);
    ~SearchFileItem();

    const QString uri();
    const std::shared_ptr<FileInfo> info() {
        return m_info;
    }
    void appendChild(SearchFileItem*);
    int childCount() const;
    void updateInfoAsync();
    SearchFileItem *getChildFromUri(QString uri);
    void findChildrenAsync();
    void childrenUpdateOfEnumerate(const QStringList &uris, bool isEnding);
public Q_SLOTS:

Q_SIGNALS:
    // void setUrisForBatchQueryInfos(const QStringList& uris,SearchFileItem *parentItem);
    void cancelFindChildren();
private:
    void connectFunc();
    SearchFileItem *m_parent = nullptr;
    std::shared_ptr<Peony::FileInfo> m_info;
    QVector<SearchFileItem*> *m_children = nullptr;
    QHash<QString, SearchFileItem*> m_uri_item_hash; /* <key:uri,value:fileItem> 必须与m_children同增减！！！ */
    std::shared_ptr<FileWatcher> m_thumbnail_watcher = nullptr;
//    std::shared_ptr<FileWatcher> m_watcher = nullptr;
    SearchFileItemModel *m_model = nullptr;
    SearchFileItem *m_item_type = nullptr;
    bool needConnectSignal = false;
    std::vector<std::shared_ptr<FileInfo>> m_pendingFileInfos;
    int m_currentBatchIndex;
};



#endif // SEARCHFILEITEM_H
