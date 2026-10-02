/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KNAVIGATIONBAR_H
#define KNAVIGATIONBAR_H

#include "gui_g.h"
#include <QListView>
#include <QModelIndex>
#include <QMouseEvent>
#include <QScrollArea>
#include <QStandardItem>
#include <QStandardItemModel>

namespace kdk
{

/** @defgroup BarModule
 * @{
 */

class KNavigationBarPrivate;
class ListView;

/**
 * @brief 导航栏控件
 * @since 1.0.0
 */
class GUI_EXPORT KNavigationBar : public QScrollArea
{
    Q_OBJECT

public:
    enum NavigationBarDataType {
        ValueRole = Qt::UserRole + 1,
        StringRole = Qt::UserRole + 2,
        boolRole = Qt::UserRole + 3
    };

    KNavigationBar(QWidget *parent);

    /**
     * @brief 增加常规Item
     * @param item
     */
    void addItem(QStandardItem *item);

    /**
     * @brief 增加次级Item
     * @param subItem
     */
    void addSubItem(QStandardItem *subItem);

    /**
     * @brief 成组增加Item,在导航栏中会显示tag
     * @param items
     * @param tag
     */
    void addGroupItems(QList<QStandardItem *> items, const QString &tag);

    /**
     * @brief 在指定tag组中指定位置row插入item
     * @param tag
     * @param row
     * @param item
     * @since 3.0.1.0
     */
    void insertGroupItems(QString tag, int row, QStandardItem *item);

    /**
     * @brief 在指定组中删除item
     * @param tag
     * @param item
     * @since 3.0.1.0
     */
    void removeGroupItem(QString tag, QStandardItem *item);
    /**
     * @brief 添加tag
     * @param tag
     */
    void addTag(const QString &tag);

    /**
     * @brief 获取model
     * @return
     */
    QStandardItemModel *model();

    /**
     * @brief 获取view
     * @return
     */
    QListView *listview();

    /**
     * @brief 设置tabvalue
     * @param item
     * @param value
     * @since 3.0.1.0
     */
    void setTabValue(QStandardItem *item, QString value); // item

    /**
     * @brief 设置tabvalue是否可见
     * @param visible
     * @since 3.0.1.0
     */
    void setTabValueVisible(bool visible);

    /**
     * @brief tag 添加add图标
     * @param tag
     * @param icon
     * @since 3.0.1.0
     */
    void setAddIcon(QString tag, QIcon icon);

    /**
     * @brief 设置tag add图标是否可见
     * @param tag
     * @param visible
     * @since 3.0.1.0
     */
    void setAddTagVisible(QString tag, bool visible);

    /**
     * @brief 设置折叠图标是否可见
     * @param tag
     * @param visible
     * @since 3.0.1.0
     */
    void setExtendTagVisible(QString tag, bool visible);

Q_SIGNALS:
    void addTagClicked(QString str);

private:
    Q_DECLARE_PRIVATE(KNavigationBar)
    KNavigationBarPrivate *const d_ptr;
};

}
/**
 * @example testnavigationbar/widget.h
 * @example testnavigationbar/widget.cpp
 * @}
 */
#endif // KNAVIGATIONBAR_H
