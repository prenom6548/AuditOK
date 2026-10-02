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

#include <gtest/gtest.h>

#include <iostream>

#include "../src/services/assistant/intentdialoguemanager/systemoperation/systemoperation.h"
static void testSystemOperation() {
    SystemOperation systemOperation;
    DialogueData::Intent intent;

    intent.name = "系统操作_设置显示器色温与护眼";
    intent.slots.push_back(DialogueData::Slot({"操作", false, {"打开"}}));
    intent.slots.push_back(DialogueData::Slot({"操作", false, {"打开"}}));
    DialogueData::CallSuccessIdentifier callSuccess = systemOperation.callSystemControlInterface(intent);

    if (callSuccess.successRes == true) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
        std::cout << callSuccess.errorMessage << std::endl;
        std::cout << callSuccess.errorCode << std::endl;
    }
}

static void testScheduleManagement() {
    SystemOperation systemOperation;
    // std::string intent = R"(```json{
    //     "日程主题": "城市智慧交通建设方案研讨",
    //     "日程开始时间": "2025-03-06 19:25",
    //     "日程结束时间": "2025-03-07 21:30"
    // }```)";
    std::string intent = R"("日程主题": "准备开秋季学期教学进度表报告会",
    "日程开始时间": "2025-04-09 09:00",
    "日程结束时间": "2025-04-09 10:00"
})";
    DialogueData::CallSuccessIdentifier callSuccess = systemOperation.callSystemScheduleManagementInterface(intent);

    if (callSuccess.successRes == true) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
        std::cout << callSuccess.errorMessage << std::endl;
        std::cout << callSuccess.errorCode << std::endl;
    }
}
static void testToDoList() {
    SystemOperation systemOperation;
    std::string intent = R"(# 2025年6月5日 星期二
- 10:00:00-23:59:59，参加远程医疗技术研讨会，了解最新医疗科技进展。

# 2025年6月6日 星期五
- 00:00:00-23:59:59，与医疗团队会议，讨论患者病例，制定个性化治疗方案。

# 2025年6月7日 星期三
- 00:00:00-23:59:59，安排值班医生，确保紧急医疗服务不受影响。

# 2025年6月8日 星期四
- 完成手术，确保患者安全。
)";
    DialogueData::CallSuccessIdentifier callSuccess = systemOperation.callSystemToDoListInterface(intent);

    if (callSuccess.successRes == true) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
        std::cout << callSuccess.errorMessage << std::endl;
        std::cout << callSuccess.errorCode << std::endl;
    }
}

static void testOpenSchedule() {
    SystemOperation systemOperation;
    std::string intent = "2025-03-10";
    DialogueData::CallSuccessIdentifier callSuccess = systemOperation.callSystemOpenScheduleInterface(intent);
    if (callSuccess.successRes == true) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "failed" << std::endl;
        std::cout << callSuccess.errorMessage << std::endl;
        std::cout << callSuccess.errorCode << std::endl;
    }
}

TEST(SystemOperationTest, ToDoList) {
    logger::init();
    testToDoList();
}
