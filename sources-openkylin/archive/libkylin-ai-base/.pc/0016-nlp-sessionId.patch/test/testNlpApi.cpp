/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <thread>
#include <stdio.h>
#include <vector>
#include <fstream>

#include <gio/gio.h>
#include <gio/giotypes.h>
#include "kylin-ai/ai-base/nlp.h"
#include <kylin-ai/config.h>

void testNlp() {
    NlpSession session;
    nlp_create_session(&session);
    nlp_init_session(session);

    int a = 30;
    nlp_set_result_callback(session,
                [](const char* result, void* user_data) {
        std::cout << "--nlp---session----" << result
                  << "--user---\n" << *(int*)user_data << std::endl;
    }, &a);

//    nlp_text_chat_async(session, "我是谁");

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}

int main(int argc, char *argv[]) {
    testNlp();

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);

    return 0;
}
