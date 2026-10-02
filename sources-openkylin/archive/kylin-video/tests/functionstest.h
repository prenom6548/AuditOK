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
#ifndef FUNCTIONSTEST_H
#define FUNCTIONSTEST_H

#include <QObject>
#include <QtTest/QtTest>

class FunctionsTest : public QObject
{
    Q_OBJECT

public:
    explicit FunctionsTest(QObject *parent = nullptr);

private slots:
    // 测试 Url 函数
    void testUrl();
    
    // 测试 executeCMD 函数
    void testExecuteCMD();
    
    // 测试 fileExt 函数
    void testFileExt();
    
    // 测试 timeToStr 函数
    void testTimeToStr();
    
    // 测试 getUrlType 函数
    void testGetUrlType();
    
    // 测试 filePath 函数
    void testFilePath();
    
    // 测试 humanSize 函数
    void testHumanSize();
    
    // 测试 isVirtualSuppotGpu 函数
    void testIsVirtualSuppotGpu();
    
    // 测试 isKirinCpu 函数
    void testIsKirinCpu();
    
    // 测试 isGreatwallDevice 函数
    void testIsGreatwallDevice();
    
    // 测试 getVersion 函数
    void testGetVersion();
};

#endif // FUNCTIONSTEST_H