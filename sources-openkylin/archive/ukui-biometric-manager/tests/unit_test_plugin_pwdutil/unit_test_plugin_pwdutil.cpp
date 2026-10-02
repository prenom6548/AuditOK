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
#include <QSettings>
#include <QFile>
#include <unistd.h>
#include <QTextStream>
#include <QTemporaryFile>
#include <QApplication>
#include <kysdk/kysdk-system/libkysysinfo.h>
#include "../../biometric-plugin/passwdcheckutil.h"
#include "../../biometric-plugin/pwdcheckthread.h"
#include "../../biometric-plugin/pwdchangethread.h"

#include "stubext.h"
using namespace stub_ext;


class PasswdCheckUtilTest : public ::testing::Test {
protected:
    std::unique_ptr<PasswdCheckUtil> passwdCheckUtil;
    std::unique_ptr<PwdCheckThread> passwdCheckThread;
    std::unique_ptr<PwdChangeThread> passwdChangeThread;

    void SetUp() override {
        passwdCheckUtil = std::make_unique<PasswdCheckUtil>();
        passwdCheckThread = std::make_unique<PwdCheckThread>();
        passwdChangeThread = std::make_unique<PwdChangeThread>();
    }


    void TearDown() override {
        passwdCheckUtil.reset();
        passwdCheckThread.reset();
        passwdChangeThread.reset();
    }


    // 创建一个临时文件并写入内容，用于测试文件读取相关函数
    QString createTempFile(const QString& content) {
        QTemporaryFile tempFile;
        if (tempFile.open()) {
            QTextStream stream(&tempFile);
            stream << content;
            tempFile.close();
            return tempFile.fileName();
        }
        return "";
    }

    // 删除临时文件
    void removeTempFile(const QString& filename) {
        QFile::remove(filename);
    }
};


// 测试 PasswdCheckUtil 的构造函数
TEST_F(PasswdCheckUtilTest, Constructor) {
    EXPECT_NE(passwdCheckUtil, nullptr);
    EXPECT_NE(passwdCheckThread, nullptr);
    EXPECT_NE(passwdChangeThread, nullptr);
}


//TEST_F(BiometricProxyTest, GetMaxFailedAutoRetry)
//{
//    int value = passwdCheckThread->setArgs(getenv("USER"));
//    ASSERT_EQ(value, 5);
//}

// 测试 getCurrentPamState 函数
TEST_F(PasswdCheckUtilTest, GetCurrentPamState) {
    // 创建临时文件进行测试
    QString tempFileName = createTempFile("pam_pwquality.so");
    if (tempFileName.isEmpty()) {
        FAIL() << "Failed to create temporary file";
    }

    bool result = passwdCheckUtil->getCurrentPamState();
    EXPECT_TRUE(result);

    // 删除临时文件
    removeTempFile(tempFileName);

    // 创建不包含 pam_pwquality.so 的临时文件
    createTempFile("some other content");
    result = passwdCheckUtil->getCurrentPamState();
    EXPECT_FALSE(result);

    removeTempFile(tempFileName);
}


// 测试 isCommunity 函数
TEST_F(PasswdCheckUtilTest, IsCommunity) {
    // 创建临时文件进行测试
    QString tempFileName = createTempFile("VERSION_ID=22.04");
    if (tempFileName.isEmpty()) {
        FAIL() << "Failed to create temporary file";
    }


    // 修改测试中使用的文件路径
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempFileName.left(tempFileName.lastIndexOf('/')));


    bool result = passwdCheckUtil->isCommunity();
    EXPECT_TRUE(result);


    // 删除临时文件
    removeTempFile(tempFileName);


    // 创建非 22.04 版本的临时文件
    tempFileName = createTempFile("VERSION_ID=23.04");
    if (tempFileName.isEmpty()) {
        FAIL() << "Failed to create temporary file";
    }
    result = passwdCheckUtil->isCommunity();
    EXPECT_FALSE(result);


    removeTempFile(tempFileName);


    // 恢复原始文件路径
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "/etc");
}


// 测试 isWayland 函数
TEST_F(PasswdCheckUtilTest, IsWayland) {
    // 设置环境变量
    setenv("XDG_SESSION_TYPE", "wayland", 1);
    bool result = passwdCheckUtil->isWayland();
    EXPECT_TRUE(result);


    // 清除环境变量
    unsetenv("XDG_SESSION_TYPE");
    result = passwdCheckUtil->isWayland();
    EXPECT_FALSE(result);
}


// 测试 isOpenkylin 函数
TEST_F(PasswdCheckUtilTest, IsOpenkylin) {
    // 模拟系统名称
//    auto originalSystemName = kdk_system_get_systemName;
//    kdk_system_get_systemName = []() -> const char* {
//        return "openkylin";
//    };


//    bool result = passwdCheckUtil->isOpenkylin();
//    EXPECT_TRUE(result);


//    // 恢复原始系统名称获取函数
//    kdk_system_get_systemName = originalSystemName;


//    kdk_system_get_systemName = []() -> const char* {
//        return "ubuntu";
//    };


//    result = passwdCheckUtil->isOpenkylin();
//    EXPECT_FALSE(result);
}


// 测试 checkPassword 函数
TEST_F(PasswdCheckUtilTest, CheckPassword) {
    // 密码长度小于默认最小长度
    int result = passwdCheckUtil->checkPassword("abc", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_MIN_LENGTH);


    // 密码包含的字符类别少于默认最小类别
    result = passwdCheckUtil->checkPassword("abcde", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_MIN_CLASSES);


    // 新密码和旧密码相同
    result = passwdCheckUtil->checkPassword("old", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_SAME_PASSWORD);


    // 密码是回文
    result = passwdCheckUtil->checkPassword("abba", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_PALINDROME);


    // 密码仅大小写不同
    result = passwdCheckUtil->checkPassword("OLD", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_CASE_CHANGES_ONLY);


    // 密码过于相似
    result = passwdCheckUtil->checkPassword("oldnew", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_TOO_SIMILAR);


    // 正常密码
    result = passwdCheckUtil->checkPassword("Abc123!@#", "old", "user");
    EXPECT_EQ(result, PWCHK_ERROR_SUCCESS);
}


// 测试 checkPasswordStrerr 函数
TEST_F(PasswdCheckUtilTest, CheckPasswordStrerr) {
    QString errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_MIN_LENGTH);
    EXPECT_EQ(errorStr, QString(QApplication::tr("The password is shorter than %1 characters")).arg(PWCHK_DEFAULT_MIN_LENGTH));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_MIN_CLASSES);
    EXPECT_EQ(errorStr, QString(QApplication::tr("The password contains less than %1 character classes")).arg(PWCHK_DEFAULT_MIN_CLASS));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_SAME_PASSWORD);
    EXPECT_EQ(errorStr, QApplication::tr("The password is the same as the old one"));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_USER_CHECK);
    EXPECT_EQ(errorStr, QApplication::tr("The password contains the user name in some form"));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_CASE_CHANGES_ONLY);
    EXPECT_EQ(errorStr, QApplication::tr("The password differs with case changes only"));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_TOO_SIMILAR);
    EXPECT_EQ(errorStr, QApplication::tr("The password is too similar to the old one"));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_PALINDROME);
    EXPECT_EQ(errorStr, QApplication::tr("The password is a palindrome"));


    errorStr = passwdCheckUtil->checkPasswordStrerr(PWCHK_ERROR_SUCCESS);
    EXPECT_EQ(errorStr, "");
}

