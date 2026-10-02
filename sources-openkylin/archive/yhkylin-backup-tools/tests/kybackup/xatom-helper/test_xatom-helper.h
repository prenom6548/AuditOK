#ifndef TEST_XATOM_HELPER_H
#define TEST_XATOM_HELPER_H

#include <gtest/gtest.h>
#include <QApplication>
#include "xatom-helper.h"

class XAtomHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试用例开始前执行
    }

    void TearDown() override {
        // 在每个测试用例结束后执行
    }
};

#endif // TEST_XATOM_HELPER_H