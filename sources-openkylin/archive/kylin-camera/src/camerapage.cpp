 /*
将按钮栏添加到显示界面下方
*/
#include "camerapage.h"
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include "unistd.h"
#include <sys/stat.h>
#include <string.h>
#include <QCameraInfo>
#include <QList>
#include <QRect>
#include <QString>
#include <QPainter>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QPalette>
#include "settingpage.h"
#include "mainwindow.h"
//enum { LOG_DEBUG = 0, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_NOTICE, LOG_FATAL };
bool CameraPage::has_device = 0;
char* CameraPage::current_indevice = new char[128];
QList<QCameraInfo>* CameraPage::cameras = new QList<QCameraInfo>;
QList<QAudioDeviceInfo> * CameraPage::audios = new QList<QAudioDeviceInfo>;
bool first = true;
QGSettings * CameraPage::m_GsettingThemeData;
QString archName = QSysInfo::currentCpuArchitecture();  //架构名字
int itmp = 0;

CameraPage::CameraPage(QWidget *parent)
: QStackedWidget(parent)
{
    m_GsettingThemeData = new QGSettings(FITTHEMEWINDOW);

    No_device = new QWidget;
    cameraViewPage = new QWidget;

    hlayout = new QHBoxLayout;
    vlayout = new QVBoxLayout;
    vlayout_out = new QVBoxLayout;
    widgetDisplay = new QWidget;
    buttom = new QStackedWidget;
    buttomWidget = new QWidget;
    recorderWidgetLayout = new QHBoxLayout;
    recorder_time_label = new QLabel;

    dead_time = new QLabel;
    recorder = new QWidget;
    recorderWidget = new QWidget;
    recordLayout = new QHBoxLayout;
    buttomLayout = new QHBoxLayout;
    btnPage = new Button;
    delayWidget = new DelayWdiget;

    layout = new QStackedLayout(cameraViewPage);
    cameraViewPage->setMinimumHeight(506); //lhl

    //添加禁止锁屏信号
    server = new QDBusInterface("org.gnome.SessionManager" , "/org/gnome/SessionManager" ,
                                "org.gnome.SessionManager" , QDBusConnection::sessionBus());

//    QDBusInterface iface("org.freedesktop.Notifications","/org/freedesktop/Notifications",
//                             "org.freedesktop.Notifications",QDBusConnection::sessionBus());

    *cameras = QCameraInfo::availableCameras();
    QString devname;
    this->addWidget(No_device);
    this->addWidget(cameraViewPage);

    if(!cameras->empty()){
        devname = cameras->at(0).deviceName();
        creatCameraPage(devname.toLocal8Bit().data());
        this->setCurrentWidget(cameraViewPage);
    }
    else{
        displayCameraError();
    }

    QPalette a(recorder_time_label->palette());
    a.setColor(QPalette::WindowText,QColor(255,255,255)); //白色字
    recorder_time_label->setPalette(a);

    QPalette pal(recorder->palette());
    pal.setColor(QPalette::Background,QColor(0,0,0,1)); //黑色背景
    recorder->setStyleSheet("{border:2px solid;}");
    recorder->setFixedSize(108,32);
    recorder->setAutoFillBackground(true);
    recorder->setPalette(pal);
    //this->setAttribute(Qt::WA_PaintOnScreen);    //不跟随窗管设置
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    connect(m_GsettingThemeData,&QGSettings::changed,this,&CameraPage::dealSystemGsettingChange);
    connect(btnPage->capture,&QPushButton::clicked,this,&CameraPage::pictureViewPage);
    connect(btnPage->video,&QPushButton::clicked,this,&CameraPage::videoViewPage);
    //设备使用过程中，改变录音设备
    QDBusConnection::sessionBus().connect(QString(),QString( "/"),"org.ukui.media","DbusSignalRecorder",this,SLOT(initAudioDevice()));
    QDBusConnection::sessionBus().connect(QString(),QString("/"),"org.ukui.media","DbusSingleTest",this,SLOT(initAudioDevice()));
}

CameraPage::~CameraPage(){
    delete No_device;
    delete cameraViewPage;
    delete buttom;
    delete delayWidget;
    delete videoDisplay;
}

//未发现摄像头设备界面
void CameraPage::displayCameraError(){

    has_device = 0;
    widget1 = new QWidget;
    widget2 = new QWidget;
    widget3 = new QWidget;
    No_device_icon = new QLabel(No_device);
    No_device_lab = new QLabel(No_device);
    No_device_lab_s = new QLabel(No_device);

    QVBoxLayout *vlayout = new QVBoxLayout;
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    QHBoxLayout *hlayout3 = new QHBoxLayout;

    hlayout1->addStretch(0);
    hlayout1->addWidget(No_device_icon);
    hlayout1->addStretch(0);
    hlayout1->setMargin(0);
    hlayout2->addStretch(0);
    hlayout2->addWidget(No_device_lab);
    hlayout2->addStretch(0);
    hlayout2->setMargin(0);
    hlayout3->addStretch(0);
    hlayout3->addWidget(No_device_lab_s);
    hlayout3->addStretch(0);
    hlayout3->setMargin(0);

    widget1->setLayout(hlayout1);
    widget2->setLayout(hlayout2);
    widget3->setLayout(hlayout3);
    vlayout->addStretch(10);
    vlayout->addWidget(widget1);
    vlayout->addWidget(widget2);
    vlayout->addWidget(widget3);
    vlayout->addStretch(10);
#ifdef __V10__
    No_device_icon->setPixmap(QPixmap(":/image/camera-disconnected.png"));
#else
    //No_device_icon->setPixmap(QIcon::fromTheme("ukui-camera-disconnected-symbolic").pixmap(QSize(32, 32)));
    No_device_icon->setPixmap(QPixmap(":/image/camera-none.svg"));
#endif
    No_device_icon->setProperty("useIconHighlightEffect", 0x8);
    No_device_lab->setText(tr("No device were found"));
    No_device_lab_s->setText(tr("Please connect the camera first"));

    QString nowThemeStyle = CameraPage::m_GsettingThemeData->get("styleName").toString();
    No_device->setGeometry(0,0,300,100);
    QPalette pal(No_device->palette());
    if (nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black"){
        //设置背景色黑色
        pal.setColor(QPalette::Background,Qt::black);
    }
    else{
        //设置背景色灰色
        pal.setColor(QPalette::Background,Qt::lightGray);
    }
    No_device->setAutoFillBackground(true);
    No_device->setPalette(pal);
    //No_device->show();

    QString labStyle = \
        "line-height: 32px;"
        "opacity: 1;";
//    No_device_lab->setStyleSheet(labStyle);  //不跟随主题大小变化
//    No_device_lab_s->setStyleSheet(labStyle);
    this->setCurrentWidget(No_device);
    No_device->setLayout(vlayout);

    if(server->isValid()){
        server->call("Uninhibit",arg_0);
    }
    No_device->show();
    if(videoDisplay !=nullptr)
    {
        delete videoDisplay;
        videoDisplay = nullptr;
    }
//    videoDisplay = nullptr;
}

//界面布局
void CameraPage::layout_operation(){

    recordLayout->addStretch(0);  //拉伸
    recordLayout->addWidget(recorder_time_label);
    recordLayout->addStretch(0);
    recordLayout->setMargin(0);  //边距
    recorder->setLayout(recordLayout);
    recorder_time_label->raise();
    recorderWidgetLayout->addStretch(0);
    recorderWidgetLayout->addWidget(recorder);
    recorderWidgetLayout->addStretch(0);
    recorderWidget->setLayout(recorderWidgetLayout);

    QFont font;
    font.setPointSize(80);
    dead_time->setFont(font);
    dead_time->resize(100,100);
    btnPage->setFixedHeight(92);   //btnPage拍照录像的按钮栏

    buttom->addWidget(delayWidget);  //buttom 录像时间和延迟拍照模块
    buttom->addWidget(recorderWidget);
    buttom->setCurrentWidget(recorder_time_label);

    buttomLayout->addStretch(0);
    buttomLayout->addWidget(buttom);
    buttomLayout->addStretch(0);
    buttomWidget->setLayout(buttomLayout);

    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::white);
    dead_time->setPalette(pal);
    dead_time->raise();
    dead_time->hide();
    buttom->hide();

    vlayout->addStretch(100);
    vlayout->addWidget(dead_time);
    vlayout->addStretch(100);
    vlayout->addWidget(buttomWidget);

    hlayout->addStretch(100);
    hlayout->addLayout(vlayout);
    hlayout->addStretch(100);

    vlayout_out->addLayout(hlayout);
    vlayout_out->addWidget(btnPage);
    vlayout_out->setMargin(0);

//    videoDisplay->setLayout(vlayout_out);
    widgetDisplay->setLayout(vlayout_out);
    layout->addWidget(widgetDisplay);
    layout->addWidget(videoDisplay);
    cameraViewPage->setLayout(layout);
    layout->setStackingMode(QStackedLayout::StackAll);
    has_device = true;
}

void CameraPage::setNoDevice(){
    this->setCurrentWidget(No_device);
}

void CameraPage::setCameraViewPage(){
    this->setCurrentWidget(cameraViewPage);
}

//创建摄像头设备界面
void CameraPage::creatCameraPage(const char *in_devname){
    CurrentDeviceInfo::available_size.clear();
    CurrentDeviceInfo::current_device_name.clear();
    CurrentDeviceInfo::current_device_name.push_back(in_devname);
    camera = new KylinCamera(cameraViewPage);
    KylinCamera::camera_log_init(LOG_WARN);
    getCameraInformation(in_devname);
    videoDisplay = camera->create(this, &m_camInfo);
    //设备被占用返回值NULL
#if 1
    if(videoDisplay == NULL)
    {
        QMessageBox::warning(this,tr("waring"),tr("The camera is occupied, please check the use of the device!"));
        delete camera;
        exit(0);
    }
#endif
    camera->camera_display_mirror(1);   //镜像设置后为正常显示
    videoDisplay->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    strcpy(current_indevice,in_devname);
//QTimer method
    qDebug() << "QTimer Method!";
    heartBeatsNum = 0;
    secTimer = new QTimer(this);
    connect(secTimer, &QTimer::timeout, [=](){
        timeoutEvent(m_camInfo);
    });
    secTimer->start(1000);

    //对显示界面进行布局
    if(first){
        first = false;
        this->setCurrentWidget(cameraViewPage);
        layout->setMargin(0);
        cameraViewPage->setLayout(layout);
        layout_operation();
    }else{
        layout->addWidget(videoDisplay);
        this->setCurrentWidget(cameraViewPage);
        has_device = true;
    }
    //增加禁止锁屏信号
    if(!server->isValid())
    {
        qDebug() << "warring : canot call dbus";
        return ;
    }
    QDBusMessage a = server->call("Inhibit","kylin-camera",(quint32)0,"open-camera",(quint32)8);
    arg_0 = a.arguments().takeFirst().toUInt();
    qDebug()<< "arg_0"<<arg_0;
}

//修改摄像头分辨率
void CameraPage::change_resolution(QPair<uint, uint> resolution){
    KylinCameraInfo camerainfo;
    memset(&camerainfo,0X00,sizeof(KylinCameraInfo));
    memcpy(camerainfo.devname,current_indevice,strlen(current_indevice)+1);

    V4l2DeviceInfo device_info_fps;
    memset(&device_info_fps,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info_fps.dev_path, current_indevice, strlen(current_indevice)+1);
    KylinCamera::enum_device(&device_info_fps);

    qDebug() <<  CurrentDeviceInfo::available_format <<  V4L2_PIX_FMT_MJPEG << V4L2_PIX_FMT_YUYV;
    camerainfo.format = CurrentDeviceInfo::available_format;
    if(archName.contains("mips")){
        camerainfo.format = V4L2_PIX_FMT_YUYV;
    }
    camerainfo.width = resolution.first;
    camerainfo.height = resolution.second;
    QString scale_num = QString::number(resolution.first) + "x" + QString::number(resolution.second);
    qDebug() << "***scale_num" << scale_num;


    TypePixFormats *pixfmt = &device_info_fps.fmt_supported[itmp];
    for(int j = 0; j < 10; j++) {
      TypeFramesizes *fsize = &pixfmt->frm_sizes[j];
      if(fsize->frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {     //遍历分辨率
          pixfmt->frm_sizes_len++;
          qDebug() << "***{ discrete: width = "<< fsize->frm_size.discrete.width<< ", height = " << fsize->frm_size.discrete.height << "}";
          QString string_num = QString::number(fsize->frm_size.discrete.width) + "x" + QString::number(fsize->frm_size.discrete.height);
          qDebug() << "***string_num" << string_num;
          if(string_num == scale_num){                      //获得与前台选择相同的分辨率
                struct v4l2_frmivalenum *fivals = &fsize->frm_ivals[0];
                if (fivals->type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                  camerainfo.width = fsize->frm_size.discrete.width;
                  camerainfo.height = fsize->frm_size.discrete.height;
                  camerainfo.fps = fivals->discrete.denominator;
                  qDebug() << "***---"<< camerainfo.width << camerainfo.height << camerainfo.fps ;
                  m_camInfo = camerainfo;
                }
          }
       }
    }
    resetCamera(camerainfo);
}

//改变摄像头设备
void CameraPage::change_device(const char *in_device){
    KylinCameraInfo camerainfo;
    memset(&camerainfo,0X00,sizeof(KylinCameraInfo));
    memcpy(camerainfo.devname,in_device,strlen(in_device)+1);

    V4l2DeviceInfo device_info;
    memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info.dev_path, in_device, strlen(in_device)+1);
    KylinCamera::enum_device(&device_info);
    CurrentDeviceInfo::available_size.clear();
    //遍历所有分辨率
    for(TypePixFormats fmt : device_info.fmt_supported){
        if(fmt.frm_sizes_len){
            for(auto a: fmt.frm_sizes){
                if(a.frm_size.pixel_format == V4L2_PIX_FMT_MJPEG){
                    CurrentDeviceInfo::available_format = a.frm_size.pixel_format;
                 }
                QPair<uint,uint> discrete;
                discrete.first = a.frm_size.discrete.width;
                discrete.second = a.frm_size.discrete.height;
                CurrentDeviceInfo::available_size.push_back(discrete);
             }
             break;
        }
    }
    CurrentDeviceInfo::sort_resolution();
    int i,j,k,tmpi;
    for(i=0;i<10;i++){
      TypePixFormats *pixfmt = &device_info.fmt_supported[i];
      unsigned int format = pixfmt->fmt.pixelformat;
      char formatArray[] = { (char)(format&0xff), (char)((format>>8)&0xff),(char)((format>>16)&0xff), (char)((format>>24)&0xff), 0 };
      qDebug() << "***pixel format: " << formatArray;
      if( strcmp(formatArray,"MJPG") == 0 ) {
          camerainfo.format = V4L2_PIX_FMT_MJPEG;
          tmpi = i;
          break;
      }
      if( strcmp(formatArray,"YUYV") == 0 ) {
          camerainfo.format = V4L2_PIX_FMT_YUYV;
          tmpi = i;
      }
    }
    //lhl debug 龙芯默认yuyv
    qDebug() << "archName: " << archName;
    if (archName.contains("mips")){
        camerainfo.format = V4L2_PIX_FMT_YUYV;
    }

    unsigned int format_current = camerainfo.format;
    char formatArrayCurrent[] = { (char)(format_current&0xff), (char)((format_current>>8)&0xff),(char)((format_current>>16)&0xff), (char)((format_current>>24)&0xff), 0 };
    qDebug() << "***pixel format current: " << formatArrayCurrent; //最后选择的编码格式
    CurrentDeviceInfo::available_format = camerainfo.format;

    TypePixFormats *pixfmt = &device_info.fmt_supported[tmpi];
    for(j = 0; j < 10; j++) {
      TypeFramesizes *fsize = &pixfmt->frm_sizes[j];
      if(fsize->frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {     //遍历分辨率 获取第一组
          pixfmt->frm_sizes_len++;
          qDebug() << "***{ discrete: width = "<< fsize->frm_size.discrete.width<< ", height = " << fsize->frm_size.discrete.height << "}";
          QString string_num = QString::number(fsize->frm_size.discrete.width) + "x" + QString::number(fsize->frm_size.discrete.height);
          qDebug() << "***string_num" << string_num;
          for(k = 0; k < 10; k++) {                             //遍历帧率,第一组获取
              struct v4l2_frmivalenum *fivals = &fsize->frm_ivals[k];
              if (fivals->type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                  camerainfo.width = fsize->frm_size.discrete.width;
                  camerainfo.height = fsize->frm_size.discrete.height;
                  camerainfo.fps = fivals->discrete.denominator;
                  qDebug() << "***---"<< camerainfo.width << camerainfo.height << camerainfo.fps ;

              }
              m_camInfo = camerainfo;
              break;
          }
          break;
       }
    }

    if(archName.contains("mips")){
        CurrentDeviceInfo::available_format = V4L2_PIX_FMT_YUYV;
    }
    camera->camera_set_param(&camerainfo);
    strcpy(current_indevice,in_device);
}

void CameraPage::paintEvent(QPaintEvent *event){      //CameraPage绘图显示
    if(btnPage->grid->isChecked()){
        QPainter painter(widgetDisplay);
        painter.setPen(Qt::white);
        drawGrid(&painter);
        painter.begin(widgetDisplay);
        qDebug() << "painter:" << painter.isActive();
    }

    QPalette a(recorder_time_label->palette());
    a.setColor(QPalette::WindowText,QColor(255,255,255));
    recorder_time_label->setPalette(a);

    QPalette pal(recorder->palette());
    recorder->setStyleSheet(".QWidget{border-radius:10px;background-color: rgba(0, 0, 0, 0.5);}");
    recorder->setFixedSize(108,32);
    recorder->setAutoFillBackground(true);
    recorder->setPalette(pal);
    QWidget::paintEvent(event);
}

void CameraPage::drawGrid(QPainter *painter)
{
    int Margin = 0;//边缘
    QRect rect;
    qDebug() << "rect" << width() << height();
    //取得绘图区域，大小要减去旁白
//  rect=QRect(Margin+300,Margin+300,width()-Margin-700,height()-Margin-500);
    rect = QRect(Margin, Margin, width() - 2 * Margin, height() - 2 * Margin);
    for(int i = 0; i < 4; i++)
    {
        int x = rect.left() + (i * (rect.width() - 1) / 3);
        painter->drawLine(x, rect.top(), x, rect.bottom());
    }

    for(int j = 0; j < 4; j++)
    {
        int y = rect.bottom() - (j * (rect.height() - 1) / 3);
        painter->drawLine(rect.left(), y, rect.right(), y);
    }
}

void CameraPage::camera_set_param(KylinCameraInfo *device_info){

    camera->camera_set_param(device_info);
}

//处理摄像头热插拔事件
void CameraPage::timeEvent()
{
    static int i = 0;
    *cameras = QCameraInfo::availableCameras();
    qDebug() << "call time : " << ++i;

    //拔出当前使用的摄像头
    if(has_device && current_indevice && access(current_indevice,F_OK) != 0){
        emit device_pull_out();

        if (Button::video_start) {
            emit btnPage->cheese_stop->clicked();
        }
        //还存在其他摄像头，切换到其他摄像头
        if(!cameras->empty()){
            change_device(cameras->at(0).deviceName().toLocal8Bit().data());
            return;
        }
        //拔出的为最后一个设备切换到未发现设备界面
        else{
            memset(current_indevice,0X00,strlen(current_indevice));
            CurrentDeviceInfo::available_size.clear();
//            if(camera !=nullptr){      //导致第二次录像后把USB闪退，屏蔽
//                //delete camera;
//                camera->deleteLater();
//                camera = nullptr;
//            }
            displayCameraError();
            usleep(300000);
            qDebug() << "push in";
            has_device = 0;
            return;
        }
    }
    //插入一个摄像头并启动摄像头。
    if(!has_device && !cameras->empty()){
        QString str = cameras->at(0).deviceName();
        qDebug() << str.toLocal8Bit().data();
//        current_indevice = NULL;
        memset(current_indevice,0X00,strlen(current_indevice));
        strcpy(current_indevice, str.toLocal8Bit().data());
        struct stat s_stat;

        //当前不存在摄像头设备，读取新插入的设备
        if(access(current_indevice,F_OK) == 0 && has_device == 0)
        {
            int i = 0;
            while(1)
            {
                sleep(1);
                memset(&s_stat,0x00,sizeof(struct stat));
                int ret = stat(current_indevice,&s_stat);  //获取文件信息
                if(ret == -1)
                {
                    return;
                }
                if((s_stat.st_mode == (unsigned long)8624))
                {
                    qDebug() << current_indevice;
//                    this->setCurrentIndex(1);
                    creatCameraPage(current_indevice);
                    return ;
                }
            }
        }
        else{

        }
    }
    else{

    }
}

void CameraPage::timeoutEvent(KylinCameraInfo cameraInfo)
{
//    bool isAlive = camera->camera_get_state();
    bool isAlive = true;
    if(!isAlive)
    {
        heartBeatsNum++;
    }
    else
    {
        heartBeatsNum = 0;
    }


    if(heartBeatsNum > 5)
    {
//        qDebug() << "camera is Dead!";
        secTimer->stop();
//        disconnect(secTimer, SIGNAL(timeout()), 0,0);
        resetCamera(cameraInfo);
    }
    else
    {
//        qDebug() << "camera is Alive!,lost time :" << heartBeatsNum;
    }
}

void CameraPage::resetCamera(KylinCameraInfo cameraInfo)
{
//    qDebug() << "camera Reset!";
    if(camera != nullptr)
    {
        delete camera;
        camera = nullptr;
    }
    int h = cameraViewPage->height();
    camera = new KylinCamera(cameraViewPage);
    KylinCamera::camera_log_init(LOG_WARN);
    //默认录音设备
    //QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    V4l2DeviceInfo device_info;
    memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info.dev_path, current_indevice, strlen(current_indevice)+1);
    KylinCamera::enum_device(&device_info);
    camera->fourcc(KylinCamera::enum_device(&device_info));
    CurrentDeviceInfo::sort_resolution();
    videoDisplay = camera->create(this, &cameraInfo);
    if(Button::video_mode == false){
        videoDisplay->setFixedHeight(h - 92);
    }
    else{
        videoDisplay->setFixedHeight(h);
    }
    videoDisplay->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    if(MainWindow::mirrorFlag == 1){       //镜像翻转显示
        camera->camera_display_mirror(0);
    }
    else{
        camera->camera_display_mirror(1);
    }
    qDebug() << "cache : " << videoDisplay->children();
    videoDisplay->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    layout->addWidget(videoDisplay);
    heartBeatsNum = 0;
    secTimer->start(1000);
}


void CameraPage::stopCamera()
{
    if(camera != nullptr)
    {
        //camera->deleteLater();
        delete camera;
        qDebug() << "camera is Running";
        camera = nullptr;
    }
}

KylinCameraInfo CameraPage::getCameraInfo()
{
    return m_camInfo;
}

void CameraPage::startExistCamera(KylinCameraInfo cameraInfo)
{
    if(camera != nullptr)
    {
        qDebug() << "startExistCamera camera is Running";
        return;
    }
    int h = cameraViewPage->height();
    camera = new KylinCamera(cameraViewPage);
    KylinCamera::camera_log_init(LOG_WARN);
    //QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    V4l2DeviceInfo device_info;
    memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info.dev_path, current_indevice, strlen(current_indevice)+1);
    sleep(1);  //部分机器CPU性能问题，重新打开造成段错误，增加延时
    KylinCamera::enum_device(&device_info);
    camera->fourcc(KylinCamera::enum_device(&device_info));
    if(has_device == 0)
    {
        memset(current_indevice,0X00,strlen(current_indevice));
    }
    else
    {
        CurrentDeviceInfo::sort_resolution();
        videoDisplay = camera->create(this, &cameraInfo);
        if(MainWindow::mirrorFlag == 1){       //镜像翻转显示
            camera->camera_display_mirror(0);
        }
        else{
            camera->camera_display_mirror(1);
        }
        if(Button::video_mode == false){
            videoDisplay->setFixedHeight(h - 92);
        }
        else{
            videoDisplay->setFixedHeight(h);
        }

        qDebug() << "cache : " << videoDisplay->children();
        videoDisplay->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
//        camera->camera_audio_disable();
//        camera->camera_audio_init(info);
        layout->addWidget(videoDisplay);
    }
}

//冗余代码 需优化
void CameraPage::dealSystemGsettingChange(const QString key){
    if(key == "styleName"){
        refreshThemeBySystemConf();
    }
}

void CameraPage::refreshThemeBySystemConf(){
    QString themeNow = CameraPage::m_GsettingThemeData->get("styleName").toString();
    this->setGeometry(0,0,300,100);
    QPalette pal(this->palette());
    //深色模式下
    if (themeNow == "ukui-dark" || themeNow == "ukui-black"){
        //设置背景色黑色
        pal.setColor(QPalette::Background,Qt::black);
    }
    else{
        //设置背景色灰色
        pal.setColor(QPalette::Background,Qt::lightGray);
    }
    No_device->setAutoFillBackground(true);
    No_device->setPalette(pal);
    No_device->show();
}

//拍照模式下页面大小调整
void CameraPage::pictureViewPage(){
    int h = cameraViewPage->height();
    qDebug() << "int h : " << h;
    if(Button::video_start){  //开始录像
        videoDisplay->setFixedHeight(h);
    }
    else{
        videoDisplay->setFixedHeight(h - 92);
    }
    videoDisplay->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

//录像模式下页面大小调整
void CameraPage::videoViewPage(){
//  resetCamera(m_camInfo);
    int h = cameraViewPage->height();
    qDebug() << "int h:: " << h;
    if(Button::delay_mode){
        videoDisplay->setFixedHeight(h - 92);
    }
    else{
        videoDisplay->setFixedHeight(h);
    }
    videoDisplay->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

//最大化页面显示
void CameraPage::maxWindowView(){
    if(Button::video_mode == false){
        videoDisplay->setFixedHeight(996 - 92);
     }
    else{
        videoDisplay->setMinimumSize(800,414);
    }
    videoDisplay->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

void CameraPage::initAudioDevice(){

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    qDebug()<< info.deviceName() <<" "<< QAudioDeviceInfo::defaultOutputDevice().deviceName();
    camera->camera_audio_init(info);
}

//获取摄像头信息
void CameraPage::getCameraInformation(const char *in_devname){

   V4l2DeviceInfo device_info;   
   memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
   memcpy(device_info.dev_path, in_devname, strlen(in_devname)+1);
   int formatMess = KylinCamera::enum_device(&device_info);
   //遍历所有分辨率
   for(TypePixFormats fmt : device_info.fmt_supported){
       if(fmt.frm_sizes_len){
           for(auto a: fmt.frm_sizes){
               if(a.frm_size.pixel_format == V4L2_PIX_FMT_MJPEG){
                   CurrentDeviceInfo::available_format = a.frm_size.pixel_format;
                }
               QPair<uint,uint> discrete;
               discrete.first = a.frm_size.discrete.width;
               discrete.second = a.frm_size.discrete.height;
               CurrentDeviceInfo::available_size.push_back(discrete);
            }
            break;
       }
   }
   CurrentDeviceInfo::sort_resolution();
   camera->fourcc(formatMess);

   int scale_num = MainWindow::setting->get("scale-index").toInt();   //初始化获取分辨率数值修正
   qDebug() << "***scale_num" << scale_num;
   if(scale_num > CurrentDeviceInfo::available_size.size())
       scale_num = 0;
    //从setting页面获得的分辨率
    int width = CurrentDeviceInfo::available_size[scale_num].first;
    int height = CurrentDeviceInfo::available_size[scale_num].second;
    QString string_numSetting = QString::number(width) + "x" + QString::number(height);

    KylinCameraInfo camera_info;
    memset(&camera_info,0X00,sizeof(KylinCameraInfo));
    memcpy(camera_info.devname, in_devname, strlen(in_devname)+1);

    int i,j,k;
    for(i=0;i<10;i++){
      TypePixFormats *pixfmt = &device_info.fmt_supported[i];
      unsigned int format = pixfmt->fmt.pixelformat;
      char formatArray[] = { (char)(format&0xff), (char)((format>>8)&0xff),(char)((format>>16)&0xff), (char)((format>>24)&0xff), 0 };
      qDebug() << "***pixel format: " << formatArray;
      if( strcmp(formatArray,"MJPG") == 0 ) {
          camera_info.format = V4L2_PIX_FMT_MJPEG;
          itmp = i;
          break;
      }
      if( strcmp(formatArray,"YUYV") == 0 ) {
          camera_info.format = V4L2_PIX_FMT_YUYV;
          itmp = i;
      }
    }
    //lhl debug 龙芯默认yuyv
    qDebug() << "archName: " << archName;
    if (archName.contains("mips")){
        camera_info.format = V4L2_PIX_FMT_YUYV;
    }
    unsigned int format_current = camera_info.format;
    char formatArrayCurrent[] = { (char)(format_current&0xff), (char)((format_current>>8)&0xff),(char)((format_current>>16)&0xff), (char)((format_current>>24)&0xff), 0 };
    qDebug() << "***pixel format current: " << formatArrayCurrent; //最后选择的编码格式

    TypePixFormats *pixfmt = &device_info.fmt_supported[itmp];
    for(j = 0; j < 10; j++) {
      TypeFramesizes *fsize = &pixfmt->frm_sizes[j];
      if(fsize->frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {     //遍历分辨率
          pixfmt->frm_sizes_len++;
          qDebug() << "***{ discrete: width = "<< fsize->frm_size.discrete.width<< ", height = " << fsize->frm_size.discrete.height << "}";
          QString string_num = QString::number(fsize->frm_size.discrete.width) + "x" + QString::number(fsize->frm_size.discrete.height);
          qDebug() << "***string_num" << string_num;
          if(string_num == string_numSetting){                      //获得与前台选择相同的分辨率
              for(k = 0; k < 10; k++) {                             //遍历帧率,第一组获取
                  struct v4l2_frmivalenum *fivals = &fsize->frm_ivals[k];
                  if (fivals->type == V4L2_FRMIVAL_TYPE_DISCRETE) {                  
                      camera_info.width = fsize->frm_size.discrete.width;
                      camera_info.height = fsize->frm_size.discrete.height;
                      camera_info.fps = fivals->discrete.denominator;
                      qDebug() << "***---"<< camera_info.width << camera_info.height << camera_info.fps ;
                      m_camInfo = camera_info;
                  }
                  break;
              }
          }

       }
    }

}
