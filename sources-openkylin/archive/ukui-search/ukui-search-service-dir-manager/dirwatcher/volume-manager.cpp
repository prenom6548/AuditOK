/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "volume-manager.h"
#include <mutex>
#include <QMutexLocker>
#include <QDebug>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#endif
static std::once_flag flag;
static VolumeManager *global_instance = nullptr;

Volume::Volume()
{
}

Volume::Volume(const QString& device)
{
    m_device = device;
}

QString Volume::device()
{
    return m_device;
}

QStringList Volume::duplicatesMountPoints() const
{
    QStringList tmp;
    for (auto iter = m_mountPoints.constBegin(); iter != m_mountPoints.constEnd(); ++iter) {
        if(iter.value().isEmpty()) {
            tmp.append(iter.key());
        }
    }
    if(tmp.size() == 1) {
        tmp.clear();
    }
    return tmp;
}

QMap<QString, QString> Volume::subVolumes() const
{
    QMap<QString, QString> tmp;
    for (auto iter = m_mountPoints.constBegin(); iter != m_mountPoints.constEnd(); ++iter) {
        if(!iter.value().isEmpty()) {
            tmp.insert(iter.key(), iter.value());
        }
    }
    return tmp;
}

QStringList Volume::mountPoints() const
{
    return m_mountPoints.keys();
}

void Volume::setDevice(const QString& device)
{
    m_device = device;
}

void Volume::addMountPoint(const QString &mountPoint, const QString &subVolume)
{
    m_mountPoints.insert(mountPoint, subVolume);
}

VolumeManager *VolumeManager::self()
{
    std::call_once(flag, [ & ] {
        global_instance = new VolumeManager();
    });
    return global_instance;
}

VolumeManager::VolumeManager(QObject *parent)
    : QObject{parent}
{
    m_volumeMonitor = g_volume_monitor_get();
    if (!m_volumeMonitor) {
        qDebug() << "Fail to init volume monitor";
        return;
    }
    this->refresh();

    m_mountAddHandle = g_signal_connect(m_volumeMonitor, "mount-added", G_CALLBACK(mountAddCallback), this);
    m_mountRemoveHandle = g_signal_connect(m_volumeMonitor, "mount-removed", G_CALLBACK(mountRemoveCallback), this);
}

VolumeManager::~VolumeManager()
{
    if(m_volumeMonitor){
        g_signal_handler_disconnect(m_volumeMonitor, m_mountAddHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_mountRemoveHandle);
        g_object_unref(m_volumeMonitor);
        m_volumeMonitor = nullptr;
    }
}

QVector<QStringList> VolumeManager::getDuplicates()
{
    QMutexLocker locker(&m_mutex);
    QVector<QStringList> tmp;
    for (auto iter = m_volumes.constBegin(); iter != m_volumes.constEnd(); ++iter) {
        QStringList duplicates = iter.value().duplicatesMountPoints();
        if(!duplicates.isEmpty()) {
            tmp.append(duplicates);
        }
    }
    return tmp;

}

QVector<Volume> VolumeManager::volumesHaveSubVolumes()
{
    QMutexLocker locker(&m_mutex);
    QVector<Volume> tmp;
    for (auto iter = m_volumes.constBegin(); iter != m_volumes.constEnd(); ++iter) {
        if(!iter.value().subVolumes().isEmpty()) {
            tmp.append(iter.value());
        }
    }
    return tmp;
}

QVector<Volume> VolumeManager::volumes()
{
    QMutexLocker locker(&m_mutex);
    return m_volumes.values().toVector();
}

void VolumeManager::refresh()
{
    QMutexLocker locker(&m_mutex);
    m_volumes.clear();
    m_uDiskVolumes.clear();
    //遍历当前系统所有挂载设备
    for (const QStorageInfo &storage: QStorageInfo::mountedVolumes()) {
        if (storage.isValid() and storage.isReady()) {
            Volume volume(storage.device());
            if(m_volumes.contains(storage.device())) {
                volume = m_volumes.value(storage.device());
            }
            volume.addMountPoint(storage.rootPath(), storage.subvolume());
            m_volumes.insert(storage.device(), volume);
        }
    }
    GList* list = g_volume_monitor_get_volumes(m_volumeMonitor);
    if (!list) {
        qWarning() << "Fail to get volume list";
        m_uDiskVolumes.clear();
    } else {
        for (auto i = 0; i < g_list_length(list); i++) {
            GVolume* volume = (GVolume*)g_list_nth_data(list, i);
            QString device = g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            bool canEject = g_volume_can_eject(volume) || g_drive_can_stop(g_volume_get_drive(volume));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (device.contains(QRegularExpression("/nvme[0-9]+n[0-9]+(p[0-9]+)?$|/sd[a-z][0-9]*$"))) {
#else
            if (device.contains(QRegExp("/nvme[0-9]+n[0-9]+(p[0-9]+)?$|/sd[a-z][0-9]*$"))) {
#endif
                if (m_volumes.contains(device) && canEject) {
                    m_uDiskVolumes.insert(device, m_volumes.value(device));
                }
            }

        }
    }
    Q_EMIT VolumeDataUpdated();
}

void VolumeManager::mountAddCallback(GVolumeMonitor *monitor, GMount *gmount, VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    QString mountPoint = getMountPoint(gmount);
    Q_EMIT pThis->MountAdded(mountPoint);
    int uDiskCount = pThis->uDiskVolumes().size();
    //TODO 识别U盘等移动设备
    pThis->refresh();
    if (pThis->uDiskVolumes().size() > uDiskCount) {
        Q_EMIT pThis->UDiskMounted(mountPoint);
    }
}

void VolumeManager::mountRemoveCallback(GVolumeMonitor *monitor, GMount *gmount, VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    const QString mountPoint = getMountPoint(gmount);
    const int uDiskCount = pThis->uDiskVolumes().size();
    pThis->refresh();
    if (pThis->uDiskVolumes().size() < uDiskCount) {
        Q_EMIT pThis->UDiskUnmounted(mountPoint);
    }
}

QString VolumeManager::getMountPoint(GMount *gmount) {
    QString mountPoint;
    if (const auto rootFile = g_mount_get_root(gmount)) {
        auto gMountPoint = g_file_get_uri(rootFile);
        mountPoint = gMountPoint;
        g_free(gMountPoint);
        if(!(mountPoint.startsWith("smb://"))){
            gMountPoint = g_filename_from_uri(mountPoint.toUtf8().constData(), nullptr, nullptr);
            mountPoint = gMountPoint;
            g_free(gMountPoint);
        }
        g_object_unref(rootFile);
    }
    return mountPoint;
}

QVector<Volume> VolumeManager::uDiskVolumes() const {
    return m_uDiskVolumes.values().toVector();
}

