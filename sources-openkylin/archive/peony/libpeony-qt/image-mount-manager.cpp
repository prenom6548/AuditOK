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
 * @date:   2024-12-26 12:35
 */

#include "image-mount-manager.h"
#include "file-info.h"

#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusUnixFileDescriptor>
#include <QFile>
#include <QThread>
#include <QProcess>

using namespace Peony;

const QStringList ImageMountManager::SUPPORTED_FILESYSTEMS = {
    "iso9660",    // 标准光盘文件系统
    "udf",        // 通用光盘格式，用于DVD和更新的光盘
    "hfsplus",    // macOS的文件系统
    "hfs",        // 较老的macOS文件系统
    "apfs",       // 较新的macOS文件系统
    "ext2",       // Linux文件系统
    "ext3",       // Linux文件系统
    "ext4",       // Linux文件系统
    "xfs",        // Linux文件系统
    "btrfs",      // Linux文件系统
    "ntfs",       // Windows文件系统
    //"fat32",      // FAT文件系统
    //"vfat",       // FAT文件系统变体
    //"exfat"       // FAT文件系统扩展版
};

QScopedPointer<ImageMountManager> ImageMountManager::instance(nullptr);

ImageMountManager* ImageMountManager::getInstance()
{
    if (!instance) {
        instance.reset(new ImageMountManager());
    }
    return instance.data();
}

ImageMountManager::ImageMountManager(QObject *parent)
    : QObject(parent)
    , m_systemBus(QDBusConnection::systemBus())
{
    // Initialize UDisks2 D-Bus interface
    m_udisks2Interface = new QDBusInterface(
        "org.freedesktop.UDisks2",
        "/org/freedesktop/UDisks2",
        "org.freedesktop.DBus.ObjectManager",
        m_systemBus,
        this
    );
}

ImageMountManager::~ImageMountManager(){}

bool ImageMountManager::mountImage(const QString& imagePath)
{
    QString mountPoint;
    if (findExistingMount(imagePath, mountPoint)) {
        Q_EMIT deviceMounted(mountPoint);
        return true;
    }

    QString normalizedPath = normalizeImagePath(imagePath);
    QString loopDevicePath;
    QString errMsg;

    // Setup loop device
    if (!setupLoop(normalizedPath, loopDevicePath, errMsg)) {
        Q_EMIT mountError(errMsg);
        return false;
    }

    QString dbusPath = devicePathToDbusInterfacePath(loopDevicePath);

    // Mount the loop device
    if (!mountLoopDevice(dbusPath, normalizedPath, mountPoint, errMsg)) {
        Q_EMIT mountError(errMsg);
        return false;
    }

    Q_EMIT deviceMounted(mountPoint);
    return true;
}

QString ImageMountManager::devicePathToDbusInterfacePath(const QString &path)
{
    if (path.startsWith("/dev/loop")) {
        QFileInfo fileInfo(path);
        return QString("/org/freedesktop/UDisks2/block_devices/%1").arg(fileInfo.fileName());
    }

    return path.startsWith("/org/freedesktop/UDisks2/block_devices") ? path : QString();
}

QString ImageMountManager::dbusInterfacePathToDevicePath(const QString& dbusPath)
{
    if (dbusPath.startsWith("/org/freedesktop/UDisks2/block_devices/")) {
        return QString("/dev/%1").arg(dbusPath.split("/").last());
    }
    return dbusPath.startsWith("/dev/") ? dbusPath : QString();
}

bool ImageMountManager::isImageFile(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    if (info->isDir() || info->isVolume()) {
        return false;
    }

    QString mimeType = info->mimeType();
    return mimeType == "application/x-iso9660-image" ||
           mimeType == "application/x-cd-image" ||
           uri.endsWith(".iso", Qt::CaseInsensitive);
}

bool ImageMountManager::unmountImage(const QString& path)
{
    QString errMsg;
    QString loopDevicePath = getLoopDevicePath(path, errMsg);
    if (loopDevicePath.isEmpty()) {
        Q_EMIT mountError(errMsg);
        return false;
    }

    // Get DBus path for the loop device
    QString dbusLoopPath = devicePathToDbusInterfacePath(loopDevicePath);
    if (dbusLoopPath.isEmpty()) {
        Q_EMIT mountError(tr("Invalid device path: %1").arg(loopDevicePath));
        return false;
    }

    // Unmount all filesystems (either partitions or the device itself)
    if (hasPartitionTable(dbusLoopPath)) {
        QString errMsg;
        QList<QDBusObjectPath> partitions = getDevicePartitions(dbusLoopPath, errMsg);
        for (const QDBusObjectPath& partition : partitions) {
            if (!unmountFilesystem(partition.path(), errMsg)) {
                Q_EMIT mountError(errMsg);
                return false;
            }
        }
    } else if (!unmountFilesystem(dbusLoopPath, errMsg)) {
        Q_EMIT mountError(errMsg);
        return false;
    }

    // Delete the loop device
    if (!deleteLoopDevice(dbusLoopPath, errMsg)) {
        Q_EMIT mountError(errMsg);
        return false;
    }

    // Cleanup mount info
    for (auto it = m_mountedImages.begin(); it != m_mountedImages.end(); ++it) {
        if (it.value().devicePath == loopDevicePath) {
            m_mountedImages.erase(it);
            break;
        }
    }

    //cleanInvalidMounts();

    Q_EMIT deviceUnmounted(loopDevicePath);
    return true;
}

QString ImageMountManager::getLoopDevicePath(const QString& path, QString& errMsg)
{
    QString devicePath = path;

    // Convert DBus path to device path
    if (devicePath.startsWith("/org/freedesktop/UDisks2/block_devices/")) {
        devicePath = QString("/dev/%1").arg(devicePath.split("/").last());
    }

    // Handle partition path
    QRegExp partitionRegex("^/dev/loop(\\d+)p(\\d+)$");
    if (partitionRegex.exactMatch(devicePath)) {
        return QString("/dev/loop%1").arg(partitionRegex.cap(1));
    }

    // Handle loop device path
    if (devicePath.startsWith("/dev/loop")) {
        return devicePath;
    }

    // Handle image file path
    QString normalizedPath = normalizeImagePath(devicePath);
    if (m_mountedImages.contains(normalizedPath)) {
        return m_mountedImages[normalizedPath].devicePath;
    }

    errMsg = tr("Cannot find loop device for path: %1").arg(devicePath);
    return QString();
}

bool ImageMountManager::unmountFilesystem(const QString& dbusPath, QString& errMsg)
{
    if (!isMounted(dbusPath)) {
        return true;  // Not mounted, consider success
    }

    QDBusInterface filesystemInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.UDisks2.Filesystem",
        m_systemBus
    );

    QDBusMessage unmountReply = filesystemInterface.call("Unmount",
        QVariant::fromValue(QVariantMap())
    );

    if (unmountReply.type() == QDBusMessage::ErrorMessage) {
        QString error = unmountReply.errorMessage();
        if (!error.contains("NotMounted")) {
            errMsg = tr("Failed to unmount filesystem: %1").arg(error);
            return false;
        }
    }
    return true;
}

bool ImageMountManager::deleteLoopDevice(const QString& dbusPath, QString& errMsg)
{
    QDBusInterface loopInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.UDisks2.Loop",
        m_systemBus
    );

    QDBusMessage deleteReply = loopInterface.call("Delete",
        QVariant::fromValue(QVariantMap())
    );

    if (deleteReply.type() == QDBusMessage::ErrorMessage) {
        errMsg = tr("Failed to delete loop device: %1").arg(deleteReply.errorMessage());
        return false;
    }
    return true;
}

bool ImageMountManager::getMountPoints(const QString& dbusPath, QList<QString>& mountPoints) const
{
    mountPoints.clear();
    QDBusInterface propsInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.DBus.Properties",
        m_systemBus
    );

    QDBusReply<QVariant> reply = propsInterface.call("Get",
        "org.freedesktop.UDisks2.Filesystem",
        "MountPoints"
    );

    if (!reply.isValid()) {
        return false;
    }

    const QDBusArgument arg = reply.value().value<QDBusArgument>();
    QList<QByteArray> mountPointsBytes;
    arg >> mountPointsBytes;

    // Convert QByteArray to QString
    for (const QByteArray& mountPoint : mountPointsBytes) {
        mountPoints.append(QString::fromUtf8(mountPoint));
    }

    return true;
}

bool ImageMountManager::isMounted(const QString& dbusPath) const
{
    QList<QString> mountPoints;
    return getMountPoints(dbusPath, mountPoints) && !mountPoints.isEmpty();
}

QString ImageMountManager::normalizeImagePath(const QString& imagePath)
{
    return imagePath.startsWith("file://") ? QUrl(imagePath).toLocalFile() : imagePath;
}

bool ImageMountManager::setupLoop(const QString& imagePath, QString& loopDevicePath, QString& errMsg)
{
    // Open image file
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errMsg = QString("Failed to open image file: %1").arg(imagePath);
        qWarning() << errMsg;
        return false;
    }

    // Setup UDisks2 manager interface
    QDBusInterface managerInterface(
        "org.freedesktop.UDisks2",
        "/org/freedesktop/UDisks2/Manager",
        "org.freedesktop.UDisks2.Manager",
        m_systemBus
    );

    // Prepare mount options
    QVariantMap options;
    options.insert("offset", QVariant(0ULL));
    options.insert("size", QVariant(0ULL));
    options.insert("read-only", QVariant(true));
    options.insert("auth.no_user_interaction", true);
    options.insert("setup-fs-type", "iso9660");

    // Setup loop device
    QDBusUnixFileDescriptor unixFd(file.handle());
    QDBusMessage reply = managerInterface.call(
        "LoopSetup",
        QVariant::fromValue(unixFd),
        QVariant::fromValue(options)
    );

    if (reply.type() == QDBusMessage::ErrorMessage) {
        errMsg = QString("Failed to setup loop device: %1").arg(reply.errorMessage());
        qWarning() << errMsg;
        return false;
    }

    QString dbusPath = reply.arguments()[0].value<QDBusObjectPath>().path();
    loopDevicePath = dbusInterfacePathToDevicePath(dbusPath);
    return true;
}

bool ImageMountManager::mountLoopDevice(const QString& dbusPath, const QString& imagePath, QString& mountPoint, QString& errMsg)
{
    bool withPartitionTable = hasPartitionTable(dbusPath);
    qDebug() << "Device has partition table:" << withPartitionTable;

    QString dbusPathToMount = dbusPath;
    if (withPartitionTable) {
        QString partitionPath = findSuitablePartition(dbusPath, errMsg);
        if (!partitionPath.isEmpty()) {
            dbusPathToMount = partitionPath;
        } else {
            qDebug() << "Failed to find suitable partition:" << errMsg;
        }
    }

    if (!waitForFilesystemInterface(dbusPathToMount, errMsg)) {
        return false;
    }

    QDBusInterface filesystemInterface(
        "org.freedesktop.UDisks2",
        dbusPathToMount,
        "org.freedesktop.UDisks2.Filesystem",
        m_systemBus
    );

    QVariantMap options;
    options["fstype"] = "auto";
    options["options"] = "ro";
    options["auth.no_user_interaction"] = true;

    QDBusMessage reply = filesystemInterface.call("Mount", QVariant::fromValue(options));

    if (reply.type() == QDBusMessage::ErrorMessage) {
        // DBus 挂载失败，尝试使用 udisksctl
        if (!mountWithUdisksctl(dbusPathToMount, mountPoint, errMsg)) {
            return false;
        }
    } else {
        mountPoint = reply.arguments().first().toString();
    }

    qDebug() << "Successfully mounted at:" << mountPoint;
    QString loopDevicePath = dbusInterfacePathToDevicePath(dbusPath);
    return saveMountInfo(loopDevicePath, imagePath, mountPoint, withPartitionTable ? dbusPathToMount : QString());
}

bool ImageMountManager::hasPartitionTable(const QString& dbusPath) const
{
    QDBusInterface introspectInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.DBus.Introspectable",
        m_systemBus
    );

    QDBusReply<QString> reply = introspectInterface.call("Introspect");
    if (reply.isValid()) {
        return reply.value().contains("org.freedesktop.UDisks2.PartitionTable");
    }
    return false;
}

QString ImageMountManager::findSuitablePartition(const QString& dbusPath, QString& errMsg)
{
    QList<QDBusObjectPath> partitions = getDevicePartitions(dbusPath, errMsg);
    if (partitions.isEmpty()) {
        return QString();
    }

    qint64 maxSize = 0;
    QString selectedPartition;

    for (const QDBusObjectPath& partition : partitions) {
        QDBusInterface blockInterface(
            "org.freedesktop.UDisks2",
            partition.path(),
            "org.freedesktop.DBus.Properties",
            m_systemBus
        );

        QDBusReply<QVariant> fsReply = blockInterface.call("Get",
            "org.freedesktop.UDisks2.Block",
            "IdType"
        );

        if (fsReply.isValid()) {
            QString fsType = fsReply.value().toString();
            qDebug() << "Partition" << partition.path() << "filesystem type:" << fsType;

            if (SUPPORTED_FILESYSTEMS.contains(fsType, Qt::CaseInsensitive)) {
                QDBusReply<QVariant> sizeReply = blockInterface.call("Get",
                    "org.freedesktop.UDisks2.Block",
                    "Size"
                );

                if (sizeReply.isValid()) {
                    qint64 size = sizeReply.value().toLongLong();
                    if (size > maxSize) {
                        maxSize = size;
                        selectedPartition = partition.path();
                    }
                }
            }
        }
    }

    if (selectedPartition.isEmpty()) {
        errMsg = "No suitable partition found";
        return QString();
    }

    return selectedPartition;
}

bool ImageMountManager::waitForFilesystemInterface(const QString& dbusPath, QString& errMsg)
{
    int retries = 30;
    while (retries--) {
        QDBusInterface introspectable(
            "org.freedesktop.UDisks2",
            dbusPath,
            "org.freedesktop.DBus.Introspectable",
            m_systemBus
        );

        QDBusReply<QString> reply = introspectable.call("Introspect");
        if (reply.isValid() && reply.value().contains("org.freedesktop.UDisks2.Filesystem")) {
            return true;
        }
        QThread::msleep(100);
    }

    errMsg = "Filesystem interface not available";
    return false;
}

bool ImageMountManager::mountWithUdisksctl(const QString& devicePath, QString& mountPoint, QString& errMsg)
{
    QString devPath = devicePath;
    if (devPath.startsWith("/org/freedesktop/UDisks2/block_devices/")) {
        devPath = dbusInterfacePathToDevicePath(devPath);
    }

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    qDebug() << "Trying to mount using udisksctl:" << devPath;

    process.start("udisksctl", QStringList() << "mount" << "-b" << devPath);
    if (!process.waitForFinished(3000)) {
        errMsg = "udisksctl timeout";
        return false;
    }

    QString output = QString::fromUtf8(process.readAll());
    qDebug() << "udisksctl output:" << output;

    if (process.exitCode() != 0) {
        errMsg = QString("udisksctl failed: %1").arg(output);
        return false;
    }

    QRegExp rx("at\\s+([^\\s]+)");
    if (rx.indexIn(output) != -1) {
        mountPoint = rx.cap(1);
        return true;
    }

    errMsg = "Could not parse mount point from udisksctl output";
    return false;
}

bool ImageMountManager::saveMountInfo(const QString& loopDevicePath, const QString& imagePath, const QString& mountPoint, const QString& partitionPath)
{
    MountInfo info;
    info.devicePath = loopDevicePath;
    info.mountPoint = mountPoint;
    if (!partitionPath.isEmpty()) {
        info.partitionPath = partitionPath;
    }
    m_mountedImages[imagePath] = info;
    return true;
}

bool ImageMountManager::findExistingMount(const QString& imagePath, QString& mountPoint)
{
    QString normalizedPath = normalizeImagePath(imagePath);

    cleanInvalidMounts();

    // First check internal cache
    if (m_mountedImages.contains(normalizedPath)) {
        mountPoint = m_mountedImages[normalizedPath].mountPoint;
        //return true;
    }

    // Check all loop devices via DBus
    QDBusMessage call = m_udisks2Interface->call("GetManagedObjects");
    if (call.type() == QDBusMessage::ErrorMessage) {
        return false;
    }

    const QDBusArgument arg = call.arguments().at(0).value<QDBusArgument>();
    QMap<QDBusObjectPath, QMap<QString, QMap<QString, QVariant>>> objects;
    arg >> objects;

    bool found = false;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        const QString& objPath = it.key().path();
        // Skip non-loop devices
        if (!objPath.contains("loop")) {
            continue;
        }

        QString errMsg;
        QString backingFile = getLoopBackingFile(objPath, errMsg);
        if (backingFile.isEmpty()) {
            continue;
        }

        // Process if it's either our target file or another ISO file
        if (backingFile == normalizedPath || isImageFile(backingFile)) {
            QString mountPath = objPath;
            QString partition;

            // Check for partition table and find suitable partition if present
            if (hasPartitionTable(objPath)) {
                partition = findSuitablePartition(objPath, errMsg);
                if (!partition.isEmpty()) {
                    mountPath = partition;
                }
            }

            // Get mount points and save mount information
            QList<QString> mountPoints;
            if (getMountPoints(mountPath, mountPoints) && !mountPoints.isEmpty()) {
                QString currentMountPoint = mountPoints.first();
                saveMountInfo(dbusInterfacePathToDevicePath(objPath), backingFile, currentMountPoint, partition);

                // Set return values if this is our target file
                if (backingFile == normalizedPath) {
                    mountPoint = currentMountPoint;
                    found = true;
                }
            }
        }
    }

    return found;
}

QString ImageMountManager::getLoopBackingFile(const QString& dbusPath, QString& errMsg) const
{
    QDBusInterface propsInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.DBus.Properties",
        m_systemBus
    );

    QDBusReply<QVariant> reply = propsInterface.call("Get",
        "org.freedesktop.UDisks2.Loop",
        "BackingFile"
    );

    if (!reply.isValid()) {
        errMsg = QString("Failed to get backing file: %1").arg(reply.error().message());
        return QString();
    }

    return reply.value().toString();
}

QList<QDBusObjectPath> ImageMountManager::getDevicePartitions(const QString& dbusPath, QString& errMsg) const
{
    QList<QDBusObjectPath> partitions;
    QDBusInterface propsInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.DBus.Properties",
        m_systemBus
    );

    QDBusReply<QVariant> reply = propsInterface.call("Get",
        "org.freedesktop.UDisks2.PartitionTable",
        "Partitions"
    );

    if (!reply.isValid()) {
        errMsg = QString("Failed to get partitions: %1").arg(reply.error().message());
        return partitions;
    }

    const QDBusArgument arg = reply.value().value<QDBusArgument>();
    arg >> partitions;
    return partitions;
}

bool ImageMountManager::isValidMount(const MountInfo& info)
{
    if (!QDir(info.mountPoint).exists()) {
        return false;
    }

    QDBusInterface blockInterface(
        "org.freedesktop.UDisks2",
        devicePathToDbusInterfacePath(info.devicePath),
        "org.freedesktop.DBus.Properties",
        m_systemBus
    );

    QDBusReply<QVariant> reply = blockInterface.call("Get",
        "org.freedesktop.UDisks2.Loop",
        "BackingFile"
    );

    return reply.isValid();
}

void ImageMountManager::cleanInvalidMounts()
{
    QMutableMapIterator<QString, MountInfo> i(m_mountedImages);
    while (i.hasNext()) {
        i.next();
        if (!isValidMount(i.value())) {
            qDebug() << "Removing invalid mount:" << i.value().imageFilePath;
            i.remove();
        }
    }
}

bool ImageMountManager::isPartition(const QString& devicePath)
{
    // 使用正则表达式匹配 /dev/loopNpN 格式
    static QRegExp partitionRegex("^/dev/loop\\d+p\\d+$");
    return partitionRegex.exactMatch(devicePath);
}

bool ImageMountManager::isVfatFilesystem(const QString& devicePath)
{
    QString dbusPath = devicePathToDbusInterfacePath(devicePath);

    QDBusInterface propsInterface(
        "org.freedesktop.UDisks2",
        dbusPath,
        "org.freedesktop.DBus.Properties",
        m_systemBus
    );

    // 获取文件系统类型
    QDBusReply<QVariant> reply = propsInterface.call("Get",
        "org.freedesktop.UDisks2.Block",
        "IdType"
    );

    if (!reply.isValid()) {
        return false;
    }

    QString fsType = reply.value().toString().toLower();
    return fsType == "vfat";
}

bool ImageMountManager::isVfatPartition(const QString& devicePath)
{
    return isPartition(devicePath) && isVfatFilesystem(devicePath);
}
