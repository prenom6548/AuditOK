#include "keyboardinfo.h"

KeyboardInfo::KeyboardInfo() {

}

QString KeyboardInfo::getValue(QString value) {
    if (value == "name") {
        return name;
    } else if (value == "devtype") {
        return devtype;
    } else if (value == "vendor") {
        return vendor;
    } else if (value == "interface") {
        return interface;
    } else if (value == "model") {
        return model;
    } else if (value == "driver") {
        return driver;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

void KeyboardInfo::setValue(QString key, QString value) {
    if (key == "name") {
        name = value;
    } else if (key == "devtype") {
        devtype = value;
    } else if (key == "vendor") {
        vendor = value;
    } else if (key == "interface") {
        interface = value;
    } else if (key == "model") {
        model = value;
    } else if (key == "driver") {
        driver = value;
    }
}

