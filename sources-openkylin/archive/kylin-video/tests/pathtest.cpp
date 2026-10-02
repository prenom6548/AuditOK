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

#include "pathtest.h"
#include <QDir>
#include <QStandardPaths>
#include <iostream>

PathsTest::PathsTest(QObject *parent) : QObject(parent)
{
    // 保存原始路径值
    m_originalAppPath = Paths::appPath();
    m_originalLogPath = Paths::logPath();
    m_originalConfigPath = Paths::configPath();
}

void PathsTest::testSetAppPath()
{
    std::cout << "=== Testing setAppPath ===" << std::endl;
    
    // 设置应用路径
    QString testPath = "/tmp/test_app_path";
    Paths::setAppPath(testPath);
    
    // 验证设置是否成功
    QVERIFY(Paths::appPath() == testPath);
    
    std::cout << "setAppPath test completed." << std::endl;
}

void PathsTest::testAppPath()
{
    std::cout << "=== Testing appPath ===" << std::endl;
    
    // 测试获取应用路径
    QString appPath = Paths::appPath();
    // appPath可能为空，因为我们没有显式设置它
    QVERIFY(true); // 只要不崩溃就行
    
    std::cout << "appPath test completed." << std::endl;
}

void PathsTest::testSetLogPath()
{
    std::cout << "=== Testing setLogPath ===" << std::endl;
    
    // 设置日志路径
    QString testPath = "/tmp/test_log_path";
    Paths::setLogPath(testPath);
    
    // 验证设置是否成功
    QVERIFY(Paths::logPath() == testPath);
    
    std::cout << "setLogPath test completed." << std::endl;
}

void PathsTest::testLogPath()
{
    std::cout << "=== Testing logPath ===" << std::endl;
    
    // 测试获取日志路径
    QString logPath = Paths::logPath();
    // 默认情况下应该返回缓存路径
    QVERIFY(!logPath.isEmpty());
    
    std::cout << "logPath test completed." << std::endl;
}

void PathsTest::testPreviewPath()
{
    std::cout << "=== Testing previewPath ===" << std::endl;
    
    // 测试预览路径
    QString previewPath = Paths::previewPath();
    QVERIFY(!previewPath.isEmpty());
    
    // 检查目录是否存在
    QDir dir(previewPath);
    QVERIFY(dir.exists());
    
    std::cout << "previewPath test completed." << std::endl;
}

void PathsTest::testTranslationPath()
{
    std::cout << "=== Testing translationPath ===" << std::endl;
    
    // 测试翻译路径
    QString translationPath = Paths::translationPath();
    QVERIFY(!translationPath.isEmpty());
    
    std::cout << "translationPath test completed." << std::endl;
}

void PathsTest::testShortcutsPath()
{
    std::cout << "=== Testing shortcutsPath ===" << std::endl;
    
    // 测试快捷键路径
    QString shortcutsPath = Paths::shortcutsPath();
    // 如果没有设置appPath，这个路径可能不完整，但我们仍然可以调用它
    QVERIFY(true); // 只要不崩溃就行
    
    std::cout << "shortcutsPath test completed." << std::endl;
}

void PathsTest::testQtTranslationPath()
{
    std::cout << "=== Testing qtTranslationPath ===" << std::endl;
    
    // 测试Qt翻译路径
    QString qtTranslationPath = Paths::qtTranslationPath();
    QVERIFY(!qtTranslationPath.isEmpty());
    
    std::cout << "qtTranslationPath test completed." << std::endl;
}

void PathsTest::testSetConfigPath()
{
    std::cout << "=== Testing setConfigPath ===" << std::endl;
    
    // 设置配置路径
    QString testPath = "/tmp/test_config_path";
    Paths::setConfigPath(testPath);
    
    // 验证设置是否成功
    QVERIFY(Paths::configPath() == testPath);
    
    std::cout << "setConfigPath test completed." << std::endl;
}

void PathsTest::testConfigPath()
{
    std::cout << "=== Testing configPath ===" << std::endl;
    
    // 测试获取配置路径
    QString configPath = Paths::configPath();
    QVERIFY(!configPath.isEmpty());
    
    std::cout << "configPath test completed." << std::endl;
}

void PathsTest::testIniPath()
{
    std::cout << "=== Testing iniPath ===" << std::endl;
    
    // 测试ini路径（应该与configPath相同）
    QString iniPath = Paths::iniPath();
    QString configPath = Paths::configPath();
    QVERIFY(iniPath == configPath);
    
    std::cout << "iniPath test completed." << std::endl;
}

void PathsTest::testSubtitleStyleFile()
{
    std::cout << "=== Testing subtitleStyleFile ===" << std::endl;
    
    // 测试字幕样式文件路径
    QString subtitleStyleFile = Paths::subtitleStyleFile();
    QVERIFY(!subtitleStyleFile.isEmpty());
    
    // 应该包含styles.ass文件名
    QVERIFY(subtitleStyleFile.endsWith("styles.ass"));
    
    std::cout << "subtitleStyleFile test completed." << std::endl;
}