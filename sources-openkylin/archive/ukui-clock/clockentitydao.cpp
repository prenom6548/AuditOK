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
#include "clockentitydao.h"
#include "fieldvalidutil.h"

ClockEntityDao::ClockEntityDao(QObject *parent) : QObject(parent)
{

}

QSqlQuery ClockEntityDao::getClockByPK(QString id)
{
    auto sqlQuery = clock_sql::getQSqlQuery();
    sqlQuery.prepare("select * from clock where id=:id");
    sqlQuery.bindValue(":id",id);
    sqlQuery.exec();
    sqlQuery.next();
    return sqlQuery;
}

bool ClockEntityDao::checkClockExist(QString id)
{
    auto sqlQuery = getClockByPK(id);
    QString queryId = sqlQuery.value(14).toString();
    return FieldValidUtil::isNotNull(queryId);
}
