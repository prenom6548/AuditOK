#include "volume-manager.h"

#include <QMutexLocker>

namespace {
VolumeManager *testVolumeManagerInstance = nullptr;
QVector<QStringList> forcedDuplicatesForTest;
}

void setForcedDuplicatesForTest(const QVector<QStringList> &duplicates)
{
    forcedDuplicatesForTest = duplicates;
}

void clearForcedDuplicatesForTest()
{
    forcedDuplicatesForTest.clear();
}

Volume::Volume() = default;

Volume::Volume(const QString &device)
    : m_device(device)
{
}

QString Volume::device()
{
    return m_device;
}

QStringList Volume::duplicatesMountPoints() const
{
    QStringList result;
    for (auto iter = m_mountPoints.constBegin(); iter != m_mountPoints.constEnd(); ++iter) {
        if (iter.value().isEmpty()) {
            result.append(iter.key());
        }
    }
    if (result.size() == 1) {
        result.clear();
    }
    return result;
}

QMap<QString, QString> Volume::subVolumes() const
{
    QMap<QString, QString> result;
    for (auto iter = m_mountPoints.constBegin(); iter != m_mountPoints.constEnd(); ++iter) {
        if (!iter.value().isEmpty()) {
            result.insert(iter.key(), iter.value());
        }
    }
    return result;
}

QStringList Volume::mountPoints() const
{
    return m_mountPoints.keys();
}

void Volume::setDevice(const QString &device)
{
    m_device = device;
}

void Volume::addMountPoint(const QString &mountPoint, const QString &subVolume)
{
    m_mountPoints.insert(mountPoint, subVolume);
}

VolumeManager *VolumeManager::self()
{
    if (!testVolumeManagerInstance) {
        testVolumeManagerInstance = new VolumeManager();
    }
    return testVolumeManagerInstance;
}

VolumeManager::VolumeManager(QObject *parent)
    : QObject(parent)
{
}

VolumeManager::~VolumeManager() = default;

QVector<QStringList> VolumeManager::getDuplicates()
{
    QMutexLocker locker(&m_mutex);
    if (!forcedDuplicatesForTest.isEmpty()) {
        return forcedDuplicatesForTest;
    }
    QVector<QStringList> result;
    for (auto iter = m_volumes.constBegin(); iter != m_volumes.constEnd(); ++iter) {
        const QStringList duplicates = iter.value().duplicatesMountPoints();
        if (!duplicates.isEmpty()) {
            result.append(duplicates);
        }
    }
    return result;
}

QVector<Volume> VolumeManager::volumesHaveSubVolumes()
{
    QMutexLocker locker(&m_mutex);
    QVector<Volume> result;
    for (auto iter = m_volumes.constBegin(); iter != m_volumes.constEnd(); ++iter) {
        if (!iter.value().subVolumes().isEmpty()) {
            result.append(iter.value());
        }
    }
    return result;
}

QVector<Volume> VolumeManager::volumes()
{
    QMutexLocker locker(&m_mutex);
    return m_volumes.values().toVector();
}

void VolumeManager::refresh()
{
    Q_EMIT VolumeDataUpdated();
}

void VolumeManager::mountAddCallback(GVolumeMonitor *monitor, GMount *gmount, VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    const QString mountPoint = getMountPoint(gmount);
    Q_EMIT pThis->MountAdded(mountPoint);
    Q_EMIT pThis->VolumeDataUpdated();
}

void VolumeManager::mountRemoveCallback(GVolumeMonitor *monitor, GMount *gmount, VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    const QString mountPoint = getMountPoint(gmount);
    Q_EMIT pThis->VolumeDataUpdated();
    Q_EMIT pThis->UDiskUnmounted(mountPoint);
}

QString VolumeManager::getMountPoint(GMount *gmount)
{
    Q_UNUSED(gmount)
    return QString();
}

QVector<Volume> VolumeManager::uDiskVolumes() const
{
    return m_uDiskVolumes.values().toVector();
}
