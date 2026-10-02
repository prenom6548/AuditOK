#include "batteryinfo.h"

BatteryInfo::BatteryInfo()
{

}

QString BatteryInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        if (model == ""){
            return "Battery";
        }else{
            return model;
        }
    } else if (value == "serailnum") {
        return serailnum;
    } else if (value == "state") {
        return state;
    } else if (value == "energy") {
        return energy;
    } else if (value == "used_times") {
        return used_times;
    } else if (value == "energy_full") {
        return energy_full;
    } else if (value == "time_to_empty") {
        return time_to_empty;
    } else if (value == "percentage") {
        return percentage;
    } else {
        return ""; // 如果传入的 value 不是以上列出的任何一个，返回空字符串
    }
}
