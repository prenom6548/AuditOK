#ifndef GRAPHICCARDFINDER_H
#define GRAPHICCARDFINDER_H

#include "devicefinder.h"
#include "../hardwareinfo/graphiccardinfo.h"

class graphicCardFinder : public deviceFinder
{
public:
    graphicCardFinder();
    ~graphicCardFinder();
    void dowork();
    QList<GraphicCardInfo> getList();

private:
    QList<GraphicCardInfo> m_cardInfoList;
};

#endif // GRAPHICCARDFINDER_H
