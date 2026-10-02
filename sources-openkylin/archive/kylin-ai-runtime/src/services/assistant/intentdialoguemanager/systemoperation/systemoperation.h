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

#ifndef SERVICES_ASSISTANT_SYSTEM_OPERATION_H
#define SERVICES_ASSISTANT_SYSTEM_OPERATION_H

#include <string>

#include "APIs/apiplatform.h"
#include "systemoperationinterface.h"

enum class SystemCommandType {
    typeNotRecognized = -1,
    systemControlWithSlot,
    systemControlWithoutSlot,
    thirdPartyAppControl,
    scheduleManagement,
    localDataSearch,
};
enum class SchedulingType {
    todoList,
    createSchedule,
    openSchedule,
};
struct SchedulingTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
};
struct SchedulingInfo {
    std::string script;
    SchedulingTime startTime;
    SchedulingTime endTime;
};

class SystemOperation : public SystemOperationInterface {
public:
    SystemOperation() = default;

    ~SystemOperation() override = default;

    DialogueData::CallSuccessIdentifier callSystemControlInterface(const DialogueData::Intent& scenarioIntent) override;

    void callSourceUrlTool(int toolId) override;

    DialogueData::CallSuccessIdentifier callSystemScheduleManagementInterface(std::string inputContent) override;

    DialogueData::CallSuccessIdentifier callSystemToDoListInterface(const std::string& inputContent) override;

    DialogueData::CallSuccessIdentifier callSystemOpenScheduleInterface(const std::string& inputContent) override;

private:
    SystemCommandType systemCommandDiscrimination(const std::string& name);

    bool thirdPartyAppControlHandle(const std::string& cmdName, const std::vector<DialogueData::Slot>& slots) const;
    bool systemControlWithSlotHandle(const std::string& cmdName, const std::vector<DialogueData::Slot>& slots) const;
    bool systemControlWithoutSlotHandle(const std::string& cmdName, const std::vector<DialogueData::Slot>& slots) const;
    bool scheduleManagementHandle(const std::string& cmdName, const std::vector<DialogueData::Slot>& slots) const;

    bool systemControlOperation(const std::string& name, const std::string& value,
                                const std::vector<DialogueData::Slot>& slots = {}) const;
    bool systemAppLaunchOperation(const std::vector<DialogueData::Slot>& slots) const;
    bool systemMediaPlayerOperation(const std::vector<DialogueData::Slot>& slots) const;

    std::string getAppActionType(const std::vector<DialogueData::Slot>& slots) const;
    std::string getAppName(const std::vector<DialogueData::Slot>& slots) const;
    bool callCreateSystemApi(const std::string& name, const std::string& value,
                             const std::vector<DialogueData::Slot>& slots = {}) const;
    SchedulingInfo getSchedulingInfo(const Json::Value& input);
    std::vector<SchedulingInfo> getSchedulingInfo(const std::string& input);
    bool parseTime(const std::string& input, SchedulingTime& time, SchedulingType type);
    void extractDateFromTaskLine(const std::string& input, SchedulingTime& startTime, SchedulingTime& endTime);
    void extractTimeAndScriptFromContentLine(const std::string& input, std::string& script, SchedulingTime& startTime,
                                             SchedulingTime& endTime);
    bool isTaskLine(const std::string& str);
    bool isValidContentLine(const std::string& str);
    bool callDbusCalendar(const SchedulingInfo& input, SchedulingType type) const;
    bool callDbusMagnifier() const;
    std::string formatJsonFromSchedulingInputContent(const std::string& input);

private:
    const std::vector<std::string> SystemCommandNamesWithoutSlot_{
        "修改用户密码",
        "选择输入输出设备",
        "设置此时间段后关闭显示器",
        "设置此时间段后系统进入睡眠",
        "查询系统配置信息",
        "查询系统状态",
        "设置开机启动应用",
        "设置登录选项",
        "设置系统音效",
        "设置触控板手势",
        "设置触摸屏手势",
        "有线网络启用",
        "设置主题",
        "屏保设置",
        "系统更新",
        "打开安全中心",
        "设置显示器分辨率",
        "设置显示器屏幕方向",
        "修改用户头像",
        "添加其他用户",
        "设置显示器刷新率",
        "设置提示音",
        "锁屏个性化设置",
        "设置系统通知",
        "远程桌面启用",
        "打印机设置",
        "触控板滚动区域设置",
        "设备快捷键设置",
        "键盘设置",
        "系统代理",
        "移动热点启用",
        "设置时间和日期",
        "区域语言格式设置",
        "系统备份还原",
        "无线局域网启用",
        "设置鼠标滚动方向",
        "飞行模式启用",
        "截屏",
    };
    const std::vector<std::string> SystemCommandNamesWithSlot_{
        "切换显示器投屏方式",
        "设置显示器屏幕缩放",
        "设置显示器亮度",
        "音量控制",
        "切换电源模式",
        "蓝牙操作",
        "设置鼠标指针大小",
        "触控板启用",
        "打字时禁用触控板设置",
        "侧边栏显示平板模式",
        "激活屏保时锁定屏幕设置",
        "合并任务栏上的图标",
        "设置任务栏在屏幕上的位置",
        "设置任务栏大小",
        "自动隐藏任务栏",
        "锁定任务栏",
        "自定义时区时间",
        "设置系统字体",
        "音量增强启用",
        "设置唤醒屏幕时需要密码",
        "设置鼠标指针速度",
        "设置鼠标滚轮速度",
        "鼠标加速启用",
        "设置在文本区域闪烁光标",
        "插入鼠标时禁用触控板设置",
        "设置触控板指针速度",
        "VPN启用",
        "设置窗口外观",
        "任务栏多任务视图启用",
        "设置免密登录",
        "设置开机自动登录",
        "设置显示器色温与护眼",
        "设置侦听此设备",
        "设置鼠标主按钮",
        "设置鼠标光标闪烁频率",
        "设置鼠标双击间隔时长",
        "触控板点击功能启用",
        "滚动条跟随手指滚动设置",
        "电源控制",
        "自动调节显示器亮度",
        "低电量节能模式",
        "显示充放电时间",
        "显示电池百分比",
        "录屏",
        "查询天气",
    };
    const std::vector<std::string> AppCommandNames_{"多媒体-音乐播放", "多媒体-视频播放", "多媒体-播放器控制",
                                                    "应用控制"};
    const std::map<std::string, std::string> AppNamesMap_{{"截图&录屏", "截图"}, {"文档管理器", "文件管理器"}};
    const std::vector<std::string> ScheduleManagementNames_{"日程查看&安排", "闹钟设置"};
    const std::map<std::string, std::string> systemCommandMap_;
    ApiPlatform& apiPlatform_ = ApiPlatform::instance();
};

#endif
