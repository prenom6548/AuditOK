/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "auto_search_result_model.h"
#include "gsettings_monitor.h"

AutoSearchResultModel::AutoSearchResultModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setColumnCount(1);
}

AutoSearchResultModel::~AutoSearchResultModel()
{

}

DeviceInformation AutoSearchResultModel::getInfo(const QModelIndex &index)
{
    DeviceInformation info = index.data(Qt::UserRole).value<DeviceInformation>();
    return info;
}

void AutoSearchResultModel::clearAllPrinter()
{
    clear();
    setColumnCount(1);
    m_devInfoList.clear();
}

void AutoSearchResultModel::addPrinter(const DeviceInformation &info)
{
    static QMutex mutex;
    if (!printerNeedInsert(info)) {
        return;
    }

    QString name = getNameFromDeviceInformation(info);

    QString type;
    QString displayName;
    switch (info.deviceType)
    {
    case DeviceType::VIRTUAL :
    case DeviceType::SERIAL_AND_PRALALLEL :
    case DeviceType::USB :
        displayName = name;
        break;

    case DeviceType::IPP :
        if (info.host.isEmpty()) {
            return;
        }
        type = tr("network");
        displayName = QString("%1 (%2/%3)").arg(info.host).arg(name).arg(type);
        break;

    case DeviceType::SOCKET :
        if (info.host.isEmpty()) {
            return;
        }

        type = tr("network") + "-socket";
        displayName = QString("%1 (%2/%3)").arg(info.host).arg(name).arg(type);
        break;

    case DeviceType::LPD :
        type = tr("network") + "-lpd";
        displayName = QString("%1 (%2)").arg(name).arg(type);
        break;

    case DeviceType::DRIVERLESS :
        type = "driverless";
        displayName = QString("%1 (%2/%3)").arg(info.driverlessDomain).arg(name).arg(type);
        break;

    default:
        break;
    }
    if (displayName.isEmpty()) {
        return;
    }
    mutex.lock();
    insertRow(0);
    setData(index(0, 0), displayName);
    setData(index(0,0), QVariant::fromValue(info), Qt::UserRole);
    itemFromIndex(index(0, 0))->setToolTip(displayName);
    m_devInfoList.insert(info.uri, info);
    mutex.unlock();
}

bool AutoSearchResultModel::printerNeedInsert(const DeviceInformation &info)
{
    if (!GsettingsMonitor::getInstance()->showProtocolsGet().contains("pdf")) {
        if (info.uri == "cups-pdf:/") {
            return false;
        }
    }

    if (!GsettingsMonitor::getInstance()->showProtocolsGet().contains("brf")) {
        if (info.uri == "cups-brf:/") {
            return false;
        }
    }

    if (!GsettingsMonitor::getInstance()->showProtocolsGet().contains("serial")) {
        if (info.uri.contains("serial:/")) {
            return false;
        }
    }

    return !printerExist(info);
}

bool AutoSearchResultModel::printerExist(const DeviceInformation &info)
{
    switch (info.deviceType)
    {
    case DeviceType::SERIAL_AND_PRALALLEL:
    case DeviceType::USB :
        if (m_devInfoList.contains(info.uri)) {
            return true;
        } else {
            return false;
        }
    case DeviceType::SOCKET :
        for (auto i = m_devInfoList.begin(); i != m_devInfoList.end(); i++) {
            if (i.value().host.isEmpty()) {
                continue;
            }
            else if (i.value().host == info.host && i.value().deviceType == info.deviceType) {
                return true;
            }
        }
        return false;
    default:
        break;
    }
    return false;
}

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    this->setFilterKeyColumn(0);// 设置应用过滤的列
    this->setFilterRole(Qt::DisplayRole);
    this->setSortRole(Qt::DisplayRole);
}

bool SortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    DeviceInformation leftData = sourceModel()->data(source_left, Qt::UserRole).value<DeviceInformation>();
    DeviceInformation rightData = sourceModel()->data(source_right, Qt::UserRole).value<DeviceInformation>();

    return int(leftData.deviceType) < int(rightData.deviceType);

}
