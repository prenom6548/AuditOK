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

#include "advanced-desktop-item-model.h"

#include "file-enumerator.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-info-manager.h"
#include "file-watcher.h"
#include "file-operation-manager.h"
#include "file-move-operation.h"
#include "file-trash-operation.h"
#include "file-copy-operation.h"
#include "file-operation-utils.h"

#include "thumbnail-manager.h"
#include "usershare-manager.h"

#include "file-meta-info.h"

#include "peony-desktop-application.h"
#include "global-settings.h"
#include "sound-effect.h"
#ifdef KY_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif
#include "desktop-menu-plugin-manager.h"
#include "emblem-provider.h"
#include "desktop-window-manager.h"
#include "tooltips-manager.h"
#include "file-label-model.h"

#include <QStandardPaths>
#include <QIcon>

#include <QMimeData>
#include <QUrl>

#include <QTimer>

#include <QMessageBox>

#include <QDebug>

#include "common.h"
using namespace Peony;
#ifdef KY_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

static bool initMetaInfo = false;

AdvancedDesktopIconItem::AdvancedDesktopIconItem(const QString &text)
    : QStandardItem(text)
{
    setData(text, UriRole);
    auto metaInfo = FileMetaInfo::fromUri(text);
    if (metaInfo) {
        QStringList strPos = metaInfo->getMetaInfoStringListV1(ITEM_GRID_POS_ATTRIBUTE);
        qDebug() << "[AdvancedDesktopIconItem::AdvancedDesktopIconItem]"<<strPos <<text;
        if (2 == strPos.count() && strPos[0] >= 0 && strPos[1] >= 0) {
            QPoint pos(strPos[0].toInt(), strPos[1].toInt());
            setData(pos, PositionRole);
            int id = metaInfo->getMetaInfoInt(SCREEN_ID);
            setData(id, ScreenIdRole);
        } else {
            int id = 0;
            QPoint currentPos;
            setData(id, ScreenIdRole);
            setData(currentPos, PositionRole);
        }
        QStringList value = metaInfo->getMetaInfoStringList(RESTORE_ITEM_GRID_POS_ATTRIBUTE);
        setData(value, ExceptedPositionRole);
        value = metaInfo->getMetaInfoStringList(RESTORE_EXTEND_ITEM_GRID_POS_ATTRIBUTE);
        setData(value, ExtendScreenPositionRole);
        value = metaInfo->getMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_GRID_POS_ATTRIBUTE);
        setData(value, SinglesSreenPositionRole);
    }
}


AdvancedDesktopIconItem::AdvancedDesktopIconItem(const QString &text, bool fileCreate)
    : QStandardItem(text)
{
    setData(text, UriRole);
    auto metaInfo = FileMetaInfo::fromUri(text);
    if (metaInfo) {
        qDebug() << "[AdvancedDesktopIconItem::AdvancedDesktopIconItem]"<<text;
        QStringList strPos = metaInfo->getMetaInfoStringListV1(ITEM_GRID_POS_ATTRIBUTE);
        int currentId = metaInfo->getMetaInfoInt(SCREEN_ID);
        bool isRenaming =  qApp->property("rename").toBool();
        auto info = FileInfo::fromUri(text);
        QStringList renamePoint = qApp->property("renamepoint").toStringList();
        bool existRenamePoint = (2 == renamePoint.count() && renamePoint[0].toInt() >= 0 && renamePoint[1].toInt() >= 0);
        bool meteInfoPoint = (2 == strPos.count() && strPos[0].toInt() >= 0 && strPos[1].toInt() >= 0);
        qDebug() << "[AdvancedDesktopIconItem::AdvancedDesktopIconItem]"<<strPos <<text;
        bool isRenamingBypeony  = qApp->property("isRenamingByPeony").toBool();
        if(isRenaming){
            isRenamingBypeony = false;
        }
        auto isbyorgtool =  qApp->property("renameingbyorg").toBool();
        if(isbyorgtool){
            int instanceid = metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID);
            auto partitoninstanceid = qApp->property("partitoninstanceid").toInt();
            if(instanceid != partitoninstanceid){
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID,partitoninstanceid);
            }
        }
        if(isRenamingBypeony){
            if(!meteInfoPoint){
                strPos = qApp->property("RenamingByPeonyPos").toStringList();
                meteInfoPoint = (2 == strPos.count() && strPos[0].toInt() >= 0 && strPos[1].toInt() >= 0);
                qApp->setProperty("RenamingByPeonyPos",QStringList());
            }
        }
        if ((isRenaming || isRenamingBypeony) && (existRenamePoint || meteInfoPoint)) {
            if(meteInfoPoint) {
                QPoint pos(strPos[0].toInt(), strPos[1].toInt());
                setData(pos, PositionRole);
            } else if(existRenamePoint) {
                QPoint pos(renamePoint[0].toInt(), renamePoint[1].toInt());
                setData(pos, PositionRole);
            }
            setData(currentId, ScreenIdRole);
            QStringList value = metaInfo->getMetaInfoStringList(RESTORE_ITEM_GRID_POS_ATTRIBUTE);
            setData(value, ExceptedPositionRole);
            value = metaInfo->getMetaInfoStringList(RESTORE_EXTEND_ITEM_GRID_POS_ATTRIBUTE);
            setData(value, ExtendScreenPositionRole);
            value = metaInfo->getMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_GRID_POS_ATTRIBUTE);
            setData(value, SinglesSreenPositionRole);
        } else {
            bool isPastetoOrg =  qApp->property("isPasteToOrgTool").toBool();
            if(isPastetoOrg){
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -999);//
            }
            int id = 0;
            QPoint currentPos = QPoint(-1, -1);
            if (info->property("createId").isValid() && info->property("createPos").isValid()) {
                id = info->property("createId").toInt();
                currentPos = info->property("createPos").toPoint();
            }
            setData(id, ScreenIdRole);
            setData(currentPos, PositionRole);
            QStringList value("");
            setData(value, ExceptedPositionRole);
            setData(value, ExtendScreenPositionRole);
            setData(value, SinglesSreenPositionRole);

            // 检测新建文件位置是否与插件相交，如果相交则执行图标挤占重排
            if (currentPos.x() >= 0 && currentPos.y() >= 0) {
                DesktopWindowManager::getInstance()->checkIconWidgetIntersection(id, currentPos);
            }
        }
        info->setProperty("createId", QVariant());
        info->setProperty("createPos", QVariant());
        qApp->setProperty("isRenamingByPeony",false);
        qApp->setProperty("RenamingUriByPeony","");
    }
}

QHash<int, QByteArray> AdvancedDesktopItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ScreenIdRole] = "screenId";
    roles[UriRole] = "uri";           // 映射 UriRole 到 "uri"
    roles[PositionRole] = "position"; // 映射 PositionRole 到 "position"
    roles[Qt::DecorationRole] = "decoration"; // 映射 Qt::DecorationRole 到 "decoration"
    roles[Qt::DisplayRole] = "display"; // 映射 Qt::DisplayRole 到 "display"
    roles[Qt::ToolTipRole] = "tooltip"; // 映射 Qt::DisplayRole 到 "display"
    roles[emblemRole] = "emblems"; // 映射 Qt::UserRole + 9 到 "emblems"
    roles[IsDirRole] = "isDir"; // 映射 Qt::UserRole + 9 到 "isDir"
    roles[SoftLinkRole] = "softLink"; // 映射 Qt::UserRole + 10 到 "softLink"
    roles[LabelColors] = "labelColors";
    roles[FileType] = "fileType";
    roles[FileSize] = "fileSize";
    roles[isDesktop] = "isDesktop";
    return roles;
}

void AdvancedDesktopIconItem::setData(const QVariant &value, int role)
{
    switch(role){
    case ScreenIdRole:{
        QString uri = data(UriRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            int id = value.toInt();
            metaInfo->setMetaInfoInt(SCREEN_ID, id);
        }
        break;
    }
    case PositionRole:{
        QString uri = data(UriRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            QPoint pos = value.toPoint();
            QStringList itemPos;
            itemPos<<QString::number(pos.x());
            itemPos<<QString::number(pos.y());
            metaInfo->setMetaInfoStringListV1(ITEM_GRID_POS_ATTRIBUTE, itemPos);
        }
        break;
    }
    case ExceptedPositionRole:{
        QString uri = data(UriRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            QStringList restorePos = value.toStringList();
            metaInfo->setMetaInfoStringList(RESTORE_ITEM_GRID_POS_ATTRIBUTE, restorePos);
        }
        break;
    }
    case SinglesSreenPositionRole:{
        QString uri = data(UriRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            QStringList singlesScreenPos = value.toStringList();
            metaInfo->setMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_GRID_POS_ATTRIBUTE, singlesScreenPos);
        }
        break;
    }
    case ExtendScreenPositionRole:{
        QString uri = data(UriRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            QStringList extendScreenPos = value.toStringList();
            metaInfo->setMetaInfoStringList(RESTORE_EXTEND_ITEM_GRID_POS_ATTRIBUTE, extendScreenPos);
        }
    }
    }
    QStandardItem::setData(value, role);
}

AdvancedDesktopItemModel::AdvancedDesktopItemModel(QObject *parent)
    : QStandardItemModel(parent)
{
    ThumbnailManager::getInstance()->setBlocked(true);
    setupTooltipConnections();

    // do not redo layout new items while we start an operation with peony's api.
    connect(FileOperationManager::getInstance(), &FileOperationManager::operationStarted, this, [=](){
        m_items_need_relayout.clear();
    });

    m_thumbnail_watcher = std::make_shared<FileWatcher>("thumbnail:///, this");

    connect(m_thumbnail_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri) {
        auto index = indexFromUri(uri);
        if (index.isValid())
            Q_EMIT this->dataChanged(index, index);
    });

    m_trash_watcher = std::make_shared<FileWatcher>("trash:///", this);

    this->connect(m_trash_watcher.get(), &FileWatcher::fileCreated, [=](const QString &uri) {
        //qDebug()<<"trash changed";
        auto trash = FileInfo::fromUri("trash:///");
        auto job = new FileInfoJob(trash);
        job->setAutoDelete();
        connect(job, &FileInfoJob::infoUpdated, [=]() {
            auto trashIndex = this->indexFromUri("trash:///");
            this->dataChanged(trashIndex, trashIndex);
            Q_EMIT this->requestClearIndexWidget(QStringList()<<uri<<"trash:///");
        });
        job->queryAsync();
    });

    this->connect(m_trash_watcher.get(), &FileWatcher::fileDeleted, [=](const QString &uri) {
        //qDebug()<<"trash changed";
        auto trash = FileInfo::fromUri("trash:///");
        auto job = new FileInfoJob(trash);
        job->setAutoDelete();
        connect(job, &FileInfoJob::infoUpdated, [=]() {
            auto trashIndex = this->indexFromUri("trash:///");
            this->dataChanged(trashIndex, trashIndex);
            Q_EMIT this->requestClearIndexWidget(QStringList()<<uri<<"trash:///");
        });
        job->queryAsync();
    });

    // monitor desktop
    QString desktopFile = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    qWarning() << "desktopfile:" << desktopFile;
    m_desktop_watcher = std::make_shared<FileWatcher>("file://" + desktopFile, this);
    m_desktop_watcher->setMonitorChildrenChange(true);
    m_desktop_watcher->connect(m_desktop_watcher.get(), &FileWatcher::fileCreated, this, &AdvancedDesktopItemModel::fileCreated);

    m_desktop_watcher->connect(m_desktop_watcher.get(), &FileWatcher::fileDeleted, [=](const QString &uri) {
        auto isbyorgtool = qApp->property("renameingbyorg").toBool();
        for (auto info : m_files) {
            if (info->uri() == uri) {
                int row = m_files.indexOf(info);
                if(!isbyorgtool){
                    removeRow(row, QModelIndex());
                    qDebug() <<"AdvancedDesktopIconView::rowsAboutToBeRemoved" <<row;
                    m_files.removeOne(info);
                }
                Q_EMIT sig_fileRemoved(uri);

                if (info->isDir()) {
                    QString displayName = info->displayName();
                    if (UserShareInfoManager::getInstance()->getUsershareLists().contains(displayName)) {
                        SharedDeleteInfoThread *thread = new SharedDeleteInfoThread(info->uri());
                        connect(thread, &SharedDeleteInfoThread::finished, thread, &SharedDeleteInfoThread::deleteLater);
                        thread->start();
                    }
                }
            }
        }

        //fix bug#329662, deleted file not update thumbnail issue
        ThumbnailManager::getInstance()->releaseThumbnail(uri);
    });

    m_desktop_watcher->connect(m_desktop_watcher.get(), &FileWatcher::fileChanged, [=](const QString &uri) {
        for (auto info : m_files) {
            if (info->uri() == uri) {
                auto job = new FileInfoJob(info);
                job->setAutoDelete();
                connect(job, &FileInfoJob::infoUpdated, this, [=]() {
                    auto metainfo = FileMetaInfo::fromUri(info->uri());
                    if (metainfo->hasHiddenInfo()) {
                        qDebug() << "hidden file info queryed" << info->uri();
                        Q_EMIT invalidateFilterRequest();
                    }
                    //file changed, force create thubnail, link tobug#83108
                    ThumbnailManager::getInstance()->releaseThumbnail(uri);
                    ThumbnailManager::getInstance()->createThumbnail(uri, m_thumbnail_watcher, true);
                    this->dataChanged(indexFromUri(uri), indexFromUri(uri));
                    Q_EMIT this->requestClearIndexWidget(QStringList()<<uri);

                    // task #355895
                    // 触发更新异常软链接，如果一直异常，则会进入轮询知道查询到正常信息
                    if (info->isSymbolLink() && !info->canRead()) {
                        this->pendingQuery(uri);
                    }
                });
                job->queryAsync();
                this->dataChanged(indexFromUri(uri), indexFromUri(uri));
                return;
            }
        }
    });

    connect(FileLabelModel::getGlobalModel(), &FileLabelModel::fileLabelChanged, this, [=](const QString &uri) {
         this->dataChanged(indexFromUri(uri), indexFromUri(uri));
    });


    //handle standard dir changing.
    m_dir_manager =new UserdirManager(this);
    //refresh after standard dir changed.
    connect(m_dir_manager,&UserdirManager::desktopDirChanged,[=](){
        refresh();
    });
    connect(m_dir_manager,&UserdirManager::thumbnailSetingChange,[=](){
        refresh();
    });

    connect(FileOperationManager::getInstance(), &FileOperationManager::operationStarted, this, [=](std::shared_ptr<FileOperationInfo> info){
        if (info.get()->m_type == FileOperationInfo::Rename) {
            m_renaming_operation_info = info;
            auto renamingUri = info.get()->m_src_uris.first();
            m_renameingUri = renamingUri;
            auto isbyorgtool =  qApp->property("renamebyorg").toBool();
            if(isbyorgtool){
                auto metaInfo = FileMetaInfo::fromUri(m_renameingUri);
                auto partitoninstanceid = metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID);
                qApp->setProperty("renameingbyorg",true);
                qApp->setProperty("partitoninstanceid",partitoninstanceid);
            }
            if (!renamingUri.endsWith(".desktop")) {
                m_renaming_operation_info = nullptr;
                return;
            }
            m_renaming_file_pos.first = renamingUri;
            m_renaming_file_pos.second = indexFromUri(renamingUri).data(PositionRole).toPoint();
        } else {
            m_renaming_file_pos.first = nullptr;
            m_renaming_file_pos.second = QPoint();
            m_renaming_operation_info = nullptr;
        }
    });
    connect(FileOperationManager::getInstance(), &FileOperationManager::operationFinished, this, [=](std::shared_ptr<FileOperationInfo> info){
        if (info.get()->m_type == FileOperationInfo::Rename) {
            qApp->setProperty("renameingbyorg",false);
            qApp->setProperty("renamebyorg",false);
            if (!info.get()->m_has_error) {
                auto renamingUri = info.get()->target();
                if (!renamingUri.endsWith(".desktop")) {
                    m_renaming_operation_info = nullptr;
                    m_renaming_file_pos.first = nullptr;
                    m_renaming_file_pos.second = QPoint();
                    return;
                }
                QPoint target_pos = indexFromUri(renamingUri).data(PositionRole).toPoint();
                //desktop文件重命名时，如果存在相同文件则不会重命名成功。由于该文件uri不会变，所以pos不变，无需更新pos
                if (target_pos.isNull() || (m_renaming_file_pos.second == target_pos)) {
                    //desktop文件重命名成功
                    m_renaming_file_pos.first = renamingUri;
                    m_items_need_relayout.removeOne(renamingUri);
                    m_items_need_relayout.removeOne(renamingUri + ".desktop");
                    auto index = indexFromUri(renamingUri);
                    setData(index, m_renaming_file_pos.second, PositionRole);
                } else {
                    //desktop文件(uri)重命名失败
                    QString &src_uri = info->m_src_uris.first();
                    Q_EMIT selectUri(src_uri);
                }
            } else {
                // restore/relayout?
            }
            m_renaming_operation_info = nullptr;
            QTimer::singleShot(100, this, [=]{
                m_renaming_file_pos.first = nullptr;
                m_renaming_file_pos.second = QPoint();
            });
        } else {
            m_renaming_file_pos.first = nullptr;
            m_renaming_file_pos.second = QPoint();
            m_renaming_operation_info = nullptr;
        }
    });

    auto settings = GlobalSettings::getInstance();
    m_showFileExtension = settings->isExist(SHOW_FILE_EXTENSION)? settings->getValue(SHOW_FILE_EXTENSION).toBool(): true;
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (SHOW_FILE_EXTENSION == key) {
            m_showFileExtension= GlobalSettings::getInstance()->getValue(key).toBool();
            beginResetModel();
            endResetModel();
        }
    });

    connect(DesktopMenuPluginManager::getInstance(), &DesktopMenuPluginManager::pluginLoadFinished, this, [=](){
       QTimer::singleShot(1000, this, [=]{
           for (auto file : m_files) {
               EmblemProviderManager::getInstance()->queryAsync(file->uri());
           }
       });
    });
    UserShareInfoManager::getInstance();

    connect(EmblemProviderManager::getInstance(), &EmblemProviderManager::requestUpdateFile, this, [=](const QString &uri){
        auto index = indexFromUri(uri);
        if (index.isValid())
            Q_EMIT this->dataChanged(index, index);
    });
}

AdvancedDesktopItemModel::~AdvancedDesktopItemModel()
{

}

bool findProgram(const QString &program)
{
    QFileInfo fi(program);
    if (!program.isEmpty() && fi.isExecutable()) {
        return true;
    }

    const QStringList paths = QFile::decodeName(qgetenv("PATH")).split(':');
    for(const QString &dir : paths) {
        QFileInfo fi= QFileInfo(dir + QDir::separator() + program);
        if (fi.isExecutable()) {
            return true;
        }
    }

    return false;
}

void AdvancedDesktopItemModel::refreshInternal()
{
    m_items_need_relayout.clear();
    ThumbnailManager::getInstance()->syncThumbnailPreferences();
    beginResetModel();
    for (auto info : m_files) {
        ThumbnailManager::getInstance()->releaseThumbnail(info->uri());
    }
    m_files.clear();
    m_enumerator = new FileEnumerator(this);
    connect(this, &AdvancedDesktopItemModel::prepareRefresh, m_enumerator, &FileEnumerator::cancel, Qt::DirectConnection);
    m_enumerator->setAutoDelete();
    QString desktopUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    m_enumerator->setEnumerateDirectory(desktopUri);
    m_enumerator->connect(m_enumerator, &FileEnumerator::enumerateFinished, this, &AdvancedDesktopItemModel::onEnumerateFinished);
    m_enumerator->enumerateAsync();
    endResetModel();
}

QVariant AdvancedDesktopItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    //qDebug()<<"data"<<m_files.at(index.row())->uri();
    auto info = m_files.at(index.row());
    switch (role) {
    case Qt::DisplayRole:{
        QString displayName = info->displayName();
        if (info->isDesktopFile())
        {
            displayName = FileUtils::handleDesktopFileName(info->uri(), info->displayName());
            return QVariant(displayName);
        }
        /* story#8359 【文件管理器】手动开启关闭文件拓展名 */
        if(!m_showFileExtension){
            if (info->isDir()) {
                return QVariant(displayName);
            }
            return QVariant(FileUtils::getBaseNameOfFile(displayName));

        }else
            return QVariant(displayName);
    }
    case Qt::ToolTipRole: {
        /**
         * @task #346285: 【Tooltips specification】Change the desktop icon (including the management-desktop-application icon) tooltips display,
         *  add supplementary text
         *
         * @author: Renyg <renyangguang@kylinos.cn>
         * @date:   2024-09-25
         */
        return getTooltipForIndex(index);
    }
    case Qt::DecorationRole: {
        return info->getIcon();
    }
    case UriRole:
        return info->uri();
    case IsLinkRole:
        return info->isSymbolLink();
    case emblemRole: {
        return getEmblems(index);
    }
    case IsDirRole: {
        //是否是目录
        return FileUtils::getFileIsFolder(index.data(Qt::UserRole).toString());
    }
    case SoftLinkRole: {
        //是否是软连接
        return FileUtils::getFileIsSymbolicLink(index.data(Qt::UserRole).toString());
    }
    case LabelColors:{
        QList<QColor> colors = info->getColors();
        QVariantList variantColors;
        for (const QColor &color : colors) {
            variantColors.append(QVariant(color));
        }
        return QVariant(variantColors);
    }
    case FileType:{
        auto type = info->fileType();
        return QVariant(type);
    }
    case FileSize:{
        auto size = info->fileSize();
        return QVariant(size);
    }
    case isDesktop:{
        auto contentType = info->type();
        auto path = info->filePath();
        auto uri = info->uri();
        bool isDesktopFile = path.endsWith(".desktop") || (contentType == "application/x-desktop") || uri=="trash:///" || uri=="computer:///";
        return QVariant(isDesktopFile);
    }
    default:
        QStandardItem *item = itemFromIndex(index);
        return item ? item->data(role) : QVariant();
    }
    return QVariant();
}


QVariant AdvancedDesktopItemModel::getEmblems(const QModelIndex &index) const
{
    QList<int> emblemPoses = {4, 3, 2, 1}; //bottom right, bottom left, top right, top left
    QStringList emblemNames = {"", "", "", ""};
    QString uri = index.data(UriRole).toString();
    FileInfo* file = FileInfo::fromUri(uri).get();
    if ((uri != "computer:///") && (uri != "trash:///")) {
        if (! file->canRead() || !file->isExistTargetOfSymlink()) {
            emblemPoses.removeOne(1);
            emblemNames[3] = "emblem-unreadable";

        } else if(! file->canWrite()/* && ! file->canExecute()*/) {
            //只读图标对应可读不可写情况，与可执行权限无关，link to bug#99998
            emblemPoses.removeOne(1);
            emblemNames[3] = "emblem-readonly";
        }
    }
    if (index.data(IsLinkRole).toBool()) {
        emblemPoses.removeOne(3);
        emblemNames[1] = "emblem-link-symbolic";
    }

    // paint extension emblems, FIXME: adjust layout, and implemet on indexwidget, other view.
    auto extensionsEmblems = EmblemProviderManager::getInstance()->getAllEmblemsForUri(file->uri());
    for (auto extensionsEmblem : extensionsEmblems) {
        if (emblemPoses.isEmpty()) {
            break;
        }
        QIcon icon = QIcon::fromTheme(extensionsEmblem);

        if (!icon.isNull()) {
            int pos = emblemPoses.takeFirst();
            switch (pos) {
            case 1: {
                emblemNames[3] = extensionsEmblem;
                break;
            }
            case 2: {
                emblemNames[2] = extensionsEmblem;
                break;
            }
            case 3: {
                emblemNames[1] = extensionsEmblem;
                break;
            }
            case 4: {
                emblemNames[0] = extensionsEmblem;
                break;
            }
            default:
                break;
            }
        }
    }
    return emblemNames;
}

std::shared_ptr<FileInfo> AdvancedDesktopItemModel::getFileInfo(const QModelIndex &index) const
{
    if (!index.isValid() || m_files.isEmpty() || index.row() >= m_files.length())
        return nullptr;

    return m_files.at(index.row());
}

void AdvancedDesktopItemModel::onEnumerateFinished(bool successed)
{
    if (!successed) {
        qWarning()<<"failed to enumerate desktop";
        beginResetModel();
        m_files.clear();
        endResetModel();
        return;
    }

    // check if there is info querying, if true, wait querying finished.
    while (!m_querying_files.isEmpty()) {
        qApp->processEvents();
    }

    if (m_files.count() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_files.count() - 1);
        m_files.clear();
        endRemoveRows();
    }

    auto computer = FileInfo::fromUri("computer:///");
    auto personal = FileInfo::fromPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    auto trash = FileInfo::fromUri("trash:///");

    QList<std::shared_ptr<FileInfo>> infos;

    infos<<computer;
    infos<<trash;
    infos<<personal;

    infos<<m_enumerator->getChildren();
    m_querying_files = infos;
    m_files = infos;

    for (auto info : infos) {
        auto asyncJob = new FileInfoJob(info);
        asyncJob->setAutoDelete();
        connect(this, &AdvancedDesktopItemModel::prepareRefresh, asyncJob, [=]{
            asyncJob->cancel();
            asyncJob->setProperty("isCancelled", true);
        }, Qt::DirectConnection);
        connect(asyncJob, &FileInfoJob::queryAsyncFinished, this, [=](bool successed){
            m_querying_files.removeOne(info);
            if (!successed) {
                m_files.removeOne(info);
            }

            if (asyncJob->property("isCancelled").toBool()) {
                // quit loop to avoid invalid data inserted;
                return;
            }

            // task #355895
            // 初始化时无法获取软链接源文件信息，进入轮询
            if (info->isSymbolLink() && !info->canRead()) {
                this->pendingQuery(info->uri());
            }

            if (m_querying_files.isEmpty()) {
                for (auto info : m_files) {
                    auto uri = info->uri();
                    auto item = new AdvancedDesktopIconItem(info->uri());
                    appendRow(item);

                    if (info->isDesktopFile()) {
                        ThumbnailManager::getInstance()->updateDesktopFileThumbnail(info->uri(), m_thumbnail_watcher);
                    } else {
                        ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher);
                    }
                }
                //qDebug()<<"startMornitor";
                m_trash_watcher->startMonitor();

                qWarning() << "desktopfile:" << m_desktop_watcher->currentUri() << " >>>> " << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                if (m_desktop_watcher->currentUri() != "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)) {
                    m_desktop_watcher->stopMonitor();
                    m_desktop_watcher->forceChangeMonitorDirectory("file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
                    m_desktop_watcher->setMonitorChildrenChange(true);
                }
                m_desktop_watcher->startMonitor();
                if (!initMetaInfo) {
                    getAllRestoreInfo();
                    initMetaInfo = true;
                    Q_EMIT emitFinish();
                    ThumbnailManager::getInstance()->setBlocked(false);
                    QTimer::singleShot(500, this, [=]{
                        for (auto file : m_files) {
                            ThumbnailManager::getInstance()->createThumbnail(file->uri(), m_thumbnail_watcher);
                        }
                    });
                } else {
                    Q_EMIT refreshed();
                }
            }
        });
        asyncJob->queryAsync();
    }
}

const QModelIndex AdvancedDesktopItemModel::indexFromUri(const QString &uri)
{
    for (auto info : m_files) {
        if (info->uri() == uri) {
            return index(m_files.indexOf(info), 0);
        }
    }
    return QModelIndex();
}

Qt::ItemFlags AdvancedDesktopItemModel::flags(const QModelIndex &index) const
{
    auto uri = index.data(UriRole).toString();
    auto info = FileInfo::fromUri(uri);
    if (index.isValid()) {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        flags |= Qt::ItemIsDragEnabled;
        flags |= Qt::ItemIsEditable;
        if (info->isDir()) {
            flags |= Qt::ItemIsDropEnabled;
        }
        return flags;
    } else {
        return Qt::ItemIsDropEnabled;
    }
}

QMimeData *AdvancedDesktopItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* data = QAbstractItemModel::mimeData(indexes);
    //set urls data URLs correspond to the MIME type text/uri-list.
    QList<QUrl> urls;
    QStringList uris;
    for (auto index : indexes) {
        QUrl url = index.data(UriRole).toString();
        if (!urls.contains(url))
            urls<<url;
        uris<<index.data(UriRole).toString();
    }
    data->setUrls(urls);
    auto string = uris.join(" ");
    data->setData("peony-qt/encoded-uris", string.toUtf8());
    data->setText(string);
    return data;
}

bool AdvancedDesktopItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    //qDebug()<<row<<column;
    //qDebug()<<"drop mime data"<<parent.data()<<index(row, column, parent).data();
    //judge the drop dest uri.
    QString destDirUri = nullptr;
    bool desktopAutoLayout = GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool();
    bool useParentUri = parent.isValid();
    if (desktopAutoLayout && parent.isValid()) {
        const QString parentUri = parent.data(UriRole).toString();
        bool isMoveTarget = parentUri.startsWith("trash:///");
        if (!isMoveTarget && !parentUri.startsWith("computer:///")) {
            auto parentInfo = FileInfo::fromUri(parentUri);
            if (parentInfo->isEmptyInfo()) {
                FileInfoJob j(parentInfo);
                j.querySync();
            }
            isMoveTarget = parentInfo->isDir();
        }
        useParentUri = isMoveTarget;
    }
    if (useParentUri) {
        destDirUri = parent.data(UriRole).toString();
    } else {
        destDirUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    //if destDirUri was not set, do not execute a drop.
    if (destDirUri.isNull()) {
        return false;
    }

    auto info = FileInfo::fromUri(destDirUri);
    if (info.get()->isEmptyInfo()) {
        // note that this case nearly won't happened.
        // but there is a bug reported due to this.
        // link to task #48798.
        FileInfoJob j(info);
        j.querySync();
    }
    if (!info->isDir()  && ! destDirUri.startsWith("trash:///")) {
        return false;
    }

    //NOTE:
    //do not allow drop on it self.
    auto urls = data->urls();
    if (urls.isEmpty())
        return false;

    bool hasPeonyEncodedUris = false;
    if (data->hasFormat("peony-qt/encoded-uris")) {
        if (!data->data("peony-qt/encoded-uris").isEmpty()) {
            hasPeonyEncodedUris = true;
        }
    }

    bool bIDMMoveToOther = false;
    QStringList srcUris;
    if (hasPeonyEncodedUris) {
        srcUris = QString(data->data("peony-qt/encoded-uris")).split(" ");
        for (QString uri : srcUris) {
            if (uri.startsWith("recent://")) {
                srcUris.removeOne(uri);
            }
            if (uri.startsWith("idm://")) {
                bIDMMoveToOther = true;
                srcUris.removeOne(uri);
                QString tempUri = uri.split('/', QString::SkipEmptyParts).last();
                QString realUri = "file://" + QUrl::fromPercentEncoding(tempUri.toUtf8());
                srcUris.append(realUri);
            }
        }
    } else {
        for (auto url : urls) {
            //can not drag file from recent
            if (url.url().startsWith("recent://"))
                return false;
            srcUris<<url.url();
        }
    }
    srcUris.removeDuplicates();

    if (srcUris.isEmpty()) {
        return false;
    }

    //can not drag file to recent
    if (destDirUri.startsWith("recent://"))
        return false;

    //not allow drag file to itself
    if (srcUris.contains(destDirUri)) {
        return false;
    }

    //can not move StandardPath to any dir
    if (action == Qt::MoveAction && FileUtils::containsStandardPath(srcUris)) {
        return false;
    }

    bool bMoveFromSearchTab = false;
    if (data->hasFormat("peony-qt/is-search")) {
        bMoveFromSearchTab = QVariant(data->data("peony-qt/is-search")).toBool();
    }

    bool b_trash_item = false;
    for(auto path : srcUris)
    {
        if (path.contains("trash:///"))
        {
            b_trash_item = true;
            break;
        }
    }
    //drag from trash to another place, return false
    //comment to fix can not drag to copy trash file,link to bug#117741
//    if (b_trash_item && destDirUri != "trash:///")
//        return false;

    auto fileOpMgr = FileOperationManager::getInstance();
    bool addHistory = true;
    bool canNotTrash = false;

    for (auto uri : srcUris) {
        if (uri.startsWith("filesafe:///")) {
            canNotTrash = true;
            break;
        }
    }

    if (destDirUri.startsWith("trash:///")) {
        // 如果是保护箱删除时，不会反馈删除弹窗，保护箱文件不影响
        if(!(srcUris.first().startsWith("filesafe:///") &&
            (QString(srcUris.first()).remove("filesafe:///").indexOf("/") == -1))) {
            //fix bug#91525, can trash file in U disk issue
            FileOperationUtils::trash(srcUris, true, bMoveFromSearchTab);
//            if(canNotTrash){
//                FileOperationUtils::trash(srcUris, false);
//            }else {
//                FileTrashOperation *trashOp = new FileTrashOperation(srcUris);
//                fileOpMgr->startOperation(trashOp, addHistory);
//            }
        }
    } else {
        qDebug() << "DesktopItemModel dropMimeData:" <<action;
        //krme files can not move to other place, default set as copy action
        if (srcUris.first().startsWith("kmre:///") || srcUris.first().startsWith("kydroid:///"))
            action = Qt::CopyAction;

        //filesafe files can not move to other place, default set as copy action
        if (srcUris.first().startsWith("filesafe:///"))
            action = Qt::CopyAction;

        //fix drag trash file to other path is copy issue,link to bug#117741
        if (srcUris.first().startsWith("trash:///") && action == Qt::MoveAction){
            //not copy move, do target move to delete file in trash
            action = Qt::TargetMoveAction;
        }

        if (srcUris.first().startsWith("idm:///") || destDirUri.startsWith("idm:///") || bIDMMoveToOther)
            action = Qt::CopyAction;

        auto op = FileOperationUtils::moveWithAction(srcUris, destDirUri, true, action, bMoveFromSearchTab);
        op->connect(op, &FileOperation::operationFinished, this, [=](){
            // op->getOperationInfo();
            auto opInfo = op->getOperationInfo();
            auto targetUirs = opInfo->dests();
            //Peony::SoundEffect::getInstance()->copyOrMoveSucceedMusic();
            //Task#152997, use sdk play sound
            if (op->hasError()) {
                return;
            }
            bool isdragfromOtherView = qApp->property("isDragFromOtherView").toBool();
            if(isdragfromOtherView){
                qApp->setProperty("isDragFromOtherView",false);
                Q_EMIT dragFromOtherViewAutoLayout(targetUirs);
            }
#ifdef KY_SDK_SOUND_EFFECTS
            kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
#endif
        }, Qt::BlockingQueuedConnection);
    }

    //NOTE:
    //we have to handle the dnd with file operation, so do not
    //use QAbstractModel::dropMimeData() here;
    return false;
}

void AdvancedDesktopItemModel::refresh()
{
    Q_EMIT prepareRefresh();

    beginResetModel();
    m_files.clear();
    m_items_need_relayout.clear();
    clear();
    endResetModel();

    m_desktop_info = FileInfo::fromPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    auto infoJob = new FileInfoJob(m_desktop_info);
    infoJob->setAutoDelete();
    connect(infoJob, &FileInfoJob::queryAsyncFinished, this, [=](bool successed){
        if (successed) {
            refreshInternal();
        } else {
            qWarning()<<"desktop model refreshs: can not query desktop info"<<m_desktop_info->uri();
        }
    });
    connect(this, &AdvancedDesktopItemModel::prepareRefresh, infoJob, &FileInfoJob::cancel, Qt::DirectConnection);
    infoJob->queryAsync();
}
void AdvancedDesktopItemModel::setdata(int i, const QVariant &value, int role)
{
    QModelIndex modelindex = index(i, 0);
    AdvancedDesktopIconItem* item =  static_cast<AdvancedDesktopIconItem*>(itemFromIndex(modelindex)) ;
    item->setData(value, role);
}
void AdvancedDesktopItemModel::updateMetaInfo(QStandardItem *item)
{
    //获取当前屏幕的view
    QString uri = item->data(UriRole).toString();
    auto metaInfo = FileMetaInfo::fromUri(uri);
    if (metaInfo) {
        int id = metaInfo->getMetaInfoInt(SCREEN_ID);
        item->setData(id, ScreenIdRole);
        QStringList strPos = metaInfo->getMetaInfoStringListV1(ITEM_GRID_POS_ATTRIBUTE);
        if (2 == strPos.count() && strPos[0] >= 0 && strPos[1] >= 0) {
            QPoint pos(strPos[0].toInt(), strPos[1].toInt());
            item->setData(pos, PositionRole);
        }
        QStringList value = metaInfo->getMetaInfoStringList(RESTORE_ITEM_GRID_POS_ATTRIBUTE);
        item->setData(value, ExceptedPositionRole);
        value = metaInfo->getMetaInfoStringList(RESTORE_EXTEND_ITEM_GRID_POS_ATTRIBUTE);
        item->setData(value, ExtendScreenPositionRole);
        value = metaInfo->getMetaInfoStringList(RESTORE_SINGLESCREEN_ITEM_GRID_POS_ATTRIBUTE);
        item->setData(value, SinglesSreenPositionRole);
    }
}

void AdvancedDesktopItemModel::getAllRestoreInfo()
{
    for (int row = 0; row < rowCount(); row++) {
        QModelIndex modelIndex = index(row, 0);
        auto uri = modelIndex.data(UriRole).toString();
        int currentId = modelIndex.data(ScreenIdRole).toInt();
        QVariant tmp = modelIndex.data(ExceptedPositionRole);
        QStringList extendPos = tmp.toStringList();
       
        if (3 == extendPos.count()) {
            int col = extendPos.at(0).toInt();
            int row = extendPos.at(1).toInt();
            int id = extendPos.takeAt(2).toInt();
            if (id == currentId && col >= 0 && row >= 0) {
                 QVariant value(extendPos);
                 QPoint pos(col, row);
                 setData(modelIndex, QStringList(""), ExceptedPositionRole);
                 setData(modelIndex, pos, PositionRole);
            }
        }
    }
}

void AdvancedDesktopItemModel::saveExtendItemInfo()
{
    bool desktopAutoLayout = GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool();
    QVector<IconData> iconDataList;
    //task#74174 扩展屏的元素记录到metInfo,以便以后恢复
    for (int i = 0; i < rowCount(); i++) {
        QModelIndex modelIndex = index(i, 0);
        //QMap<int, QVariant>  roleData = itemData(modelIndex);
        auto position = modelIndex.data(PositionRole).toPoint();
        int currentId = modelIndex.data(ScreenIdRole).toInt();
        QStringList extendPos = modelIndex.data(ExtendScreenPositionRole).toStringList();
        //恢复单屏下的图标
        if (desktopAutoLayout) {
            QString uri =  modelIndex.data(UriRole).toString();
            iconDataList.append({currentId, position, modelIndex, uri});
        } else {
            QStringList pos = modelIndex.data(SinglesSreenPositionRole).toStringList();
            if (pos.count() == 2) {
                setData(modelIndex, QPoint(pos[0].toInt(), pos[1].toInt()), PositionRole);
                setData(modelIndex, QStringList(), SinglesSreenPositionRole);
            }
        }
        //如果不存在扩展屏坐标，则保存当前坐标到扩展屏坐标
        if (extendPos.count() != 3) {
            QStringList topLeft;
            topLeft<<QString::number(position.x());
            topLeft<<QString::number(position.y());
            topLeft<<QString::number(currentId);
            setData(modelIndex, topLeft, ExtendScreenPositionRole);
        }

        setData(modelIndex, 0, ScreenIdRole);
    }

    if (!iconDataList.empty()) {
        // 排序
        std::sort(iconDataList.begin(), iconDataList.end());

        int newPositionIndex = 0;
        for (const auto &iconData : iconDataList) {
            setData(iconData.modelIndex, QPoint(newPositionIndex, 0), PositionRole);
            setData(iconData.modelIndex, QStringList(), SinglesSreenPositionRole);
            ++newPositionIndex;
        }
    }
}

void AdvancedDesktopItemModel::resetExtendItemInfo(QSet<int> idList)
{
    for (int row = 0; row < rowCount(); row++) {
        QModelIndex modelIndex = index(row, 0);
        //QMap<int, QVariant>  roleData = itemData(modelIndex);
        QVariant value = modelIndex.data(UriRole);
        QPoint currentPos = modelIndex.data(PositionRole).toPoint();
        value = modelIndex.data(ExtendScreenPositionRole);
        QStringList extendPos = value.toStringList();
        if (3 == extendPos.count()) {
            int col = extendPos.at(0).toInt();
            int row = extendPos.at(1).toInt();
            int id = extendPos.takeAt(2).toInt();
            if (col >= 0 && row >= 0 && idList.contains(id)) {
                 QPoint pos(col, row);
                 setData(modelIndex, pos, PositionRole);
                 setData(modelIndex, id, ScreenIdRole);
                 setData(modelIndex, QStringList(""), ExtendScreenPositionRole);
            }
        }
        value = modelIndex.data(SinglesSreenPositionRole);
        QStringList screenlist = value.toStringList();
        if (screenlist.count() != 2) {
            QStringList tmp;
            tmp<<QString::number(currentPos.x());
            tmp<<QString::number(currentPos.y());
            setData(modelIndex, tmp, SinglesSreenPositionRole);
        }
    }
}

void AdvancedDesktopItemModel::clearExtendItemPos(bool saveId)
{
    //重置扩展屏和单屏坐标
    for (int row = 0; row < rowCount(); row++) {
        QModelIndex modelIndex = index(row, 0);
        //QMap<int, QVariant>  roleData = itemData(modelIndex);
        QStringList tmp("");
        setData(modelIndex, tmp, SinglesSreenPositionRole);
        if (saveId) {
            setData(modelIndex, tmp, ExceptedPositionRole);
            QStringList extendPos =  modelIndex.data(ExceptedPositionRole).toStringList();
            if (extendPos.count() == 3) {
                tmp.clear();
                tmp<<"-1"<<"-1"<<extendPos[2];
            }
        }
        setData(modelIndex, tmp, ExtendScreenPositionRole);
    }
}

void AdvancedDesktopItemModel::fileCreated(const QString &uri)
{
    qDebug()<<"desktop file created"<<uri;

    auto info = FileInfo::fromUri(uri);
    bool exsited = false;
    int index = 0;
    for (auto file : m_files) {
        if (file->uri() == info->uri()) {
            exsited = true;
            break;
        }
    }
    auto isbyorgtool = qApp->property("renameingbyorg").toBool();
    for (auto file : m_files) {
        if (m_renameingUri == file->uri()) {
            break;
        }
        index++;
    }
    if (!exsited) {
        if (!m_renaming_file_pos.first.isEmpty() && uri != m_renaming_file_pos.first && uri.contains(m_renaming_file_pos.first)&&!uri.endsWith(".desktop")) {
            return;
        }
        if(qApp->property("isRenamingByPeony").toBool()){  //过滤desktop renmae创建临时文件
            QString peonyRenameUri = qApp->property("RenamingUriByPeony").toString();
            if(!peonyRenameUri.isEmpty() &&  uri!= peonyRenameUri && uri.contains(peonyRenameUri) && !uri.endsWith(".desktop")){
                return;
            }
        }
        m_items_need_relayout.append(uri);
        m_items_need_relayout.removeOne(m_renaming_file_pos.first);
        m_items_need_relayout.removeOne(m_renaming_file_pos.first + ".desktop");
        if (m_renaming_operation_info.get()) {
            m_items_need_relayout.removeOne(m_renaming_operation_info.get()->target());
        }
        m_items_need_relayout.removeDuplicates();

        auto job = new FileInfoJob(info);
        job->setAutoDelete();
        job->querySync();

        // locate new item =====
        //task#74174 扩展模式下支持拖拽图标放置到扩展屏, 创建文件获取当前view

        //file changed, force create thubnail, link tobug#83108
        ThumbnailManager::getInstance()->createThumbnail(info->uri(), m_thumbnail_watcher, true);
        //m_files<<info;
        Q_EMIT setInfoPos(info->uri());
        AdvancedDesktopIconItem *item = new AdvancedDesktopIconItem(info->uri(), true);
        if(isbyorgtool){
             m_files.replace(index,info);
            this->setItem(index,item);
        }else{
            m_files<<info;
            this->appendRow(item);
        }
        int id = item->data(ScreenIdRole).toInt();
        Q_EMIT sig_fileCreated(id, uri);
    }
    else{
        //file content changed, need update fileinfo, fix bug#76908
        auto job = new FileInfoJob(info);
        job->setAutoDelete();
        job->querySync();
    }
}

void AdvancedDesktopItemModel::pendingQuery(const QString &uri)
{
    // task: #355895
    // note: 指向外部分区的软链接在桌面初始化时可能查询不到链接的源文件，
    // 这可能是因为外部分区挂载比桌面拉起慢导致的，这里增加一个pending机制
    if (!m_pending_query_timer) {
        m_pending_query_timer = new QTimer(this);
        m_pending_query_timer->setInterval(1000);
        connect(m_pending_query_timer, &QTimer::timeout, this, [=]{
            for (auto uri : m_pending_query_uris) {
                // 使用现有文件改变流程触发更新
                m_desktop_watcher->fileChanged(uri);
            }
            m_pending_query_uris.clear();
        });
    }
    m_pending_query_uris.insert(uri);
    m_pending_query_timer->start();
}

void AdvancedDesktopItemModel::saveExtendItemInfo(int id)
{
    //task#74174 销毁时保存扩展屏元素的坐标点
    for (int row = 0; row < rowCount(); row++) {
        QModelIndex modelIndex = index(row, 0);
        if (!modelIndex.isValid()) {
            continue;
        }
        int currentId = modelIndex.data(ScreenIdRole).toInt();
        if(currentId != id) {
            continue;
        }
        //QMap<int, QVariant>  roleData = itemData(modelIndex);
        auto uri = modelIndex.data(UriRole).toString();
        QVariant tmp = modelIndex.data(ExceptedPositionRole);
        QStringList extendPos = tmp.toStringList();
        bool needUpdatePosition = (extendPos.size() != 3) ||
                                 (extendPos.size() == 3 && extendPos[2].toInt() != currentId);
        if (needUpdatePosition) {
            QPoint pos = modelIndex.data(PositionRole).toPoint();
            extendPos = QStringList{ QString::number(pos.x()), QString::number(pos.y()), QString::number(id) };
        }
        setData(modelIndex, QStringList(), ExceptedPositionRole);
        setData(modelIndex, extendPos, ExtendScreenPositionRole);
        QStringList singlesScreen = modelIndex.data(SinglesSreenPositionRole).toStringList();
        if(2 == singlesScreen.count()) {
            int col = singlesScreen.at(0).toInt();
            int row = singlesScreen.at(1).toInt();
            QPoint pos(col, row);
            setData(modelIndex, pos, PositionRole);
        }
        setData(modelIndex, 0, ScreenIdRole);
    }
}

void AdvancedDesktopItemModel::setupTooltipConnections()
{
    connect(&TooltipsManagerInstance, &TooltipsManager::tooltipReady, this, [this](const QString& uri, const QString&) {
        auto index = indexFromUri(uri);
        if (index.isValid()) {
            m_pendingTooltips.remove(uri);
            Q_EMIT dataChanged(index, index, {Qt::ToolTipRole});
        }
    });

    m_pendingTooltips.clear();
}

QVariant AdvancedDesktopItemModel::getTooltipForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return QVariant();

    auto info = m_files.at(index.row());
    QString uri = info->uri();

    if (TooltipsManagerInstance.isTimeConsumingFile(info.get())) {
        if (TooltipsManagerInstance.hasValidMetadataCache(uri)) {
            return QVariant(TooltipsManagerInstance.generateTooltip(info.get()));
        }

        if (!m_pendingTooltips.contains(uri)) {
            m_pendingTooltips.insert(uri);
            QMetaObject::invokeMethod(&TooltipsManagerInstance, "loadMetadataAsync", Qt::QueuedConnection, Q_ARG(QString, uri));
        }

        return QVariant(TooltipsManagerInstance.generateTooltip(info.get()));
    }

    return QVariant(TooltipsManagerInstance.generateTooltip(info.get()));
}

void AdvancedDesktopItemModel::updateTooltips(const QString &uri)
{
    auto index = indexFromUri(uri);
    if (index.isValid()) {
        Q_EMIT dataChanged(index, index, {Qt::ToolTipRole});
    }
}
