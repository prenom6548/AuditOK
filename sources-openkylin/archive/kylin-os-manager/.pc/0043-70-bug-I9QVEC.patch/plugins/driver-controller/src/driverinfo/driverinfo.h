#ifndef DRIVERINFO_H
#define DRIVERINFO_H

#include <QString>
#include <QStringList>



class DriverInfo
{
public:
    DriverInfo();

    QString driverName{QString()};      // 驱动名称
    QString arch{QString()};            // 架构
    QString version{QString()};         // 驱动版本号
    QString priority{QString()};        // 优先级
    QString section{QString()};         // 软件包所属组别
    QString source{QString()};          // 软件包的源码软件包
    QString origin{QString()};          // 软件包的来源
    QString maintainer{QString()};      // 软件包维护者姓名和电子邮件地址
    QString bugs{QString()};            // 报告软件bug的联系信息
    QString installedSize{QString()};   // 软件包已安装后所占用的磁盘控件大小（kb）
    QString depends{QString()};         // 软件包运行所需依赖
    QString recommends{QString()};      // 建议安装的相关软件包，提供附加功能
    QString filename{QString()};        // 软件包的二进制文件存储在软件仓库中的路径
    QString size{QString()};            // 软件包二进制文件在压缩状态下的大小
    QStringList modaliases{QStringList()};   // N卡驱动甄别关键信息

};

#endif // DRIVERINFO_H
