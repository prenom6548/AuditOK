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

void CameraInfo::setValue(QString key, QString value) {
    if (key == "vendor") {
        vendor = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "name") {
        name = value;
    } else if (key == "interface") {
        interface = value;
    } else if (key == "driver") {
        driver = value;
    } else if (key == "version") {
        version = value;
    } else if (key == "businfo") {
        businfo = value;
    }else if (key == "speed") {
        speed = value;
    }
}
