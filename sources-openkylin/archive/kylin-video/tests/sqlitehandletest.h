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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SQLITEHANDLETEST_H
#define SQLITEHANDLETEST_H

#include <QObject>
#include <QtTest/QtTest>

class SqliteHandle;

class SqliteHandleTest : public QObject
{
    Q_OBJECT

public:
    explicit SqliteHandleTest(QObject *parent = nullptr);

private slots:
    // 测试 getMarkCharacter 函数
    void testGetMarkCharacter();
    
    // 测试 insertBookMark 函数
    void testInsertBookMark();
    
    // 测试 updateLastTime 函数
    void testUpdateLastTime();
    
    // 测试析构函数（通过对象生命周期管理）
    void testDestructor();
    
    // 测试 insertPlayList 错误处理
    void testInsertPlayListError();
    
    // 测试 updateLastTime 错误处理
    void testUpdateLastTimeError();
    
    // 测试 updateDuration 错误处理
    void testUpdateDurationError();
    
    // 测试 deletePlayList 错误处理
    void testDeletePlayListError();
    
    // 测试 clearPlayList 错误处理
    void testClearPlayListError();
    
    // 测试 insertBookMark 错误处理
    void testInsertBookMarkError();
    
    // 测试 deleteBookMark 错误处理
    void testDeleteBookMarkError();
    
    // 测试 getMarkCharacter 错误处理
    void testGetMarkCharacterError();
    
    // 测试 getLastTime 错误处理
    void testGetLastTimeError();
    
    // 测试 getDuration 错误处理
    void testGetDurationError();

private:
    SqliteHandle* m_sqliteHandle { nullptr };
};

#endif // SQLITEHANDLETEST_H