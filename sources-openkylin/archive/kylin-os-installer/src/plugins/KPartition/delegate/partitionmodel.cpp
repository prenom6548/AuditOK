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

#include "partitionmodel.h"
#include <QThread>
#include <QDebug>
#include "../PluginService/kthread.h"
#include "../partman/partition_server.h"
#include "../partman/partition_unit.h"

namespace KInstaller {
using namespace Partman;
using namespace KServer;
PartitionModel::PartitionModel(QObject *parent) : QObject(parent),
    partition_server(new PartitionServer),
    m_partThread(new QThread(this)),
    m_backQuickInstallThread(new QThread(this)),
    m_backCustomInstallThread(new QThread(this)),
    m_backFullProcess(new QuickInsatllerProcess),
    m_customProcess(new CustomInstallerProcess)
{
    partition_server->moveToThread(m_partThread);
    m_partThread->start();
    initAllConnects();
}

PartitionModel::~PartitionModel()
{
    quitThreadRun(m_backQuickInstallThread);
    quitThreadRun(m_backCustomInstallThread);
    quitThreadRun(m_partThread);
}

void PartitionModel::fullPart()
{

//    emit signalInstallQuickPart();

    m_backFullProcess->moveToThread(m_backQuickInstallThread);
    m_backQuickInstallThread->start();
    qDebug() << "m_backQuickInstallThread:" << m_backQuickInstallThread->currentThreadId();
//    emit partition_server->signalQuickPart();
}



void PartitionModel::createPartitionTable(QString dev_path)
{
    if(isEFIEnabled()) {
//        emit partition_server->signalCreatePartitionTable(dev_path, PartTableType::GPT);
        partition_server->doCreatePartitionTable(dev_path, PartTableType::GPT);
    } else {
        partition_server->doCreatePartitionTable(dev_path, PartTableType::MsDos);
//        emit partition_server->signalCreatePartitionTable(dev_path, PartTableType::MsDos);
    }
}

void PartitionModel::customPart(OperationDiskList operators,DeviceList virtualDevices)
{
    m_operators = operators;
    m_virtualDevices = virtualDevices;
    m_customProcess->moveToThread(m_backCustomInstallThread);
    m_backCustomInstallThread->start();
    qDebug() << "m_backCustomInstallThread:" << m_backCustomInstallThread->currentThreadId();

}

void PartitionModel::scanDevices()
{
    qDebug() << Q_FUNC_INFO ;
    const bool umount = true;
    const bool enable_os_prober = true;
//    emit partition_server->signalRefreshDevices(umount, enable_os_prober);
    partition_server->signalRefreshDevices(umount, enable_os_prober);
    qDebug() << Q_FUNC_INFO ;
}

void PartitionModel::initAllConnects()
{
//    connect(partition_server, &PartitionServer::signalQuikPartDone, [=]{m_backQuickInstallThread->start();});
//    connect(partition_server, &PartitionServer::signalCustomPartDone, [=](bool flag, DeviceList devs){
//        if(flag)
//            m_backCustomInstallThread->start();
//        else
//            emit customPartDone(1);
//   });
    connect(partition_server, &PartitionServer::signalDevicesRefreshed, [=](DeviceList devs){emit deviceRefreshed(devs);});

    connect(m_backCustomInstallThread, &QThread::started, [=]{
       // partition_server->doCustomPart(m_operators);
        partition_server->writeCustomPartInfo(m_operators,m_virtualDevices);
        m_customProcess->exec();
    });
    connect(m_backQuickInstallThread, &QThread::started, [=]{
        partition_server->doQuickPart();
        m_backFullProcess->exec();
    });

    connect(m_backQuickInstallThread, &QThread::finished, m_backFullProcess, &QuickInsatllerProcess::deleteLater);
    connect(m_backCustomInstallThread, &QThread::finished, m_customProcess, &CustomInstallerProcess::deleteLater);

//    connect(m_partThread, &QThread::started, this, &PartitionModel::scanDevices);


    //需要将信号先从准备安装发送给分区主窗口，在转到系统主窗口
    connect(m_backFullProcess, &QuickInsatllerProcess::signalProcessStatus, [=](int exitcode){emit fullPartDone(exitcode);});
    connect(m_customProcess, &CustomInstallerProcess::signalProcessStatus, [=](int exitcode){emit customPartDone(exitcode);});

//    qDebug() << "111111PartitionModel::initAllConnects" << connect(this, &PartitionModel::signalInstallCustomPart, this, &PartitionModel::customPart);
//    qDebug() << "111111PartitionModel::initAllConnects" << connect(this, &PartitionModel::signalInstallQuickPart, this, &PartitionModel::quickPart);
}



}
