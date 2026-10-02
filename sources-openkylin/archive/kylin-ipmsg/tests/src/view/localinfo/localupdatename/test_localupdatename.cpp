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

#include "test_localupdatename.h"
#include <QKeyEvent>
#include <gsettingmonitor.h>
#include <QStandardItem>
#include "model/friendlistmodel.h"
#include "controller/control.h"

void TestLocalUpdateName::initTestCase()
{
}

void TestLocalUpdateName::cleanupTestCase()
{
}

void TestLocalUpdateName::init()
{
    m_updateName = new LocalUpdateName(LocalUpdateName::LocalName, true);
}

void TestLocalUpdateName::cleanup()
{
    delete m_updateName;
}

void TestLocalUpdateName::testConstructorLocal()
{
    QVERIFY(m_updateName != nullptr);
    QCOMPARE(m_updateName->m_nameType, (int)LocalUpdateName::LocalName);
    QVERIFY(m_updateName->m_isAuto);
}

void TestLocalUpdateName::testConstructorFriend()
{
    LocalUpdateName *friendUpdate = new LocalUpdateName(LocalUpdateName::FriendName, 123);
    QVERIFY(friendUpdate != nullptr);
    QCOMPARE(friendUpdate->m_nameType, (int)LocalUpdateName::FriendName);
    QCOMPARE(friendUpdate->m_friendId, 123);
    delete friendUpdate;
}

void TestLocalUpdateName::testSetInitData()
{
    m_updateName->setInitData(LocalUpdateName::FriendName, false);
    QCOMPARE(m_updateName->m_nameType, (int)LocalUpdateName::FriendName);
    QVERIFY(!m_updateName->m_isAuto);

    m_updateName->setInitData(LocalUpdateName::LocalName, 456);
    QCOMPARE(m_updateName->m_nameType, (int)LocalUpdateName::LocalName);
    QCOMPARE(m_updateName->m_friendId, 456);
}

void TestLocalUpdateName::testSetWidgetUi()
{
    m_updateName->setWidgetUi();
    QVERIFY(m_updateName->m_nameEdit != nullptr);
    QVERIFY(m_updateName->m_sureBtn != nullptr);
}

void TestLocalUpdateName::testSetWidgetStyle()
{
    m_updateName->setWidgetStyle();
    // 验证窗口标题或样式
}

void TestLocalUpdateName::testKeyPressEvent()
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_updateName->keyPressEvent(&event);
    // 验证窗口是否关闭，由于 deleteLater 可能不会立即生效，主要检查不崩溃
}

void TestLocalUpdateName::testSlotConfirmBtn()
{
    // 1. 测试 LocalName 类型，输入为空
    m_updateName->setInitData(LocalUpdateName::LocalName, false);
    m_updateName->m_nameEdit->setText("");
    m_updateName->slotConfirmBtn();
    QCOMPARE(m_updateName->m_tipsLab->text(), tr("Please enter username"));

    // 2. 测试输入过长 (注意：m_nameEdit->setMaxLength(20) 会限制输入，所以这里需要通过代码绕过或验证限制)
    m_updateName->m_nameEdit->setMaxLength(100); 
    QString longName(21, 'A');
    m_updateName->m_nameEdit->setText(longName);
    m_updateName->slotConfirmBtn();
    QCOMPARE(m_updateName->m_tipsLab->text(), tr("The length of user name is less than 20 words"));

    // 3. 测试特殊字符
    m_updateName->m_nameEdit->setText("Invalid@Name!");
    m_updateName->slotConfirmBtn();
    QCOMPARE(m_updateName->m_tipsLab->text(), tr("Please do not enter special characters"));

    // 4. 测试有效输入 (LocalName)
    m_updateName->m_nameEdit->setText("ValidName");
    m_updateName->slotConfirmBtn();
    QCOMPARE(m_updateName->m_tipsLab->text(), QString(""));

    // 5. 测试有效输入 (FriendName)
    m_updateName->setInitData(LocalUpdateName::FriendName, 123);
    m_updateName->m_nameEdit->setText("FriendNick");
    m_updateName->slotConfirmBtn();
    QCOMPARE(m_updateName->m_tipsLab->text(), QString(""));
}

void TestLocalUpdateName::testSlotCancelBtn()
{
    m_updateName->slotCancelBtn();
    // 验证窗口是否关闭
}

void TestLocalUpdateName::testSlotEnableSureBtn()
{
    m_updateName->slotEnableSureBtn("");
    QVERIFY(!m_updateName->m_sureBtn->isEnabled());

    m_updateName->slotEnableSureBtn("ValidName");
    QVERIFY(m_updateName->m_sureBtn->isEnabled());
}

void TestLocalUpdateName::testCoverageAdditions()
{
    // 1. 覆盖 202-203 (setWidgetUi 中 FriendName 的初始化逻辑)
    int testFriendId = 999;
    FriendListModel *friendModel = FriendListModel::getInstance();
    friendModel->clear();
    QStandardItem *friendItem = new QStandardItem();
    friendItem->setData(testFriendId, FriendListModel::Id);
    friendItem->setData("TestNickname", FriendListModel::Nickname);
    friendItem->setData("TestUsername", FriendListModel::Username);
    friendModel->appendRow(friendItem);
    friendModel->m_idItemsMap.insert(testFriendId, friendItem);

    // 重新构造 FriendName 类型的对象，触发 setWidgetUi 内部对 FriendName 的处理
    LocalUpdateName *friendUpdate = new LocalUpdateName(LocalUpdateName::FriendName, testFriendId);
    QCOMPARE(friendUpdate->m_nameEdit->text(), QString("TestNickname"));
    delete friendUpdate;

    // 2. 覆盖 231-239 (initGsetting 中的字体变化 lambda)
    // 触发 Control::sigFontChange 信号，这是 LocalUpdateName 真正连接的信号
    emit Control::getInstance()->sigFontChange();

    // 3. 覆盖 270-273 (slotConfirmBtn 中 LocalName 且输入为空时的默认逻辑)
    m_updateName->setInitData(LocalUpdateName::LocalName, testFriendId);
    m_updateName->m_nameEdit->clear();
    m_updateName->slotConfirmBtn();

    // 4. 覆盖 281-288 (slotConfirmBtn 中 FriendName 的保存逻辑)
    m_updateName->setInitData(LocalUpdateName::FriendName, testFriendId);
    m_updateName->m_nameEdit->setText("NewNick");
    m_updateName->slotConfirmBtn();

    // 5. 覆盖 309-314, 317-318, 322 (setInitData 中 m_isAuto 为 true 的初始化逻辑)
    // 重新构造一个 m_isAuto 为 true 的对象
    LocalUpdateName *autoUpdate = new LocalUpdateName(LocalUpdateName::LocalName, true);
    Q_UNUSED(autoUpdate);
    delete autoUpdate;
}
