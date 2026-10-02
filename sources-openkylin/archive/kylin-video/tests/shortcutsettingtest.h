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
#ifndef SHORTCUTSETTINGTEST_H
#define SHORTCUTSETTINGTEST_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSettings>
#include "global/shortcutsetting.h"

class ShortCutSettingTest : public QObject
{
    Q_OBJECT

public:
    explicit ShortCutSettingTest(QObject *parent = nullptr);

private slots:
    // 测试 restoreKeyString 函数
    void testRestoreKeyString();
    
    // 测试 resetShort 函数
    void testResetShort();
    
    // 测试 initShortCutFunc 中的 lambda 函数
    void testInitShortCutFuncLambdas();
    
    // 测试 ShortCutSetting 析构函数
    void testDestructor();

    void testSetupShortCut();

private:
    QSettings* m_settings;
    ShortCutSetting* m_shortcutSetting;
};

#endif // SHORTCUTSETTINGTEST_H
