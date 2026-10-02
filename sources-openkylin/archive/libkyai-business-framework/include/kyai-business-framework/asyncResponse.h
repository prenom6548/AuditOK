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

#include <string>

#include "response.h"

namespace scene {
extern bool asyncResponse(const std::string& taskId, const Response& response);
extern bool asyncFlowResponse(const std::string& taskId, const Response& response, bool end);
}  // namespace scene

/**
 * 异步返回：
 * 结果成功或失败都调用 ASYNC_RESPONSE
 */
#define ASYNC_RESPONSE(taskId, response) scene::asyncResponse(taskId, response)

/**
 * 异步流式返回：
 * 每阶段处理成功调用 ASYNC_FLOW_RESPONSE_CONTINUE
 * 最后处理完成调用 ASYNC_FLOW_RESPONSE_END
 * 每阶段处理失败调用 ASYNC_FLOW_RESPONSE_END
 */
#define ASYNC_FLOW_RESPONSE(taskId, response, end) scene::asyncFlowResponse(taskId, response, end)
#define ASYNC_FLOW_RESPONSE_CONTINUE(taskId, response) ASYNC_FLOW_RESPONSE(taskId, response, false)
#define ASYNC_FLOW_RESPONSE_END(taskId, response) ASYNC_FLOW_RESPONSE(taskId, response, true)
