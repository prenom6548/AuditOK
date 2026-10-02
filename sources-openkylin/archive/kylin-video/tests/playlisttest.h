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
#ifndef PLAYLISTTEST_H
#define PLAYLISTTEST_H

#include <QObject>
#include <QtTest/QtTest>

class MainWindow;
class PlayList;

class PlayListTest : public QObject
{
    Q_OBJECT

public:
    explicit PlayListTest(QObject *parent = nullptr);

private slots:
    // 测试 addDirectory 函数
    void testAddDirectory();
    
    // 测试 deleteInvalidItems 函数
    void testDeleteInvalidItems();
    
    // 测试 setAvaliable 函数
    void testSetAvaliable();
    
    // 测试 addMark 函数
    void testAddMark();
    
    // 测试 deleteMark 函数
    void testDeleteMark();
    
    // 测试 initData 函数
    void testInitData();
    
    // 测试 clearList 函数
    void testClearList();
    
    // 测试 updateIndex 函数
    void testUpdateIndex();
    
    // 测试 setPlayIndex 函数
    void testSetPlayIndex();
    
    // 测试 playNext 函数
    void testPlayNext();
    
    // 测试 playPrev 函数
    void testPlayPrev();
    
    // 测试 setNotAvaliable 函数
    void testSetNotAvaliable();
    
    // 测试 addFiles 函数
    void testAddFiles();
    
    // 测试 deleteFile 函数
    void testDeleteFile();
    
    // 测试 playOrder 函数
    void testPlayOrder();
    
    // 测试 addDirectory 错误处理
    void testAddDirectoryError();
    
    // 测试 addFiles 错误处理
    void testAddFilesError();
    
    // 测试 playNext 错误处理
    void testPlayNextError();
    
    // 测试 playPrev 错误处理
    void testPlayPrevError();

private:
    MainWindow* m_win { nullptr };
    PlayList* m_playList { nullptr };
    
    // 声明友元类，以便访问MainWindow的私有成员
    friend class MainWindow;
};

#endif // PLAYLISTTEST_H