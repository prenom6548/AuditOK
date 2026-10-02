/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "sessionlistwidget.h"
#include <QScrollBar>
#include "sessionitemwidget.h"

SessionListWidget::SessionListWidget(LockDialogModel *model, QWidget *parent)
    : MyListWidget(parent), m_modelLockDialog(model)
{
    initUI();
    initConnections();
}

void SessionListWidget::initUI()
{
    QString userListStyle
        = "QListWidget{ background-color: rgba(255,255,255,15%); border-radius: 8px; padding: 5px 5px 5px 5px;}"
          "QListWidget::item{background:rgba(255,255,255,0%);height:40px; border-radius:4px}"
          "QListWidget::item:hover{color:#333333; background-color:rgba(255,255,255,20%)}"
          "QListWidget::item::selected{border-radius: 4px;background-color:rgba(255,255,255,40%);}";
    setStyleSheet(userListStyle);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setProperty("drawScrollBarGroove", false);

    onUpdateListInfo();
}

void SessionListWidget::initConnections()
{
    //  响应点击事件
    connect(this, &SessionListWidget::itemClicked, this, &SessionListWidget::onListItemClicked);
}

void SessionListWidget::onUpdateListInfo()
{
    this->clear();
    for (auto session : m_modelLockDialog->sessionsInfo()) {
        QListWidgetItem *userItem = new QListWidgetItem();
        userItem->setSizeHint(QSize(400, 40));
        insertItem(count(), userItem);
        SessionItemWidget *itemWidget = new SessionItemWidget(this);
        itemWidget->setSessionName(session);
        setItemWidget(userItem, itemWidget);
    }
}

void SessionListWidget::onListItemClicked(QListWidgetItem *item)
{
    QWidget *widget = itemWidget(item);
    SessionItemWidget *currentItem = qobject_cast<SessionItemWidget *>(widget);
    if (currentItem) {
        this->hide();
        Q_EMIT sessionSelected(currentItem->sessionName());
    }
}

void SessionListWidget::updateWidgetSize()
{
    setFixedWidth(420);
    if (maxHeight() >= 0 && (count() * 40 + 10) > maxHeight()) {
        setFixedHeight(maxHeight());
    } else {
        setFixedHeight(count() * 40 + 10);
    }
    adjustSize();
}
