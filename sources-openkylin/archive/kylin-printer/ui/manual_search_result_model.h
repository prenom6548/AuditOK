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
#ifndef MANUAL_SEARCH_RESULT_MODEL
#define MANUAL_SEARCH_RESULT_MODEL

#include <QStandardItemModel>
#include "device_information.h"


class ManualSearchResultModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ManualSearchResultModel(QStandardItemModel *parent = nullptr);
    ~ManualSearchResultModel();
    void addPrinter(const DeviceInformation &info);
    DeviceInformation getInfo(const QModelIndex &index);
    void clearAllPrinter();
    bool printerExist(const DeviceInformation &info);
    bool printerNeedInsert(const DeviceInformation &info);
private:
    QMap<QString, DeviceInformation> m_devInfoList;
};

#endif //MANUAL_SEARCH_RESULT_MODEL
