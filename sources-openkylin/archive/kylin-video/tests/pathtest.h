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
#ifndef PATHTEST_H
#define PATHTEST_H

#include <QObject>
#include <QtTest/QtTest>
#include "global/path.h"

class PathsTest : public QObject
{
    Q_OBJECT

public:
    explicit PathsTest(QObject *parent = nullptr);

private slots:
    // 测试 setAppPath 和 appPath 函数
    void testSetAppPath();
    void testAppPath();
    
    // 测试 setLogPath 和 logPath 函数
    void testSetLogPath();
    void testLogPath();
    
    // 测试其他路径相关函数
    void testPreviewPath();
    void testTranslationPath();
    void testShortcutsPath();
    void testQtTranslationPath();
    void testSetConfigPath();
    void testConfigPath();
    void testIniPath();
    void testSubtitleStyleFile();

private:
    // 保存原始路径值以便恢复
    QString m_originalAppPath;
    QString m_originalLogPath;
    QString m_originalConfigPath;
};

#endif // PATHTEST_H