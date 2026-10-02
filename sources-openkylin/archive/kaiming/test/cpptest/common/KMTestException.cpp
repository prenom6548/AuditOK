#include <gtest/gtest.h>
#include "KMException.h"

class KMExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试用的错误消息
        testMsg1 = "Test error message";
        testMsg2 = "Another error message";
        emptyMsg = "";
    }

    std::string testMsg1;
    std::string testMsg2;
    std::string emptyMsg;
};

// 测试构造函数和what()方法
TEST_F(KMExceptionTest, ConstructorAndWhat) {
    KMException ex1(testMsg1);
    ASSERT_STREQ(ex1.what(), testMsg1.c_str());
    
    KMException ex2(testMsg2);
    ASSERT_STREQ(ex2.what(), testMsg2.c_str());
}

// 测试空错误消息
TEST_F(KMExceptionTest, EmptyErrorMessage) {
    KMException ex(emptyMsg);
    ASSERT_STREQ(ex.what(), "");
}

// 测试继承自std::exception
TEST_F(KMExceptionTest, IsStdException) {
    KMException ex(testMsg1);
    std::exception& baseEx = ex; // 向上转型
    
    ASSERT_STREQ(baseEx.what(), testMsg1.c_str());
}

// 测试异常抛出和捕获
TEST_F(KMExceptionTest, ThrowAndCatch) {
    try {
        throw KMException(testMsg1);
        FAIL() << "Expected KMException to be thrown";
    } 
    catch (const KMException& ex) {
        ASSERT_STREQ(ex.what(), testMsg1.c_str());
    }
    catch (...) {
        FAIL() << "Expected KMException but got different exception";
    }
}

// 测试通过std::exception捕获
TEST_F(KMExceptionTest, CatchAsStdException) {
    try {
        throw KMException(testMsg2);
        FAIL() << "Expected KMException to be thrown";
    } 
    catch (const std::exception& ex) {
        ASSERT_STREQ(ex.what(), testMsg2.c_str());
    }
    catch (...) {
        FAIL() << "Expected std::exception but got different exception";
    }
}

// 测试析构函数
TEST_F(KMExceptionTest, Destructor) {
    // 主要是验证析构不会抛出异常
    ASSERT_NO_THROW({
        KMException ex(testMsg1);
    });
    
    ASSERT_NO_THROW({
        KMException* pEx = new KMException(testMsg2);
        delete pEx;
    });
}