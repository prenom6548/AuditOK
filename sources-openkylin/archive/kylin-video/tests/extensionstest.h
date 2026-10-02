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
#ifndef EXTENSIONSTEST_H
#define EXTENSIONSTEST_H

#include <QObject>
#include <QtTest/QtTest>
#include "global/extensions.h"

class ExtensionsTest : public QObject
{
    Q_OBJECT

public:
    explicit ExtensionsTest(QObject *parent = nullptr);

private slots:
    // 测试 forFilter 函数
    void testForFilter();
    
    // 测试 forDirFilter 函数
    void testForDirFilter();
    
    // 测试 extensionFromUrl 函数
    void testExtensionFromUrl();
    
    // 测试 forRegExp 函数
    void testForRegExp();
    
    // 测试 Extensions 类的构造函数
    void testExtensionsConstructor();
    
    // 测试 Extensions 类的各种 getter 方法
    void testExtensionsGetters();

private:
    ExtensionList* m_extensionList { nullptr };
    Extensions* m_extensions { nullptr };
};

#endif // EXTENSIONSTEST_H