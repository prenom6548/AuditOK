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

#ifndef SEARCHMSGMODEL_H
#define SEARCHMSGMODEL_H

#include <QWidget>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDebug>

#include "global/database/chatmsgdb.h"
#include "model/friendlistmodel.h"
#include "model/chatmsgmodel.h"

class SearchMsgModel : public QStandardItemModel
{
    Q_OBJECT
    friend class TestSearchMsgModel;

public:
    SearchMsgModel();
    ~SearchMsgModel();

    static SearchMsgModel *getInstance();

private:
    void getModel(void);
    void getSearchResult(QString);

    QMap<int, int> m_searchResult;
    QHash<int, QStandardItem *> m_msgIdItemsMap;

public slots:
    void slotSelect(QString);
};

#endif // SEARCHMSGMODEL_H
