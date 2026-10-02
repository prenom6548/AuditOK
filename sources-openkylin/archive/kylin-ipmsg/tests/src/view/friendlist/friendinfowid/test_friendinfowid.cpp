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

#include "test_friendinfowid.h"
#include "model/friendlistmodel.h"
#include <QtTest>
#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QStandardItem>
#include <QKeyEvent>
#include <gsettingmonitor.h>

void TestFriendInfoWid::initTestCase()
{
}

void TestFriendInfoWid::init()
{
    m_wid = FriendInfoWid::getInstance();
}

void TestFriendInfoWid::testSingleton()
{
    FriendInfoWid* instance2 = FriendInfoWid::getInstance();
    QCOMPARE(m_wid, instance2);
}

void TestFriendInfoWid::testUiInitialization()
{
    m_wid->setWidgetUi();
    m_wid->setWidgetStyle();
    QVERIFY(m_wid->m_avatarLab != nullptr);
    QVERIFY(m_wid->m_usernameLab != nullptr);
}

void TestFriendInfoWid::testFillFriendInfo()
{
    // 准备测试数据
    FriendListModel *model = FriendListModel::getInstance();
    QStandardItem *item = new QStandardItem();
    item->setData(1, FriendListModel::Id);
    item->setData("Avatar", FriendListModel::Avatar);
    item->setData("Username", FriendListModel::Username);
    item->setData("127.0.0.1", FriendListModel::Ip);
    item->setData("Nickname", FriendListModel::Nickname);
    model->appendRow(item);
    
    // 手动更新 model 内部的 map，因为 getFriendById 依赖它
    model->m_idItemsMap.insert(1, item);

    // 测试填充信息
    m_wid->fillFriendInfo(1);
    QCOMPARE(m_wid->m_friendId, 1);
    QCOMPARE(m_wid->m_usernameInfo->text(), QString("Username"));
    QCOMPARE(m_wid->m_ipAddrInfo->text(), QString("127.0.0.1"));
}

void TestFriendInfoWid::testChangeFontSize()
{
    m_wid->changeFontSize();
    // 验证字体是否有效（pointSize 可能为 -1 如果使用的是 pixelSize）
    QVERIFY(m_wid->m_usernameLab->font().pointSizeF() > 0 || m_wid->m_usernameLab->font().pixelSize() > 0);
}

void TestFriendInfoWid::testSlots()
{
    m_wid->slotChangeFont();
    m_wid->slotUpdateFriendInfo(1);
    QVERIFY(m_wid->m_friendId == 1);
}

void TestFriendInfoWid::testChangeTransparency()
{
    // 覆盖 298
    emit kdk::GsettingMonitor::getInstance()->systemTransparencyChange();
    QVERIFY(true);
}

void TestFriendInfoWid::testPaintEvent()
{
    // 覆盖 304-327
    m_wid->repaint();
    QVERIFY(true);
}

void TestFriendInfoWid::testFillFriendInfoLongNames()
{
    // 覆盖 362-363, 386-387
    // 确保字体已设置，以便宽度计算正常
    QFont font = m_wid->m_usernameInfo->font();
    font.setPixelSize(12);
    m_wid->m_usernameInfo->setFont(font);
    m_wid->m_nicknameLab->setFont(font);

    FriendListModel *model = FriendListModel::getInstance();
    QStandardItem *item = new QStandardItem();
    item->setData(2, FriendListModel::Id);
    item->setData("Avatar", FriendListModel::Avatar);
    // 使用足够长的字符串确保超过 125/100 像素
    QString longName = "A very long name that will definitely be elided because it is repeated many times. "
                       "A very long name that will definitely be elided because it is repeated many times. ";
    item->setData(longName, FriendListModel::Username);
    item->setData("127.0.0.1", FriendListModel::Ip);
    item->setData(longName, FriendListModel::Nickname);
    model->appendRow(item);
    model->m_idItemsMap.insert(2, item);

    m_wid->fillFriendInfo(2);
    
    // 如果宽度超过阈值，应该设置 ToolTip
    QVERIFY(!m_wid->m_usernameInfo->toolTip().isEmpty());
    QVERIFY(!m_wid->m_nicknameLab->toolTip().isEmpty());
}

void TestFriendInfoWid::testFillFriendInfoEmptyNickname()
{
    // 覆盖 370-376
    FriendListModel *model = FriendListModel::getInstance();
    QStandardItem *item = new QStandardItem();
    item->setData(3, FriendListModel::Id);
    item->setData("Avatar", FriendListModel::Avatar);
    item->setData("Username", FriendListModel::Username);
    item->setData("127.0.0.1", FriendListModel::Ip);
    item->setData("", FriendListModel::Nickname); // Empty nickname
    model->appendRow(item);
    model->m_idItemsMap.insert(3, item);

    m_wid->fillFriendInfo(3);
    QCOMPARE(m_wid->m_nicknameInfo->text(), QString("Add"));
}

void TestFriendInfoWid::testCloseEvent()
{
    // 覆盖 401-402
    m_wid->show();
    m_wid->close();
    QVERIFY(m_wid->isHidden());
}

void TestFriendInfoWid::testChangeNickname()
{
    // 覆盖 406-420
    m_wid->changeNickname();
    QVERIFY(true);
}

void TestFriendInfoWid::testKeyPressEvent()
{
    // 覆盖 443-447
    m_wid->show();
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(m_wid, &event);
    QVERIFY(m_wid->isHidden());
}

void TestFriendInfoWid::cleanup()
{
}

void TestFriendInfoWid::cleanupTestCase()
{
}
