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
#include "search-file-item.h"
#include"QTimer"
#include"file-utils.h"
SearchFileItem::SearchFileItem(std::shared_ptr<Peony::FileInfo> info, SearchFileItem *parentItem, SearchFileItemModel *model, QObject *parent)
{
//    qRegisterMetaType<QVector<SearchFileItem*>* >("QVector<FileItem*>*");
//    qRegisterMetaType<QHash<QString, SearchFileItem*>>("QHash<QString, FileItem*>");
//    qRegisterMetaType<std::vector<std::shared_ptr<FileInfo> >>("std::vector<std::shared_ptr<FileInfo> >&");
    m_parent = parentItem;
    m_info = info;
    m_children = new QVector<SearchFileItem*>();
    m_uri_item_hash.clear();
    m_model = model;
    setParent(m_model);
    if(this->m_info!=nullptr){
//        m_watcher = std::make_shared<FileWatcher>(this->m_info->uri(), nullptr, true);
//        //m_watcher->startMonitor();
    }else{
        needConnectSignal = true;
    }
    m_thumbnail_watcher = std::make_shared<Peony::FileWatcher>("thumbnail://");
    connectFunc();

}

SearchFileItem::~SearchFileItem()
{
    if (m_children) {
        qDeleteAll(*m_children);  // 删除所有指针指向的对象
        m_children->clear();      // 清空向量
    }
    m_uri_item_hash.clear();
    disconnect(m_model->m_fileManagerThread, &FileManagerThread::finishQueryFileInfos, this, nullptr);
}

const QString SearchFileItem::uri()
{
    return m_info->uri();
}

void SearchFileItem::appendChild(SearchFileItem *child)
{
    if (!child) return;

    m_children->append(child);
    if(m_info!=nullptr){
       m_uri_item_hash.insert(child->uri(), child);
    }
    child->m_parent = this;
}

int SearchFileItem::childCount() const
{
    return m_children->size();
}

void SearchFileItem::connectFunc()
{
    if(needConnectSignal){
        connect(m_model->m_fileManagerThread, &FileManagerThread::finishQueryFileInfos, this, [=](std::vector<std::shared_ptr<FileInfo> >& retFileInfos, SearchFileItem *parentItem,bool isfinish,bool isupdate){
            /* 查询结果返回，更新数据 */
            if(isupdate){
                for (auto& info : retFileInfos) {
                    auto item = m_uri_item_hash.value(info->uri());
                    if (!item)
                        continue;
                    Peony::FileInfoManagerHelper::updateFileInfo(info);

                }
                m_model->updated();
            }else{
                if(parentItem && this != parentItem)
                    return;
                if(!retFileInfos.empty()){
                    m_model->clearNoResultsDisplay(parentItem);
                }
                m_item_type = parentItem;
                QModelIndex model = m_model->indexFromItem(parentItem);
                m_model->beginInsertRows(model, m_children->count(), m_children->count() + retFileInfos.size() -1);
                for (auto& info : retFileInfos) {
                    if(this->getChildFromUri(info->uri())){
                        continue;
                    }
                    Peony::FileInfoManagerHelper::updateFileInfo(info);
                    auto item = new SearchFileItem(info, this, m_model);
                    m_children->append(item);
                    m_uri_item_hash.insert(item->uri(), item);
                    ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                }
                m_model->endInsertRows();
                if(isfinish){
                    m_model->finishSearch();
                }
            }
        });
    }
    connect(m_thumbnail_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri){
        auto index = m_model->indexFromUri(uri,m_item_type);
        if (index.isValid()) {
            auto item = m_model->itemFromIndex(index);
            if (item) {
                m_model->updated();
                //m_model->thumbnailUpdated(uri);
            }
        }
    });
    connect(m_thumbnail_watcher.get(), &FileWatcher::thumbnailUpdated, this, [=](const QString &uri) {
            m_model->updated();
            //m_model->dataChanged(m_model->indexFromUri(uri), m_model->indexFromUri(uri));
        }, Qt::UniqueConnection);
}

void SearchFileItem::updateInfoAsync()
{
    FileInfoJob *job = new FileInfoJob(m_info);
    job->setAutoDelete();
    job->connect(job, &FileInfoJob::infoUpdated, this, [=]() {
        m_model->updated();
        ThumbnailManager::getInstance()->createThumbnail(this->uri(), m_thumbnail_watcher, true);
        //Q_EMIT m_model->updateFilter();
    });
    job->queryAsync();
}

SearchFileItem *SearchFileItem::getChildFromUri(QString uri)
{
    // optimize
    auto index = m_model->indexFromUri(uri,m_item_type);
    if (index.isValid()) {
        return m_model->itemFromIndex(index);
    }
    return nullptr;
}

void SearchFileItem::findChildrenAsync()
{
    auto info = FileInfo::fromUri(m_info.get()->uri());
    auto infoJob = new FileInfoJob(info);
    infoJob->setAutoDelete();
    infoJob->queryAsync();

    Peony::FileEnumerator *enumerator = new Peony::FileEnumerator;
    enumerator->setEnumerateDirectory(m_info->uri());
    //NOTE: entry a new root might destroyed the current enumeration work.
    //the root item will be delete, so we should cancel the previous enumeration.
    enumerator->connect(this, &SearchFileItem::cancelFindChildren, enumerator, &FileEnumerator::cancel);
    enumerator->connect(enumerator, &FileEnumerator::cancelled, m_model, [=](){
        // if (enumerator->getEnumerateUri() != m_model->getRootUri()) {
        //     // try fix #164883, error cusor while searching
        //     return;
        // }
        //m_model->findChildrenFinished();
    });

    enumerator->connect(enumerator, &FileEnumerator::prepared, this, [=](std::shared_ptr<GErrorWrapper> err, const QString &targetUri, bool critical) {
        if (!targetUri.isNull()) {
            if (targetUri != this->uri()) {
                this->m_info = FileInfo::fromUri(targetUri);

                GFile *targetFile = g_file_new_for_uri(targetUri.toUtf8().constData());
                QUrl targetUrl = targetUri;
                auto path = g_file_get_path(targetFile);
                enumerator->setEnumerateDirectory(targetFile);
                g_object_unref(targetFile);
            }
            enumerator->enumerateAsync();
            return;
        }

        auto target = Peony::FileUtils::getTargetUri(m_info->uri());
        if (!target.isEmpty()) {
            enumerator->cancel();
            m_model->setRootUri(target);
            return;
        }

        enumerator->enumerateAsync();
    });


    enumerator->connect(enumerator, &Peony::FileEnumerator::childrenUpdated, this, [=](const QStringList &uris, bool isEnding) {
        if (uris.isEmpty()) {
            if (isEnding) {
                Q_EMIT m_model->updated();
            }
        }

        if (!m_children) {
            enumerator->disconnect();
            delete enumerator;
            return ;
        }

        childrenUpdateOfEnumerate(uris, isEnding);
    });

    enumerator->connect(enumerator, &Peony::FileEnumerator::enumerateFinished, this, [=](bool successed) {
        delete enumerator;

        if (!successed) {
            //Q_EMIT m_model->findChildrenFinished();
            return;
        }

        if (!m_model||!m_children||!m_info)
            return;

    });

    enumerator->prepare();
}

void SearchFileItem::childrenUpdateOfEnumerate(const QStringList &uris, bool isEnding)
{
    uris.toSet().toList();/* 去重 */
    if(uris.size()<=0){
        if(isEnding){
            m_model->updateSearchstatus(true);
            m_model->finishSearch();
        }
        return;
    }
    auto parentitem = m_model->getFileNameItem();
    QStringList originalList = uris; /* 原始列表 */
    /* 遍历时第一次先加载100个显示在桌面上，其余按大批量查询 */
    QStringList firstTimeUris;
    int count = 100;
    if(isEnding){
        m_model->updateSearchstatus(true);
    }
    if(originalList.size() > count){
        for(int idx = 0; idx < count; idx++){
            QString uri = originalList.takeFirst();
            firstTimeUris.append(uri);
        }
        Q_EMIT m_model->setUrisForBatchQueryInfos(firstTimeUris, parentitem, isEnding);
    }

    /* 大批量查询 */
    int chunkSize = 1000; /* 每个列表的大小 */
    QList<QStringList> splitLists;
    if(originalList.size() > chunkSize){
        int numChunks = originalList.count() / chunkSize;
        if (originalList.count() % chunkSize)
            numChunks++;

        for (int i = 0; i < numChunks; i++) {
            QStringList chunkList;
            for (int j = 0; j < chunkSize && ((i * chunkSize) + j) < originalList.count(); j++) {
                chunkList.append(originalList.at((i * chunkSize) + j));
            }
            splitLists.append(chunkList);
        }
    }else{
        splitLists.append(originalList);
    }

    for(auto &queryUris : splitLists){
        Q_EMIT m_model->setUrisForBatchQueryInfos(queryUris, parentitem, isEnding);
    }//end
}

