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
 *
 */

#include "side-bar-vfs-item.h"
#include "vfs-plugin-iface.h"
#include "vfs-plugin-manager.h"
#include "file-utils.h"

#include "side-bar-model.h"
#include "file-enumerator.h"
#include "side-bar-separator-item.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-watcher.h"
#include "idm-dbus-manager.h"
#include <QUrl>

using namespace Peony;

SideBarVFSItem::SideBarVFSItem(const QString& uri, SideBarVFSItem* parentItem, SideBarModel *model, QObject *parent) :
    SideBarAbstractItem(model, parent)
  ,m_parentItem(parentItem)
{
   m_uri = uri;
}

SideBarVFSItem::~SideBarVFSItem()
{
    if(m_enumerator)
    {
        m_enumerator->cancel();
        delete m_enumerator;
        m_enumerator = nullptr;
    }
}

QString SideBarVFSItem::uri()
{
    return m_uri;
}

QString SideBarVFSItem::displayName()
{
    if (m_displayName.isEmpty()) {
        auto vfsFile = VFSPluginManager::getInstance()->newVFSFile(uri());
        auto info = g_file_query_info(vfsFile, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
        m_displayName = g_file_info_get_display_name(info);
        g_object_unref(info);
        g_object_unref(vfsFile);
    }

    return m_displayName;
}

QString SideBarVFSItem::iconName()
{
    if (m_iconName.isEmpty()) {
        m_iconName = FileUtils::getFileIconName(m_uri);
    }

    return m_iconName;
}

void SideBarVFSItem::findChildren()
{
    clearChildren();
    /* 枚举操作 */
    if(!m_enumerator)
        m_enumerator= new FileEnumerator();
    else {
        m_enumerator->cancel();
        m_enumerator->deleteLater();
        m_enumerator = new FileEnumerator();
    }
    m_enumerator->setEnumerateDirectory(m_uri);

    connect(m_enumerator,&FileEnumerator::enumerateFinished,this, &SideBarVFSItem::slot_enumeratorFinish);
    m_enumerator->enumerateAsync();

    if(!m_watcher){
        m_watcher = std::make_shared<FileWatcher>(m_uri, nullptr, true);
        connect(m_watcher.get(), &FileWatcher::fileCreated, this, &SideBarVFSItem::slot_fileCreate);
        connect(m_watcher.get(), &FileWatcher::fileDeleted, this, &SideBarVFSItem::slot_fileDelete);
        connect(m_watcher.get(), &FileWatcher::fileChanged, this, &SideBarVFSItem::slot_fileSafeUpdate);
        connect(m_watcher.get(), &FileWatcher::directoryUnmounted, this, &SideBarVFSItem::slot_fileSafeLocked);
    }
    m_watcher->setMonitorChildrenChange();
    m_watcher->startMonitor();

    if (m_uri == "idm:///") {
        qDebug() << "==========connect";
        auto idmDbusManager = IDMDBusManager::getInstance();
        connect(idmDbusManager, &IDMDBusManager::spaceAdded, this, &SideBarVFSItem::slot_spaceAdded);
        connect(idmDbusManager, &IDMDBusManager::spaceRemoved, this, &SideBarVFSItem::slot_spaceDeleted);
        connect(idmDbusManager, &IDMDBusManager::spaceRenamed, this, &SideBarVFSItem::slot_spaceRenamed);
    }
}

void SideBarVFSItem::findChildrenAsync()
{
    //TODO add async method.
    findChildren();
}

void SideBarVFSItem::clearChildren()
{
    if(m_watcher)
        m_watcher->stopMonitor();
    SideBarAbstractItem::clearChildren();
}

void SideBarVFSItem::mount()
{
    if (m_uri.startsWith("idm://")) {
        IDMDBusManager::getInstance()->onSpaceCreateRequest();
    }
}

void SideBarVFSItem::slot_enumeratorFinish(bool successed)
{
    auto infos = m_enumerator->getChildren();    
    if (infos.isEmpty() || !successed) {
        auto separator = new SideBarSeparatorItem(SideBarSeparatorItem::EmptyFile, this, m_model);
        m_model->beginInsertRows(this->firstColumnIndex(), 0, 0);
        this->m_children->prepend(separator);
        m_model->endInsertRows();
        Q_EMIT findChildrenFinished();
        return;
    }

    int real_children_count = infos.count();
    QVector<SideBarAbstractItem *> tmpItems;
    bool isEmpty = true;
    for (auto info: infos) {
        if (!info->displayName().startsWith(".") && (info->isDir())) {
            isEmpty = false;
        }

        if (!(info->isDir())){/* 只显示文件夹，文件不显示 */
            real_children_count--;
            continue;
        }

        /* 更新fileinfo */
        FileInfoJob job(info);
        job.querySync();

        SideBarVFSItem *item = new SideBarVFSItem(info->uri(), this, m_model);
        tmpItems.append(item);
    }
    if (real_children_count > 0) {
        m_model->beginInsertRows(this->firstColumnIndex(), 0, real_children_count - 1);
        m_children->append(tmpItems);
        m_model->endInsertRows();
    }

    if (isEmpty) {/* 目录下只有文件的情形 */
        auto separator = new SideBarSeparatorItem(SideBarSeparatorItem::EmptyFile, this, m_model);
        m_model->beginInsertRows(this->firstColumnIndex(), 0, 0);
        this->m_children->prepend(separator);
        m_model->endInsertRows();
    }

    Q_EMIT findChildrenFinished();
}

void SideBarVFSItem::slot_fileCreate(const QString &uri)
{
    qDebug()<<"created:"<<uri;
    for (auto item : *m_children) {
        if (item->uri() == uri) {
            return;
        }
    }

    /* 更新fileinfo */
    auto info = FileInfo::fromUri(uri);
    FileInfoJob job(uri, this);
    job.querySync();

    if (!(info->isDir())){/* 只显示文件夹，文件不显示 */
        return;
    }
    /* 新增项 */
    SideBarVFSItem *item = new SideBarVFSItem(info->uri(), this, m_model);
    item->m_iconName =info->iconName();/* 解决有时候没有图标的情况 */
    m_model->beginInsertRows(this->firstColumnIndex(), m_children->count(), m_children->count());
    m_children->append(item);
    m_model->endInsertRows();
    m_model->indexUpdated(this->firstColumnIndex());/* 如果添加了非空项，则需要删除空项 */
    m_model->dataChanged(item->firstColumnIndex(), item->lastColumnIndex());

}

void SideBarVFSItem::slot_fileDelete(const QString &uri)
{
    qDebug()<<"deleted:"<<uri;
    for (auto child : *m_children) {
        if (child->uri() == uri) {
            int index = m_children->indexOf(child);
            m_model->beginRemoveRows(firstColumnIndex(), index, index);
            m_children->removeOne(child);
            m_model->endRemoveRows();
            child->deleteLater();
            break;
        }
    }
    m_model->indexUpdated(this->firstColumnIndex());/* 如果删除了所有项，则需要添加空项 */
}

void SideBarVFSItem::slot_fileSafeLocked(const QString &uri)
{
    Q_UNUSED(uri);
    /* 锁定后折叠子项 */
    auto index = this->firstColumnIndex();
    Q_EMIT m_model->signal_collapsedChildren(index);
    m_model->dataChanged(this->firstColumnIndex(), this->lastColumnIndex());
}

void SideBarVFSItem::slot_fileSafeUpdate(const QString &uri)
{
    /* 更新iconName */
    for (auto item : *m_children){
        if (item->uri() == uri){
            auto info = FileInfo::fromUri(item->m_uri);
            /* 更新file info */
            FileInfoJob j(info);
            j.querySync();

            item->m_iconName = info.get()->iconName();
            m_model->dataChanged(item->firstColumnIndex(), item->lastColumnIndex());
            break;
        }
    }
}

void SideBarVFSItem::slot_fileRename(const QString &oldUri, const QString &newUri)
{
    for (auto child :  *m_children) {
        if (child->uri() == oldUri) {

            SideBarVFSItem *item = qobject_cast<SideBarVFSItem *>(child);
            if (!item)
                continue;

            auto info = FileInfo::fromUri(newUri);
            /* 更新file info */
            FileInfoJob j(info);
            j.querySync();

            item->m_uri = newUri;
            item->m_iconName = info.get()->iconName();
            item->m_displayName = info.get()->displayName();

            m_model->dataChanged(item->firstColumnIndex(), item->lastColumnIndex());
            break;
        }
    }
}

void SideBarVFSItem::slot_spaceAdded(const QString &spaceName)
{
    QString uri = m_uri + QString(QUrl::toPercentEncoding(spaceName));
    slot_fileCreate(uri);
}

void SideBarVFSItem::slot_spaceDeleted(const QString &spaceName)
{
    QString uri = m_uri + QString(QUrl::toPercentEncoding(spaceName));
    slot_fileDelete(uri);
}

void SideBarVFSItem::slot_spaceRenamed(const QString &oldName, const QString &newName)
{
    QString oldUri = m_uri + QString(QUrl::toPercentEncoding(oldName));
    QString newUri = m_uri + QString(QUrl::toPercentEncoding(newName));
    slot_fileRename(oldUri, newUri);
}

