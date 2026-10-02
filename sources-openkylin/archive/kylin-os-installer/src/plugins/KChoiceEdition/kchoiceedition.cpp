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

#include "kchoiceedition.h"

KChoiceEdition::KChoiceEdition()
{
}
QWidget* KChoiceEdition::CreateObject(QWidget *parent)//新建本插件界面
{
    m_PluginInterface = new choiseEditionUI(parent);//新建本插件界面类对象
    return m_PluginInterface;
}
QWidget* KChoiceEdition::getWidget()//获取本插件界面
{
      return m_PluginInterface;//返回本插件界面类对象地址
}
QString KChoiceEdition::getWidgetName()//预留接口，暂未用到
{
    QString s = QString("");
    return (QString)s;
}
