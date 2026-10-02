#ifndef DRIVERMODEINFO_H
#define DRIVERMODEINFO_H

#include <QString>
#include <QStringList>

class DriverModeInfo
{
public:
    DriverModeInfo();

    QString driverName{QString()};         // 驱动名称
    QString filename{QString()};           // 内核模块ko文件的路径
    QString license{QString()};            // 模块的许可证信息
    QString description{QString()};        // 模块的描述信息
    QString author{QString()};             // 模块的作者信息
    QString version{QString()};            // 驱动版本
    QString srcversion{QString()};         // 模块的源代码版本
    QStringList alias;                     // 设备别名
    QString depends{QString()};            // 此模块依赖的其他模块
    QString softdep{QString()};            // 内核模块的软依赖关系（加载可选）
    QString vermagic{QString()};           // 内核的版本信息
};

#endif // DRIVERMODEINFO_H
