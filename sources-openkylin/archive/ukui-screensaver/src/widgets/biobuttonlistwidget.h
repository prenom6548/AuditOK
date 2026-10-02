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
#ifndef BIOBUTTONLISTWIDGET_H
#define BIOBUTTONLISTWIDGET_H

#include <QWidget>
#include "mylistwidget.h"

class QListWidgetItem;

extern float scale;
class BioButtonListWidget : public MyListWidget
{
    Q_OBJECT
public:
    explicit BioButtonListWidget(QWidget *parent = nullptr);

    void updateWidgetSize();

    void addOptionButton(unsigned uLoginOptType, int nDrvId, QString strDrvName);

    void onOptionSelected(int nIndex);

    void updateUI();

public Q_SLOTS:
    void onListItemClicked(QListWidgetItem *item);
    void onListItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

Q_SIGNALS:
    void userSelected(QString strUserName);

private:
    void initUI();
    void initConnections();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QList<int> m_listDriveId;
};

#endif // BIOBUTTONLISTWIDGET_H
