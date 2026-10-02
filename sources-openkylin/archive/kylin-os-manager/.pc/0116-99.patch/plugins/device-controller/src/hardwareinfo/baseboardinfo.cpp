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
