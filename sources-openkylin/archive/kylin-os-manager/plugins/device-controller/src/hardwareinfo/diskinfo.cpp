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
        if (type.toInt() == 1 || type == "SSD") {
            return "SSD";
        } else if (type.toInt() == 0 || type == "HDD"){
            return "HDD";
        } if (type.toInt() == 2){
            return QApplication::tr("Removable storage devices");
        } if (type.toInt() == 3 || type == "UFS"){
            return "UFS";
        }else{
            return type;
        }
    } else if (value == "is_main") {
        if (is_main == 0) {
            return QApplication::tr("No");
        } else if (is_main == 1){
            return QApplication::tr("Yes");
        }
    } else if (value == "ufs") {
        return ufs;
    } else if (value == "ufsrevision") {
        return ufsrevision;
    }else if (value == "rota") {
        return rota;
    }else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

void DiskInfo::setValue(QString key, QString value) {
    if (key == "model") {
        model = value;
    } else if (key == "vendor") {
        vendor = value;
    } else if (key == "name") {
        name = value;
    } else if (key == "size") {
        size = value;
    } else if (key == "serial") {
        serial = value;
    } else if (key == "frimwareversion") {
        frimwareversion = value;
    } else if (key == "interface") {
        interface = value;
    } else if (key == "type") {
        type = value;
    } else if (key == "is_main") {
        if( value.contains("No") || value.contains("否") ){
            is_main = 0;
        }else{
            is_main = 1;
        }
    }else if (key == "ufs") {
        ufs = value;
    }else if (key == "ufsrevision") {
        ufsrevision = value;
    }else if (key == "rota") {
        rota = value;
    }
}
