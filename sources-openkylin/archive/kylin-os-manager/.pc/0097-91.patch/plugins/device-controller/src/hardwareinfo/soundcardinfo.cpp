#include "soundcardinfo.h"

SoundCardInfo::SoundCardInfo() {

}

QString SoundCardInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if (model == "" && name == "") {
            return "Sound Card";
        } else if (model != "") {
            return model;
        } else if (name != "") {
            return name;
        }
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "others") {
        return others;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "driver_version_now") {
        return driver_version_now;
    } else if (value == "sysPath") {
        return sysPath;
    } else if (value == "devicePath") {
        return devicePath;
    } else if (value == "busNumber") {
        return busNumber;
    } else if (value == "deviceNumber") {
        return deviceNumber;
    } else if (value == "VID") {
        return VID;
    } else if (value == "PID") {
        return PID;
    } else if (value == "upgradeable") {
        return QString::number(upgradeable);
    } else if (value == "isOnline") {
        return QString::number(isOnline);
    } else if (value == "bus") {
        return bus;
    } else if (value == "clock") {
        if (clock == "0 MHz") {
            return "";
        } else {
            return clock;
        }
    } else if (value == "width") {
        if (width == "0 bits") {
            return "";
        } else {
            return width;
        }
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}
