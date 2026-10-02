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
#include "userlistwidget.h"
#include <QScrollBar>
#include "useritemwidget.h"

UserListWidget::UserListWidget(LockDialogModel *model, QWidget *parent) : MyListWidget(parent), m_modelLockDialog(model)
{
    initUI();
    initConnections();
    setUserListWidget(true);
}

void UserListWidget::initUI()
{
    QString userListStyle
        = "QListWidget{ background-color: rgba(255,255,255,15%); border-radius: 8px; padding: 5px 5px 5px 5px;}"
          "QListWidget::item{background:rgba(255,255,255,0%);height:40px; border-radius:4px}"
          "QListWidget::item:hover{color:#333333; background-color:rgba(255,255,255,20%)}"
          "QListWidget::item::selected{border-radius: 4px;background-color:rgba(255,255,255,40%);}";
    setStyleSheet(userListStyle);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setProperty("drawScrollBarGroove", false);

    m_curFontSize = m_modelLockDialog->getCurFontSize();
    m_ptToPx = m_modelLockDialog->getPtToPx();
    onUpdateListInfo();
}

void UserListWidget::initConnections()
{
    //  响应点击事件
    connect(this, &UserListWidget::itemClicked, this, &UserListWidget::onListItemClicked);

    connect(m_modelLockDialog, &LockDialogModel::usersInfoChanged, this, &UserListWidget::onUpdateListInfo);
}

void UserListWidget::onUpdateListInfo()
{
    this->clear();
    for (auto user : m_modelLockDialog->usersInfo()) {
        if (user->isSystemAccount()) {
            continue;
        }
        QListWidgetItem *userItem = new QListWidgetItem();
        userItem->setSizeHint(QSize(420, 40));
        insertItem(count(), userItem);
        UserItemWidget *itemWidget = new UserItemWidget(this);
        QPixmap p(user->headImage());
        itemWidget->setUserPixmap(p);
        if (user->name() == "*login") {
            itemWidget->setUserNickName(tr("Login"));
        } else if (user->name() == "*guest") {
            itemWidget->setUserNickName(tr("Guest"));
        } else {
            itemWidget->setUserNickName(user->fullName());
        }
        itemWidget->setUserStatus(user->isLoggedIn());
        if (user->fullName() == m_modelLockDialog->currentUserName()) {
            setCurrentItem(userItem, QItemSelectionModel::SelectionFlag::SelectCurrent);
        }
        itemWidget->setUserName(user->name());
        itemWidget->setFontSize((14 + m_curFontSize) * m_ptToPx);
        setItemWidget(userItem, itemWidget);
    }
    updateWidgetSize();
}

void UserListWidget::onListItemClicked(QListWidgetItem *item)
{
    QWidget *widget = itemWidget(item);
    UserItemWidget *currentItem = qobject_cast<UserItemWidget *>(widget);
    if (currentItem) {
        Q_EMIT userSelected(currentItem->getUserName());
    }
}

void UserListWidget::updateWidgetSize()
{
    setFixedWidth(420);
    if (maxHeight() >= 0 && (count() * 40 + 10) > maxHeight()) {
        setFixedHeight(maxHeight());
    } else {
        setFixedHeight(count() * 40 + 10);
    }
    adjustSize();
    Q_EMIT widgetSizeChanged();
}

void UserListWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTranslate();
    }
}

void UserListWidget::refreshTranslate()
{
    for (int n = 0; n < count(); n++) {
        QListWidgetItem *widgetItem = item(n);
        if (widgetItem) {
            QWidget *widget = itemWidget(widgetItem);
            UserItemWidget *item = qobject_cast<UserItemWidget *>(widget);
            if (item) {
                if (item->getUserName() == "*login") {
                    item->setUserNickName(tr("Login"));
                } else if (item->getUserName() == "*guest") {
                    item->setUserNickName(tr("Guest"));
                }
            }
        }
    }
}
