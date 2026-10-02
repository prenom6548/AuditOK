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

#ifndef MOCK_WRAPPER_H
#define MOCK_WRAPPER_H

#include <QObject>
#include <QAbstractListModel>
#include <QStyledItemDelegate>

// Mock wrapper for SearchMsgDelegate
class SearchMsgDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SearchMsgDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyledItemDelegate::paint(painter, option, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QStyledItemDelegate::sizeHint(option, index);
    }
};

// Mock wrapper for SearchMsgModel
class SearchMsgModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SearchMsgModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    static SearchMsgModel* getInstance() {
        static SearchMsgModel* instance = nullptr;
        if (instance == nullptr) {
            instance = new SearchMsgModel();
        }
        return instance;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return 0;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        return QVariant();
    }
};

// Mock wrapper for FriendListModel
class FriendListModel : public QObject
{
    Q_OBJECT

public:
    FriendListModel(QObject *parent = nullptr) : QObject(parent) {}

    static FriendListModel* getInstance() {
        static FriendListModel* instance = nullptr;
        if (instance == nullptr) {
            instance = new FriendListModel();
        }
        return instance;
    }

    void clearUnreadMsgNum(int friendId) {
        // Mock implementation
    }
};

// Mock wrapper for GlobalSizeData
class GlobalSizeData : public QObject
{
    Q_OBJECT

public:
    GlobalSizeData(QObject *parent = nullptr) : QObject(parent) {}

    static void SDKPointMainSearch() {
        // Mock implementation
    }
};

#endif // MOCK_WRAPPER_H