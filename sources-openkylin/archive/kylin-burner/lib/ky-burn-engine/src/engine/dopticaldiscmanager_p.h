#ifndef OPTICALDISCMANAGER_P_H
#define OPTICALDISCMANAGER_P_H

#include "dburn_global.h"

#include <QPair>

DFM_BURN_BEGIN_NS

class DOpticalDiscManagerPrivate
{
public:
    QString errorMsg;
    QString curDev;
    QPair<QString, QString> files;   // first: local disk path, second: optical disk path
};

DFM_BURN_END_NS

#endif   // OPTICALDISCMANAGER_P_H
