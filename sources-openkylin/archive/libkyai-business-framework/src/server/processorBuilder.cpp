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

#include "processorBuilder.h"

#include "processorWrapper.h"

/**
 * 根据服务名称创建processor
 * @param serviceName
 * @param connection
 * @return
 */
std::shared_ptr<scene::BaseProcessor> scene::ProcessorBuilder::build(const std::string& serviceName,
                                                                     GDBusConnection* connection) {
    return std::make_shared<ProcessorWrapper>(serviceName, connection);
}
