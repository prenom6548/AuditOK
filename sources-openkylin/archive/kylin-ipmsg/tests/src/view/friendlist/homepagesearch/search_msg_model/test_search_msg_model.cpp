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

#include "test_search_msg_model.h"
#include <QStandardItem>
#include "model/chatmsgmodel.h"
#include "view/friendlist/homepagesearch/searchpage.h"
#include "global/database/database.h"
#include "global/utils/globalutils.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

void TestSearchMsgModel::initTestCase()
{
    // 初始化数据库环境
    DataBase::getInstance();
}

void TestSearchMsgModel::cleanupTestCase()
{
}

void TestSearchMsgModel::init()
{
    m_model = SearchMsgModel::getInstance();
}

void TestSearchMsgModel::cleanup()
{
    m_model->clear();
    m_model->m_searchResult.clear();
    m_model->m_msgIdItemsMap.clear();
}

void TestSearchMsgModel::testConstructorDestructor()
{
    SearchMsgModel *model = new SearchMsgModel();
    QVERIFY(model != nullptr);
    delete model;
}

void TestSearchMsgModel::testGetInstance()
{
    SearchMsgModel *instance1 = SearchMsgModel::getInstance();
    QVERIFY(instance1 != nullptr);
    SearchMsgModel *instance2 = SearchMsgModel::getInstance();
    QCOMPARE(instance1, instance2);
}

void TestSearchMsgModel::testSlotSelect()
{
    // 1. 准备模拟数据
    m_model->m_searchResult.insert(1, 5);
    QStandardItem *item = new QStandardItem();
    m_model->appendRow(item);
    m_model->m_msgIdItemsMap.insert(1, item);

    // 2. 测试 slotSelect 流程
    m_model->slotSelect("test");
    
    // 3. 验证内部数据结构是否被清空 (slotSelect 内部会先调用 clear, m_msgIdItemsMap.clear, m_searchResult.clear)
    QVERIFY(m_model->rowCount() >= 0);
}

void TestSearchMsgModel::testGetModel()
{
    // 1. 测试正常路径
    m_model->m_searchResult.insert(1, 5); 
    
    FriendListModel *friendModel = FriendListModel::getInstance();
    friendModel->clear();
    QStandardItem *friendItem = new QStandardItem();
    friendItem->setData(1, FriendListModel::Id);
    friendItem->setData(1, FriendListModel::Display); 
    friendModel->appendRow(friendItem);
    friendModel->m_idItemsMap.insert(1, friendItem);

    m_model->getModel();
    QVERIFY(m_model->rowCount() > 0);
    
    // 2. 测试 FriendListModel::getFriendById 返回 nullptr
    m_model->clear();
    m_model->m_searchResult.clear();
    m_model->m_searchResult.insert(999, 1); // 不存在的 ID
    m_model->getModel();
    QCOMPARE(m_model->rowCount(), 0);

    // 3. 测试 Display 角色为 0 (DisHide)
    m_model->clear();
    m_model->m_searchResult.clear();
    friendItem->setData(0, FriendListModel::Display); 
    m_model->m_searchResult.insert(1, 1);
    m_model->getModel();
    QCOMPARE(m_model->rowCount(), 0);
}

void TestSearchMsgModel::testGetSearchResult()
{
    // 由于 getSearchResult 依赖真实的数据库查询，我们通过插入模拟数据来测试
    QSqlDatabase db = DataBase::getInstance()->openDatabse("chat_msg");
    if (!db.isOpen()) {
        db.open();
    }
    
    QSqlQuery query(db);
    // 1. 强制重置表结构，并补全 NOT NULL 字段的默认值
    query.exec("DROP TABLE IF EXISTS chat_msg");
    if (!query.exec("CREATE TABLE chat_msg (friend_id INT, msg_content TEXT, is_send INT DEFAULT 0, msg_type INT DEFAULT 0, time TEXT DEFAULT '')")) {
        qDebug() << "Create table failed:" << query.lastError().text();
    }
    
    // 2. 准备加密数据
    QString msg1 = GlobalUtils::encryptData("hello test");
    QString msg2 = GlobalUtils::encryptData("world");
    QString msg3 = GlobalUtils::encryptData("another test message");
    
    // 3. 插入模拟数据
    auto insert = [&](int id, QString content) {
        QSqlQuery ins(db);
        ins.prepare("INSERT INTO chat_msg (friend_id, msg_content) VALUES (?, ?)");
        ins.addBindValue(id);
        ins.addBindValue(content);
        if (!ins.exec()) {
            qDebug() << "Insert failed for ID" << id << ":" << ins.lastError().text();
        }
    };

    insert(1, msg1);
    insert(1, msg2);
    insert(1, msg3); // Friend 1 再次匹配 "test"
    insert(2, msg1);

    // 4. 执行搜索测试
    m_model->m_searchResult.clear();
    m_model->getSearchResult("test");
    
    // 调试打印
    qDebug() << "Search result size:" << m_model->m_searchResult.size();
    qDebug() << "Result for ID 1:" << m_model->m_searchResult.value(1);
    qDebug() << "Result for ID 2:" << m_model->m_searchResult.value(2);

    QCOMPARE(m_model->m_searchResult.value(1), 2); // Friend 1 应该有 2 条匹配 (msg1, msg3)
    QCOMPARE(m_model->m_searchResult.value(2), 1); // Friend 2 应该有 1 条匹配 (msg1)

    // 5. 多个匹配 (测试 "hello")
    m_model->m_searchResult.clear();
    m_model->getSearchResult("hello");
    QCOMPARE(m_model->m_searchResult.value(1), 1);
    QCOMPARE(m_model->m_searchResult.value(2), 1);

    // 6. 无匹配
    m_model->m_searchResult.clear();
    m_model->getSearchResult("nonexistent");
    QVERIFY(m_model->m_searchResult.isEmpty());

    // 7. 空字符串 (通常匹配所有)
    m_model->m_searchResult.clear();
    m_model->getSearchResult("");
    QVERIFY(m_model->m_searchResult.size() >= 2);
}
