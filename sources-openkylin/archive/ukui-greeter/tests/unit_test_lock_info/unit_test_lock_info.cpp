/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#include "../../ukui-greeter/greeter/lockinfo.h"
#include <gtest/gtest.h>
#include <QDBusReply>
#include <QDBusError>
#include <QDebug>


class LockInfoTest : public ::testing::Test {
protected:

};


// 测试 getSystemdInhibitor 函数
TEST_F(LockInfoTest, GetSystemdInhibitorTest) {
    // 测试正常情况
    bool result = lockinfo::getSystemdInhibitor(QString("shutdown"));
    EXPECT_TRUE(result);


    result = lockinfo::getSystemdInhibitor(QString("sleep"));
    EXPECT_FALSE(result);
}
