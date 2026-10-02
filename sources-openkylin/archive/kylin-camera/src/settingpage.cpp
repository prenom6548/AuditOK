#include "settingpage.h"
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>
#include <QtCore/QCoreApplication>
#include <QErrorMessage>
#include "currentdeviceinfo.h"
#include "unistd.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QDir>
#include <QStandardItem>
#include <kylin_system/window_management.hpp>
#include "globalinstance.h"
#include <QKeyEvent>

bool SettingPage::press = false;
int SettingPage::dev_change = 1;
QSettings *SettingPage::resolution_setting = new QSettings(QCoreApplication::applicationDirPath() + "/.configure/kylin-camera/resolution_setting");
SettingPage::SettingPage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose,true);
    camera = new KylinCamera();
    settingPageTitle = new SettingPageTitle(this);
    settingPageTitle->setAccessibleName("kylin-camera_settingpage_SettingPageTitle_settingPageTitle");
    //辅助的功能类
    gsettingItem = new DefaultGetSet;
    gsettingItem->setAccessibleName("kylin-camera_settingpage_DefaultGetSet_gsettingItem");
    //设置存储位置行
    dirlab = new QLabel(this);//默认存储位置
    dirlab->setAccessibleName("kylin-camera_QLabel_dirlab");
    dir = new QLineEdit(this);//默认存储位置
    dir->setAccessibleName("kylin-camera_settingpage_QLineEdit_dir");
    dir->setReadOnly(true);
    dir->setCursor(Qt::IBeamCursor);
    dir->setMinimumWidth(230);
    currentDir = gsettingItem->getSavePath();
    browse = new QPushButton(this);//浏览
    browse->setAccessibleName("kylin-camera_QPushButton_browse");
    browse->setText(tr("browse"));//浏览
    dirlab->setText(tr("image path:")); //默认存储位置
    currentDirLab = new QLabel(this);
    currentDirLab->setAccessibleName("kylin-camera_QLabel_currentDirLab");

    //设置录像存储位置
    videoDirlab = new QLabel(this);//默认存储位置
    videoDirlab->setAccessibleName("kylin-camera_QLabel_videoDirlab");
    videoDir = new QLineEdit(this);//默认存储位置
    videoDir->setAccessibleName("kylin-camera_settingpage_QLineEdit_videoDir");
    videoDir->setReadOnly(true);
    videoDir->setCursor(Qt::IBeamCursor);
    videoDir->setMinimumWidth(230);
    currentVideoDir = gsettingItem->getVideoSavePath();
    videoBrowse = new QPushButton(this);//浏览
    videoBrowse->setAccessibleName("kylin-camera_QPushButoon_videoBrowse");
    videoBrowse->setText(tr("browse"));//浏览
    videoDirlab->setText(tr("video path:")); //默认存储位置
    currentVideoDirLab = new QLabel(this);
    currentVideoDirLab->setAccessibleName("kylin-camera_QLabel_currentVideoDirLab");

    //设置选用摄像头行
    cameraDevicelab = new QLabel(this);//默认摄像头
    cameraDevicelab->setAccessibleName("kylin-camera_QLabel_cameraDevicelab");
    cameraDevice = new QComboBox(this);//默认摄像头
    cameraDevice->setAccessibleName("kylin-camera_QComboBox_cameraDevice");
    cameraDevice->setMinimumWidth(300);
    cameraDevice->setDuplicatesEnabled(false);
    cameraDeviceLayout = new QHBoxLayout;
    cameraDevicelab->setText(tr("device:"));//默认摄像头
    currentCameraDev = new QLabel(this);
    currentCameraDev->setAccessibleName("kylin-camera_QLabel_currentCameraDev");
    cameraDeviceLayoutWidget = new QWidget(this);//默认摄像头水平布局
    cameraDeviceLayoutWidget->setAccessibleName("kylin-camera_settingpage_QWidget_cameraDeviceLayoutWidget");
    //设置照片格式行
    savePhotoFormatlab = new QLabel(this);//照片格式
    savePhotoFormatlab->setAccessibleName("kylin-camera_QLabel_savePhotoFormatlab");
    savePhotoFormat = new QComboBox(this);//照片格式
    savePhotoFormat->setAccessibleName("kylin-camera_QComboBox_savePhotoFormat");
    savePhotoFormat->setMinimumWidth(300);
    savePhotoFormat->addItem("jpg");
    savePhotoFormatLayout = new QHBoxLayout;
    savePhotoFormatlab->setText(tr("photo format:"));//照片格式
    //设置视频格式行
    savevideoFormatlab = new QLabel(this);//视频格式
    savevideoFormatlab->setAccessibleName("kylin-camera_QLabel_savevideoFormatlab");
    savevideoFormat = new QComboBox(this);//视频格式
    savevideoFormat->setAccessibleName("kylin-camera_QComboBox_savevideoFormat");
    savevideoFormat->setMinimumWidth(300);
    savevideoFormat->addItem("mp4");
    savevideoFormatLayout = new QHBoxLayout;
    savevideoFormatlab->setText(tr("video format:"));//视频格式
    current_savevideoFormat = new QLabel(this);
    current_savevideoFormat->setAccessibleName("kylin-camera_QLabel_current_savevideoFormat");
    //设置拍摄分辨率行
    scalelab = new QLabel(this);//拍摄分辨率
    scalelab->setAccessibleName("kylin-camera_QLabel_scalelab");
    scale = new QComboBox(this);//拍摄分辨率
    scale->setAccessibleName("kylin-camera_QComboBox_scale");
    scale->setMinimumWidth(300);
    scaleLayout = new QHBoxLayout;
    scalelab->setText(tr("scale:"));//拍摄分辨率
    //设置选定/取消行
    confirm = new QPushButton(this);
    confirm->setAccessibleName("kylin-camera_QPushButton_confirm");
    cancel = new QPushButton(this);
    cancel->setAccessibleName("kylin-camera_QPushButton_cancel");
    btnLayout = new QHBoxLayout;
    confirm->setText(tr("confirm"));//确认
    confirm->setProperty("isImportant", true);
    cancel->setText(tr("cancel"));//取消
    gridLayout = new QGridLayout;

    gridLayout->addWidget(cameraDevicelab,0,0,1,1,Qt::AlignLeft);
    gridLayout->addWidget(cameraDevice,0,1,1,3);

    gridLayout->addWidget(dirlab,1,0,1,1,Qt::AlignLeft);
    gridLayout->addWidget(dir,1,1,1,2);
    gridLayout->addWidget(browse,1,3,1,1,Qt::AlignRight);

    gridLayout->addWidget(videoDirlab,2,0,1,1,Qt::AlignLeft);
    gridLayout->addWidget(videoDir,2,1,1,2);
    gridLayout->addWidget(videoBrowse,2,3,1,1,Qt::AlignRight);

    gridLayout->addWidget(savePhotoFormatlab,3,0,1,1,Qt::AlignLeft);
    gridLayout->addWidget(savePhotoFormat,3,1,1,3);
    gridLayout->addWidget(savevideoFormatlab,4,0,1,1,Qt::AlignLeft);
    gridLayout->addWidget(savevideoFormat,4,1,1,3);
    gridLayout->addWidget(scalelab,5,0,1,1,Qt::AlignLeft);
    gridLayout->addWidget(scale,5,1,1,3);
    gridLayout->addWidget(confirm,6,3,1,1,Qt::AlignRight);
    gridLayout->addWidget(cancel,6,2,1,1,Qt::AlignRight);
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    gridLayout->setMargin(20);
    gridLayout->setColumnStretch(0,3);
    gridLayout->setColumnStretch(1,2);
    gridLayout->setColumnStretch(2,2);
    gridLayout->setColumnStretch(3,2);

    settingPageLayout = new QVBoxLayout;
    settingPageLayout->addWidget(settingPageTitle);
    settingPageLayout->addLayout(gridLayout);
    settingPageLayout->addStretch(1);
    settingPageLayout->setMargin(5);
    this->setLayout(settingPageLayout);

    nowThemeStyle = gsettingItem->getStyleName();
    QPalette pal(this->palette());
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    kdk::kabase::WindowManagement::setWindowMotifHint(this->winId());

    change_item();
    initConnect();
#ifdef __V10__
    this->setStyleSheet( \
            "background: #1C1C1E;"
            "box-shadow: 0px 3px 16px rgba(0, 0, 0, 0.5);"
            "opacity: 1;");

#endif

}

SettingPage::~SettingPage(){
    settingPageTitle->deleteLater();
    delete gsettingItem;
}
void SettingPage::noDeviceSlot(int code){
    Q_UNUSED(code);
    cameraDevice->clear();
    CurrentDeviceInfo::available_size.clear();
    scale->clear();
}
void SettingPage::Geometry(QPoint *pos){
    this->setGeometry(pos->x(),pos->y(),0,0);
}

void SettingPage::initConnect(){
    connect(gsettingItem,&DefaultGetSet::sigThemeChange, this,&SettingPage::dealSystemGsettingChange);
    connect(browse,&QPushButton::clicked,this,&SettingPage::dir_click);
    connect(videoBrowse,&QPushButton::clicked,this,&SettingPage::video_dir_click);
    connect(confirm,&QPushButton::clicked,this,&SettingPage::confirm_click);
    connect(cancel,&QPushButton::clicked,this,&SettingPage::cancel_click);
    connect(cameraDevice,SIGNAL(currentIndexChanged(int)),this,SLOT(updateResolution(int)));
    connect(GlobalInstance::getInstance(),&GlobalInstance::settingPageSig,this,&SettingPage::change_item);
    connect(GlobalInstance::getInstance(),&GlobalInstance::displayCameraErrorSig,this,&SettingPage::noDeviceSlot);
    connect(GlobalInstance::getInstance(),&GlobalInstance::settingOptionsOn,this,&SettingPage::settingOptionsOnSlot);

}
void SettingPage::updateResolution(int x){
    Q_UNUSED(x);
    if(cameraDevice->currentText() == ""){
        return;
    }
    QString devname = sysNameArrEasyNamy[cameraDevice->currentText()];
    QString str2 = fullNameArrSysName[devname];
    cameraDevice->setToolTip(str2);
    char *in_device = devname.toLocal8Bit().data();
    V4l2DeviceInfo device_info;
    memset(&device_info,0X00,sizeof(V4l2DeviceInfo));
    memcpy(device_info.dev_path, in_device, strlen(in_device)+1);
    camera->enum_device(&device_info);
    CurrentDeviceInfo::available_size_tmp.clear();

    int formatFlagTmp = 0;
    uint available_formatTmp = 0;

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
          continue;
      }
    }
    if(available_formatTmp == 0){//该摄像头不支持MJPG/YUYV编码，设置为默认编码
        available_formatTmp = device_info.fmt_supported[formatFlagTmp].fmt.pixelformat;
    }

    TypePixFormats fmt = device_info.fmt_supported[formatFlagTmp];
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
                CurrentDeviceInfo::available_size_tmp.push_back(cell);
                qDebug() << "showMeg" << cell.available_size.first << cell.available_size.second << cell.fps;
            }else{
                continue;
            }
        }
    }

    CurrentDeviceInfo::sort_resolution_tmp();
    QString Scales = scale->currentText();
    scale->clear();
    scalelist.clear();
    int scaleCount = 0;
    for(auto p : CurrentDeviceInfo::available_size_tmp){
        QString str;
        str = QString::number(p.available_size.first) + "x" + QString::number(p.available_size.second);
        qDebug () << str;
        scale->addItem(str);
        scalelist.append(str);
        scaleCount++;
    }

    if(scalelist.contains(Scales)){
        scale->setCurrentText(Scales);
    }else{
        scale->setCurrentIndex(0);
    }
}



void SettingPage::close(){
    this->window()->close();
}

//当设备有变化时更新设备列表，
//加入设备到device选项中
void SettingPage::change_item(){
    qDebug()<<"列表数量变化了！";

    dir->setText(gsettingItem->getSavePath());
    dir->setToolTip(gsettingItem->getSavePath());
    //添加摄像头设备
   //由于读取设备文件需要时间,这里开一个线程等待设备读取
    elideCameraText();
    updateResolution(1);
    updateShow();
}
QString SettingPage::dirNameELideText(QString str){
    QFontMetrics fm = this->fontMetrics();
    QString showText;
    int dif = fm.width(str) - dir->width()+20;
    if (dif > 0) {
        showText = fm.elidedText(str, Qt::ElideLeft, dir->width()-20);
    }else{
        showText = str;
    }
    return showText;
}

//设置存储路径
void SettingPage::dir_click(){
    currentDir = QFileDialog::getExistingDirectory(this,tr("Select the directory"),"/home",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(QDir(currentDir).exists() && currentDir != ""){//"/media为u盘所在目录"
        if(!currentDir.contains(str,Qt::CaseSensitive) && !currentDir.contains("/media", Qt::CaseSensitive)){
            QMessageBox mes;
            mes.warning(this,tr("error"),tr("The directory does not have write permissions, select the user directory to store the files ."));
            mes.move(this->geometry().center()-mes.rect().center());
            currentDir = gsettingItem->getSavePath();
            return;
        }
        currentDir.append('/');
        QString showText = dirNameELideText(currentDir);
        dir->setText(showText);
        dir->setToolTip(currentDir);
    }
    else if(currentDir == ""){
        currentDir = gsettingItem->getSavePath();
        QString showText = dirNameELideText(currentDir);
        dir->setText(showText);
        dir->setToolTip(currentDir);
    }
}
//设置存储路径
void SettingPage::video_dir_click(){
    currentVideoDir = QFileDialog::getExistingDirectory(this,tr("Select the directory"),"/home",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if(QDir(currentVideoDir).exists() && currentVideoDir != ""){ //"/media为u盘所在目录"
        if(!currentVideoDir.contains(str,Qt::CaseSensitive) && !currentDir.contains("/media", Qt::CaseSensitive)){
            QMessageBox mes;
            mes.warning(this,tr("error"),tr("The directory does not have write permissions, select the user directory to store the files ."));
            mes.move(this->geometry().center()-mes.rect().center());
            currentVideoDir = gsettingItem->getVideoSavePath();
            return;
        }
        currentVideoDir.append('/');
        QString showText = dirNameELideText(currentVideoDir);
        videoDir->setText(showText);
        videoDir->setToolTip(currentVideoDir);
    }
    else if(currentVideoDir == ""){
        currentVideoDir = gsettingItem->getVideoSavePath();
        QString showText = dirNameELideText(currentVideoDir);
        videoDir->setText(showText);
        videoDir->setToolTip(currentVideoDir);
    }
}
void SettingPage::isChangeDevice(bool success){
    Q_UNUSED(success);
}
void SettingPage::settingOptionsOnSlot(bool on){
    settingOnFlag = on;
}

void SettingPage::confirm_click(){
    //设置摄像头
    //SettingPage
    if(Button::video_start || !settingOnFlag) {
        cancel_click();
        QMessageBox mes;
        if(Button::video_start){
            mes.warning(this,tr("waring"),tr("The settings cannot be changed during recording"));
        }else{
            mes.warning(this,tr("waring"),tr("The settings cannot be changed,please try again later"));
        }
        mes.move(this->geometry().center()-mes.rect().center());
        return;
    }
    //设置存储路径
    if(currentDir != gsettingItem->getSavePath()){
        QString showText = dirNameELideText(currentDir);
        dir->setText(showText);
        dir->setToolTip(currentDir);
        gsettingItem->setSavePath(currentDir); //将路径储存到配置文件里
        Q_EMIT GlobalInstance::getInstance()->dir_change();
        gsettingItem->addChangeSavePathSDKPoint();
    }

    if(currentVideoDir != gsettingItem->getVideoSavePath()){
        QString showText = dirNameELideText(currentVideoDir);
        videoDir->setText(showText);
        videoDir->setToolTip(currentVideoDir);
        gsettingItem->setVideoSavePath(currentVideoDir); //将路径储存到配置文件里
        Q_EMIT GlobalInstance::getInstance()->video_dir_change();
        gsettingItem->addChangeSavePathSDKPoint();
    }

    //切换设备
    bool needChangeDev = false;
    QString newCamera;
    if(HasDevicePage::has_device && CurrentDeviceInfo::current_indevice != sysNameArrEasyNamy[cameraDevice->currentText()]){
        QString text = cameraDevice->currentText();
        newCamera = sysNameArrEasyNamy[text];
        dev_change = 1;
        gsettingItem->addCameraSelectionSDKPoint();
        needChangeDev = true;
        Q_EMIT GlobalInstance::getInstance()->sigCameraInfoSet(newCamera.toLocal8Bit().data(), "");
    }
    //切换分辨率
    int index = scale->currentIndex();
    if(HasDevicePage::has_device && gsettingItem->getScaleIndex() != index && index != -1){
        CurrentDeviceInfo::current_size.available_size.first= CurrentDeviceInfo::available_size[index].available_size.first;
        CurrentDeviceInfo::current_size.available_size.second= CurrentDeviceInfo::available_size[index].available_size.second;
        CurrentDeviceInfo::current_size.fps = CurrentDeviceInfo::available_size[index].fps;
        gsettingItem->setScaleIndex(index);
        gsettingItem->setScale(CurrentDeviceInfo::available_size[index].available_size.first, CurrentDeviceInfo::available_size[index].available_size.second);
        Q_EMIT GlobalInstance::getInstance()->change_resolutions(CurrentDeviceInfo::current_size.available_size); //获取分辨率
        gsettingItem->addResolutionSelectionSDKPoint();
    }else if(needChangeDev == true){
        Q_EMIT GlobalInstance::getInstance()->change_dev(newCamera.toLocal8Bit().data());
    }
    //选择存储图片格式
    if(HasDevicePage::has_device && gsettingItem->getPhotoFormat()!= savePhotoFormat->currentIndex()){
        gsettingItem->setPhotoFormat(savePhotoFormat->currentIndex());
        gsettingItem->addPictureFormatSDKPoint();
    }
    //选择视频存储格式
    if(HasDevicePage::has_device && gsettingItem->getVideoFormat() != savevideoFormat->currentIndex()){
        gsettingItem->setVideoFormat(savevideoFormat->currentIndex());
        gsettingItem->addVideoFormatSDKPoint();
    }
    cancel_click();
}


void SettingPage::cancel_click(){
    this->window()->close();
}


void SettingPage::mousePressEvent(QMouseEvent *event){
    press = true;
    if(event->button() == Qt::LeftButton){
        m_start = event->globalPos();
    }
}


void SettingPage::mouseMoveEvent(QMouseEvent *event)
{
    if(!press) return;
    this->move(event->globalPos() + this->geometry().topLeft() - m_start);
    m_start = event->globalPos();
}

void SettingPage::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标左键释放
    if (event->button() == Qt::LeftButton) {
        // 记录鼠标状态
        press = false;
    }
}

//更新设备信息
void SettingPage::updateShow(){
    //下面几项设置都恢复到更改前
    /*默认存储文件夹*/
    QString str = gsettingItem->getSavePath(); //将路径储存到配置文件里
    QString showText = dirNameELideText(str);
    dir->setText(showText);
    dir->setToolTip(str);

    QString str_video = gsettingItem->getVideoSavePath(); //将路径储存到配置文件里
    QString showText_video = dirNameELideText(str_video);
    videoDir->setText(showText_video);
    videoDir->setToolTip(str_video);

    /*设备*/
    for(int i = 0;i < cameraDevice->count();i++){
        QString val = cameraDevice->itemText(i);
        if(sysNameArrEasyNamy[val] == CurrentDeviceInfo::current_indevice){
            cameraDevice->setCurrentIndex(i);
            QString str = sysNameArrEasyNamy[cameraDevice->currentText()];
            QString str2 = fullNameArrSysName[str];
            cameraDevice->setToolTip(str2);
            break;
        }
    }
    /*分辨率*/
    scale->setCurrentIndex(gsettingItem->getScaleIndex());
    /*图片格式*/
    savePhotoFormat->setCurrentIndex(gsettingItem->getPhotoFormat());
    /*视频格式*/
    savevideoFormat->setCurrentIndex(gsettingItem->getVideoFormat());
}

//冗余代码 需优化
void SettingPage::dealSystemGsettingChange(const QString themeStyle){
    QPalette pal(this->palette());
    nowThemeStyle = themeStyle;
    //深色模式下
    if (themeStyle == "ukui-dark" || themeStyle == "ukui-black"){
        //设置背景色黑色
        pal.setColor(QPalette::Background,Qt::black);
    }
    else{
        //设置背景色白色
        pal.setColor(QPalette::Background,Qt::white);
    }
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}
void SettingPage::elideCameraText(){
    int index = cameraDevice->currentIndex();
    int scaleIndex = scale->currentIndex();
    fullNameArrSysName.clear();
    sysNameArrEasyNamy.clear();
    cameraDevice->clear();
    /*遍历当前设备列表，更新cameraDevice的显示*/
    if(model != nullptr){
        delete model;
        model = nullptr;
    }
    model = new QStandardItemModel;
    int cur_index;
    for(int i =0; i < CurrentDeviceInfo::current_device_list.size();i++){
        QCameraInfo cameraInfo = CurrentDeviceInfo::current_device_list.at(i);
        //1. 获取完整设备名
        QString device_name;
        device_name = cameraInfo.description() + cameraInfo.deviceName();
        //2. 完成完整设备名 到 系统设备名的映射
        fullNameArrSysName[cameraInfo.deviceName()] = device_name;
        //3. 完成系统设备名 到 完整设备名的映射
        QFontMetrics fm = this->fontMetrics();
        int dif = fm.width(device_name) - cameraDevice->width()+50;
        if (dif > 0) {
            QString showText = fm.elidedText(device_name, Qt::ElideMiddle, cameraDevice->width()-50);
            sysNameArrEasyNamy[showText] = cameraInfo.deviceName();
            cameraDevice->addItem(showText);
            QStandardItem *item = new QStandardItem(showText);
            item->setToolTip(device_name);
            model->appendRow(item);

            //4. 确定cameraDevice的显示
            //QComboBox使用setModel后自动选中第一项填充当前ComboBox的显示，使用setCurrentText没用，必须用setCurrentIndex更新一下
            if(device_name.contains(CurrentDeviceInfo::current_indevice)){
                cur_index = i;
                cameraDevice->setCurrentIndex(cur_index);
                cameraDevice->setToolTip(device_name);
            }

        }else{
            sysNameArrEasyNamy[device_name] = cameraInfo.deviceName();
           // cameraDevice->addItem(device_name);
            QStandardItem *item = new QStandardItem(device_name);
            item->setToolTip(device_name);
            model->appendRow(item);
            //4. 确定cameraDevice的显示
            //QComboBox使用setModel后自动选中第一项填充当前ComboBox的显示，使用setCurrentText没用，必须用setCurrentIndex更新一下
            if(device_name.contains(CurrentDeviceInfo::current_indevice)){
                cur_index = i;
                cameraDevice->setCurrentIndex(cur_index);
                cameraDevice->setToolTip(device_name);                
            }
        }
    }
    cameraDevice->setModel(model);
    //QComboBox使用setModel后自动选中第一项填充当前ComboBox的显示，使用setCurrentText没用，必须用setCurrentIndex更新一下
    cameraDevice->setCurrentIndex(cur_index);

    if(changeEventFlag){
        changeEventFlag = false;
        cameraDevice->setCurrentIndex(index);
        QString str = sysNameArrEasyNamy[cameraDevice->currentText()];
        QString str2 = fullNameArrSysName[str];
        cameraDevice->setToolTip(str2);

        scale->setCurrentIndex(scaleIndex);
        return;
    }


}
void SettingPage::changeEvent(QEvent* event) {
    if(event->type() != QEvent::FontChange){
        return;
    }
    currentDir = gsettingItem->getSavePath();
    QString showText = dirNameELideText(currentDir);
    dir->setText(showText);
    dir->setToolTip(currentDir);
    changeEventFlag = true;
    elideCameraText();
    QWidget::changeEvent(event);
}

//弹窗显示后按Esc关闭弹窗
void SettingPage::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Escape){
        this->window()->close();
    }
}
