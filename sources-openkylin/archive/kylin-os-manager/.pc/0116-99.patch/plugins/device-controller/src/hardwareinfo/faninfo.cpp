#include "faninfo.h"


FanInfo::FanInfo()
{

}

QString FanInfo::getValue(QString value) {
    if (value == "name") {
        return name;
    } else if (value == "speed") {
        if(speed == "0" || speed == "0 RPM"){
            return "";
        }
        return speed;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}

