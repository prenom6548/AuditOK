/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef CACHECHECK_H
#define CACHECHECK_H

#include <QObject>
#include "CacheBase.h"

class CacheCheck : public CacheBase
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new Cache Check object
     * 
     * @param parent 
     */
    CacheCheck(QObject *parent = nullptr);
public slots:
    /**
     * @brief 在数据库中删除不符合条件的数据
     * 
     * @param storageDuration 最大存储间隔
     * @param cacheRows 最大存储行数
     * @param uploadTimes 最大上传次数
     */
    void checkDataBase(qlonglong storageDuration, qlonglong cacheRows,qlonglong uploadTimes);
private:
    /**
     * @brief 在数据库中删除超过存储间隔的数据
     * 
     * @param tableName 表名
     * @param storageDuration 最大存储间隔
     */
    void checkTime(QString tableName,qlonglong storageDuration);
    /**
     * @brief 在数据库中删除超过上传次数的数据
     * 
     * @param tableName 表名
     * @param uploadTimes 最大上传次数
     */
    void checkTimes(QString tableName,qlonglong uploadTimes);
    /**
     * @brief 在数据库中删除超过行数的数据
     * 
     * @param tableName 表名
     * @param key 检测的键
     * @param cacheRows 最大存储行数
     */
    void checkRows(QString tableName,QString key,qlonglong cacheRows);
};

#endif // CACHECHECK_H
