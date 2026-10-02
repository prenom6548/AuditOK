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
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QString>
#include <memory>
#include "../../biometric-manager/src/treemodel.h"
#include "../../biometric-manager/src/treeitem.h"
#include "../../biometric-manager/src/customtype.h"

#include "stubext.h"
using namespace stub_ext;


class TreeModelTest : public ::testing::Test {
protected:
    std::unique_ptr<TreeModel> treeModel;


    void SetUp() override {
        treeModel = std::make_unique<TreeModel>(123, BIOTYPE_FACE);
    }


    void TearDown() override {
        treeModel.reset();
    }


    void setupTestDataForModel(TreeModel *model) {
        TreeItem *user1 = new TreeItem({"1", "kylin", "左拇指"}, model->rootItem);
        user1->setIndex(1);
        TreeItem *feature12 = new TreeItem({"1", "kylin", "右拇指"}, user1);
        feature12->setIndex(2);
        TreeItem *feature13 = new TreeItem({"", "", "左食指"}, user1);
        feature13->setIndex(3);
        user1->appendChild(feature12);
        user1->appendChild(feature13);


        TreeItem *user2 = new TreeItem({"2", "nackee", "中指"}, model->rootItem);
        user2->setIndex(1);


        TreeItem *user3 = new TreeItem({"3", "yh", "大拇指"}, model->rootItem);
        user3->setIndex(1);
        TreeItem *feature32 = new TreeItem({"", "", "无名指"}, user3);
        feature32->setIndex(2);
        user3->appendChild(feature32);


        model->rootItem->appendChild(user1);
        model->rootItem->appendChild(user2);
        model->rootItem->appendChild(user3);
    }
};


// 测试 TreeModel 的构造函数
TEST_F(TreeModelTest, Constructor) {
    EXPECT_NE(treeModel->rootItem, nullptr);
    EXPECT_EQ(treeModel->uid_, 123);
    EXPECT_EQ(treeModel->type_, BIOTYPE_FACE);
}


// 测试 columnCount 函数
TEST_F(TreeModelTest, ColumnCount) {
    // 根节点的列数
//    int rootColumnCount = treeModel->columnCount(QModelIndex());
//    EXPECT_EQ(rootColumnCount, treeModel->rootItem->columnCount());


//    // 子节点的列数
//    QModelIndex childIndex = treeModel->index(0, 0);
//    int childColumnCount = treeModel->columnCount(childIndex);
//    EXPECT_EQ(childColumnCount, treeModel->rootItem->child(0)->columnCount());
}


// 测试 rowCount 函数
TEST_F(TreeModelTest, RowCount) {
    // 根节点的行数
//    int rootRowCount = treeModel->rowCount(QModelIndex());
//    EXPECT_EQ(rootRowCount, treeModel->rootItem->childCount());


//    // 子节点的行数
//    QModelIndex childIndex = treeModel->index(0, 0);
//    int childRowCount = treeModel->rowCount(childIndex);
//    EXPECT_EQ(childRowCount, treeModel->rootItem->child(0)->childCount());
}


// 测试 roleNames 函数
TEST_F(TreeModelTest, RoleNames) {
    QHash<int, QByteArray> roles = treeModel->roleNames();
    EXPECT_EQ(roles[TreeModel::IndexRole], "index");
    EXPECT_EQ(roles[TreeModel::UidRole], "uid");
}


// 测试 data 函数
TEST_F(TreeModelTest, Data) {
    QModelIndex index = treeModel->index(0, 0);
    QVariant displayRoleData = treeModel->data(index, Qt::DisplayRole);
    QVariant userRoleData = treeModel->data(index, Qt::UserRole);
    QVariant uidRoleData = treeModel->data(index, TreeModel::UidRole);


    EXPECT_NE(displayRoleData, QVariant());
    EXPECT_NE(userRoleData, QVariant());
    EXPECT_NE(uidRoleData, QVariant());
}


// 测试 index 函数
TEST_F(TreeModelTest, Index) {
    QModelIndex index = treeModel->index(0, 0);
    EXPECT_EQ(index.column(), 0);


    QModelIndex childIndex = treeModel->index(0, 0, index);
    EXPECT_EQ(childIndex.parent(), index);
}


// 测试 parent 函数
TEST_F(TreeModelTest, Parent) {
    QModelIndex index = treeModel->index(0, 0);
    QModelIndex parentIndex = treeModel->parent(index);
    EXPECT_EQ(parentIndex, QModelIndex());


    QModelIndex childIndex = treeModel->index(0, 0, index);
    QModelIndex grandParentIndex = treeModel->parent(childIndex);
    EXPECT_EQ(grandParentIndex, index);
}


// 测试 flags 函数
TEST_F(TreeModelTest, Flags) {
    QModelIndex index = treeModel->index(0, 0);
    Qt::ItemFlags flags = treeModel->flags(index);
    EXPECT_EQ(flags, Qt::ItemIsEnabled | Qt::ItemIsSelectable);


    QModelIndex column1Index = treeModel->index(0, 1);
    Qt::ItemFlags column1Flags = treeModel->flags(column1Index);
    EXPECT_EQ(column1Flags, Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}


// 测试 headerData 函数
TEST_F(TreeModelTest, HeaderData) {
    QVariant headerData = treeModel->headerData(0, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_EQ(headerData, treeModel->rootItem->data(0));
}


// 测试 setData 函数
TEST_F(TreeModelTest, SetData) {
    QModelIndex index = treeModel->index(0, 0);
    bool result = treeModel->setData(index, "NewData", Qt::DisplayRole);
    EXPECT_TRUE(result);


    QVariant updatedData = treeModel->data(index, Qt::DisplayRole);
    EXPECT_EQ(updatedData.toString(), "NewData");
}


// 测试 setMySort 函数
TEST_F(TreeModelTest, SetMySort) {
    setupTestDataForModel(treeModel.get());


    // 保存排序前的状态
    QList<QString> beforeSort;
    for (int i = 0; i < treeModel->rowCount(QModelIndex()); ++i) {
        beforeSort.append(treeModel->data(treeModel->index(i, 1), Qt::DisplayRole).toString());
    }


    treeModel->setMySort();


    // 保存排序后的状态
    QList<QString> afterSort;
    for (int i = 0; i < treeModel->rowCount(QModelIndex()); ++i) {
        afterSort.append(treeModel->data(treeModel->index(i, 1), Qt::DisplayRole).toString());
    }


    // 简单比较排序前后是否有变化
    EXPECT_NE(beforeSort, afterSort);
}


// 测试 setupTestData 函数
TEST_F(TreeModelTest, SetupTestData) {
//    treeModel->setupTestData();


//    EXPECT_EQ(treeModel->rowCount(QModelIndex()), 3);
//    EXPECT_EQ(treeModel->rootItem->child(0)->childCount(), 2);
}


// 测试 setModelData 函数
TEST_F(TreeModelTest, SetModelData) {
    QList<FeatureInfo*> featureInfoList;
    // 假设 FeatureInfo 有相应的构造函数和数据设置
    featureInfoList.push_back(new FeatureInfo());


    treeModel->setModelData(featureInfoList);


    EXPECT_EQ(treeModel->rowCount(QModelIndex()), 1);
    // 根据 FeatureInfo 的设置检查数据
    // 例如：EXPECT_EQ(treeModel->data(treeModel->index(0, 0),...);


    for (auto featureInfo : featureInfoList) {
        delete featureInfo;
    }
}


// 测试 createItem 函数
TEST_F(TreeModelTest, CreateItem) {
    FeatureInfo featureInfo;
    TreeItem *item = treeModel->createItem(1, &featureInfo, TreeModel::NORMAL);


    EXPECT_NE(item, nullptr);
    EXPECT_EQ(item->data(0).toString(), "1");


    delete item;
}


// 测试 appendData 函数
TEST_F(TreeModelTest, AppendData) {
    FeatureInfo featureInfo;
    treeModel->appendData(&featureInfo);


    EXPECT_EQ(treeModel->rowCount(QModelIndex()), 1);


    QModelIndex index = treeModel->index(0, 0);
    EXPECT_EQ(treeModel->data(index, Qt::DisplayRole).toString(), "1");
}


// 测试 insertData 函数
TEST_F(TreeModelTest, InsertData) {
    FeatureInfo featureInfo;
    treeModel->insertData(&featureInfo);


    // 检查插入后的数据状态
    // 例如：EXPECT_EQ(treeModel->data(treeModel->index(0, 0),...);
}


// 测试 findInsertPosition 函数
TEST_F(TreeModelTest, FindInsertPosition) {
    setupTestDataForModel(treeModel.get());


    FeatureInfo featureInfo;
    int position = treeModel->findInsertPosition(&featureInfo, treeModel->rootItem);


    // 根据数据状态检查插入位置是否正确
    // 例如：EXPECT_EQ(position,...);
}


// 测试 removeRow 函数
TEST_F(TreeModelTest, RemoveRow) {
    setupTestDataForModel(treeModel.get());


    bool result = treeModel->removeRow(0);


    EXPECT_TRUE(result);
    EXPECT_EQ(treeModel->rowCount(QModelIndex()), 2);
}


// 测试 removeAll 函数
TEST_F(TreeModelTest, RemoveAll) {
    setupTestDataForModel(treeModel.get());


    treeModel->removeAll();


    EXPECT_EQ(treeModel->rowCount(QModelIndex()), 0);
}


// 测试 updateSerialNum 函数
TEST_F(TreeModelTest, UpdateSerialNum) {
    setupTestDataForModel(treeModel.get());


    treeModel->updateSerialNum();


    // 检查序列号是否更新
    // 例如：EXPECT_EQ(treeModel->data(treeModel->index(0, 0), "1");
}


// 测试 freeIndex 函数
TEST_F(TreeModelTest, FreeIndex) {
    int index = treeModel->freeIndex();


    // 根据数据状态检查空闲索引是否正确
    // 例如：EXPECT_EQ(index,...);
}


// 测试 hasFeature 函数
TEST_F(TreeModelTest, HasFeature) {
    setupTestDataForModel(treeModel.get());


    bool hasFeature = treeModel->hasFeature(123, "左拇指");


    EXPECT_TRUE(hasFeature);
}
