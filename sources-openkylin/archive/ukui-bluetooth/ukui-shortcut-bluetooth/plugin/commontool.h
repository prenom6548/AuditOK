#ifndef COMMONTOOL_H
#define COMMONTOOL_H

#include <QString>
#include <QVariant>

#include "CSingleton.h"

enum BluetoothType {
    Phone = 0,
    Modem,
    Computer,
    Network,
    Headset,
    Headphones,
    AudioVideo,
    Keyboard,
    Mouse,
    Joypad,
    Tablet,
    Peripheral,
    Camera,
    Printer,
    Imaging,
    Wearable,
    Toy,
    Health,

    //LoadingIcon = 200
    LoadingIcon = 200,

    Uncategorized = 0xffff,
};

class CommonTool
{
public:
    CommonTool();
    ~CommonTool();

    bool isWayland();
    int getSystemCurrentMouseAndTouchPadDevCount();
    int getSystemCurrentKeyBoardDevCount();

    friend class SingleTon<CommonTool>;
};

typedef SingleTon<CommonTool>  COMMONTOOL;

#endif // COMMONTOOL_H
