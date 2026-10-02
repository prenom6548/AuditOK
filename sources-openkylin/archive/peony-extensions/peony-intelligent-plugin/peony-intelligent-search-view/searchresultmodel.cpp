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
#include "searchresultmodel.h"
#include "QUrl"
#include "searchresultmodel.h"
#include <QDebug>
#include <QtDBus/QDBusMetaType>
#include <QUuid>
#include <QTimer>
#include <QCollator>
#include <PeonyFileInfo>
#include <file-meta-info.h>
#include <file-utils.h>
#include <file-batch-rename-operation.h>
#include "search-view.h"
#include "file-rename-operation.h"
#include "thumbnail/searchthumbnailmanager.h"
#include <QPixmapCache>
#include <QImageReader>
#include "search-file-item.h"
#include "file-label-model.h"
QLocale locale = QLocale(QLocale::system().name());
QCollator comparer = QCollator(locale);
using namespace Peony;
SearchFileItemModel::SearchFileItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
        qDBusRegisterMetaType<QList<QStringList>>();
        qRegisterMetaType<QList<QStringList>>("QList<QStringList>");
        QDBusConnection connection = QDBusConnection::sessionBus();
        if (!connection.isConnected()) {
            qDebug() << "信号创建失败:" << connection.lastError().message();
            return ;
        }
        // 连接信号
        bool connected = connection.connect("com.peony.idm.service", "/org/peony/idm/space/service", "org.peony.idm.modelcontroller",
                           "searchfileReload", this,SLOT(addFileNameResults(QList<QStringList>,QString)));
        connected = connection.connect("com.peony.idm.service", "/org/peony/idm/space/service", "org.peony.idm.modelcontroller",
                                            "searchfileReloadByBatch", this,SLOT(addFileNameResultsByBatch(QStringList,QString,bool)));
        connection.connect("com.peony.idm.service", "/org/peony/idm/space/service", "org.peony.idm.modelcontroller",
                           "searchContentReload", this,SLOT(addContentResults(QStringList,QString)));
        // connection.connect("com.peony.idm.service", "/org/peony/idm/space/service", "org.peony.idm.modelcontroller",
        //                    "searchTextContentReload", this,SLOT(addTextContentResults(QList<QStringList>,QString)));
        auto settings = GlobalSettings::getInstance();
        m_showFileExtension = settings->isExist(SHOW_FILE_EXTENSION)? settings->getValue(SHOW_FILE_EXTENSION).toBool(): true;
        connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
            if (SHOW_FILE_EXTENSION == key) {
                m_showFileExtension= GlobalSettings::getInstance()->getValue(key).toBool();
                beginResetModel();
                endResetModel();
                Q_EMIT expand(0);
            }
        });

        m_fileManagerThread = new FileManagerThread();
        m_fileManagerThread->moveToThread(m_fileManagerThread);
        connect(this, &SearchFileItemModel::setUrisForBatchQueryInfos, this, [=] (const QStringList& uris, SearchFileItem *parentItem,bool isfinish) {
            auto infos = FileInfo::fromUris(uris);
            m_infosJob = new FileInfosJob(infos, parentItem);
            m_infosJob->connect(this, &SearchFileItemModel::cancelBatchQuery, m_infosJob, &FileInfosJob::batchCancel);
            Q_EMIT m_fileManagerThread->setParamForBatchQueryInfos(m_infosJob, parentItem,isfinish);
        });
        m_rootItem = new SearchFileItem(nullptr, nullptr, this, this);
        m_fileCategory = new SearchFileItem(nullptr, m_rootItem, this, this);
        m_contentCategory = new SearchFileItem(nullptr, m_rootItem, this, this);
        m_rootItem->appendChild(m_fileCategory);
        m_rootItem->appendChild(m_contentCategory);
        m_fileManagerThread->start();
        m_changeChildTimer = new QTimer(this);
        m_changeChildTimer->setSingleShot(true);
        connect(FileLabelModel::getGlobalModel(), &FileLabelModel::fileLabelChanged, this, [=](const QString &uri) {
            QString originalUri = FileUtils::getOriginalUri(uri);
            onChanged(originalUri);
            // this->dataChanged(indexFromUri(originalUri,m_fileCategory), indexFromUri(originalUri,m_fileCategory));
            // this->dataChanged(indexFromUri(originalUri,m_contentCategory), indexFromUri(originalUri,m_contentCategory));
        });

        connect(m_changeChildTimer, &QTimer::timeout, this, [=]{
            m_waiting_update_queue.removeDuplicates(); /* 去重 */
            QStringList list;
            if(m_waiting_update_queue.size() >= 2000){/* 每次批量处理最多数量 */
                for(int i = 0; i < 2000; i++){
                    QString uri = m_waiting_update_queue.takeFirst();
                    list.append(uri);
                }
            }else{
                list.swap(m_waiting_update_queue);
            }
            auto infos = FileInfo::fromUris(list);
            m_infosJob = new FileInfosJob(infos, nullptr);
            m_infosJob->connect(this, &SearchFileItemModel::cancelBatchQuery, m_infosJob, &FileInfosJob::batchCancel);
            Q_EMIT m_fileManagerThread->setParamForBatchQueryInfos(m_infosJob, nullptr,false,true);
            if (m_waiting_update_queue.size()>0 && !m_changeChildTimer->isActive()) {
                m_changeChildTimer->start();
            }
        },Qt::UniqueConnection);
}

SearchFileItemModel::~SearchFileItemModel()
{
    QPixmapCache::clear();
    malloc_trim(0);
}

QModelIndex SearchFileItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    if (!hasIndex(row, column, parent))
            return QModelIndex();

        SearchFileItem *parentItem = nullptr;
        if (!parent.isValid()) {
            parentItem = m_rootItem;
        } else {
            parentItem = static_cast<SearchFileItem*>(parent.internalPointer());
        }

        if (row < parentItem->childCount()) {
            SearchFileItem *childItem = parentItem->m_children->at(row);
            return createIndex(row, column, childItem);
        }
        return QModelIndex();
}

QModelIndex SearchFileItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
      return QModelIndex();

    SearchFileItem *childItem = static_cast<SearchFileItem*>(index.internalPointer());
    SearchFileItem *parentItem = childItem->m_parent;

    if (parentItem == m_rootItem || parentItem == nullptr)
      return QModelIndex();
    if (isFileItem(index)) {
        return createIndex(0, 0, parentItem);
    }
    // 内容项的子节点
    else if (isContentItem(index)) {
        return createIndex(1, 0, parentItem);
    }
}

int SearchFileItemModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) { // 根节点
        return 2; // 两个分类节点
    }
     SearchFileItem *Item = static_cast<SearchFileItem*>(parent.internalPointer());
     return Item->childCount();
}

int SearchFileItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5; // 名称、修改日期、类型、大小 path
}

QVariant SearchFileItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int column = index.column();
    SearchFileItem *item = static_cast<SearchFileItem*>(index.internalPointer());
    if (item == m_noResultsItem || item == m_noContentResultsItem) {
        switch (role) {
        case Qt::DisplayRole:
            if (index.column() == 0) {
                if(item == m_noResultsItem){
                    return tr("No name matching files"); // "暂无名称匹配文件"
                }else if(item == m_noContentResultsItem){
                    return tr("No content matching files"); // "暂无名称匹配文件"
                }
            }
            return QString(); // 其他列返回空字符串
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter; // 文本居中
        case IsNoResultsItemRole:
            return true; // 标识为暂无结果项
        case IsSpanRowRole:
            return true; // 需要跨列显示
        default:
            return QVariant();
        }
    }
    // 分类节点数据
    if (isFileCategory(index)) {
        if (column == 0 && role == Qt::DisplayRole) {
            return tr("File Name Match");
        }
        return QVariant();
    }
    else if (isContentCategory(index)) {
        if (column == 0 && role == Qt::DisplayRole) {
#ifdef ARCH_AVAILABLE_AI
          return tr("Intelligent Content Recognition Match");
#else
          return tr("Text Content");
#endif
        }
        return QVariant();
    }/*else if(isTextContentCategory(id)){
        if (column == 0 && role == Qt::DisplayRole) {
            return tr("Text Content");
        }
    }*/
    auto info = item->m_info;
    if(info == nullptr){
        if(m_hasDefaultContent && role == Qt::DisplayRole && column == 0){
            return QVariant(m_DefaultContent);
        }else{
            return QVariant();
        }
    }
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
        switch (column) {
        case 0: {
            if(!m_showFileExtension){
                if (info->isDir()||(!info->displayName().contains("."))) {
                    return QVariant(info->displayName());
                }
                return QVariant(FileUtils::getBaseNameOfFile(info->displayName()));
            }else{
                return QVariant(info->displayName());
            }
        }
        case 1: return info->modifiedDate();//modifytime format
        case 2: return info->fileType();//filetype
        case 3: return info->fileSize();//sizeformat
        case 4: return info->filePath();
        }
        break;
//    case Qt::BackgroundRole:
//        if (item->type() == SearchFileItem::ContentItem)
//            return QColor(240, 245, 255);
//        break;
    case Qt::UserRole:
        return item->uri();//uri
            break;
    case Qt::UserRole+1:
        return info->filePath();//filepath
            break;
    case Qt::UserRole+2:
            return info->mimeType();//fileminetyle
            break;
    case Qt::UserRole+3:
            return info->size();//int  size
            break;
    case Qt::UserRole+4:
        return info->modifiedTime()*1000;//modifytime int
            break;
    case Qt::UserRole+5:
        return info->iconName();//iconname
            break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if(!m_isAiavailable && isContentItem(index) && info->iconName()=="")
                return "";
            if (!item->m_info->isExistTargetOfSymlink()) {
                return QIcon::fromTheme("unknown");
            }
            return item->m_info->getIcon();
        }
        break;
    }
    return QVariant();
}

QVariant SearchFileItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Modified Date");
        case 2: return tr("File Type");
        case 3: return tr("File Size");
        case 4: return tr("Path");
        }
    }
    return QVariant();
}

Qt::ItemFlags SearchFileItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if(m_isSelecting && ((isContentItem(index) && m_hasDefaultContent)|| isContentCategory(index) || isFileCategory(index))){
        return defaultFlags & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsSelectable;
    }else{
        if(isContentItem(index) && m_hasDefaultContent){
            return defaultFlags & ~Qt::ItemIsDragEnabled;
        }
        if(isContentCategory(index) || isFileCategory(index)){
            return defaultFlags & ~Qt::ItemIsDragEnabled;
        }
    }
    SearchFileItem *item = static_cast<SearchFileItem*>(index.internalPointer());
    if (item == m_noResultsItem || item == m_noContentResultsItem) {
       return Qt::NoItemFlags;
    }
    if (index.isValid()) {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        if (index.column() == 0) {
            flags |= Qt::ItemIsDragEnabled;
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }
}

QMimeData* SearchFileItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QStringList uris;
    QStringList encodedUris;
    for (const QModelIndex &index : indexes) {
        if (index.isValid() && index.column() == 0) {
            QString uri = data(index, Qt::UserRole).toString(); // 假设 URI 存储在 UserRole 中
            uris << uri;
            encodedUris << uri;
        }
    }

    QList<QUrl> urlList;
    for (const QString &uri : uris) {
        urlList << QUrl(uri);
    }

    mimeData->setUrls(urlList);
    auto encodedString = encodedUris.join(" ");
    mimeData->setData("peony-qt/encoded-uris", encodedString.toUtf8());
    auto string = uris.join(" ");
    mimeData->setText(string);
    return mimeData;
}


void SearchFileItemModel::addContentResults(QStringList contentResults,QString clientid)
{
    if (contentResults.isEmpty() || (clientid!=m_clientId)){
        searchContentfinish = true;
        QTimer::singleShot(100, this, [this]() {
            if (!m_isAiavailable) {
                addDefaultContent();
            }
            updateNoContentResultsDisplay();
        });       
        if(searchFilefinish) {
            Q_EMIT searchFinished();
        }
        return;
    }
    searchContentfinish = true;
    bool isfinish = searchFilefinish && searchContentfinish;
    Q_EMIT setUrisForBatchQueryInfos(contentResults,m_contentCategory,isfinish);
    Q_EMIT resizeColFitContent();
}

void SearchFileItemModel::clearResult()
{
    beginRemoveRows(index(0, 0), 0, m_fileCategory->m_children->size() - 1);
    m_fileCategory->m_children->clear();
    m_fileCategory->m_uri_item_hash.clear();
    endRemoveRows();

    beginRemoveRows(index(1, 0), 0, m_contentCategory->m_children->size() - 1);
    m_contentCategory->m_children->clear();
    m_contentCategory->m_uri_item_hash.clear();
    endRemoveRows();

    m_noResultsItem = nullptr;
    m_noContentResultsItem = nullptr;
    m_hasDefaultContent = false;
}

void SearchFileItemModel::cancelFindChildren()
{
    if(m_searchRootItem)
        Q_EMIT m_searchRootItem->cancelFindChildren();
}

// void SearchFileItemModel::clearAll()
// {
//     Q_EMIT layoutAboutToBeChanged();
//     m_fileCategory->m_children->clear();
//     m_contentCategory->m_children->clear();
//     Q_EMIT layoutChanged();
// }

void SearchFileItemModel::keywordchange(QString keyword){
    m_keyword = keyword;
    Q_EMIT cancelBatchQuery();
    cancelFindChildren();
    clearResult();
}

bool SearchFileItemModel::isContentCategory(const QModelIndex &index) const
{
    SearchFileItem *Item = static_cast<SearchFileItem*>(index.internalPointer());
    return Item == m_contentCategory;
}

bool SearchFileItemModel::isFileCategory(const QModelIndex &index) const
{
    SearchFileItem *Item = static_cast<SearchFileItem*>(index.internalPointer());
    return Item == m_fileCategory;
}

bool SearchFileItemModel::isFileItem(const QModelIndex &index) const
{
    SearchFileItem *Item = static_cast<SearchFileItem*>(index.internalPointer());
    if (Item!=nullptr){
        return Item->m_parent == m_fileCategory;
    }
    return false;
}

bool SearchFileItemModel::isContentItem(const QModelIndex &index) const
{
    SearchFileItem *Item = static_cast<SearchFileItem*>(index.internalPointer());
    if (Item!=nullptr){
        return Item->m_parent == m_contentCategory;
    }
    return false;
}

bool SearchFileItemModel::isDefaultContentItem(const QModelIndex &index) const
{
    if(!m_hasDefaultContent){
        return false;
    }
    SearchFileItem *Item = static_cast<SearchFileItem*>(index.internalPointer());
    return Item == m_DefaultContentItem ;
}



QModelIndex SearchFileItemModel::indexFromItem(SearchFileItem *item) const
{
    if (item->m_parent == nullptr) {
        int index = m_rootItem->m_children->indexOf(item);
        if (index == -1) {
            return QModelIndex();
        } else {
            return createIndex(index, 0, item);
        }
    } else {
        int index = item->m_parent->m_children->indexOf(item);
        if (index == -1) {
            return QModelIndex();
        } else {
            return createIndex(index, 0, item);
        }
    }
}

const QModelIndex SearchFileItemModel::indexFromUri(const QString &uri,SearchFileItem *item)
{
    if (!item)
        return QModelIndex();
    if(item->m_uri_item_hash.contains(uri)) {
        auto child = item->m_uri_item_hash[uri];
        return indexFromItemAndUri(child, uri);
    }else if(item->m_uri_item_hash.contains(FileUtils::getEncodedUri(uri))){/* 中文编码问题 */
        QString encodedUri = FileUtils::getEncodedUri(uri);
        auto child = item->m_uri_item_hash[encodedUri];
        return indexFromItemAndUri(child, encodedUri);
    }else if(item->m_uri_item_hash.contains(FileUtils::urlDecode(uri))){/* 中文编码问题 */
        QString decodedUri = FileUtils::urlDecode(uri);
        auto child = item->m_uri_item_hash[decodedUri];
        return indexFromItemAndUri(child, decodedUri);
    }
    return QModelIndex();
}

SearchFileItem *SearchFileItemModel::itemFromIndex(const QModelIndex &index) const
{
    return static_cast<SearchFileItem*>(index.internalPointer());
}

const QModelIndex SearchFileItemModel::indexFromItemAndUri(SearchFileItem *item, const QString &uri)
{
    if(!item)
        return QModelIndex();

    GFile *left = g_file_new_for_uri(item->uri().toUtf8().constData());
    GFile *right = g_file_new_for_uri(uri.toUtf8().constData());
    bool equal = g_file_equal(left, right);
    g_object_unref(left);
    g_object_unref(right);
    if (equal) {
        return indexFromItem(item);
    }
    return QModelIndex();
}

void SearchFileItemModel::addFileNameResultsByBatch(QStringList fileResults, QString clientid, bool finish)
{
    if (fileResults.isEmpty() || clientid != m_clientId) {
        if(finish) {
            searchFilefinish = true;
            if(searchContentfinish) {
                Q_EMIT searchFinished();
            }
            updateNoResultsDisplay(); // 更新显示
        }
        return;
    }
    if( m_noResultsItem != nullptr) {
        int row = m_fileCategory->m_children->indexOf(m_noResultsItem);
        if (row >= 0) {
            beginRemoveRows(index(0, 0), row, row);
            m_fileCategory->m_children->removeAt(row);
            m_noResultsItem->deleteLater();
            m_noResultsItem = nullptr;
            endRemoveRows();
        }
    }
    if(finish) {
        searchFilefinish = true;
    }
    bool isfinish = searchFilefinish && searchContentfinish;
    QTimer::singleShot(10, this, [=](){
        Q_EMIT setUrisForBatchQueryInfos(fileResults, m_fileCategory,isfinish);
    });   
}

void SearchFileItemModel::updateSearchstatus(bool isFinish){
    searchFilefinish = isFinish;
    searchContentfinish = isFinish;
}

void SearchFileItemModel::setClientId(QString clientid)
{
    m_clientId = clientid;
}

void SearchFileItemModel::addDefaultContent()
{
    if (!m_isAiavailable){
        if(searchFilefinish){
            Q_EMIT searchFinished();
        }
        searchContentfinish = true;
    }
    bool hasRealResults = false;
    for (int i = 0; i < m_contentCategory->childCount(); ++i) {
        SearchFileItem *child = m_contentCategory->m_children->at(i);
        if (child != m_noContentResultsItem && child->m_info) {
            hasRealResults = true;
            break;
        }
    }
    if(m_hasDefaultContent || hasRealResults){
        return;
    }
#ifdef ARCH_AVAILABLE_AI
    m_DefaultContent = tr("To Obtain Natural Language Recognition Results, Please in【Settings】->【Global Search】<span style='color:#3790FA;'>Open \"AI Index\"</span> ");
#else
    if(isFileContentOn()){
        return;
    }
    m_DefaultContent = tr("To Obtain the Matching Results of Text, Pictures, Videos. Please<span style='color:#3790FA;'> Open \"File Content Index\"</span>");
#endif
    m_hasDefaultContent = true;
    beginInsertRows(index(1, 0), 0, 0);
    auto item = new SearchFileItem(nullptr, m_contentCategory, this);
    m_DefaultContentItem = item;
    m_contentCategory->m_children->append(item);
    endInsertRows();
    Q_EMIT resizeColFitContent();
}

void SearchFileItemModel::setAiCondition(bool isavailable)
{
    m_isAiavailable = isavailable;
}

void SearchFileItemModel::childRemove(const QString uri)
{
    SearchFileItem *child = getChildFromUri(uri,m_fileCategory);
    if (child) {
        beginRemoveRows(indexFromItem(m_fileCategory), m_fileCategory->m_children->indexOf(child), m_fileCategory->m_children->indexOf(child));
        m_fileCategory->m_children->removeOne(child);
        m_fileCategory->m_uri_item_hash.remove(child->uri());
        child->deleteLater();
        endRemoveRows();
        return;
    }

    child = getChildFromUri(uri,m_contentCategory);
    if (child) {
        beginRemoveRows(indexFromItem(m_contentCategory), m_contentCategory->m_children->indexOf(child), m_contentCategory->m_children->indexOf(child));
        m_contentCategory->m_children->removeOne(child);
        m_contentCategory->m_uri_item_hash.remove(child->uri());
        child->deleteLater();
        endRemoveRows();
        return;
    }

    ThumbnailManager::getInstance()->releaseThumbnail(uri);
}

bool SearchFileItemModel::childRename(const QString& oldUri, const QString& newUri)
{
    qDebug()<<"renamed";
    ThumbnailManager::getInstance()->releaseThumbnail(newUri);
    auto newChild = getChildFromUri(newUri,m_fileCategory);
    if (newChild) {
        qDebug()<<"new child has arealy in view";
        newChild->updateInfoAsync();
    }
    SearchFileItem *child = getChildFromUri(oldUri,m_fileCategory);
    if (child) {
        if (!newChild) {
            int index = m_fileCategory->m_children->indexOf(child);
            m_fileCategory->m_children->at(index)->m_info= FileInfo::fromUri(newUri);
            m_fileCategory->m_uri_item_hash.remove(oldUri);
            child->updateInfoAsync();
            m_fileCategory->m_uri_item_hash.insert(child->uri(), child);

        } else {
            beginRemoveRows(indexFromItem(m_fileCategory), m_fileCategory->m_children->indexOf(child), m_fileCategory->m_children->indexOf(child));
            m_fileCategory->m_children->removeOne(child);
            m_fileCategory->m_uri_item_hash.remove(child->uri());
            child->deleteLater();
            endRemoveRows();
        }
    }

    newChild = getChildFromUri(newUri,m_contentCategory);
    if (newChild) {
        qDebug()<<"new child has arealy in view";
        newChild->updateInfoAsync();
    }
    child = getChildFromUri(oldUri,m_contentCategory);
    if (child) {
        if (!newChild) {
            int index = m_contentCategory->m_children->indexOf(child);
            m_contentCategory->m_children->at(index)->m_info= FileInfo::fromUri(newUri);
            m_contentCategory->m_uri_item_hash.remove(oldUri);
            child->updateInfoAsync();
            m_contentCategory->m_uri_item_hash.insert(child->uri(), child);

        } else {
            beginRemoveRows(indexFromItem(m_contentCategory), m_contentCategory->m_children->indexOf(child), m_contentCategory->m_children->indexOf(child));
            m_contentCategory->m_children->removeOne(child);
            m_contentCategory->m_uri_item_hash.remove(child->uri());
            child->deleteLater();
            endRemoveRows();
        }
    }

    ThumbnailManager::getInstance()->releaseThumbnail(oldUri);
    return true;
}

bool SearchFileItemModel::hasDefaultContent()
{
    return m_hasDefaultContent;
}
SearchFileItem *SearchFileItemModel::getChildFromUri(QString uri,SearchFileItem *item)
{
    // optimize
    auto index = indexFromUri(uri,item);
    if (index.isValid()) {
        return itemFromIndex(index);
    }
    return nullptr;
}

SearchFileItem *SearchFileItemModel::getFileNameItem()
{
    return m_fileCategory;
}

SearchFileItem *SearchFileItemModel::getFileContentItem()
{
    return m_contentCategory;
}

void SearchFileItemModel::finishSearch()
{
    if(searchContentfinish && searchFilefinish) {
        Q_EMIT searchFinished();
        QTimer::singleShot(100, this, [this]() {
            if (!m_isAiavailable) {
                addDefaultContent();
            }
            updateNoContentResultsDisplay();
        });
    }
}

void SearchFileItemModel::setRootUri(const QString &uri)
{
    if (uri.isNull()) {
        setRootUri("file:///");
        m_root_uri = "file:///";
        return;
    }
    searchContentfinish = true;
    m_root_uri = uri;
    auto info = FileInfo::fromUri(uri);
    m_searchRootItem = new SearchFileItem(info, nullptr, this, this);
    m_searchRootItem->findChildrenAsync();
}

void SearchFileItemModel::updateNoResultsDisplay()
{
    // 检查文件分类是否有真实子项
    bool hasRealResults = false;
    for (int i = 0; i < m_fileCategory->childCount(); ++i) {
        SearchFileItem *child = m_fileCategory->m_children->at(i);
        if (child != m_noResultsItem && child->m_info) {
            hasRealResults = true;
            break;
        }
    }
    // 如果没有结果且没有虚拟项，添加虚拟项
    if (!hasRealResults && m_noResultsItem == nullptr) {
        beginInsertRows(index(0, 0), 0, 0);
        m_noResultsItem = new SearchFileItem(nullptr, m_fileCategory, this, this);
        m_fileCategory->appendChild(m_noResultsItem);
        endInsertRows();
    }
    // 如果有结果但存在虚拟项，移除虚拟项
    else if (hasRealResults && m_noResultsItem != nullptr) {
        int row = m_fileCategory->m_children->indexOf(m_noResultsItem);
        if (row >= 0) {
            beginRemoveRows(index(0, 0), row, row);
            m_fileCategory->m_children->removeAt(row);
            m_noResultsItem->deleteLater();
            m_noResultsItem = nullptr;
            endRemoveRows();
        }
    }
}

void SearchFileItemModel::clearNoResultsDisplay(SearchFileItem* parentitem)
{
    auto removePlaceholder = [this](SearchFileItem *category, SearchFileItem *&placeholder) {
        if (!category || !placeholder) {
            return;
        }

        const int row = category->m_children->indexOf(placeholder);
        if (row >= 0) {
            beginRemoveRows(indexFromItem(category), row, row);
            category->m_children->removeAt(row);
            endRemoveRows();
        }
        placeholder->deleteLater();
        placeholder = nullptr;
    };


    if (parentitem == m_fileCategory) {
        removePlaceholder(m_fileCategory, m_noResultsItem);
        return;
    }

    if (parentitem == m_contentCategory) {
        removePlaceholder(m_contentCategory, m_noContentResultsItem);
        removePlaceholder(m_contentCategory, m_DefaultContentItem);
        m_hasDefaultContent = false;
    }
}

void SearchFileItemModel::updateNoContentResultsDisplay()
{
    // 检查文件分类是否有真实子项
    if(m_hasDefaultContent){
        return;
    }
    bool hasRealResults = false;

    for (int i = 0; i < m_contentCategory->childCount(); ++i) {
        SearchFileItem *child = m_contentCategory->m_children->at(i);
        if (child != m_noContentResultsItem && child->m_info) {
            hasRealResults = true;
            break;
        }
    }
    // 如果没有结果且没有虚拟项，添加虚拟项
    if (!hasRealResults && m_noContentResultsItem == nullptr) {
        beginInsertRows(index(1, 0), 0, 0);
        m_noContentResultsItem = new SearchFileItem(nullptr, m_contentCategory, this, this);
        m_contentCategory->appendChild(m_noContentResultsItem);
        endInsertRows();
    }
    // 如果有结果但存在虚拟项，移除虚拟项
    else if (hasRealResults && m_noContentResultsItem != nullptr) {
        int row = m_contentCategory->m_children->indexOf(m_noContentResultsItem);
        if (row >= 0) {
            beginRemoveRows(indexFromItem(m_contentCategory), row, row);
            m_contentCategory->m_children->removeAt(row);
            m_noContentResultsItem->deleteLater();
            m_noContentResultsItem = nullptr;
            endRemoveRows();
        }
    }
}

void SearchFileItemModel::setSelecting(bool isSelecting)
{
    m_isSelecting = isSelecting;
}

QString SearchFileItemModel::getDefaultString()
{
    if(m_hasDefaultContent){
        QModelIndex index =  indexFromItem(m_DefaultContentItem);
        QString text = data(index, Qt::DisplayRole).toString();
        return text;
    }
    return "";
}

void SearchFileItemModel::onChanged(const QString &uri)
{
    m_waiting_update_queue.append(uri);
    if (!m_changeChildTimer->isActive()) {
        m_changeChildTimer->start(100);
    }
}

SearchFilterProxyModel::SearchFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {
    auto settings = Peony::GlobalSettings::getInstance();
    m_settings = settings;
    setDynamicSortFilter(false);
}


bool SearchFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    // 第一级项目总是显示且保持原始顺序
    if (!sourceParent.isValid()) {
        return true;
    }
//        // 第二级项目应用过滤
    if (!sourceParent.parent().isValid()) {
        auto sourcemodel =  static_cast<SearchFileItemModel*>(sourceModel());
        if(sourcemodel->hasDefaultContent()&& sourcemodel->isContentItem(index)){
            return true;
        }
        const QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        const QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent); // 修改日期
        const QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent); // 文件类型
        const QModelIndex index3 = sourceModel()->index(sourceRow, 3, sourceParent); // 文件大小

        QString displayName = sourceModel()->data(index0, Qt::DisplayRole).toString();
        qint64 modifiedDate = sourceModel()->data(index1, Qt::UserRole+4).toLongLong();
        QString fileType = sourceModel()->data(index2, Qt::UserRole+2).toString();
        qint64 fileSize = sourceModel()->data(index3, Qt::UserRole+3).toLongLong();
        if (!checkFileTypeFilter(fileType))
            return false;
        if (! checkFileModifyTimeFilter(modifiedDate))
            return false;
        bool isdir = fileType =="inode/directory";
        if (! checkFileSizeOrTypeFilter(fileSize,isdir) )
            return false;
        if (! checkFileNameFilter(displayName))
            return false;
        quintptr id = index.internalId();
        if(!needFileContent && sourcemodel->isContentItem(index)){
            return false;
        }
    }
    return true;
}

bool SearchFilterProxyModel::isIdmSpaceUri(const QString &uri)
{
    return uri.startsWith(QStringLiteral("idm:///"))
            || uri.contains(QStringLiteral("search_uris=idm:///"));
}

int SearchFilterProxyModel::effectiveFileTypeForIdmSpace(int storedType)
{
    if (storedType <= ALL_TYPE)
        return ALL_TYPE;
    if (storedType >= OTHERS)
        return storedType;
    return storedType + 1;
}

int SearchFilterProxyModel::effectiveFileType(int storedType, const QString &uri)
{
    if (!isIdmSpaceUri(uri))
        return storedType;
    return effectiveFileTypeForIdmSpace(storedType);
}

void SearchFilterProxyModel::updateconditions(QString conditionsobj){
    m_keywords.clear();
    m_conditions.clear();
    m_contains.clear();
    needFileContent = true;
    m_keywordsContain = true;
    QJsonDocument doc = QJsonDocument::fromJson(conditionsobj.toUtf8());
    QJsonObject jsonObj = doc.object();
    int count = jsonObj["conditions count"].toInt();
    for (int i = 0; i < count; i++) {
        auto conditionObj = jsonObj[QString::number(i)].toObject();
        int type = conditionObj["condition type"].toInt();
        bool contains = conditionObj["contains"].toBool();
        if(type == int(Type::FileName))
       {
            auto jsonArray = conditionObj["keywords"].toArray();
            QStringList keywords;
            for (auto value : jsonArray) {
                if(!value.toString().isEmpty())
                    keywords << value.toString();
            }
            m_keywords<<keywords;
            m_keywordsContain = contains;
        }
        else if(type == int(Type::ConditionMatchRule)){
             int filtertype =  conditionObj["type"].toInt();
             needFileContent = filtertype == 0?  true:false;
        }
        else{
            int filtertype =  conditionObj["type"].toInt();
            if (type == int(Type::FileType))
                filtertype = effectiveFileType(filtertype, m_uri);
            m_conditions.insert(type,filtertype);
            m_contains.insert(type,contains);
        }
    }
    invalidateFilter();
    Q_EMIT conditionsChange();
}


QStringList SearchFilterProxyModel::getFiltereUrilist(){
    urilist.clear();
   // 获取源模型根索引
   QModelIndex sourceRoot = mapToSource(QModelIndex());
   // 查找目标分类
   auto searchmodel =  qobject_cast<SearchFileItemModel *>(sourceModel());
   for (int i = 0; i < rowCount(sourceRoot); ++i) {
       QModelIndex catIndex = index(i, 0, sourceRoot);
       for (int j = 0; j < rowCount(catIndex); ++j) {
           QModelIndex childIndex = index(j, 0, catIndex);
           if(i == 1 && searchmodel->isDefaultContentItem(mapToSource(childIndex))){
               continue;
           }
           QString uri = data(childIndex, Qt::UserRole+1).toString();
           if (!uri.isEmpty()) {
               urilist << uri;
           }
       }
    }
   return urilist;
}

void SearchFilterProxyModel::update()
{
    invalidateFilter();
}

bool SearchFilterProxyModel::checkFileModifyTimeFilter(quint64 modifiedTime) const{
    int ConditionType =  int(Type::DateRange);
    int cur = m_conditions[ConditionType];
    bool contains = m_contains[ConditionType];
    if(!m_conditions.contains(ConditionType))
        return true;
    if(cur==ALL_TYPE){
        return contains;
    }
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto dateTime = QDateTime::fromMSecsSinceEpoch(time);
    //qDebug() << "cur QDateTime:" <<dateTime.toString(Qt::SystemLocaleShortDate);
    QDate date = dateTime.date();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    QDate md_date = QDateTime::fromMSecsSinceEpoch(modifiedTime).date();
    //qDebug() << "modify time:" <<QDateTime::fromMSecsSinceEpoch(modifiedTime *1000).toString(Qt::SystemLocaleShortDate);
    int md_year= md_date.year();
    int md_month = md_date.month();
    int md_day = md_date.day();
    switch(cur)
    {
    case TODAY:
    {
        if (year == md_year && month == md_month && day == md_day)
            return contains;
        break;
    }
    case YESTERDAY:
    {
        QDate yesterday = date.addDays(-1);
        if(yesterday == md_date)/* 判断给定日期是否是昨天 */
            return contains;
        break;
    }
    case THIS_WEEK:
    {
        QDate md_date(md_year, md_month, md_day);
        int week, md_week,week_year, md_week_year;
        week = date.weekNumber(&week_year);
        md_week = md_date.weekNumber(&md_week_year);
        if (week == md_week && week_year == md_week_year)
            return contains;
        break;
    }
    case LAST_WEEK:
    {
        QDate monDate = date.addDays(-date.dayOfWeek() + 1);
        if(monDate.addDays(-7) <= md_date &&  md_date <= monDate.addDays(-1))/* 判断给定日期是否在上周 */
            return contains;
        break;
    }
    case THIS_MONTH:
    {
        if (year == md_year && month == md_month)
            return contains;
        break;
    }
    case LAST_MONTH:
    {
        int lastMonth = (month == 1) ? 12 : month - 1;
        if(md_month == lastMonth){/*  判断给定日期是否在上个月 */
            if((month == 1 && md_year == (year - 1)) || (month !=1 && year == md_year)){
                return contains;
            }
        }
        break;

    }
    case THIS_YEAR:
    {
        if (year == md_year)
            return contains;
        break;
    }
    case LAST_YEAR:
    {
        int lastYear = year - 1;
        if(md_year == lastYear)/* 判断给定日期是否在去年 */
            return contains;
        break;
    }
    case BEFORE_LAST_YEAR:
    {
        int lastYear = year - 1;
        if(md_year < lastYear)/* 判断给定日期是否在去年以前 */
            return contains;
        break;
    }
    default:
        break;
    }
    return !contains;
}


bool SearchFilterProxyModel::checkFileSizeFilter(quint64 size) const{
    int ConditionType =  int(Type::FileSize);
    int cur = m_conditions[ConditionType];
    bool contains = m_contains[ConditionType];
    if(!m_conditions.contains(ConditionType))
        return true;
    if(cur==ALL_TYPE){
        return contains;
    }
    switch (cur)
    {
    case EMPTY: //[0K]
    {
        if (size == 0)
            return contains;
        break;
    }
    case TINY: //（0-16K]
    {
        if (size > 0 && size <= TINY_BASE)
            return contains;
        break;
    }
    case SMALL:  //（16k-1M]
    {
        if(size > TINY_BASE && size <= SMALL_BASE)
            return contains;
        break;
    }
    case MEDIUM: //(1M-128M]
    {
        if(size > SMALL_BASE && size <= MEDIUM_BASE)
            return contains;
        break;
    }
    case BIG:  //(128M-1G]
    {
        if(size > MEDIUM_BASE && size <= BIG_BASE)
            return contains;
        break;
    }
    case LARGE: //(1-4G]
    {
        if (size > BIG_BASE && size <= LARGE_BASE)
            return contains;
        break;
    }
    case GREAT: //>4G
    {
        if (size > LARGE_BASE)
            return contains;
        break;
    }
    default:
        break;
    }
    return !contains;
}

bool SearchFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // 一级节点按原顺序倒序排列
    Qt::SortOrder order = this->sortOrder();
    bool resort = order == Qt::AscendingOrder;
    bool flag = left.row() > right.row();
    if (!left.parent().isValid() && !right.parent().isValid()) {//固定一级节点排序 不让文件名匹配在内容匹配下面
           return resort ? !flag:flag;
    }
    SearchFileItemModel *sourceRootModel = static_cast<SearchFileItemModel*>(sourceModel());
    // 获取对应的SearchFileItemModel指针
    if (left.isValid() && right.isValid()) {
        // 获取当前排序的列和顺序
        int sortColumn = this->sortColumn();

        // 获取对应的数据角色
        int role ;
        switch (sortColumn) {
            case 0: // 名称列
                role = Qt::DisplayRole;
                break;
            case 1: // 修改日期列
                role = Qt::UserRole + 4;
                break;
            case 2: // 文件类型列
                role = Qt::UserRole + 2;
                break;
            case 3: // 文件大小列
                role = Qt::UserRole + 3;
                break;
            default:
                return QSortFilterProxyModel::lessThan(left, right);
        }
        QVariant leftData = sourceRootModel->data(left, role);
        QVariant rightData = sourceRootModel->data(right, role);
        // 根据数据类型进行比较
        if (role == Qt::UserRole + 4 || role == Qt::UserRole + 3) {
            // 处理数值类型（qint64）
            qint64 leftValue = leftData.toLongLong();
            qint64 rightValue = rightData.toLongLong();
            if(leftValue == rightValue)
                goto default_sort;
            bool result = (leftValue > rightValue);
            return result;
        } else if (role == Qt::UserRole + 2) {
            // 文件类型使用字符串比较
            QString leftDisplayType = leftData.toString();
            QString rightDisplayType= rightData.toString();

            if (leftDisplayType == rightDisplayType)
            {
                 goto default_sort;
            }
            if (m_use_default_name_sort_order) {
                bool lesser = true;
                if(startWithChinese(leftDisplayType) && ! startWithChinese(rightDisplayType)) {
                    // fix #103343
                    lesser = true;
                    if (sortOrder() == Qt::AscendingOrder)
                        return lesser;
                    return !lesser;
                }
                else if(! startWithChinese(leftDisplayType) && startWithChinese(rightDisplayType)) {
                    lesser = false;
                    if (sortOrder() == Qt::AscendingOrder)
                        return lesser;
                    return !lesser;
                }
                else
                    return comparer.compare(leftDisplayType, rightDisplayType) > 0;
            }
            return comparer.compare(leftDisplayType, rightDisplayType) > 0;

        }else if(role == Qt::DisplayRole){ //文件名
            goto default_sort;
        }/*else {
            // 其他字符串类型比较
            QString leftStr = leftData.toString();
            QString rightStr = rightData.toString();
            return QString::localeAwareCompare(leftStr, rightStr) < 0;
        }*/

default_sort:
            const QModelIndex index0 = sourceModel()->index(left.row(), 0, left.parent());
            const QModelIndex index1 = sourceModel()->index(right.row(), 0, right.parent());

            QString leftDisplayName = sourceRootModel->data(index0, Qt::DisplayRole).toString();
            QString rightDisplayName = sourceRootModel->data(index1, Qt::DisplayRole).toString();

            if (m_use_default_name_sort_order) {
                //fix chinese first sort wrong issue, link to bug#70836
                //fix bug#97408,change indicator meanings
                bool lesser = true;
                if(startWithChinese(leftDisplayName) && ! startWithChinese(rightDisplayName)) {
                    // fix #103343
                    lesser = true;
                    if (sortOrder() == Qt::AscendingOrder)
                        return lesser;
                    return !lesser;
                }
                else if(!startWithChinese(leftDisplayName) && startWithChinese(rightDisplayName)) {
                    lesser = false;
                    if (sortOrder() == Qt::AscendingOrder)
                        return lesser;
                    return !lesser;
                }
                else
                    return comparer.compare(leftDisplayName, rightDisplayName) > 0;
            }
            return comparer.compare(leftDisplayName, rightDisplayName) > 0;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}


void SearchFilterProxyModel::checkSortSettings()
{
    m_use_global_sort = m_settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool();
    if (m_use_global_sort) {
        //m_show_hidden = m_settings->isExist(SHOW_HIDDEN_PREFERENCE)? m_settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
        m_use_default_name_sort_order = m_settings->isExist(SORT_CHINESE_FIRST)? m_settings->getValue(SORT_CHINESE_FIRST).toBool(): false;
        m_folder_first = m_settings->isExist(SORT_FOLDER_FIRST)? m_settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
    } else {
        auto info = Peony::FileInfo::fromUri(m_uri);
        auto fileMetaInfo = Peony::FileMetaInfo::fromUri(m_uri);
        if (fileMetaInfo) {
            //m_show_hidden = fileMetaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).isValid()? fileMetaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).toBool(): false;
            m_use_default_name_sort_order = fileMetaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).isValid()? fileMetaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).toBool(): true;
            m_folder_first = fileMetaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).isValid()? fileMetaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).toBool(): true;
        } else {
            m_use_default_name_sort_order = true;
            m_folder_first = true;
        }
    }
}

void SearchFilterProxyModel::setCurrenturi(QString uri)
{
    m_uri = uri;
}

bool SearchFilterProxyModel::startWithChinese(const QString &displayName) const
{
    //NOTE: a newly created file might could not get display name soon.
    if (displayName.isEmpty()) {
        return false;
    }
    auto firstStrUnicode = displayName.at(0).unicode();
    return (firstStrUnicode <=0x9FA5 && firstStrUnicode >= 0x4E00);
}

bool SearchFilterProxyModel::checkFileSizeOrTypeFilter(quint64 size, bool isDir) const{
    int ConditionType =  int(Type::FileSize);
    int cur = m_conditions[ConditionType];
    bool contains = m_contains[ConditionType];
    if(!m_conditions.contains(ConditionType))
        return true;
    if(cur==ALL_TYPE){
        return contains;
    }
    if(isDir)
        return false;
   return checkFileSizeFilter(size);
}

bool SearchFilterProxyModel::checkFileNameFilter(const QString &displayName) const{
    if(m_keywords.isEmpty())
        return true;
    for (auto key:m_keywords) {
        if (displayName.contains(key, Qt::CaseInsensitive))
            return m_keywordsContain;
    }
    return false;
}

bool SearchFilterProxyModel::checkFileContentFilter(const QString &displayName) const{
    if(!needFileContent)
        return false;
    return true;
}


bool SearchFilterProxyModel::checkFileTypeFilter(QString type) const
{
    int ConditionType =  int(Type::FileType);
    int cur = m_conditions[ConditionType];
    bool contains = m_contains[ConditionType];
    if(!m_conditions.contains(ConditionType))
        return true;
    if(cur==ALL_TYPE){
        return contains;
    }
    switch (cur)
    {
    case FILE_FOLDER:
    {
        if (type == Folder_Type)
            return contains;
        break;
    }
    case PICTURE:
    {
        if (type.contains(Image_Type))
            return contains;
        break;
    }
    case VIDEO:
    {
        if (type.contains(Video_Type))
            return contains;
        break;
    }
    case TXT_FILE:
    {
        if (type.contains(Text_Type))
            return contains;
        break;
    }
    case WPS_FILE:
    {
        if (type.contains(Wps_Type))
            return contains;
        break;
    }
    case AUDIO:
    {
        if (type.contains(Audio_Type) || type.contains("application/x-smaf"))
            return contains;
        break;
    }
    case OTHERS:
    {
        //exclude classfied types, show the rest other types
        if (type != Folder_Type && ! type.contains(Image_Type) && ! type.contains(Video_Type)
                && ! type.contains(Text_Type) && !type.contains(Wps_Type) && ! type.contains(Audio_Type) && !type.contains("application/x-smaf"))
            return contains;
        break;
    }
    default:
        break;
    }
    return !contains;
}

bool SearchFilterProxyModel::hasChildren(const QModelIndex& parent) const {
    if (!parent.isValid()) {
        return sourceModel()->hasChildren(mapToSource(parent));
    }
    if (!parent.parent().isValid()) {
        return true; // 第一级项目总是显示为有子项
    }
    return QSortFilterProxyModel::hasChildren(parent);
}

void TreeviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.isValid()) return;

    // 检查是否是文件分类节点且没有子项
    QVariant spanRole = index.data(SearchFileItemModel::IsSpanRowRole);
    if (spanRole.isValid() && spanRole.toBool()) {
        paintSpanRow(painter, option, index);
        return;
    }
    auto clipedUris = Peony::ClipboardUtils::getInstance()->getCutFileUris();
    if (!clipedUris.isEmpty()){
        if (clipedUris.contains(index.data(Qt::UserRole).toString())) {
            painter->setOpacity(0.5);
        } else {
            painter->setOpacity(1.0);
        }
    } else {
        painter->setOpacity(1.0);
    }

    // 第一层级或非第0列使用默认绘制
    if (!index.parent().isValid() || index.column() != 0) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
     auto view = qobject_cast<SearchView *>(parent());

     auto proxmodel = static_cast<SearchFilterProxyModel*>(view->model());
     auto searchmodel =  qobject_cast<SearchFileItemModel *>(proxmodel->sourceModel());
     auto item = searchmodel->itemFromIndex(proxmodel->mapToSource(index));
     QStyleOptionViewItem opt = option;
     opt.textElideMode = Qt::ElideRight;
     initStyleOption(&opt, index);

    // 保存原始内容
    QString text = opt.text;
    QIcon icon = opt.icon;

    // 1. 先绘制默认背景和焦点状态（关键修改）
    const QWidget* widget = opt.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);
    auto info = (item == nullptr) ? nullptr : item->info();
    if (info != nullptr && !info->isEmptyInfo()) {
        auto colors = info->getColors();
        if (index.column() == 0 && colors.count() >0) {
            if (!view->isDragging() || !view->selectionModel()->selectedIndexes().contains(index)) {
                paintLabel(opt, view->getLabelAlignment(), colors, painter);
            }
        }
    }
    // 2. 准备绘制参数
    QFont defaultFont = opt.font;
    QFont boldFont = defaultFont;
    boldFont.setBold(true);

    QFontMetrics fmNormal(defaultFont);
    QFontMetrics fmBold(boldFont);

    int xPos = opt.rect.left();
    int yPos = opt.rect.top();
    int availableWidth = opt.rect.width();

    // 3. 绘制图标（如果有）
    if (!icon.isNull()) {
        const int iconLeftMargin = 2;
        const int iconSize = qMin(opt.decorationSize.width(), opt.rect.height() - 4);

        opt.rect.adjust(-18, 0, 0, 0);
        QRect iconRect(opt.rect.left() + iconLeftMargin,
                       opt.rect.top() + (opt.rect.height() - iconSize) / 2,
                       iconSize,
                       iconSize);

        icon.paint(painter, iconRect, Qt::AlignLeft | Qt::AlignVCenter);
        xPos = iconRect.right() + 4;
        availableWidth -= (xPos - opt.rect.left());
    }

    // 获取正确的文本颜色（根据选中状态）
    QColor textColor = opt.state & QStyle::State_Selected
                           ? opt.palette.highlightedText().color()
                           : opt.palette.text().color();

    // 4. 检查是否是HTML内容
    bool isHtml = Qt::mightBeRichText(text);

    // 如果没有关键字且不是HTML，直接绘制文本（带Elide）
    if (m_keyword.isEmpty() && !isHtml) {
        painter->save();
        painter->setPen(textColor);

        QString elidedText = fmNormal.elidedText(text, Qt::ElideRight, availableWidth);
        painter->drawText(QRect(xPos, yPos, availableWidth, opt.rect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          elidedText);
        painter->restore();
        return;
    }

    // 如果是HTML内容，使用文本文档绘制
    if (isHtml) {
        QTextDocument doc;
          doc.setDefaultFont(defaultFont);
          doc.setDocumentMargin(0);
          doc.setTextWidth(-1);

          QString processedText = text;

          // 选中时修改HTML源码中的颜色
          if (opt.state & QStyle::State_Selected) {
              QString highlightColor = opt.palette.highlightedText().color().name();
              // 替换所有颜色为高亮色
              processedText = processedText.replace("color:#3790FA", "color:" + highlightColor);
          }

          doc.setHtml(processedText);

          if (doc.idealWidth() > availableWidth) {
              QString plainText = doc.toPlainText();
              QString elidedPlain = fmNormal.elidedText(plainText, Qt::ElideRight, availableWidth);
              doc.setPlainText(elidedPlain);
          }

          qreal docHeight = doc.size().height();
          qreal yOffset = (opt.rect.height() - docHeight) / 2;
          yOffset = qMax(0.0, yOffset);

          painter->save();
          painter->translate(xPos-10, yPos + yOffset);
          QRect clip(0, 0, availableWidth, opt.rect.height());
          painter->setClipRect(clip);

          QAbstractTextDocumentLayout::PaintContext ctx;
          ctx.palette = opt.palette;
          if (opt.state & QStyle::State_Selected) {
              ctx.palette.setColor(QPalette::Text, opt.palette.highlightedText().color());
          }
          doc.documentLayout()->draw(painter, ctx);
          painter->restore();
          return;
    }

    // 5. 有关键字时，分段绘制（高亮 + Elide处理）
    QRegularExpression re("(" + QRegularExpression::escape(m_keyword) + ")",
                          QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    int lastPos = 0;
    bool isElided = false;

    while (it.hasNext() && !isElided) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart();
        int length = match.capturedLength();

        // 绘制匹配前的普通文本
        if (start > lastPos) {
            QString before = text.mid(lastPos, start - lastPos);
            int beforeWidth = fmNormal.horizontalAdvance(before);

            if (xPos + beforeWidth > opt.rect.right()) {
                QString remainingText = text.mid(lastPos);
                QString elidedText = fmNormal.elidedText(remainingText, Qt::ElideRight, opt.rect.right() - xPos);
                painter->setFont(defaultFont);
                painter->setPen(textColor);  // 使用正确的文本颜色
                painter->drawText(QRect(xPos, yPos, opt.rect.right() - xPos, opt.rect.height()),
                                  Qt::AlignLeft | Qt::AlignVCenter,
                                  elidedText);
                isElided = true;
                break;
            }

            painter->setFont(defaultFont);
            painter->setPen(textColor);  // 使用正确的文本颜色
            painter->drawText(QRect(xPos, yPos, beforeWidth, opt.rect.height()),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              before);
            xPos += beforeWidth;
        }

        // 绘制高亮关键字
        if (!isElided) {
            QString keyword = match.captured();
            int keywordWidth = fmBold.horizontalAdvance(keyword);

            if (xPos + keywordWidth > opt.rect.right()) {
                QString remainingText = text.mid(lastPos);
                QString elidedText = fmNormal.elidedText(remainingText, Qt::ElideRight, opt.rect.right() - xPos);
                painter->setFont(defaultFont);
                painter->setPen(textColor);  // 使用正确的文本颜色
                painter->drawText(QRect(xPos, yPos, opt.rect.right() - xPos, opt.rect.height()),
                                  Qt::AlignLeft | Qt::AlignVCenter,
                                  elidedText);
                isElided = true;
                break;
            }

            // 绘制高亮背景
            painter->fillRect(QRect(xPos, yPos + 2, keywordWidth, opt.rect.height() - 4),
                              QColor(0, 255, 0));  // 绿色背景

            // 绘制关键字文本
            painter->setFont(defaultFont);
            painter->setPen(textColor);  // 选中时用黑色，否则用默认颜色
            painter->drawText(QRect(xPos, yPos, keywordWidth, opt.rect.height()),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              keyword);
            xPos += keywordWidth;
            lastPos = start + length;
        }
    }

    // 6. 绘制剩余文本（如果未Elide）
    if (!isElided && lastPos < text.length()) {
        QString after = text.mid(lastPos);
        int afterWidth = fmNormal.horizontalAdvance(after);

        if (xPos + afterWidth > opt.rect.right()) {
            after = fmNormal.elidedText(after, Qt::ElideRight, opt.rect.right() - xPos);
        }

        painter->setFont(defaultFont);
        painter->setPen(textColor);  // 使用正确的文本颜色
        painter->drawText(QRect(xPos, yPos, opt.rect.right() - xPos, opt.rect.height()),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          after);
    }
    painter->restore();
}

QSize TreeviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QFontMetrics fm(opt.font);
    int height = qMax(40, fm.height());
    int width = fm.horizontalAdvance(opt.text);
    if (index.column() == 0 && !opt.icon.isNull()) {
        width += opt.decorationSize.width() + 8;
    }
    return QSize(width, height);
}

void TreeviewDelegate::paintSpanRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() != 0) {
        return;
    }

    QRect spanRect = option.rect;

    if (option.widget) {
        QAbstractItemView *view = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget));
        if (view) {
            int totalWidth = view->viewport()->width();
            spanRect.setWidth(totalWidth);
            spanRect.moveLeft(0);
        }
    }

    painter->setCompositionMode(QPainter::CompositionMode_Source);
    QColor bgColor = option.palette.color(QPalette::Background);
    painter->fillRect(spanRect, bgColor);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->save();
    QString displayText = index.data(Qt::DisplayRole).toString();

    // 确保文本有足够的边距
    QRect textRect = spanRect.adjusted(15, 0, -15, 0);
    painter->drawText(textRect, Qt::AlignCenter, displayText);

    painter->restore();

}
QWidget *TreeviewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    TextEdit *edit = new TextEdit(parent);
    edit->setAcceptRichText(false);
    //edit->setContextMenuPolicy(Qt::CustomContextMenu);
    edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    edit->setWordWrapMode(QTextOption::NoWrap);

    edit->blockSignals(true);
    auto displayString = index.data(Qt::DisplayRole).toString();
    auto displayName = index.data(Qt::UserRole + 1).toString();
    auto uri = index.data(Qt::UserRole).toString();
    auto suffix = displayName.remove(displayString);
    auto fsType = FileUtils::getFsTypeFromFile(uri);
    auto info = FileInfo::fromUri(uri);
    int32_t maxLength = 255;
    if (info->isDesktopFile()) {
        suffix = ".desktop";
    }
    if (FileUtils::isFuseFileSystem(uri)) {
        fsType = "fuse.kyfs";
    }
    if (fsType.contains("ext")) {
        maxLength = 255 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType == "ecryptfs") {
        maxLength = 143 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType == "udf") {
        maxLength = 254 - suffix.toLocal8Bit().length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType.contains("ntfs")) {
        edit->setLimitBytes(false);
        maxLength = 255 - suffix.length();
        edit->setMaxLengthLimit(maxLength);
    } else if (fsType.contains("fuse.kyfs")) {
        edit->setLimitBytes(false);
        QDBusInterface iface ("com.kylin.file.system.fuse","/com/kylin/file/system/fuse","com.kylin.file.system.fuse",QDBusConnection::systemBus());
        QDBusReply<int32_t> reply = iface.call("GetFilenameLength");
        if (reply.isValid()) {
            maxLength = reply.value();
        }
        maxLength = maxLength - suffix.length();
        edit->setMaxLengthLimit(maxLength);
    }
    edit->blockSignals(false);

    //    QTimer::singleShot(1, parent, [=]() {
    //        this->updateEditorGeometry(edit, option, index);
    //    });

    //    connect(edit, &TextEdit::textChanged, this, [=]() {
    //        updateEditorGeometry(edit, option, index);
    //    });

    connect(edit, &TextEdit::textChanged, this, [=]() {
        auto text = edit->toPlainText();
        //fix bug#220283, rename edit position wrong issue
        //short file name no need update to avoid position wrong
        edit->adjustText();
        updateEditorGeometry(edit, option, index);
    });

    connect(edit, &TextEdit::finishEditRequest, this, &TreeviewDelegate::slot_finishEdit);
    connect(edit, &QWidget::destroyed, this, [=]() {
        Q_EMIT isEditing(false);
    });
    // connect(edit, &TextEdit::focusLost, [edit]() {
    //     edit->finishEditRequest();
    // });
    return edit;
}

void TreeviewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    TextEdit *edit = qobject_cast<TextEdit *>(editor);
    if (!edit)
        return;

    Q_EMIT isEditing(true);
    edit->setText(index.data(Qt::DisplayRole).toString());
    auto cursor = edit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    bool isDir = FileUtils::getFileIsFolder(index.data(Qt::UserRole).toString());
    bool isDesktopFile = index.data(Qt::UserRole).toString().endsWith(".desktop");
    bool isSoftLink = FileUtils::getFileIsSymbolicLink(index.data(Qt::UserRole).toString());
    if (!isDesktopFile && !isSoftLink && !isDir && edit->toPlainText().contains(".") && !edit->toPlainText().startsWith(".")) {
        int n = 1;
        if(index.data(Qt::DisplayRole).toString().contains(".tar.")) //ex xxx.tar.gz xxx.tar.bz2
            n = 2;
        while(n){
            cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, 1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            --n;
        }
    }
    //qDebug()<<cursor.anchor();
    edit->setTextCursor(cursor);
}

void TreeviewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);

    TextEdit *edit = qobject_cast<TextEdit*>(editor);
    if (!edit || !edit->m_backgroundEdit)
        return;

    QRect editRect = option.rect;
    auto leftShift = option.decorationSize.width()/2;
    editRect.setLeft(editRect.left() + leftShift);
    editRect.setWidth(editRect.width());
    editor->setGeometry(editRect);
    edit->m_backgroundEdit->setGeometry(1, 2, edit->width() - 2, edit->height() - 4);
    QTimer::singleShot(0, edit, [=]() {
        qreal docHeight = edit->document()->size().height();
        int availableHeight = edit->height();
        int topMargin = (availableHeight - docHeight) / 2;
        topMargin = qMax(2, static_cast<int>(topMargin));
        edit->setMargins(1, topMargin, 1, 2);
    });
}

void TreeviewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    TextEdit *edit = qobject_cast<TextEdit*>(editor);
    if (!edit)
        return;

    auto text = edit->toPlainText();
    if (text.isEmpty())
        return;
    if (text == index.data(Qt::DisplayRole).toString())
        return;
    //process special name . or .. or only space
    if (text == "." || text == ".." || text.trimmed() == "")
        return;

    if (! index.isValid())
        return;

    auto view = qobject_cast<SearchView *>(parent());
    auto oldName = index.data(Qt::DisplayRole).toString();
    if (text == oldName)
    {
        //create new file, should select the file or folder
        auto flags = QItemSelectionModel::Select|QItemSelectionModel::Rows;
        view->selectionModel()->select(index, flags);
        view->setFocus();
        return;
    }

    if (view->getSelections().count() > 1) {
        auto fileOpMgr = FileOperationManager::getInstance();
        QStringList lists = view->getSelections();
        auto renameOp = new FileBatchRenameOperation(lists, text);

        connect(renameOp, &FileBatchRenameOperation::operationFinished, view, [=](){
                auto info = renameOp->getOperationInfo().get();
                auto uri = info->target();
                QTimer::singleShot(100, view, [=](){
                    view->setSelections(QStringList()<<uri);
                    //after rename will nor sort immediately, comment to fix bug#60482
                    //view->scrollToSelection(uri);
                    view->setFocus();
                });
            }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    } else {
        auto fileOpMgr = FileOperationManager::getInstance();
        QString olduri = index.data(Qt::UserRole).toString();
        auto renameOp = new FileRenameOperation(index.data(Qt::UserRole).toString(), text);
        connect(renameOp, &FileRenameOperation::operationFinished, view, [=](){
                auto info = renameOp->getOperationInfo().get();
                auto uri = info->target();
                Q_EMIT reNameFinish(olduri, uri,text);
                QTimer::singleShot(100, view, [=](){
                    view->setSelections(QStringList()<<uri);
                    //after rename will nor sort immediately, comment to fix bug#60482
                    //view->scrollToSelection(uri);
                    view->setFocus();
                });
            }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    }
}

void TreeviewDelegate::slot_finishEdit()
{
    auto edit = qobject_cast<QWidget *>(sender());
    commitData(edit);
    closeEditor(edit, QAbstractItemDelegate::SubmitModelCache);
    if(edit){
        delete edit;
        edit = nullptr;
    }
}
void TreeviewDelegate::paintLabel(QStyleOptionViewItem &opt, int aalignment, QList<QColor> colors, QPainter *painter) const
{
    //修改标记个数最多为3个，以及标记位置
    int xOffSet = 0;
    int yOffSet = 0;
    int labelSize = 12;
    const int MAX_LABEL_NUM = 3;
    switch(aalignment) {
    case SearchView::LabelAlignment::AlignVertical: {
        xOffSet = 5;
        int index = 0;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num = colors.count() - startIndex + 1;

        //set color label on center, fix bug#40609
        auto iconSize = opt.decorationSize;
        labelSize = iconSize.height()/3;
        if (labelSize > 10)
            labelSize = 10;
        if (labelSize <6)
            labelSize = 6;

        yOffSet = (opt.rect.height()-labelSize*num/2)/2;
        if(yOffSet < 2)
        {
            yOffSet = 2;
        }
        for (int i = startIndex; i < colors.count(); ++i, ++index) {
            auto color = colors.at(i);
            painter->save();
            //fix bug#147348
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->translate(0, opt.rect.topLeft().y());
            painter->translate(2, 0);
            painter->setPen(opt.palette.highlightedText().color());
            painter->setBrush(color);
            painter->drawEllipse(QRectF(xOffSet, yOffSet, labelSize, labelSize));
            painter->restore();
            yOffSet += labelSize/2;
        }
        break;
    }
    case SearchView::LabelAlignment::AlignHorizontal:{
        QRect rect = opt.rect;
        xOffSet = rect.topRight().x() - labelSize/2 - 20;
        yOffSet = rect.height()/2 - labelSize/2;
        int startIndex = (colors.count() > MAX_LABEL_NUM ? colors.count() - MAX_LABEL_NUM : 0);
        int num =  colors.count() - startIndex;
        int width = rect.width();
        if(num > 0){
//            //bug#94242 修改标记位置后和名称重叠，设置标记位置的背景颜色
//            QRect markRect = rect;
//            markRect.setLeft(rect.width() - (num+1)*labelSize/2 );
//            bool isHover = (opt.state & QStyle::State_MouseOver) && (opt.state & ~QStyle::State_Selected);
//            bool isSelected = opt.state & QStyle::State_Selected;
//            bool enable = opt.state & QStyle::State_Enabled;
//            QColor color = opt.palette.color(enable? QPalette::Active: QPalette::Disabled,
//                                             QPalette::Highlight);

//            if (isSelected) {
//                color.setAlpha(255);
//            } else if (isHover) {
//                color = opt.palette.color(QPalette::Active, QPalette::BrightText);
//                color.setAlphaF(0.05);
//            } else {
//                color.setAlpha(0);
//            }

//            painter->save();
//            painter->fillRect(markRect, color);
//            painter->restore();
            width = width - (num+1)*labelSize/2 - 20;
        }
        for (int i = startIndex; i < colors.count(); ++i) {
            auto color = colors.at(i);
            painter->save();
            //fix bug#147348
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->translate(0, rect.topLeft().y());
            painter->translate(2, 2);
            painter->setPen(opt.palette.highlightedText().color());
            painter->setBrush(color);
            painter->drawEllipse(QRectF(xOffSet, yOffSet, labelSize, labelSize));
            painter->restore();
            xOffSet -= labelSize/2;
        }
        //bug#94242 修改标记位置后和名称重叠，设置汉字宽度
        opt.rect.setWidth(width);
        break;
    }
    }
}





TextEdit::TextEdit(QWidget *parent) : QTextEdit (parent)
{
    // fix #164278, icon view text editor doesn't cover view item.
    // note on ukui platform theme, style panel frame is not visible.
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    setViewportMargins(1, 2, 1, 2);
    if (m_backgroundEdit == nullptr) {
        m_backgroundEdit = new QTextEdit(this);
        m_backgroundEdit->setReadOnly(true);
        m_backgroundEdit->setFrameShape(QTextEdit::NoFrame);

        m_backgroundEdit->setGeometry(1, 2, size().width() - 2, size().height() - 4);
        m_backgroundEdit->lower();
    }
}

void TextEdit::adjustText()
{
    if (m_max_length_limit) {
        //fix #154584
        blockSignals(true);
        auto position = textCursor().position();
        while (true) {
            if (m_limit_bytes) {
                auto local8Bit = toPlainText().toLocal8Bit();
                if (local8Bit.length() <= m_max_length_limit) {
                    break;
                }
            } else {
                if (toPlainText().length() <= m_max_length_limit) {
                    break;
                }
            }
            if (position > 0) {
                position--;
                textCursor().beginEditBlock();
                textCursor().setPosition(position);
                textCursor().deletePreviousChar();
                textCursor().endEditBlock();
            } else {
                break;
            }
        }
        blockSignals(false);
    }
}

void TextEdit::setMaxLengthLimit(int length)
{
    m_max_length_limit = length;
}

void TextEdit::setLimitBytes(bool limitBytes)
{
    m_limit_bytes = limitBytes;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        Q_EMIT finishEditRequest();
        return;
    }
    return QTextEdit::keyPressEvent(e);
}

void TextEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);
    Q_EMIT focusLost(); // 发出我们自己的信号
}

void TextEdit::setMargins(int left, int top, int right, int bottom)
{
    setViewportMargins(left, top, right, bottom);
}

bool isFileContentOn()
{
    bool flage = false;
    const QByteArray id("org.ukui.search.settings");
    if (QGSettings::isSchemaInstalled(id)) {
        auto settings = new QGSettings(id, QByteArray(), nullptr);
        QStringList keys = settings->keys();
        if (keys.contains("contentIndexEnable")) {
            flage = settings->get("contentIndexEnable").toBool();
        }
        delete settings;
    }
    return flage;
}

FileManagerThread::FileManagerThread():QThread(nullptr)
{
    connect(this, &FileManagerThread::setParamForBatchQueryInfos, this, &FileManagerThread::batchQueryFileInfos);
}

FileManagerThread::~FileManagerThread()
{

}

void FileManagerThread::batchQueryFileInfos(Peony::FileInfosJob *infosJob, SearchFileItem *parentItem,bool isfinish,bool isupdate)
{
    auto retFileInfos = infosJob->batchQuerySync();
    if(infosJob){
        infosJob->deleteLater();
    }
    Q_EMIT finishQueryFileInfos(retFileInfos, parentItem,isfinish,isupdate);

}

void FileManagerThread::run()
{
    exec();
}
