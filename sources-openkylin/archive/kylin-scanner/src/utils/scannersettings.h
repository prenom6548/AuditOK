#ifndef SCANNERSETTINGS_H
#define SCANNERSETTINGS_H


#include <QString>

class ScannerSettings
{
public:
    explicit ScannerSettings();

    QString DeviceName{QString()};
    QString Pages{QString()};    // 单页 or 多页
    QString Type{QString()};     // 馈纸 or 平面 or 馈纸双面
    QString Color{QString()};    // 黑白 or 彩色
    QString Resolution{QString()};
    QString PageSize{QString()};
    QString Format{QString()};   // 保存格式
    QString SavePath{QString()}; // 保存地址
};

#endif // SCANNERSETTINGS_H
