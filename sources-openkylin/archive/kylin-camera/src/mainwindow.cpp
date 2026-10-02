#include "mainwindow.h"
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include <QList>
#include <QDesktopWidget>
#include <QThread>
#include <QPoint>
#include <QTime>
#include "customstyle.h"
#include <QDesktopWidget>
#include <sys/file.h>
#include "util/unix_signals.h"
#include "util/xatom-helper.h"
#include <sys/file.h>
#include <fcntl.h>
#include <fstream>
#include <kylin_system/window_management.hpp>
#include <kylin_system/user_manual.hpp>
#include <QMessageBox>
#include "globalinstance.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include "windowmanager/windowmanager.h"
#include "ukuiwindowhelper.h"
#define COUNTDOWN_TIME 3 //倒计时时间
#define TRANS_PATH  "/usr/share/kylin-camera/translations/"
#define picture_width 270  //相册宽度

QString user; // 用户名
QString photoFormat;
QString videoFormat;

static const QString FM1_DEST = "org.freedesktop.FileManager1";
static const QString FM1_PATH = "/org/freedesktop/FileManager1";
static const QString FM1_IFACE = "org.freedesktop.FileManager1.ShowItems";

MainWindow::MainWindow(QStringList strList,QWidget *parent)
   : CustomWidget(parent),m_dbusHanler(nullptr)
{
    Q_UNUSED(strList);

    if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
        windowHelper->removeTitleBar();
    } else {
        kdk::UkuiStyleHelper::self()->removeHeader(this);
    }
    //辅助的功能类
    gsettingItem = new DefaultGetSet;
    gsettingItem->setAccessibleName("kylin-camera_mainwindow_DefaultGetSet_gsettingItem");
    initSetting();

#ifdef __V10__
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif
    //摄像头的主窗体
    cameraTopPage = new CameraTopPage(this);
    cameraTopPage->setAccessibleName("kylin-camera_mainwindow_CameraTopPage_cameraTopPage");
    cameraTopPage->setMinimumSize(900,518);
    modeSwitch();
    rotateChangeInfoObj = new RotateChangeInfo(this);
    bool isPCMode = rotateChangeInfoObj->getCurrentMode();
    QPair<int,int> screenScale = rotateChangeInfoObj->getCurrentScale();
    slotIntelHVModeChanged(isPCMode,screenScale.first,screenScale.second);
    setWid = new Setting(this, parent);
    setWid->setAccessibleName("kylin-camera_mainwindow_Setting_setWid");
    setWid->hide();
    windowButtonBar()->menuButton()->setMenu(setWid->menu);
    //相册浏览的页面，但是尚未启用
    viewpage = new PictureViewPage(this);
    viewpage->setAccessibleName("kylin-camera_mainwindow_PictureViewPage_viewpage");
    viewpage->setFixedWidth(picture_width);
    viewpage->hide();
    pictureLayout = new QGridLayout;
    //总体页面布局
    mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(cameraTopPage);
    baseBar()->setLayout(mainLayout);
    //应用居中
    this->update();
#ifdef __V10__
    this->setStyleSheet("MainWindow{background-color:#000000;}");
#endif
    initConnect();
    initDbus();
    resize(minimumSize());
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, this, [=](const kdk::WindowId& window_id){
        if (getpid() == kdk::WindowManager::getPid(window_id))
        {
            m_windowId = window_id;
        }
    });
}
kdk::WindowId MainWindow::windowId()
{
    return m_windowId;
}
bool MainWindow::videoCodeStop(){
    if(coadState != nullptr){
           delete coadState;
           coadState = nullptr;
       }
       cameraTopPage->hasDevicePage->camera->camera_stop_record(); //仅触发停止
//       m_dbus->unInhibit();
       return true;
}
MainWindow::~MainWindow()
{
    qDebug() << "kill them all";
    if(cameraTopPage->hasDevicePage->camera && Button::video_start){
        videoCodeStop();
    }
    while(1){
        // 如果没插入摄像头，直接break，避免传出空指针
        if(cameraTopPage->hasDevicePage->camera == nullptr){
            break;
        }
        int state = cameraTopPage->hasDevicePage->camera->camera_get_state(CAMERA_ENCODE);
        qDebug() << "camera state:" <<state<< " zyz";
        if(state == CAMERA_ENCODE_STOP) break;
        else{
            QThread::msleep(100);
            continue;
        }
    }
    delete gsettingItem;
//    delete m_dbus;
    proc->deleteLater();
    viewpage->deleteLater();
    cameraTopPage->deleteLater();

    // 将 exit(0) 放入下一个事件循环，确保底层库对象释放完毕
    QMetaObject::invokeMethod(this, &MainWindow::exitApp, Qt::QueuedConnection);
}

void MainWindow::exitApp()
{
    exit(0);
}
void MainWindow::initSetting(){
    m_currentUser = qgetenv("USER");
    qDebug() << "USER =" << qgetenv("USER");
    if (m_currentUser.isEmpty()){
        m_currentUser = qgetenv("USERNAME");
    }
    gsettingItem->setUser(m_currentUser);
    proc = new QProcess();  //启动一个进程
    //设置标题栏窗口显示
    setWidgetName(tr("Camera"));
    imagePath = gsettingItem->getSavePath();
    if(imagePath.compare("") == 0){
        imagePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/";
        QDir dir(imagePath);
        if (!dir.exists()) {
            dir.mkpath(imagePath);
        }
        gsettingItem->setSavePath(imagePath);
        qDebug()<<"image path:"<<imagePath;
    }

    videoPath = gsettingItem->getVideoSavePath();
    if(videoPath.compare("") == 0){
        videoPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/";
        QDir dir(videoPath);
        if (!dir.exists()) {
            dir.mkpath(videoPath);
        }
        gsettingItem->setVideoSavePath(videoPath);
        qDebug()<<"video path:"<<videoPath;
    }
    //从配置文件读取，默认为3
    dead_time_sec = COUNTDOWN_TIME;

}

//总体页面布局，设置公共组件连接槽函数
void MainWindow ::initConnect(){
    connect(cameraTopPage->hasDevicePage->btnPage,&Button::winHeight,this,&MainWindow::modeSwitch);
    connect(this,&MainWindow::winHeight,cameraTopPage->hasDevicePage,&HasDevicePage::videoDisplayResize);
    //设置事件
    connect(GlobalInstance::getInstance(),&GlobalInstance::dir_change,this,&MainWindow::save_dir_change);
    connect(GlobalInstance::getInstance(),&GlobalInstance::dir_change,cameraTopPage->hasDevicePage->dirChange,&DirChange::dirChangeHandle);
    connect(GlobalInstance::getInstance(),&GlobalInstance::video_dir_change,cameraTopPage->hasDevicePage->dirChange,&DirChange::videoDirChangeHandle);
    connect(GlobalInstance::getInstance(),&GlobalInstance::change_resolutions,cameraTopPage->hasDevicePage,&HasDevicePage::change_resolution);
    connect(GlobalInstance::getInstance(),&GlobalInstance::change_dev,cameraTopPage->hasDevicePage,&HasDevicePage::change_device);
    connect(GlobalInstance::getInstance(),&GlobalInstance::sigCameraInfoSet,cameraTopPage->hasDevicePage,&HasDevicePage::cameraInfoSet);
    connect(this,&MainWindow::clickPhotoSignal,cameraTopPage->hasDevicePage,&HasDevicePage::clickPhoto);
    connect(rotateChangeInfoObj,&RotateChangeInfo::rotationChanged,this,&MainWindow::slotIntelHVModeChanged);
    connect(GlobalInstance::getInstance(),&GlobalInstance::optionStatus, [this](bool canUse) {
             windowButtonBar()->menuButton()->setEnabled(canUse);
        });

}
void MainWindow::initDbus()
{
    // connect signals
    m_dbusHanler = new DaemonIpcDbus;
    connect(m_dbusHanler, SIGNAL(request(QStringList)), this,
        SLOT(kylinCameraDbusRequest(QStringList)));
}
//lhl debug 无保存路径处理
void MainWindow::noPathHandle()
{
    imagePath = QFileDialog::getExistingDirectory(this,"设置存储路径","/home", //current_dir
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    videoPath = QFileDialog::getExistingDirectory(this,"设置存储路径","/home", //current_dir
                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    qDebug() << "current_dir:" << imagePath;
    qDebug() << "current_dir:" << videoPath;
}

//lhl debug 增加提示设置路径消息提示框
void MainWindow::settingPathPrompt()
{
    QMessageBox mes;
    mes.warning(this,tr("waring"),tr("path does not exist, please set storage path"));
    mes.move(this->geometry().center()-mes.rect().center());
}

//命令行输入
int MainWindow::kylinCameraDbusRequest(QStringList command)
{
    qDebug() << "command :";
    for (QString s : command)
        qDebug() << s;

    if(command.size()==0 || command.size()>2){
        qDebug() << "Command size is " <<command.size();
        return 1;
    }
    QString cmd = command.at(0);
    if(cmd.compare("--raise") == 0) {
        //Not right...
        stayTop();
    }else if(cmd.compare("--video")==0||cmd.compare("-v")==0) {
        //在录像模式中
        if(cameraTopPage->hasDevicePage->btnPage->video->isChecked()){
            cameraTopPage->hasDevicePage->btnPage->cheese_video->click();
            qDebug() << "开始录像";
        }
        else{
            qDebug() << "当前为拍照模式请切换到录像模式";
        }
    }else if(cmd.compare("--cheese")==0||cmd.compare("-c")==0) {
        if(cameraTopPage->hasDevicePage->btnPage->capture->isChecked()){
            cameraTopPage->hasDevicePage->clickPhoto();
            qDebug() << "拍照";
        }
        else{
            //Pop up...
            qDebug() << "当前为录像模式请切换到拍照模式";
        }
    }else if(cmd.compare("--stop")==0||cmd.compare("-s")==0){
        //符合这个按钮的要求
        if(Button::video_start){
            cameraTopPage->hasDevicePage->btnPage->cheese_stop->click();
            qDebug() << "结束录像";
        }else{
            qDebug() << "非法操作";
        }
    }else if(cmd.compare("--video-mode")==0||cmd.compare("-vm")==0){
        cameraTopPage->hasDevicePage->btnPage->video->click();
        qDebug() << "切换为录像模式";
    }else if(cmd.compare("--capture-mode")==0||cmd.compare("-cm")==0){
        cameraTopPage->hasDevicePage->btnPage->capture->click();
        qDebug() << "切换为拍照模式";
    }else if(cmd.compare("--mirror")==0||cmd.compare("-m")==0){
        //if in
    }else if(cmd.compare("--delay")==0||cmd.compare("-d")==0){

    }else if(cmd.compare("--save-path")==0||cmd.compare("-ssd")==0){
        if(command.size()!=2){
            qDebug() << "Save path command parameter error.";
            return -1;
        }
        QFileInfo dir(command.at(1));
        if(dir.isDir() && dir.isWritable()){
            imagePath = dir.absolutePath();
            qDebug() << imagePath;
            setWid->setPage->currentDirLab->setText(imagePath);
            setWid->setPage->currentDirLab->setToolTip(imagePath);
            imagePath = imagePath + "/";
            gsettingItem->setSavePath(imagePath);
            qDebug() << "设置存储路径";
        }else{
            qDebug() << "Wrong path...";
            //popup
        }
    } else if(cmd.compare("--quit") == 0 || cmd.compare("-q") == 0 ){
        //Stop record and then exit...
        this->deleteLater();
        //exit(0);
    } else {
        qDebug() << "Unknown command "<<command;
    }
    return 0;
}




void MainWindow::keyPressEvent(QKeyEvent *event){     //作用 何时触发
    if(event->key() == Qt::Key_F1){
        userManualTest();
    }
}
void MainWindow::userManualTest(void)
{
    kdk::kabase::UserManualManagement userManualTest;
    if (!userManualTest.callUserManual(kdk::kabase::AppName::KylinCamera)) {
        qCritical() << "user manual call fail!";
    }
}

void MainWindow::resizeEvent(QResizeEvent *event){    //作用 何时触发
    modeSwitch();
    CustomWidget::resizeEvent(event);
}


//鼠标按下事件,收起设置菜单
void MainWindow::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::RightButton)
        return;

    press = true;
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    qDebug() << pButton;
    if(event->button() == Qt::LeftButton){
        m_start = event->globalPos();
    }

}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(!press || event->button() == Qt::RightButton){
        return;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标左键释放
    if (event->button() == Qt::LeftButton) {
        // 记录鼠标状态
        press = false;
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        if(this->windowState() == Qt::WindowMaximized){
            this->window()->showNormal();
        }
        else if(this->windowState() == Qt::WindowNoState){
            this->window()->showMaximized();
        }
    }
}

//应用置顶  debug 不可用
void MainWindow::stayTop()
{
    qDebug()<<"Set mainwindow to toplevel.";
    if(!this->isActiveWindow())
    {
        this->hide();
        this->show();
    }
    this->show();
    this->raise();
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive) || QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        kdk::WindowManager::activateWindow(m_windowId);
    }else{
        this->activateWindow();
    }
}

//输入命令
void MainWindow::startCommend(QString cmd)
{
    proc->startDetached(cmd);
}
//更改保存路径
void MainWindow::save_dir_change(){
    QString tmp = gsettingItem->getFileName();
    cameraTopPage->hasDevicePage->btnPage->pictureWrap->setStatusTip(tmp);
}

bool MainWindow::singleInstance(QStringList strList)
{
    //argv[0]: instance
    //argv[1]: command
    //argv[2]: parameter
    //Check command...
    QString app = strList.at(0);
    if(strList.size() > 1) {
        QString command = strList.at(1);
        if(command.compare("--help")==0 || command.compare("-h")==0 ){
            qDebug()<<"\nkylin-camera [cmd]\n"
                      "-c --cheese  拍照\n"
                      "-v --video   录像\n"
                      "-s --stop    结束录像\n"
                      "-cm --capture-mode  切换拍照模式\n"
                      "-vm -video-mode  切换录像模式\n"
                      "-ssd --save-path [save directory]设置保存目录\n"
                      "-q -quit 退出";
            return false;
        }

        if (command.compare("--help") != 0 && command.compare("-h") != 0
            && command.compare("--cheese") != 0 && command.compare("-c") != 0
            && command.compare("--video") != 0 && command.compare("-v") != 0
            && command.compare("--save-path") != 0
            && command.compare("-ssd") != 0
            && command.compare("--capture-mode") != 0
            && command.compare("-cm") != 0
            && command.compare("--video-mode") != 0
            && command.compare("-vm") != 0 && command.compare("--quit") != 0
            && command.compare("-q") != 0 && command.compare("--stop") != 0
            && command.compare("-s") != 0) {
            qDebug() << "参数不合法,使用 kylin-camera --help 获取帮助.";
            return false;
        }
        //Check parameter
        QDBusInterface interface( "org.ukui.kylin_camera", "/org/ukui/kylin_camera","org.ukui.kylin_camera.request", QDBusConnection::sessionBus());
        if(strList.size() == 2){
            qDebug() << "Command:"<<strList.at(1);
            interface.call( "request", command);
        }
        else if(strList.size() == 3){
            QString param = strList.at(2);
            qDebug() << "Command:"<<command << " : "<<param;
            interface.call( "request", command, param);
        }
        return false;
    } else {
        //Check instance
        //modify lock path to app instance
        int fd = open(TRANS_PATH,O_RDONLY);
        flock(fd,LOCK_UN);
        if(fd == -1)
            return false;
        int ret = flock(fd,LOCK_EX | LOCK_NB);
        if(ret == -1){
            qDebug() << "进入dbus";
            QDBusInterface interface( "org.ukui.kylin_camera", "/org/ukui/kylin_camera","org.ukui.kylin_camera.request", QDBusConnection::sessionBus());
            interface.call("request", "--raise");
            if (started())
            {
                QMessageBox mes;
                mes.warning(this,tr("waring"),tr("The camera is turned on and cannot be started again."));
                mes.move(this->geometry().center()-mes.rect().center());
                //QMessageBox::warning(this,tr("waring"),tr("The camera is turned on and cannot be started again."));
                exit(0);
            }
            return false;
        }
    }
    return true;
}

void MainWindow::inhibit(QString what,QString why)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({what,
                                       "Kylin Camera",
                                       why,
                                       QStringLiteral("block")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    qDebug() << "xx" << reply.value().isValid() ;
    reply.value().swap(m_inhibitFileDescriptor);
}

void MainWindow::unInhibit()
{
    qDebug() <<"yy" <<m_inhibitFileDescriptor.isValid();
    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}
//单例时调用 在kylin-camera已启动，再次启动给提示
bool MainWindow::started()
{
    QString comm;
    QDir dir("/proc/");
    QFileInfo file;
    std::ifstream in;
    std::string s;
    for(int i = 0; i < dir.entryInfoList().size(); i++)
    {
        QFileInfo f = dir.entryInfoList().at(i);
        if (f.baseName() == QString("%1").arg(getpid())|| f.baseName() == "self" ||f.baseName() == "thread-self")
            continue;
        //QString userId = QString("%1").arg(getuid());
        comm = QString("/proc/%1/comm").arg(f.baseName());
        file.setFile(comm);
        if (file.exists())
        {
            in.open(file.absoluteFilePath().toStdString());
            if (in.is_open())
            {
                getline(in, s);
                comm = QString::fromStdString(s);
                if (comm == "kylin-camera"){
                    in.close();
                    return true;
                }

            }
            in.close();
        }
    }
    return false;
}
void MainWindow::modeSwitch(){
    CurrentDeviceInfo::winHeight = cameraTopPage->height();
    CurrentDeviceInfo::winWidth = cameraTopPage->width();
    //1. 判断当前摄像头设备在不在
    struct stat s_stat;
    memset(&s_stat,0x00,sizeof(struct stat));
    int ret = stat(CurrentDeviceInfo::current_indevice,&s_stat);  //获取文件信息
    if(ret == -1){
        return;
    }else{
       Q_EMIT winHeight();
    }
}
void MainWindow::handleIconClickedSub(){
    kdk::kabase::WindowManagement::setWindowMotifHint(this->winId());
    if(!this->isVisible()){
        QDesktopWidget* m = QApplication::desktop();
        QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
        int desk_x = desk_rect.width();
        int desk_y = desk_rect.height();
        int x = this->width();
        int y = this->height();
        this->move(desk_x/2-x/2+desk_rect.left(),desk_y/2-y/2+desk_rect.top());
    }
    this->showNormal();
    this->raise();
    this->activateWindow();
}

void MainWindow::slotIntelHVModeChanged(bool isPCMode,int width,int height){
    if(isPCMode){
        this->windowButtonBar()->maximumButton()->show();
    }else{
        this->windowButtonBar()->maximumButton()->hide();
    }
}
