#include "baseboardinfo.h"

BaseboardInfo::BaseboardInfo() {

}

QString BaseboardInfo::getValue(QString value) {
    if (value == "model") {
        if(model != ""){
            return model;
        }else{
            return "Baseboard";
        }
    } else if (value == "vendor") {
        return vendor;
    } else if (value == "version") {
        return version;
    } else if (value == "chassishandle") {
        return chassishandle;
    } else if (value == "biosvendor") {
        return biosvendor;
    } else if (value == "biosversion") {
        return biosversion;
    } else if (value == "chipset") {
        return chipset;
    } else if (value == "serial_num") {
        return serial_num;
    } else if (value == "publish_date") {
        return publish_date;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

void BaseboardInfo::setValue(QString key, QString value) {
    if (key == "model") {
        model = value;
    } else if (key == "vendor") {
        vendor = value;
    } else if (key == "version") {
        version = value;
    } else if (key == "chassishandle") {
        chassishandle = value;
    } else if (key == "biosvendor") {
        biosvendor = value;
    } else if (key == "biosversion") {
        biosversion = value;
    } else if (key == "chipset") {
        chipset = value;
    } else if (key == "serial_num") {
        serial_num = value;
    } else if (key == "publish_date") {
        publish_date = value;
    }
}
