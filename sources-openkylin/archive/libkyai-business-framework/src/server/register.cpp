/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include "register.h"

#include "log.h"

std::map<std::string, scene::SyncWorkHandler> scene::ServiceRegister::methodHandlerMap;
std::map<std::string, scene::AsyncWorkHandler> scene::ServiceRegister::asyncMethodHandlerMap;
std::map<std::string, scene::AsyncWorkHandler> scene::ServiceRegister::asyncFlowMethodHandlerMap;

void scene::ServiceRegister::registerHandler(const std::string& method, scene::SyncWorkHandler handler) {
    PRINT_DEBUG("register handler method: %s\n", method.c_str());

    auto ret = methodHandlerMap.insert(std::make_pair(method, handler));
    if (!ret.second) {
        PRINT_WARN("%s already register.\n", method.c_str());
        return;
    }
}

void scene::ServiceRegister::asyncRegisterHandler(const std::string& method, scene::AsyncWorkHandler asyncHandler) {
    PRINT_DEBUG("async register handler method: %s\n", method.c_str());

    auto ret = asyncMethodHandlerMap.insert(std::make_pair(method, asyncHandler));
    if (!ret.second) {
        PRINT_WARN("%s already register.\n", method.c_str());
        return;
    }
}

void scene::ServiceRegister::asyncFlowRegisterHandler(const std::string& method, scene::AsyncWorkHandler asyncHandler) {
    PRINT_DEBUG("async flow register handler method: %s\n", method.c_str());

    auto ret = asyncFlowMethodHandlerMap.insert(std::make_pair(method, asyncHandler));
    if (!ret.second) {
        PRINT_WARN("%s already register.\n", method.c_str());
        return;
    }
}
