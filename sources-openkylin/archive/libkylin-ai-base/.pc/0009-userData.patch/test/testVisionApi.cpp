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
#include <gio/gio.h>
#include <gio/giotypes.h>
#include "../include/kylin-ai/ai-base/vision.h"

int main(int argc, char *argv[]) {
    VisionSession session;
    vision_create_session(&session);
    vision_init_session(session);

    void* a = nullptr;
    vision_set_prompt2image_callback(
                session,
                [](VisionImageData data, void* user_data) {
        std::cout << "---testVisionApi--data--"
                  << data.data << "--index--" << data.index
                  << "--total--" << data.total
                  << "--width--" << data.width << std::endl;
    },
    a);
//    vision_set_prompt2image_size(session, 1024, 1024);
//    vision_set_prompt2image_number(session, 1);
//    vision_set_prompt2image_style(session, VisionImageStyle::EXPLORE_INFINITY);

//    vision_prompt2image_async(session, "花");

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);

    return 0;
}

