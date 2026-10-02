#ifndef MEMORYINFO_H
#define MEMORYINFO_H

#include <QString>

class MemoryInfo {
public:
    MemoryInfo();

    QString locator{QString{}};     // 位置信息
    QString totalwidth{QString{}};  // 总线宽度
    QString size{QString{}};        // 内存大小
    QString serial{QString{}};      // 序列号
    QString verdor{QString{}};      // 厂商
    QString datawidth{QString{}};   // 数据宽度
    QString type{QString{}};       // 类型
    QString speed{QString{}};      // 内存速度
    QString configuredspeed{QString{}};  // 配置的内存速度
    QString banklocator{QString{}};  // 插槽
    QString partnumber{QString{}};  // 零件号
    QString displayname{QString{}};  // 展示名称
};

#endif //MEMORYINFO_H
