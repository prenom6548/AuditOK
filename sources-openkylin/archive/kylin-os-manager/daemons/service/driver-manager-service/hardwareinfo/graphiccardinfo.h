#ifndef GRAPHICCARDINFO_H
#define GRAPHICCARDINFO_H

#include "hardwareinfo.h"

class GraphicCardInfo : public HardWareInfo
{
public:
    GraphicCardInfo();

    QString modalias{QString()}; // 设备pci:

    QString subsystem{QString()};
    QString memorysize{QString()};
    QString cardtype{QString()};
    QString gddrsize{QString()};
    QString businfo{QString()};
    QString clock{QString()};
    QString physid{QString()};
    QString version{QString()};
    QString width{QString()};
    QString egl_version{QString()};
    QString egl_apis{QString()};
    QString gl_version{QString()};
    QString glsl_version{QString()};
};

#endif // GRAPHICCARDINFO_H
