#ifndef STATETARGET_H
#define STATETARGET_H


#include <memory>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_set>

enum class StateTarget : uint64_t {
    None = 0ULL, // 无状态
    All = ~None, // 全部状态
    Init = 1ULL << 0, // 初始化
    Self = 1ULL << 1, // 自身变化

    PowerMode = 1ULL << 10, // 电源模式
    PowerType = 1ULL << 11, // 电源类型
    DeviceMode = 1ULL << 12, // 设备模式
    ResourceLimit = 1ULL << 13, // 资源限制
    SceneManager = 1ULL << 14, // 场景识别
    ResourceScheduling = 1ULL << 15, // 资源调度
    AppWhiteList = 1ULL << 16, // 白名单
    TopAppList = 1ULL << 17, // TopApp名单
    StatusMode = 1ULL << 18, // 状态模式
    ScreenMode = 1ULL << 19, // 屏幕模式
    DPMSMode = 1ULL << 20, // DPMS模式
    ProcessInfo = 1ULL << 21, // appinfo产生了变化
    PrepareForSleep = 1ULL << 22, // 准备睡眠
    PrepareForShutdown = 1ULL << 23, // 准备关机
    AppFocusSpell = 1ULL << 24, // app焦点一段时间
    AppWinChangedSpell = 1ULL << 25, // app窗口变化一段时间
    BenchmarkStateChanged = 1ULL << 26, // 跑分检测状态变化
    PowerLevelChanged = 1ULL << 27, // 电量等级变化
    SoftFreezeModeEnabled = 1ULL << 28, // 软冻模式
    ResourceThreshold = 1ULL << 29, // 资源阈值
    InputMode = 1ULL << 30 // 输入状态模式
};

inline StateTarget operator|(StateTarget a, StateTarget b) {
    return static_cast<StateTarget>(
        static_cast<uint64_t>(a) | static_cast<uint64_t>(b)
    );
}

inline StateTarget operator&(StateTarget a, StateTarget b) {
    return static_cast<StateTarget>(
        static_cast<uint64_t>(a) & static_cast<uint64_t>(b)
    );
}

inline StateTarget& operator|=(StateTarget& a, StateTarget b) {
    return a = a | b;
}

inline StateTarget& operator&=(StateTarget& a, StateTarget b) {
    return a = a & b;
}

inline StateTarget operator~(StateTarget a)
{
    return static_cast<StateTarget>(
        ~static_cast<uint64_t>(a));
}

#endif // !STATEEVENT_H