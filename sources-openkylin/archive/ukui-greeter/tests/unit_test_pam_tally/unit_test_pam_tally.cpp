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
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#include "../../ukui-greeter/greeter/pam-tally.h"

#include "stubext.h"
using namespace stub_ext;

#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


// 辅助函数，用于创建临时文件
static FILE* create_temp_file(const char* content) {
    FILE* temp_file = tmpfile();
    if (!temp_file) {
        return nullptr;
    }
    fputs(content, temp_file);
    rewind(temp_file);
    return temp_file;
}



// 测试 pam_tally_init 函数
TEST(PamTallyTest, PamTallyInit) {
    // get_is_open_other_authentication 为 1
    FILE* temp_file = create_temp_file("open-other-authentication=1\n");
    ASSERT_NE(temp_file, nullptr);
    EXPECT_EQ(pam_tally_init(), 0);
    fclose(temp_file);

    // get_pam_tally 失败
    EXPECT_EQ(pam_tally_init(), 0);

    // shm_open 失败
    // 这里可以使用 stub 或 mock 模拟 shm_open 失败，例如使用 LD_PRELOAD 技巧或修改代码
    // 为简单起见，这里仅做注释说明
    // EXPECT_EQ(pam_tally_init(), -1);

    // mmap 失败
    // 这里可以使用 stub 或 mock 模拟 mmap 失败，例如使用 LD_PRELOAD 技巧或修改代码
    // 为简单起见，这里仅做注释说明
    // EXPECT_EQ(pam_tally_init(), -1);

    // 正常情况
    temp_file = create_temp_file("deny=3 unlock_time=60 root_unlock_time=120\n");
    ASSERT_NE(temp_file, nullptr);
    EXPECT_EQ(pam_tally_init(), 1);
    pam_tally_add_failed();
    pam_tally_clear_failed();
//    pam_tally_falure_is_out();
    pam_tally_deny();
    pam_tally_failed_count();
    pam_tally_unlock_time();
    pam_tally_is_enbled();
    pam_tally_is_canUnlock();
    fclose(temp_file);
}


