#include <gtest/gtest.h>
#include "KMObject.h"

// 测试 className() 和 setClassName() 方法
TEST(KMObjectTest, ClassNameOperations) {
    KMObject obj;
    
    // 测试默认类名
    EXPECT_EQ(obj.className(), "KMObject");
    
    // 测试设置新类名
    obj.setClassName("NewClassName");
    EXPECT_EQ(obj.className(), "NewClassName");
    
    // 测试设置空类名
    obj.setClassName("");
    EXPECT_EQ(obj.className(), "");
    
    // 测试设置长类名
    std::string longName(100, 'x');
    obj.setClassName(longName);
    EXPECT_EQ(obj.className(), longName);
}

// 测试析构函数（基本验证）
TEST(KMObjectTest, Destructor) {
    std::unique_ptr<KMObject> obj = std::make_unique<KMObject>();
    EXPECT_NO_THROW(obj.reset()); // 验证析构不会抛出异常
}