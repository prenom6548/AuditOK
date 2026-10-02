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

#include "computer-proxy-model.h"
#include "computer-model.h"
#include "abstract-computer-item.h"
#include "global-settings.h"
#include "manager-control.h"

#include <QApplication>
#include <QDir>
#include <QDebug>

#include <gio/gunixmounts.h>
#include <gudev/gudev.h>

static ComputerProxyModel *global_instance = nullptr;

static bool getRawDeviceIsEncrypted(const QString &mapperDevice)
{
    if (mapperDevice.startsWith("/dev/dm") || mapperDevice.startsWith("/dev/mapper")) {
        g_autoptr (GUdevClient) client = g_udev_client_new(nullptr);
        g_return_val_if_fail(client, false);
        g_autoptr (GUdevDevice) device = g_udev_client_query_by_device_file(client, mapperDevice.toLocal8Bit().constData());
        g_return_val_if_fail(device, false);
        QString sysfsPath = g_udev_device_get_sysfs_path(device);
        QString rawDeviceLinkDirPath = sysfsPath + "/slaves";
        QDir dir(rawDeviceLinkDirPath);
        auto results = dir.entryList(QDir::NoDotAndDotDot|QDir::AllEntries);
        if (!results.isEmpty()) {
            QString result = results.first();
            if (!result.startsWith("dm")) {
                result.prepend("/dev/");
                g_autoptr (GUdevDevice) rootDevice = g_udev_client_query_by_device_file(client, result.toLocal8Bit().constData());
                QString fsUsage = g_udev_device_get_property(rootDevice, "ID_FS_USAGE");
                if (fsUsage.contains("crypto")) {
                    return true;
                } else {
                    return false;
                }
            } else {
                qWarning() << "failed to get raw device file for" << result;
                result.prepend("/dev/");
                if (result != mapperDevice) {
                    // 处理嵌套映射场景，如加密分区套lvm逻辑卷的场景，关联bug#
                    return getRawDeviceIsEncrypted(result);
                }
                return false;
            }
        }
        return false;
    } else {
        return false;
    }
}

ComputerProxyModel::ComputerProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    auto systemMount = g_unix_mount_at("/", nullptr);
    if (systemMount) {
        QString devPath = g_unix_mount_get_device_path(systemMount);
        if (devPath.startsWith("/dev/mapper")) {
            bool isSystemEncrypted = getRawDeviceIsEncrypted(devPath);
            qApp->setProperty("isSystemEncrypted", isSystemEncrypted);
        }
    }

    auto dataMount = g_unix_mount_at("/data", nullptr);
    if (dataMount) {
        QString devPath = g_unix_mount_get_device_path(dataMount);
        if (devPath.startsWith("/dev/mapper")) {
            bool isDataEncrypted = getRawDeviceIsEncrypted(devPath);
            qApp->setProperty("isDataEncrypted", isDataEncrypted);
        }
    }

    setDynamicSortFilter(true);
    auto computerModel = new ComputerModel(this);
    m_model = computerModel;
    setSourceModel(computerModel);

    m_locale = QLocale(QLocale::system().name());
    m_collator = QCollator(m_locale);
    m_collator.setNumericMode(true);

    m_show_optical_drive = MANAGER_CONTROL->checkOpticalDrivePermission();

    connect(m_model, &ComputerModel::updateLocationRequest, this, &ComputerProxyModel::updateLocationRequest);
    connect(m_model, &ComputerModel::invalidateRequest, this, &ComputerProxyModel::invalidateFilter);
    connect(m_model, &ComputerModel::updateRequest, this, &ComputerProxyModel::updateRequest);
    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [=](const QString &key){
        if (key == ENABLE_OPTICAL_DRIVE) {
            m_show_optical_drive = MANAGER_CONTROL->checkOpticalDrivePermission();
            invalidateFilter();
        }
    });
}

ComputerProxyModel *ComputerProxyModel::globalInstance()
{
    if (!global_instance) {
        global_instance = new ComputerProxyModel;
    }
    return global_instance;
}

AbstractComputerItem *ComputerProxyModel::itemFromIndex(const QModelIndex &proxyIndex)
{
    auto index = mapToSource(proxyIndex);
    return static_cast<AbstractComputerItem *>(index.internalPointer());
}

QString ComputerProxyModel::tryGetVolumeUriFromMountTarget(const QString &mountTargetUri)
{
    return m_model->tryGetVolumeUriFromMountRoot(mountTargetUri);
}

QString ComputerProxyModel::tryGetVolumeRealUriFromUri(const QString &uri)
{
    return m_model->tryGetVolumeRealUriFromUri(uri);
}

void ComputerProxyModel::refresh()
{
    m_model->refresh();
}

bool ComputerProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto item = static_cast<AbstractComputerItem *>(m_model->index(source_row, 0, source_parent).internalPointer());
    if (!m_show_optical_drive && item->itemType() == AbstractComputerItem::Volume) {
        if (item->m_unixDeviceName.contains("/dev/sr")) {
            return false;
        }
    }
    return !item->isHidden();
}

bool ComputerProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (!source_left.parent().isValid() && source_right.parent().isValid()) {
        return sortOrder() == Qt::AscendingOrder ? false : true;
    }

    if (!(source_left.isValid() && source_right.isValid())) {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }

    auto leftItem = static_cast<AbstractComputerItem*>(source_left.internalPointer());
    auto rightItem = static_cast<AbstractComputerItem*>(source_right.internalPointer());
    if (leftItem->itemType() != AbstractComputerItem::Volume || rightItem->itemType() != AbstractComputerItem::Volume) {
        return false;
    }

    if (leftItem->uri().compare("file:///") == 0) {
        return true;
    }
    if (rightItem->uri().compare("file:///") == 0) {
        return false;
    }

    if (leftItem->uri().compare("computer:///ukui-data-volume") == 0 || leftItem->uri().compare("file:///data") == 0) {
        return true;
    }
    if (rightItem->uri().compare("computer:///ukui-data-volume") == 0 || rightItem->uri().compare("file:///data") == 0) {
        return false;
    }

    if (!leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/bus/usb") && !leftItem->m_unixDeviceName.startsWith("/dev/loop")
            && !rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/bus/usb") && !rightItem->m_unixDeviceName.startsWith("/dev/loop")) {
        return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
    } else if (!leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/bus/usb") && !leftItem->m_unixDeviceName.startsWith("/dev/loop")) {
        return true;
    } else if (!rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/bus/usb") && !rightItem->m_unixDeviceName.startsWith("/dev/loop")) {
        return false;
    }

    if ((leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/sr"))
            && (rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/sr"))) {
        return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
    } else if (leftItem->canEject() && !leftItem->m_unixDeviceName.startsWith("/dev/sr")) {
        return true;
    } else if (rightItem->canEject() && !rightItem->m_unixDeviceName.startsWith("/dev/sr")) {
        return false;
    }

    if (leftItem->m_unixDeviceName.startsWith("/dev/bus/usb") && rightItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
    } else if (leftItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return true;
    } else if (rightItem->m_unixDeviceName.startsWith("/dev/bus/usb")) {
        return false;
    }
    return m_collator.compare(leftItem->m_unixDeviceName, rightItem->m_unixDeviceName) < 0;
}
