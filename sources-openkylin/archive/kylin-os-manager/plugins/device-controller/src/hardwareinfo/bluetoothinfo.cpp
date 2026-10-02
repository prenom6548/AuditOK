#include "bluetoothinfo.h"
// #include <libkydevicesec_bluetooth.h>
#include <QDebug>

BluetoothInfo::BluetoothInfo() {

}

BluetoothInfo::BluetoothInfo(BluetoothInfo *others) {
//    address = others->address;
    alcmtu = others->alcmtu;
    bus = others->bus;
    dev_version = others->dev_version;
    features = others->features;
    link_mode = others->link_mode;
    link_policy = others->link_policy;
    dev_name = others->dev_name;
    packettype = others->packettype;
    scomtu = others->scomtu;
    speed = others->speed;
    modeinfo = others->modeinfo;

}

QString BluetoothInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if (model == "") {
            return "Bluetooth Device";
        } else {
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
        return modeinfo.version;
    } else if (value == "address") {
        return address;
    } else if (value == "bus") {
        return bus;
    } else if (value == "dev_version") {
        return dev_version;
    } else if (value == "link_mode") {
        return link_mode;
    } else if (value == "link_policy") {
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

bool BluetoothInfo::checkStatus() {
    //int status = kdk_device_get_bluetooth_status();
    //if (status == 0){
    //    return false;
    //}
    //if(status == 1){
    //    return true;
    //}
    //if (status == -1){
    //    qWarning() << "获取蓝牙状态失败";
    //}
    return true;
}

void BluetoothInfo::setValue(QString key, QString value) {
    if (key == "vendor") {
        vendor = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "vendor") {
        vendor = value;
    } else if (key == "serial") {
        serial = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "name") {
        name = value;
    } else if (key == "deviceId") {
        deviceId = value;
    } else if (key == "driver_now") {
        driver_now = value;
    } else if (key == "version") {
        modeinfo.version = value;
    } else if (key == "address") {
        address = value;
    } else if (key == "bus") {
        bus = value;
    } else if (key == "dev_version") {
        dev_version = value;
    } else if (key == "link_mode") {
        link_mode = value;
    } else if (key == "link_policy") {
        link_policy = value;
    } else if (key == "packettype") {
        packettype = value;
    } else if (key == "scomtu") {
        scomtu = value;
    } else if (key == "aclmtu") {
        alcmtu = value;
    } else if (key == "features") {
        features = value;
    }
}
