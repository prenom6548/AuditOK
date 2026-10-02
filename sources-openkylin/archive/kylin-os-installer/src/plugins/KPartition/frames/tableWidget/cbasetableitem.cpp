/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "cbasetableitem.h"

CBaseTableItem::CBaseTableItem(ItemContentType itemContentType,QString text,QString iconAddr,QWidget *widget1,QWidget *widget2)
{
    this->text=text;//由参数对item对象传递数据
    this->iconAddr=iconAddr;
    this->widget1=widget1;
    this->widget2=widget2;
    this->itemContentType=itemContentType;

}

void CBaseTableItem::setItemType(CBaseTableItem::ItemContentType itemContentType){this->itemContentType = itemContentType;}

void CBaseTableItem::setItemText(QString text){this->text = text;}

void CBaseTableItem::setItemIcon(QString iconAddr){this->iconAddr = iconAddr;}

void CBaseTableItem::setItemWidget1(QWidget *widget1){this->widget1 = widget1;}

void CBaseTableItem::setItemWidget2(QWidget *widget2){this->widget2 = widget2;}



