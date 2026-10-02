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

#include "locationlistwidget.h"
#include <QLabel>
#include <QPushButton>
#include "locationget.h"
namespace kmre_gps
{
LocationListWidget::LocationListWidget(QWidget *parent) : QWidget(parent)
{
    this->hide();
    initWidget();
}

void LocationListWidget::initWidget()
{
    setFixedSize(526, 280);
    m_fileListWidget = new QListWidget(this);
    m_fileListWidget->hide();
    m_fileListWidget->resize(526, 280);
    item0 = new QListWidgetItem(m_fileListWidget);
    item1 = new QListWidgetItem(m_fileListWidget);
    item2 = new QListWidgetItem(m_fileListWidget);
    item3 = new QListWidgetItem(m_fileListWidget);
    item4 = new QListWidgetItem(m_fileListWidget);

    item0->setSizeHint(QSize(424, 55));
    item1->setSizeHint(QSize(424, 55));
    item2->setSizeHint(QSize(424, 55));
    item3->setSizeHint(QSize(424, 55));
    item4->setSizeHint(QSize(424, 55));

    LocationListItem0 = new LocationListItem();
    LocationListItem1 = new LocationListItem();
    LocationListItem2 = new LocationListItem();
    LocationListItem3 = new LocationListItem();
    LocationListItem4 = new LocationListItem();

    m_fileListWidget->setItemWidget(item0, LocationListItem0);
    m_fileListWidget->setItemWidget(item1, LocationListItem1);
    m_fileListWidget->setItemWidget(item2, LocationListItem2);
    m_fileListWidget->setItemWidget(item3, LocationListItem3);
    m_fileListWidget->setItemWidget(item4, LocationListItem4);

    connect(LocationListItem0, &LocationListItem::clicked, this, [=]() {num = 0;slotChooseShow();});
    connect(LocationListItem1, &LocationListItem::clicked, this, [=]() {num = 1;slotChooseShow();});
    connect(LocationListItem2, &LocationListItem::clicked, this, [=]() {num = 2;slotChooseShow();});
    connect(LocationListItem3, &LocationListItem::clicked, this, [=]() {num = 3;slotChooseShow();});
    connect(LocationListItem4, &LocationListItem::clicked, this, [=]() {num = 4;slotChooseShow();});
}

void LocationListWidget::listShow()
{

    QString poistion[5] = {""};
    for (int i = 0; i < 5; i++) {
        poistion[i] = LocationGet::getInstance()->m_locationArray[i].name;
        poistion[i].append("\n");
        if (LocationGet::getInstance()->m_locationArray[i].cityName
            != LocationGet::getInstance()->m_locationArray[i].province) {
            poistion[i].append(LocationGet::getInstance()->m_locationArray[i].province);
            poistion[i].append(LocationGet::getInstance()->m_locationArray[i].cityName);
        } else {
            poistion[i].append(LocationGet::getInstance()->m_locationArray[i].cityName);
        }
        poistion[i].append(LocationGet::getInstance()->m_locationArray[i].adName);
        poistion[i].append(LocationGet::getInstance()->m_locationArray[i].address);
    }

    for (int i = 0; i < 5; i++) {
        switch (i) {
        case 0:
            LocationListItem0->setItemName(poistion[0]);
            break;
        case 1:
            LocationListItem1->setItemName(poistion[1]);
            break;
        case 2:
            LocationListItem2->setItemName(poistion[2]);
            break;
        case 3:
            LocationListItem3->setItemName(poistion[3]);
            break;
        case 4:
            LocationListItem4->setItemName(poistion[4]);
            break;
        default:
            break;
        }
    }
    //如果搜索返回的内容为空的话只有"\n"
    if (poistion[0] != "\n") {
        this->show();
        m_fileListWidget->show();
    } else {
        this->hide();
        m_fileListWidget->hide();
    }
}

void LocationListWidget::slotChooseShow()
{
    Q_EMIT sigCurrentShow();
}

LocationListWidget::~LocationListWidget() {}

} // namespace kmre_gps
