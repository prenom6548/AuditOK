/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef CUSTOMTREEVIEW_H
#define CUSTOMTREEVIEW_H

#include <QGSettings>
#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStandardItemModel>
#include <QListView>
#include <QTreeView>

#include "UIControl/tableview/musiclistmodel.h"
#include "UIControl/base/musicDataBase.h"
#include "customTreeViewDelegate.h"



class CustomTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit CustomTreeView(bool isHistory = false, QWidget *parent = nullptr);
    ~CustomTreeView();

    MusicListModel *m_model = nullptr; // 设置表头

    void initStyle();
    void setIsHistoryView(bool isHistory);
    void initGsettings();

    void setSearchText(QString text);
    void setSearchListName(QString listName);

    QString pluginBtnHoverColor(QString styleName, bool hoverFlag);
    QString getSearchText()const;
    QString getSearchListName()const;
    QString getNowPlayListName()const;

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
//    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) override;
public Q_SLOTS:
    void setNowPlayListName(QString listName);

Q_SIGNALS:
    void hoverIndexChanged(QModelIndex index);
    void hoverColorChanged(QString hoverColor);
    void doubleClickedSignal(QModelIndex index);
    void leaveFromItem();
    void showVerticalScrollBar();
    void hideVerticalScrollBar();

private:
    bool m_isHistory = false;

    QGSettings *qtSettings = nullptr;
    QString hoverColor;
    QString clickColor;

    CustomTreeViewDelegate *m_delegate = nullptr; // 绘制歌曲列表样式
    QString m_searchText;
    QString m_searchListName;
    QString m_nowListName = "";
};

#endif // CUSTOMTREEVIEW_H
