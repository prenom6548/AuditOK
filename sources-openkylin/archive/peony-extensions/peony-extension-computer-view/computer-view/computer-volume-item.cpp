/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
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
#include "computer-volume-item.h"
#include <peony-qt/volumeManager.h>
#include <peony-qt/file-utils.h>
//#include <peony-qt/datacdrom.h>
#ifndef KY_UDF_BURN
#include <peony-qt/datacdrom.h>
#else
#include <libkyudfburn/datacdrom.h>
using namespace UdfBurn;
#endif

#include <udisks/udisks.h>
#include <sys/stat.h>
#include <peony-qt/global-fstabdata.h>
#include <peony-qt/sync-thread.h>
#include <peony-qt/image-mount-manager.h>

#include "computer-model.h"
#include "computer-proxy-model.h"
#include "computer-user-share-item.h"
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDir>
#include <QStringList>
#include <QFileInfoList>
#include <QTimer>

#ifdef signals
#undef signals
#endif

#ifdef KY_SDK_DISKINFO
#include <kysdk/kysdk-system/libkydiskinfo.h>
#endif

#include <gio/gunixmounts.h>

ComputerVolumeItem::ComputerVolumeItem(GVolume *volume, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent) : AbstractComputerItem(model, parentNode, parent)
{
    m_model->beginInsertItem(parentNode->itemIndex(), parentNode->m_children.count());
    parentNode->m_children<<this;

    if (parentNode->itemType() != Volume) {
        m_displayName = tr("Volume");
        m_model->endInsterItem();
        return;
    }

    m_cancellable = g_cancellable_new();
    if (!volume) {
        setProperty("isFileSystem", true);
        if (qApp->property("isSystemEncrypted").toBool()) {
            setProperty("emblemIconName", "emblem-unlocked");
        }
        m_icon = QIcon::fromTheme("drive-harddisk-system");
        m_uri = "file:///";
        m_displayName = tr("System Disk");
        auto file = g_file_new_for_uri("file:///");
        g_file_query_filesystem_info_async(file, "*", 0, m_cancellable,
                                           GAsyncReadyCallback(query_root_info_async_callback), this);
        m_model->endInsterItem();
        return;
    }

    // Keep volume alive across transient remove/add sequences (label changes etc.)
    // Otherwise callbacks may still run while the underlying GVolume is already finalized.
    m_gvolume = G_VOLUME(g_object_ref(volume));
    // We own the extra ref in m_gvolume, so Peony::Volume must NOT take over.
    m_volume = std::make_shared<Peony::Volume>(m_gvolume, false);
    m_volumeChangedHandle = g_signal_connect(m_gvolume, "changed", G_CALLBACK(volume_changed_callback), this);
    m_volumeRemovedHandle = g_signal_connect(m_gvolume, "removed", G_CALLBACK(volume_removed_callback), this);

    m_volumeMonitor = g_volume_monitor_get();
    m_mountChangedHandle = g_signal_connect(m_volumeMonitor, "mount_changed", G_CALLBACK(mount_changed_callback), this);
    m_mountAddedHandle = g_signal_connect(m_volumeMonitor, "mount_added", G_CALLBACK(mount_added_callback), this);

    updateInfoAsync();

    m_model->endInsterItem();
}

//ComputerVolumeItem::ComputerVolumeItem(const QString uri,ComputerModel *model,AbstractComputerItem *parentNode,QObject *parent) : AbstractComputerItem(model,parentNode,parent){
//    if(uri.isNull() || uri.isEmpty())
//        return;

//    //collectInfoWhenGpartedOpen(uri);
//}

ComputerVolumeItem::~ComputerVolumeItem()
{
    if (m_volumeMonitor) {
        if (m_mountChangedHandle)
            g_signal_handler_disconnect(m_volumeMonitor, m_mountChangedHandle);
        if (m_mountAddedHandle)
            g_signal_handler_disconnect(m_volumeMonitor, m_mountAddedHandle);
    }
    if (m_gvolume) {
        if (m_volumeChangedHandle)
            g_signal_handler_disconnect(m_gvolume, m_volumeChangedHandle);
        if (m_volumeRemovedHandle)
            g_signal_handler_disconnect(m_gvolume, m_volumeRemovedHandle);
    }
    if (m_cancellable) {
        g_cancellable_cancel(m_cancellable);
        g_object_unref(m_cancellable);
        m_cancellable = nullptr;
    }
    if (m_gvolume) {
        g_object_unref(m_gvolume);
        m_gvolume = nullptr;
    }

    if(m_watcher){
        m_watcher->stopMonitor();
        delete m_watcher;
    }
}

QString ComputerVolumeItem::getDevicePath(GVolume* volume)
{
    char *deviceName;
    QString unixDeviceName;
    deviceName = g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if(deviceName){
        unixDeviceName = QString(deviceName);
        g_free(deviceName);
    }
    return unixDeviceName;
}

void ComputerVolumeItem::updateInfoAsync()
{
    if (!m_volume) {
        m_icon = QIcon::fromTheme("drive-harddisk-system");
        m_uri = "file:///";
        m_displayName = tr("System Disk");
        auto file = g_file_new_for_uri("file:///");
        g_file_query_filesystem_info_async(file, "*", 0, m_cancellable,
                                           GAsyncReadyCallback(query_root_info_async_callback), this);
        return;
    }

    char *deviceName;

    // Volume may be in a transient invalid state (remove/add) when label changes.
    if (!m_gvolume || !G_IS_VOLUME(m_gvolume)) {
        return;
    }

    m_displayName = m_volume ? m_volume->name() : QString();
    //Handle the Chinese name of fat32 udisk.
    deviceName = g_volume_get_identifier(m_gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if(deviceName){
        m_unixDeviceName = QString(deviceName);
        qDebug()<<"unix Device Name"<< m_unixDeviceName;
        Peony::FileUtils::handleVolumeLabelForFat32(m_displayName, m_unixDeviceName);
        g_free(deviceName);
    }

    //fix u-disk show as hard-disk icon issue, task#25343
    updateBlockIcons();
    //qDebug()<<m_displayName;

    check();

    auto mount = g_volume_get_mount(m_gvolume);
    if (mount) {
        // g_volume_get_mount() returns a new reference; let Peony::Mount own and unref it.
        m_mount = std::make_shared<Peony::Mount>(mount, true);
        auto active_root = g_mount_get_root(mount);
        if (active_root) {
            auto uri = g_file_get_uri(active_root);
            if (uri) {
                m_uri = uri;
                g_free(uri);
            }
            if (m_uri == "file:///data") {
                if (Peony::FileUtils::isFileExsit("file:///data/usershare")) {
                    m_isHidden = true;
                }
            }
            g_file_query_filesystem_info_async(active_root, "*",
                                               0, m_cancellable, GAsyncReadyCallback(query_info_async_callback), this);
            g_object_unref(active_root);
        }
    } else {
        //m_mount = nullptr;
        //mount first
        //FIXME: check auto mount
//        this->mount();
    }

    bool isData = false;
    Peony::GlobalFstabData *globalFstabData = Peony::GlobalFstabData::getInstance();
    if(!globalFstabData->getUuidState()){
        if(globalFstabData->isMountPoints(m_unixDeviceName.toUtf8(), "/data")){
            isData = true;
        }
    }else{
        if(globalFstabData->isMountPoints(getDeviceUUID(m_unixDeviceName.toUtf8()), "/data")){
            isData = true;
        }
    }

    if(m_uri == "file:///data" || isData){
        m_displayName = tr("Data");
    }

    auto index = this->itemIndex();
    m_model->dataChanged(index, index);
    m_model->invalidateRequest();
}

const QString ComputerVolumeItem::displayName()
{
    return m_displayName;
}

const QIcon ComputerVolumeItem::icon()
{
    return m_icon.isNull()? AbstractComputerItem::icon(): m_icon;
}

bool ComputerVolumeItem::isMount()
{
    return Peony::FileUtils::isMountPoint(m_uri);
}

void ComputerVolumeItem::findChildren()
{
    //add root
    auto root = new ComputerVolumeItem(nullptr, m_model, this);

    //enumerate
    auto volume_monitor = g_volume_monitor_get();
    auto current_volumes = g_volume_monitor_get_volumes(volume_monitor);
    GList *l = current_volumes;
    while (l) {
        auto volume = G_VOLUME(l->data);
        QString devicePath = getDevicePath(volume);
        if (!devicePath.isEmpty()) {
            bool isVfatLoop = IMAGE_MOUNT_MANAGER->isVfatPartition(devicePath);
            qDebug() << "findChildren()" << __LINE__ << devicePath << "isVfatLoop:" << isVfatLoop;
            if (isVfatLoop) {
                l = l->next;
                continue;
            }
        }

        auto item = new ComputerVolumeItem(volume, m_model, this);
        l = l->next;
    }

    //comment gparted process code to fix duplicated volume issue, bug#41623
//    if(!current_volumes)
//        findChildrenWhenGPartedOpen();

    //monitor
    auto volumeManager = Peony::VolumeManager::getInstance();
    connect(volumeManager, &Peony::VolumeManager::volumeAdded, this, &ComputerVolumeItem::onVolumeAdded);

    //watcher
//    m_watcher = new Peony::FileWatcher("computer:///",this);
//    connect(m_watcher, &Peony::FileWatcher::fileCreated, this, &ComputerVolumeItem::onFileAdded);
//    connect(m_watcher, &Peony::FileWatcher::fileDeleted, this, &ComputerVolumeItem::onFileRemoved);
//    m_watcher->startMonitor();

    if (Peony::FileUtils::isFileExsit("file:///data/usershare")) {
        new ComputerUserShareItem(nullptr, m_model, this);
    }
}

void ComputerVolumeItem::check()
{
    if (!m_volume)
        return;

    if (!m_gvolume || !G_IS_VOLUME(m_gvolume))
        return;

    auto active_root = g_volume_get_activation_root(m_gvolume);
    if (active_root) {
        auto uri = g_file_get_uri(active_root);
        auto path = g_file_get_path(active_root);
        if (QString(uri) == "file:///data") {
            if (Peony::FileUtils::isFileExsit("file:///data/usershare")) {
                m_isHidden = true;
            }
        }
        //QMessageBox::information(0, 0, QString("%1 has active root %2").arg(m_displayName).arg(uri));
        qDebug() << "ComputerVolumeItem uri:" <<uri;
        if (uri) {
            /*!
              \bug
              when a volume added, it could have active root represond with a uri,
              such as mtp://, etc...
              however it could not get the local path of active root correctly at the
              time. that makes some volumes location changement meet troubles.
              */
            m_uri = uri;

            g_free(uri);
        }

        if (path) {
            //try to fix bug#174666, part phone mtp mode access wrong issue
            //m_uri = QString("file://%1").arg(path);
            //fix task#355901, access one plus phone error issue
            m_uri = g_filename_to_uri(path, nullptr, nullptr);
            qDebug() << "path:" <<path <<" m_uri:"<<m_uri;
            g_free(path);
        }
        g_object_unref(active_root);
    }

    if (m_uri.isNull()) {
        GFile* file = nullptr;
        GMount* mount = g_volume_get_mount(m_gvolume);
        if (nullptr != mount) {
            file = g_mount_get_root (mount);
            if (nullptr != file) {
                m_uri = g_file_get_uri(file);
            }
        }
        if (nullptr != file) {
            g_object_unref(file);
        }
        if (nullptr != mount) {
            g_object_unref (mount);
        }
    }
}

bool ComputerVolumeItem::canEject()
{
    GVolume *gvolume;
    GDrive  *gdrive;
    bool ejectAble = false;

    if("file:///" == m_uri || m_volume == nullptr)    /*The root File System cannot eject*/
        return false;

    if (m_gvolume && G_IS_VOLUME(m_gvolume)) {
        gvolume = (GVolume*)g_object_ref(m_gvolume);
        gdrive = g_volume_get_drive(gvolume);
        if(gdrive){
            //qDebug() <<"uri ejectAble:"<<m_uri<<g_drive_can_eject(gdrive)<<g_drive_can_stop(gdrive)<<g_drive_is_removable(gdrive);
            //try to fix bug#149182, SD card option not the same with side bar menu
            ejectAble = g_drive_can_eject(gdrive) || g_drive_can_stop(gdrive) /*|| g_drive_is_removable(gdrive)*/;
            g_object_unref(gdrive);
        }
        g_object_unref(gvolume);
    }

    return ejectAble;
}

void ComputerVolumeItem::eject(GMountUnmountFlags ejectFlag)
{
    GVolume *g_volume;
    GMount *g_mount;
    QString ejectNotify = QObject::tr("It need to synchronize before operating the device,place wait!");
    g_autoptr(GMountOperation) mount_op = g_mount_operation_new();

    if (tryStopDrive(ejectFlag, mount_op)) {
        return;
    }

    /*If udisk device is mounted,eject it from here*/
    if (m_mount && (g_mount = m_mount->getGMount())) {
        if (g_mount_can_eject(g_mount) && !m_unixDeviceName.startsWith("/dev/sd")) {/* 由于要显示异常U盘，U盘弹出用stop */
            Peony::SyncThread::notifyUser(ejectNotify);
            g_mount_eject_with_operation(g_mount, ejectFlag, mount_op, m_cancellable,
                                         GAsyncReadyCallback(eject_async_callback), this);
        } else {
            auto g_drive = g_mount_get_drive(g_mount);
            if (!g_drive)
                return;

            if (g_drive_can_stop(g_drive)|| (g_drive_is_removable(g_drive) && !m_unixDeviceName.startsWith("/dev/mmc"))){// for mobile harddisk.
                /* 加"(g_drive_is_removable(m_drive) && !m_device.startsWith("/dev/mmc"))"这个判断是为了解决bug#184111和bug#149182；有些U盘的can-stop为false,其中为一款sd卡的devicename */
                Peony::SyncThread::notifyUser(ejectNotify);
                g_drive_stop(g_mount_get_drive(g_mount), ejectFlag, mount_op, nullptr, GAsyncReadyCallback(stop_async_callback), this);/*m_cancellable改为nullptr，linkto bug145072【HWE】弹出成功后提示操作被取消 */
            }else if(g_drive_is_removable(g_drive)){
                //fix bug#141782, SD card eject can not recgonize issue
                g_mount_eject_with_operation(g_mount, ejectFlag, mount_op, m_cancellable,
                                             GAsyncReadyCallback(eject_async_callback), this);
            }

            g_object_unref(g_drive);
        }

        return;
    }

    /*If udisk device is unmounted,eject it from here*/
    if (m_gvolume && G_IS_VOLUME(m_gvolume) && (g_volume = m_gvolume)) {
        if (g_volume_can_eject(g_volume)) {
            Peony::SyncThread::notifyUser(ejectNotify);
            g_volume_eject_with_operation(g_volume, ejectFlag, mount_op, m_cancellable,
                                         GAsyncReadyCallback(eject_async_callback), this);
        } else {
            auto g_drive = g_mount_get_drive(g_mount);
            if (!g_drive)
                return;

            if (g_drive_can_stop(g_drive)){
                Peony::SyncThread::notifyUser(ejectNotify);
                g_drive_stop(g_mount_get_drive(g_mount), ejectFlag, mount_op, nullptr, GAsyncReadyCallback(stop_async_callback), this);/*m_cancellable改为nullptr，linkto bug145072【HWE】弹出成功后提示操作被取消 */
            }

            g_object_unref(g_drive);
        }
    }
}

bool ComputerVolumeItem::canUnmount()
{
    if(m_uri.endsWith(".mount") || m_uri.endsWith(".volume"))
        return true;
    if("file:///data" == m_uri)
        return false;
    return m_mount != nullptr;
}

bool ComputerVolumeItem::unMountIsoImage(GMount* gmount)
{
    GVolume *g_volume = nullptr;

    bool result = false;
    g_volume = g_mount_get_volume(gmount);
    if(g_volume) {
        g_autofree char *device_identifier = g_volume_get_identifier(g_volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (device_identifier) {
            QString devicePath = QString(device_identifier);
            if(devicePath.startsWith("/dev/loop") && !m_uri.contains("OfflineSource")) {
                result = IMAGE_MOUNT_MANAGER->unmountImage(devicePath);
                if (result) {
                    qDebug() << QString("unmount successed, devicePath: %1").arg(devicePath);
                } else {
                    qDebug() << QString("unmount failed, devicePath: %1").arg(devicePath);
                }
            }
        }
        g_object_unref(g_volume);
    }

    return result;
}

void ComputerVolumeItem::unmount(GMountUnmountFlags unmountFlag)
{
    GMount *g_mount = nullptr;
    GFile *file = nullptr;
    GVolume *g_volume = nullptr;

    g_autoptr(GMountOperation) mount_op = g_mount_operation_new();

    m_vfs_uri = m_model->m_volumeTargetMap.key(m_uri);
    QString unmountNotify = QObject::tr("It need to synchronize before operating the device,place wait!");
    if (!m_vfs_uri.isEmpty()) {
        file = g_file_new_for_uri(m_vfs_uri.toUtf8().constData());
        if(file){
            Peony::SyncThread::notifyUser(unmountNotify);
            g_file_unmount_mountable_with_operation(file,unmountFlag,
                                                    mount_op,nullptr,
                                                    GAsyncReadyCallback(unmount_async_callback),
                                                    this);
        }
        g_object_unref(file);
    } else if (m_mount && (g_mount = m_mount->getGMount())) {
        Peony::SyncThread::notifyUser(unmountNotify);
        if (unMountIsoImage(g_mount))
            return;
        g_mount_unmount_with_operation(g_mount, unmountFlag, mount_op, m_cancellable,
                                       GAsyncReadyCallback(unmount_async_callback), this);
    } else if (!m_uri.isEmpty()) {
        file = g_file_new_for_uri(m_uri.toUtf8().constData());
        if(file){
            Peony::SyncThread::notifyUser(unmountNotify);
            g_file_unmount_mountable_with_operation(file,unmountFlag,
                                                    mount_op,nullptr,
                                                    GAsyncReadyCallback(unmount_async_callback),
                                                    this);
        }
        g_object_unref(file);
    }
}

void ComputerVolumeItem::mount()
{
    if (m_uri == "file:///") {
        return;
    }

    if (m_mount) {
        auto root = g_mount_get_root(m_mount->getGMount());
        if (root) {
            auto uri = g_file_get_uri(root);
            if (uri) {
                m_uri = uri;
                g_free(uri);
            }
            g_file_query_filesystem_info_async(root, "*",
                                               0, m_cancellable, GAsyncReadyCallback(query_info_async_callback), this);
            g_object_unref(root);
        }
    } else {
        if (!m_gvolume || !G_IS_VOLUME(m_gvolume)) {
            return;
        }
        auto volume = Experimental_Peony::Volume(m_gvolume);
        volume.mount();
//        g_autoptr (GMountOperation) op = g_mount_operation_new();
//        g_volume_mount(m_volume->getGVolume(),
//                       G_MOUNT_MOUNT_NONE,
//                       op,
//                       m_cancellable,
//                       GAsyncReadyCallback(mount_async_callback),
//                       this);
    }
}

QModelIndex ComputerVolumeItem::itemIndex()
{
    if (!m_parentNode) {
        return m_model->createItemIndex(0, this);
    } else {
        return m_model->createItemIndex(m_parentNode->m_children.indexOf(this), this);
    }
}

bool ComputerVolumeItem::isHidden()
{
    return m_isHidden;
}

QString ComputerVolumeItem::getDeviceUUID(const char *device)
{
    struct stat statbuf;
    if (stat (device, &statbuf) != 0)
    {
        return nullptr;
    }

    g_autoptr(UDisksClient) client = udisks_client_new_sync (NULL,NULL);
    g_autoptr(UDisksBlock) block = udisks_client_get_block_for_dev(client, statbuf.st_rdev);
    if (!block)
        return nullptr;

    const gchar *uuid = udisks_block_get_id_uuid(block);
    return uuid;
}

void ComputerVolumeItem::volume_changed_callback(GVolume *volume, ComputerVolumeItem *p_this)
{
    //QMessageBox::information(0, 0, tr("Volume Changed"));
    if(!p_this)
        return;
    p_this->m_mount = nullptr;
    p_this->m_uri = nullptr;
    p_this->m_icon = QIcon();
    p_this->m_displayName = nullptr;
    p_this->m_usedSpace = 0;
    p_this->m_totalSpace = 0;
    p_this->updateInfo();
}

void ComputerVolumeItem::volume_removed_callback(GVolume *volume, ComputerVolumeItem *p_this)
{
    auto parentNode = p_this->m_parentNode;
    if (!parentNode)
        return;

    // Stop using the volume immediately to avoid UAF in any pending callbacks.
    if (p_this->m_gvolume) {
        if (p_this->m_volumeChangedHandle)
            g_signal_handler_disconnect(p_this->m_gvolume, p_this->m_volumeChangedHandle);
        if (p_this->m_volumeRemovedHandle)
            g_signal_handler_disconnect(p_this->m_gvolume, p_this->m_volumeRemovedHandle);
        g_object_unref(p_this->m_gvolume);
        p_this->m_gvolume = nullptr;
    }
    p_this->m_volume = nullptr;
    p_this->m_mount = nullptr;
    if (p_this->m_cancellable)
        g_cancellable_cancel(p_this->m_cancellable);

    auto row = parentNode->m_children.indexOf(p_this);
    parentNode->m_model->beginRemoveItem(parentNode->itemIndex(), row);
    parentNode->m_children.removeAt(row);
    p_this->deleteLater();
    parentNode->m_model->endRemoveItem();
}

void ComputerVolumeItem::mount_changed_callback(GVolumeMonitor *volumeMonitor, GMount *gmount, ComputerVolumeItem *p_this)
{
    if (p_this){
        p_this->m_usedSpace = 0;
        p_this->m_totalSpace = 0;
        p_this->updateInfo();
        qDebug()<<"mount changed uri: "<<p_this->uri()<<p_this->m_unixDeviceName<<p_this->m_bMtpQueryInfoFailed;

        /* hotfix bug#247327 【文件管理器】使用USB连接小米/红米手机，选择传输文件后，计算机界面不显示总容量和已用容量
         * hotfix bug#260993 【文件管理器】从任务栏打开文件管理器-计算机，使用USB连接小米手机，选择传输文件后，计算机界面不显示总容量和已用容量 */
        if(p_this && p_this->m_unixDeviceName.startsWith("/dev/bus/usb")){
            if(p_this->m_totalSpace > 0){
                return;
            }
            qDebug()<<"mtp mount changed"<<p_this->m_unixDeviceName<<p_this->uri();
            p_this->m_bMtpQueryInfoFailed = true;
            // Delay 100ms before calling updateInfo() to ensure thread safety
            // Capture p_this by value to ensure object is still valid when timer fires
            QTimer::singleShot(100, p_this, [p_this]() {
                if (p_this) {
                    p_this->updateInfo();
                }
            });
        }//end
    }
}

void ComputerVolumeItem::mount_added_callback(GVolumeMonitor *volumeMonitor, GMount *gmount, ComputerVolumeItem *p_this)
{
    if (p_this){
        p_this->updateInfo();
        qDebug()<<"mount added uri: "<<p_this->uri();
    }
}

QString iconFileFromMountpoint(const QString& mountpoint){
    bool isReadOnly;
    QDir mountDir;
    QString iconPath;
    GUnixMountEntry* entry;

    if(mountpoint.isEmpty())
        return iconPath;

    entry = g_unix_mount_for(mountpoint.mid(7).toUtf8().constData(),NULL);
    if(entry){
       isReadOnly = g_unix_mount_is_readonly(entry);
       g_unix_mount_free(entry);
       if(!isReadOnly)//is not a boot disk
           return iconPath;
    }

    mountDir.setPath(mountpoint.mid(7));//remove 'file://'
    if(mountDir.exists()){
        QStringList filters;
        filters << "*.ico";
        mountDir.setNameFilters(filters);

        QFileInfoList list = mountDir.entryInfoList();
        if(0 != list.length())
            iconPath = list.at(0).absoluteFilePath();
	}

    return iconPath;
}

bool isMountReadOnly(const QString& mountpoint)
{
    bool isReadOnly = false;
    GUnixMountEntry* entry;

    if(mountpoint.isEmpty() || mountpoint.length() < 8)
        return false;

    entry = g_unix_mount_for(mountpoint.mid(7).toUtf8().constData(),NULL);
    if(entry){
       isReadOnly = g_unix_mount_is_readonly(entry);
       g_unix_mount_free(entry);
    }

    qDebug() << "isMountReadOnly:"<<mountpoint<<isReadOnly;
    return isReadOnly;
}

void ComputerVolumeItem::query_info_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    auto info = g_file_query_filesystem_info_finish(file, res, &err);
    if (info) {
        /* hotfix bug#247327、260993 手机信息查询，容量显示问题 */
        if(p_this->m_unixDeviceName.startsWith("/dev/bus/usb") && p_this->m_bMtpQueryInfoFailed){
            p_this->m_bMtpQueryInfoFailed = false;
        }//end
        char *fs_type = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
        QString fsType(fs_type);
        /*
        *由于对DVD+RW或者是DVD-RW类型的光盘，无法通过cdromdata类获取到使用的容量，所以使用的容量统一使用gio函数获取
        */
        quint64 used = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
        bool bNotDisk = true;
        //ignore disk update when the disk deviece is busy,related to bug#143293
        if (p_this->m_unixDeviceName.startsWith("/dev/sr") && ! Peony::FileUtils::isBusyDevice(p_this->m_unixDeviceName)) {
#ifdef KY_UDF_BURN
            UdfBurn::DataCDROM *cdrom = new UdfBurn::DataCDROM(p_this->m_unixDeviceName);
#else
            Peony::DataCDROM *cdrom = new Peony::DataCDROM(p_this->m_unixDeviceName);
#endif
            if (cdrom) {
                cdrom->getCDROMInfo();
                //p_this->m_usedSpace = used;
                //used无法正确获取追加刻录后光盘的使用容量，getCDROMUsedCapacity()无法获取可擦除光盘的使用容量
                p_this->m_usedSpace = cdrom->getCDROMUsedCapacity();
                if((cdrom->getCDROMType()).contains("DVD+RW") || (cdrom->getCDROMType()).contains("DVD-RW")){
                    if(fsType.toLower().startsWith("iso")){/* "iso9660"文件系统类型"DVD+RW"和"DVD-RW"光盘的使用容量从libkyudfburn中获取,link bug#271455. */
                        p_this->m_usedSpace = cdrom->getCDROMUsedCapacity();
                    }else{
                        p_this->m_usedSpace  = used;
                    }
                }
#ifdef  KY_SDK_DISKINFO
                p_this->m_totalSpace = kdk_udf_get_cdrom_capacity(p_this->m_unixDeviceName.toUtf8().constData());
                p_this->m_usedSpace = kdk_udf_get_cdrom_used_capacity(p_this->m_unixDeviceName.toUtf8().constData());
#else
                p_this->m_totalSpace = cdrom->getCDROMCapacity();
#endif
                delete cdrom;
                cdrom = nullptr;
                bNotDisk = false;
            }
        }

        //fix block not update volume issue, link to bug#63326
        if (bNotDisk || 0 == p_this->m_totalSpace) {
            quint64 total = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
            quint64 free = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
#ifdef KY_SDK_DISKINFO
            if (!p_this->m_unixDeviceName.isNull() && !p_this->m_unixDeviceName.isEmpty()) {
                char *tmpTotalSize = kdk_get_hard_disk_size(p_this->m_unixDeviceName.toUtf8().constData());
                if (tmpTotalSize) {
                    quint64 totalSize = atoi(tmpTotalSize);
                    std::free(tmpTotalSize);
                    quint64 freeSize = kdk_get_partition_available_space(p_this->m_unixDeviceName.toUtf8().constData());
                    if (totalSize > 0 && freeSize > 0) {
                        totalSize = totalSize * 1024 * 1024;
                        freeSize = freeSize * 1024 * 1024;
                        total = totalSize;
                        free = freeSize;
                    }
                }
            }
#endif

            if (total > 0 && (used > 0 || free > 0)) {
                if (used > 0 && used <= total) {
                    p_this->m_usedSpace = used;
                    p_this->m_totalSpace = total;
                } else if(free > 0 && free <= total) {
                    p_this->m_usedSpace = total - free;
                    p_this->m_totalSpace = total;
                }

                p_this->updateBlockIcons();
             }
            if (fsType.contains("ext")) {
                p_this->m_usedSpace = total - free;
            }
         }

        qWarning()<<"udisk name"<<p_this->m_volume->name();
        qWarning()<<"udisk used space"<<p_this->m_usedSpace;
        qWarning()<<"udisk total space"<<p_this->m_totalSpace;
        /***************************collect info when gparted open*************************/
//        if(p_this->m_icon.name().isEmpty()){
//            QString iconName = Peony::FileUtils::getFileIconName(p_this->m_uri);
//            if(iconName.isNull())               /*Some startup disks cannot get the icon*/
//                iconName = "drive-harddisk-usb";
//            p_this->m_icon = QIcon::fromTheme(iconName);
//        }
//        if(p_this->m_displayName.isEmpty()){
//            p_this->m_displayName = Peony::FileUtils::getFileDisplayName(p_this->m_uri);

//            if(!p_this->m_targetUri.isEmpty()){
//               char *realMountPoint = g_filename_from_uri(p_this->m_targetUri.toUtf8().constData(),NULL,NULL);
//               const char *unixDev = Peony::VolumeManager::getUnixDeviceFileFromMountPoint(realMountPoint);
//               QString unixDeviceName = unixDev;
//               Peony::FileUtils::handleVolumeLabelForFat32(p_this->m_displayName,unixDeviceName);
//               g_free(realMountPoint);
//            }
//        }
        /**********************************************************************************/

        auto index = p_this->itemIndex();
        p_this->m_model->dataChanged(index, index);
        //p_this->m_model->dataChanged(p_this->itemIndex(), p_this->itemIndex());

        g_object_unref(info);

    }
    if (err) {
        /* related bug(304129、247327、260993) */
        qDebug()<<"Fail to query filesystem info,error code:"<<err->code<<", error message:"<<err->message;
        //try to fix bug#526767, errorcode is 18 or 19, error info not match
        //comment this code to avoid crash issue, should not call in call back function, will cause thread safety issue
//        if ((err->code == G_IO_ERROR_CANCELLED) || (err->code == G_IO_ERROR_CLOSED) && (QString(err->message).startsWith("No such interface"))) {
//            if(p_this && p_this->m_bMtpQueryInfoFailed && !p_this->m_unixDeviceName.isNull() && p_this->m_unixDeviceName.startsWith("/dev/bus/usb")){
//                qDebug()<<"Failure in query mobile phone information, retrying.";
//                p_this->updateInfoAsync();
//            }
//        }//end if
        g_error_free(err);
    }
}

void ComputerVolumeItem::query_root_info_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    GFileInfo *info = g_file_query_info_finish(file, res, &err);
    if (info) {
        quint64 total = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
        quint64 used = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
        quint64 available = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
        QString unixDevice = getRootUnixDevice(p_this->uri().split("file://").last());
#ifdef KY_SDK_DISKINFO
            if (!unixDevice.isNull() && !unixDevice.isEmpty()) {
                char *tmpTotalSize = kdk_get_hard_disk_size(unixDevice.toUtf8().constData());
                if (tmpTotalSize) {
                    quint64 totalSize = atoi(tmpTotalSize);
                    free(tmpTotalSize);
                    quint64 freeSize = kdk_get_partition_available_space(unixDevice.toUtf8().constData());
                    if (totalSize > 0 && freeSize > 0) {
                        totalSize = totalSize * 1024 * 1024;
                        freeSize = freeSize * 1024 * 1024;
                        total = totalSize;
                        available = freeSize;
                    }
                }
            }
#endif
        char *fs_type = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
        QString type(fs_type);
        if(type.contains("ext")) {
            used = total - available;
        }

        p_this->m_totalSpace = total;
        p_this->m_usedSpace = used;
        auto index = p_this->itemIndex();
        p_this->m_model->dataChanged(index, index);
        //p_this->m_model->dataChanged(p_this->itemIndex(), p_this->itemIndex());

        g_object_unref(info);
    }
    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        g_error_free(err);
    }
}

void ComputerVolumeItem::mount_async_callback(GVolume *volume, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    bool successed = g_volume_mount_finish(volume, res, &err);
    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        if (g_error_matches(err, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)) {
            Experimental_Peony::Volume v(volume);
            v.mount();
        }
        g_error_free(err);
    }

    if (successed) {
        QString unmountNotify = QObject::tr("The device has been mount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
        p_this->updateInfoAsync();
    }
}

void ComputerVolumeItem::unmount_async_callback(GObject* object,GAsyncResult *res,ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    QString errorMsg;
    bool successed;

    if(G_IS_MOUNT(object)){
        if(successed)
             p_this->m_mount = nullptr;
        successed = g_mount_unmount_with_operation_finish(G_MOUNT(object),res,&err);
    }else if(G_IS_FILE(object)){
        successed = g_file_unmount_mountable_with_operation_finish(G_FILE(object),res,&err);
    }

    if(successed){
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);

        // 检查是否需要继续执行完整的弹出操作（针对加密设备）
        if (p_this->property("pendingFullEject").toBool()) {
            qDebug() << "unmount_async_callback: Continuing with full device ejection after unmounting encrypted volume";
            p_this->setProperty("pendingFullEject", false);

            // 获取之前保存的ejection flags
            GMountUnmountFlags ejectFlag = static_cast<GMountUnmountFlags>(
                p_this->property("ejectionFlags").toInt());

            // 延迟一点时间让系统完成卸载操作
            QTimer::singleShot(100, [p_this, ejectFlag]() {
                p_this->eject(ejectFlag);
            });
        }
    }

    if(err){
        errorMsg = err->message;
        if(strstr(err->message,"target is busy")){
            errorMsg = QObject::tr("One or more programs prevented the unmount operation.");
            QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n").arg(errorMsg), QMessageBox::Yes);
        }else if(strstr(err->message,"umount: /media/")){
            //chinese name need to be converted,this may be a error that from glib2/gio2.
            errorMsg = QObject::tr("Unable to unmount it, you may need to close some programs, such as: GParted etc.");
            QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("%1").arg(errorMsg), QMessageBox::Ok);
        } else if (err->code == G_IO_ERROR_PERMISSION_DENIED || errorMsg.contains("authorized", Qt::CaseInsensitive)) {
            // do nothing because we have requested polkit dialog yet.
        } else {
            QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n").arg(err->message), QMessageBox::Ok);
        }

        g_error_free(err);
    }
}

void ComputerVolumeItem::eject_async_callback(GObject *object, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    bool successed;

    if(G_IS_MOUNT(object))
        successed = g_mount_eject_with_operation_finish(G_MOUNT(object), res, &err);
    else if(G_IS_VOLUME(object))
        successed = g_volume_eject_with_operation_finish(G_VOLUME(object), res, &err);

    if (successed) {
        //QMessageBox::information(0, 0, "Volume Ejected");
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
    }
    if (err) {
        qDebug()<<"error code of drive stop:"<<err->code<<", error message:"<<err->message;
        QString errMsg = err->message;
        if (!errMsg.contains("authorized", Qt::CaseInsensitive)) {
            //fix bug#104482, pull device immediately when eject it, error message not translated issue
            if (err->code == G_IO_ERROR_FAILED){
                errMsg = QObject::tr("Eject device failed, the reason may be that the device has been removed, etc.");
            }
            QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Eject failed"), errMsg, QMessageBox::Ok);
            warningBox.exec();
        }

        g_error_free(err);
    }
}

void ComputerVolumeItem::stop_async_callback(GDrive *drive, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    bool successed;

    successed = g_drive_stop_finish(drive, res, &err);
    if (!successed) {
        qDebug()<<"error code of drive stop:"<<err->code<<", error message:"<<err->message;
        if(!strcmp(err->message, "Not authorized to perform operation")){/* gmountOperation会弹出授权框，防止二次弹框,linkto bug#101075 */
            g_error_free(err);
            return;
        }

        //fix bug#104482, pull device immediately when eject it, error message not translated issue
        QString errMessage = err->message;
        if (err->code == G_IO_ERROR_FAILED){
            errMessage = QObject::tr("Eject device failed, the reason may be that the device has been removed, etc.");
        }
        QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Eject failed"), errMessage, QMessageBox::Ok);
        warningBox.exec();
        g_error_free(err);
    } else {
        QString ejectNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(ejectNotify);
    }
}

void ComputerVolumeItem::onVolumeAdded(const std::shared_ptr<Peony::Volume> volume)
{
    auto g_volume = volume->getGVolume();

    QString devicePath = getDevicePath(g_volume);
    if (!devicePath.isEmpty()) {
        bool isVfatLoop = IMAGE_MOUNT_MANAGER->isVfatPartition(devicePath);
        qDebug() << "onVolumeAdded" << __LINE__ << devicePath << "isVfatLoop:" << isVfatLoop;
        if (isVfatLoop) {
            return;
        }
    }

    if (Peony::FileUtils::isMountMatchFstab(g_volume, "/backup")) {
        return;
    }
    auto item = new ComputerVolumeItem(g_volume, m_model, this);
}

/*
 * 根据反馈的bug做逻辑调整：
 * 1.默认使用 m_volume->iconName() 图标；
 * 2.只读系统，系统盘优先使用gmount icon，关联bug#120459(删掉第二条，系统盘图标不特殊处理，不使用gmount icon，linkto bug#174770)
 * 3.需要区分部分型号的U盘显示成移动硬盘图标问题，关联bug#182809, #57660, #70014 #96652
 * 4.在Windows系统使用工具，比如软碟通制作的系统盘，不是只读格式，获取iconName后缀带'.ico'做判断
 * */
void ComputerVolumeItem::updateBlockIcons()
{
    //qDebug() << "updateBlockIcons volume icon:"<<m_volume->iconName()<<m_uri;
    if (!m_gvolume || !G_IS_VOLUME(m_gvolume) || !m_volume) {
        return;
    }

    m_icon = QIcon::fromTheme(m_volume->iconName());

    // 显示加密分区角标
    GVolume *volume = m_gvolume;
    if (volume) {
        g_autoptr (GIcon) volumeIcon = g_volume_get_icon(volume);
        if (G_IS_EMBLEMED_ICON(volumeIcon)) {
            GEmblemedIcon *emblemedIcon = G_EMBLEMED_ICON (volumeIcon);
            GList *l = g_emblemed_icon_get_emblems(emblemedIcon);
            if (g_list_length (l) > 0) {
                GEmblem *emblem = G_EMBLEM (l->data);
                GIcon *emblemIcon = g_emblem_get_icon(emblem);
                QString emblemIconName = Peony::FileUtils::getIconStringFromGIcon(emblemIcon);
                setProperty("emblemIconName", emblemIconName);
            } else {
                setProperty("emblemIconName", QVariant());
            }
        } else {
            setProperty("emblemIconName", QVariant());
        }
    }

    GDrive* gdrive = g_volume_get_drive(m_gvolume);
    QString tmpDevice;
    if(gdrive){
        g_autofree char* gdevice = g_drive_get_identifier(gdrive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        tmpDevice = gdevice;
        g_object_unref(gdrive);
    }

    //iso image files are mounted on loop devices, always use the "media-optical" icon, link to #358341
    if (m_unixDeviceName.startsWith("/dev/loop")) {
        m_icon = QIcon::fromTheme("media-optical");
        return;
    }

    if(tmpDevice.startsWith("/dev/sr") && m_volume->iconName().startsWith("media-optical")){/* 规范光盘图标，与竞品对比，光盘图标使用的是"media-optical",linkto bug#174770、#120459 */
        m_icon = QIcon::fromTheme("media-optical");
        return;
    }

    //fix bug#182809, U disk show mobile-SSD icon issue, only readonly device use gmount icon
    if (m_volume->iconName() == "drive-harddisk-usb"){
        //用设备容量区分U盘和移动硬盘，大于128GiB为移动硬盘
        //FIXME with a better solution, fix bug#57660, #70014 #96652
        double size = 0.0;
        if(!tmpDevice.isEmpty()){
            size = Peony::FileUtils::getDeviceSize(tmpDevice.toUtf8().constData());
        }
        if (m_totalSpace/(1024 * 1024 *1024) > 128 || size > 128)
            m_icon = QIcon::fromTheme("drive-harddisk-usb");
        else
            m_icon = QIcon::fromTheme("drive-removable-media-usb");
    }
}

/* Usage scenarios:
 *   -> open Peony first,then open Gparted.
 *   -> when computer:///xxx.mount or computer:///xxx.volume is generated,
 *      it will query their information from here.
 */
//comment gparted process code to fix duplicated volume issue, bug#41623
//void ComputerVolumeItem::collectInfoWhenGpartedOpen(QString uri)
//{
//    GFile *file = NULL;

//    m_uri = uri;
//    m_displayName = "";
//    m_icon = QIcon();
//    m_cancellable = g_cancellable_new();
//    m_targetUri = Peony::FileUtils::getTargetUri(m_uri);
//    file = g_file_new_for_uri(m_targetUri.toUtf8().constData());

//    g_file_query_filesystem_info_async(file,"*",0,
//                            m_cancellable,
//                            GAsyncReadyCallback(qeury_info_async_callback),this);
//    g_object_unref(file);
//}

/* monitor file generation for computer:///xxx.mount or computer:///xxx.volume
 */
//void ComputerVolumeItem::onFileAdded(const QString &uri)
//{
//    QString targetUri = Peony::FileUtils::getTargetUri(uri);
//    if(!targetUri.contains("file:///") || targetUri.isEmpty())
//        return;

//    for (auto item : m_children) {
//        qDebug() << "ComputerVolumeItem onFileAdded uri:"<<uri<<item->uri();
//        if (item->uri() == uri)
//            return;
//    }

//    m_model->beginInsertItem(itemIndex(), m_children.count());
//    auto item = new ComputerVolumeItem(uri, m_model, this);
//    m_children<<item;
//    m_model->endInsterItem();
//}

//void ComputerVolumeItem::onFileRemoved(const QString &uri)
//{
//    int row = -1;
//    for (auto item : m_children) {
//        qDebug() << "ComputerVolumeItem onFileRemoved uri:"<<uri<<item->uri();
//        if (item->uri() == uri) {
//            row = m_children.indexOf(item);
//            break;
//        }
//    }

//    if (row < 0)
//        return;

//    m_model->beginRemoveItem(itemIndex(), row);
//    auto item = m_children.takeAt(row);
//    item->deleteLater();
//    m_model->endRemoveItem();
//}

/* Usage scenarios:
 *   -> open Gparted first,then open Peony.
 *   -> when Gparted is open,it is used to find all partition devices
 *      except the root partition.
 */
//void ComputerVolumeItem::findChildrenWhenGPartedOpen()
//{
//    GFile *file;

//    file = g_file_new_for_uri("computer:///");
//    m_tmpCancellable = g_cancellable_new();
//    g_file_enumerate_children_async(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 0,
//                                    m_tmpCancellable, GAsyncReadyCallback(enumerate_async_callback), this);
//    //g_object_unref(file);
//}

//void ComputerVolumeItem::enumerate_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this)
//{
//    GError *err = nullptr;
//    auto enumerator = g_file_enumerate_children_finish(file, res, &err);
//    if (enumerator) {
//        g_file_enumerator_next_files_async(enumerator, 9999, 0, p_this->m_tmpCancellable,
//                                           GAsyncReadyCallback(find_children_async_callback), p_this);
//    }
//    if (err)
//        g_error_free(err);
//}

//void ComputerVolumeItem::find_children_async_callback(GFileEnumerator *enumerator, GAsyncResult *res, ComputerVolumeItem *p_this)
//{
//    GError *err = nullptr;
//    auto infos = g_file_enumerator_next_files_finish(enumerator, res, &err);
//    GList *l = infos;
//    while (l) {
//        auto info = G_FILE_INFO(l->data);
//        l = l->next;
//        if (!info)
//            continue;

//        auto file = g_file_enumerator_get_child(enumerator, info);
//        if (!file)
//            continue;

//        auto uri = g_file_get_uri(file);
//        if (!uri)
//            continue;

//        //not include ftp://xxx etc.
//        QString targetUri;
//        targetUri = Peony::FileUtils::getTargetUri(uri);
//        if(targetUri.isEmpty())
//            continue;
//        if("file:///" == targetUri)
//            continue;
//        if(!targetUri.contains("file:///"))
//            continue;

//        p_this->m_model->beginInsertItem(p_this->itemIndex(), p_this->m_children.count());
//        auto item = new ComputerVolumeItem(uri, p_this->m_model, p_this);
//        qDebug() << "find_children_async_callback uri:" <<uri<<targetUri;
//        p_this->m_children<<item;
//        p_this->m_model->endInsterItem();
//        g_free(uri);
//        g_object_unref(file);
//    }

//    if (infos)
//        g_list_free_full(infos, g_object_unref);

//    if (enumerator) {
//        g_file_enumerator_close(enumerator, nullptr, nullptr);
//        g_object_unref(enumerator);
//    }

//    if(p_this->m_tmpCancellable){
//        g_cancellable_cancel(p_this->m_tmpCancellable);
//        g_object_unref(p_this->m_tmpCancellable);
//    }

//    if (err) {
//        g_error_free(err);
//    }
//}

quint64 calcVolumeCapacity(ComputerVolumeItem* pThis)
{
    char* tmpDevice;
    GVolume* gVolume;
    quint64 capacityBytes;
    QString unixDevice,dbusPath;

    if(!pThis->m_mount && pThis->m_targetUri.isEmpty())
        return 0;

    if(pThis->m_mount){
        if (pThis->m_gvolume && G_IS_VOLUME(pThis->m_gvolume)) {
            gVolume = pThis->m_gvolume;
            tmpDevice = g_volume_get_identifier(gVolume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            if (tmpDevice) {
                unixDevice = QString(tmpDevice+5);
                //fix crash issue
                if (tmpDevice)
                   g_free(tmpDevice);
            } else {
                unixDevice = Peony::FileUtils::getUnixDevice(pThis->m_uri);
                unixDevice = unixDevice.section('/',-1);
            }
        }
    }else{
        unixDevice = Peony::FileUtils::getUnixDevice(pThis->m_uri);
        unixDevice = unixDevice.section('/',-1);
    }

    if (unixDevice.isEmpty()) {
        return 0;
    }

    dbusPath = "/org/freedesktop/UDisks2/block_devices/" + unixDevice;
    QDBusInterface blockInterface("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.UDisks2.Block",
                                  QDBusConnection::systemBus());

    capacityBytes = 0;
    if(blockInterface.isValid())
        capacityBytes = blockInterface.property("Size").toULongLong();

    return capacityBytes;
}

bool ComputerVolumeItem::tryStopDrive(GMountUnmountFlags ejectFlag, GMountOperation *mount_op)
{
    bool stopped = false;

    qDebug() << "tryStopDrive: Starting for device" << m_unixDeviceName << "uri:" << m_uri;

    if (m_unixDeviceName.startsWith("/dev/sr")) {
        return false;
    }

    // 检查是否为加密设备
    bool isEncryptedAndMounted = false;
    if ((m_unixDeviceName.startsWith("/dev/dm-") || m_unixDeviceName.contains("dm-"))) {
        isEncryptedAndMounted = true;
        qDebug() << "tryStopDrive: Detected mounted encrypted device (dm device)";
    }

    // 对于已挂载的加密设备，先卸载
    if (isEncryptedAndMounted) {
        qDebug() << "tryStopDrive: Attempting to unmount encrypted volume first";

        // 设置属性，以便在卸载回调中继续完整弹出流程
        setProperty("pendingFullEject", true);
        setProperty("ejectionFlags", static_cast<int>(ejectFlag));

        unmount(ejectFlag);
        return true;
    }

    // 尝试从mount获取drive
    GDrive *g_drive = nullptr;
    if (m_mount && m_mount->getGMount()) {
        g_drive = g_mount_get_drive(m_mount->getGMount());
        qDebug() << "tryStopDrive: Got drive from mount:" << (g_drive ? "yes" : "no");
    } else {
        qDebug() << "tryStopDrive: No valid mount available";
    }

    if (!g_drive && m_gvolume && G_IS_VOLUME(m_gvolume)) {
        g_drive = g_volume_get_drive(m_gvolume);
        qDebug() << "tryStopDrive: Got drive from volume:" << (g_drive ? "yes" : "no");
    } else if (!m_volume) {
        qDebug() << "tryStopDrive: No valid volume available";
    }

    if (g_drive) {
        bool can_stop = g_drive_can_stop(g_drive);
        bool is_removable = g_drive_is_removable(g_drive);
        bool is_mmc = m_unixDeviceName.startsWith("/dev/mmc");

        qDebug() << "tryStopDrive: Drive capabilities - can_stop:" << can_stop
                 << "is_removable:" << is_removable
                 << "is_mmc:" << is_mmc;

        char *drive_name = g_drive_get_name(g_drive);
        qDebug() << "tryStopDrive: Drive name:" << (drive_name ? drive_name : "unknown");
        if (drive_name) g_free(drive_name);

        qDebug() << "tryStopDrive: Has media:" << g_drive_has_media(g_drive)
                 << "Is media removable:" << g_drive_is_media_removable(g_drive);

        if (can_stop || (is_removable && !is_mmc)) {
            qDebug() << "tryStopDrive: Attempting to stop drive";
            QString ejectNotify = QObject::tr("It need to synchronize before operating the device,place wait!");
            Peony::SyncThread::notifyUser(ejectNotify);
            g_drive_stop(g_drive, ejectFlag, mount_op, nullptr, GAsyncReadyCallback(stop_async_callback), this);
            stopped = true;
            qDebug() << "tryStopDrive: Stop operation initiated";
        } else {
            qDebug() << "tryStopDrive: Drive cannot be stopped with current conditions";
        }
    } else {
        qDebug() << "tryStopDrive: No drive found for device" << m_unixDeviceName;
    }

    if (g_drive) {
        g_object_unref(g_drive);
    }

    qDebug() << "tryStopDrive: Result for" << m_unixDeviceName << ":" << (stopped ? "stop attempted" : "no action taken");
    return stopped;
}
