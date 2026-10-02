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
#include "dbusnoticeshowagainadaptor.h"



DbusNoticeShowAgainAdaptor::DbusNoticeShowAgainAdaptor(QObject *parent, NoticeAlarmPoolUtil *currentNotice): DbusBaseClockAdaptor(parent),
    m_notice(currentNotice)
{

}

QString DbusNoticeShowAgainAdaptor::noticeShowAgain(QString param)
{
    neb::CJsonObject oJson(param.toStdString());
    QString id = QString::fromStdString(oJson["id"].ToString());
    
    // 检查ID字段是否存在
    if (!oJson.KeyExist("id")) {
        return formatReturnMsg(clockInterface::FAIL,"主键id为空");
    }
    
    // 检查ID是否为空字符串或仅包含引号
    if(FieldValidUtil::isNull(id) || id == "\"\"") {
        return formatReturnMsg(clockInterface::FAIL,"主键id为空");
    }
    
    id = FieldValidUtil::QStringFilter(id);
    qDebug()<<"dbq-id"<<id;
    m_notice->showAgain(id.toUInt());
    return formatReturnMsg(clockInterface::SUCCESS,"设置稍后提醒成功");
}
