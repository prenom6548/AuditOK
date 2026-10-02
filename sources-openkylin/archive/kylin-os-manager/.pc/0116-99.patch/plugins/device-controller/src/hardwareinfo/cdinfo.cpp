#include "cdinfo.h"

CDInfo::CDInfo()
{

}

QString CDInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        return model;
    } else if (value == "businfo") {
        return businfo;
    } else if (value == "name") {
        return name;
    } else if (value == "version") {
        return version;
    } else if (value == "driver") {
        return driver;
    } else if (value == "devicenum") {
        return devicenum;
    } else if (value == "speed") {
        return speed;
    } else if (value == "serialnum") {
        return serialnum;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

