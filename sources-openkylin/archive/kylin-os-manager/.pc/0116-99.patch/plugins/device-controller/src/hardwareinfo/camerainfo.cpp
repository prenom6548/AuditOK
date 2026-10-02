#include "camerainfo.h"

CameraInfo::CameraInfo()
{

}

QString CameraInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        return model;
    } else if (value == "name") {
        return name;
    } else if (value == "interface") {
        return interface;
    } else if (value == "driver") {
        return driver;
    } else if (value == "version") {
        return version;
    } else if (value == "businfo") {
        return businfo;
    } else if (value == "speed") {
        return speed;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}
