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
#include "manual_search_result_model.h"
#include <QMutex>

ManualSearchResultModel::ManualSearchResultModel(QStandardItemModel *parent)
    : QStandardItemModel(parent)
{
    setColumnCount(1);
}

ManualSearchResultModel::~ManualSearchResultModel()
{

}

DeviceInformation ManualSearchResultModel::getInfo(const QModelIndex &index)
{
    DeviceInformation info = index.data(Qt::UserRole).value<DeviceInformation>();
    return info;
}

void ManualSearchResultModel::clearAllPrinter()
{
    clear();
    setColumnCount(1);
    m_devInfoList.clear();
}

void ManualSearchResultModel::addPrinter(const DeviceInformation &info)
{
    static QMutex mutex;
    qDebug() << info.makeAndModel;

    QString name = getNameFromDeviceInformation(info);

    QString type;
    QString displayName;
    switch (info.protocolType)
    {
    case ProtocolType::SMB:
    case ProtocolType::LPD:
        name = info.name;
        break;
    default:
        break;
    }
    displayName = name;
    if (displayName.isEmpty()) {
        return;
    }
    mutex.lock();
    int lastRow = rowCount() - 1;
    insertRow(lastRow + 1);
    setData(index(lastRow + 1, 0), displayName);
    setData(index(lastRow + 1, 0), QVariant::fromValue(info), Qt::UserRole);
    itemFromIndex(index(lastRow + 1, 0))->setToolTip(displayName);
    m_devInfoList.insert(info.uri, info);
    mutex.unlock();
}
