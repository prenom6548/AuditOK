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

#include "util.h"

namespace logger {
spdlog::level::level_enum getLogLevel() {
#ifdef RUN_IN_RELEASE
    std::filesystem::path configPath = std::filesystem::path(CONFIGS_PATH) / "runtime" / "log.json";
    std::string fileContent = util::readFile(configPath.string());

    if (fileContent.empty()) {
        LOGI("logger::getLogLevel() log.json is empty");
        return spdlog::level::info;
    }

    Json::Value root = util::stringToJson(fileContent);
    if (root.isNull() || !root.isMember("log_level")) {
        LOGI("logger::getLogLevel() log_level is not found");
        return spdlog::level::info;
    }
    int logLevel = root["log_level"].asInt();

    if (logLevel >= 0 && logLevel <= 6) {
        static spdlog::level::level_enum levels[] = {
            spdlog::level::trace, spdlog::level::debug,    spdlog::level::info, spdlog::level::warn,
            spdlog::level::err,   spdlog::level::critical, spdlog::level::off};
        LOGI("logger::getLogLevel() log_level: {}", logLevel);
        return levels[logLevel];
    }
    LOGI("logger::getLogLevel() log_level is invalid");
    return spdlog::level::info;
#else
    return spdlog::level::debug;
#endif
}

void init() {
    try {
        int maxMB = 5;
        std::filesystem::path logPath =
            std::filesystem::path(g_get_home_dir()) / ".log" / "kylin-ai-runtime" / "kylin-ai-runtime.log";
        std::string path = logPath.string();

        auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto sink2 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, 1024 * 1024 * maxMB, 5);
        std::vector<spdlog::sink_ptr> sinks = {sink1, sink2};

#ifdef RUN_IN_DEBUG
        sink1->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#:%!] [tid %t]\n%v\n");
        sink2->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#:%!] [tid %t]\n%v\n");
#else
        sink1->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#] [tid %t]\n%v\n");
        sink2->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^%l%$ [%s:%#] [tid %t]\n%v\n");
#endif

        auto logger = std::make_shared<spdlog::logger>("global log", sinks.begin(), sinks.end());
        logger->flush_on(spdlog::level::debug);  // 日志立即刷新
        spdlog::set_default_logger(logger);
        spdlog::set_level(getLogLevel());

        LOGI("===============================START===============================");
    } catch (std::exception& exception) {
        LOGE("logger::init() exception: {}", exception.what());
    }
}

}  // namespace logger
