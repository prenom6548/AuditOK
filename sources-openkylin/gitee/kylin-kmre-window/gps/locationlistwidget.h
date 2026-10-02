/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOCATIONLISTWIDGET_H
#define LOCATIONLISTWIDGET_H
#include <QListWidget>
#include "locationlistitem.h"

namespace kmre_gps
{
class LocationListWidget : public QWidget
{
    Q_OBJECT
public:
    LocationListWidget(QWidget *parent = nullptr);
    ~LocationListWidget();
    QString m_data;
    QListWidget *m_fileListWidget = nullptr;
    void listShow();
    int num = 1;
private:
    void initWidget();
    void tipShow();
    LocationListItem *LocationListItem0;
    LocationListItem *LocationListItem1;
    LocationListItem *LocationListItem2;
    LocationListItem *LocationListItem3;
    LocationListItem *LocationListItem4;
    QListWidgetItem *item0;
    QListWidgetItem *item1;
    QListWidgetItem *item2;
    QListWidgetItem *item3;
    QListWidgetItem *item4;

private Q_SLOTS:
    void slotChooseShow();

Q_SIGNALS:
    void sigCurrentShow();
};

} // namespace kmre_gps
#endif // LOCATIONLISTWIDGET_H
