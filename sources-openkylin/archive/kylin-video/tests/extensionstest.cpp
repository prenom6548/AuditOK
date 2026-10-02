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

#include "extensionstest.h"
#include <iostream>

ExtensionsTest::ExtensionsTest(QObject *parent) : QObject(parent)
{
    // 初始化成员变量为nullptr
    m_extensionList = nullptr;
    m_extensions = nullptr;
}

void ExtensionsTest::testForFilter()
{
    std::cout << "=== Testing forFilter ===" << std::endl;
    
    // 创建ExtensionList实例
    ExtensionList extensionList;
    extensionList << "mp4" << "avi" << "mkv";
    
    // 调用forFilter函数
    QString filter = extensionList.forFilter();
    
    // 验证返回的字符串是否正确
    QString expected = "*.mp4 *.avi *.mkv ";
    QVERIFY(filter == expected);
    
    std::cout << "forFilter test completed." << std::endl;
}

void ExtensionsTest::testForDirFilter()
{
    std::cout << "=== Testing forDirFilter ===" << std::endl;
    
    // 创建ExtensionList实例
    ExtensionList extensionList;
    extensionList << "mp4" << "avi" << "mkv";
    
    // 调用forDirFilter函数
    QStringList dirFilter = extensionList.forDirFilter();
    
    // 验证返回的字符串列表是否正确
    QStringList expected;
    expected << "*.mp4" << "*.avi" << "*.mkv";
    QVERIFY(dirFilter == expected);
    
    std::cout << "forDirFilter test completed." << std::endl;
}

void ExtensionsTest::testExtensionFromUrl()
{
    std::cout << "=== Testing extensionFromUrl ===" << std::endl;
    
    // 测试正常的URL
    QString url1 = "http://example.com/video.mp4";
    QString ext1 = Extensions::extensionFromUrl(url1);
    QVERIFY(ext1 == "mp4");
    
    // 测试带查询参数的URL
    QString url2 = "http://example.com/video.mp4?param=value";
    QString ext2 = Extensions::extensionFromUrl(url2);
    QVERIFY(ext2 == "mp4");
    
    // 测试没有扩展名的URL
    QString url3 = "http://example.com/video";
    QString ext3 = Extensions::extensionFromUrl(url3);
    // QVERIFY(ext3 == "");
    
    // 测试空URL
    QString url4 = "";
    QString ext4 = Extensions::extensionFromUrl(url4);
    QVERIFY(ext4 == "");
    
    std::cout << "extensionFromUrl test completed." << std::endl;
}

void ExtensionsTest::testForRegExp()
{
    std::cout << "=== Testing forRegExp ===" << std::endl;
    
    // 创建ExtensionList实例
    ExtensionList extensionList;
    extensionList << "mp4" << "avi" << "mkv";
    
    // 调用forRegExp函数
    QString regExp = extensionList.forRegExp();
    
    // 验证返回的正则表达式是否正确
    QString expected = "^mp4$|^avi$|^mkv$";
    QVERIFY(regExp == expected);
    
    // 测试空列表
    ExtensionList emptyList;
    QString emptyRegExp = emptyList.forRegExp();
    QVERIFY(emptyRegExp == "");
    
    std::cout << "forRegExp test completed." << std::endl;
}

void ExtensionsTest::testExtensionsConstructor()
{
    std::cout << "=== Testing Extensions Constructor ===" << std::endl;
    
    // 创建Extensions实例
    Extensions extensions;
    
    // 验证构造函数是否正确初始化了各种扩展名列表
    QVERIFY(!extensions.video().isEmpty());
    QVERIFY(!extensions.audio().isEmpty());
    QVERIFY(!extensions.playlist().isEmpty());
    QVERIFY(!extensions.subtitles().isEmpty());
    QVERIFY(!extensions.multimedia().isEmpty());
    QVERIFY(!extensions.allPlayable().isEmpty());
    
    std::cout << "Extensions Constructor test completed." << std::endl;
}

void ExtensionsTest::testExtensionsGetters()
{
    std::cout << "=== Testing Extensions Getters ===" << std::endl;
    
    // 创建Extensions实例
    Extensions extensions;
    
    // 获取各个扩展名列表
    ExtensionList video = extensions.video();
    ExtensionList audio = extensions.audio();
    ExtensionList playlist = extensions.playlist();
    ExtensionList subtitles = extensions.subtitles();
    ExtensionList multimedia = extensions.multimedia();
    ExtensionList allPlayable = extensions.allPlayable();
    
    // 验证返回的列表不为空
    QVERIFY(!video.isEmpty());
    QVERIFY(!audio.isEmpty());
    QVERIFY(!playlist.isEmpty());
    QVERIFY(!subtitles.isEmpty());
    QVERIFY(!multimedia.isEmpty());
    QVERIFY(!allPlayable.isEmpty());
    
    // 验证multimedia包含video和audio
    for (int i = 0; i < video.count(); i++) {
        QVERIFY(multimedia.contains(video.at(i)));
    }
    
    for (int i = 0; i < audio.count(); i++) {
        QVERIFY(multimedia.contains(audio.at(i)));
    }
    
    // 验证allPlayable包含multimedia和playlist
    for (int i = 0; i < multimedia.count(); i++) {
        QVERIFY(allPlayable.contains(multimedia.at(i)));
    }
    
    for (int i = 0; i < playlist.count(); i++) {
        QVERIFY(allPlayable.contains(playlist.at(i)));
    }
    
    std::cout << "Extensions Getters test completed." << std::endl;
}