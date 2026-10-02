/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef SERVICES_ASSISTANT_SYSTEM_OPERATION_INTERFACE_H
#define SERVICES_ASSISTANT_SYSTEM_OPERATION_INTERFACE_H

#include <string>

#include "assistant/intentdialoguemanager/dialoguemessage.h"

class SystemOperationInterface {
public:
    virtual ~SystemOperationInterface() = default;

    virtual DialogueData::CallSuccessIdentifier callSystemControlInterface(
        const DialogueData::Intent& scenarioIntent) = 0;  // 系统操作控制接口

    virtual DialogueData::CallSuccessIdentifier callSystemScheduleManagementInterface(std::string inputContent) = 0;

    virtual DialogueData::CallSuccessIdentifier callSystemToDoListInterface(const std::string& inputContent) = 0;

    virtual DialogueData::CallSuccessIdentifier callSystemOpenScheduleInterface(const std::string& inputContent) = 0;

    virtual void callSourceUrlTool(int toolId) = 0;
};

#endif  // SERVICES_ASSISTANT_SYSTEM_OPERATION_INTERFACE_H