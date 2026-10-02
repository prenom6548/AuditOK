#include "currentdeviceinfo.h"
#include <algorithm>
QList<sizeAndfps> CurrentDeviceInfo::available_size_tmp;
QList<sizeAndfps> CurrentDeviceInfo::available_size;
uint CurrentDeviceInfo::available_format;
int CurrentDeviceInfo::formatFlag = 0;
sizeAndfps CurrentDeviceInfo::current_size;
QString CurrentDeviceInfo::current_device_name;
QList<QCameraInfo> CurrentDeviceInfo::current_device_list;
int CurrentDeviceInfo::count;
char* CurrentDeviceInfo::current_indevice = new char[128];
int CurrentDeviceInfo::winHeight = 518;
int CurrentDeviceInfo::winWidth = 900;
CurrentDeviceInfo::CurrentDeviceInfo()
{

}

bool compare(sizeAndfps a,sizeAndfps b){
    if(a.available_size.first > b.available_size.first){
        return true;
    } else if(a.available_size.first < b.available_size.first){
        return false;
    }
    return a.available_size.second > b.available_size.second;
}
void CurrentDeviceInfo::sort_resolution(){
    std::sort(available_size.begin(),available_size.end(),compare);
    //移除重复的分辨率项——当前使用设备的分辨率列表
    for(int i = 0;i< CurrentDeviceInfo::available_size.size()-1;i++){
        QString str;
        sizeAndfps a = CurrentDeviceInfo::available_size[i];
        sizeAndfps b = CurrentDeviceInfo::available_size[i+1];
        if(a.available_size.first == 0) continue;
        if(a.available_size.first == b.available_size.first && a.available_size.second == b.available_size.second){
            qDebug () << "Remove repeated item!";
            CurrentDeviceInfo::available_size.removeAt(i+1);
        }
        qDebug () << str;
    }
}
void CurrentDeviceInfo::sort_resolution_tmp(){
    std::sort(available_size_tmp.begin(),available_size_tmp.end(),compare);
    //移除重复的分辨率项——读取其他可用设备时，临时读取分辨率列表
    for(int i = 0;i< CurrentDeviceInfo::available_size_tmp.size()-1;i++){
        QString str;
        sizeAndfps a = CurrentDeviceInfo::available_size_tmp[i];//sizeAndfps是一个记录分辨率和帧率的结构体
        sizeAndfps b = CurrentDeviceInfo::available_size_tmp[i+1];
        if(a.available_size.first == 0) continue;
        if(a.available_size.first == b.available_size.first && a.available_size.second == b.available_size.second){
            qDebug () << "Remove repeated item!";
            CurrentDeviceInfo::available_size_tmp.removeAt(i+1);
        }
        qDebug () << str;
    }

}
