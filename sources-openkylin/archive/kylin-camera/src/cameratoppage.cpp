#include "cameratoppage.h"
#include "currentdeviceinfo.h"
#include "globalinstance.h"

CameraTopPage::CameraTopPage(QWidget *parent)
: QStackedWidget(parent)
{
    this->setMinimumSize(900,518);
    noDevicePage = new NoDevicePage(this);
    noDevicePage->setAccessibleName("kylin-camera_cameratoppage_NoDevicePage_noDevicePage");
    hasDevicePage = new HasDevicePage(this);
    hasDevicePage->setAccessibleName("kylin-camera_cameratoppage_HasDevicePage_hasDevicePage");
    devicePull = new DevicePull;//处理外接摄像头插拔的类
    devicePull->setAccessibleName("kylin-camera_cameratoppage_DevicePull_devicePull");
    this->addWidget(noDevicePage);
    this->addWidget(hasDevicePage);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(this,&CameraTopPage::noDevice,hasDevicePage,&HasDevicePage::noDeviceHandle);
    connect(hasDevicePage,&HasDevicePage::swithToHasDevicePage,this,&CameraTopPage::displayCamera);
    connect(hasDevicePage,&HasDevicePage::swithToNoDevicePage,this,&CameraTopPage::displayCameraError);
    connect(devicePull,&DevicePull::creatCameraPageSig,hasDevicePage,&HasDevicePage::creatCameraPage);
    connect(devicePull,&DevicePull::change_deviceSig,hasDevicePage,&HasDevicePage::creatCameraPage);
    connect(GlobalInstance::getInstance(),&GlobalInstance::displayCameraErrorSig,this,&CameraTopPage::displayCameraError);
    connect(devicePull,&DevicePull::device_pull_out,hasDevicePage,&HasDevicePage::exception_handing);
    connect(hasDevicePage,&HasDevicePage::lockSigForDevicePull,devicePull,&DevicePull::systemLock);
    CurrentDeviceInfo::current_device_list.clear();
    CurrentDeviceInfo::current_device_list = QCameraInfo::availableCameras();
    QString devname;
    if(!CurrentDeviceInfo::current_device_list.empty()){
        devname = CurrentDeviceInfo::current_device_list.at(0).deviceName();
        hasDevicePage->creatCameraPage(devname.toLocal8Bit().data());
    }
    else{
        displayCameraError(1);
    }
}
//未发现摄像头设备界面
void CameraTopPage::displayCameraError(int code){
    this->setCurrentWidget(noDevicePage);
    this->show();
    QString tip1;
    QString tip2;
    if(code == 1){
        tip1 = tr("No device were found");
        tip2 = tr("Please connect the camera first and app will continuously retrieve avaliable devices for you");
        noDevicePage->setTips(tip1,tip2);
    }else if(code == 2){
        tip1 = tr("not support current device");
        tip2 = tr("Please check the camera or switch to avaliable device");
        noDevicePage->setTips(tip1,tip2);
    }
}
//有设备界面
void CameraTopPage::displayCamera(){
    this->setCurrentWidget(hasDevicePage);
    this->show();
}
CameraTopPage::~CameraTopPage(){
    delete devicePull;
}
