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
#include "noticealarmpoolutil.h"

void NoticeAlarmPoolUtil::showAgain(uint id)
{
    qDebug()<<"dbq-showAgain"<<id;
    auto notice = noticesMap->value(id);
    if(notice!=nullptr){
        notice->show_again();
    }
}

void NoticeAlarmPoolUtil::inseartNoticesMap(uint id, Natice_alarm *notice)
{
    if(!noticesMap->contains(id)){
        noticesMap->insert(id,notice);
    }
}

void NoticeAlarmPoolUtil::deleteNoticesMap(uint id)
{
    noticesMap->remove(id);
}

NoticeAlarmPoolUtil::NoticeAlarmPoolUtil(QObject *parent) : QObject(parent)
{
    noticesMap = new QMap<uint,Natice_alarm *>();
    /*
    QMapIterator<int,KySmallPluginInterface *> iterator(*pluginsMap);
    int index = 0;
    while (iterator.hasNext()) {
        iterator.next();
        KySmallPluginInterface * temp = iterator.value();
        QString pluginName = temp->name();
        qWarning()<<"small-plugin-manage ini plugin widget start"<<pluginName;
        try{
            //创建窗体，加入窗体栈
            auto widget = temp->createWidget(this);
            qWarning()<<"small-plugin-manage ini plugin widget finish"<<pluginName;
            widget->setFixedSize(400,550);
            pulginsStackWidget->addWidget(widget);
            //初始化切换按钮
            iniSwitchBtn(temp);
            pluginNameWidgetIndexMap->insert(temp->name(),index);
            widgetIndexPluginNameMap->insert(index,temp->name());
            index++;
        }catch(...){
            qWarning()<<"small-plugin-manage  install plugin "<<pluginName<<" fail";
        }

    }
    */
}
