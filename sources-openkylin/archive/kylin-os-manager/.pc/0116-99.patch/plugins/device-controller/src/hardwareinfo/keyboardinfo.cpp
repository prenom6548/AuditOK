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

