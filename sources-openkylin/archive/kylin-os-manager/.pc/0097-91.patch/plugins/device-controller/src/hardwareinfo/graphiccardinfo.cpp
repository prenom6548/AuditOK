#include "graphiccardinfo.h"

GraphicCardInfo::GraphicCardInfo()
{

}

QString GraphicCardInfo::getValue(QString value) {
    if (value == "vendor") {
        return vendor;
    } else if (value == "model") {
        return model;
    } else if (value == "serial") {
        return serial;
    } else if (value == "name") {
        return name;
    } else if (value == "deviceId") {
        return deviceId;
    } else if (value == "driver_now") {
        return driver_now;
    } else if (value == "driver_version_now") {
        return driver_version_now;
    } else if (value == "modalias") {
        return modalias;
    } else if (value == "subsystem") {
        return subsystem;
    } else if (value == "memorysize") {
        return memorysize;
    } else if (value == "businfo") {
        return businfo;
    } else if (value == "clock") {
        return clock;
    } else if (value == "physid") {
        return physid;
    } else if (value == "version") {
        return version;
    } else if (value == "width") {
        return width;
    } else if(value == "gddrsize"){
        return gddrsize;
    }else if(value == "egl_version"){
        return egl_version;
    }else if(value == "egl_apis"){
        return egl_apis;
    }else if(value == "gl_version"){
        return gl_version;
    }else if(value == "glsl_version"){
        return glsl_version;
    }else{
        return "";
    }
}
