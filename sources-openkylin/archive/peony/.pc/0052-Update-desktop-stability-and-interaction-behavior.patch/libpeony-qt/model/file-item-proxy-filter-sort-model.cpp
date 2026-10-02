/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "file-item-model.h"
#include "file-item.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-meta-info.h"
#include "file-label-model.h"
#include "manager-control.h"

#include "file-utils.h"
#include "file-operation-utils.h"

#include "global-settings.h"
#include "idm-dbus-manager.h"

#include <QDebug>
#include <QMessageBox>
#include <QDate>

#include <QLocale>
#include <QCollator>
#include <QStandardPaths>

#include <QRegularExpression>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>

using namespace Peony;

QLocale locale = QLocale(QLocale::system().name());
QCollator comparer = QCollator(locale);

const QString getModelDirectoryUri(FileItemProxyFilterSortModel *model)
{
    FileItemModel *srcModel = qobject_cast<FileItemModel *>(model->sourceModel());
    if (!srcModel) {
        qInfo()<<"source model not avaliable now";
        return nullptr;
    }
    return srcModel->getRootUri();
}

void setModelMetaInfo(FileItemProxyFilterSortModel *model, const QString &key, const QVariant &value)
{
    auto uri = getModelDirectoryUri(model);
    if (uri.isEmpty()) {
        qCritical()<<"set model meta info failed, can not get model directory uri";
    } else {
        auto fileMetaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(model));
        if (fileMetaInfo) {
            fileMetaInfo->setMetaInfoVariant(key, value);
        } else {
            qCritical()<<"set model meta info failed, can not get model meta info.";
        }
    }
}

FileItemProxyFilterSortModel::FileItemProxyFilterSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(false);
    //enable number sort, like 100 is after 99
    comparer.setNumericMode(true);
    auto settings = GlobalSettings::getInstance();
    m_settings = settings;

    m_sortTimer = new QTimer(this);
    m_sortTimer->setSingleShot(true);
    connect(m_sortTimer, &QTimer::timeout, this, &FileItemProxyFilterSortModel::handleSortTypeChanged);

    m_show_hidden = settings->isExist(SHOW_HIDDEN_PREFERENCE)? settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (SHOW_HIDDEN_PREFERENCE == key) {
            m_show_hidden= GlobalSettings::getInstance()->getValue(key).toBool();
            invalidateFilter();
        }
    });

    m_cachedHiddenFilePath = "";
    m_cachedHiddenEntries.clear();

    //fix bug#174512, set hidden file should hidden in time
    connect(GlobalSettings::getInstance(), &GlobalSettings::updateHiddenFile, this, [=] (const QString& fileName) {
        qDebug() << "updateHiddenFile:"<<fileName;
        // 清除缓存
        m_cachedHiddenFilePath = "";
        m_cachedHiddenEntries.clear();
        QTimer::singleShot(100, this, [=](){
            update();
        });
    });

    //黑白名单更新
    // QDBusConnection conn = QDBusConnection::sessionBus();
    // if (! conn.isConnected()) {
    //     qCritical()<<"failed to init mDbusPeonyServer, can not connect to session dbus";
    //     return;
    // }

    // mDbusPeonyServer = new QDBusInterface("org.ukui.peony",
    //                                   "/org/ukui/peony",
    //                                   "org.ukui.peony",
    //                                   QDBusConnection::sessionBus());

    // if (! mDbusPeonyServer->isValid()){
    //     qCritical() << "Create /org/ukui/peony Interface Failed " << QDBusConnection::systemBus().lastError();
    //     return;
    // }

    //同步黑白名单数据
    syncBlackAndWhiteData();

    //接收到black_and_white_update信号则进行更新
    // QDBusConnection::sessionBus().connect("org.ukui.peony",
    //                                       "/org/ukui/peony",
    //                                       "org.ukui.peony",
    //                                       "black_and_white_update",
    //                                       this,
    //                                       SLOT(updateBlackAndWhiteList()));
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString& key){
        if (key == APP_WHITE_LIST || key == APP_BLACK_LIST) {
            updateBlackAndWhiteList();
        }
    });
}

void FileItemProxyFilterSortModel::setSourceModel(QAbstractItemModel *model)
{
    if (sourceModel())
        disconnect(sourceModel());
    QSortFilterProxyModel::setSourceModel(model);
    //FileItemModel *file_item_model = static_cast<FileItemModel*>(model);

    auto fileInfo = FileInfo::fromUri(getModelDirectoryUri(this));
    if (!fileInfo.get()->isEmptyInfo()) {
        auto job = new FileInfoJob(fileInfo);
        job->setAutoDelete();
        connect(job, &FileInfoJob::queryAsyncFinished, this, [=]{
            checkSortSettings();
            update();
        });
        job->queryAsync();
    } else {
        checkSortSettings();
    }

    connect(m_settings, &GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == USE_GLOBAL_DEFAULT_SORTING) {
            setUseGlobalSort(m_settings->getValue(key).toBool());
        }
    });

    //connect(file_item_model, &FileItemModel::updated, this, &FileItemProxyFilterSortModel::update);
    // connect(file_item_model, &FileItemModel::updateProxySort, this, [=](){
    //     qDebug() << __func__ << __LINE__ << m_sortType << m_sortOrder;
    //     this->invalidate();
    //     this->sort(m_sortType, m_sortOrder);
    // });
}

FileItem *FileItemProxyFilterSortModel::itemFromIndex(const QModelIndex &proxyIndex)
{
    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    QModelIndex index = mapToSource(proxyIndex);
    return model->itemFromIndex(index);
}

QModelIndex FileItemProxyFilterSortModel::getSourceIndex(const QModelIndex &proxyIndex)
{
    return mapToSource(proxyIndex);
}

const QModelIndex FileItemProxyFilterSortModel::indexFromUri(const QString &uri)
{
    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    const QModelIndex sourceIndex = model->indexFromUri(uri);
    return mapFromSource(sourceIndex);
}

bool FileItemProxyFilterSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    //comment these improve code to fix disorder issue
    //and chinese first or folder first has not effect issue
    //Fix me with better solutions
//    if (left.data().isNull())
//        return sortOrder() == Qt::AscendingOrder? false: true;

//    if (right.data().isNull())
//        return sortOrder() == Qt::AscendingOrder? true: false;

//    if (left.column() != 0 || right.column() != 0) {
//        return true;
//    }

    //qDebug()<<left<<right;
    if (left.isValid() && right.isValid()) {
        FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
        auto leftItem = model->itemFromIndex(left);
        auto rightItem = model->itemFromIndex(right);
        if (!(leftItem->hasChildren() && rightItem->hasChildren())) {
            //make folder always has a higher order
            if ((leftItem->hasChildren() || rightItem->hasChildren()) && m_folder_first) {
                bool lesser = leftItem->hasChildren();
                if (sortOrder() == Qt::AscendingOrder)
                    return lesser;
                return !lesser;
            }
        }/*else if (sortColumn() != FileItemModel::ModifiedDate){
            goto default_sort;
        }*/

        //fix bug#97408,change indicator meanings
        //箭头向上为升序，向下为降序，与通常的理解对应
        //将所有排序对比跟之前的方式反过来，小于改为大于，返回true的改为false
        switch (sortRole()) {
        case FileItemModel::FileMostUsedRole: {
            quint64 leftUsedCount = leftItem->m_info->property("opened-count").toInt();
            quint64 rightUsedCount = rightItem->m_info->property("opened-count").toInt();
            if (leftUsedCount != rightUsedCount) {
                return leftUsedCount > rightUsedCount;
            }
            quint64 leftAtime = leftItem->m_info->accessTime();
            quint64 rightAtime = rightItem->m_info->accessTime();
            if (leftAtime == rightAtime) {
                goto default_sort;
            }
            return leftAtime > rightAtime;
        }
        case FileItemModel::FileAddModeRole: {
            QString leftUri = leftItem->m_info->uri();
            QString rightUri = rightItem->m_info->uri();
            if (leftUri.startsWith("idm:///") && rightUri.startsWith("idm:///")
                    && leftUri != "idm:///" && rightUri != "idm:///") {
                QString spaceName = leftUri.remove("idm://").split('/', QString::SkipEmptyParts).first();
                spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
                QString leftPath = FileUtils::urlDecode(leftItem->m_info->targetUri());
                QString rightPath = FileUtils::urlDecode(rightItem->m_info->targetUri());
                if (leftPath.isEmpty() || rightPath.isEmpty()) {
                    goto default_sort;
                }

                leftPath = leftPath.remove("file://");
                rightPath = rightPath.remove("file://");
                bool leftIsUserAdded = IDMDBusManager::getInstance()->isUserAddFile(spaceName, leftPath);
                bool rightIsUserAdded = IDMDBusManager::getInstance()->isUserAddFile(spaceName, rightPath);
                bool lesser = rightIsUserAdded && !leftIsUserAdded;
                bool greater = leftIsUserAdded && !rightIsUserAdded;
                bool same = !lesser && !greater;

                if (same) {
                    goto default_sort;
                } else if (lesser) {
                    return !m_userAddedFirst;
                } else {
                    return m_userAddedFirst;
                }
            } else {
                goto default_sort;
            }
        }
        default:
            break;
        }

        switch (sortColumn()) {
        case FileItemModel::FileName: {
            goto default_sort;
        }
        case FileItemModel::FileSize: {
//            qDebug() << "FileSize:" <<leftItem->m_info->displayName()<<leftItem->m_info->size()
//                     <<rightItem->m_info->displayName()<<rightItem->m_info->size();
            //fix refresh sort order change issue, use file name to compare, link to bug#92525
            if (leftItem->m_info->size() == rightItem->m_info->size())
            {
                goto default_sort;
            }
            return leftItem->m_info->size() > rightItem->m_info->size();
        }
        case FileItemModel::FileType: {
            //fix refresh sort order change issue, use file name to compare, link to bug#92525
            if (leftItem->m_info->fileType() == rightItem->m_info->fileType())
            {
                 goto default_sort;
            }

            //使用文件类型的显示名字排序，也遵循中文优先和文件夹优先规则，关联bug#328899
            //排序条件下的值相等时，使用名称进行排序，保持排序稳定，刷新后不改变, 关联bug#92525
            QString leftDisplayType = leftItem->m_info->fileType();
            QString rightDisplayType= rightItem->m_info->fileType();
            if (m_use_default_name_sort_order) {
                //fix chinese first sort wrong issue, link to bug#70836
                //fix bug#97408,change indicator meanings
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
        }
        case FileItemModel::ModifiedDate: {
            //delete time sort in trash, fix bug#63093
            if (leftItem->uri().startsWith("trash://"))
            {
                if (leftItem->m_info->deletionDate() == rightItem->m_info->deletionDate())
                {
                    goto default_sort;
                }
                return leftItem->m_info->deletionDate() > rightItem->m_info->deletionDate();
            }

            if (Peony::GlobalSettings::getInstance()->getShowCreateTime()) {
                if (leftItem->m_info->createTime() == rightItem->m_info->createTime()) {
                    goto default_sort;
                }
                return leftItem->m_info->createTime() > rightItem->m_info->createTime();
            }

            //non trash files, use modifiedTime sort
            if (leftItem->m_info->modifiedTime() == rightItem->m_info->modifiedTime())
            {
                goto default_sort;
            }
            return leftItem->m_info->modifiedTime() > rightItem->m_info->modifiedTime();
        }
        case FileItemModel::FilePath: {
            QString leftString,rightString;
            if(leftItem->uri().startsWith("trash://")){
                leftString= leftItem->m_info->property("orig-path").toString();
                rightString = rightItem->m_info->property("orig-path").toString();
            }
            if(leftItem->uri().startsWith("search:///")){
                leftString = leftItem->m_info->filePath();
                rightString = rightItem->m_info->filePath();
            }
            if (leftString == rightString) {
                goto default_sort;
            }
            return comparer.compare(leftString, rightString) > 0;
        }
        default:
            break;
        }

default_sort:
        //function was deprecated
//        if (FileOperationUtils::leftNameIsDuplicatedFileOfRightName(leftItem->m_info->displayName(), rightItem->m_info->displayName())) {
//            return FileOperationUtils::leftNameLesserThanRightName(leftItem->info()->displayName(), rightItem->info()->displayName());
//        }

        //when other sort value is the same, use sort by name way，link to bug#92525
        //排序条件下的值相等时，使用名称进行排序，保持排序稳定，刷新后不改变, 关联bug#92525
        QString leftDisplayName = leftItem->m_info->displayName();
        QString rightDisplayName = rightItem->m_info->displayName();
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
            else if(! startWithChinese(leftDisplayName) && startWithChinese(rightDisplayName)) {
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

bool FileItemProxyFilterSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    //FIXME:
    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    //root
    auto childIndex = model->index(sourceRow, 0, sourceParent);
    if (childIndex.isValid()) {
        auto item = static_cast<FileItem*>(childIndex.internalPointer());
        FileInfo* fileInfo =  item->m_info.get();/*FileInfo::fromUri(item->uri()).get()*/;

        if (!m_show_hidden && isFileHidden(fileInfo)) {
            return false;
        }

        if(!item->shouldShow())
            return false;

        //special Volumn of 839 M upgrade part not show process
        auto targetUri = FileUtils::getTargetUri(item->uri());
        if (targetUri == "")
            targetUri = item->uri();
        if (targetUri.startsWith("file:///media/") && targetUri.endsWith("/2691-6AB8"))
            return false;

        //not show system orgin lost+found and sec_storage_data folder in DATA, fix bug#67084
        /* not show system orgin lost+found and bbox_logs folder in file:///data, fix bug#104820、bug#105714 */
        if(targetUri.endsWith("data/bbox_logs",Qt::CaseInsensitive))
            return false;
        if (targetUri.endsWith("data/lost+found",Qt::CaseInsensitive) || targetUri.endsWith("data/sec_storage_data",Qt::CaseInsensitive))
        {
            if (! item->m_info->canWrite() && ! item->m_info->canExecute())
               return false;
        }

        //FIXME use display name to hide 839 MB disk
        if (item->m_info->displayName().contains("839 MB"))
            return false;

        //check the file info filter conditions
        //qDebug()<<"start filter conditions check"<<item->m_info->displayName()<<item->m_info->type();
        if (! checkFileTypeFilter(item->m_info->type()))
            return false;
        if (! checkFileModifyTimeFilter(item->m_info->modifiedTime()))
            return false;
        if (! checkFileSizeOrTypeFilter(item->m_info->size(), item->m_info->isDir()))
            return false;

        //fix bug162927, desktop file should consider file name
        if (item->uri().endsWith(".desktop")){
            QString originName = item->uri().split("/").last();
            QString AppName = item->m_info->displayName();
            //对性能有影响，暂时屏蔽，后续再考虑优化
//           if (! item->info()->canExecute())
//               AppName = FileUtils::getApplicationName(item->uri());
           if (! checkFileNameFilter(AppName) && ! checkFileNameFilter(originName))
               return false;
        }
        else if (! checkFileNameFilter(item->m_info->displayName()))
            return false;

        if (!checkFileContentFilter(item->m_info->displayName())) {
            return false;
        }

        // 处理添加模式中仅手动添加和仅自动添加
        if (item->m_info->uri().startsWith("idm:///") && item->m_info->uri() != "idm:///") {
            QString path = FileUtils::urlDecode(item->m_info->targetUri());
            path = path.remove("file://");
            if (!checkFileAddModeFilter(item->m_info->uri(), path)) {
                return false;
            }
        }

        //check the file label filter conditions
        if (m_label_name != "" || m_label_color != Qt::transparent)
        {
            QString uri = item->m_info->uri();
            if (m_label_name != "")
            {
                auto names = FileLabelModel::getGlobalModel()->getFileLabels(uri);
                if (! names.contains(m_label_name))
                    return false;
            }

            if (m_label_color != Qt::transparent)
            {
                auto colors = FileLabelModel::getGlobalModel()->getFileColors(uri);
                if (! colors.contains(m_label_color))
                    return false;
            }
        }

        //check multiple label filter conditions, file has any one of these label is accepted
        if(m_show_label_names.size() > 0 || m_show_label_colors.size() > 0)
        {
            bool bfind = false;
            QString uri = item->m_info->uri();
            if (m_show_label_names.size() > 0)
            {
                auto names = FileLabelModel::getGlobalModel()->getFileLabels(uri);
                for(auto temp : m_show_label_names)
                {
                    if(names.contains(temp))
                    {
                        bfind = true;
                        break;
                    }
                }
            }

            if (! bfind && m_show_label_colors.size() > 0)
            {
                auto colors = FileLabelModel::getGlobalModel()->getFileColors(uri);
                for(auto temp : m_show_label_colors)
                {
                    if (colors.contains(temp))
                    {
                        bfind = true;
                        break;
                    }
                }
            }

            if (! bfind)
                return false;
        }

        //check the blur name, can use as search color labels
        if (m_blur_name != "")
        {
            QString uri = item->m_info->uri();
            auto names = FileLabelModel::getGlobalModel()->getFileLabels(uri);
            bool find = false;
            for(auto temp : names)
            {
                if ((m_case_sensitive && temp.indexOf(m_blur_name) >= 0) ||
                        (! m_case_sensitive && temp.toLower().indexOf(m_blur_name.toLower()) >= 0))
                {
                    find = true;
                    break;
                }
            }
            if (! find)
                return false;
        }

        if (!m_mimeTypeFilters.isEmpty()) {
            if (!m_mimeTypeFilters.contains(fileInfo->fileType())) {
                return false;
            }
        }

        if (!m_nameFilters.isEmpty()) {
            if (!fileInfo->isDir()) {
                bool contains = false;
                for (auto nameFilter : m_nameFilters) {

                    QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(nameFilter), this->filterCaseSensitivity()? QRegularExpression::NoPatternOption: QRegularExpression::CaseInsensitiveOption);
                    QRegularExpressionMatch match = rx.match(fileInfo->displayName());
                    if (match.hasMatch()) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    return false;
                }
            }
        }

        QDir::Filters dirFilters = QDir::Filters(m_dirFilters);
        if (m_dirFilters != -1) {
            bool showFiles = dirFilters & QDir::Files;
            bool showDirs = dirFilters & QDir::Dirs || dirFilters & QDir::AllDirs;
            if (!showFiles && !fileInfo->isDir()) {
                return false;
            }
            if (!showDirs && fileInfo->isDir()) {
                return false;
            }
        }

        //黑白名单处理
        if (item->m_info->isDesktopFile() && nullptr != item->m_info->desktopName()){
            if (m_bw_list_model != BW_LIST_NORMAL){
                bool exist = m_bwListInfo.contains(item->m_info->desktopName());
                if (m_bw_list_model == BW_LIST_BLACK){
                   return ! exist;
                }else if (m_bw_list_model == BW_LIST_WHITE){
                   return exist;
                }
            }
        }
    }

    return true;
}

bool FileItemProxyFilterSortModel::checkFileNameFilter(const QString &displayName) const
{
    if (m_file_name_list.size() == 0)
        return true;

    for(auto key:m_file_name_list)
    {
        //Filter criteria are not case sensitive. fix bug#92478
        if (displayName.contains(key, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

bool FileItemProxyFilterSortModel::checkFileContentFilter(const QString &displayName) const
{
    if (m_fileContent.isEmpty())
        return true;

    if (displayName.contains(m_fileContent, Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

bool FileItemProxyFilterSortModel::checkFileAddModeFilter(const QString &uri, const QString &path) const
{
    if (m_show_file_add_mode_list.size() == 0 || m_show_file_add_mode_list.contains(ALL_MODE)) {
        return true;
    }
    if (uri.startsWith("idm:///") && uri != "idm:///") {
        QString tmpUri = uri;
        QString spaceName = tmpUri.remove("idm:///").split('/', QString::SkipEmptyParts).first();
        spaceName =  QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
        int count = m_show_file_add_mode_list.count();
        for(int i = 0; i < count; i++)
        {
            auto cur = m_show_file_add_mode_list[i];
            switch (cur)
            {
            case USER_ADDED:
            {
                if (IDMDBusManager::getInstance()->isUserAddFile(spaceName, path))
                    return true;
                break;
            }
            case AUTO_ADDED:
            {
                if (!IDMDBusManager::getInstance()->isUserAddFile(spaceName, path))
                    return true;
                break;
            }
            default:
                break;
            }
        }
    }
    return false;
}

void FileItemProxyFilterSortModel::checkSortSettings()
{
    m_use_global_sort = m_settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool();
    if (m_use_global_sort) {
        m_show_hidden = m_settings->isExist(SHOW_HIDDEN_PREFERENCE)? m_settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool(): false;
        m_use_default_name_sort_order = m_settings->isExist(SORT_CHINESE_FIRST)? m_settings->getValue(SORT_CHINESE_FIRST).toBool(): false;
        m_folder_first = m_settings->isExist(SORT_FOLDER_FIRST)? m_settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
    } else {
        auto info = FileInfo::fromUri(getModelDirectoryUri(this));
        auto fileMetaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
        if (fileMetaInfo && !info->isEmptyInfo()) {
            m_show_hidden = fileMetaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).isValid()? fileMetaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).toBool(): false;
            m_use_default_name_sort_order = fileMetaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).isValid()? fileMetaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).toBool(): true;
            m_folder_first = fileMetaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).isValid()? fileMetaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).toBool(): true;
        } else {
            qCritical()<<"could not get metainfo"<<getModelDirectoryUri(this);
            m_show_hidden = false;
            m_use_default_name_sort_order = true;
            m_folder_first = true;
        }
    }
}

void FileItemProxyFilterSortModel::syncBlackAndWhiteData()
{
    QStringList whiteList = m_settings->getValue(APP_WHITE_LIST).toStringList();
    QStringList blackList = m_settings->getValue(APP_BLACK_LIST).toStringList();

    const bool whiteEnabled = !whiteList.isEmpty();
    const bool blackEnabled = !blackList.isEmpty();

    if (whiteEnabled) {
        m_bw_list_model = BW_LIST_WHITE;
        m_bwListInfo = whiteList;
    } else if (blackEnabled) {
        m_bw_list_model = BW_LIST_BLACK;
        m_bwListInfo = blackList;
    } else {
        m_bw_list_model = BW_LIST_NORMAL;
    }
    qDebug() << "syncBlackAndWhiteData:" << "1.mode: "<< m_bw_list_model << "2.list" << m_bwListInfo;
}

void FileItemProxyFilterSortModel::updateBlackAndWhiteList()
{
    syncBlackAndWhiteData();
    update();
}

void FileItemProxyFilterSortModel::setSelectionModeHint(QAbstractItemView::SelectionMode mode)
{
    setProperty("selectionMode", int(mode));
    Q_EMIT this->setSelectionModeChanged();
}

void FileItemProxyFilterSortModel::handleSortTypeChanged()
{
    checkSortSettings();
    qDebug()<<"sort type:"<<m_sortType<<" sort order:"<<m_sortOrder<<" folder first:"<<m_folder_first;
    if (m_sortType < 6) {
        this->clearFileAddModeFilter();
        QSortFilterProxyModel::setSortRole(-1); // 恢复默认角色
        QSortFilterProxyModel::sort(m_sortType, m_sortOrder);
        this->update();
    } else if (m_sortType == FileItemModel::FileMostUsed + 1) {
        // 处理智能排序
        this->clearFileAddModeFilter();
        QSortFilterProxyModel::setSortRole(FileItemModel::FileMostUsedRole);
        QSortFilterProxyModel::invalidate();
        QSortFilterProxyModel::sort(0, m_sortOrder);
        this->update();
    } else if (m_sortType == FileItemModel::FileAddMode + 1 || m_sortType == FileItemModel::FileAddMode + 2) {
        // 处理手动添加排序优先
        if (m_sortType == FileItemModel::FileAddMode + 1) {
            m_userAddedFirst = true;
        } else {
            m_userAddedFirst = false;
        }

        this->clearFileAddModeFilter();
        QSortFilterProxyModel::setSortRole(FileItemModel::FileAddModeRole);
        QSortFilterProxyModel::invalidate();
        QSortFilterProxyModel::sort(0, m_sortOrder);
        this->update();
    } else if (m_sortType == FileItemModel::FileAddMode + 3) {
        QList<int> mode;
        mode.append(USER_ADDED);
        this->setFileAddModeFilter(mode);
    } else if (m_sortType == FileItemModel::FileAddMode + 4) {
        QList<int> mode;
        mode.append(AUTO_ADDED);
        this->setFileAddModeFilter(mode);
    }

    bool sort = Peony::GlobalSettings::getInstance()->getValue(SORT_UPDATE).toBool();
    Peony::GlobalSettings::getInstance()->setValue(SORT_UPDATE, !sort);

    Q_EMIT this->sortFinished();
}

QVariant FileItemProxyFilterSortModel::getDirectorySettings(const QString &key)
{
    auto metaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
    if (metaInfo) {
        return metaInfo->getMetaInfoVariant(key);
    } else {
        qCritical()<<"can not get meta info"<<getModelDirectoryUri(this);
    }
    return QVariant();
}

void FileItemProxyFilterSortModel::setDirectorySettings(const QString &key, const QVariant &value)
{
    auto metaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
    if (metaInfo) {
        return metaInfo->setMetaInfoVariant(key, value);
    } else {
        qCritical()<<"can not set meta info"<<getModelDirectoryUri(this);
    }
}

void FileItemProxyFilterSortModel::calculationFileTypeCount(QString type) const
{
    int fileType = ALL_TYPE;
    if (type == Folder_Type) {
        fileType = FILE_FOLDER;
    } else if (type.contains(Image_Type)) {
        fileType = PICTURE;
    } else if (type.contains(Video_Type)) {
        fileType = VIDEO;
    } else if (type.contains(Text_Type)) {
        fileType = TXT_FILE;
    } else if (type.contains(Wps_Type)) {
        fileType = WPS_FILE;
    } else if (type.contains(Audio_Type) || type.contains("application/x-smaf")) {
        fileType = AUDIO;
    } else if (type != Folder_Type && ! type.contains(Image_Type) && ! type.contains(Video_Type)
               && ! type.contains(Text_Type) && !type.contains(Wps_Type) && ! type.contains(Audio_Type) && !type.contains("application/x-smaf")) {
        fileType = OTHERS;
    }

    m_file_type_map[fileType]++;
}

void FileItemProxyFilterSortModel::calculationFileModifyTimeCount(quint64 modifiedTime) const
{
    auto time = QDateTime::currentMSecsSinceEpoch();
    auto dateTime = QDateTime::fromMSecsSinceEpoch(time);
    QDate date = dateTime.date();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    QDate md_date = QDateTime::fromMSecsSinceEpoch(modifiedTime * 1000).date();
    int md_year = md_date.year();
    int md_month = md_date.month();
    int md_day = md_date.day();

    // 计算本周的起止时间
    QDate weekStart = date.addDays(-date.dayOfWeek() + 1);  // 本周一
    QDate weekEnd = weekStart.addDays(6);  // 本周日

    // 先判断是否是今年
    if (year == md_year) {
        // 再判断是否是本月
        if (month == md_month) {
            // 最后判断是否是今天或昨天
            if (day == md_day) {
                // 今天
                m_file_modify_time_map[TODAY]++;
                m_file_modify_time_map[THIS_WEEK]++;
                m_file_modify_time_map[THIS_MONTH]++;
                m_file_modify_time_map[THIS_YEAR]++;
            }
            else if (day == md_day + 1) {
                // 昨天(同年同月)
                m_file_modify_time_map[YESTERDAY]++;
                // 判断是否属于本周
                if (md_date >= weekStart && md_date <= weekEnd) {
                    m_file_modify_time_map[THIS_WEEK]++;
                }
                m_file_modify_time_map[THIS_MONTH]++;
                m_file_modify_time_map[THIS_YEAR]++;
            }
            else {
                // 本月其他日期
                if (md_date >= weekStart && md_date <= weekEnd) {
                    m_file_modify_time_map[THIS_WEEK]++;
                } else if (md_date >= weekStart.addDays(-7) && md_date < weekStart) {
                    m_file_modify_time_map[LAST_WEEK]++;
                }
                m_file_modify_time_map[THIS_MONTH]++;
                m_file_modify_time_map[THIS_YEAR]++;
            }
        }
        else if (month == md_month + 1) {
            // 上月(同年)
            // 只有月初几天可能属于上周
            if (md_date >= weekStart.addDays(-7) && md_date < weekStart) {
                m_file_modify_time_map[LAST_WEEK]++;
            }
            m_file_modify_time_map[LAST_MONTH]++;
            m_file_modify_time_map[THIS_YEAR]++;
        }
        else {
            // 今年更早的月份，相差超过1个月
            m_file_modify_time_map[THIS_YEAR]++;
        }
    }
    else if (year == md_year + 1) {
        // 去年
        if (month == 1 && md_month == 12 && day == 1 && md_day == 31) {
            // 跨年的昨天
            m_file_modify_time_map[YESTERDAY]++;
            if (md_date >= weekStart && md_date <= weekEnd) {
                m_file_modify_time_map[THIS_WEEK]++;
            }
            m_file_modify_time_map[LAST_MONTH]++;
        }
        else {
            // 只有年初几天可能属于上周(如果跨年时正好是同一周)
            if (md_date >= weekStart.addDays(-7) && md_date < weekStart) {
                m_file_modify_time_map[LAST_WEEK]++;
            }
            // 如果是上个月的文件(跨年)
            if (month == 1 && md_month == 12) {
                m_file_modify_time_map[LAST_MONTH]++;
            }
        }
        m_file_modify_time_map[LAST_YEAR]++;
    }

    m_file_modify_time_map[ALL_TIME]++;
}

void FileItemProxyFilterSortModel::calculationFileSizeCount(quint64 size) const
{
    int fileSize = ALL_SIZE;
    if (size == 0) {
        fileSize = EMPTY;
    } else if (size > 0 && size <= TINY_BASE) {
        fileSize = TINY;
    } else if (size > TINY_BASE && size <= SMALL_BASE) {
        fileSize = SMALL;
    } else if (size > SMALL_BASE && size <= MEDIUM_BASE) {
        fileSize = MEDIUM;
    } else if (size > MEDIUM_BASE && size <= BIG_BASE) {
        fileSize = BIG;
    } else if (size > BIG_BASE && size <= LARGE_BASE) {
        fileSize = LARGE;
    } else if (size > LARGE_BASE) {
        fileSize = GREAT;
    }

    m_file_size_map[fileSize]++;
}

void FileItemProxyFilterSortModel::calculationFileLabelCount(QStringList names, QList<QColor> colors) const
{
    QList<FileLabelItem *> allLabels = FileLabelModel::getGlobalModel()->getAllFileLabelItems();
    int size = allLabels.size();
    for (int i = 0; i < size; i++) {
        if (names.contains(allLabels.at(i)->name()) && colors.contains(allLabels.at(i)->color())) {
            m_file_label_map[i]++;
        }
    }
}

void FileItemProxyFilterSortModel::calculationAll(FileItem *item) const
{
    calculationFileTypeCount(item->m_info->type());
    calculationFileModifyTimeCount(item->m_info->modifiedTime());
    calculationFileSizeCount(item->m_info->size());
    calculationFileLabelCount(FileLabelModel::getGlobalModel()->getFileLabels(item->m_info->uri()), FileLabelModel::getGlobalModel()->getFileColors(item->m_info->uri()));
}

bool FileItemProxyFilterSortModel::checkFileTypeFilter(QString type) const
{
    //qDebug()<<"m_show_file_type: "<<m_show_file_type<<" "<<type;
    //multiple condition, advance search and default search
    if (m_show_file_type == ALL_FILE && m_file_type_list.count() == 0
        || m_file_type_list.contains(ALL_FILE))
        return true;

    //support multiple file type choose
    QList<int> totalTypeList;
    if (m_file_type_list.count() > 0)
        totalTypeList.append(m_file_type_list);
    if (! totalTypeList.contains(m_show_file_type) && m_show_file_type != ALL_FILE)
        totalTypeList.append(m_show_file_type);

    int count = totalTypeList.count();
    for(int i = 0; i < count; i++)
    {
        auto cur = totalTypeList[i];
        switch (cur)
        {
        case FILE_FOLDER:
        {
            if (type == Folder_Type)
                return true;
            break;
        }
        case PICTURE:
        {
            if (type.contains(Image_Type))
                return true;
            break;
        }
        case VIDEO:
        {
            if (type.contains(Video_Type))
                return true;
            break;
        }
        case TXT_FILE:
        {
            if (type.contains(Text_Type))
                return true;
            break;
        }
        case WPS_FILE:
        {
            if (type.contains(Wps_Type))
                return true;
            break;
        }
        case AUDIO:
        {
            if (type.contains(Audio_Type) || type.contains("application/x-smaf"))
                return true;
            break;
        }
        case OTHERS:
        {
            //exclude classfied types, show the rest other types
            if (type != Folder_Type && ! type.contains(Image_Type) && ! type.contains(Video_Type)
                    && ! type.contains(Text_Type) && !type.contains(Wps_Type) && ! type.contains(Audio_Type) && !type.contains("application/x-smaf"))
                return true;
            break;
        }
        default:
            break;
        }
    }

    return false;
}

bool FileItemProxyFilterSortModel::checkFileModifyTimeFilter(quint64 modifiedTime) const
{
    //qDebug()<<"checkFileModifyTimeFilter";
    //have no date filter
    if (m_modify_time_list.count() == 0 || m_modify_time_list.contains(ALL_FILE))
        return true;

    auto time = QDateTime::currentMSecsSinceEpoch();
    auto dateTime = QDateTime::fromMSecsSinceEpoch(time);
    //qDebug() << "cur QDateTime:" <<dateTime.toString(Qt::SystemLocaleShortDate);
    QDate date = dateTime.date();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    QDate md_date =  QDateTime::fromMSecsSinceEpoch(modifiedTime * 1000).date();
    //qDebug() << "modify time:" <<QDateTime::fromMSecsSinceEpoch(modifiedTime *1000).toString(Qt::SystemLocaleShortDate);
    int md_year= md_date.year();
    int md_month = md_date.month();
    int md_day = md_date.day();

    int count = m_modify_time_list.size();
    for(int i = 0; i < count; i++)
    {
        auto cur = m_modify_time_list[i];
        switch(cur)
        {
        case TODAY:
        {
            if (year == md_year && month == md_month && day == md_day)
                return true;
            break;
        }
        case YESTERDAY:
        {
            QDate yesterday = date.addDays(-1);
            if(yesterday == md_date)/* 判断给定日期是否是昨天 */
                return true;
            break;
        }
        case THIS_WEEK:
        {
            QDate md_date(md_year, md_month, md_day);
            int week, md_week,week_year, md_week_year;
            week = date.weekNumber(&week_year);
            md_week = md_date.weekNumber(&md_week_year);
            if (week == md_week && week_year == md_week_year)
                return true;
            break;
        }
        case LAST_WEEK:
        {
            QDate monDate = date.addDays(-date.dayOfWeek() + 1);
            if(monDate.addDays(-7) <= md_date &&  md_date <= monDate.addDays(-1))/* 判断给定日期是否在上周 */
                return true;
            break;
        }
        case THIS_MONTH:
        {
            if (year == md_year && month == md_month)
                return true;
            break;
        }
        case LAST_MONTH:
        {
            int lastMonth = (month == 1) ? 12 : month - 1;
            if(md_month == lastMonth){/*  判断给定日期是否在上个月 */
                if((month == 1 && md_year == (year - 1)) || (month !=1 && year == md_year)){
                    return true;
                }
            }
            break;

        }
        case THIS_YEAR:
        {
            if (year == md_year)
                return true;
            break;
        }
        case LAST_YEAR:
        {
            int lastYear = year - 1;
            if(md_year == lastYear)/* 判断给定日期是否在去年 */
                return true;
            break;
        }
        case BEFORE_LAST_TEAR:
        {
            int lastYear = year - 1;
            if(md_year < lastYear)/* 判断给定日期是否在去年以前 */
                return true;
            break;
        }
        default:
            break;
        }
    }

    return false;
}

bool FileItemProxyFilterSortModel::checkFileSizeFilter(quint64 size) const
{
    //qDebug()<<"checkFileSizeFilter: "<<m_show_file_size<<" "<<size;
    if (m_file_size_list.count() == 0 || m_file_size_list.contains(ALL_FILE))
        return true;

    int count = m_file_size_list.count();
    for(int i = 0; i < count; i++)
    {
        auto cur = m_file_size_list[i];
        switch (cur)
        {
        case EMPTY: //[0K]
        {           
            if (size == 0)
                return true;
            break;
        }
        case TINY: //（0-16K]
        {          
            if (size > 0 && size <= TINY_BASE)
                return true;
            break;
        }
        case SMALL:  //（16k-1M]
        {
            if(size > TINY_BASE && size <= SMALL_BASE)
                return true;
            break;
        }
        case MEDIUM: //(1M-128M]
        {
            if(size > SMALL_BASE && size <= MEDIUM_BASE)
                return true;
            break;
        }
        case BIG:  //(128M-1G]
        {
            if(size > MEDIUM_BASE && size <= BIG_BASE)
                return true;
            break;
        }
        case LARGE: //(1-4G]
        {
            if (size > BIG_BASE && size <= LARGE_BASE)
                return true;
            break;
        }
        case GREAT: //>4G
        {
            if (size > LARGE_BASE)
                return true;
            break;
        }
        default:
            break;
        }

    }
    return false;
}

bool FileItemProxyFilterSortModel::checkFileSizeOrTypeFilter(quint64 size, bool isDir) const
{
    if (m_file_size_list.count() == 0 || m_file_size_list.contains(ALL_FILE))
        return true;
    else if(isDir)
        return false;

   return checkFileSizeFilter(size);
}

void FileItemProxyFilterSortModel::update()
{
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setUseGlobalSort(bool use)
{
    m_use_global_sort = use;
    checkSortSettings();
    update();
}

void FileItemProxyFilterSortModel::setShowHidden(bool showHidden)
{
    if (!MANAGER_CONTROL->checkShowHiddenPermission()) {
        return;
    }

    if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        GlobalSettings::getInstance()->setGSettingValue(SHOW_HIDDEN_PREFERENCE, showHidden);
    } else {
        setModelMetaInfo(this, SHOW_HIDDEN_PREFERENCE, showHidden);
        QString desktopFile = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        if (getModelDirectoryUri(this) ==  desktopFile && GlobalSettings::getInstance()->isExist(SHOW_DESKTOP_HIDDEN_PREFERENCE)) {
            GlobalSettings::getInstance()->setGSettingValue(SHOW_DESKTOP_HIDDEN_PREFERENCE, showHidden);
        }
    }

    m_show_hidden = showHidden;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setUseDefaultNameSortOrder(bool use)
{
    if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        GlobalSettings::getInstance()->setValue(SORT_CHINESE_FIRST, use);
    } else {
        setModelMetaInfo(this, SORT_CHINESE_FIRST, use);
    }

    m_use_default_name_sort_order = use;
    beginResetModel();
    sort(sortColumn()>0? sortColumn(): 0, sortOrder()==Qt::DescendingOrder? Qt::DescendingOrder: Qt::AscendingOrder);
    endResetModel();
}

void FileItemProxyFilterSortModel::setFolderFirst(bool folderFirst)
{
    if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        GlobalSettings::getInstance()->setValue(SORT_FOLDER_FIRST, folderFirst);
    } else {
        setModelMetaInfo(this, SORT_FOLDER_FIRST, folderFirst);
    }

    m_folder_first = folderFirst;
    beginResetModel();
    sort(sortColumn()>0? sortColumn(): 0, sortOrder()==Qt::DescendingOrder? Qt::DescendingOrder: Qt::AscendingOrder);
    endResetModel();
}

void FileItemProxyFilterSortModel::addFileNameFilter(QString key, bool updateNow)
{
    m_file_name_list.append(key);
    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::addFilterCondition(int option, int classify, bool updateNow)
{
    switch (option) {
    case 0:
        if (! m_file_type_list.contains(classify))
            m_file_type_list.append(classify);

        break;
    case 1:
        if (! m_file_size_list.contains(classify))
            m_file_size_list.append(classify);
        break;
    case 2:
        if (! m_modify_time_list.contains(classify))
            m_modify_time_list.append(classify);
        break;
    default:
        break;
    }

    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::removeFilterCondition(int option, int classify, bool updateNow)
{
    switch (option) {
    case 1:
        if (! m_file_type_list.contains(classify))
            m_file_type_list.removeOne(classify);

        break;
    case 2:
        if (! m_modify_time_list.contains(classify))
            m_modify_time_list.removeOne(classify);
        break;
    case 3:
        if (! m_file_size_list.contains(classify))
            m_file_size_list.removeOne(classify);
        break;
    default:
        break;
    }

    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::clearConditions()
{
    m_file_name_list.clear();
    m_file_type_list.clear();
    m_file_size_list.clear();
    m_modify_time_list.clear();

    m_mimeTypeFilters.clear();
    m_nameFilters.clear();
    m_dirFilters = -1;

    m_show_label_names.clear();
    m_show_label_colors.clear();
    m_file_type_map.clear();
    m_file_modify_time_map.clear();
    m_file_size_map.clear();
    m_file_label_map.clear();
}

void FileItemProxyFilterSortModel::addFileContentFilter(QString key, bool updateNow)
{
    qDebug() << __func__ << key;
    m_fileContent = key;
    if (updateNow)
        invalidateFilter();
}

void FileItemProxyFilterSortModel::clearFileContentConditions()
{
    m_fileContent.clear();
}

void FileItemProxyFilterSortModel::clearFileAddModeFilter()
{
    m_show_file_add_mode_list.clear();
}

void FileItemProxyFilterSortModel::setFilterConditions(int fileType, int modifyTime, int fileSize)
{
    m_show_file_type = fileType;
    m_show_file_size = fileSize;
    m_show_modify_time = modifyTime;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setFilterConditions(const QStringList &mimeTypeFilters, const QStringList &nameFilters,  QDir::Filters dirFilters, Qt::CaseSensitivity caseSensitivity)
{
    m_mimeTypeFilters = mimeTypeFilters;
    m_nameFilters = nameFilters;
    m_dirFilters = dirFilters;
    setFilterCaseSensitivity(caseSensitivity);
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setFilterLabelConditions(QString name, QColor color)
{
    m_label_name = name;
    m_label_color = color;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setMutipleLabelConditions(QStringList names, QList<QColor> colors)
{
    m_show_label_names.clear();
    m_show_label_colors.clear();

    for(auto name : names)
    {
        m_show_label_names.append(name);
    }
    for(auto color : colors)
    {
        m_show_label_colors.append(color);
    }
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setLabelBlurName(QString blurName, bool caseSensitive)
{
    m_blur_name = blurName;
    m_case_sensitive = caseSensitive;
    invalidateFilter();
}

void FileItemProxyFilterSortModel::setFileAddModeFilter(QList<int> addModes)
{
    m_show_file_add_mode_list.clear();
    for (auto mode : addModes) {
        m_show_file_add_mode_list.append(mode);
    }
    invalidateFilter();
}

bool FileItemProxyFilterSortModel::startWithChinese(const QString &displayName) const
{
    //NOTE: a newly created file might could not get display name soon.
    if (displayName.isEmpty()) {
        return false;
    }
    auto firstStrUnicode = displayName.at(0).unicode();
    return (firstStrUnicode <=0x9FA5 && firstStrUnicode >= 0x4E00);
}

QModelIndexList FileItemProxyFilterSortModel::getAllFileIndexes()
{
    //FIXME: how about the tree?
    QModelIndexList l;
    int i = 0;
    while (this->index(i, 0, QModelIndex()).isValid()) {
        auto index = this->index(i, 0, QModelIndex());
        if (m_show_hidden) {
            l<<index;
        } else {
            auto disyplayName = index.data(Qt::DisplayRole).toString();
            if (disyplayName.isEmpty()) {
                auto uri = this->index(i, 0, QModelIndex()).data(FileItemModel::UriRole).toString();
                disyplayName = FileUtils::getFileDisplayName(uri);
            }
            if (!disyplayName.startsWith(".")) {
                l<<index;
            }
        }

        i++;
    }
    return l;
}

QAbstractItemView::SelectionMode FileItemProxyFilterSortModel::getSelectionModeHint()
{
    if (property("selectionMode").isValid()) {
        return QAbstractItemView::SelectionMode(property("selectionMode").toInt());
    }
    return QAbstractItemView::NoSelection;
}

void FileItemProxyFilterSortModel::sort(int column, Qt::SortOrder order)
{
    m_sortType = column;
    m_sortOrder = order;
    if(!m_sortTimer->isActive()){
        m_sortTimer->start(50);
    }
}

int FileItemProxyFilterSortModel::expectedSortType()
{
    return m_sortType;
}

Qt::SortOrder FileItemProxyFilterSortModel::expectedSortOrder()
{
    return m_sortOrder;
}

void FileItemProxyFilterSortModel::manualUpdateExpectedSortInfo(int sortType, Qt::SortOrder order)
{
    m_sortType = sortType;
    m_sortOrder = order;
}

void FileItemProxyFilterSortModel::clearAllMapsCount()
{
    m_file_type_map.clear();
    m_file_modify_time_map.clear();
    m_file_size_map.clear();
    m_file_label_map.clear();
}

QMap<int, int> FileItemProxyFilterSortModel::getFileTypeCount()
{
    return m_file_type_map;
}

QMap<int, int> FileItemProxyFilterSortModel::getFileModifyTimeCount()
{
    return m_file_modify_time_map;
}

QMap<int, int> FileItemProxyFilterSortModel::getFileSizeCount()
{
    return m_file_size_map;
}

QMap<int, int> FileItemProxyFilterSortModel::getFileLabelCount()
{
    return m_file_label_map;
}

void FileItemProxyFilterSortModel::checkSettingsAndSort()
{
    checkSortSettings();
    invalidateFilter();

    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        m_sortType = settings->isExist(SORT_COLUMN)? settings->getValue(SORT_COLUMN).toInt(): 0;
        m_sortOrder = Qt::SortOrder(settings->isExist(SORT_ORDER)? settings->getValue(SORT_ORDER).toInt(): 1);
    } else {
        auto info = FileInfo::fromUri(getModelDirectoryUri(this));
        auto fileMetaInfo = FileMetaInfo::fromUri(getModelDirectoryUri(this));
        if (fileMetaInfo && !info->isEmptyInfo()) {
            m_sortType = fileMetaInfo->getMetaInfoVariant(SORT_COLUMN).isValid()? fileMetaInfo->getMetaInfoInt(SORT_COLUMN): 0;
            m_sortOrder = Qt::SortOrder(fileMetaInfo->getMetaInfoVariant(SORT_ORDER).isValid()? fileMetaInfo->getMetaInfoInt(SORT_ORDER): 1);
        }
    }
    QSortFilterProxyModel::sort(m_sortType, m_sortOrder);
}


QStringList FileItemProxyFilterSortModel::getAllFileUris()
{
    QStringList l;
    auto indexes = getAllFileIndexes();
    for (auto index : indexes) {
        if (index.column() == 0)
            l<<index.data(FileItemModel::UriRole).toString();
    }
    return l;
}

void FileItemProxyFilterSortModel::recalculateAllCounts()
{
    m_file_type_map.clear();
    m_file_modify_time_map.clear();
    m_file_size_map.clear();
    m_file_label_map.clear();

    FileItemModel *model = static_cast<FileItemModel*>(sourceModel());
    if (!model) return;

    for (int i = 0; i < model->rowCount(QModelIndex()); i++) {
        QModelIndex srcIndex = model->index(i, 0, QModelIndex());

        if (filterAcceptsRow(srcIndex.row(), srcIndex.parent())) {
            auto item = model->itemFromIndex(srcIndex);
            if (item) {
                calculationFileTypeCount(item->m_info->type());
                calculationFileModifyTimeCount(item->m_info->modifiedTime());
                calculationFileSizeCount(item->m_info->size());
                calculationFileLabelCount(
                    FileLabelModel::getGlobalModel()->getFileLabels(item->m_info->uri()),
                    FileLabelModel::getGlobalModel()->getFileColors(item->m_info->uri())
                );
            }
        }
    }
}

bool FileItemProxyFilterSortModel::isFileHidden(FileInfo* fileInfo) const
{
    if (!fileInfo) {
        return false;
    }

    if (fileInfo->isHiddenFile()) {
        return true;
    }

    QString fileUri = fileInfo->uri();
    QString parentUri;
    
    if (!m_cachedParentUri.isEmpty()) {
        if (fileUri.startsWith(m_cachedParentUri + "/")) {
            parentUri = m_cachedParentUri;
        } else {
            parentUri = FileUtils::getParentUri(fileUri);
            m_cachedParentUri = parentUri;
        }
    } else {
        parentUri = FileUtils::getParentUri(fileUri);
        m_cachedParentUri = parentUri;
    }

    QString hiddenFilePath = QUrl(parentUri).path() + "/.hidden";

    bool needRefreshCache = false;

    if (hiddenFilePath != m_cachedHiddenFilePath) {
        needRefreshCache = true;
    } else if (m_cachedHiddenFileExists) {
        QFileInfo fileInfo(hiddenFilePath);
        if (fileInfo.lastModified() != m_cachedHiddenFileTimestamp) {
            needRefreshCache = true;
        }
    }

    if (needRefreshCache) {
        m_cachedHiddenFilePath = hiddenFilePath;
        m_cachedHiddenFileExists = QFile::exists(hiddenFilePath);
        
        if (!m_cachedHiddenFileExists) {
            m_cachedHiddenEntries.clear();
            return false;
        }

        m_cachedHiddenEntries.clear();
        QFile hiddenFile(hiddenFilePath);
        if (hiddenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&hiddenFile);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty()) {
                    m_cachedHiddenEntries << line;
                }
            }
            hiddenFile.close();

            QFileInfo fileInfo(hiddenFilePath);
            m_cachedHiddenFileTimestamp = fileInfo.lastModified();
        }
    }

    if (!m_cachedHiddenFileExists) {
        return false;
    }

    if (m_cachedHiddenEntries.isEmpty()) {
        return false;
    }

    return m_cachedHiddenEntries.contains(fileInfo->displayName());
}

void FileItemProxyFilterSortModel::setMultiSelectMode(bool enabled)
{
    if (m_multiSelectMode == enabled)
        return;

    m_multiSelectMode = enabled;

    // 用 dataChanged 刷新复选框，避免 layoutChanged 导致绘制时索引失效崩溃
    const int rows = rowCount();
    const int cols = columnCount();
    if (rows > 0 && cols > 0) {
        Q_EMIT dataChanged(index(0, 0), index(rows - 1, cols - 1));
    }
}

QVariant FileItemProxyFilterSortModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        if (!m_multiSelectMode || !index.isValid() || index.model() != this) {
            return QVariant();
        }
        QItemSelectionModel *selectionModel = m_selectionModel.data();
        if (!selectionModel || selectionModel->model() != this) {
            return QVariant();
        }
        return selectionModel->isSelected(index) ? Qt::Checked : Qt::Unchecked;
    }

    return QSortFilterProxyModel::data(index, role);
}

Qt::ItemFlags FileItemProxyFilterSortModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QSortFilterProxyModel::flags(index);

    if (index.column() == 0 && m_multiSelectMode) {
        return defaultFlags | Qt::ItemIsUserCheckable;
    }

    return defaultFlags;
}

bool FileItemProxyFilterSortModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0 && index.isValid() && index.model() == this) {
        QItemSelectionModel *selectionModel = m_selectionModel.data();
        if (!selectionModel || selectionModel->model() != this) {
            return false;
        }
        const Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        if (state == Qt::Checked) {
            selectionModel->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        } else {
            selectionModel->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
        }
        return true;
    }

    return QSortFilterProxyModel::setData(index, value, role);
}

void FileItemProxyFilterSortModel::setSelectionModel(QItemSelectionModel *selectionModel)
{
    // 断开旧连接
    if (m_selectionModel) {
        disconnect(m_selectionModel, nullptr, this, nullptr);
    }

    m_selectionModel = selectionModel;

    // 选择变化时刷新复选框
    if (m_selectionModel) {
        connect(m_selectionModel, &QItemSelectionModel::selectionChanged,
                this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
            auto refreshIndexes = [this](const QItemSelection &sel) {
                for (const auto &range : sel) {
                    int top = qMin(range.top(), range.bottom());
                    int bottom = qMax(range.top(), range.bottom());
                    for (int row = top; row <= bottom; ++row) {
                        QModelIndex idx = index(row, 0, range.topLeft().parent());
                        if (idx.isValid()) {
                            Q_EMIT dataChanged(idx, idx, {Qt::CheckStateRole});
                        }
                    }
                }
            };
            refreshIndexes(deselected);
            refreshIndexes(selected);
        });
    }
}
