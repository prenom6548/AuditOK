/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "machinemodel.h"
#include <QSettings>
#include <QProcess>
#include <QDebug>

std::shared_ptr<MachineModel> MachineModel::m_machineModel = nullptr;
std::mutex MachineModel::m_mutex;

MachineModel::MachineModel(QObject *parent) : QObject(parent)
{
    initMachineType();
}

QString MachineModel::getTheMachineType()
{
    return m_machineType;
}

std::shared_ptr<MachineModel> MachineModel::getMachineModelInstance()
{
    if (m_machineModel == nullptr) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_machineModel == nullptr) {
            m_machineModel = std::shared_ptr<MachineModel>(new MachineModel);
        }
    }
    return m_machineModel;
}

void MachineModel::initMachineType()
{
    QSettings setting(":/assets/data/conf.ini", QSettings::IniFormat);
    setting.beginGroup("MachineType"); //节点开始

    QString type = getSysVendor() + getProductFamily();
    qInfo() << __FILE__ << __LINE__ << type;
    if (setting.contains(type)) {
        m_machineType = setting.value(type).toString();
    } else {
        m_machineType = QString();
    }
    qInfo() << __FILE__ << __LINE__ << m_machineType;
    setting.endGroup(); //节点结束
}

const QString MachineModel::getSysVendor() const
{
    QProcess process;
    QStringList options;
    options << "-c";
    options << "cat /sys/class/dmi/id/sys_vendor";
    process.start("/bin/bash", options);
    process.waitForFinished();
    QString result = process.readAllStandardOutput();

    QStringList list = result.split("\n");
    result = list.at(0);
    qInfo() << __FILE__ << __LINE__ << "获取设备厂商为：" << result;
    return result;
}

const QString MachineModel::getProductName() const
{
    QProcess process;
    QStringList options;
    options << "-c";
    options << "cat /sys/class/dmi/id/product_name";
    process.start("/bin/bash", options);
    process.waitForFinished();
    QString result = process.readAllStandardOutput();

    QStringList list = result.split("\n");
    result = list.at(0);
    qInfo() << __FILE__ << __LINE__ << "获取产品名为：" << result;
    return result;
}

const QString MachineModel::getProductFamily() const
{
    QProcess process;
    QStringList options;
    options << "-c";
    options << "cat /sys/class/dmi/id/product_family";
    process.start("/bin/bash", options);
    process.waitForFinished();
    QString result = process.readAllStandardOutput();

    QStringList list = result.split("\n");
    result = list.at(0);
    qInfo() << __FILE__ << __LINE__ << "获取设备Family为：" << result;
    return result;
}
