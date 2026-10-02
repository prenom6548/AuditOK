/*
 * Peony-Qt
 *
 * Copyright (C) 2024, Tianjin KYLIN Information Technology Co., Ltd.
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
 * @author: Renyg <renyangguang@kylinos.cn>
 * @date:   2024-12-26 12:22
 */

#ifndef IMAGEMOUNTMANAGER_H
#define IMAGEMOUNTMANAGER_H

#include "peony-core_global.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <QDBusInterface>
#include <QDBusConnection>
#include <memory>

namespace Peony {

/**
 * @brief The ImageMountManager class handles mounting and unmounting of disk image files
 * @details This class provides functionality to mount and unmount disk image files (e.g., ISO files)
 * using UDisks2 D-Bus interface. It manages loop devices and mount points.
 */
class PEONYCORESHARED_EXPORT ImageMountManager : public QObject
{
    Q_OBJECT
public:
    friend class QScopedPointer<ImageMountManager>;

    /**
     * @brief Get the singleton instance of ImageMountManager
     * @return Pointer to the ImageMountManager instance
     */
    static ImageMountManager* getInstance();
    ~ImageMountManager();

    /**
     * @brief Mount a disk image file
     * @param imagePath Path or URI to the image file
     * @return true if mounting succeeded, false otherwise
     */
    bool mountImage(const QString& imagePath);

    /**
     * @brief Unmount a mounted image
     * @param path Path to either the image file or the loop device
     * @return true if unmounting succeeded, false otherwise
     */
    bool unmountImage(const QString& path);

    /**
     * @brief Convert device path to UDisks2 D-Bus interface path
     * @param path Device path (e.g., /dev/loop0)
     * @return D-Bus interface path
     */
    static QString devicePathToDbusInterfacePath(const QString& path);

    static QString dbusInterfacePathToDevicePath(const QString& dbusPath);

    /**
     * @brief Check if a file is a mountable disk image
     * @param uri URI or path to the file
     * @return true if the file is a disk image, false otherwise
     */
    static bool isImageFile(const QString& uri);

    void cleanInvalidMounts();

    static bool isPartition(const QString& devicePath);
    bool isVfatFilesystem(const QString& devicePath);
    bool isVfatPartition(const QString& devicePath);

Q_SIGNALS:
    /**
     * @brief Emitted when a device is successfully mounted
     * @param mountPoint Path where the device is mounted
     */
    void deviceMounted(const QString& mountPoint);

    /**
     * @brief Emitted when a device is successfully unmounted
     * @param devicePath Path of the unmounted device
     */
    void deviceUnmounted(const QString& devicePath);

    /**
     * @brief Emitted when an error occurs during mount/unmount operations
     * @param errorMessage Description of the error
     */
    void mountError(const QString& errorMessage);

private:
    /**
     * @brief Structure to hold information about mounted images
     */
    struct MountInfo {
        QString devicePath;     ///< Loop device path
        QString mountPoint;     ///< Mount point directory
        QString imageFilePath;  ///< Original image file path
        QString partitionPath;  ///< Path to the mounted partition if the image has partitions (e.g., "/org/freedesktop/UDisks2/block_devices/loop0p1")
                                ///< Empty if the image doesn't have partitions
    };

    explicit ImageMountManager(QObject *parent = nullptr);
    static QScopedPointer<ImageMountManager> instance;

    bool setupLoop(const QString& imagePath, QString& loopDevicePath, QString& errMsg);
    bool mountLoopDevice(const QString& dbusPath, const QString& imagePath, QString& mountPoint, QString& errMsg);
    bool deleteLoopDevice(const QString& dbusPath, QString& errMsg);
    QString getLoopDevicePath(const QString& path, QString& errMsg);

    bool hasPartitionTable(const QString& dbusPath) const;
    QString findSuitablePartition(const QString& dbusPath, QString& errMsg);

    QString normalizeImagePath(const QString& imagePath);
    bool waitForFilesystemInterface(const QString& dbusPath, QString& errMsg);

    bool mountWithUdisksctl(const QString& devicePath, QString& mountPoint, QString& errMsg);
    bool saveMountInfo(const QString& loopDevicePath, const QString& imagePath, const QString& mountPoint, const QString& partitionPath);

    bool getMountPoints(const QString& dbusPath, QList<QString>& mountPoints) const;
    bool isMounted(const QString& dbusPath) const;
    bool findExistingMount(const QString& imagePath, QString& mountPoint);
    QList<QDBusObjectPath> getDevicePartitions(const QString& dbusPath, QString& errMsg) const;
    QString getLoopBackingFile(const QString& dbusPath, QString& errMsg) const;
    bool unmountFilesystem(const QString& dbusPath, QString& errMsg);
    bool isValidMount(const MountInfo& info);

private:
    QMap<QString, MountInfo> m_mountedImages;  ///< Map of mounted images
    QDBusInterface* m_udisks2Interface;        ///< UDisks2 D-Bus interface
    QDBusConnection m_systemBus;               ///< System D-Bus connection

    static const QStringList SUPPORTED_FILESYSTEMS;
};

#define IMAGE_MOUNT_MANAGER Peony::ImageMountManager::getInstance()
}

#endif // IMAGEMOUNTMANAGER_H
