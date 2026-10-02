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
#ifndef UTILTEST_H
#define UTILTEST_H

#include <QObject>
#include <QtTest/QtTest>

class UtilTest : public QObject
{
    Q_OBJECT

public:
    explicit UtilTest(QObject *parent = nullptr);

private slots:
    // 测试 FormatNumberWithAmpersand 函数
    void testFormatNumberWithAmpersand();
    
    // 测试 HumanSize 函数
    void testHumanSize();
    
    // 测试 IsValidLocation 函数
    void testIsValidLocation();
    
    // 测试 ShowInFolder 函数
    void testShowInFolder();
    
    // 测试 GCD 函数
    void testGCD();
    
    // 测试 Ratio 函数
    void testRatio();
    
    // 测试 ToNativeSeparators 函数
    void testToNativeSeparators();
    
    // 测试 FromNativeSeparators 函数
    void testFromNativeSeparators();
    
    // 测试 FormatTime 函数
    void testFormatTime();
    
    // 测试 FormatRelativeTime 函数
    void testFormatRelativeTime();
    
    // 测试 FormatNumber 函数
    void testFormatNumber();
    
    // 测试 IsValidUrl 函数
    void testIsValidUrl();
};

#endif // UTILTEST_H