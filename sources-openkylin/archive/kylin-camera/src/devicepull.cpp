#include "devicepull.h"
#include "unistd.h"
#include <sys/stat.h>
#include "currentdeviceinfo.h"
#include <QDBusInterface>
#include "globalinstance.h"

DevicePull::DevicePull(){
    //检测设备发生变化
    watcher = new QFileSystemWatcher;
    watcher->addPath("/dev/");
    devRead = new DeviceRead;
    connect(watcher,&QFileSystemWatcher::directoryChanged,devRead,[=](){devRead->start();});
    //启动计时器，监听设备，
    connect(devRead,&DeviceRead::finish,this,&DevicePull::handle);

    m_dbus = new login1dbusManager;
    connect(m_dbus,&login1dbusManager::locking,this,&DevicePull::setNoReactionFlag);
}
DevicePull::~DevicePull(){
    delete watcher;
    delete devRead;
}
void DevicePull::handle(){
    //1.判断外接摄像头是插还是拔，并根据情况调用不同的处理函数
    countHistory = CurrentDeviceInfo::current_device_list.size();
    countCurrent = QCameraInfo::availableCameras().size();

    if(countHistory < countCurrent){//當前設備列表長度增加
        if(!flag){
            devicePullIn();
        }
    }
    else if(countHistory > countCurrent){//當前設備列表長度減少
        if(!flag){
            devicePullOut();
        }
    }
    else{
        return;
    }
    //2.更新设备列表(currentdeviceinfo中的)
    CurrentDeviceInfo::current_device_list.clear();
    CurrentDeviceInfo::current_device_list = QCameraInfo::availableCameras();
    Q_EMIT GlobalInstance::getInstance()->settingPageSig();
}
void DevicePull::systemLock(bool res){
    /*在锁屏期间，暂停热插拔的监听;解除锁屏后，开启热插拔的监听*/
    flag = res;
}
void DevicePull::setNoReactionFlag(){
    noReactionFlag = true;
}

//1.拔摄像头情况的处理
void DevicePull::devicePullOut(){
    QString tips = tr("Device is pulled out");
    showNotify(tips);
    for(int i = 0; i < countCurrent; i++){
       if(CurrentDeviceInfo::current_indevice == (QCameraInfo::availableCameras())[i].deviceName()){//判斷拔出的攝像頭是當前正在使用的攝像頭
           return;
       }
    }
    devicePullOutInUse();//拔出的攝像頭是正在使用的攝像頭
}
//2.插摄像头情况的处理
void DevicePull::devicePullIn(){
    QString tips = tr("Device is pulled in");
    showNotify(tips);
    if(countHistory <= 0){//歷史列表爲空，處於無設備界面狀態
        while(1){
            sleep(1);
            CurrentDeviceInfo::current_device_list.clear();
            CurrentDeviceInfo::current_device_list = QCameraInfo::availableCameras();
            if(CurrentDeviceInfo::current_device_list.size() > 0){
                QString str = CurrentDeviceInfo::current_device_list.at(0).deviceName();
                qDebug() << str.toLocal8Bit().data();
                memset(CurrentDeviceInfo::current_indevice,0X00,strlen(CurrentDeviceInfo::current_indevice));
                strcpy(CurrentDeviceInfo::current_indevice, str.toLocal8Bit().data());
                Q_EMIT creatCameraPageSig(CurrentDeviceInfo::current_indevice);
                return;
            }
        }
    }
}
//1.1.拔出的是当前正在使用的设备
void DevicePull::devicePullOutInUse(){
    Q_EMIT device_pull_out();
    if(countCurrent > 0){//拔出当前使用设备，历史列表中有其他设备
        CurrentDeviceInfo::current_device_list.clear();
        CurrentDeviceInfo::current_device_list = QCameraInfo::availableCameras();
        QString str = CurrentDeviceInfo::current_device_list.at(0).deviceName();
        memset(CurrentDeviceInfo::current_indevice,0X00,strlen(CurrentDeviceInfo::current_indevice));
        strcpy(CurrentDeviceInfo::current_indevice, str.toLocal8Bit().data());
        Q_EMIT change_deviceSig(CurrentDeviceInfo::current_indevice);
        return;
    }else{//拔出当前使用设备，历史列表中该设备是唯一设备/*
        memset(CurrentDeviceInfo::current_indevice,0X00,strlen(CurrentDeviceInfo::current_indevice));
        Q_EMIT GlobalInstance::getInstance()->displayCameraErrorSig(1);
        usleep(300000);
        qDebug() << "push in";
        return;
    }
}
void DevicePull::showNotify(QString message){
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<(tr("Camera"))
        <<((unsigned int) 0)
        <<QString("kylin-camera")
        <<tr("kylin camera message") //显示的是什么类型的信息
        <<message //显示的具体信息
        <<QStringList()
        <<QVariantMap()
        <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}
