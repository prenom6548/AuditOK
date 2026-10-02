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
#include <QTextStream>
#include <QFile>
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
    QString type = getSysVendor() + getProductFamily();
    qDebug() << "MachineType:" << type;

    QSettings setting(":/assets/data/conf.ini", QSettings::IniFormat);
    setting.beginGroup("MachineType"); //节点开始
    if (setting.contains(type)) {
        m_machineType = setting.value(type).toString();
    } else {
        m_machineType = QString();
    }
    setting.endGroup(); //节点结束
    qDebug() << "SettingsMachineType:" << m_machineType;
}

static QString readFileSafely(const QString& filePath)
{
    QString strContent = "";
    if (!QFile::exists(filePath)) {
        qWarning() << "file not exists:" << filePath;
        return strContent;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "file open failed " << filePath << file.error();
        return strContent;
    }

    QTextStream testStream(&file);
    return testStream.readAll();
}

const QString MachineModel::getSysVendor() const
{
    QString result = readFileSafely("/sys/class/dmi/id/sys_vendor");

    QStringList list = result.split("\n");
    result = list.at(0);
    qDebug() << __FILE__ << __LINE__ << "获取设备厂商为：" << result;
    return result;
}

const QString MachineModel::getProductName() const
{
    QString result = readFileSafely("/sys/class/dmi/id/product_name");

    QStringList list = result.split("\n");
    result = list.at(0);
    qDebug() << __FILE__ << __LINE__ << "获取产品名为：" << result;
    return result;
}

const QString MachineModel::getProductFamily() const
{
    QString result = readFileSafely("/sys/class/dmi/id/product_family");

    QStringList list = result.split("\n");
    result = list.at(0);
    qDebug() << __FILE__ << __LINE__ << "获取设备Family为：" << result;
    return result;
}
