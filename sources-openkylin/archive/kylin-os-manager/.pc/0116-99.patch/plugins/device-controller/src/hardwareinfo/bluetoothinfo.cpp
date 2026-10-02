#include "bluetoothinfo.h"

BluetoothInfo::BluetoothInfo()
{

}

QString BluetoothInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if(model == ""){
            return "Bluetooth Device";
        }else{
            return model;
        }
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "version") {
        return  modeinfo.version;
    } else if (value == "address") {
        return address;
    } else if (value == "bus") {
        return bus;
    } else if (value == "dev_version") {
        return dev_version;
    } else if (value == "link_mode") {
        return link_mode;
    }else if (value == "link_policy") {
        return link_policy;
    } else if (value == "packettype") {
        return packettype;
    } else if (value == "scomtu") {
        return scomtu;
    } else if (value == "aclmtu") {
        return alcmtu;
    } else if (value == "features") {
        return features;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}
