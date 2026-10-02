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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "logger.h"

#include <glib.h>

#include <filesystem>

namespace loggerNlp {

void init() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        int max_mb = 5;
        std::filesystem::path log_path =
            std::filesystem::path(g_get_home_dir()) / ".log" /
            "kylin-ondevice-nlp-engine.log";
        std::string path = log_path.string();

        auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto sink2 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            path, 1024 * 1024 * max_mb, 5);
        std::vector<spdlog::sink_ptr> sinks = {sink1, sink2};

#ifdef RUN_IN_DEBUG
        sink1->set_pattern(
            "[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#:%!] [tid %t]\n%v\n");
        sink2->set_pattern(
            "[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#:%!] [tid %t]\n%v\n");
#else
        sink1->set_pattern(
            "[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#] [tid %t]\n%v\n");
        sink2->set_pattern(
            "[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#] [tid %t]\n%v\n");
#endif

        auto logger = std::make_shared<spdlog::logger>(
            "nlp_engine_log", sinks.begin(), sinks.end());
        logger->flush_on(spdlog::level::debug); // 日志立即刷新
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::debug);

        LOGI(">>>>>>>>>>>>>>>>>>>>>>>>>> ondevice nlp START "
             "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    });
}

} // namespace loggerNlp
