#ifndef TEST_ABOUTDIALOG_H
#define TEST_ABOUTDIALOG_H

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include "aboutdialog.h"

class AboutDialogTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试用例开始前执行
        aboutDialog = new AboutDialog();
    }

    void TearDown() override {
        // 在每个测试用例结束后执行
        if (aboutDialog) {
            delete aboutDialog;
            aboutDialog = nullptr;
        }
    }

    AboutDialog* aboutDialog = nullptr;
};

#endif // TEST_ABOUTDIALOG_H