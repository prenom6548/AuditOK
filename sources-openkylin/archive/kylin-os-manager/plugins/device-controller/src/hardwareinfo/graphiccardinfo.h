#ifndef GRAPHICCARDINFO_H
#define GRAPHICCARDINFO_H

#include "hardwareinfo.h"
#include "deviceinfo.h"

class GraphicCardInfo : public HardWareInfo, public DeviceInfo {
public:
    GraphicCardInfo();

    QString modalias{QString()}; // 设备pci:

    QString subsystem{QString()};
    QString memorysize{QString()};
    QString gddrsize{QString()};
    QString businfo{QString()};
    QString clock{QString()};
    QString physid{QString()};
    QString version{QString()};
    QString width{QString()};
    QString egl_version{QString()};    // EGL 版本号
    QString egl_apis{QString()};       // EGL 支持的图形渲染API列表
    QString gl_version{QString()};     // OpenGL Version
    QString glsl_version{QString()};   // GLSL Version

    QString getValue(QString value);

    void setValue(QString key, QString value);

    QString getGpuBusWidth();

    QString getGpuType();

    QString getGDDRSize();
};

#endif // GRAPHICCARDINFO_H
