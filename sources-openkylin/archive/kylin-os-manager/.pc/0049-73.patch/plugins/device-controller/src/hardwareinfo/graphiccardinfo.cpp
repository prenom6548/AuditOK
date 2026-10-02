#include "graphiccardinfo.h"

GraphicCardInfo::GraphicCardInfo()
{

}

QString GraphicCardInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        return model;
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "driver_version_now") {
        return driver_version_now;
    } else if (value == "modalias") {
        return modalias;
    } else if (value == "subsystem") {
        return subsystem;
    } else if (value == "memorysize") {
        return memorysize;
    } else if (value == "businfo") {
        return businfo;
    } else if (value == "clock") {
        return clock;
    } else if (value == "physid") {
        return physid;
    } else if (value == "version") {
        return version;
    } else if (value == "width") {
        return width;
    } else if(value == "gddrsize"){
        return gddrsize;
    }else{
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}
