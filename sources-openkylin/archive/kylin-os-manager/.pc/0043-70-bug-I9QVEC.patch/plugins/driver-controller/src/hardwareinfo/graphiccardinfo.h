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
    QString businfo{QString()};
    QString clock{QString()};
    QString physid{QString()};
    QString version{QString()};
    QString width{QString()};
};

#endif // GRAPHICCARDINFO_H
