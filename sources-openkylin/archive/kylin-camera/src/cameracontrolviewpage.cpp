#include "cameracontrolviewpage.h"
/*有设备界面除去显示摄像头的部分，其他的控件部分(包含倒计时，录像计时，延时选择框)*/
CameraControlViewPage::CameraControlViewPage(QWidget *parent) :
    QWidget(parent)
{
    recordModule = new RecordModule(this);
    recordModule->setAccessibleName("kylin-camera_cameracontrolviewpage_RecordModule_recordModule");
    recordModuleLayout = new QHBoxLayout;
    recordModuleLayout->addWidget(recordModule,Qt::AlignHCenter);
    recordModuleContainer = new QWidget(this);
    recordModuleContainer->setAccessibleName("kylin-camera_cameracontrolviewpage_QWidget_recordModuleContainer");
    recordModuleContainer->setLayout(recordModuleLayout);

    nPixCapWidget = new NPixCapWidget;
    nPixCapWidget->setAccessibleName("kylin-camera_cameracontrolviewpage_NPixCapWidget_nPixCapWidget");
    nPixCapWidgetLayout = new QHBoxLayout;
    nPixCapWidgetLayout->addWidget(nPixCapWidget,Qt::AlignHCenter);
    nPixCapWidgetContainer = new QWidget(this);
    nPixCapWidgetContainer->setAccessibleName("kylin-camera_cameracontrolviewpage_QWidget_nPixCapWidgetContainer");
    nPixCapWidgetContainer->setLayout(nPixCapWidgetLayout);

    delayWidget = new DelayWdiget;//延时选择按钮组Widget
    delayWidget->setAccessibleName("kylin-camera_cameracontrolviewpage_DelayWdiget_delayWidget");
    delayWidgetLayout = new QHBoxLayout;
    delayWidgetContainer = new QWidget(this);
    delayWidgetContainer->setAccessibleName("kylin-camera_cameracontrolviewpage_QWidget_delayWidgetContainer");
    delayWidgetLayout->addWidget(delayWidget,Qt::AlignHCenter);
    delayWidgetContainer->setLayout(delayWidgetLayout);

    //有摄像头的显示界面下-》底部的功能悬浮框
    buttom = new QStackedWidget(this);//包含（延时选择按钮组delayWidget）和（录像计时recorder）的堆叠Widget
    buttom->setAccessibleName("kylin-camera_cameracontrolviewpage_QStackedWidget_buttom");
    buttom->addWidget(delayWidgetContainer);  //buttom 录像时间和延迟拍照模块
    buttom->addWidget(recordModuleContainer);
    buttom->addWidget(nPixCapWidgetContainer);
    buttom->setCurrentWidget(delayWidgetContainer);
    buttom->hide();

    //有摄像头的显示界面下-》倒计时显示模块
    dead_time = new QLabel;
    dead_time->setAccessibleName("kylin-camera_QLable_dead_time");
    QFont font;
    font.setPointSize(80);
    dead_time->setFont(font);
    dead_time->resize(100,100);
    QPalette palDeadTime;
    palDeadTime.setColor(QPalette::Text,Qt::white);
    dead_time->setPalette(palDeadTime);
    dead_time->raise();
    dead_time->hide();
    hlayout = new QHBoxLayout;
    hlayout->addStretch(1);
    hlayout->addWidget(dead_time);
    hlayout->addStretch(1);
    h2layout = new QHBoxLayout;
    h2layout->addStretch(1);
    h2layout->addWidget(buttom);
    h2layout->addStretch(1);
    vlayout = new QVBoxLayout(this);
    vlayout->addStretch(1);
    vlayout->addLayout(hlayout);
    vlayout->addStretch(1);
    vlayout->addLayout(h2layout);
    buttom->hide();
    this->setLayout(vlayout);
}

CameraControlViewPage::~CameraControlViewPage(){
    delete nPixCapWidget;
    delete filterWidget;
    delete delayWidget;
    delete buttom;
    delete dead_time;
}
