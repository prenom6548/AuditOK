#include "hasdevicepage.h"
#include <QDBusConnection>
#include <QAudioDeviceInfo>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include "albumread.h"
#include <QPainter>
#include "unistd.h"
#include <sys/stat.h>
#include <QWidget>
#include <QList>
#include <QPropertyAnimation>
#include "globalinstance.h"
#include <gsettings.hpp>

//static int VIEWWINDOWHEIGHT = 418;
static int BUTTONPAGEHEIGHT = 100;


struct cellBase{
    int whiteBalanceState;
    int whiteBalance;
    int brightness;
    int contrast;
    int saturation;
    int hue;
    int sharpness;
    int backlComp;
};

bool HasDevicePage::has_device;//静态全局变量保存摄像头的状态，当前摄像头状态和现有的设备列表
cellBase filterBase;
HasDevicePage::HasDevicePage(QWidget *parent) :
    QWidget(parent)
{
    //辅助的功能类    
    gsettingItem = new DefaultGetSet;//gsetting读取设置的接口类
    gsettingItem->setAccessibleName("kylin-camera_hasdevicepage_DefaultGetSet_gsettingItem");
    dirChange = new DirChange;
    dirChange->setAccessibleName("kylin-camera_ hasdevicepage_DirChange_dirChange");
    //有摄像头的界面显示相关模块
    gridLinePaint = new GridLinePaint(this);//摄像头主界面-》堆叠样式下layout）的Widget3：网格线功能的显示界面
    gridLinePaint->setAccessibleName("kylin-camera_hasdevicepage_GridLinePaint_gridLinePaint");
    gridLinePaint->hide();
    widgetDisplay = new CameraControlViewPage(this);//摄像头主界面-》堆叠样式下（layout）的Widget1：摄像头功能控件显示界面
    widgetDisplay->setAccessibleName("kylin-camera_hasdevicepage_CameraControlViewPage_widgetDisplay");
    videoDisplay = nullptr;//摄像头主界面-》堆叠样式下layout）的Widget2：摄像头视频流的显示界面
    btnPage = new Button(this);//按钮控件组页面
    btnPage->setAccessibleName("kylin-camera_hasdevicepage_Button_btnPage");
    btnPage->setFixedHeight(100);   //btnPage拍照录像的按钮栏
    //界面上功能控制相关的容器
    controlView = new QVBoxLayout(this);
    controlView->addWidget(widgetDisplay);
    controlView->addWidget(btnPage);
    controlView->setMargin(0);
    controlViewWidget = new QWidget(this);
    controlViewWidget->setAccessibleName("kylin-camera_hasdevicepage_QWidget_controlViewWidget");
    controlViewWidget->setLayout(controlView);
    layout = new QStackedLayout(this);//cameraViewPage下的样式
    layout->setStackingMode(QStackedLayout::StackAll);    
    layout->addWidget(controlViewWidget);
    layout->addWidget(gridLinePaint);
    //layout->addWidget(videoDisplay);
    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //拍照音效
    player = new QMediaPlayer;
    player->setMedia(QUrl("qrc:/voice/12200.wav"));
    player->setVolume(80);
    //拍照动画
    initConnect();
    initDbus();
    //缩略图根据上一张拍摄的文件存在与否决定显示与否的预设
    clearAlbumView();
    archName = QSysInfo::currentCpuArchitecture();
    int photoFormatIndex = gsettingItem->getPhotoFormat();
    //格式转换，实现方法需更改，测试
    if (photoFormatIndex == 1){
        photoFormat = ".png";
    }else if (photoFormatIndex == 2){
        photoFormat = ".bmp";
    }else{
        photoFormat = ".jpg";
    }
    //初始化延时时间为0（默认不开启延时）
    deadTimeSec = 0;
    m_videoDeadTimeSec = 0;
}
HasDevicePage::~HasDevicePage(){
    camera->deleteLater();
    if(m_pGraphicsOpacityEffect1){
        delete m_pGraphicsOpacityEffect1;
        m_pGraphicsOpacityEffect1 = nullptr;
    }
    delete player;
    delete gsettingItem;
    delete dirChange;
}
void HasDevicePage::defaultFilterSet(){//【暂未启用】恢复摄像头设备的默认设置
    if(videoDisplay){
        widgetDisplay->filterWidget->original->setChecked(true);
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,V4L2_WHITE_BALANCE_MANUAL);
        camera->camera_set_contrl(WHITE_BALANCE,filterBase.whiteBalance);
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,filterBase.whiteBalanceState);
        camera->camera_set_contrl(BRIGHTNESS,filterBase.brightness);
        camera->camera_set_contrl(CONTRAST,filterBase.contrast);
        camera->camera_set_contrl(SATURATION,filterBase.saturation);
        camera->camera_set_contrl(HUE,filterBase.hue);
        camera->camera_set_contrl(SHARPNESS,filterBase.sharpness);
        camera->camera_set_contrl(BACKL_COMP,filterBase.backlComp);
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,filterBase.whiteBalanceState);
    }
}
void HasDevicePage::clearAlbumView(){//更改缩略图的槽函数
    /*变更需求为如果上一张应用拍得图片存在则显示，否则不显示*/
    QString filePath = gsettingItem->getFileName();
    QFileInfo info(filePath);
    QString fileBaseName = info.baseName();
    if(info.exists()){//上一张拍得视频/图片存在
        //3.1如果该项是图片，则直接加载
        QPixmap *pixmap = new QPixmap;
        QPixmap fitPixmap;
        if(filePath.contains(".jpg")||filePath.contains(".png")||filePath.contains(".bmp")){
            btnPage->pictureVideo->hide();
            if(!pixmap->load(filePath,"jpg")){
                btnPage->picture->clear();
                return;
            }

        }else{//3.2如果该项是视频，根据文件名的联系得到隐藏文件夹中该视频对应的缩略图的路径，间接的加载出来
            QString videoCapture = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
            videoCapture += "/kylin-camera/";
            //缩略图名和视频名是相同的，通过名字的联系，寻找视频的缩略图
            videoCapture = videoCapture + "." + fileBaseName + ".jpg";
            if(!pixmap->load(videoCapture,"jpg")){
                btnPage->picture->clear();
                btnPage->pictureVideo->hide();
                return;
            }
            btnPage->pictureVideo->show();
        }
        //3.3 设置合适的缩略图
        fitPixmap = pixmap->scaled(48,36,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        btnPage->picture->setPixmap(fitPixmap);
    }else{//上一张拍得视频/图片不存在
        btnPage->picture->clear();
        btnPage->pictureVideo->hide();
    }
}
void HasDevicePage::initConnect(){
    //设备使用过程中，改变录音设备
    QDBusConnection::sessionBus().connect(QString(),QString( "/"),"org.ukui.media","DbusSignalRecorder",this,SLOT(initAudioDevice()));
    QDBusConnection::sessionBus().connect(QString(),QString("/"),"org.ukui.media","DbusSingleTest",this,SLOT(initAudioDevice()));
    connect(btnPage->cheese, &QPushButton::clicked, this, &HasDevicePage::clickPhoto);
    connect(btnPage->cheese_dark, &QPushButton::clicked, this, &HasDevicePage::clickPhoto); //lhl debug
    connect(btnPage->cheese_video, &QPushButton::clicked, this, &HasDevicePage::clickStartRecord);
    connect(btnPage->video, &QPushButton::clicked, this, &HasDevicePage::clickVideoMode);
    connect(btnPage->capture, &QPushButton::clicked, this, &HasDevicePage::clickCaptureMode);
    connect(widgetDisplay->delayWidget,&DelayWdiget::delay_emit,this,&HasDevicePage::delay_operation); //开始延时操作
    connect(widgetDisplay->nPixCapWidget,&NPixCapWidget::cap_emit,this,&HasDevicePage::nPixOperation); //开始连拍操作
    connect(dirChange,&DirChange::setPixLabNull,this,&HasDevicePage::clearAlbumView);

    connect(btnPage,&Button::showGirdLine,this,&HasDevicePage::gridOn);
    connect(btnPage,&Button::delay,this,&HasDevicePage::clickDelay);   //显示延迟模块
    connect(btnPage,&Button::burst,this,&HasDevicePage::clickNPix);   //显示连拍模块
    connect(btnPage,&Button::mirror,this,&HasDevicePage::mirror);

    connect(this,&HasDevicePage::clickRecordSig,widgetDisplay->recordModule,&RecordModule::clickRecordSlot);
    connect(widgetDisplay->recordModule,&RecordModule::recordSuspendSig,this,&HasDevicePage::suspendRecord);
    connect(widgetDisplay->recordModule,&RecordModule::recordStopSig,this,&HasDevicePage::clickStopRecord);
    connect(this,&HasDevicePage::recoredTimeChanged,widgetDisplay->recordModule,&RecordModule::updateRecordingTime);
    connect(GlobalInstance::getInstance(),&GlobalInstance::optionStatus, [this](bool canUse) {
        //连拍状态时禁止使用录像模式，待连拍结束或连拍解除可以使用录像模式
        btnPage->video->setEnabled(canUse);
        });
}
void HasDevicePage::gridOn(bool flag){
    if(flag){//显示网格线
        gridLinePaint->show();
        gsettingItem->addGridLineSDKPoint();
    }else{//隐藏网格线
        gridLinePaint->hide();
    }
}
//显示延时和录像时间模块
void HasDevicePage::clickDelay(bool flag){
    widgetDisplay->buttom->setCurrentWidget(widgetDisplay->delayWidgetContainer);
    if(flag || !widgetDisplay->delayWidget->close->isChecked()){//点击延时按钮，如果当前延时按钮是选中状态/延时功能打开，应显示buttom
        m_isHidden = widgetDisplay->buttom->isHidden();
        btnPage->m_captureDelay->setChecked(true);
        btnPage->m_videoDelay->setChecked(true);
        widgetDisplay->buttom->show();
        if(widgetDisplay->nPixCapWidget->close->isChecked()){
            btnPage->m_captureBurst->setChecked(false);
        }else{
            btnPage->m_captureBurst->setChecked(true);
        }
        if (!isConnected) {
            isConnected = true;
            connect(btnPage->m_captureDelay, &CameraButton::buttonTriggered,
                    this,
                    [this] {
                showConnect(m_isHidden);
            });
            connect(btnPage->m_videoDelay, &CameraButton::buttonTriggered,
                    this,
                    [this] {
                showConnect(m_isHidden);
            });
        }
    }else{
        widgetDisplay->buttom->hide();
        if (isConnected) {
            isConnected = false;
            disconnect(btnPage->m_captureDelay, &CameraButton::buttonTriggered,
                       this, nullptr);
            disconnect(btnPage->m_videoDelay, &CameraButton::buttonTriggered,
                       this, nullptr);
        }
    }
}
//显示连拍模块
void HasDevicePage::clickNPix(bool flag){
    widgetDisplay->buttom->setCurrentWidget(widgetDisplay->nPixCapWidgetContainer);
    if(flag || !widgetDisplay->nPixCapWidget->close->isChecked()){
        m_captureBurst_isHidden = widgetDisplay->buttom->isHidden();
        btnPage->m_captureBurst->setChecked(true);
        widgetDisplay->buttom->show();
        if(widgetDisplay->delayWidget->close->isChecked()){
            btnPage->m_captureDelay->setChecked(false);
            btnPage->m_videoDelay->setChecked(false);
        }else{
            btnPage->m_captureDelay->setChecked(true);
            btnPage->m_videoDelay->setChecked(true);
        }
        if (!isConnected) {
            isConnected = true;
            connect(btnPage->m_captureBurst, &CameraButton::buttonTriggered,
                    this,
                    [this] {
                showConnect(m_captureBurst_isHidden);
            });
        }
    }else{
        widgetDisplay->buttom->hide();
        if (isConnected) {
            isConnected = false;
            disconnect(btnPage->m_captureBurst, &CameraButton::buttonTriggered,
                       this, nullptr);
        }
    }
}

void HasDevicePage::showConnect(bool ishide){
    if (ishide) {
        widgetDisplay->buttom->show();
        ishide = widgetDisplay->buttom->isHidden();
    } else {
        widgetDisplay->buttom->hide();
        ishide = widgetDisplay->buttom->isHidden();
    }
}

bool HasDevicePage::notNeedMirror(const QString &sys, const QString current)
{
    QStringList parts = sys.split(';');
    for(const QString& part : parts){
        QString trimmedPart = part.trimmed();
        if(trimmedPart == current){
            return true;
        }
    }
    return false;
}

QString HasDevicePage::getProductValue(const QString &deviceNode)
{
    QString product = "";
    QString ueventPath = "/sys/class/video4linux/";
    int pos = deviceNode.lastIndexOf('/');
    if (pos != -1) {
        // 提取设备节点字符串中的 videoX 部分并拼接
        ueventPath += deviceNode.mid(pos + 1);
    }
    ueventPath += "/device/uevent";

    // 尝试打开 uevent 文件
    QFile file(ueventPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 文件打开失败，输出警告信息
        qWarning() << "无法打开文件: " << ueventPath;
    } else {
        QTextStream in(&file);
        // 逐行读取文件内容
        while (!in.atEnd()) {
            QString line = in.readLine();
            // 查找以 PRODUCT= 开头的行
            if (line.startsWith("PRODUCT=")) {
                // 返回 PRODUCT= 后面的值
                product = line.mid(8);
                break;
            }
        }
        file.close();
    }
    return product;

}

void HasDevicePage::clickFilter(bool flag){//【暂未启用，滤镜相关】
    if(flag || !widgetDisplay->filterWidget->original->isChecked()){
        widgetDisplay->buttom->setCurrentWidget(widgetDisplay->filterWidgetContainer);
        widgetDisplay->buttom->show();
    }else{
        widgetDisplay->buttom->hide();
    }
}
bool HasDevicePage::cameraInfoSetTmp(const char *in_devname){//用于探测
    //1. 根据设备名，读取该设备的一些设置信息
    V4l2DeviceInfo device_info;
    memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info.dev_path, in_devname, strlen(in_devname)+1);
    camera->enum_device(&device_info);//获取包含设备信息的一个结构体
    //2. 读取kylinCamera的几项基本信息：设备名、编码格式、分辨率、帧率
    KylinCameraInfo camera_info;
    //2.1 获取并设置设备名
    memset(&camera_info,0X00,sizeof(KylinCameraInfo));
    memcpy(camera_info.devname, in_devname, strlen(in_devname)+1);

    uint available_formatTmp = 0;
    int formatFlagTmp = 0;
    for(int i=0;i<10;i++){
      TypePixFormats *pixfmt = &device_info.fmt_supported[i];
      unsigned int format = pixfmt->fmt.pixelformat;
      char formatArray[] = { (char)(format&0xff), (char)((format>>8)&0xff),(char)((format>>16)&0xff), (char)((format>>24)&0xff), 0 };
      qDebug() << "***pixel format: " << formatArray;
      if( strcmp(formatArray,"MJPG") == 0 ) {
          formatFlagTmp = i;
          available_formatTmp = V4L2_PIX_FMT_MJPEG;
          break;
      }else if( strcmp(formatArray,"YUYV") == 0 ) {
          formatFlagTmp = i;
          available_formatTmp = V4L2_PIX_FMT_YUYV;
      }else{
          qDebug() << "unsupported format: " << formatArray;
      }
    }
    if(available_formatTmp == 0){//该摄像头不支持MJPG/YUYV编码，设置为默认编码
        available_formatTmp = device_info.fmt_supported[0].fmt.pixelformat;
    }
    camera_info.format = available_formatTmp;
    //2.2 获取设备支持的分辨率，以及各分辨率下支持的最大帧率
    TypePixFormats fmt = device_info.fmt_supported[formatFlagTmp];
    for(int j = 0; j < 50; j++) {
        TypeFramesizes *fsize = &fmt.frm_sizes[j];
        if(fsize->frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            QPair<uint,uint> discrete;
            discrete.first = fsize->frm_size.discrete.width;
            discrete.second = fsize->frm_size.discrete.height;
            if(discrete.first != 0){
                camera_info.width = discrete.first;
                camera_info.height = discrete.second;
                camera_info.fps = fsize->frm_ivals[0].discrete.denominator;
                break;
            }else{
                continue;
            }
        }
    }
    bool res = camera_info.devname && camera_info.format &&camera_info.fps && camera_info.width;
    return res;
}
KylinCameraInfo HasDevicePage::cameraInfoSet(const char *in_devname,QString scale){//更新当前使用设备的各项设置
    //1. 重置设备信息,设备名需要更新
    CurrentDeviceInfo::available_size.clear();
    //2. 根据设备名，读取该设备的一些可用的设置信息
    V4l2DeviceInfo device_info;
    memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info.dev_path, in_devname, strlen(in_devname)+1);
    camera->enum_device(&device_info);//获取包含设备信息的一个结构体
    //2.1 获取设备支持的编码格式，并根据优先级设置好要使用的编码格式
    CurrentDeviceInfo::available_format = 0;
    CurrentDeviceInfo::formatFlag = 0;
    for(int i=0;i<10;i++){
      TypePixFormats *pixfmt = &device_info.fmt_supported[i];
      unsigned int format = pixfmt->fmt.pixelformat;
      char formatArray[] = { (char)(format&0xff), (char)((format>>8)&0xff),(char)((format>>16)&0xff), (char)((format>>24)&0xff), 0 };
      qDebug() << "***pixel format: " << formatArray;
      if( strcmp(formatArray,"MJPG") == 0 ) {
          CurrentDeviceInfo::formatFlag = i;
          CurrentDeviceInfo::available_format = V4L2_PIX_FMT_MJPEG;
          break;
      }else if( strcmp(formatArray,"YUYV") == 0 ) {
          CurrentDeviceInfo::formatFlag = i;
          CurrentDeviceInfo::available_format = V4L2_PIX_FMT_YUYV;
      }else{
          qDebug() << "unsupported format: " << formatArray;
      }
    }
    if(!(CurrentDeviceInfo::available_format == V4L2_PIX_FMT_YUYV ||
            CurrentDeviceInfo::available_format == V4L2_PIX_FMT_MJPEG)){//该摄像头不支持MJPG/YUYV编码，设置为默认编码
        Q_EMIT swithToNoDevicePage(2);
    }
    qDebug() << "showMegFormat" << CurrentDeviceInfo::available_format;
    //2.2 获取设备支持的分辨率，以及各分辨率下支持的最大帧率
    int scaleCount = 0;
    TypePixFormats fmt = device_info.fmt_supported[CurrentDeviceInfo::formatFlag];
    for(int j = 0; j < 50; j++) {
        TypeFramesizes *fsize = &fmt.frm_sizes[j];
        if(fsize->frm_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            QPair<uint,uint> discrete;
            discrete.first = fsize->frm_size.discrete.width;
            discrete.second = fsize->frm_size.discrete.height;

            sizeAndfps cell;

            if(discrete.first != 0){
                cell.available_size = discrete;
                cell.fps = fsize->frm_ivals[0].discrete.denominator;
                CurrentDeviceInfo::available_size.push_back(cell);
                qDebug() << "showMeg" << cell.available_size.first << cell.available_size.second << cell.fps;
                scaleCount++;
            }else{
                continue;
            }
        }
    }
    qDebug()<<"count of resolution pairs:"<<scaleCount;
    //3. 对读到的分辨率排序以及设置默认分辨率选项
    CurrentDeviceInfo::sort_resolution();
    int scale_num = 0;//默认取第一项分辨率
    //3.1 判断gsettings中保存的分辨率是否在当前摄像头中存在
    QStringList Scales;
    unsigned int Scale_width, Scale_height;
    if(scale != ""){
        Scales = scale.split("*");
        Scale_width = Scales.at(0).toUInt();
        Scale_height = Scales.at(1).toUInt();
        for(int i = 0; i < CurrentDeviceInfo::available_size.length(); i++){
            if(CurrentDeviceInfo::available_size[i].available_size.first == Scale_width && CurrentDeviceInfo::available_size[i].available_size.second == Scale_height){
                scale_num = i;
                break;
            }
        }
    }
    gsettingItem->setScaleIndex(scale_num);
    gsettingItem->setScale(CurrentDeviceInfo::available_size[scale_num].available_size.first, CurrentDeviceInfo::available_size[scale_num].available_size.second);
    //4. 初始化设备信息（具体的，当前使用的参数）
    //4.1 设备名
    KylinCameraInfo camera_info;
    memset(&camera_info,0X00,sizeof(KylinCameraInfo));
    memcpy(camera_info.devname, in_devname, strlen(in_devname)+1);
    strcpy(CurrentDeviceInfo::current_indevice,in_devname);
    qDebug() << "archName: " << archName;
    //4.2 编码格式
    camera_info.format = CurrentDeviceInfo::available_format;
    //4.3 分辨率
    camera_info.width = CurrentDeviceInfo::available_size[scale_num].available_size.first;
    camera_info.height = CurrentDeviceInfo::available_size[scale_num].available_size.second;
    CurrentDeviceInfo::current_size.available_size.first = camera_info.width;
    CurrentDeviceInfo::current_size.available_size.second = camera_info.height;
    //4.4 帧率
    camera_info.fps = CurrentDeviceInfo::available_size[scale_num].fps;
    CurrentDeviceInfo::current_size.fps = camera_info.fps;

    qDebug() << "showMeg" << camera_info.format;
    qDebug() << "showMeg" << camera_info.width << "*" <<camera_info.height << "*" << camera_info.fps;

    m_camInfo = camera_info;

    return camera_info;
}
//创建摄像头设备界面
void HasDevicePage::creatCameraPage(const char *in_devname){
    //1. 新建一个摄像头的对象
    if(camera == nullptr){
        camera = new KylinCamera();
        camera->camera_log_init(LOG_NOTICE);
    }    

    KylinCameraInfo camera_info = cameraInfoSet(in_devname, gsettingItem->getScale());//默认选取最高分辨率，所以第二个参数为0
    if(!(CurrentDeviceInfo::available_format == V4L2_PIX_FMT_MJPEG ||
        CurrentDeviceInfo::available_format == V4L2_PIX_FMT_YUYV)){
        has_device = true;
        return;
    }
    //2. 初始化摄像头设备，初始化镜像、捕捉画面的显示
    if(videoDisplay){
        //delete videoDisplay;
        videoDisplay = nullptr;
    }
    videoDisplay = camera->create(this, &camera_info);
    //避免设备被占用时发生崩溃
    if(videoDisplay != nullptr){
        //3.默认需要镜像翻转，特殊设备无需翻转
        //获取当前使用设备的 PRODUCT 信息
        QString product = getProductValue(in_devname);
        //获取已配置为无需镜像翻转的设备信息
        QString noMirrorDevice = gsettingItem->getNoMirrorDevice();

        //若当前使用设备已被配置为无需镜像翻转的设备
        if(notNeedMirror(noMirrorDevice, product)){
            camera->camera_display_mirror(0);
            isSpecialCamera = true;
            qDebug()<<"this device does not requires mirror reversal";
        } else {
            isSpecialCamera = false;
            qDebug()<<"this device requires mirror reversal";
            camera->camera_display_mirror(1);   //设置镜像
        }
    }
    QString tips = tr("camera is being used by kylin-camera");
    showNotify(tips);
    m_pGraphicsOpacityEffect1 = new QGraphicsOpacityEffect(this);
    //设备被占用返回值NULL
    if(videoDisplay == NULL){
        QMessageBox mes;
        mes.warning(this,tr("waring"),tr("The camera is occupied, please check the use of the device!"));
        mes.move(this->geometry().center()-mes.rect().center());
        camera = nullptr;
        m_pGraphicsOpacityEffect1 = nullptr;
        exit(0);
    }
    gsettingItem->addPreviewMonitorSDKPoint();
    camera->camera_set_background(QColor("#232426"));
    videoDisplayResize();
    connect(videoDisplay,SIGNAL(readout()),this,SLOT(pullOut()));//打开设备后，开启监听设备中断连接的信号
    if(Button::video_mode == false){
        videoDisplay->setMinimumHeight(CurrentDeviceInfo::winHeight - 100);
    }
    layout->addWidget(videoDisplay);   

    has_device = true;
    //4. 检查是否开启了镜像功能，必要情况手动镜像。（首次起应用不存在这种情况，但插拔后必须判断）
    if(btnPage->getMirror()){
        mirror(true);
    }
    //5. 切换到有设备的页面
    Q_EMIT swithToHasDevicePage();
}

void HasDevicePage::change_device(const char *in_device){//改变摄像头设备
    //1. 创建探测设备是否可用的kylinCamera类
    // cameraTmp = new KylinCamera;
    // cameraTmp->camera_log_init(LOG_NOTICE);
    bool res = cameraInfoSetTmp(in_device);
    //设备被占用返回值NULL
    if(!res){
        //切换失败的提示
        QMessageBox mes;
        mes.warning(this->window(),tr("waring"),tr("The camera is occupied or there is "
                                         "an exception in the target switching device, please check the device!"));
    }else{
        //delete cameraTmp;
        //cameraTmp = nullptr;
        stopCamera();
        creatCameraPage(in_device);
        return;
        uint formatBefore = CurrentDeviceInfo::available_format;
        //设备可用，丢弃上个设备的使用信息，切换选中的设备，并更改为默认状态
        KylinCameraInfo camera_infox = cameraInfoSet(in_device, gsettingItem->getScale());

        if(CurrentDeviceInfo::available_format != V4L2_PIX_FMT_YUYV &&
                CurrentDeviceInfo::available_format != V4L2_PIX_FMT_MJPEG){
            stopCamera();
            Q_EMIT swithToNoDevicePage(2);
        }else{
            QString tips = tr("camera is being used by kylin-camera");
            showNotify(tips);
            if(formatBefore == V4L2_PIX_FMT_MJPEG || formatBefore == V4L2_PIX_FMT_YUYV){
                camera->camera_set_param(&camera_infox);
            }else{
                startExistCamera(camera_infox);
            }

//            camera->camera_set_param(&camera_infox);
            Q_EMIT swithToHasDevicePage();
        }
        return;
    }
}
void HasDevicePage::initAudioDevice(){
    camera->camera_audio_disable();
}
void HasDevicePage::clickPhoto(){//点击拍照按钮的槽函数
    /*拍照按钮目前是一个复合型的多功能按钮，
     * ①直接拍照②进入延时拍照模式③延时过程中，中止延时直接拍④进入连拍模式⑤连拍过程中中止连拍，
     * 要结合拍照流程注意各功能的响应优先级！！！*/
    //1. 存储路径的判断
    if(!check_save_path()) {
        QMessageBox mes;
        mes.warning(this->window(),tr("waring"),tr("save path can't write"));
        mes.move(this->geometry().center()-mes.rect().center());
        return;
    }
    //2. case1：连拍过程中、倒计时过程中终止拍照
    if(burstTimer && burstTimer->isActive()){
        Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
        burstCaptureInterrupt();
        return;
    }
    //3.case2：开启计时，进入延时拍照模式
    if((!timer || !timer->isActive()) && has_device && deadTimeSec != 0){
        Q_EMIT GlobalInstance::getInstance()->optionStatus(false);
        //延时倒计时和计数器开启,并返回
        is_video = false;
        gsettingItem->setIsVideo(false);
        Button::delay_mode = true; //关闭录像切换功能
        if(timer != nullptr){
            delete timer;
            timer = nullptr;
        }
        timer = new QTimer(this);
        connect(timer,&QTimer::timeout, this, &HasDevicePage::timeEvent);
        btnPage->cheese->setIcon(QIcon(":/image/shot-hover.svg"));
        timer->start(1000);
        deadTimeSecTmp = 0;
        gsettingItem->addDelaySDKPoint();
        btnPage->enableButtons(false);
        return;
    }
    //4. case3：延时过程中，终止计时,拍照
       if(Button::delay_mode && timer && timer->isActive()){
           Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
           timer->stop();
           Button::delay_mode = false;   //开启录像切换功能
           deadTimeSecTmp = 0;      //倒计时清0
           widgetDisplay->dead_time->hide();
           is_video = false;
           btnPage->enableButtons(true);
           takePhotoPre();
           Q_EMIT settingOptionsOn(true);
           return;
       }

    //5. case4：直接拍照
    is_video = false;
    btnPage->enableButtons(true);
    takePhotoPre();
}

void HasDevicePage::takePhotoPre(){//实施具体的拍照操作，分三种情况
    if(!has_device){
        return;
    }
    //case1：录像后拍缩略图
    if(is_video){
        QString name = fileNameTemp;
        takePhoto(name,true);
        return;
    }
    //case2：单次拍照
    if(nPix == 1){
        QString name = creatName();
        onOpacityWindow();
        takePhoto(name,false);
        gsettingItem->addSingleShotSDKPoint();
    }else{
    //case3：连拍
        btnPage->enableButtons(false);
        Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(false);
        Q_EMIT GlobalInstance::getInstance()->optionStatus(false);
        if(burstTimer){
            delete burstTimer;
            burstTimer = nullptr;
        }
        burstTimer = new QTimer(this);
        gsettingItem->addContinuousShotSDKPoint();
        burstControl();        
        connect(burstTimer,SIGNAL(timeout()),this,SLOT(burstControl()));
//        burstTimer->start(1000);
    }
}
bool HasDevicePage::check_save_path(){
    //存储路径不存在,将储存路径修改为用户目录
    QString imagePath = gsettingItem->getSavePath();
    QString videoPath = gsettingItem->getVideoSavePath();
    qDebug() << "存储路径" << imagePath;
    qDebug() << "存储路径" << videoPath;
    //lhl debug 方案1 路径不存在建立默认路径，并提示。
    if(!QDir(imagePath).exists()){
        qDebug() << "image path is not exist:" << imagePath;
        noPathPrompt();
        imagePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/";
        QDir dir(imagePath);
        if (!dir.exists()) {
            dir.mkpath(imagePath);
        }
        gsettingItem->setSavePath(imagePath);
    }
    if(!QDir(videoPath).exists()){
        qDebug() << "video path is not exist:" << gsettingItem->getVideoSavePath();
        noPathPrompt();
        QString videoPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/";
        QDir dir(videoPath);
        if (!dir.exists()) {
            dir.mkpath(videoPath);
        }
        gsettingItem->setVideoSavePath(videoPath);
    }
    return true;
}
void HasDevicePage::noPathPrompt(){
    QMessageBox mes;
    mes.warning(this->window(),tr("waring"),tr("path does not exist, save to default path"));
    mes.move(this->geometry().center()-mes.rect().center());
}
//创建录像或录像名
QString HasDevicePage::creatName(){
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyyMMdd_hhmmss")+"_"+QString::number(rand() % 10);
}
//拍照功能
//name:图像路径+名字；isvedo：是否为录像 1为录像模式 ，0为拍照模式
void HasDevicePage::takePhoto(QString name ,bool isvideo){
    //1. 检查有无设备
    if(!has_device){
        return;
    }
    //2. 生成文件名（根据视频/图片的文件名类型不同，生成不同的文件名）
    QString path;
    if(isvideo){
        QString homeLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
        if(!QDir(homeLocation + "/kylin-camera").exists()) {
            QDir(homeLocation).mkdir("kylin-camera");
        }
        path = path + homeLocation + "/kylin-camera/";
        configPath = path;
        path = path + "." +name + ".jpg";//将文件存为隐藏文件
    }
    else{
        imagePath = gsettingItem->getSavePath();
        photoFormat = "jpg";          //暂时拍照只支持jpg
        path = imagePath + name + "." + photoFormat;
        gsettingItem->setFileName(path);
    }
    //3. 调用库函数拍照，库默认照片格式是Mjpg
    camera->camera_take_photo(path.toLocal8Bit().data());
    qDebug() << path;
    AlbumRead *albRear = new AlbumRead(path, this);
    connect(albRear,&AlbumRead::finish,this,&HasDevicePage::imageDisplay);
    albRear->start();
}
//将图片显示在缩略图label上
void HasDevicePage::imageDisplay(QString filename){
    fileNameZ = filename;
    if (photoFormat == ""){filename += ".jpg";}
    qDebug() << "filename" << filename;
    QPixmap *pixmap = new QPixmap;
    if(!pixmap->load(filename,"jpg"))
    {
       qDebug() << "load error";
    }
    QPixmap fitPixmap = pixmap->scaled(48,36,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    QListWidgetItem *imageItem = new QListWidgetItem;
    //将信息写入照片缩略图
    btnPage->picture->setPixmap(fitPixmap);
    QString tmp = gsettingItem->getSavePath();
    imageItem->setIcon(QIcon(fitPixmap));
    //判断是否是录像
    if(is_video){
        btnPage->pictureVideo->show();
        imageItem->setStatusTip(imagePath + fileNameTemp + videoFormat);
        video_path_n = imagePath + fileNameTemp + videoFormat;
    }
    else{
        btnPage->pictureVideo->hide();
        imageItem->setStatusTip(filename);
    }
    delete pixmap;
}
//延时计时器触发函数
void HasDevicePage::timeEvent(){
    bool flag = ((deadTimeSec - deadTimeSecTmp) <= 4) && ((deadTimeSec - deadTimeSecTmp) != 0);
    if(flag && timer->isActive()){
        Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(false);
    }else{
        Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(true);
    }
    //1. 倒计时未结束时的前端操作
    if(deadTimeSecTmp < deadTimeSec){
        QString str = QString::number(deadTimeSec - deadTimeSecTmp);
        widgetDisplay->dead_time->setText(str);
        widgetDisplay->dead_time->show();
        deadTimeSecTmp++;
        return;
    }
    //2.倒计时结束时的前端操作
    Button::delay_mode = false;   //开启录像切换功能
    deadTimeSecTmp = 0;      //倒计时清0
    widgetDisplay->dead_time->hide();
    btnPage->cheese->setIcon(QIcon(":/image/shot-click.svg"));

    //3.停止计时器，准备拍照
    timer->stop();
    btnPage->enableButtons(true);
    takePhotoPre();
    // 如果此时是连拍模式，需要保持选项按钮禁用，直到连拍结束或被打断
    if(nPix == 1){
        Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
    }
    Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(true);
}

void HasDevicePage::videoTimerEvent(){
    bool flag = ((m_videoDeadTimeSec - m_videoDeadTimeSecTmp) <= 4) && ((m_videoDeadTimeSec - m_videoDeadTimeSecTmp) != 0);
    if(flag && m_videoTimer->isActive()){
        Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(false);
    }else{
        Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(true);
    }
    //1. 倒计时未结束时的前端操作
    if(m_videoDeadTimeSecTmp < m_videoDeadTimeSec){
        Q_EMIT GlobalInstance::getInstance()->optionStatus(false);
        QString str = QString::number(m_videoDeadTimeSec - m_videoDeadTimeSecTmp);
        widgetDisplay->dead_time->setText(str);
        widgetDisplay->dead_time->show();
        m_videoDeadTimeSecTmp++;
        return;
    }
    //2.倒计时结束时的前端操作
    Button::delay_mode = false;   //开启录像切换功能
    m_videoDeadTimeSecTmp = 0;      //倒计时清0
    widgetDisplay->dead_time->hide();
    btnPage->cheese_video->setIcon(QIcon(":/image/shot-click.svg"));

    //3.停止计时器，准备拍照
    btnPage->enableButtons(true);
    m_videoTimer->stop();
    startRecord();
    Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(true);
}

void HasDevicePage::burstControl(){//连拍的槽函数
    burstTimer->stop();
    QString name = creatName();
    takePhoto(name,false);
    onOpacityWindow();
    gsettingItem->playAlertSound("photo");
    pixCount++;
    if(pixCount >= nPix){//恢复连拍过程中禁掉的按钮
        Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
        btnPage->enableButtons(true);
        Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(true);
//        burstTimer->stop();
        delete burstTimer;
        burstTimer = nullptr;
        pixCount = 0;
    }else{
        // 确保每拍一张后调用一次拍照接口（camera_take_photo）。避免调用过快时，导致出现拍照请求被覆盖的现象（连拍张数不够）。
        burstTimer->start(1000);
    }
}
//录像
void HasDevicePage::startRecord()
{
    QTime time;
    time.start();
    //1. 判断当前摄像头设备在不在
    struct stat s_stat;
    memset(&s_stat,0x00,sizeof(struct stat));
    int ret = stat(CurrentDeviceInfo::current_indevice,&s_stat);  //获取文件信息
    if(ret == -1){
        return;
    }
    qDebug() << " HasDevicePage::startRecord time 1 is: " << time.restart();
    //2. 检查保存路径
    if(!check_save_path()) {
        QMessageBox mes;
        mes.warning(this,tr("waring"),tr("save path can't write"));
        mes.move(this->geometry().center()-mes.rect().center());
        return;
    }
    qDebug() << " HasDevicePage::startRecord time 2 is: " << time.restart();
    //3. 检查摄像头是否正处于编码状态
    enum CameraState state;
    state = camera->camera_get_state(CAMERA_ENCODE);
    //if(state == CAMERA_ENCODE_START){
    if(state != CAMERA_ENCODE_STOP){
        QMessageBox mes;
        mes.warning(this->window(),tr("waring"),tr("File generation in progress, please try again later"));
        mes.move(this->geometry().center()-mes.rect().center());
        return;
    }
    qDebug() << " HasDevicePage::startRecord time 3 is: " << time.restart();
    //4. 创建新录像的文件名
    //目前仅支持MP4格式的视频
    videoFormat = ".mp4";
    QString name = creatName();
    fileNameTemp = name;
    is_video = true;
    gsettingItem->setIsVideo(true);
    QString tmp = gsettingItem->getVideoSavePath() + name + videoFormat;
    gsettingItem->setFileName(tmp);
    qDebug() << " HasDevicePage::startRecord time 4 is: " << time.restart();
    //5. 准备在录像期间监听系统锁屏、睡眠信号-》为了保证睡眠/休眠时录像可以正常保存
    m_dbus->inhibit(login1dbusManager::INHIBIT_SHUTDOWN);
    m_dbus->inhibit(login1dbusManager::INHIBIT_SLEEP);
    //阻止系统在用户无操作时锁屏
    flag = sessionManagementTest.setInhibitLockScreen(kdk::kabase::AppName::KylinCamera , QString("recording"));
    if (flag == 0) {
        qCritical() << "set inhibit lock screen fail";
        return;
    }
    qInfo() << "set inhibit lock screen success ! flag = " << flag;
    qDebug() << " HasDevicePage::startRecord time 5 is: " << time.restart();
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    qDebug() << " HasDevicePage::startRecord time 6 is: " << time.restart();
    //7. 查找录音设备
    camera->camera_audio_init(info);
    qDebug() << " HasDevicePage::startRecord time 7 is: " << time.restart();
    qDebug() << info.deviceName() <<" "<< QAudioDeviceInfo::defaultOutputDevice().deviceName();
    //8. 拍一张缩略图
    takePhotoPre();
    enum CameraState stateCapturing;
    stateCapturing = camera->camera_get_state(CAMERA_PHOTO);
    while(stateCapturing == CAMERA_PHOTO_BUZY){
        stateCapturing = camera->camera_get_state(CAMERA_PHOTO);
        qDebug() << "is capturing!";
        QThread::msleep(50);
    }
    qDebug() << " HasDevicePage::startRecord time 8 is: " << time.restart();
    //9. 调用录像的库函数
    camera->camera_start_record(tmp.toLocal8Bit().data(),false);
    gsettingItem->addVideotapeSDKPoint();
    qDebug() << " HasDevicePage::startRecord time 9 is: " << time.restart();
    //10. 开启获取录像时长的计时器
    if(recordTimer){
        delete recordTimer;
        recordTimer = nullptr;
    }
    qDebug() << " HasDevicePage::startRecord time 10 is: " << time.restart();
    recordTimer = new QTimer;
    connect(recordTimer,&QTimer::timeout, this, &HasDevicePage::updateRecordTime);
    recordTimer->start(150);
    /********前端页面相关start*************/
    //录像不使用延时功能，检查延时按钮开启，则关闭
    if(btnPage->m_captureDelay->isChecked() && !if_delay){
        btnPage->m_captureDelay->click();
    }
    Button::video_start = 1;
    widgetDisplay->buttom->setCurrentWidget(widgetDisplay->recordModuleContainer);
    btnPage->hide();
    widgetDisplay->buttom->show();
    qDebug() << " HasDevicePage::startRecord time final is: " << time.elapsed();
    Q_EMIT clickRecordSig();
    /********前端页面相关end*************/
}

void HasDevicePage::clickStartRecord()
{
    //点击开始录像，禁用菜单栏选项按钮
    Q_EMIT GlobalInstance::getInstance()->optionStatus(false);
    /*拍照按钮目前是一个复合型的多功能按钮，
     * ①直接拍照②进入延时拍照模式③延时过程中，中止延时直接拍④进入连拍模式⑤连拍过程中中止连拍，
     * 要结合拍照流程注意各功能的响应优先级！！！*/
    //1. 存储路径的判断
    if(!check_save_path()) {
        QMessageBox mes;
        mes.warning(this->window(),tr("waring"),tr("save path can't write"));
        mes.move(this->geometry().center()-mes.rect().center());
        return;
    }
    //2.case2：开启计时，进入延时录像模式
    if((!m_videoTimer || !m_videoTimer->isActive()) && has_device && m_videoDeadTimeSec != 0){
        //延时倒计时和计数器开启,并返回
        btnPage->enableButtons(false);
        gsettingItem->setIsVideo(false);
        Button::delay_mode = true; //关闭录像切换功能
        if(m_videoTimer != nullptr){
            delete m_videoTimer;
            m_videoTimer = nullptr;
        }
        m_videoTimer = new QTimer(this);
        connect(m_videoTimer,&QTimer::timeout, this, &HasDevicePage::videoTimerEvent);
        btnPage->cheese_video->setIcon(QIcon(":/image/shot-hover.svg"));
        m_videoTimer->start(1000);
        m_videoDeadTimeSecTmp = 0;
        gsettingItem->addDelaySDKPoint();
        return;
    }
    //3. case3：延时过程中，终止计时,开始录像
       if(Button::delay_mode && m_videoTimer && m_videoTimer->isActive()){
           btnPage->enableButtons(true);
           m_videoTimer->stop();
           Button::delay_mode = false;   //开启录像切换功能
           deadTimeSecTmp = 0;      //倒计时清0
           widgetDisplay->dead_time->hide();
           is_video = false;
           startRecord();
           Q_EMIT settingOptionsOn(true);
           return;
       }

    //5. case4：录像
    is_video = false;
    btnPage->enableButtons(true);
    startRecord();
}
//停止录像
void HasDevicePage::clickStopRecord(){
    Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
    //1. 关闭获取录像计时的计时器
    if(recordTimer){
        recordTimer->stop();
        delete recordTimer;
        recordTimer = nullptr;
    }
    int secTmp = camera->camera_get_record_time();
    qDebug() << "录制时长" << (secTmp/1000)%60;

    //2. 调用停止录像的库函数
    gsettingItem->playAlertSound("screen");
    camera->camera_stop_record(); //仅触发停止
    //4. 为了录像的完整性，对特殊情况进行异常检测（在除突然断电类似情况下，保证使录像编码结束后再退出应用）
    coadState = new CameraCodeState(camera);
    coadState->setFlag(exceptionFlag);
    connect(coadState, &CameraCodeState::camerafinish, [=](){btnPage->pictureVideo->setEnabled(true);btnPage->installEventFilter();});
    connect(coadState, SIGNAL(camerafinish()), coadState, SLOT(deleteLater()));
    coadState->start();
    //修复bug#212030 【整机生态-SP1-2403】【宝信创NF271】【摄像头】\打开摄像头软件后将机器睡眠，唤醒后显示无摄像头可用 影响未知。
//    m_dbus->unInhibit();//停掉阻止系统睡眠/休眠
    sessionManagementTest.unInhibitLockScreen(flag);
    //6. 前端显示相关
    widgetDisplay->recordModule->videoPauseBtn->show();
    widgetDisplay->recordModule->hide();
    widgetDisplay->recordModule->hour = 0;
    widgetDisplay->recordModule->min = 0;
    widgetDisplay->recordModule->sec = 0;
    hour = min = sec = -1;
    btnPage->cheese_stop->click();
    btnPage->pictureVideo->setEnabled(false);
    btnPage->rmEventFilter();
    btnPage->show();
    widgetDisplay->buttom->hide();
}
//关于延时的操作
void HasDevicePage::delay_operation(int i){
    switch (i) {
    case 0:
        deadTimeSec = 0;
        m_videoDeadTimeSec = deadTimeSec;
        if_delay = false;
        btnPage->m_captureDelay->setIcon(QIcon::fromTheme("ukui-countselect-symbolic"));
        btnPage->m_videoDelay->setIcon(QIcon::fromTheme("ukui-countselect-symbolic"));
        break;
    case 1:
        deadTimeSec = 3;
        m_videoDeadTimeSec = deadTimeSec;
        btnPage->m_captureDelay->setIcon(QIcon::fromTheme("ukui-countselect-3s-symbolic"));
        btnPage->m_videoDelay->setIcon(QIcon::fromTheme("ukui-countselect-3s-symbolic"));
        if_delay = true;
        break;
    case 2:
        deadTimeSec = 6;
        m_videoDeadTimeSec = deadTimeSec;
        btnPage->m_captureDelay->setIcon(QIcon::fromTheme("ukui-countselect-6s-symbolic"));
        btnPage->m_videoDelay->setIcon(QIcon::fromTheme("ukui-countselect-6s-symbolic"));
        if_delay = true;
        break;
    case 3:
        deadTimeSec = 9;
        m_videoDeadTimeSec = deadTimeSec;
        btnPage->m_captureDelay->setIcon(QIcon::fromTheme("ukui-countselect-9s-symbolic"));
        btnPage->m_videoDelay->setIcon(QIcon::fromTheme("ukui-countselect-9s-symbolic"));
        if_delay = true;
        break;
    default:
        break;
    }
    if(deadTimeSec != 0){
        btnPage->m_captureDelay->setChecked(true);
        btnPage->m_videoDelay->setChecked(true);
    }else{
        btnPage->m_captureDelay->setChecked(false);
        btnPage->m_videoDelay->setChecked(false);
    }
    widgetDisplay->buttom->hide();
    gsettingItem->setDelayTime(deadTimeSec);
}
//关于连拍的操作
void HasDevicePage::nPixOperation(int i){
    nPix = i;
    btnPage->setBurstCount(i);
    widgetDisplay->buttom->hide();
}
//镜像 与实际相反 反向设置
void HasDevicePage::mirror(bool mirror){
    if(!has_device) return;

    // 0 表示使用设备返回的原始画面（特殊设备原始画面是镜像的）；1 表示使用翻转之后的画面
    // 普通设备 check 镜像按钮 或 特殊设备未 check 镜像按钮时，需要拍摄到镜像画面;
    if(mirror && !isSpecialCamera || !mirror && isSpecialCamera){
        camera->camera_display_mirror(0);
    }
    // 普通设备未 check 镜像按钮  或 特殊设备 check 镜像按钮时，需要拍摄到非镜像画面
    else {
        camera->camera_display_mirror(1);
    }

}
//更新录像时间
void HasDevicePage::updateRecordTime(){
    long long tmp = camera->camera_get_record_time();
    tmp += 0;//为了向上取整，也避免加的值超过1秒(1000ms)引发初始值显示错误
    int secTmp = sec;
    sec = (tmp/1000)%60;
    if(secTmp == sec){
        return;
    }else{
        hour = (tmp/1000/60/60)%60;
        min = (tmp/1000/60)%60;
        sec = (tmp/1000)%60;
        Q_EMIT recoredTimeChanged(hour,min,sec);
    }
}
void HasDevicePage::startExistCamera(KylinCameraInfo cameraInfo){
    if(camera == nullptr){
        camera = new KylinCamera;
        camera->camera_log_init(LOG_NOTICE);
    }
    videoDisplay = camera->create(this, &cameraInfo);
    //设备被占用返回值NULL
    if(videoDisplay == NULL){
        QMessageBox mes;
        mes.warning(this,tr("waring"),tr("The camera is occupied, please check the use of the device!"));
        mes.move(this->geometry().center()-mes.rect().center());
        delete camera;
        exit(0);
    }
    gsettingItem->addPreviewMonitorSDKPoint();
    camera->camera_set_background(QColor("#232426"));
    QString tips = tr("camera is being used by kylin-camera");
    showNotify(tips);
    m_pGraphicsOpacityEffect1 = new QGraphicsOpacityEffect(this);
    //设备被占用返回值NULL

    videoDisplayResize();
    //btnPage->cheese_video->setDisabled(false);
    connect(videoDisplay,SIGNAL(readout()),this,SLOT(pullOut()));
    if(Button::video_mode == false){
        videoDisplay->setMinimumHeight(CurrentDeviceInfo::winHeight - BUTTONPAGEHEIGHT);
    }
    mirror(btnPage->getMirror());
    qDebug() << "cache : " << videoDisplay->children();
    layout->addWidget(videoDisplay);
}
void HasDevicePage::pullOut(){
    if(Button::video_start){
        clickStopRecord();
    }

}
void HasDevicePage::camera_set_param(KylinCameraInfo *device_info){
    camera->camera_set_param(device_info);
}
//修改摄像头分辨率
void HasDevicePage::change_resolution(QPair<uint, uint> resolution){
    KylinCameraInfo camerainfo;
    memset(&camerainfo,0X00,sizeof(KylinCameraInfo));
    memcpy(camerainfo.devname,CurrentDeviceInfo::current_indevice,strlen(CurrentDeviceInfo::current_indevice)+1);
    qDebug() <<"devname:" << camerainfo.devname;
    qDebug() <<  CurrentDeviceInfo::available_format <<  V4L2_PIX_FMT_MJPEG << V4L2_PIX_FMT_YUYV;
    camerainfo.format = CurrentDeviceInfo::available_format;
    camerainfo.width = resolution.first;
    camerainfo.height = resolution.second;
    camerainfo.fps = CurrentDeviceInfo::current_size.fps;
    qDebug() << "showMegChange" << camerainfo.width << camerainfo.height << camerainfo.fps;

    m_camInfo = camerainfo;
    //camera->camera_set_param(&m_camInfo);
    stopCamera();
    creatCameraPage(CurrentDeviceInfo::current_indevice);
}
KylinCameraInfo HasDevicePage::getCameraInfo(){
    return m_camInfo;
}
void HasDevicePage::noDeviceHandle(){
    if(camera !=nullptr){
        delete camera;
        camera = nullptr;
    }
    has_device = false;
}
void HasDevicePage::stopCamera()
{
    if(camera != nullptr || CurrentDeviceInfo::available_format == V4L2_PIX_FMT_MJPEG || CurrentDeviceInfo::available_format == V4L2_PIX_FMT_YUYV)
    {
        delete camera;
        camera = nullptr;
        m_pGraphicsOpacityEffect1 = nullptr;
    }
}
void HasDevicePage::initDbus()
{
    //对特殊的、会打断应用的系统信号进行监听
    m_dbus = new login1dbusManager;
    connect(m_dbus,&login1dbusManager::locking,this,&HasDevicePage::recordInterrupt);
    connect(m_dbus,&login1dbusManager::unlocking,this,&HasDevicePage::recordInterruptDone);
    connect(m_dbus,&login1dbusManager::shuttingDown,this,&HasDevicePage::onShutDown);
    //Unix signals...
    UnixSignals* usignals = new UnixSignals(this);
    connect(usignals, SIGNAL(sigTerm()), this, SLOT(onSigTerm()));
    connect(usignals, SIGNAL(sigHup()), this, SLOT(onSigHup()));
    connect(usignals, SIGNAL(sigAbrt()), this, SLOT(onSigAbrt()));
}
void HasDevicePage::recordingStop(bool res){
    Q_UNUSED(res);
    changeUserFlag = true;
}
//收到睡眠信号或者锁屏信号时调用，锁屏时调用两次
//进入锁屏、进入休眠时调用（参数是true）
//解除锁屏时调用（参数是false）
//接触休眠不调用
void HasDevicePage::recordingInterrupt(bool in){//录像过程被特殊情况打断
    if (in) {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                 << " 01recordingInterrupt:" << in;
        if (Button::video_start) {
//            Q_EMIT btnPage->cheese_stop->clicked();
            exceptionFlag = true;
            clickStopRecord();
            coadState->wait();

        }
        //判断是否在连拍状态的检查流程
        burstCaptureInterrupt();
        //Delete Camera
        stopCamera();

    } else {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                 << " 02recordingInterrupt:" << in;
        //Create New Camera
        KylinCameraInfo info = getCameraInfo();
        startExistCamera(info);
        exceptionFlag = false;
    }
}
void HasDevicePage::burstCaptureInterrupt(){//连拍过程被打断的处理
    if(burstTimer){
        if(burstTimer->isActive()){
            qDebug()<<"is capturing photo,break continuous capturing photo!";
            btnPage->enableButtons(true);
            Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
            Q_EMIT GlobalInstance::getInstance()->settingOptionsOn(true);
            burstTimer->stop();
            delete burstTimer;
            burstTimer = nullptr;
            pixCount = 0;
        }
    }
}
void HasDevicePage::recordInterrupt(bool flag){
    qDebug()<<QThread::currentThreadId();
    /*changeUserFlag是 锁屏信号处理流程 和 解锁屏信号处理流程 的互斥锁*/
    while(changeUserFlag){
        sleep(1);
    }
    qDebug() << "has got screenLock signal" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //录像中的善后操作
    if (Button::video_start) {
        exceptionFlag = true;
        qDebug()<<"break recording!";
        clickStopRecord();
        //Delete Camera
        while(!coadState->isFinished()){
            QThread::msleep(100);
            qDebug()<<"waiting for compacting video!";
        }
    }
    //连拍中的善后操作
    burstCaptureInterrupt();
    changeUserFlag = true;

    //开启锁屏期间关闭摄像头的计时器，5s内计时器未被关闭&&摄像头处于开启状态，则手动关闭摄像头
    if(flag){
        countDownStopCameraEvent();
    }else{
        countDownStopCameraTimer = new QTimer(this);
        connect(countDownStopCameraTimer,&QTimer::timeout, this, &HasDevicePage::countDownStopCameraEvent);
        countDownStopCameraTimer->start(5000);
        qDebug() << "open heartBeat test!";
    }
}
void HasDevicePage::countDownStopCameraEvent(){
    qDebug() << "heartBeat test work successful!";
    if(countDownStopCameraTimer != nullptr){
        countDownStopCameraTimer->stop();
        delete countDownStopCameraTimer;
        countDownStopCameraTimer = nullptr;
    }
    if(camera){
        stopCamera();
        Q_EMIT swithToNoDevicePage(1);
        //关闭热插拔信号监听
        Q_EMIT lockSigForDevicePull(true);
        qDebug() << "listening pull out/in siganal,[off]";
    }
}
void HasDevicePage::recordInterruptDone()
{
    qDebug()<<QThread::currentThreadId();
    /*changeUserFlag是 锁屏信号处理流程 和 解锁屏信号处理流程 的互斥锁*/
    while(!changeUserFlag){
        qDebug() << "has got screenUnLock signal,waiting for screenLock process end!";
        sleep(1);
    };
    //锁屏期间是否关闭摄像头的计时器处于触发状态？关闭：不管
    if(countDownStopCameraTimer){
        qDebug() << "close heartBeat test!";
        countDownStopCameraTimer->stop();
        delete countDownStopCameraTimer;
        countDownStopCameraTimer = nullptr;
    }
    //判断锁屏和解锁屏前后是否切换了用户
    QString currentUser = qgetenv("USER");
    qDebug() << "USER =" << qgetenv("USER");
    if (currentUser.isEmpty()){
        currentUser = qgetenv("USERNAME");
    }
    QString appUser = gsettingItem->getUser();
    bool userChanged;
    currentUser == appUser ? userChanged = false : userChanged = true;

    if(userChanged && camera){
        /* 用户切换（true） + 摄像头状态（on）-》关闭摄像头 */
        qDebug() << "user is not app owner,camera ready to close!";
        stopCamera();
        Q_EMIT swithToNoDevicePage(1);
    }else if(!userChanged && !camera){
        /*
         * 用户切换（false） + 摄像头状态（off）-》视有无可用摄像头的情况决定是否开启摄像头
         *  1. 无设备，切换到无设备页面
         *  2. 有设备，重启设备，两种情况：
         *      2.1. 锁屏前的设备还在，起该设备
         *      2.2. 锁屏前的设备不在了，切到可用的默认设备
        */
        qDebug() << "user is app owner,try to keep camera open!";
        QList<QCameraInfo> list = QCameraInfo::availableCameras();
        if(!list.isEmpty()){
            //2. 有设备，重启设备，两种情况：
            qDebug() << "camera avaliable,restarting camera...";
            struct stat s_stat;
            memset(&s_stat,0x00,sizeof(struct stat));
            int ret = stat(CurrentDeviceInfo::current_indevice,&s_stat);  //获取文件信息
            if(ret == -1){
                //2.2. 锁屏前的设备不在了，切到可用的默认设备
                if(!list.isEmpty()){
                    qDebug() << "restart camera:camera before screenLock != camera after screenUnLock";
                    QString str = QCameraInfo::availableCameras().at(0).deviceName();
                    memset(CurrentDeviceInfo::current_indevice,0X00,strlen(CurrentDeviceInfo::current_indevice));
                    strcpy(CurrentDeviceInfo::current_indevice, str.toLocal8Bit().data());
                    creatCameraPage(CurrentDeviceInfo::current_indevice);
                }
            }else{
                //2.1. 锁屏前的设备还在，起该设备
                qDebug() << "restart camera:camera before screenLock == camera after screenUnLock";
                creatCameraPage(CurrentDeviceInfo::current_indevice);
            }
            exceptionFlag = false;
        }else{
            //1. 无设备，切换到无设备页面
            qDebug() << "camera not avaliable,switch to noDevicePage";
            Q_EMIT swithToNoDevicePage(1);
        }
        CurrentDeviceInfo::current_device_list.clear();
        CurrentDeviceInfo::current_device_list = QCameraInfo::availableCameras();
        Q_EMIT GlobalInstance::getInstance()->settingPageSig();
        //开启热插拔信号监听
        qDebug() << "listening pull out/in siganal,[on]";
        Q_EMIT lockSigForDevicePull(false);

    }else{
        /*
         * ①用户切换（true） + 摄像头状态（on）,do nothing
         * ②用户切换（false） + 摄像头状态（off）,do nothing
        */
        bool isCameraOn = camera==nullptr?false:true;
        qDebug() << "keep camera state: userchanged?"<<userChanged << "camera on?"<< isCameraOn;
    }
    changeUserFlag = false;
}
void HasDevicePage::onShutDown()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
             << " onShutDown signal received!" ;
    if (Button::video_start) {
        btnPage->video_start = false;
        exceptionFlag = true;
        clickStopRecord();
        coadState->wait();
    }
    stopCamera();
    //判断是否在连拍状态的检查流程
    burstCaptureInterrupt();
}
void HasDevicePage::onSigTerm()
{
    if(Button::video_start){
        btnPage->video_start = false;
        exceptionFlag = true;
        clickStopRecord();
        coadState->wait();
    }

    stopCamera();
    //判断是否在连拍状态的检查流程
    burstCaptureInterrupt();
    qDebug() << "sig exc err1";
    exit(0);
}
void HasDevicePage::onSigHup()
{
    if(Button::video_start){
        btnPage->video_start = false;
        exceptionFlag = true;
        clickStopRecord();
        coadState->wait();
    }
    stopCamera();
    //判断是否在连拍状态的检查流程
    burstCaptureInterrupt();
    qDebug() << "sig exc err2";
    exit(0);
}
void HasDevicePage::onSigAbrt(){
    if(Button::video_start){
        btnPage->video_start = false;
        exceptionFlag = true;
        clickStopRecord();
        coadState->wait();
    }

    stopCamera();
    //判断是否在连拍状态的检查流程
    burstCaptureInterrupt();
    qDebug() << "sig exc err3";
    exit(0);
}
//拔出摄像头的异常处理
void HasDevicePage::exception_handing(){

    if(btnPage->video_start){
        btnPage->video_start = false;
        exceptionFlag = true;
        clickStopRecord();        
        btnPage->cheese_stop->hide();
        btnPage->cheese_video->show();
        coadState->wait();
    }
    videoDisplay = nullptr;
    //判断是否在连拍状态的检查流程
    burstCaptureInterrupt();
    stopCamera();
    if(Button::delay_mode){
        //停止延时拍照计时，拍照按钮切换为未点击时的初始状态，恢复菜单栏选项按钮为可点击状态
        if(timer && timer->isActive()){
            btnPage->cheese->setIcon(QIcon(":/image/shot-click.svg"));
            btnPage->cheese_dark->setIcon(QIcon(":/image/shot-dark.svg"));
            timer->stop();
            Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
        }
        //停止延时摄像计时，摄像按钮切换为未点击时的初始状态，恢复菜单栏选项按钮为可点击状态
        if(m_videoTimer && m_videoTimer->isActive()){
            m_videoTimer->stop();
            btnPage->cheese_video->setIcon(QIcon(":/image/actiondark-click.svg"));
            Q_EMIT GlobalInstance::getInstance()->optionStatus(true);
        }
        btnPage->enableButtons(true);
        Button::delay_mode = false;   //开启录像切换功能
        deadTimeSecTmp = 0;      //倒计时清0
        m_videoDeadTimeSecTmp = 0;
        widgetDisplay->dead_time->hide();
    }
}
//QWidget控件透明动画实现
//要做子控件透明需要用到QGraphicsOpacityEffect类
void HasDevicePage::onOpacityWindow(){//闪屏特效
    if(!has_device||videoDisplay==NULL){
        return;
    }
    camera->camera_set_cover(1);
    gsettingItem->playAlertSound("photo");
//    m_pGraphicsOpacityEffect1->setOpacity(1.0);
//    videoDisplay->setGraphicsEffect(m_pGraphicsOpacityEffect1);
//    QPropertyAnimation *pAnimation = new QPropertyAnimation(m_pGraphicsOpacityEffect1,"opacity",this);
//    pAnimation->setDuration(100);
//    pAnimation->setKeyValueAt(0, 1);
//    pAnimation->setKeyValueAt(0.5, 0);
//    pAnimation->setKeyValueAt(1, 1);
//    pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}
void HasDevicePage::videoDisplayResize(){//根据拍照/录像模式的不同，重置videoDisplay的大小
    if(videoDisplay == nullptr){
        return;
    }
    if(Button::video_mode){
        videoDisplay->setFixedHeight(CurrentDeviceInfo::winHeight);
    }else{
        videoDisplay->setFixedHeight(CurrentDeviceInfo::winHeight-BUTTONPAGEHEIGHT);
    }
}
void HasDevicePage::suspendRecord(bool flag){
    if(flag){
        recordTimer->stop();
    }else{
        recordTimer->start(150);
    }
    camera->camera_record_pause(flag);
}
void HasDevicePage::clickVideoMode(){//拍照到录像模式切换时选项框要隐藏掉
    if(!widgetDisplay->buttom->isHidden()){
        widgetDisplay->buttom->hide();
    }
    //若未设置延时时间，在切换模式时将延时按钮置为未选中
    if(!if_delay){
        btnPage->m_videoDelay->setChecked(false);
        btnPage->m_captureDelay->setChecked(false);
    }
    //保证点击了连拍按钮且未设置连拍张数时，切换拍照-录像模式时，将连拍按钮置为未选中状态
    if(nPix == 1){
        btnPage->m_captureBurst->setChecked(false);
    }
    //defaultFilterSet();
}
void HasDevicePage::clickCaptureMode(){//录像到拍照模式切换时隐藏选项框
    if(!widgetDisplay->buttom->isHidden()){
        widgetDisplay->buttom->hide();
    }
    //若未设置延时时间，在切换模式时将延时按钮置为未选中
    if(!if_delay){
        btnPage->m_videoDelay->setChecked(false);
        btnPage->m_captureDelay->setChecked(false);
    }
}
void HasDevicePage::cameraFilter(int i){
    if(i == 1){//暖色
//        btnPage->filter->setText("warm");
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,V4L2_WHITE_BALANCE_MANUAL);
        camera->camera_set_contrl(WHITE_BALANCE,6500);
        camera->camera_set_contrl(BRIGHTNESS,filterBase.brightness);
        camera->camera_set_contrl(CONTRAST,filterBase.contrast);
        camera->camera_set_contrl(SATURATION,filterBase.saturation);
        camera->camera_set_contrl(HUE,filterBase.hue);
        camera->camera_set_contrl(SHARPNESS,filterBase.sharpness);
        camera->camera_set_contrl(BACKL_COMP,filterBase.backlComp);
    }else if(i == 2){//冷色
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,V4L2_WHITE_BALANCE_MANUAL);
        camera->camera_set_contrl(WHITE_BALANCE,1800);
        camera->camera_set_contrl(BRIGHTNESS,filterBase.brightness);
        camera->camera_set_contrl(CONTRAST,filterBase.contrast);
        camera->camera_set_contrl(SATURATION,filterBase.saturation);
        camera->camera_set_contrl(HUE,filterBase.hue);
        camera->camera_set_contrl(SHARPNESS,filterBase.sharpness);
        camera->camera_set_contrl(BACKL_COMP,filterBase.backlComp);
    }else if(i == 3){//黑白
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,V4L2_WHITE_BALANCE_MANUAL);
        camera->camera_set_contrl(WHITE_BALANCE,filterBase.whiteBalance);
        camera->camera_set_contrl(BRIGHTNESS,filterBase.brightness);
        camera->camera_set_contrl(CONTRAST,filterBase.contrast);
        camera->camera_set_contrl(SATURATION,0);
        camera->camera_set_contrl(HUE,filterBase.hue);
        camera->camera_set_contrl(SHARPNESS,filterBase.sharpness);
        camera->camera_set_contrl(BACKL_COMP,filterBase.backlComp);
    }else{//原图
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,V4L2_WHITE_BALANCE_MANUAL);
        camera->camera_set_contrl(WHITE_BALANCE,filterBase.whiteBalance);
        camera->camera_set_contrl(V4L2_CID_AUTO_WHITE_BALANCE,V4L2_WHITE_BALANCE_AUTO);
        camera->camera_set_contrl(BRIGHTNESS,filterBase.brightness);
        camera->camera_set_contrl(CONTRAST,filterBase.contrast);
        camera->camera_set_contrl(SATURATION,filterBase.saturation);
        camera->camera_set_contrl(HUE,filterBase.hue);
        camera->camera_set_contrl(SHARPNESS,filterBase.sharpness);
        camera->camera_set_contrl(BACKL_COMP,filterBase.backlComp);
    }
    widgetDisplay->buttom->hide();
    if(i==1 || i == 2 || i == 3){
        btnPage->filter->setChecked(true);
    }else{
        btnPage->filter->setChecked(false);
    }

}

void HasDevicePage::showNotify(QString message){
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
