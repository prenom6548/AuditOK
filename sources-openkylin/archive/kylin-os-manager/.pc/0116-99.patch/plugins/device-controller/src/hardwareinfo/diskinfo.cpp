#include "diskinfo.h"
#include <QApplication>

DiskInfo::DiskInfo() {

}

QString DiskInfo::getValue(QString value) {
    if (value == "model") {
        return model;
    } else if (value == "vendor") {
        return vendor;
    } else if (value == "name") {
        return name;
    } else if (value == "size") {
        return size;
    } else if (value == "serial") {
        return serial;
    } else if (value == "frimwareversion") {
        return frimwareversion;
    }else if (value == "interface") {
        return interface;
    } else if (value == "type") {
        if (type.toInt() == 1) {
            return "SSD";
        } else if (type.toInt() == 0){
            return "HDD";
        } if (type.toInt() == 2){
            return QApplication::tr("Removable storage devices");
        } if (type.toInt() == 3){
            return "UFS";
        }
    } else if (value == "is_main") {
        if (is_main == 0) {
            return QApplication::tr("No");
        } else {
            return QApplication::tr("Yes");
        }
    } else if (value == "ufs") {
        return ufs;
    } else if (value == "ufsrevision") {
        return ufsrevision;
    }else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}
