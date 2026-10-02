#ifndef MEMINFO_H
#define MEMINFO_H

#include <QtCore/QtGlobal>

namespace KServer {
struct SysMemsInfo
{
    qint64 buffers = 0;
    qint64 cached = 0;
    qint64 memAvailable = 0;
    qint64 memFree = 0;
    qint64 memTotal = 0;
    qint64 swapFree = 0;
    qint64 swapTotal = 0;
};

SysMemsInfo getComputerMemInfo();
}

#endif // MEMINFO_H
