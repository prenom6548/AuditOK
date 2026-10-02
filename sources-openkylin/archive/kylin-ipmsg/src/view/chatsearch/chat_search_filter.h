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

#ifndef __CHAT_SEARCH_FILTER_H__
#define __CHAT_SEARCH_FILTER_H__

#include <QSortFilterProxyModel>

class ChatSearchFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ChatSearchFilter();
    ~ChatSearchFilter();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    bool judgeWhetherPicture(QString path) const;
    bool judgeWhetherLink(QString str) const;

private:
    int verify_file(char *const filename) const;  /* 代码规范 */
signals:

public slots:
};

#endif
