/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>

#include "common/KMDynamicCreator.h"

// 测试用的简单派生类
class TestObject : public KMObject {
public:
    int value = 0;
};

// 测试注册和创建功能
TEST(KMDynamicCreatorTest, RegisterAndCreate) {
    // 注册测试类
    auto constructor = []() -> KMObject* { return new TestObject(); };
    int count = KMDynamicCreator::registerClass("test_id", "TestClass", constructor);
    EXPECT_GT(count, 0);  // 至少注册了一个类
    
    // 测试创建对象
    KMObject* obj = KMDynamicCreator::createObject("TestClass");
    EXPECT_NE(obj, nullptr);
    EXPECT_NE(dynamic_cast<TestObject*>(obj), nullptr);
    delete obj;
    
    // 测试通过ID创建对象
    obj = KMDynamicCreator::createObjectById("test_id");
    EXPECT_NE(obj, nullptr);
    EXPECT_NE(dynamic_cast<TestObject*>(obj), nullptr);
    delete obj;
}

// 测试错误情况
TEST(KMDynamicCreatorTest, ErrorCases) {
    // 测试创建不存在的类
    EXPECT_EQ(KMDynamicCreator::createObject("NonExistClass"), nullptr);
    
    // 测试通过不存在的ID创建
    EXPECT_EQ(KMDynamicCreator::createObjectById("non_exist_id"), nullptr);
    
    // 测试空构造器
    KMDynamicCreator::registerClass("empty_id", "EmptyClass", nullptr);
    EXPECT_EQ(KMDynamicCreator::createObject("EmptyClass"), nullptr);
}

// 测试多次注册
TEST(KMDynamicCreatorTest, MultipleRegistration) {
    auto constructor = []() -> KMObject* { return new TestObject(); };
    
    // 第一次注册
    int count1 = KMDynamicCreator::registerClass("id1", "Class1", constructor);
    EXPECT_GT(count1, 0);
    
    // 第二次注册
    int count2 = KMDynamicCreator::registerClass("id2", "Class2", constructor);
    EXPECT_GT(count2, count1);
    
    // 验证可以创建两个不同的类
    KMObject* obj1 = KMDynamicCreator::createObject("Class1");
    EXPECT_NE(obj1, nullptr);
    delete obj1;
    
    KMObject* obj2 = KMDynamicCreator::createObject("Class2");
    EXPECT_NE(obj2, nullptr);
    delete obj2;
}
