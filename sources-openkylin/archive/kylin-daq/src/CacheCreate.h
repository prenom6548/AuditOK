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
#ifndef CACHECREATE_H
#define CACHECREATE_H

#include <QObject>
#include "CacheBase.h"

class CacheCreate : public CacheBase
{
    Q_OBJECT
private:
    /**
     * @brief 上传数据表创建字符串
     *
     */
    static constexpr const char* s_userInfoTableCreateCmd = "\
                                            CREATE TABLE  IF NOT EXISTS Data1\
                                            ( \
                                            Id INTEGER PRIMARY KEY   AUTOINCREMENT,\
                                            Value1 TEXT NOT NULL, \
                                            Value2 TEXT NOT NULL \
                                            )";
    /**
     * @brief 公共数据表创建字段
     *
     */
    static constexpr const char* s_commonInfoTableCreateCmd = "\
                                            CREATE TABLE IF NOT EXISTS Data2\
                                            ( \
                                            Id INTEGER PRIMARY KEY   AUTOINCREMENT UNIQUE,\
                                            Value1 TEXT NOT NULL \
                                            )";
    /**
     * @brief 重传数据表创建字符串
     *
     */
    static constexpr const char* s_reuploadTableCreateCmd = " \
                                            CREATE TABLE IF NOT EXISTS Data3\
                                            ( \
                                            Id INTEGER PRIMARY KEY   AUTOINCREMENT,\
                                            Value1 TEXT NOT NULL, \
                                            Value2 TEXT NOT NULL \
                                            )";
            /**
              * @brief 事件信息存储表
              *
              */
              static constexpr const char* s_eventMessageTableCreateCmd = "\
                                                                      CREATE TABLE IF NOT EXISTS eventMessage\
                                                                      ( \
                                                                          Id INTEGER PRIMARY KEY   AUTOINCREMENT,\
                                                                          Value1 TEXT NOT NULL, \
                                                                          Value2 TEXT NOT NULL\
                                                                      )";

              /**
               * @brief 事件信息存储表
               *
               */
               static constexpr const char* s_eventReuploadMessageTableCreateCmd = "\
                                                                                CREATE TABLE IF NOT EXISTS eventReuploadMessage\
                                                                                ( \
                                                                                    Id INTEGER PRIMARY KEY   AUTOINCREMENT,\
                                                                                    Value1 TEXT NOT NULL, \
                                                                                    Value2 TEXT NOT NULL\
                                                                                )";
    /**
     * @brief tid数据表创建字符串
     *
     */
    static constexpr const char* s_tidInfoTableCreateCmd = "\
                                            CREATE TABLE IF NOT EXISTS Data4\
                                            ( \
                                                Value1 TEXT PRIMARY KEY NOT NULL UNIQUE\
                                            )";
    static constexpr const char* s_stateInfoTableCreateCmd = " \
                                            CREATE TABLE IF NOT EXISTS Data5\
                                            ( \
                                            Value1 TEXT PRIMARY KEY NOT NULL,\
                                            Value2 TEXT NOT NULL, \
                                            Value3 TEXT NOT NULL, \
                                            Value4 TEXT NOT NULL, \
                                            Value5 INTEGER NOT NULL, \
                                            Value6 TEXT NOT NULL \
                                            )";
public:
    /**
     * @brief Construct a new Cache Create object
     *
     * @param parent
     */
    CacheCreate(QObject *parent=nullptr);
public slots:
    /**
     * @brief 创建数据库表
     *
     */
    void createTable();
private:
    /**
     * @brief 当前数据库 schema 版本号（PRAGMA user_version）
     */
    static constexpr int s_dbVersion = 2;
    /**
     * @brief 标记本进程生命周期内 2.0 迁移是否已执行，避免 user_version 写入失败时定时器反复 DROP Data4/Data5
     */
    bool m_migration_2_0_Done = false;
    /**
     * @brief 重建/补建全部 7 张表
     */
    void createAllTables() const;
    /**
     * @brief 2.0 迁移：DROP Data4/Data5 以新 schema 重建
     */
    void migrate_2_0() const;
};

#endif // CACHECREATE_H
