/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QVariant>
#include <QString>
#include <memory>
#include "../../biometric-manager/src/treeitem.h"

#include "stubext.h"
using namespace stub_ext;


class TreeItemTest : public ::testing::Test {
protected:
    std::unique_ptr<TreeItem> parentItem;


    void SetUp() override {
        QList<QVariant> parentData = { "ParentData1", "ParentData2", "ParentData3" };
        parentItem = std::make_unique<TreeItem>(parentData);
    }


    void TearDown() override {
        parentItem.reset();
    }
};


// 测试 TreeItem 的构造函数
TEST_F(TreeItemTest, Constructor) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    int uid = 123;
    int index = 0;
    TreeItem item(data, nullptr, uid, index);


    // 检查数据成员是否正确初始化
    EXPECT_EQ(item.columnCount(), 3);
    EXPECT_EQ(item.getUid(), uid);
    EXPECT_EQ(item.getIndex(), index);
    EXPECT_EQ(item.data(0).toString(), "Data1");
    EXPECT_EQ(item.data(1).toString(), "Data2");
    EXPECT_EQ(item.data(2).toString(), "Data3");
}


// 测试添加子项的函数
TEST_F(TreeItemTest, AppendChild) {
    QList<QVariant> childData = { "ChildData1", "ChildData2", "ChildData3" };
    TreeItem *child = new TreeItem(childData, parentItem.get());


    parentItem->appendChild(child);


    // 检查子项是否添加成功
    EXPECT_EQ(parentItem->childCount(), 1);
    EXPECT_EQ(parentItem->child(0)->data(0).toString(), "ChildData1");
    EXPECT_EQ(parentItem->child(0)->data(1).toString(), "ChildData2");
    EXPECT_EQ(parentItem->child(0)->data(2).toString(), "ChildData3");


//    delete child;
}


// 测试插入子项的函数
TEST_F(TreeItemTest, InsertChild) {
    QList<QVariant> childData1 = { "ChildData1", "ChildData2", "ChildData3" };
    QList<QVariant> childData2 = { "ChildData4", "ChildData5", "ChildData6" };
    TreeItem *child1 = new TreeItem(childData1, parentItem.get());
    TreeItem *child2 = new TreeItem(childData2, parentItem.get());


    parentItem->appendChild(child1);
    parentItem->insertChild(0, child2);


    // 检查子项是否插入成功
    EXPECT_EQ(parentItem->childCount(), 2);
    EXPECT_EQ(parentItem->child(0)->data(0).toString(), "ChildData4");
    EXPECT_EQ(parentItem->child(1)->data(0).toString(), "ChildData1");


//    delete child1;
//    delete child2;
}


// 测试获取子项的函数
TEST_F(TreeItemTest, Child) {
    QList<QVariant> childData = { "ChildData1", "ChildData2", "ChildData3" };
    TreeItem *child = new TreeItem(childData, parentItem.get());


    parentItem->appendChild(child);


    // 检查获取子项是否正确
    EXPECT_EQ(parentItem->child(0), child);


//    delete child;
}


// 测试获取子项数量的函数
TEST_F(TreeItemTest, ChildCount) {
    QList<QVariant> childData1 = { "ChildData1", "ChildData2", "ChildData3" };
    QList<QVariant> childData2 = { "ChildData4", "ChildData5", "ChildData6" };
    TreeItem *child1 = new TreeItem(childData1, parentItem.get());
    TreeItem *child2 = new TreeItem(childData2, parentItem.get());


    parentItem->appendChild(child1);
    parentItem->appendChild(child2);


    // 检查子项数量是否正确
    EXPECT_EQ(parentItem->childCount(), 2);


//    delete child1;
//    delete child2;
}


// 测试获取列数的函数
TEST_F(TreeItemTest, ColumnCount) {
    QList<QVariant> data = { "Data1", "Data2", "Data3", "Data4" };
    TreeItem item(data);


    // 检查列数是否正确
    EXPECT_EQ(item.columnCount(), 4);
}


// 测试获取数据的函数
TEST_F(TreeItemTest, Data) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    TreeItem item(data);


    // 检查获取数据是否正确
    EXPECT_EQ(item.data(0).toString(), "Data1");
    EXPECT_EQ(item.data(1).toString(), "Data2");
    EXPECT_EQ(item.data(2).toString(), "Data3");
}


// 测试设置数据的函数
TEST_F(TreeItemTest, SetData) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    TreeItem item(data);


    item.setData(1, "NewData2");


    // 检查数据是否设置成功
    EXPECT_EQ(item.data(1).toString(), "NewData2");
}


// 测试获取行号的函数
TEST_F(TreeItemTest, Row) {
    QList<QVariant> parentData = { "ParentData1", "ParentData2", "ParentData3" };
    TreeItem parent(parentData);


    QList<QVariant> childData = { "ChildData1", "ChildData2", "ChildData3" };
    TreeItem *child = new TreeItem(childData, &parent);


    parent.appendChild(child);


    // 检查行号是否正确
    EXPECT_EQ(child->row(), 0);


//    delete child;
}


// 测试获取父项的函数
TEST_F(TreeItemTest, Parent) {
    QList<QVariant> parentData = { "ParentData1", "ParentData2", "ParentData3" };
    TreeItem parent(parentData);


    QList<QVariant> childData = { "ChildData1", "ChildData2", "ChildData3" };
    TreeItem *child = new TreeItem(childData, &parent);


    // 检查父项是否正确
    EXPECT_EQ(child->parent(), &parent);


    delete child;
}


// 测试设置父项的函数
TEST_F(TreeItemTest, SetParent) {
    QList<QVariant> parentData = { "ParentData1", "ParentData2", "ParentData3" };
    TreeItem parent(parentData);


    QList<QVariant> childData = { "ChildData1", "ChildData2", "ChildData3" };
    TreeItem *child = new TreeItem(childData);


    child->setParent(&parent);


    // 检查父项是否设置成功
    EXPECT_EQ(child->parent(), &parent);


    delete child;
}


// 测试设置索引的函数
TEST_F(TreeItemTest, SetIndex) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    TreeItem item(data);


    item.setIndex(5);


    // 检查索引是否设置成功
    EXPECT_EQ(item.getIndex(), 5);
}


// 测试获取索引的函数
TEST_F(TreeItemTest, GetIndex) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    TreeItem item(data);


    // 检查索引是否正确
    EXPECT_EQ(item.getIndex(), 0);
}


// 测试获取 UID 的函数
TEST_F(TreeItemTest, GetUid) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    int uid = 123;
    TreeItem item(data, nullptr, uid);


    // 检查 UID 是否正确
    EXPECT_EQ(item.getUid(), uid);
}


// 测试设置 UID 的函数
TEST_F(TreeItemTest, SetUid) {
    QList<QVariant> data = { "Data1", "Data2", "Data3" };
    TreeItem item(data);


    item.setUid(456);


    // 检查 UID 是否设置成功
    EXPECT_EQ(item.getUid(), 456);
}


// 测试删除子项的函数
TEST_F(TreeItemTest, RemoveChild) {
    QList<QVariant> childData1 = { "ChildData1", "ChildData2", "ChildData3" };
    QList<QVariant> childData2 = { "ChildData4", "ChildData5", "ChildData6" };
    TreeItem *child1 = new TreeItem(childData1, parentItem.get());
    TreeItem *child2 = new TreeItem(childData2, parentItem.get());


    parentItem->appendChild(child1);
    parentItem->appendChild(child2);


    // 测试普通删除
    bool result = parentItem->removeChild(0, false);
    EXPECT_TRUE(result);
    EXPECT_EQ(parentItem->childCount(), 1);


    // 测试递归删除
    result = parentItem->removeChild(0, true);
    EXPECT_TRUE(result);
    EXPECT_EQ(parentItem->childCount(), 0);


//    delete child1;
//    delete child2;
}


// 测试删除多个子项的函数
TEST_F(TreeItemTest, RemoveChildren) {
    QList<QVariant> childData1 = { "ChildData1", "ChildData2", "ChildData3" };
    QList<QVariant> childData2 = { "ChildData4", "ChildData5", "ChildData6" };
    QList<QVariant> childData3 = { "ChildData7", "ChildData8", "ChildData9" };
    TreeItem *child1 = new TreeItem(childData1, parentItem.get());
    TreeItem *child2 = new TreeItem(childData2, parentItem.get());
    TreeItem *child3 = new TreeItem(childData3, parentItem.get());


    parentItem->appendChild(child1);
    parentItem->appendChild(child2);
    parentItem->appendChild(child3);


    // 测试删除多个子项
    bool result = parentItem->removeChildren(0, 2, false);
    EXPECT_TRUE(result);
    EXPECT_EQ(parentItem->childCount(), 1);


//    delete child1;
//    delete child2;
//    delete child3;
}


// 测试清理子项的函数
TEST_F(TreeItemTest, CleanChildren) {
    QList<QVariant> childData1 = { "ChildData1", "ChildData2", "ChildData3" };
    QList<QVariant> childData2 = { "ChildData4", "ChildData5", "ChildData6" };
    TreeItem *child1 = new TreeItem(childData1, parentItem.get());
    TreeItem *child2 = new TreeItem(childData2, parentItem.get());


    parentItem->appendChild(child1);
    parentItem->appendChild(child2);


    parentItem->cleanChildren();


    // 检查子项是否清理干净
    EXPECT_EQ(parentItem->childCount(), 0);


//    delete child1;
//    delete child2;
}


// 测试排序函数
TEST_F(TreeItemTest, Sort) {
    QList<QVariant> childData1 = { "Data1", "2" };
    QList<QVariant> childData2 = { "Data2", "1" };
    QList<QVariant> childData3 = { "Data3", "3" };
    TreeItem *child1 = new TreeItem(childData1, parentItem.get());
    TreeItem *child2 = new TreeItem(childData2, parentItem.get());
    TreeItem *child3 = new TreeItem(childData3, parentItem.get());


    parentItem->appendChild(child1);
    parentItem->appendChild(child2);
    parentItem->appendChild(child3);


    parentItem->sort();


    // 检查排序结果
    EXPECT_EQ(parentItem->child(0)->data(1).toString(), "1");
    EXPECT_EQ(parentItem->child(1)->data(1).toString(), "2");
    EXPECT_EQ(parentItem->child(2)->data(1).toString(), "3");


//    delete child1;
//    delete child2;
//    delete child3;
}
