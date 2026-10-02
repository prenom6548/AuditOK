#include "hwinfostorage.h"

// 获取单例实例
HwInfoSingleton &HwInfoSingleton::instance() {
    static HwInfoSingleton instance;
    return instance;
}

// 构造函数
HwInfoSingleton::HwInfoSingleton() {
}


