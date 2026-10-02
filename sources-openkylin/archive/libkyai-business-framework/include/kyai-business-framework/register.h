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

#pragma once

#include <map>
#include <string>

#include "request.h"
#include "response.h"

namespace scene {
using SyncWorkHandler = Response (*)(const Request& request);  // 业务同步接口的函数格式
using AsyncWorkHandler = Response (*)(const std::string& taskId, const Request& request);  // 业务异步接口的函数格式

class ServiceRegister {
public:
    struct Register {
        Register(const std::string& method, SyncWorkHandler handler) { registerHandler(method, handler); }

        Register(const std::string& method, AsyncWorkHandler asyncHandler, bool isFlow) {
            if (isFlow) {
                asyncFlowRegisterHandler(method, asyncHandler);
            } else {
                asyncRegisterHandler(method, asyncHandler);
            }
        }
    };

    static void registerHandler(const std::string& method, SyncWorkHandler handler);
    static void asyncRegisterHandler(const std::string& method, AsyncWorkHandler asyncHandler);
    static void asyncFlowRegisterHandler(const std::string& method, AsyncWorkHandler asyncHandler);

public:
    static std::map<std::string, SyncWorkHandler> methodHandlerMap;  // 同步接口名与业务处理函数的映射
    static std::map<std::string, AsyncWorkHandler> asyncMethodHandlerMap;  // 异步接口名与业务处理函数的映射
    static std::map<std::string, AsyncWorkHandler> asyncFlowMethodHandlerMap;  // 异步接口名与业务处理函数的映射
};
}  // namespace scene

// 业务同步接口注册时需要调用
#define REGISTER_SERVICE_HANDLER(methodName, syncHandler) \
    static scene::ServiceRegister::Register reg_##syncHandler##__LINE__(methodName, syncHandler);
// 业务异步接口注册时需要调用
#define REGISTER_SERVICE_ASYNC_HANDLER(methodName, asyncHandler) \
    static scene::ServiceRegister::Register reg_##asyncHandler##__LINE__(methodName, asyncHandler, false);
// 业务异步流式返回接口注册时需要调用
#define REGISTER_SERVICE_ASYNC_FLOW_HANDLER(methodName, asyncHandler) \
    static scene::ServiceRegister::Register reg_##asyncHandler##__LINE__(methodName, asyncHandler, true);
