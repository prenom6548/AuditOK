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

#ifndef CHATMSGDELEGATE_H
#define CHATMSGDELEGATE_H

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QAbstractItemDelegate>
#include <QStyleOption>
#include <QModelIndex>
#include <QEvent>
#include <QObject>
#include <QPainter>
#include <QVariant>
#include <QRectF>
#include <QPoint>
#include <QFont>
#include <QLabel>

#include "model/chatmsgmodel.h"

// #include <QDir>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QDebug>

class ChatMsgDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    friend class TestChatMsgDelegate;

public:
    explicit ChatMsgDelegate();
    ~ChatMsgDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // 根据消息内容获取消息框大小
    static QSize getMsgRectSize(QString msgStr, QFont font, int textWidth);

    // 根据消息内容获取消息框大小
    static void setItemHeight(int height);

    // 将byte转换成KB或MB
    static QString sizeFormat(qint64 size);

    // private:
    static int itemWidth;
    static int itemHeight;

    // 所选消息框坐标及尺寸
    static QPoint borderPoint;
    static QSize borderSize;
};


#endif // CHATMSGDELEGATE_H