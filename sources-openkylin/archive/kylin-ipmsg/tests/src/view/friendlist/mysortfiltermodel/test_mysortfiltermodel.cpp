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

#include "test_mysortfiltermodel.h"
#include "model/friendlistmodel.h"
#include "global/utils/globalutils.h"

// 测试 getCompareData 逻辑
TEST_F(TestMySortFilterModel, GetCompareDataTest) {
    // 使用 QDateTime 直接生成时间戳，避免依赖 GlobalUtils 的格式
    QString format = "yyyy-MM-dd HH:mm:ss.zzz";
    QString time1 = "2023-01-01 12:00:00.000";
    QString time2 = "2023-01-01 13:00:00.000";
    
    // 确保主项目代码能解析这个时间
    // 我们手动计算预期值来验证逻辑
    
    qint64 data1 = MySortFilterModel::getCompareData(1, 1, time1);
    qint64 data2 = MySortFilterModel::getCompareData(1, 1, time2);
    
    // 如果 data1 == data2，说明时间解析失败了
    if (data1 == data2) {
        qDebug() << "Time parsing failed in getCompareData. Check GlobalUtils::getTimeFormat().";
        qDebug() << "Current format:" << GlobalUtils::getTimeFormat();
    }
    
    EXPECT_GT(data2, data1);
    
    qint64 dataOnline = MySortFilterModel::getCompareData(1, 0, time1);
    qint64 dataOffline = MySortFilterModel::getCompareData(0, 0, time1);
    
    EXPECT_GT(dataOnline, dataOffline);
}

// 测试过滤逻辑
TEST_F(TestMySortFilterModel, FilterAcceptsRowTest) {
    QStandardItem *item1 = new QStandardItem("Alice");
    item1->setData("AliceNickname", FriendListModel::Nickname);
    item1->setData("AliceUser", FriendListModel::Username);
    
    QStandardItem *item2 = new QStandardItem("Bob");
    item2->setData("", FriendListModel::Nickname);
    item2->setData("BobUser", FriendListModel::Username);
    
    sourceModel->appendRow(item1);
    sourceModel->appendRow(item2);
    
    // 设置过滤正则
    proxyModel->setFilterRegExp("Alice");
    EXPECT_EQ(proxyModel->rowCount(), 1);
    
    proxyModel->setFilterRegExp("Bob");
    EXPECT_EQ(proxyModel->rowCount(), 1);
    
    proxyModel->setFilterRegExp("Unknown");
    EXPECT_EQ(proxyModel->rowCount(), 0);
}

// 测试排序逻辑 (lessThan)
TEST_F(TestMySortFilterModel, LessThanTest) {
    QStandardItem *item1 = new QStandardItem("Alice");
    item1->setData(1, FriendListModel::OnlineState); // Online
    item1->setData(1, FriendListModel::Priority);
    item1->setData("2023-01-01 12:00:00.000", FriendListModel::RecentMsgTime);
    
    QStandardItem *item2 = new QStandardItem("Bob");
    item2->setData(0, FriendListModel::OnlineState); // Offline
    item2->setData(0, FriendListModel::Priority);
    item2->setData("2023-01-01 11:00:00.000", FriendListModel::RecentMsgTime);
    
    sourceModel->appendRow(item1);
    sourceModel->appendRow(item2);
    
    // 确保时间格式正确，否则 getCompareData 会返回相同的值（如果解析失败）
    // item1 的 compareData 应该大于 item2
    
    EXPECT_TRUE(proxyModel->lessThan(sourceModel->index(0, 0), sourceModel->index(1, 0)));
    EXPECT_FALSE(proxyModel->lessThan(sourceModel->index(1, 0), sourceModel->index(0, 0)));
}


