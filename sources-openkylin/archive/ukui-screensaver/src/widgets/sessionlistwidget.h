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
#ifndef SESSIONLISTWIDGET_H
#define SESSIONLISTWIDGET_H

#include <QWidget>
#include "lock-dialog/lockdialogmodel.h"
#include "mylistwidget.h"

class QListWidgetItem;
class SessionListWidget : public MyListWidget
{
    Q_OBJECT
public:
    explicit SessionListWidget(LockDialogModel *model, QWidget *parent = nullptr);

    void updateWidgetSize();

public Q_SLOTS:
    void onUpdateListInfo();
    void onListItemClicked(QListWidgetItem *item);

Q_SIGNALS:
    void sessionSelected(QString strSession);

private:
    void initUI();
    void initConnections();

private:
    LockDialogModel *m_modelLockDialog = nullptr;
};

#endif // SESSIONLISTWIDGET_H
