/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "partition_server.h"

#include <parted/parted.h>
#include <parted/device.h>
#include <QCollator>
#include "device.h"
#include <QDebug>
#include <QLibrary>
#include <QDir>
#include "libparted_unit.h"
#include "partition_usage.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/base_unit/file_unit.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/sysInfo/device_disk.h"
#include "../PluginService/sysInfo/mount.h"
#include "partition_unit.h"
#include "operationdisk.h"
#include "proberos.h"
#include "../PluginService/kdir.h"
#include <QMetaType>

namespace KInstaller {
using namespace KServer;
namespace Partman {

bool UnmountDevices()
{
    bool isOK;
    QString output, error;
    int exitcode = 0;
    QString ScriptPath =  KServer::GetKylinInstallPath() + "/scripts" ;

    isOK = KServer::KCommand::getInstance()->RunScripCommand("/bin/bash", {"./prepare/00umount"}, ScriptPath,  output, error, exitcode);
    if(!isOK) {
        qWarning() << "failed to unmount all devices" << output << error;
    }
    return isOK;
}


PartitionServer::PartitionServer(QObject *parent) : QObject(parent),
    m_blosprober(true)
{
    this->setObjectName("PartitionServer");
    qRegisterMetaType<DeviceList>("DeviceList");
    qRegisterMetaType<OperationDiskList>("OperatorDiskList");
    qRegisterMetaType<PartTableType>("PartTableType");

    this->initAllConnect();


}

PartitionServer::~PartitionServer()
{

}

void PartitionServer::initAllConnect()
{
    connect(this, &PartitionServer::signalCreatePartitionTable,
            this, &PartitionServer::doCreatePartitionTable);
    connect(this, &PartitionServer::signalRefreshDevices,
            this, &PartitionServer::doRefreshDevices);
    connect(this, &PartitionServer::signalQuickPart,
            this, &PartitionServer::doQuickPart);
    connect(this, &PartitionServer::signalCustomPart,
            this, &PartitionServer::doCustomPart);
}

bool PartitionServer::osproberDevice( QString &devPath)
{
    QString output = "";
    QString error = "";
    int exitcode = 0;
 //   QString program = "bash /usr/share/kylin-os-installer/data/os-prober";
 //   QStringList args = {devPath};
    QString ScriptPath =  KServer::GetKylinInstallPath() + "/data" ;

    bool isOK;
    isOK = KServer::KCommand::getInstance()->RunScripCommand("/bin/bash", {"./os-prober", devPath}, ScriptPath,  output, error, exitcode);
    return isOK;
}

void osproberDmsetup()
{
    QString output = "";
    QString error = "";
    int exitcode = 0;
    QString ScriptPath =  KServer::GetKylinInstallPath() + "/data" ;
    KServer::KCommand::getInstance()->RunScripCommand("bash", {"./os-prober-dmsetup"}, ScriptPath,  output, error, exitcode);
}


QString getCdromBootDisk()
{
    QProcess process(0);
    QString cmd = "mount";
    QStringList argvList;
    argvList.clear();
    process.start(cmd, argvList);
    process.waitForFinished(-1);
    QString outPutString = process.readAllStandardOutput();
    QStringList stringList = outPutString.split("\n");
    foreach (QString string, stringList) {
        if (string.contains("/cdrom")) {
            QStringList cdromStringList = string.split(" ");
            foreach (QString string, cdromStringList) {
                if (string.compare("/cdrom") == 0) {
                    return cdromStringList.at(0);
                }
            }
        }
    }
    return nullptr;
}

void PartitionServer::doCreatePartitionTable( QString &devPath, PartTableType table)
{
    if(!createPartitionTable(devPath, table)) {
        qCritical() << "PartitionServer failed to create partition table at"
                    << devPath;
    }
    qDebug() << Q_FUNC_INFO ;
    DeviceList devices = scanAllDevices(m_blosprober);
    emit this->signalDevicesRefreshed(devices);
}

void PartitionServer::doRefreshDevices(bool umount, bool osprober_enable)
{
    if(umount) {
        UnmountDevices();
    }
    qDebug() << Q_FUNC_INFO;
    m_blosprober = osprober_enable;
    DeviceList devices = scanAllDevices(osprober_enable);
    emit signalDevicesRefreshed(devices);
}

void PartitionServer::doQuickPart()
{
//    if(!QFile::exists(script_path)) {
//        qCritical() << "partition script file not found!" << script_path;
//        emit signalQuikPartDone(false);
//        return;
//    }

    emit signalQuikPartDone();
}

void PartitionServer::writeCustomPartInfo(OperationDiskList& operations ,DeviceList& virtualDevices)
{
    qDebug() <<"Start custom installer";
    OperationDiskList operations_(operations);
    DeviceList m_virtualDevices(virtualDevices);
    int num=1;
    QString infoPartition = "";
    QString options= "custom-info";
    for(OperationDisk operate : operations_) {
            if(num==10) {
                    num=90;
            }
            else if(num==100) {
                    num=990;
            }
            if(operate.m_type == OperatorTYPE::Create) {
                    if(operate.m_newPartition->m_devPath == "lvm-vg" ) {
                        continue;
                    }

                    qDebug() <<"Create  Partition111111";
                    qDebug() << "operate.m_newPartition->m_devPath)=" << operate.m_newPartition->m_devPath;

                    infoPartition = infoPartition + "operator=" + "create" + ";";
                    infoPartition = infoPartition + "devPath=" + operate.m_newPartition->m_devPath + ";";
                    infoPartition = infoPartition + "partPath=" + operate.m_newPartition->m_partitionPath + ";";
                    infoPartition = infoPartition + "num=" + QString::number(operate.m_newPartition->m_num) + ";";

                    if(operate.m_newPartition->m_isMounted || operate.m_newPartition->m_mountPoint != "") {
                            if(operate.m_newPartition->m_mountPoint != "empty") {
                                    qDebug() << "operate.m_newPartition->m_mountPoint=" << operate.m_newPartition->m_mountPoint;
                            }
                    }
                    if(operate.m_newPartition->fs != FSType::Empty) {
                            qDebug() << "findNameByFSType(operate.m_newPartition->fs)=" << findNameByFSType(operate.m_newPartition->fs);
                            infoPartition = infoPartition + "fs=" +findNameByFSType(operate.m_newPartition->fs) + ";";
                    }

                    switch (operate.m_newPartition->m_type) {
                    case PartitionType::Normal:
                            infoPartition = infoPartition + "partType=" + "primary"  + ";";
                            break;
                    case PartitionType::Logical:
                            infoPartition = infoPartition + "partType=" + "logical"  + ";";
                            break;
                    case PartitionType::Extended:
                            infoPartition = infoPartition + "partType=" + "extended"  + ";";
                            break;
                    default:
                            break;
                    }
                    infoPartition = infoPartition + "partStart=" + QString::number(operate.m_newPartition->m_sectorStart*operate.m_newPartition->m_sectorSize/1024/1024) + ";";
                    infoPartition = infoPartition + "partEnd=" + QString::number(operate.m_newPartition->m_sectorEnd*operate.m_newPartition->m_sectorSize/1024/1024 + 1) + ";";

                    qDebug() << "infoPartition=" << infoPartition;
                    options= options + QString::number(num);
                    KServer::WriteSettingToIni("custompartition", options , infoPartition);
                    num++;
                    options= "custom-info";
                    infoPartition = "";

            }
	    if(operate.m_type == OperatorTYPE::Delete) {
                    if(operate.m_newPartition->m_devPath == "lvm-vg" ) {
                        continue;
                    }
                    qDebug() <<"Delete  Partition111111";
                    qDebug() << "QString::number(operate.m_origPartition->m_num)=" <<  QString::number(operate.m_origPartition->m_num);
                    qDebug() << "operate.m_origPartition->m_devPath=" << operate.m_origPartition->m_devPath;

                    infoPartition = infoPartition + "operator=" + "delete" + ";";
                    infoPartition = infoPartition + "devPath=" + operate.m_origPartition->m_devPath + ";";
                    infoPartition = infoPartition + "partPath=" + operate.m_origPartition->m_partitionPath + ";";
                    infoPartition = infoPartition + "num=" + QString::number(operate.m_origPartition->m_num) + ";";
                    qDebug() << "infoPartition=" << infoPartition;
                    options= options + QString::number(num);
                    KServer::WriteSettingToIni("custompartition", options , infoPartition);
                    num++;
                    options= "custom-info";
                    infoPartition = "";

            }
            if(operate.m_type == OperatorTYPE::Format) {
                    if(operate.m_newPartition->m_devPath == "lvm-vg" ) {
                        continue;
                    }
                    qDebug() <<"Format  Partition111111";
                    qDebug() << "QString::number(operate.m_newPartition->m_num)=" <<  QString::number(operate.m_newPartition->m_num);
                    qDebug() << "operate.m_newPartition->m_devPath)=" << operate.m_newPartition->m_devPath;
                    qDebug() << "findNameByFSType(operate.m_newPartition->fs)=" << findNameByFSType(operate.m_newPartition->fs);

                    infoPartition = infoPartition + "operator=" + "format" + ";";
                    infoPartition = infoPartition + "partPath=" + operate.m_newPartition->m_partitionPath + ";";
                    infoPartition = infoPartition + "num=" + QString::number(operate.m_newPartition->m_num) + ";";
                    infoPartition = infoPartition + "fs=" +findNameByFSType(operate.m_newPartition->fs) + ";";
                    qDebug() << "infoPartition=" << infoPartition;
                    options= options + QString::number(num);
                    KServer::WriteSettingToIni("custompartition", options , infoPartition);
                    num++;
                    options= "custom-info";
                    infoPartition = "";
            }
	    if(operate.m_type == OperatorTYPE::NewPartitionTable) {
                    if(operate.m_dev->m_partTableType == PartTableType::GPT) {
                            qDebug() <<"create  GPT111111";
                            qDebug() << "operate.m_dev->m_path=" << operate.m_dev->m_path;

                            infoPartition = infoPartition + "operator=" + "newPartTable" + ";";
                            infoPartition = infoPartition + "partPath=" + operate.m_dev->m_path + ";";
                            infoPartition = infoPartition + "partTableType=" + "GPT" + ";";
                            qDebug() << "infoPartition=" << infoPartition;
                            options= options + QString::number(num);
                            KServer::WriteSettingToIni("custompartition", options , infoPartition);
                            num++;
                            options= "custom-info";
                            infoPartition = "";

                    } else if(operate.m_dev->m_partTableType == PartTableType::MsDos) {
                            qDebug() <<"create  MBR111111";
                            qDebug() << "operate.m_dev->m_path=" << operate.m_dev->m_path;
                            infoPartition = infoPartition + "operator=" + "newPartTable" + ";";
                            infoPartition = infoPartition + "partPath=" + operate.m_dev->m_path + ";";
                            infoPartition = infoPartition + "partTableType=" + "MsDos" + ";";
                            qDebug() << "infoPartition=" << infoPartition;
                            options= options + QString::number(num);
                            KServer::WriteSettingToIni("custompartition", options , infoPartition);
                            num++;
                            options= "custom-info";
                            infoPartition = "";
                    }
            }

     }
     for (int k = 0; k < m_virtualDevices.length(); ++k) {
        if(m_virtualDevices.at(k)->isLvm==true){
                for(int m = 0; m < m_virtualDevices.at(k)->partitions.length(); m++) {
                        if(num==10) {
                                num=90;
                        }
                        else if(num==100) {
                                num=990;
                        }

                        if(m_virtualDevices.at(k)->partitions.at(m)->fs == FSType::Vg){
                                continue;
                        }
                        qDebug() <<"Create  Partition111111";
                        qDebug() << "m_virtualDevices.at(k)->partitions.at(m)->m_devPath)=" << m_virtualDevices.at(k)->partitions.at(m)->m_devPath;

                        infoPartition = infoPartition + "operator=" + "create" + ";";
                        infoPartition = infoPartition + "devPath=" + m_virtualDevices.at(k)->partitions.at(m)->m_devPath + ";";
                        infoPartition = infoPartition + "partPath=" + m_virtualDevices.at(k)->partitions.at(m)->m_partitionPath + ";";
                        infoPartition = infoPartition + "num=" + QString::number(m_virtualDevices.at(k)->partitions.at(m)->m_num) + ";";

                        if(m_virtualDevices.at(k)->partitions.at(m)->m_isMounted || m_virtualDevices.at(k)->partitions.at(m)->m_mountPoint != "") {
                                if(m_virtualDevices.at(k)->partitions.at(m)->m_mountPoint != "empty") {
                                        qDebug() << "m_virtualDevices.at(k)->partitions.at(m)->m_mountPoint=" << m_virtualDevices.at(k)->partitions.at(m)->m_mountPoint;
                                }
                        }
                        if(m_virtualDevices.at(k)->partitions.at(m)->fs != FSType::Empty) {
                                qDebug() << "findNameByFSType(operate.m_newPartition->fs)=" << findNameByFSType(m_virtualDevices.at(k)->partitions.at(m)->fs);
                                infoPartition = infoPartition + "fs=" +findNameByFSType(m_virtualDevices.at(k)->partitions.at(m)->fs) + ";";
                        }

                        infoPartition = infoPartition + "partType=" + "primary"  + ";";
                        infoPartition = infoPartition + "partStart=" + QString::number(m_virtualDevices.at(k)->partitions.at(m)->m_sectorStart*m_virtualDevices.at(k)->partitions.at(m)->m_sectorSize/1024/1024) + ";";
                        infoPartition = infoPartition + "partEnd=" + QString::number(m_virtualDevices.at(k)->partitions.at(m)->m_sectorEnd*m_virtualDevices.at(k)->partitions.at(m)->m_sectorSize/1024/1024) + ";";

                        qDebug() << "infoPartition=" << infoPartition;
                        options= options + QString::number(num);
                        KServer::WriteSettingToIni("custompartition", options , infoPartition);
                        num++;
                        options= "custom-info";
                        infoPartition = "";

                }
        }
    }

}


void PartitionServer::doCustomPart(OperationDiskList& operations)
{
    qDebug() << Q_FUNC_INFO << "operations:";
    bool isOK = true;

    OperationDiskList realOperations(operations);
    for(int i = 0; isOK && i < realOperations.length(); i++) {
        OperationDisk operation = realOperations[i];
        if(operation.m_type != OperatorTYPE::NewPartitionTable) {
            qDebug() << "doCustomPart:" << i << "-" << operation.m_newPartition->m_partitionPath << "-" << operation.m_origPartition->m_partitionPath;
            qDebug() << "doCustomPart:" << i << "--" << operation.m_newPartition->m_sectorStart << "--" << operation.m_newPartition->m_sectorEnd;
        }
        isOK = operation.applyToDisk();
    }
    qDebug() << Q_FUNC_INFO << "realoperations:";
    DeviceList devices;
    if(isOK) {
        devices = scanAllDevices(false);
        std::map<QString, Partition::Ptr> mountMap;
        for(const OperationDisk& operation : realOperations) {
            if((operation.m_type == OperatorTYPE::Create) ||
                    (operation.m_type == OperatorTYPE::Format) ||
                    (operation.m_type == OperatorTYPE::MountPoint)) {
                mountMap[operation.m_newPartition->m_mountPoint] = operation.m_newPartition;
            }
            if(operation.m_type == OperatorTYPE::Delete) {
                mountMap.erase(operation.m_origPartition->m_mountPoint);
            }
        }

        for(Device::Ptr dev : devices) {
            for(Partition::Ptr part : dev->partitions) {
                auto it = std::find_if(mountMap.cbegin(), mountMap.cend(),
                                       [=](std::pair<QString, Partition::Ptr>pair) {
                    return (part->m_devPath == pair.second->m_devPath) &&
                            (part->m_sectorStart == pair.second->m_sectorStart) &&
                            (part->m_sectorEnd == pair.second->m_sectorEnd);
                });
                if(it != mountMap.cend()) {
                    std::pair<QString, Partition::Ptr> pair = *it;
                    part->m_mountPoint = pair.second->m_mountPoint;
                }
            }
        }
    }

    emit signalCustomPartDone(isOK, devices);
}



void EnableVG(bool enable)
{
    const QString cmd = "vgchange";
    const QStringList args = {"-a", enable ? "y" : "n"};
    QString cmdpath = "";
    QString output = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(cmd, args, cmdpath, output, error, exitcode)) {
        qWarning() << QString("EnableVG:Failed to enable VG(%1)").arg(enable);
        if(!error.isEmpty()) {
            qWarning() << QString("EnableVG:{%1}").arg(error);
        }
    }
    if(!output.isEmpty()) {
        qInfo() << QString("EnableVG:{%1}").arg(output);
    }
}

void getDiskPartitonDefaultSize(QString devpath,qint64& sizeDataDefault, qint64& sizeRootDefault, qint64& sizeSwapDefault, qint64& sizeBackupDefault)
{
    if(devpath.isEmpty())
    {
        qDebug()<< Q_FUNC_INFO <<" parameters error!";
        return;
    }

    QString output = "";
    QString ScriptPath  = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("/usr/share/kylin-os-installer/data/systemdatasize.sh",
                     {devpath}, ScriptPath, output, error, exitcode)) {
        qDebug()<< Q_FUNC_INFO <<"Run systemdatasize.sh error!";
        return;
    }

    //单位MiB
    for(const QString& line : output.split('\n')) {
        if(line.startsWith("swap_default_size:")) {
            sizeSwapDefault = line.split(':').last().trimmed().toLong();
        }
        if(line.startsWith("backup_default_size:")) {
            sizeBackupDefault = line.split(':').last().trimmed().toLong();
        }
        if(line.startsWith("root_default_size:")) {
            sizeRootDefault = line.split(':').last().trimmed().toLong();
        }
        if(line.startsWith("data_default_size:")) {
            sizeDataDefault = line.split(':').last().trimmed().toLong();
        }
    }
}



PartitionFlags getPartitionFlags(PedPartition* ppartition)
{
    Q_ASSERT(ppartition);
    PartitionFlags flags;
    for(PedPartitionFlag pflag = ped_partition_flag_next(static_cast<PedPartitionFlag>(NULL));
        pflag;
        pflag = ped_partition_flag_next(pflag)) {
        if(ped_partition_is_flag_available(ppartition, pflag) &&
                ped_partition_get_flag(ppartition, pflag)) {
            flags.append(static_cast<PartitionFlag>(pflag));
        }
    }
    return flags;
}

PartitionList readPartitions(PedDisk* pdisk)
{
    Q_ASSERT(pdisk);
    PartitionList partitions;
    for(PedPartition* pPartition = ped_disk_next_partition(pdisk, nullptr);
        pPartition != nullptr;
        pPartition = ped_disk_next_partition(pdisk, pPartition)) {
        Partition::Ptr partition(new Partition);
        if(pPartition->type == PED_PARTITION_NORMAL) {
            partition->m_type = PartitionType::Normal;
        } else if(pPartition->type == PED_PARTITION_EXTENDED) {
            partition->m_type = PartitionType::Extended;
            partition->m_name = "extend";
        } else if (pPartition->type == (PED_PARTITION_LOGICAL | PED_PARTITION_FREESPACE)) {
            partition->m_type = PartitionType::Unallocated;
        } else if(pPartition->type == PED_PARTITION_LOGICAL) {
            partition->m_type = PartitionType::Logical;
        } else if(pPartition->type == PED_PARTITION_FREESPACE) {
            partition->m_type = PartitionType::Unallocated;

        } else {
            continue;
        }

        if(ped_partition_is_active(pPartition)) {
            partition->flags = getPartitionFlags(pPartition);
        }
        if(partition->flags.contains(PartitionFlag::Swap)) {
            partition->fs = FSType::LinuxSwap;
        }


        if(pPartition->fs_type) {
            partition->fs = findFSTypeByName(pPartition->fs_type->name);

            if((partition->fs == FSType::Fat32 || partition->fs == FSType::Fat16) &&
                    partition->flags.contains(PartitionFlag::ESP)) {
                partition->fs = FSType::EFI;
                partition->m_mountPoint = "/boot/efi";
                partition->m_isMounted = true;

            }
        } else {
            partition->fs = FSType::Empty;
        }
        partition->m_sectorStart = pPartition->geom.start;
        partition->m_sectorEnd = pPartition->geom.end;
        partition->m_sectorSize = pPartition->disk->dev->sector_size;
        partition->m_num = pPartition->num;
        partition->m_partitionPath = getPartitionPath(pPartition);
        partition->m_devPath = pdisk->dev->path;
        partition->m_total = partition->getByteLength();
        partition->blFormatPartition = false;

        if(!partition->m_partitionPath.isEmpty() &&
                partition->m_type != PartitionType::Unallocated &&
                partition->m_type != PartitionType::Extended) {

            getSizeInfo(partition->m_partitionPath, partition->fs, partition->m_freespace,
                      partition->m_total);

            if((partition->fs == FSType::LinuxSwap || partition->fs == FSType::KylinData) && partition->m_total <=0) {
                partition->m_total = partition->getByteLength();
                partition->m_freespace = partition->m_total;
            }

            partition->m_name = ped_partition_get_name(pPartition);
        }
        if(pPartition->type == PED_PARTITION_FREESPACE &&  pPartition->num < 0) {
            partition->m_freespace = partition->getByteLength();
        }

        if(partition->fs == FSType::Ext4){
	    QString testKylinData ="kylin-data=";
            testKylinData =testKylinData + partition->m_partitionPath;
            qDebug() << "testKylinData =" << testKylinData;
	    if(KServer::KReadFile("/var/log/installer/os-prober.log").contains(testKylinData)){
                partition->fs = FSType::KylinData;
            }
	}
	partition->realFs= partition->fs;
        partitions.append(partition);
    }
    return partitions;

}

DeviceList scanAllDevices(bool osprober_enable)
{
    qDebug() << Q_FUNC_INFO << "0";
//    EnableVG(false);
    //针对ventoy进行特殊判断 2021-3-9
    QString ventoyPath = getVentoyDevPath();

    // 获取cdrom分区路径
    QString cmdromPath = getCdromBootDisk();

    DeviceList devicelist;
    ped_device_probe_all();
#if 0
#ifdef QT_DEBUG
    QDir dir("/dev/");
    dir.setFilter(QDir::System);
    QFileInfoList list = dir.entryInfoList();
    for(const QFileInfo& info : list) {
        if(info.filePath().startsWith("/dev/loop")) {
            qDebug() << "Loop device:" << info.filePath();
            ped_device_get(info.filePath().toUtf8().data());
        }
    }

#endif
#endif

    const LabelItems labelitems = getparsedLabelDir();
    const MountItemList mounts = parseMountItems();

    OSProberItemList ositems;
    if(osprober_enable) {
	ositems=refreshOSProberItems();
	osproberDmsetup();
        //ositems = getOsProberItems();
    }

    for(PedDevice* pDevice = ped_device_get_next(nullptr);
        pDevice != nullptr;
        pDevice = ped_device_get_next(pDevice)) {
        PedDiskType* disktype = ped_disk_probe(pDevice);
        //针对ventoy进行特殊判断 2021-3-9
        if(pDevice->path == ventoyPath) {
            continue;
        }

        //过滤/dev/mapper设备2021-3-5 17:09:00
        QString tmp = pDevice->path;
        if(tmp.startsWith("/dev/mapper")) {
            continue;
        }

        if (cmdromPath == pDevice->path) {
            continue;
        }

        if(pDevice->read_only) {
            qInfo() << QString("ignored: by readOnly {%1} {%2}").arg(pDevice->read_only).arg(pDevice->path);
            continue;
        }

        const bool validDev = [&](PedDeviceType type)->bool {
            std::list<PedDeviceType> blackList {

#ifndef QT_DEBUG
                PedDeviceType::PED_DEVICE_LOOP,
#endif
                PedDeviceType::PED_DEVICE_UNKNOWN,

            };
            for(PedDeviceType ptype : blackList) {
                if(ptype == type) {
                    return false;
                }
            }
            return true;
        }(pDevice->type);

        if(!validDev) {
            qDebug() << "device type:" << pDevice->type;
            continue;
        }

        Device::Ptr device(new Device);

        if(disktype == nullptr) {
            device->m_partTableType = PartTableType::Empty;

        } else {
           if(strcmp(disktype->name, "msdos") == 0) {
                device->m_partTableType = PartTableType::MsDos;
            } else if (strcmp(disktype->name, "gpt") == 0) {
                device->m_partTableType = PartTableType::GPT;
            } else if(strcmp(disktype->name, "loop") == 0) {
                device->m_partTableType = PartTableType::Others;
                qDebug() << "device :" << device->m_path;
            } else if(strcmp(disktype->name, "mac")) {
                device->m_partTableType = PartTableType::Others;
                qDebug() << "device :" << device->m_path;
            }
            else {
                device->m_partTableType = PartTableType::Empty;
                qWarning() << "other type of device :" << device->m_path;
            }
        }
        device->m_path = pDevice->path;
	device->m_history=false;
	//QString testHistory1 ="save-data=";
        //testHistory1 =testHistory1 + device->m_path;
        //qDebug() << "testHistory1 =" << testHistory1;
        //if(KServer::KReadFile("/var/log/installer/os-prober.log").contains(testHistory1)){
        //        device->m_history=true;
        //        qDebug() << "It can be save history data installer in " << device->m_path;
        //}

        for(int i = 0; i < ositems.length(); i++) {
            bool isOk=false;
	    QString name = ositems.at(i).path;
	    QString osClass = ositems.at(i).OSdescription;
            qDebug() << "name=" << name;
            qDebug() << "osClass=" << osClass;
            qDebug() << "device->m_path=" << device->m_path;

            if(name.contains(device->m_path)) {
	//	device->m_history=false;
                device->m_osName = ositems.at(i).OSdescription;
                if(osClass.contains("Kylin")&&(osClass.contains("SP")||osClass.contains("V11")||osClass.contains("openKylin"))) {
                 //       isOk=osproberDevice(device->m_path);
		//	if(isOk==true){
		//	if(KServer::ReadSettingIni("specialmodel",device->m_path)=="save-data"){
		//		device->m_history=true;
		//	}
	    		QString testHistory ="save-data=";
			testHistory =testHistory + device->m_path; 
            		qDebug() << "testHistory =" << testHistory;
			if(KServer::KReadFile("/var/log/installer/os-prober.log").contains(testHistory)){
				device->m_history=true;
            			qDebug() << "It can be save history data installer in " << device->m_path;
			}
                }
            }
        }
        device->m_model = pDevice->model;
        device->m_sectorSize = pDevice->sector_size;
        device->m_readyOnly = pDevice->read_only;
        device->m_length = pDevice->length;
        device->m_heads = pDevice->bios_geom.heads;
        device->m_sectors = pDevice->bios_geom.sectors;
        device->m_cylinders = pDevice->bios_geom.cylinders;
        device->m_diskTotal = device->getByteLength();
	getDiskPartitonDefaultSize(device->m_path,device->m_data_default_size,device->m_root_default_size,device->m_swap_default_size,device->m_backup_default_size);

	qDebug() << "1111device->m_data_default_size= " << device->m_data_default_size;
	qDebug() << "1111device->m_root_default_size= " << device->m_root_default_size;
	qDebug() << "1111device->m_swap_default_size= " << device->m_swap_default_size;
	qDebug() << "1111device->m_backup_default_size= " << device->m_backup_default_size;
        if(device->m_partTableType == PartTableType::Empty) {
            Partition::Ptr freePartition(new Partition);
            freePartition->m_devPath = device->m_path;
            freePartition->m_partitionPath = "";
            freePartition->m_num = -1;
            freePartition->m_sectorStart = 1;
            freePartition->m_sectorEnd = device->m_length;
            freePartition->m_sectorSize = device->m_sectorSize;
            freePartition->m_total = device->getByteLength();
            freePartition->m_type = PartitionType::Unallocated;
            freePartition->blFormatPartition = false;
            device->partitions.append(freePartition);
        } else if(device->m_partTableType == PartTableType::MsDos ||
                  device->m_partTableType == PartTableType::GPT ||
                  device->m_partTableType == PartTableType::Others) {
            PedDisk* pDisk = nullptr;
            pDisk = ped_disk_new(pDevice);

            if(pDisk) {
                device->m_maxPrims = ped_disk_get_max_primary_partition_count(pDisk);

                device->partitions = readPartitions(pDisk);
                for(Partition::Ptr part : device->partitions) {
                    part->m_devPath = device->m_path;
                    part->m_sectorSize = device->m_sectorSize;
                    if(!part->m_partitionPath.isEmpty() &&
                            part->m_type != PartitionType::Unallocated) {
                        const QString labelStr = labelitems.value(part->m_partitionPath, getPartitionLabel(part));
                        if(!labelStr.isEmpty()) {
                            part->m_FSlabel = labelStr;
                        }
                        for(const OSProberItemStr& item : ositems) {
                            if(item.path == part->m_partitionPath) {
                                part->osType = item.osType;
                                part->m_OSname = item.OSdescription;
//                                device->m_osName = item.OSdescription;
                                break;
                            }
                        }
                        for(const MountItemStr& mount : mounts) {
                            if(mount.path == part->m_partitionPath) {
                                part->m_mountPoint = mount.mount;
                                part->m_isMounted = true;
                                break;
                            }
                        }
                    }
                }
                device->getDiskUsedPercent();
                ped_disk_destroy(pDisk);
            } else {
                qCritical() << "failed to get disk object:" << device->m_path;
                device->m_diskFreeSpace = device->m_diskTotal;
            }
        }
        devicelist.append(device);
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(devicelist.begin(), devicelist.end(), [=](Device::Ptr a, Device::Ptr b) {
        return collator.compare(a->m_path, b->m_path) < 0;
    });

    return devicelist;
}


}
}
