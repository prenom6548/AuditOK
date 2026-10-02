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
#ifndef AUTO_SEARCH_RESULT_MODEL
#define AUTO_SEARCH_RESULT_MODEL

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "device_information.h"

class AutoSearchResultModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit AutoSearchResultModel(QObject *parent = nullptr);
    ~AutoSearchResultModel();
    void addPrinter(const DeviceInformation &info);
    DeviceInformation getInfo(const QModelIndex &index);
    void clearAllPrinter();
    bool printerExist(const DeviceInformation &info);
    bool printerNeedInsert(const DeviceInformation &info);
private:
    QMap<QString, DeviceInformation> m_devInfoList;
};

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SortFilterProxyModel(QObject *parent = nullptr);
protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif //AUTO_SEARCH_RESULT_MODEL
