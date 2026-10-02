#ifndef GRAPHICCARDINFO_H
#define GRAPHICCARDINFO_H

#include "hardwareinfo.h"
#include "deviceinfo.h"

class GraphicCardInfo : public HardWareInfo, public DeviceInfo
{
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

    QString getValue(QString value);
};

#endif // GRAPHICCARDINFO_H
