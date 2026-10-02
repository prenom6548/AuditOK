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

#include "test_friendItemdelegate.h"
#include <QPainter>
#include <QPixmap>
#include <QStandardItem>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QTemporaryFile>
#include <QImage>

// 测试初始化
void TestFriendItemDelegate::initTestCase()
{
    // 在所有测试开始前执行一次
}

// 每个测试前的初始化
void TestFriendItemDelegate::init()
{
    // 初始化测试环境
    m_friendItemDelegate = new FriendItemDelegate();
    m_model = new QStandardItemModel();
}

// 测试构造函数和析构函数
void TestFriendItemDelegate::testConstructorAndDestructor()
{
    FriendItemDelegate* delegate = new FriendItemDelegate();
    QVERIFY(delegate != nullptr);
    delete delegate;
}

// 测试sizeHint方法
void TestFriendItemDelegate::testSizeHint()
{
    QStyleOptionViewItem option;
    QModelIndex index;
    
    QSize size = m_friendItemDelegate->sizeHint(option, index);
    QCOMPARE(size.width(), 350);
    QCOMPARE(size.height(), 80);
}

// 测试paint方法
void TestFriendItemDelegate::testPaint()
{
    // 创建一个QPixmap用于绘制
    QPixmap pixmap(350, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    
    // 创建模型项并设置数据
    QStandardItem* item = new QStandardItem();
    item->setData("", FriendListModel::Avatar);  // 头像文字
    item->setData("username", FriendListModel::Username);
    item->setData("nickname", FriendListModel::Nickname);
    item->setData("Hello World", FriendListModel::RecentMsgContent);
    item->setData("2023-01-01 12:00:00", FriendListModel::RecentMsgTime);
    item->setData(0, FriendListModel::UnreadMsgNum);
    item->setData(FriendListModel::Online, FriendListModel::OnlineState);
    item->setData(FriendListModel::PriDefault, FriendListModel::Priority);
    
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // 创建视图选项
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 350, 80);
    
    // 调用paint方法，不应该抛出异常
    m_friendItemDelegate->paint(&painter, option, index);
    
    // 验证绘制没有导致崩溃
    QVERIFY(!pixmap.isNull());
    
    // 清理
    m_model->removeRow(0);
}

// 测试paint方法 - 鼠标悬停状态
void TestFriendItemDelegate::testPaintMouseOver()
{
    // 创建一个QPixmap用于绘制
    QPixmap pixmap(350, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    
    // 创建模型项并设置数据
    QStandardItem* item = new QStandardItem();
    item->setData("A", FriendListModel::Avatar);  // 头像文字
    item->setData("username", FriendListModel::Username);
    item->setData("nickname", FriendListModel::Nickname);
    item->setData("Hello World", FriendListModel::RecentMsgContent);
    item->setData("2023-01-01 12:00:00", FriendListModel::RecentMsgTime);
    item->setData(0, FriendListModel::UnreadMsgNum);
    item->setData(FriendListModel::Online, FriendListModel::OnlineState);
    item->setData(FriendListModel::PriDefault, FriendListModel::Priority);
    
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // 创建视图选项
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 350, 80);
    option.state |= QStyle::State_MouseOver;  // 设置鼠标悬停状态
    
    // 调用paint方法，不应该抛出异常
    m_friendItemDelegate->paint(&painter, option, index);
    
    // 验证绘制没有导致崩溃
    QVERIFY(!pixmap.isNull());
    
    // 清理
    m_model->removeRow(0);
}

// 测试paint方法 - 选中状态
void TestFriendItemDelegate::testPaintHasFocus()
{
    // 创建一个QPixmap用于绘制
    QPixmap pixmap(350, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    
    // 创建模型项并设置数据
    QStandardItem* item = new QStandardItem();
    item->setData("A", FriendListModel::Avatar);  // 头像文字
    item->setData("username", FriendListModel::Username);
    item->setData("nickname", FriendListModel::Nickname);
    item->setData("Hello World", FriendListModel::RecentMsgContent);
    item->setData("2023-01-01 12:00:00", FriendListModel::RecentMsgTime);
    item->setData(0, FriendListModel::UnreadMsgNum);
    item->setData(FriendListModel::Online, FriendListModel::OnlineState);
    item->setData(FriendListModel::PriDefault, FriendListModel::Priority);
    
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // 创建视图选项
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 350, 80);
    option.state |= QStyle::State_HasFocus;  // 设置选中状态
    
    // 调用paint方法，不应该抛出异常
    m_friendItemDelegate->paint(&painter, option, index);
    
    // 验证绘制没有导致崩溃
    QVERIFY(!pixmap.isNull());
    
    // 清理
    m_model->removeRow(0);
}

// 测试paint方法 - 置顶状态
void TestFriendItemDelegate::testPaintStayTop()
{
    // 创建一个QPixmap用于绘制
    QPixmap pixmap(350, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    
    // 创建模型项并设置数据
    QStandardItem* item = new QStandardItem();
    item->setData("A", FriendListModel::Avatar);  // 头像文字
    item->setData("username", FriendListModel::Username);
    item->setData("nickname", FriendListModel::Nickname);
    item->setData("Hello World", FriendListModel::RecentMsgContent);
    item->setData("2023-01-01 12:00:00", FriendListModel::RecentMsgTime);
    item->setData(0, FriendListModel::UnreadMsgNum);
    item->setData(FriendListModel::Online, FriendListModel::OnlineState);
    item->setData(FriendListModel::PriStayTop, FriendListModel::Priority);  // 置顶状态
    
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // 创建视图选项
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 350, 80);
    
    // 调用paint方法，不应该抛出异常
    m_friendItemDelegate->paint(&painter, option, index);
    
    // 验证绘制没有导致崩溃
    QVERIFY(!pixmap.isNull());
    
    // 清理
    m_model->removeRow(0);
}

// 测试paint方法 - 离线状态
void TestFriendItemDelegate::testPaintOffline()
{
    // 创建一个QPixmap用于绘制
    QPixmap pixmap(350, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    
    // 创建模型项并设置数据
    QStandardItem* item = new QStandardItem();
    item->setData("A", FriendListModel::Avatar);  // 头像文字
    item->setData("username", FriendListModel::Username);
    item->setData("nickname", FriendListModel::Nickname);
    item->setData("Hello World", FriendListModel::RecentMsgContent);
    item->setData("2023-01-01 12:00:00", FriendListModel::RecentMsgTime);
    item->setData(0, FriendListModel::UnreadMsgNum);
    item->setData(FriendListModel::Offline, FriendListModel::OnlineState);  // 离线状态
    item->setData(FriendListModel::PriDefault, FriendListModel::Priority);
    
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // 创建视图选项
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 350, 80);
    
    // 调用paint方法，不应该抛出异常
    m_friendItemDelegate->paint(&painter, option, index);
    
    // 验证绘制没有导致崩溃
    QVERIFY(!pixmap.isNull());
    
    // 清理
    m_model->removeRow(0);
}

// 测试paint方法 - 有未读消息
void TestFriendItemDelegate::testPaintWithUnreadMsg()
{
    // 创建一个QPixmap用于绘制
    QPixmap pixmap(350, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    
    // 创建模型项并设置数据
    QStandardItem* item = new QStandardItem();
    item->setData("A", FriendListModel::Avatar);  // 头像文字
    item->setData("username", FriendListModel::Username);
    item->setData("nickname", FriendListModel::Nickname);
    item->setData("Hello World", FriendListModel::RecentMsgContent);
    item->setData("2023-01-01 12:00:00", FriendListModel::RecentMsgTime);
    item->setData(5, FriendListModel::UnreadMsgNum);  // 5条未读消息
    item->setData(FriendListModel::Online, FriendListModel::OnlineState);
    item->setData(FriendListModel::PriDefault, FriendListModel::Priority);
    
    m_model->appendRow(item);
    QModelIndex index = m_model->index(0, 0);
    
    // 创建视图选项
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 350, 80);
    
    // 调用paint方法，不应该抛出异常
    m_friendItemDelegate->paint(&painter, option, index);
    
    // 验证绘制没有导致崩溃
    QVERIFY(!pixmap.isNull());
    
    // 清理
    m_model->removeRow(0);
}

// 测试changeHighLightColor方法
void TestFriendItemDelegate::testChangeHighLightColor()
{
    // 覆盖 247-256
    QColor targetColor(255, 0, 0);  // 红色
    
    // 创建一个临时图片文件，包含各种类型的像素
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        QString path = tempFile.fileName();
        QImage testImage(10, 10, QImage::Format_ARGB32);
        testImage.fill(Qt::transparent);
        
        // 设置一些符合条件的像素 (非透明，非白色)
        testImage.setPixelColor(0, 0, QColor(100, 100, 100)); // 灰色，应该被替换
        
        // 设置一些不符合条件的像素
        testImage.setPixelColor(1, 1, qRgba(0, 0, 0, 1));    // 透明像素，应该 skip
        testImage.setPixelColor(2, 2, QColor(250, 250, 250)); // 白色像素 (250 > 205)，应该 skip
        
        testImage.save(path, "PNG");
        
        QPixmap result = m_friendItemDelegate->changeHighLightColor(targetColor, path);
        
        // 验证结果
        QImage resultImage = result.toImage();
        QCOMPARE(resultImage.pixelColor(0, 0), targetColor);
        QCOMPARE(resultImage.pixelColor(1, 1), QColor(qRgba(0, 0, 0, 1)));
        QCOMPARE(resultImage.pixelColor(2, 2), QColor(250, 250, 250));
    }
    
    // 验证方法调用没有导致崩溃
    QVERIFY(true);
}

// 每个测试后的清理
void TestFriendItemDelegate::cleanup()
{
    // 清理测试环境
    if (m_friendItemDelegate) {
        delete m_friendItemDelegate;
        m_friendItemDelegate = nullptr;
    }
    
    if (m_model) {
        delete m_model;
        m_model = nullptr;
    }
}

// 测试结束后的清理
void TestFriendItemDelegate::cleanupTestCase()
{
    // 在所有测试结束后执行一次
}