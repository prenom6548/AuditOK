/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "datacdrom.h"
#include "src/frame/logger.h"
#include "src/frame/signals.h"
#include "src/po/translation.h"
#include "src/view/modaldialog.h"
#include "src/view/progress.h"
#include "src/model/logicmain.h"
#include "src/process/processlsof.h"
#include "src/view/tip_burnresult.h"
#include "src/data/iso9660.h"
#include "src/data/iso9660backend.h"
#include "src/status-fix/discscsi.h"

#include "burnjobmanager.h"
#include "burnjob.h"

#include "dopticaldiscinfo.h"
#include "dopticaldiscmanager.h"

#include <glib.h>
#include <gio/gio.h>

#include <unistd.h>
#include <fcntl.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <mntent.h>

#include <QThread>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTimer>
#include <QDate>
#include <QUuid>
#include <QSharedPointer>


#define WAIT_SEC   10
#define FACTOR_CD  175
#define FACTOR_DVD 1385
#define FACTOR_BD 4495

using namespace ky_plugin_burn;
using namespace dfmburn;
DataMedium::DataMedium(QString mediuType, QString blockName, QObject *parent) :
    QObject(parent),
    m_oMediumType(mediuType),
    m_oBlockName(blockName),
    m_bAppRunning(true),
    m_bErasable(false),
    m_u64Capacity(0),
    m_u64FreeCapacity(0),
    m_u32TrackNumber(0),
    m_u64UsedCapacity(0),
    m_mountPointTimer(new QTimer(this)),
    m_oCDROM(qobject_cast<DataCDROM *>(parent))
{

    m_oCapacity = _("%1 B").arg(0.00);
    m_oUsedCapacity = _("%1 B").arg(0.00);
    m_oFreeCapacity = _("%1 B").arg(0.00);
    m_oMediumName.clear();
    if (m_oCDROM) {
        m_oCDROM->updateVerficationData(false); // 刚插入的光盘默认不进行校验数据
    }
    LOGGER->registerName(this, _("Medium Data<%1>").arg(m_oBlockName));
    connect(LogicMain::instance(), &LogicMain::exit, [=](){m_bAppRunning = false;});

    //UDF功能：不管做什么操作，刚插入光盘显示为ISO9660格式
    connect(g_user_signal,SIGNAL(changBurnFormat()),this,SLOT(slotsChangeBurnFormat_CDROM()));

    if (m_oMediumType.startsWith("CD"))
        m_oIcon = QIcon::fromTheme("media-optical-cd-audio");
    else
        m_oIcon = QIcon::fromTheme("media-optical-dvd");


    m_discread = new DiscRead(blockName, this);
}

DataMedium::DataMedium(DataMedium &other, QObject *parent) :
    QObject(parent)
{
    m_u8Status = other.status();

    //m_bAppRunning;
    m_bErasable = other.isErasable();

    //m_bUDF;
    m_bEnableUDF = other.isEnableUDF()    ;
    m_oCDROM = qobject_cast<DataCDROM *>(parent);

    //m_u32TrackNumber;
    m_iNumDataTrack = other.numDataTracks();
    m_u64UsedCapacity = other.get32UsedCapacity();
    m_u64FreeCapacity = other.get32FreeCapacity();
    m_u64Capacity  = other.get32Capacity();

    //m_oVolumeID;
    m_oBlockName = other.getBlockName();
    m_oMediumType = other.getType();
    m_oUsedCapacity = other.getUsedCapacity();
    m_oFreeCapacity = other.getFreeCapacity();
    m_oCapacity = other.getCapacity();
    m_oMountPoint = other.getMountPoint();

    //m_oMediumName;
    m_oDisplay = other.display();
    m_oIcon = other.icon();
    m_iSpeeds = other.getNumSpeed();
    m_oSpeeds = other.speeds();
    m_mountPointTimer = nullptr;
    m_discread = nullptr;
}


void DataMedium::load()
{
    LOGGER->info(QString("( DataMedium::load ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_oCDROM || !m_discread || !m_mountPointTimer) {
        LOGGER->warn(QString("( DataMedium::load ) Line:%1 medium is invalid, dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }

    connect(m_mountPointTimer,&QTimer::destroyed,this,[&]{m_mountPointTimer = nullptr;},Qt::UniqueConnection);
    connect(m_mountPointTimer, &QTimer::timeout, this, &DataMedium::slotsMountPoint,Qt::UniqueConnection);

    if (!m_oCDROM->isOpen()) {
        if (!m_oCDROM->open()) {
            LOGGER->warn(QString("( DataCDROM::load ) device = %1 , open device faild").arg(m_oBlockName));
        }
    }

    type();
    speed();
    m_oCDROM->close();
    m_oDisplay = _("%1 - %2 medium, left space : %3")
            .arg(m_oMediumName.isEmpty() ? _("Empty medium") : m_oCDROM->settingVolumnID())
            .arg(m_oMediumType).arg(m_oFreeCapacity);

    connect(m_discread, &DiscRead::workFinished, this, &DataMedium::fixDiscStatus);

    m_discread->work();
    LOGGER->info(QString("( DataMedium::load ) Line:%1 运行完成 dev=%2").arg(__LINE__).arg(m_oBlockName));
}

void DataMedium::fixDiscStatus(){
    LOGGER->info(QString("( DataMedium::fixDiscStatus ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_discread->isSuccess()) {
        qDebug() << __func__ << __LINE__ << "获取光盘状态数据失败，准备第二次查询";
        connect(this, &DataMedium::secondGetDiscStatus, this, &DataMedium::slotsSecondGetDiscStatus);
        Q_EMIT this->secondGetDiscStatus();
        return;
    }
    qDebug() << __func__ << __LINE__ << " 获取光盘状态数据成功";
    m_discread->disconnect(); //关闭connect,可能会有第二次，使用不同槽函数

    if(m_discread->isBlank()){
        m_u8Status = MEDIUM_EMPTY;
    }else if(m_discread->canAppend()){
        m_u8Status = MEDIUM_INCOMPLETE;
    }else
        m_u8Status = MEDIUM_FINALIZED;

    m_iNumDataTrack = m_discread->numDataTracks();

    //UDF功能
    EnableUDF();
    emit g_user_signal->changBurnFormat();

    if (m_u8Status == MEDIUM_EMPTY) {
        capacity();
    }
    m_mountPointTimer->start(1000);
}

void DataMedium::slotsSecondGetDiscStatus()
{
    LOGGER->info(QString("( DataMedium::slotsSecondGetDiscStatus ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    m_discread->work();
}

void DataMedium::secondFixDiscStatus()
{
    LOGGER->info(QString("( DataMedium::secondFixDiscStatus ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    qDebug() << "m_discread->canAppend() = " << m_discread->canAppend();
    if(m_discread->canAppend()) {
        m_u8Status = MEDIUM_INCOMPLETE;
        capacity();
        Q_EMIT g_user_signal->secondChangeDiscStatus();
        Q_EMIT g_user_signal->changeBtnBusnessName();// 修改mainwindow中btnBusness按钮的名字(与光盘状态有关)
        qDebug() << QString("[%1][%2] 挂载点已查到：[%3]  定时器已关，发送信号").arg(__func__).arg(__LINE__).arg(m_oMountPoint);
    }

    if (m_discread->discFilesystemType().contains("udf"))
        capacity();

    if (!m_discread->discLabel().isEmpty()) {
            m_oVolumeID = m_discread->discLabel();
            qDebug() << QString("[%1][%2]: set卷标=%3").arg(__func__).arg(__LINE__).arg(m_oVolumeID);
            m_oCDROM->setVolumnID(m_oVolumeID);
            m_oDisplay = _("%1 - %2 medium, left space : %3")
                    .arg(m_oMediumName.isEmpty() ? _("Empty medium") : m_oVolumeID)
                    .arg(m_oMediumType).arg(m_oFreeCapacity);
            qDebug()<<__LINE__<<__func__<<m_oFreeCapacity;
        } else {
            m_oCDROM->setVolumnID(_("KylinBurner"));
            m_oDisplay = _("%1 - %2 medium, left space : %3")
                    .arg(m_oMediumName.isEmpty() ? _("Empty medium") : _("KylinBurner"))
                    .arg(m_oMediumType).arg(m_oFreeCapacity);
        }

    m_iNumDataTrack = m_discread->numDataTracks();
    Q_EMIT g_user_signal->sigChangeDiscDisplayInfo((void*)m_oCDROM, m_oDisplay);

}

void DataMedium::slotsMountPoint()
{
    LOGGER->info(QString("( DataMedium::slotsMountPoint ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));

    if ((m_u8Status > MEDIUM_EMPTY && m_u8Status < MEDIUM_OTHER) &&
            (m_oMountPoint.isEmpty() || m_oMediumName.isEmpty()) &&
            m_bAppRunning)
    {
        FILE *fp = nullptr;
        struct mntent *mp = nullptr;

        fp = setmntent("/proc/mounts", "r");
        if (nullptr == fp)
        {
            LOGGER->error(_("Cannot load mount information.%1").arg(strerror(errno)));
            return;
        }
        while (nullptr != (mp = getmntent(fp))) {
            if (QString::fromUtf8(mp->mnt_fsname) == m_oBlockName)
            {
                m_oMountPoint = QString::fromUtf8(mp->mnt_dir);
                m_oMediumName = m_oMountPoint.split("/").takeLast();
                if (QString::fromUtf8(mp->mnt_type).contains("UDF") ||
                        QString::fromUtf8(mp->mnt_type).contains("udf"))
                {
                    m_bUDF = true;
                    LOGGER->debug(_("Medium %1 mounted by UDF").arg(m_oMountPoint));
                }

                connect(m_discread, &DiscRead::workFinished, this, &DataMedium::secondFixDiscStatus);
                m_discread->work();

                m_mountPointTimer->stop();
                m_mountPointTimer->disconnect();

                LOGGER->info(QString("[%1][%2] 挂载点已查到：[%3]  定时器已关").arg(__func__).arg(__LINE__).arg(m_oMountPoint));
                break;
            }
        }
        endmntent(fp);
    } else {
        if (m_u8Status == MEDIUM_EMPTY) {
            m_mountPointTimer->stop();
            m_mountPointTimer->disconnect();
            LOGGER->info(QString("[%1][%2] 空光盘  定时器已关").arg(__func__).arg(__LINE__));
        }
    }

    if (!m_discread->discLabel().isEmpty()) {
        m_oVolumeID = m_discread->discLabel();
        m_oCDROM->setVolumnID(m_oVolumeID);
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oMediumName.isEmpty() ? _("Empty medium") : m_oVolumeID)
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    } else {
        m_oCDROM->setVolumnID(_("KylinBurner"));
        m_oDisplay = _("%1 - %2 medium, left space : %3")
                .arg(m_oMediumName.isEmpty() ? _("Empty medium") : _("KylinBurner"))
                .arg(m_oMediumType).arg(m_oFreeCapacity);
    }

    m_oDisplay = _("%1 - %2 medium, left space : %3")
        .arg(m_oMediumName.isEmpty() ? _("Empty medium") : m_oCDROM->settingVolumnID())
        .arg(m_oMediumType).arg(m_oFreeCapacity);

    Q_EMIT g_user_signal->changeDiscStatus();
    Q_EMIT g_user_signal->sigChangeDiscDisplayInfo((void*)m_oCDROM, m_oDisplay);
}

void DataMedium::slotsChangeBurnFormat_CDROM()
{
    m_oCDROM->setSettingBurnFormat(QString("ISO9660"));
}

void DataMedium::EnableUDF()
{
    bool is_empty_disk = (m_oCDROM->mediumStatus() == DataMedium::MEDIUM_EMPTY);
    if(!m_oMediumType.contains("RW") && (m_oMediumType != QString("DVD+R DL")) && !m_oMediumType.contains("BD") && (getFileSystemType().contains("udf")))
    {
        m_bEnableUDF = true;
    }
    else if(is_empty_disk && !m_oMediumType.contains("RW") && !m_oMediumType.contains("BD") && (m_oMediumType != QString("DVD+R DL")) )
    {
        m_bEnableUDF = true;
    }
    else
    {
        m_bEnableUDF = false;
    }
}

void DataMedium::type()
{
    LOGGER->info(QString("( DataMedium::type ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    unsigned short len = 0;
    unsigned char  cdb[10] = { 0x51,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x20,
                               0 };
    unsigned char  result[32] = { 0 };

    if (!m_oCDROM->execSCSI(cdb, 10, result, 32)) {
        LOGGER->warn(QString("( DataMedium::type ) Line:%1 执行execSCSI失败 dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }

    len = (result[0] << 8) | result[1];
    if ((len - 32) % 8)
    {
        LOGGER->warn(QString("( DataMedium::type ) Line:%1 获取错误的response信息 dev=%2").arg(__LINE__).arg(m_oBlockName));
        return; // len is 32 + 8 * n
    }

    if (((result[2] & 0x10) >> 4) == 1)
    {
        LOGGER->debug(_("Can erase mediun"));
        m_bErasable = true;
    }

    switch (result[2] & 0x03)
    {
    case 0x00: // empty
        m_u8Status = MEDIUM_EMPTY;
        LOGGER->debug(_("empty mediun."));
        break;
    case 0x01: // incomplete
        m_u8Status = MEDIUM_INCOMPLETE;
        LOGGER->debug(_("incomplete mediun."));
        break;
    case 0x02:
        m_u8Status = MEDIUM_FINALIZED;
        LOGGER->debug(_("finalized mediun."));
        break;
    case 0x03:
        m_u8Status = MEDIUM_OTHER;
        break;
    default:
        break;
    }

    m_u32TrackNumber = (((result[11] << 8) & 0xFF00) | result[6]);
}

void DataMedium::capacity()
{
    LOGGER->info(QString("( DataMedium::capacity ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));

    unsigned char  cdb[10] = { 0x52,
                               0x01,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x28,
                               0 };
    unsigned char  result[40] = { 0 }; // 256 + 12


    if (MEDIUM_FINALIZED == m_u8Status && !m_discread->discFilesystemType().contains("udf")) // finalized all 0.
    {
        LOGGER->warn(_("Donot load type of finalized capacity."));
        return;
    }

    GFile *file;
    GFileInfo *info;
    QString targetUri = "file://" + m_oMountPoint;
    qDebug() << "targetUri = " << targetUri;
    file = g_file_new_for_uri(targetUri.toUtf8().constData());
    info = g_file_query_filesystem_info(file, "*", nullptr, nullptr);


    // 计算DVD+/-RW盘容量
    if (m_oMediumType.contains("DVD+RW") || m_oMediumType.contains("DVD-RW"))
    {
        //计算总容量
        QScopedPointer<DOpticalDiscInfo> disk_info { DOpticalDiscManager::createOpticalInfo(m_oCDROM->device()) };
        m_u64Capacity = disk_info->totalSize();

        if (MEDIUM_EMPTY == m_u8Status)
        {
            m_u64FreeCapacity = m_u64Capacity;
            m_oFreeCapacity = toUnitString(m_u64FreeCapacity);
            m_oCapacity = toUnitString(m_u64Capacity);
            LOGGER->info(QString("Capacity: %1 - used(%2) - free(%3)").arg(m_oCapacity).arg(toUnitString(0)).arg(m_oFreeCapacity));
            return;
        }

        //计算已使用容量
        if (MEDIUM_EMPTY != m_u8Status && m_discread->discFilesystemType().contains("udf")) {
            // UDF文件系统格式
            m_u64UsedCapacity = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
            m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;

        } else {
            // ISO9660文件系统格式
            Iso9660SimplePrimaryDescriptor isoDesc;
            isoDesc = Iso9660SimplePrimaryDescriptor();
            Iso9660 iso( new Iso9660DeviceBackend( m_oCDROM ) );
            iso.setStartSector( 0 );
            iso.setPlainIso9660( true );
            if( iso.open() ) {
                isoDesc = iso.primaryDescriptor();
                m_u64UsedCapacity = isoDesc.getUsedCapacity();
                m_u64FreeCapacity = m_u64Capacity - m_u64UsedCapacity;
            } else {
                // 没有获取到已使用数据容量，可能是Win刻录的UDF盘或空光盘
                LOGGER->info(QString("[%1][%2] Failed to obtain the capacity. It may be a real UDF disk").arg(__func__).arg(__LINE__));
            }
        }

        m_oUsedCapacity = toUnitString(m_u64UsedCapacity);
        m_oCapacity = toUnitString(m_u64Capacity);
        m_oFreeCapacity = toUnitString(m_u64FreeCapacity);

        LOGGER->info(QString("Capacity: %1 - used(%2) - free(%3)").arg(m_oCapacity).arg(m_oUsedCapacity).arg(m_oFreeCapacity));
        return;
    }

    // 计算CD-RW盘容量
    if (MEDIUM_EMPTY != m_u8Status && m_discread->discFilesystemType().contains("udf") && m_oMediumType.contains("CD-RW")) {
        m_u64Capacity = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
        m_u64UsedCapacity  = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
        m_u64FreeCapacity  = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);

        m_oCapacity = toUnitString(m_u64Capacity);
        m_oUsedCapacity = toUnitString(m_u64UsedCapacity);
        m_oFreeCapacity = toUnitString(m_u64FreeCapacity);

        LOGGER->info(QString("Capacity: %1 - used(%2) - free(%3)").arg(m_oCapacity).arg(m_oUsedCapacity).arg(m_oFreeCapacity));
        return;
    }

    // 计算CD-R、DVD-R、DVD+R(DL)光盘盘容量
    cdb[2] = (m_u32TrackNumber >> 24);
    cdb[3] = (m_u32TrackNumber >> 16);
    cdb[4] = (m_u32TrackNumber >> 8);
    cdb[5] = m_u32TrackNumber;

    if (!m_oCDROM->execSCSI(cdb, 10, result, 40)) return;

    m_u64FreeCapacity = ((result[16] << 24) | (result[17] << 16) | (result[18] << 8) | result[19]);
    m_u64FreeCapacity *= 2048;
    m_u64UsedCapacity = ((result[8] << 24) | (result[9] << 16) | (result[10] << 8) | result[11]);
    m_u64UsedCapacity *= 2048;
    m_u64Capacity = m_u64UsedCapacity + m_u64FreeCapacity;

    m_oCapacity = toUnitString(m_u64Capacity);
    m_oUsedCapacity = toUnitString(m_u64UsedCapacity);
    m_oFreeCapacity = toUnitString(m_u64FreeCapacity);

    LOGGER->info(QString("Capacity: %1 - used(%2) - free(%3)").arg(m_oCapacity).arg(m_oUsedCapacity).arg(m_oFreeCapacity));
}

void DataMedium::speed()
{
    int            maxSpeed = 0;
    int            factor = 0;
    unsigned short len = 0;
    unsigned short len2 = 0;
    unsigned char  cdb[12] = { 0xAC, // Get Performance
                               0x00, //
                               0x00,
                               0x00,
                               0x00,
                               0x00, // LBA
                               0x00, // reserve
                               0x00, // reserve
                               0x00,
                               0x00, // Maximun Number of Descriptors
                               0x03, /* type
                                      * 00 Performance data
                                      * 01 Unusable Area data
                                      * 02 Defect Status data
                                      * 03 Write Speed Descriptor
                                      * ...
                                      * MMC-5 6.8.2.5
                                      */
                               0 };
    unsigned char  result[2048] = { 0 };
    LOGGER->info(QString("( DataMedium::speed ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (!m_oCDROM->execSCSI(cdb, 12, result, 8)) {
        LOGGER->warn(QString("( DataMedium::speed ) Line:%1 执行execSCSI失败 dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }
    len = (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | result[3];
    len += 4;

    len2 = len - 8; len2 /= 16;
    cdb[9] = (len2 & 0xFF);
    cdb[8] = ((len2 & 0xFF00) >> 8);
    if (!m_oCDROM->execSCSI(cdb, 12, result, len)) {
        LOGGER->warn(QString("( DataMedium::speed ) Line:%1 执行execSCSI失败 dev=%2").arg(__LINE__).arg(m_oBlockName));
        return;
    }
    m_iSpeeds.clear();
    for (int i = 0; i < len2; ++i)
    {
        m_iSpeeds << ((result[i * 16 + 20] << 24) |
                (result[i * 16 + 21] << 16) |
                (result[i * 16 + 22] << 8) |
                result[i * 16 + 23]);
    }
    if (m_iSpeeds.size() <= 0) return;
    maxSpeed = m_iSpeeds.first();
    if (0 == maxSpeed)
    {
        LOGGER->error(_("Get invalid speed."));
        return;
    }
    if (m_oMediumType.startsWith("CD"))
        factor = FACTOR_CD;
    else if (m_oMediumType.startsWith("DVD"))
        factor = FACTOR_DVD;
    else
        factor = FACTOR_BD;
    for (int i = 0; i < m_iSpeeds.size(); ++i)
    {
        m_oSpeeds << QString("%1").arg(m_iSpeeds.at(i) / factor);
        LOGGER->debug(_("Speed %1 : %2x - rate : %3 Kbps").arg(i).arg(m_oSpeeds.at(i)).arg(m_iSpeeds.at(i)));
    }

}

QString DataMedium::toUnitString(unsigned long size64)
{
    qreal size;
    int u;
    QString ret;

    size = size64 * 1.0;
    u = -1;
    ret.clear();

    while (size > 1024.00)
    {
        ++u; size /= 1024.00;
    }

    ret = QString("%1 ").arg(size, 0, 'f', 1);

    switch (u)
    {
    case 0:
        ret.append(_("KB"));
        break;
    case 1:
        ret.append(_("MB"));
        break;
    case 2:
        ret.append(_("GB"));
        break;
    default:
        ret.append(_("B"));
        break;
    }
    return ret;
}

DataMedium::~DataMedium()
{
    LOGGER->info(QString("( DataMedium::~DataMedium ) Line:%1 进入函数 func=%2").arg(__LINE__).arg(__FUNCTION__));
    m_bAppRunning = false;
    if(m_mountPointTimer)
    {
        if(m_mountPointTimer->isActive())
        {
            m_mountPointTimer->stop();
        }
        m_mountPointTimer->disconnect(this);
        if (m_mountPointTimer->parent() != this) {
            delete m_mountPointTimer;
        }
        m_mountPointTimer = nullptr;
    }
    if (m_discread) {
        m_discread->disconnect(this);
        if (m_discread->parent() != this) {
            delete m_discread;
        }
        m_discread = nullptr;
    }
    m_oCDROM = nullptr;
}

QString DataMedium::getFileSystemType()
{
    if (m_discread) {
        return m_discread->discFilesystemType();
    } else {
        return QString();
    }
}

QString DataMedium::getFileSystemExtendType()
{
    if (m_discread) {
        return m_discread->fileSystemExtendInf();
    } else {
        return QString();
    }
}

bool DataMedium::isRWType()
{
    return m_oMediumType.contains("RW");
}

bool DataMedium::isCDType()
{
    return m_oMediumType.contains("CD");
}

bool DataMedium::isDVDType()
{
    return m_oMediumType.contains("DVD");
}

DataCDROM::DataCDROM(QString blockName, QObject *parent) :
    QObject(parent),
    m_oSetting(new DataSetting(this)),
    m_oBlockName(blockName),
    m_bCDROM(true),
    m_bStart(false),
    m_bAppRunning(true),
    m_iHandle(-1),
    m_is_using(0),
    m_oMedium(nullptr),
    m_bSupportBD(false),
    m_bUpdateStart(false){

    m_oMediumType.clear();
    LOGGER->registerName(this, _("CDROM Data<%1>").arg(m_oBlockName));
    connect(m_oSetting, SIGNAL(speed(int)), BUSNESSSETTING, SLOT(setSpeed(int)));
    connect(m_oSetting, SIGNAL(clean(bool)), BUSNESSSETTING, SLOT(setClean(bool)));
    connect(m_oSetting, SIGNAL(closeSession(bool)), BUSNESSSETTING, SLOT(setCloseSession(bool)));
    connect(m_oSetting, SIGNAL(open(bool)), BUSNESSSETTING, SLOT(setOpen(bool)));
    connect(m_oSetting, SIGNAL(createISO(bool)), BUSNESSSETTING, SLOT(setCreateISO(bool)));
    connect(m_oSetting, SIGNAL(verficationData(bool)), BUSNESSSETTING, SLOT(setVerficationData(bool)));
    connect(m_oSetting, SIGNAL(preparer(QString)), BUSNESSSETTING, SLOT(setPreparer(QString)));
    connect(m_oSetting, SIGNAL(publisher(QString)), BUSNESSSETTING, SLOT(setPublisher(QString)));
    connect(m_oSetting, SIGNAL(volumnID(QString)), BUSNESSSETTING, SLOT(setVolumnID(QString)));
    connect(m_oSetting, SIGNAL(copyright(QString)), BUSNESSSETTING, SLOT(setCopyright(QString)));
    connect(m_oSetting, SIGNAL(tempPath(QString)), BUSNESSSETTING, SLOT(setTempPath(QString)));
    connect(m_oSetting, SIGNAL(burnFormat(QString)), BUSNESSSETTING, SLOT(setBurnFormat(QString)));
    connect(m_oSetting, SIGNAL(IsShowBeforBurn(bool)), BUSNESSSETTING, SLOT(setIsShowBeforBurn(bool)));


    connect(LogicMain::instance(), &LogicMain::exit, [=](){m_bAppRunning = false;});

    if (!m_oBlockName.startsWith("/dev/"))
    {
        m_bCDROM = false;
        m_oDisplay.clear();
        m_oDisplay = _("No need burner.");
    }

    if (!m_bCDROM) changeISOName(m_oSetting->getVolumnID());

    m_unique_uuid = QUuid::createUuid().toString();

    use();

}

DataCDROM::DataCDROM(DataCDROM &other, QString blockName, QObject *parent) :
    QObject(parent)
{
    m_oBlockName = blockName;
    m_bCDROM = true;
    m_bStart = false;
    m_bAppRunning = true;
    m_iHandle = -1;
    m_is_using = 0;
    m_oMedium = nullptr;
    m_oSetting = nullptr;
    m_bUpdateStart = false;

    if (!m_oBlockName.startsWith("/dev/"))
    {
        m_bCDROM = false;
        m_oDisplay.clear();
        m_oDisplay = _("No need burner.");
    }
    use();
}

DataCDROM * DataCDROM::clone()
{
    DataCDROM *cdrom = new DataCDROM(*this, this->m_oBlockName);
    cdrom->m_oSetting = new DataSetting(*this->m_oSetting,cdrom);
    cdrom->m_oMedium = new DataMedium(*this->m_oMedium, cdrom);
    cdrom->m_iHandle = this->m_iHandle;
    cdrom->m_bSupportBD = this->m_bSupportBD;
    cdrom->m_u32MediumRSupport = this->m_u32MediumRSupport;
    cdrom->m_u32MediumWSupport = this->m_u32MediumWSupport;
    cdrom->m_u32BurnModeSupport = this->m_u32BurnModeSupport;
    cdrom->m_bEjected = this->m_bEjected;
    cdrom->m_bCDROM = this->m_bCDROM;
    cdrom->m_bAppRunning = this->m_bAppRunning;
    cdrom->m_oDisplay = this->m_oDisplay;
    cdrom->m_oVendor = this->m_oVendor;
    cdrom->m_oProductor = this->m_oVendor;
    cdrom->m_oBlockName = this->m_oBlockName;
    cdrom->m_oMediumType = this->m_oMediumType;
    cdrom->m_is_using = 1000; //防止光驱拔出后，触发release函数导致的析构函数
    connect(cdrom, &DataCDROM::updateCDROMBurnStart, this, &DataCDROM::slotsUpdateCDROMBurnStart);

    return cdrom;
}

bool DataCDROM::umountMedium(QString device)
{
    int processExitStatus, processExitCode;
    QString umountOutput;
    QStringList umountArgs;
    QProcess umountProcess;

    if (device.isEmpty()) {
        LOGGER->info(QString("(DataCDROM::umount)  disc(%1) is empty, umount fail").arg(device));
        return false;
    }

    umountArgs << "-v" << device;
    umountProcess.setProcessChannelMode(QProcess::MergedChannels);
    umountProcess.start("umount", umountArgs);
    umountProcess.waitForFinished(-1);
    umountOutput = umountProcess.readAll();
    umountProcess.close();

    processExitCode = umountProcess.exitCode();
    processExitStatus = umountProcess.exitStatus();
    LOGGER->info(QString("(DataCDROM::umount)  disc(%1) processExitCode = %2， processExitStatus = %3").arg(device).arg(processExitCode).arg(processExitStatus));
    LOGGER->info(QString("(DataCDROM::umount)  disc(%1) umount  output : {%2}").arg(device).arg(umountOutput));

    if (QProcess::NormalExit != processExitStatus ||  0 != processExitCode) {
        qDebug() << "(DataCDROM::umountProcess) processExitStatus = " << processExitStatus << " processExitCode = " << processExitCode;
        if (processExitCode == 1 && umountOutput.contains("not mounted")) {
            LOGGER->info(QString("(DataCDROM::disc(%1) has been umount").arg(device));
            return true;
        }
        LOGGER->info(QString("(DataCDROM::umount)  disc(%1)  umount fail").arg(device));
        return false;
    }

    if (umountOutput.contains("Error unmounting block device") || umountOutput.contains("target is busy")) {
        LOGGER->info(QString("(DataCDROM::umount)  disc(%1)  umount fail").arg(device));
        return false;
    }
    LOGGER->info(QString("(DataCDROM::umount)  disc(%1)  umount success").arg(device));
    return true;
}

bool DataCDROM::mountMedium(QString device)
{
    int processExitStatus, processExitCode;
    QString gioMountOutput;
    QStringList gioMountArgs;
    QProcess gioMountProcess;

    if (device.isEmpty()) {
        LOGGER->info(QString("(DataCDROM::mountMedium)  disc(%1) is empty, mount fail").arg(device));
        return false;
    }

    gioMountArgs << "mount" << "-d" << device;
    gioMountProcess.setProcessChannelMode(QProcess::MergedChannels);
    gioMountProcess.start("gio", gioMountArgs);
    gioMountProcess.waitForFinished(-1);
    gioMountOutput = gioMountProcess.readAll();
    gioMountProcess.close();

    processExitCode = gioMountProcess.exitCode();
    processExitStatus = gioMountProcess.exitStatus();
    LOGGER->info(QString("(DataCDROM::mountMedium)  disc(%1) processExitCode = %2， processExitStatus = %3").arg(device).arg(processExitCode).arg(processExitStatus));
    LOGGER->info(QString("(DataCDROM::mountMedium)  disc(%1) gio mount output : {%2}").arg(device).arg(gioMountOutput));

    if (QProcess::NormalExit != processExitStatus ||  0 != processExitCode) {
        LOGGER->info(QString("(DataCDROM::mountMedium) disc(%1) mount fail").arg(device));
        return false;
    }

    LOGGER->info(QString("(DataCDROM::mountMedium) disc(%1) mount success").arg(device));
    return true;
}

bool DataCDROM::update()
{
    connect(g_user_signal, &GlobalUserSignal::changeDiscStatus, this, &DataCDROM::receiveDiscStatusChange,Qt::UniqueConnection);
    use();
    QString blockName = m_oBlockName;
    int cnt = 3;
    if (!m_bCDROM)
    {
        release();
        LOGGER->info(QString("DataCDROM::initDevice Line:%1 非CDROM，退出，dev = %2").arg(__LINE__).arg(blockName));
        return false;
    }

    destroyMedium();

    m_oMediumType.clear();

    if (isOpen()){
        close();
    }

    while(cnt--) {
        if (!isOpen()) {
            if(!open()){
                LOGGER->warn(QString("( DataCDROM::update ) device = %1 , open device faild").arg(blockName));
            } else {
                LOGGER->info((QString("( DataCDROM::update ) device = %1 , open device success, cnt = %2").arg(blockName).arg(cnt)));
                break;
            }
        }
    }
    if(cnt <= 0 && !isOpen()) {
        LOGGER->info(QString("[%1] [%2] open:[%3] error 3 times").arg(__func__).arg(__LINE__).arg(blockName));
        qDebug() << QString("[%1] [%2] open:[%3] error 3 times").arg(__func__).arg(__LINE__).arg(blockName);
        release();
        return false;
    }
    // 1. vendor and descriptio
    display(blockName);
    // 2. load support mediu types.
    support();
    //checkCDROM();
    // 3. check have medium or not.
    checkMedium();
    close();
    release();
    LOGGER->info(QString("( DataCDROM::update ) Line:%1 运行完成，dev=%2").arg(__LINE__).arg(blockName));
    return true;
}

void DataCDROM::receiveDiscStatusChange(){
    LOGGER->info(QString("( DataCDROM::receiveDiscStatusChange ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if(m_is_using > 0 && isCDROM() && mediumStatus() >=0){
        sendMediumStatusToOtherView();
    }
}

//在update()后使用，需要判断（有光驱，有光盘）后使用
//在插入有光盘的光驱后，checkMedium会有异步可能会使得执行到这，光驱已经
//拔了，而没有delete掉，在发送光盘信号会导致界面出错。
void DataCDROM::sendMediumStatusToOtherView()
{
    LOGGER->info(QString("( DataCDROM::sendMediumStatusToOtherView ) Line:%1 进入函数 dev=%2").arg(__LINE__).arg(m_oBlockName));
    if (m_oMedium)
    {
        switch (m_oMedium->status())
        {
        case DataMedium::MEDIUM_EMPTY:
            emit empty();
            break;
        case DataMedium::MEDIUM_INCOMPLETE:
            if(!mediumMountPoint().isEmpty())
                emit appendale();
            break;
        case DataMedium::MEDIUM_FINALIZED:
            if(!mediumMountPoint().isEmpty())
                emit finished();
            break;
        default:
            break;
        }
    }
}


bool DataCDROM::umount()
{
    int processExitStatus, processExitCode;
    QString umountOutput, device;
    QStringList umountArgs;
    QProcess umountProcess;

    device = m_oMedium->getBlockName();

    if (device.isEmpty())
    {
        LOGGER->info(QString("(DataCDROM::umount)  disc(%1) is empty, umount fail").arg(device));
        return false;
    }

    umountArgs << "-v" << device;
    umountProcess.setProcessChannelMode(QProcess::MergedChannels);
    umountProcess.start("umount", umountArgs);
    umountProcess.waitForFinished(-1);
    umountOutput = umountProcess.readAll();
    umountProcess.close();

    processExitCode = umountProcess.exitCode();
    processExitStatus = umountProcess.exitStatus();
    LOGGER->info(QString("(DataCDROM::umount)  disc(%1) processExitCode = %2， processExitStatus = %3").arg(device).arg(processExitCode).arg(processExitStatus));
    LOGGER->info(QString("(DataCDROM::umount)  disc(%1) umount  output : {%2}").arg(device).arg(umountOutput));

    if (QProcess::NormalExit != processExitStatus ||  0 != processExitCode) {
        LOGGER->info(QString("(DataCDROM::umount)  disc(%1)  umount fail").arg(device));
        return false;
    }

    if (umountOutput.contains("Error unmounting block device") || umountOutput.contains("target is busy")) {
        LOGGER->info(QString("(DataCDROM::umount)  disc(%1)  umount fail").arg(device));
        return false;
    }
    LOGGER->info(QString("(DataCDROM::umount)  disc(%1)  umount success").arg(device));
    return true;
}

bool DataCDROM::eject()
{
    if (checkUsingOfOtherApp()) {
        return false;
    }

    int processExitStatus, processExitCode;
    QString ejectOutput, device;
    QStringList ejectArgs;
    QProcess ejectProcess;

    device = m_oBlockName;

    if (device.isEmpty())
    {
        LOGGER->info(QString("(DataCDROM::eject)  disc(%1) is empty, eject fail").arg(device));
        return false;
    }

    ejectArgs << "-v" << device;
    ejectProcess.setProcessChannelMode(QProcess::MergedChannels);
    ejectProcess.start("/usr/bin/eject", ejectArgs);
    ejectProcess.waitForFinished(-1);
    ejectOutput = ejectProcess.readAll();
    ejectProcess.close();

    processExitCode = ejectProcess.exitCode();
    processExitStatus = ejectProcess.exitStatus();
    LOGGER->info(QString("( DataCDROM::eject )  disc(%1) processExitCode = %2， processExitStatus = %3").arg(device).arg(processExitCode).arg(processExitStatus));
    LOGGER->info(QString("( DataCDROM::eject )  disc(%1) eject  output : {%2}").arg(device).arg(ejectOutput));

    if (QProcess::NormalExit != processExitStatus ||  0 != processExitCode) {
        LOGGER->info(QString("( DataCDROM::eject )  disc(%1)  eject fail").arg(device));

        if (doorIsOpen()) {
            eject(false);
        } else {
            eject(true);
        }
        return false;
    }

    if (doorIsOpen()) {
        eject(false);
    } else {
        eject(true);
    }

    LOGGER->info(QString("( DataCDROM::eject )  disc(%1)  eject success").arg(device));
    return true;
}

void DataCDROM::earse()
{
    if (checkUsingOfOtherApp()) {
        return;
    }
    
    if (isOpen()) {
        close();
    }

    //umount device
    bool umountSucceed = DataCDROM::umountMedium(device());
    if (!umountSucceed) {
        LOGGER->info(QString("(ProcessBurnData::working) cdrom run umount:%1").arg(umountSucceed));
    }

    QIcon progressIcon(QIcon::fromTheme("ukui-loading-0-symbolic"));
    Progress* progressDlg = static_cast<Progress*>(ModalDialog::instance()->showPage(ModalDialog::Progress));
    progressDlg->title(_("Earsing"));
    progressDlg->setsSetRange(0,100);
    progressDlg->slotsSetValue(0, progressIcon,  _("earsing...."));
    ModalDialog::instance()->cancelEnable(false);
    ModalDialog::instance()->show();

    BurnJobManager::instance()->disconnect();
    QSharedPointer<int> lastEraseProgress = QSharedPointer<int>::create(0);
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [progressDlg, lastEraseProgress](int value) {
        qDebug() << "ProgressValue is:" << value;
        if (value < 0 || value > 100) {
            qDebug() << "Ignore invalid erase progress:" << value;
            return;
        }

        if (value < *lastEraseProgress) {
            qDebug() << "Ignore rollback erase progress:" << *lastEraseProgress << "->" << value;
            return;
        }

        if (value < 100 && *lastEraseProgress < 10 && value >= 50) {
            qDebug() << "Ignore abnormal erase progress jump:" << *lastEraseProgress << "->" << value;
            return;
        }

        *lastEraseProgress = value;
        if(value >= 100){
            progressDlg->slotsSetValueWithoutClosing(value, _("Fixating(Will wait minutes)"));
        }else{
            progressDlg->slotsSetValueWithoutClosing(value, _("earsing...."));
        }
    },Qt::UniqueConnection);

    //Qt::UniqueConnection doesn't work
    disconnect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, 0, 0);
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){

        emit g_user_signal->noMedium(this);
        eject();

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();
        if(result){
            Tip_BurnResult::instance()->slotsSuccess(_("Earse"));
        }else{
            Tip_BurnResult::instance()->slotsFailed( _("Earse"));
        }

    });

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startEraseDisc(device());
    loop.exec();
}

bool DataCDROM::isEjected()
{
    m_bEjected = doorIsOpen();
    return m_bEjected;
}


/***
 *
 * return: states of this datacdrom is using by other app
 */
bool DataCDROM::checkUsingOfOtherApp(bool isHideModalDialog)
{
    ProcessLsof lsofPro;
    if (lsofPro.checkDevice(this)) {
        const QList<ProcessLsof::Process>& apps = lsofPro.usingApplications();
        if (apps.count() > 0) {
            QString errInfo = _("Device '%1' is used by the '%2' application! Please try again later").arg(this->display()).arg(lsofPro.joinProcessNames(apps));
            if (isHideModalDialog && !ModalDialog::instance()->isHidden()) {
                ModalDialog::instance()->hide();
            }
            Tip_BurnResult::instance()->slotsWarn(errInfo);
            return true;
        }
    }
    return false;
}

QString DataCDROM::mDisplay()
{
    if (m_bCDROM)
        return m_oMedium ? m_oMedium->display() : _("No medium");
    else
        return m_oBlockName;
}

QIcon DataCDROM::mediumIcon()
{
    return m_oMedium ? m_oMedium->icon() : QIcon::fromTheme("application-x-cd-image");
}

DataSetting *DataCDROM::getSettings()
{
    if(m_oSetting){
        return m_oSetting;
    }
    return nullptr;
}

void DataCDROM::updateVerficationData(bool b)
{
    if (m_oSetting) {
        m_oSetting->updateVerficationData(b);
    } else {
    }
}

bool DataCDROM::open()
{
    use();
    if (-1 != m_iHandle)
    {
        release();
        return true;
    }
    m_iHandle = ::open(m_oBlockName.toUtf8().constData(), O_NONBLOCK | O_CLOEXEC | O_RDONLY);
    if (m_iHandle < 0)
    {
        LOGGER->error(_("Open block device %1 faied.%2").arg(m_oBlockName).arg(strerror(errno)));
        release();

        return false;
    }
    release();
    return true;
}

void DataCDROM::close()
{
    use();
    if (-1 != m_iHandle) {
        int ret = ::close(m_iHandle);
        if (ret != 0) {
            LOGGER->warn(QString("( DataCDROM::close ) evice = %1, handle = %2 ,close device failed").arg(m_oBlockName).arg(m_iHandle));
        }
        LOGGER->debug(QString("( DataCDROM::close ) close device = %1, handle = %2").arg(m_oBlockName).arg(m_iHandle));
    }
    m_iHandle = -1;
    release();
}

bool DataCDROM::execSCSI(const unsigned char *cdb, const int cdbLength,
                         unsigned char *result, const int resultLength)
{
    sg_io_hdr_t sgio;

    if (!isOpen()) {
        if (!open()) {
            LOGGER->warn(QString("( DataCDROM::execSCSI ) device = %1 , open device faild").arg(m_oBlockName));
            return false;
        }
    } 
    if (!cdb || !result || resultLength < 1) return false;
    memset(&sgio, 0, sizeof(sg_io_hdr_t));
    sgio.interface_id = 'S';
    sgio.dxfer_direction = SG_DXFER_FROM_DEV;
    sgio.flags = SG_FLAG_LUN_INHIBIT;
    sgio.cmdp = (unsigned char *)cdb;
    sgio.cmd_len = cdbLength;
    sgio.dxferp = result;
    sgio.dxfer_len = resultLength;

    if (0 != ioctl(m_iHandle, SG_IO, &sgio))
    {
        // LOGGER->error(_("Do SCSI CDB [%1] failed.%2").arg(cdb[0]).arg(strLOGGER->error(errno)));
        return false;
    }
    return true;
}

bool DataCDROM::execSCSI(int fd, const unsigned char *cdb, const int cdbLength,
                         unsigned char *result, const int resultLength)
{
    sg_io_hdr_t sgio;

    if (!cdb || !result || resultLength < 1) return false;
    memset(&sgio, 0, sizeof(sg_io_hdr_t));
    sgio.interface_id = 'S';
    sgio.dxfer_direction = SG_DXFER_FROM_DEV;
    sgio.flags = SG_FLAG_LUN_INHIBIT;
    sgio.cmdp = (unsigned char *)cdb;
    sgio.cmd_len = cdbLength;
    sgio.dxferp = result;
    sgio.dxfer_len = resultLength;

    if (0 != ioctl(fd, SG_IO, &sgio))
    {
        return false;
    }
    return true;
}

bool DataCDROM::read10(unsigned char *data, unsigned int dataLen, unsigned long startAdress, unsigned int length, bool fua)
{
    ::memset( data, 0, dataLen );

    DiscScsi cmd(m_oBlockName);
    cmd[0] = 0x28; //  MMC_READ_10
    cmd[1] = ( fua ? 0x8 : 0x0 );
    cmd[2] = startAdress>>24;
    cmd[3] = startAdress>>16;
    cmd[4] = startAdress>>8;
    cmd[5] = startAdress;
    cmd[7] = length>>8;
    cmd[8] = length;
    cmd[9] = 0;      // Necessary to set the proper command length

    if( cmd.transport( TR_DIR_READ, data, dataLen ) ) {
        qDebug()  << m_oBlockName << ": READ 10 failed!";
        return false;
    }
    else
        return true;
}
void DataCDROM::updateMediumUsedSize(unsigned long size)
{
    if (0 == size) return;
    if (m_oMedium && m_oMedium->status() == DataMedium::MEDIUM_FINALIZED)
    {
        m_oMedium->setUsedSize(size);
    }
}

void DataCDROM::prepare()
{
    unsigned char cdb[6] = { 0x00,
                              0x00,
                              0x00,
                              0x00,
                              0x00,
                              0 };
    unsigned char result[8] = { 0 };

    //open函数打开设备失败3次，强制退出该函数
    int cnt = 3;
    do
    {
        if (cnt-- <= 0) {
            LOGGER->info(QString("[%1] [%2] [%3] --- cnt = [%4]-------break 退出了---\n").arg(__func__).arg(__LINE__).arg(m_oBlockName).arg(cnt));
            break;
        }
        else LOGGER->info(QString("[%1] [%2] [%3] --- cnt = [%4]-------").arg(__func__).arg(__LINE__).arg(m_oBlockName).arg(cnt));
    } while (!execSCSI(cdb, 6, result, 8));
    LOGGER->info(QString("[%1] [%2] [%3] --- cnt = [%4]---完成了----").arg(__func__).arg(__LINE__).arg(m_oBlockName).arg(cnt));
}

void DataCDROM::slotsUpdateCDROMBurnStart(bool b)
{
    m_bStart = b;
}

QString DataCDROM::getUnique_uuid() const
{
    return m_unique_uuid;
}


QString DataCDROM::display(QString blockName)
{
    Q_UNUSED(blockName);
    LOGGER->info(QString("( DataCDROM::display ) Line:%1, 进入函数， dev = %2").arg(__LINE__).arg(m_oBlockName));
    unsigned char  cdb[6] = {0x12, // inquiry
                             0x00,
                             0x00,
                             0x00,
                             0x24, // result length : 36
                             0
                            };
    char vendor[9] = {0}; // 最多使用8个字节，第9位赋值为0
    char productor[16] = {0};
    unsigned char  result[36] = {0};
    unsigned char *p = result;

    memset(vendor, 0, 9);
    memset(productor, 0, 16);
    memset(result, 0, 36);

    use();

    m_oVendor.clear(); m_oProductor.clear();
    m_oDisplay = _("Unkown.");
    if (!execSCSI(cdb, 6, result, 36))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::display ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
        return m_oDisplay;
    }
    ::memcpy(vendor, p+8, 8);
    m_oVendor.append(QString::fromUtf8(vendor));
    m_oVendor = m_oVendor.trimmed();
    LOGGER->debug(_("Vendor is %1").arg(m_oVendor));
    ::memcpy(productor, p + 16, 15);
    m_oProductor.append(QString::fromUtf8(productor));
    m_oProductor = m_oProductor.trimmed();
    LOGGER->debug(_("Productor is %1").arg(m_oProductor));
    m_oDisplay.clear();
    m_oDisplay.append(m_oVendor);
    m_oDisplay.append(" ");
    m_oDisplay.append(m_oProductor);
    release();
    LOGGER->info(QString("( DataCDROM::display ) Line:%1, 运行完成， dev = %2, m_oDisplay = %3").arg(__LINE__).arg(m_oBlockName).arg(m_oDisplay));
    return m_oDisplay;
}

bool DataCDROM::doorIsOpen()
{
    int deviceHandle = ::open(m_oBlockName.toUtf8().constData(), O_NONBLOCK | O_RDONLY);
    if (deviceHandle < 0)
    {
        LOGGER->error(QString("(DataCDROM::doorIsOpen()) device = %1， open failed:%2 , return退出 ").arg(m_oBlockName).arg(strerror(errno)));
        release();
        return false;
    }
    unsigned char cdb[10] = { 0x4A,
                              0x01,
                              0x00,
                              0x00,
                              0x10, // bit 4 |7 6 5 4 3 2 1 0|
                              0x00,
                              0x00,
                              0x00,
                              0x08,
                              0 };
    unsigned char result[8] = { 0 };
    int ret = -1;
    if (!execSCSI(deviceHandle, cdb, 10, result, 8)){
        ret = ::close(deviceHandle);
        if (ret != 0) {
            LOGGER->warn(QString("( DataCDROM::doorIsOpen ) device = %1, handle = %2 ,close device failed, line:%3").arg(m_oBlockName).arg(deviceHandle).arg(__LINE__));
        }
        return false;
    }

    ret = ::close(deviceHandle);
    if (ret != 0) {
        LOGGER->warn(QString("( DataCDROM::doorIsOpen ) device = %1, handle = %2 ,close device failed, line:%3").arg(m_oBlockName).arg(deviceHandle).arg(__LINE__));
    }

    if (result[5] & 0x01) {
        return true;
    }
    return false;
}

void DataCDROM::setVolumnID(QString str)
{
    emit m_oSetting->volumnID(str);
}

void DataCDROM::destroyMedium()
{
    if(m_oMedium)
    {
        delete m_oMedium;
        m_oMedium = nullptr;
    }
}

void DataCDROM::support()
{
    LOGGER->info(QString("( DataCDROM::support ) Line:%1, 进入函数， dev = %2").arg(__LINE__).arg(m_oBlockName));
    unsigned short len = 0;
    unsigned char  cdb[12] = { 0x46, // get configuraion
                               0x00,
                               0x00,
                               0x00, // fearture low
                               0x00, // reserved
                               0x00, // reserved
                               0x00, // reserved
                               0x00, // allocation length high
                               0x0C,
                               0 };
    unsigned char  result[65536] = {0};

    // get header length first.
    if (!execSCSI(cdb, 12, result, 12)) {
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
        return;
    }
    if (!(result[8] == cdb[2] && result[9] == cdb[3])) {
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
        return;
    }

    len = (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | result[3];
    cdb[7] = ((len & 0xFF00) >> 8);
    cdb[8] = (len & 0x00FF);
    // get real result.
    if (!execSCSI(cdb, 12, result, len)) {
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
        return;
    }
    len = result[11];

    use();
    m_u32MediumRSupport = 0;
    m_u32MediumWSupport = 0;
    m_u32BurnModeSupport = 0;

    // loop result, adjust support read medium type.
    for (int i = 0; i < len; ++i)
    {
        //QCoreApplication::processEvents();
        switch ((result[i + 12] << 8) | result[i + 13])
        {
        case 0x0008: // CD-ROM
            m_u32MediumRSupport |= DataMedium::MEDIUM_CD_ROM;
            LOGGER->info(_("%1 support read mode : CD-ROM").arg(m_oDisplay));
            break;
        case 0x0009: // CD-R
            m_u32MediumRSupport |= DataMedium::MEDIUM_CD_R;
            LOGGER->info(_("%1 support read mode : CD-R").arg(m_oDisplay));
            break;
        case 0x000A: // CD-RW
            m_u32MediumRSupport |= DataMedium::MEDIUM_CD_RW;
            LOGGER->info(_("%1 support read mode : CD-RW").arg(m_oDisplay));
            break;
        // 000B - 000F reserved.
        case 0x0010: // DVD-ROM
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_ROM;
            LOGGER->info(_("%1 support read mode : DVD-ROM").arg(m_oDisplay));
            break;
        case 0x0011: // DVD-R
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_R;
            LOGGER->info(_("%1 support read mode : DVD-R").arg(m_oDisplay));
            break;
        case 0x0012: // DVD-RAM
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_RAM;
            LOGGER->info(_("%1 support read mode : DVD-RAM").arg(m_oDisplay));
            break;
        // define in MMC-5
        case 0x0013: // DVD-RW
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_RW_OVERWRITE;
            LOGGER->info(_("%1 support read mode : DVD-RW(OVERWRITE)").arg(m_oDisplay));
            break;
        case 0x0014:
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_RW_SEQ;
            LOGGER->info(_("%1 support read mode : DVD-RW(SEQ)").arg(m_oDisplay));
            break;
        case 0x0015:
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_R_DL_SEQ;
            LOGGER->info(_("%1 support read mode : DVD-R-DL(SEQ)").arg(m_oDisplay));
            break;
        case 0x0016:
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_R_DL_JUMP;
            LOGGER->info(_("%1 support read mode : DVD-R-DL(JUMP)").arg(m_oDisplay));
            break;
        case 0x001A: // DVD+RW
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_PLUS_RW;
            LOGGER->info(_("%1 support read mode : DVD+RW").arg(m_oDisplay));
            break;
        case 0x001B: //DVD+R
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_PLUS_R;
            LOGGER->info(_("%1 support read mode : DVD+R").arg(m_oDisplay));
            break;
        case 0x002B:
            m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_PLUS_R_DL;
            LOGGER->info(_("%1 support read mode : DVD+R-DL").arg(m_oDisplay));
            break;
        case 0x0040:
            m_u32MediumRSupport |= DataMedium::MEDIA_BD_ROM;
            m_bSupportBD = true;
            LOGGER->info(_("%1 support read mode : BD-ROM").arg(m_oDisplay));
            break;
        case 0x0041:
            m_u32MediumRSupport |= DataMedium::MEDIA_BD_R_SRM;
            m_bSupportBD = true;
            LOGGER->info(_("%1 support read mode : BD-R SRM").arg(m_oDisplay));
            break;
        case 0x0042:
            m_u32MediumRSupport |= DataMedium::MEDIA_BD_R_RRM;
            m_bSupportBD = true;
            LOGGER->info(_("%1 support read mode : BD-R PPM").arg(m_oDisplay));
            break;
        case 0x0043:
            m_u32MediumRSupport |= DataMedium::MEDIA_BD_RE;
            m_bSupportBD = true;
            LOGGER->info(QString("%1 support read mode : BD-RE").arg(m_oDisplay));
            break;
        case 0x0002:
            LOGGER->debug(_("Removable CDROM."));
            break;
        default:
            break;
        }
    }

    // load write suppport
    // CD TAO
    cdb[2] = 0x00; cdb[3] = 0x2D; cdb[7] = 0x00; cdb[8] = 0x10;

    if (!execSCSI(cdb, 12, result, 16))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回(CD TAO)").arg(__LINE__));
        return;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3])
    {
        m_u32BurnModeSupport |= BURN_CD_TAO;
        LOGGER->info(_("%1 supprot burn CD by mode TAO").arg(m_oDisplay));
        m_u32MediumWSupport |= DataMedium::MEDIUM_CD_R;
        LOGGER->info(_("%1 support write mode : CD-R").arg(m_oDisplay));
        if (result[12] & 0x02)
        {
            m_u32MediumWSupport |= DataMedium::MEDIUM_CD_RW;
            LOGGER->info(_("%1 support write mode : CD-RW").arg(m_oDisplay));
        }
    }
    //CD SAO
    cdb[2] = 0x00; cdb[3] = 0x2E; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回(CD SAO)").arg(__LINE__));
        return;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3])
    {
        m_u32MediumWSupport |= DataMedium::MEDIUM_CD_R;
        LOGGER->info(_("%1 support write mode : CD-R").arg(m_oDisplay));
        if (result[12] & 0x02)
        {
            m_u32MediumWSupport |= DataMedium::MEDIUM_CD_RW;
            LOGGER->info(_("%1 support write mode : CD-RW").arg(m_oDisplay));
        }
        if (result[12] & 0x20)
        {
            m_u32BurnModeSupport |= BURN_CD_SAO;
            LOGGER->info(_("%1 supprot burn CD by mode SAO").arg(m_oDisplay));
        }
    }
    //DVD
    cdb[2] = 0x00; cdb[3] = 0x1F; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回(DVD)").arg(__LINE__));
        return;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3])
    {
        m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_ROM;
        LOGGER->info(_("%1 sure support read mode : DVD-ROM").arg(m_oDisplay));
    }
    //DVD+RW
    cdb[2] = 0x00; cdb[3] = 0x2A; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回(DVD+RW)").arg(__LINE__));
        return;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3])
    {
        if (result[12] & 0x01)
        {
            m_u32MediumWSupport |= DataMedium::MEDIUM_DVD_PLUS_RW;
            LOGGER->info(_("%1 support write mode : DVD+RW").arg(m_oDisplay));
        }
        else
        {
            m_u32MediumWSupport |= DataMedium::MEDIUM_DVD_ROM;
            LOGGER->info(_("%1 support write mode : DVD-ROM").arg(m_oDisplay));
        }
    }
    //DVD+R
    cdb[2] = 0x00; cdb[3] = 0x2B; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回(DVD+R)").arg(__LINE__));
        return;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3])
    {
        m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_PLUS_R;
        LOGGER->info(_("%1 sure support read mode : DVD+R").arg(m_oDisplay));
        if (result[12] & 0x01)
        {
            m_u32MediumWSupport |= DataMedium::MEDIUM_DVD_PLUS_R;
            LOGGER->info(_("%1 support write mode : DVD+RW").arg(m_oDisplay));
        }
    }
    //DVD-R / DVD-RW
    cdb[2] = 0x00; cdb[3] = 0x2F; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::support ) Line:%1 执行execSCSI失败返回(DVD-R(W))").arg(__LINE__));
        return;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3])
    {
        m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_R;
        LOGGER->info(_("%1 sure support read mode : DVD-R").arg(m_oDisplay));
        m_u32MediumWSupport |= DataMedium::MEDIUM_DVD_R;
        LOGGER->info(_("%1 support write mode : DVD-R").arg(m_oDisplay));
        m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_RW;
        LOGGER->info(_("%1 sure support read mode : DVD-RW").arg(m_oDisplay));
        if (result[12] & 0x02)
        {
            m_u32MediumWSupport |= DataMedium::MEDIUM_DVD_RW;
            LOGGER->info(_("%1 support write mode : DVD-RW").arg(m_oDisplay));
        }
    }
    if (m_u32MediumRSupport & ((DataMedium::MEDIUM_DVD_RW_OVERWRITE | DataMedium::MEDIUM_DVD_RW_SEQ)))
    {
        m_u32MediumRSupport |= DataMedium::MEDIUM_DVD_RW;
        LOGGER->info(_("%1 support read mode : DVD-RW from adjust.").arg(m_oDisplay));
    }
    LOGGER->info(QString("( DataCDROM::support) Line:%1 执行完成 dev = %1").arg(__LINE__).arg(m_oBlockName));
    release();
}

void DataCDROM::checkMedium()
{
    unsigned short profile = 0;
    unsigned int len = 0;
    unsigned char  cdb[12] = {0x46, // operation code
                              0x01, // RT : 0x10b
                              0x00, // fearture high
                              0x00, // fearture low
                              0x00, // reserved
                              0x00, // reserved
                              0x00, // reserved
                              0x00, // allocation length high
                              0x0C, // allocation length low
                              0};
    unsigned char result[65536] = {0};

    LOGGER->info(QString("( DataCDROM::checkMedium) Line:%1，进入函数 dev = %1").arg(__LINE__).arg(m_oBlockName));
    use();

    m_oMediumType.clear();

    if (doorIsOpen())
    {
        m_bEjected = true;
        m_oMediumType.clear();

        destroyMedium();

        LOGGER->info(_("No medium.Because the cdrom's door is open."));
        LOGGER->info(QString("( DataCDROM::checkMedium ) cdrom door has opend，prepare close device(%1), handle = %2").arg(m_oBlockName).arg(m_iHandle));
        if (isOpen()) {
            close();
        }
        eject(false);

        release();
        return;
    }
    m_bEjected = false;
    eject(true);

    if (!execSCSI(cdb, 12, result, 12))
    {
        release();
        LOGGER->warn(QString("( DataCDROM::checkMedium ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
        return;
    }
    if (!(result[8] == cdb[2] && result[9] ==cdb[3]))
    {
        LOGGER->warn(QString("( DataCDROM::checkMedium ) Line:%1 execSCSI返回值不符合返回").arg(__LINE__));
        release();
        return;
    }
    profile = (result[6] << 8) | result[7];
    if (0x0000 == profile) // 未读取到定义类型
    {
        // 再次尝试读取全量的返回信息
        len = result[11];
        if (len > 65536) len = 65536;
        cdb[7] = ((len & 0xFF00) >> 8);
        cdb[8] = (len & 0x00FF);
        if (!execSCSI(cdb, 12, result, len))
        {
            release();
            LOGGER->warn(QString("( DataCDROM::checkMedium ) Line:%1 执行execSCSI失败返回").arg(__LINE__));
            return;
        }
        for (int i = 0; i < len; ++i)
        {
            //QCoreApplication::processEvents();
            if (result[14 + i] & 0x01)
            {
                profile = (result[12 + i] << 8) | result[13 + i];
                break;
            }
        }
    }

    switch (profile)
    {
    case 0x0008: // CD-ROM
        if (m_u32MediumRSupport & DataMedium::MEDIUM_CD_ROM)
        {
            m_oMediumType = "CD-ROM";
            LOGGER->info(_("Loaded CD-ROM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read CD-ROM medium"));
        break;
    case 0x0009: //CD-R
        if (m_u32MediumRSupport & DataMedium::MEDIUM_CD_R)
        {
            m_oMediumType = "CD-R";
            LOGGER->info(_("Loaded CD-R medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read CD-R medium"));
        break;
    case 0x000A: // CD-RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_CD_RW)
        {
            m_oMediumType = "CD-RW";
            LOGGER->info(_("Loaded CD-RW medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read CD-RW medium"));
        break;
    case 0x0010: // DVD-ROM
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_ROM)
        {
            m_oMediumType = "DVD-ROM";
            LOGGER->info(_("Loaded DVD-ROM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-ROM medium"));
        break;
    case 0x0011: // DVD-R
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_R)
        {
            m_oMediumType = "DVD-R";
            LOGGER->info(_("Loaded DVD-R medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-R medium"));
        break;
    case 0x0012: // DVD-RAM
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_RAM)
        {
            m_oMediumType = "DVD-RAM";
            LOGGER->info(_("Loaded DVD-RAM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-RAM medium"));
        break;
    case 0x0013: // DVD-RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_RW_OVERWRITE)
        {
            m_oMediumType = "DVD-RW";
            LOGGER->info(_("Loaded DVD-RW(OVERWRITE) medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-RW(OVERWRITE) medium"));
        break;
    case 0x0014: // DVD-RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_RW_SEQ)
        {
            m_oMediumType = "DVD-RW";
            LOGGER->info(_("Loaded DVD-RW medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD-RW medium"));
        break;
    case 0x001A: // DVD+RW
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_PLUS_RW)
        {
            m_oMediumType = "DVD+RW";
            LOGGER->info(_("Loaded DVD+RW medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD+RW medium"));
        break;
    case 0x001B: // DVD+R
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_PLUS_R)
        {
            m_oMediumType = "DVD+R";
            LOGGER->info(_("Loaded DVD+R medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD+R medium"));
        break;
    case 0x002B: // DVD+R DL
        if (m_u32MediumRSupport & DataMedium::MEDIUM_DVD_PLUS_R_DL)
        {
            m_oMediumType = "DVD+R DL";
            LOGGER->info(_("Loaded DVD+R DL medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read DVD+R DL medium"));
        break;
    case 0x40:
        if (m_u32MediumRSupport & DataMedium::MEDIA_BD_ROM)
        {
            m_oMediumType = "BD-ROM";
            LOGGER->info(_("Loaded BD-ROM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read BD-ROM medium"));
        break;
    case 0x41:
        if (m_u32MediumRSupport & DataMedium::MEDIA_BD_R_SRM) {
            m_oMediumType = "BD-R SRM";
            LOGGER->info(_("Loaded BD-R SRM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read BD-R SRM medium"));
        break;
    case 0x42:
        if (m_u32MediumRSupport & DataMedium::MEDIA_BD_R_RRM)
        {
            m_oMediumType = "BD-R RRM";
            LOGGER->info(_("Loaded BD-R RRM medium in %1").arg(m_oDisplay));
            break;
        }
        LOGGER->error(_("CDROM %1 cannot support to read BD-R RRM medium"));
        break;
    default:
        break;
    }
    if (!m_oMediumType.isEmpty())
    {
        destroyMedium();
        m_oMedium = new DataMedium(m_oMediumType, m_oBlockName, this);
        m_oMedium->load();
        LOGGER->info(_("Created medium of type %1.").arg(m_oMediumType));
        release();
        return;
    }
    LOGGER->debug(_("There is no mediun in CDROM"));
    release();
    LOGGER->info(QString("( DataCDROM::checkMedium ) 运行完成 dev = %1").arg(m_oBlockName));
}

bool DataCDROM::loading()
{
    LOGGER->info(QString("( DataCDROM::loading ) 进入函数 dev = %1").arg(m_oBlockName));
    unsigned char cdb[10] = { 0x4A,
                              0x01,
                              0x00,
                              0x00,
                              0x10,
                              0x00,
                              0x00,
                              0x00,
                              0x08,
                              0 };
    unsigned char result[8] = { 0 };
    if (!execSCSI(cdb, 10, result, 8)) {
        LOGGER->warn(QString("( DataCDROM::loading ) Line:%1 execSCSI执行失败 dev=%2").arg(__LINE__).arg(m_oBlockName));
        return false;
    }
    if (result[5] & 0x0F) {
        LOGGER->warn(QString("( DataCDROM::loading ) Line:%1 execSCSI执行失败 dev=%2").arg(__LINE__).arg(m_oBlockName));
        return true;
    }
    return false;
}

bool DataCDROM::checkCDROM()
{
    int ret;
    unsigned char buf[4096] = {0};

    if (!isOpen()) {
        if (!open()) {
            LOGGER->warn(QString("( DataCDROM::checkCDROM ) device = %1 , open device faild").arg(m_oBlockName));
        }
    }
    ret = ::read(m_iHandle, buf, 4096);
    if (ret <= 0) return false;

    return true; // have mediun.
}

void DataCDROM::changeISOName(QString name)
{
    QFileInfo f;
    QStringList after;
    if (!m_bCDROM)
    {
        f.setFile(m_oBlockName);
        if (f.isDir())
        {
            return;
        }
        after = m_oBlockName.split(f.baseName());
        m_oBlockName = after.first();
        m_oBlockName.append(name).append(after.last());
    }
}

void DataCDROM::changeISOPath(QString path)
{
    QFileInfo f;
    if (!m_bCDROM)
    {
        f.setFile(m_oBlockName);
        m_oBlockName = path + "/" + f.fileName();
        setSettingVolumnID(m_oSetting->getVolumnID());
    }
}


DataCDROM::~DataCDROM()
{
    LOGGER->info(QString("( DataCDROM::~DataCDROM() ) close device = %1").arg(m_oBlockName));
    if (-1 != m_iHandle) {
        LOGGER->info(QString("( DataCDROM::~DataCDROM() ) close device = %1, handle = %2").arg(m_oBlockName).arg(m_iHandle));
        ::close(m_iHandle);
    }
    m_iHandle = -1;

    destroyMedium();

}

bool DataCDROM::isOpen()
{
    return m_iHandle >= 0;
}

bool DataCDROM::isUdf()
{
    if (m_oMedium) {
        if (m_oMedium->getFileSystemType().contains("udf")) {
            return true;
        }
    }
    return false;
}

MEDIUM_SYSTEM_TYPE DataCDROM::fileSystemTypeWithExtendName()
{
    MEDIUM_SYSTEM_TYPE sysType = UNKNOWN;
    if (m_oMedium) {
        QString fileSysMainName = m_oMedium->getFileSystemType();
        QString fileSysExtendName = m_oMedium->getFileSystemExtendType();
        QString fullSysName = fileSysMainName + fileSysExtendName;

        if(fullSysName == QString("udf1.02")){
            sysType = UDF102;
        }else if(fullSysName == QString("udf2.01")){
            sysType = UDF201;
        }else if(fullSysName.startsWith("iso9660")){
            sysType = ISO9660;
        }else{
            sysType = UNKNOWN;
        }
    }
    return sysType;
}

void DataCDROM::setSettingSpeed(int speed)
{
    m_oSetting->updateSpeed(speed);
}
void DataCDROM::setSettingClean(bool isClean)
{
    m_oSetting->updateClean(isClean);
}
void DataCDROM::setSettingCreateISO(bool isCreateISO)
{
    m_oSetting->updateCreateISO(isCreateISO);
}
void DataCDROM::setSettingCloseSession(bool isCloseSession)
{
    m_oSetting->updateCloseSession(isCloseSession);
}
void DataCDROM::setSettingVerficationData(bool isVerficationData)
{
    m_oSetting->updateVerficationData(isVerficationData);
}
void DataCDROM::setSettingSetOpen(bool statesOfOpen)
{
    m_oSetting->updateOpen(statesOfOpen);
}
void DataCDROM::setSettingCopyright(QString strCopyright)
{
    m_oSetting->updateCopyright(strCopyright);
}
void DataCDROM::setSettingPreparer(QString strPreparer)
{
    m_oSetting->updatePreparer(strPreparer);
}
void DataCDROM::setSettingPublisher(QString strPublisher)
{
    m_oSetting->updatePublisher(strPublisher);
}
void DataCDROM::setSettingVolumnID(QString strVolumnID)
{
    m_oSetting->updateVolumnID(strVolumnID);
    emit volumnID(strVolumnID);
    changeISOName(strVolumnID);

}
void DataCDROM::setSettingTempPath(QString strTempPath)
{
    m_oSetting->updateTempPath(strTempPath);
    emit tempPath(strTempPath);
}
void DataCDROM::setSettingBurnFormat(QString strBurnFormat)
{
    m_oSetting->updateBurnFormat(strBurnFormat);
}
void DataCDROM::setSettingIsShowBeforBurn(bool isShowBeforBurn)
{
    m_oSetting->updateIsShowBeforBurn(isShowBeforBurn);
}
