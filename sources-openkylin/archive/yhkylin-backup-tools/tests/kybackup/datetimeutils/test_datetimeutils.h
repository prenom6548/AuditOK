#ifndef TEST_DATETIMEUTILS_H
#define TEST_DATETIMEUTILS_H

#include <gtest/gtest.h>
#include <QApplication>
#include <QDateTime>
#include "datetimeutils.h"

class DateTimeUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试用例开始前执行
        datetimeUtils = new DateTimeUtils();
    }

    void TearDown() override {
        // 在每个测试用例结束后执行
        if (datetimeUtils) {
            delete datetimeUtils;
            datetimeUtils = nullptr;
        }
    }

    DateTimeUtils* datetimeUtils = nullptr;
};

#endif // TEST_DATETIMEUTILS_H