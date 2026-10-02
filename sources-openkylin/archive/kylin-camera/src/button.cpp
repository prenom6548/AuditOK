#include "button.h"
#include <QDebug>
#include <QPainter>
#include "mainwindow.h"
#include <QMessageBox>
#include <QAction>

#define BUTTONWIDTH 36
#define BUTTONHEIGHT 36

CameraButton::CameraButton(QWidget* parent): kdk::KPressButton(parent)
{
    connect(this,&CameraButton::clicked,this,&CameraButton::onClicked);
}
CameraButton::~CameraButton(){}

void CameraButton::setIgnorePress(bool ignore)
{
    m_ignorePress = ignore;
}

void CameraButton::onClicked(bool b)
{
    if(!m_ignorePress) {
        Q_EMIT buttonTriggered();
    } else {
        setChecked(!b);
    }
}

bool Button::video_start = false;
bool Button::video_mode = false;
bool Button::voice_mode = false;
bool Button::picturepage_show = false;
bool Button::delay_mode = false;
Button::Button(QWidget *parent)
    : QWidget(parent)
{
    QString btnStyle="QPushButton{border-radius:4px;}";
    //辅助的功能类
    gsettingItem = new DefaultGetSet;//gsetting读取设置的接口类
    gsettingItem->setAccessibleName("kylin-camera_button_DefaultGetSet_gsettingItem");
    //btnLayout的界面设置相关
    pictureWrap = new QWidget;
    pictureWrap->setAccessibleName("kylin-camera_button_QWidget_pictureWrap");
    pictureWrap->setFixedSize(56,44);
    picture = new QLabel;
    picture->setAccessibleName("kylin-camera_QLable_picture");
    picture->setFixedSize(56,44);
    picture->setFocusPolicy(Qt::NoFocus);
    picStackWidget = new QStackedLayout;
    picStackWidget->setStackingMode(QStackedLayout::StackAll);
    //    picStackWidget->setWindowOpacity(0);
    pictureVideo = new QPushButton;
    pictureVideo->setAccessibleName("kylin-camera_QPushButton_pictureVideo");
    pictureVideo->setFlat(true);
    pictureVideo->setStyleSheet("QPushButton{border:none;background:transparent;}");
    pictureVideo->setFixedSize(56,44);
    pictureVideo->setIconSize(QSize(22,22));
    pictureVideo->setIcon(QIcon(":/image/ukui-play-symbolic.svg"));
    pictureVideo->setWindowOpacity(0);
    picStackWidget->addWidget(picture);
    picStackWidget->addWidget(pictureVideo);
    pictureWrap->setLayout(picStackWidget);

    // 设置鼠标事件穿透
      pictureVideo->setAttribute(Qt::WA_TransparentForMouseEvents, true);
      pictureWrap->setStyleSheet(
                  "QWidget{"
                  "   border: 4px solid transparent;" //默认透明边框（用于占位）
                  "   box-sizing:border-box;}"    // 确保边框宽度计入控件总尺寸
                  "QWidget:hover{"
                  "   border: 4px solid #E6E6E6;" // 悬停时边框变为灰色
                  "}");

    m_captureDelay = new CameraButton;
    m_captureDelay->setAccessibleName("kylin-camera_button_CameraButton_m_captureDelay");
    m_captureDelay->setCheckable(true);
    m_captureDelay->setFocusPolicy(Qt::NoFocus);
    m_captureDelay->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    cheese = new kdk::KPushButton;
    cheese->setAccessibleName("kylin-camera_button_KPushButton_cheese");
    cheese->setButtonType(kdk::KPushButton::CircleType);
    cheese->setTranslucent(true);
    cheese->setFixedSize(54,54);
    cheese->setCheckable(false);
//    cheese->setStyleSheet(btnStyle);
    cheese->setIconSize(QSize(50,50));
    cheese->setFocusPolicy(Qt::NoFocus);
    cheese_dark = new kdk::KPushButton;
    cheese_dark->setAccessibleName("kylin-camera_button_KPushButton_cheese_dark");
    cheese_dark->setButtonType(kdk::KPushButton::CircleType);
    cheese_dark->setTranslucent(true);
    cheese_dark->hide();  //lhl debug
    cheese_dark->setFixedSize(54,54);
    cheese_dark->setCheckable(false);
//    cheese_dark->setStyleSheet(btnStyle);
    cheese_dark->setIconSize(QSize(50,50));
    cheese_dark->setFocusPolicy(Qt::NoFocus);
    cheese_video = new QPushButton;
    cheese_video->setAccessibleName("kylin-camera_QPushButton_cheese_video");
    cheese_video->setFixedSize(50,50);
    cheese_video->setStyleSheet(btnStyle);
    cheese_video->setIconSize(QSize(50,50));
    cheese_video->setFocusPolicy(Qt::NoFocus);
    cheese_stop = new kdk::KPressButton;
    cheese_stop->setAccessibleName("kylin-camera_button_KPressButton_cheese_stop");
    cheese_stop->hide();
    cheese_stop->setFixedSize(50,50);
    cheese_stop->setStyleSheet(btnStyle);
    cheese_stop->setIconSize(QSize(50,50));
    cheese_stop->setFocusPolicy(Qt::NoFocus);
    m_captureMirror = new CameraButton;
    m_captureMirror->setAccessibleName("kylin-camera_button_CameraButton_m_captureMirror");
    m_captureMirror->setCheckable(true);
    m_captureMirror->setFocusPolicy(Qt::NoFocus);
    m_captureMirror->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    m_captureGrid = new CameraButton;
    m_captureGrid->setAccessibleName("kylin-camera_button_CameraButton_m_captureGrid");
    m_captureGrid->setCheckable(true);
    m_captureGrid->setFocusPolicy(Qt::NoFocus);
    m_captureGrid->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    m_captureBurst = new CameraButton;
    m_captureBurst->setAccessibleName("kylin-camera_button_CameraButton_m_captureBurst");
    m_captureBurst->setCheckable(true);
    m_captureBurst->setFocusPolicy(Qt::NoFocus);
    m_captureBurst->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);

    btnLayout1 = new QHBoxLayout;
    btnLayout1->addStretch(1);
    btnLayout1->addWidget(m_captureMirror);
    btnLayout1->addSpacing(16);
    btnLayout1->addWidget(m_captureDelay);
    btnLayout1->addSpacing(22);
    btnLayout1->addWidget(cheese);
    btnLayout1->addWidget(cheese_dark);
    btnLayout1->addSpacing(22);
    btnLayout1->addWidget(m_captureBurst);
    btnLayout1->addSpacing(16);
    btnLayout1->addWidget(m_captureGrid);
    btnLayout1->addStretch(1);
    btnLayout1->addSpacing(75);
    btnLayout1->setMargin(0);
    btnWidget1 = new QWidget(this);
    btnWidget1->setAccessibleName("kylin-camera_button_QWidget_btnWidget1");
    btnWidget1->setLayout(btnLayout1);

    m_videoDelay = new CameraButton;
    m_videoDelay->setAccessibleName("kylin-camera_button_CameraButton_m_videoDelay");
    m_videoDelay->setCheckable(true);
    m_videoDelay->setFocusPolicy(Qt::NoFocus);
    m_videoDelay->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    m_videoDelay->setIcon(QIcon::fromTheme("ukui-countselect-symbolic"));

    m_videoGrid = new CameraButton;
    m_videoGrid->setAccessibleName("kylin-camera_button_CameraButton_m_videoGrid");
    m_videoGrid->setCheckable(true);
    m_videoGrid->setFocusPolicy(Qt::NoFocus);
    m_videoGrid->setFixedSize(BUTTONWIDTH,BUTTONHEIGHT);
    m_videoGrid->setIcon(QIcon::fromTheme("ukui-grid-close-symbolic"));

    btnLayout2 = new QHBoxLayout;
    btnLayout2->addStretch(1);
    btnLayout2->addWidget(m_videoDelay);
    btnLayout2->addSpacing(22);
    btnLayout2->addWidget(cheese_video);
    btnLayout2->addWidget(cheese_stop);
    btnLayout2->addSpacing(22);
    btnLayout2->addWidget(m_videoGrid);
    btnLayout2->addStretch(1);
    btnLayout2->addSpacing(72);
    btnLayout2->setMargin(0);
    btnWidget2 = new QWidget(this);
    btnWidget2->setAccessibleName("kylin-camera_button_QWidget_btnWidget2");
    btnWidget2->setLayout(btnLayout2);

    btnContainerLayout = new QStackedLayout;
    btnContainerLayout->addWidget(btnWidget1);
    btnContainerLayout->addWidget(btnWidget2);

    //switchLayout的界面设置相关
    capture = new kdk::KPressButton;
    capture->setAccessibleName("kylin-camera_button_KPressButton_capture");
    capture->setFixedSize(48,24);
    capture->setCheckable(true);
    capture->setFocusPolicy(Qt::NoFocus);
    video = new kdk::KPressButton;
    video->setAccessibleName("kylin-camera_button_KPressButton_video");
    video->setFixedSize(48,24);
    video->setCheckable(true);
    video->setFocusPolicy(Qt::NoFocus);
    switchLayout = new QHBoxLayout;
    switchLayout->addStretch(99);
    switchLayout->addWidget(capture);
    switchLayout->addSpacing(-8);
    switchLayout->addWidget(video);
    switchLayout->addStretch(99);
    switchLayout->setMargin(0);

    picAndButton = new QHBoxLayout;
    picAndButton->addSpacing(20);
    picAndButton->addWidget(pictureWrap);
    picAndButton->addLayout(btnContainerLayout);

    //界面总体布局
    vLayout = new QVBoxLayout;
    vLayout->addSpacing(5);
    vLayout->addLayout(switchLayout);
    vLayout->addLayout(picAndButton);
    vLayout->setMargin(0);
    this->setLayout(vLayout);
    this->setAttribute(Qt::WA_StyledBackground);
    // 注册 D-Bus 服务
    m_dbus_interface_peony = new QDBusInterface("org.freedesktop.FileManager1",
                                   "/org/freedesktop/FileManager1",
                                   "org.freedesktop.FileManager1",
                                   QDBusConnection::sessionBus());

    //从配置文件中读取之前的配置
    if(!set->value("model").toInt()){//gsetting记录应用上次关闭前处于拍照模式/录像模式
        btnContainerLayout->setCurrentWidget(btnWidget1);

        video_mode = false;
        capture->setChecked(1);
        video->setChecked(0);
#ifdef __V10__
        capture->setStyleSheet("QPushButton{background-color:#3D6BE5;border-radius:4px;}");
#endif
    }
    else{
        video_mode = true;
        btnContainerLayout->setCurrentWidget(btnWidget2);
        capture->setChecked(0);
        video->setChecked(1);

#ifdef __V10__
        video->setStyleSheet("QPushButton{background-color:#3D6BE5;border-radius:4px;}");
#endif
    }
#ifdef __V10__
    //    capture->setIcon(QIcon(":/image/camera1.png"));
    //    video->setIcon(QIcon(":/image/video1.png"));
    cheese->setIcon(QIcon(":/image/paizhao-kaishi.png"));
    cheese_dark->setIcon(QIcon(":/image/shot-dark.png"));
    picture->setIcon(QIcon(":/image/picture1.png"));
    cheese_video->setIcon(QIcon(":/image/luxiang-kaishi.png"));
    cheese_stop->setIcon(QIcon(":/image/luxiang-zanting.png"));
#else
    capture->setIcon(QIcon::fromTheme("camera-photo-symbolic"));
    video->setIcon(QIcon::fromTheme("camera-video-symbolic"));

//    cheese->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/image/shot.svg);}"
//                                       "QPushButton::hover{border:0px; background:transparent; border-image:url(:/image/shot-hover.svg);}"
//                                       "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/image/shot-click.svg);}");
//    cheese_dark->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/image/shot-dark.svg);}"
//                               "QPushButton::hover{border:0px; background:transparent; border-image:url(:/image/shot-dark-hover.svg);}"
//                               "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/image/shot-dark-click.svg);}");

    cheese->setIcon(QIcon(":/image/shot-click.svg"));
    cheese_dark->setIcon(QIcon(":/image/shot-dark.svg"));

    cheese_video->setIcon(QIcon(":/image/actiondark-click.svg"));
    //    cheese_stop->setIcon(QIcon(":/image/complete.svg"));
    //    cheese_stop->setIcon(QIcon::fromTheme("media-playback-stop-symbolic"));
    m_captureDelay->setIcon(QIcon::fromTheme("ukui-countselect-symbolic"));
    m_captureGrid->setIcon(QIcon::fromTheme("ukui-grid-close-symbolic"));
    m_captureMirror->setIcon(QIcon::fromTheme("object-flip-horizontal-symbolic"));
    m_captureBurst->setIcon(QIcon::fromTheme("ukui-continuous-close-symbolic"));

    capture->setToolTip(tr("capture mode"));
    video->setToolTip(tr("video mode"));
    cheese->setToolTip(tr("cheese"));
    cheese_dark->setToolTip(tr("cheese"));
    cheese_video->setToolTip(tr("video"));
    cheese_stop->setToolTip(tr("stop"));
    pictureWrap->setToolTip(tr("album"));
    m_captureDelay->setToolTip(tr("delay"));
    m_videoDelay->setToolTip(tr("delay"));
    m_captureMirror->setToolTip(tr("mirror"));
    m_captureGrid->setToolTip(tr("grid"));
    m_videoGrid->setToolTip(tr("grid"));
    m_captureBurst->setToolTip(tr("seiral shoot"));
    //适配窗管
    picture->setProperty("useIconHighlightEffect", 0x8);
    capture->setProperty("useIconHighlightEffect", 0x8);
    video->setProperty("useIconHighlightEffect", 0x8);
    cheese->setProperty("useIconHighlightEffect", 0x8);
    cheese_dark->setProperty("useIconHighlightEffect", 0x8);
    m_captureDelay->setProperty("useIconHighlightEffect", 0x8);
    m_captureGrid->setProperty("useIconHighlightEffect", 0x8);
    m_captureMirror->setProperty("useIconHighlightEffect", 0x8);
    m_captureBurst->setProperty("useIconHighlightEffect", 0x8);
#endif
    initConnect();
    initThemeApt();

    picture->installEventFilter(this);
    pictureVideo->installEventFilter(this);
}
Button::~Button(){
    delete gsettingItem;
    delete picture;
    delete m_captureDelay;
    delete cheese;
    delete cheese_dark;
    delete cheese_video;
    delete cheese_stop;
    delete m_captureMirror;
    delete m_captureGrid;
    delete m_captureBurst;
    delete capture;
    delete video;
}
void Button::initThemeApt(){
    //拍照按钮的图标变换
    //获取主题 lhl debug
    nowThemeStyle = gsettingItem->getStyleName();
    qDebug() << "has_device:" << HasDevicePage::has_device;
    //拍照按钮的图标变换 黑夜模式
    if(!video_mode){
        if(nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black"){
            cheese_dark->show();
            cheese->hide();
        }
        else{
            cheese_dark->hide();
            cheese->show();
        }
    }

//    拍照按钮的图标变换
    connect(cheese_dark,&QPushButton::pressed,this,[=](){
        if(!video_mode && HasDevicePage::has_device){
            cheese_dark->setIcon(QIcon(":/image/shot-dark-click.svg"));
        }
    });
    connect(cheese,&QPushButton::pressed,this,[=](){
        if(!video_mode && HasDevicePage::has_device){
            cheese->setIcon(QIcon(":/image/shot-hover.svg"));
        }
    });
}
void Button::initConnect(){
    connect(capture,&QPushButton::clicked,this,[=]() {
        if (video_mode == true) {
            capture_click();
        } else {
            capture->setChecked(1);
            video->setChecked(0);
        }
    });
    connect(video,&QPushButton::clicked,this,[=]() {
        if (video_mode == false) {
            video_click();
        } else {
            capture->setChecked(0);
            video->setChecked(1);
        }
    });
    connect(cheese,&QPushButton::clicked,this,&Button::cheese_click);
    connect(cheese_dark,&QPushButton::clicked,this,&Button::cheese_dark_click);
    connect(cheese_stop,&QPushButton::clicked,this,&Button::cheese_stop_click);
    connect(gsettingItem,&DefaultGetSet::sigThemeChange, this,&Button::dealSystemGsettingChange);

    connect(m_videoGrid,&CameraButton::buttonTriggered,this,&Button::onVideoGridClicked);
    connect(m_captureGrid,&CameraButton::buttonTriggered,this,&Button::onVideoGridClicked);
    connect(m_captureMirror,&CameraButton::buttonTriggered,this,&Button::onMirrorClicked);
    connect(m_captureBurst,&CameraButton::buttonTriggered,this,&Button::onBurstClicked);
    connect(m_captureDelay,&CameraButton::buttonTriggered,this,&Button::onDelayClicked);
    connect(m_videoDelay,&CameraButton::buttonTriggered,this,&Button::onDelayClicked);
}
void Button::paintEvent(QPaintEvent* event){
    QPainter painter(this);
    if(video_mode){
        //录像模式控制面板背景色
        QColor videoModeBtnBackground = Qt::black;
        videoModeBtnBackground.setAlpha(76);
        painter.fillRect(rect(),videoModeBtnBackground);
    }else{
        painter.fillRect(rect(), palette().color(QPalette::Base));
    }

    QWidget::paintEvent(event);

}

void Button::capture_click(){
    //开始录像时不能切换
    nowThemeStyle = gsettingItem->getStyleName();
    qDebug () << m_videoDelay->isChecked() << m_captureDelay->isChecked();
    if(m_videoDelay->isChecked() && delay_mode == true){
        //进入延时不能切换
        btnContainerLayout->setCurrentWidget(btnWidget2);
        video_mode = true;
        capture->setChecked(0);
        video->setChecked(1);
#ifdef __V10__
        capture->setStyleSheet("QPushButton{background-color:#3D6BE5;border-radius:4px;}");
        video->setStyleSheet("QPushButon{background-color:#000000}");
#endif
    } else if(!video_start){
        if(nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black"){
            cheese_dark->show();
            cheese->hide();
        }else{
            cheese_dark->hide();
            cheese->show();
        }
        btnContainerLayout->setCurrentWidget(btnWidget1);
        video_mode = false;
        capture->setChecked(1);
        video->setChecked(0);
#ifdef __V10__
        capture->setStyleSheet("QPushButton{background-color:#3D6BE5;border-radius:4px;}");
        video->setStyleSheet("QPushButon{background-color:#000000}");
#endif
    }else{
        video_mode = true;
        capture->setChecked(0);
        video->setChecked(1);
    }
    Q_EMIT winHeight();
}

void Button::video_click(){
    if(!video_start && !delay_mode){
        video_mode = true;
        capture->setChecked(0);
        video->setChecked(1);
#ifdef __V10__
        video->setStyleSheet("QPushButton{background-color:#3D6BE5;border-radius:4px;}");
        capture->setStyleSheet("QPushButon{background-color:#000000}");
#endif
        btnContainerLayout->setCurrentWidget(btnWidget2);
        set->setValue("model",0);
    }
    if(video_start){
        video->setChecked(1);
    }
    if(delay_mode){
        video->setChecked(0);
    }
    if(m_captureDelay->isChecked() && m_captureDelay->text() == ""){
        //m_captureDelay->setChecked(false);
    }
    if(m_captureBurst->isChecked() && m_captureBurst->text() == ""){
        //m_captureBurst->setChecked(false);
    }
    Q_EMIT winHeight();
}

void Button::cheese_click(){
    //拍照模式
    if(!video_mode && HasDevicePage::has_device){
        cheese->setIcon(QIcon(":/image/shot-click.svg"));
    }
}

void Button::cheese_dark_click(){
    //拍照模式
    if(!video_mode && HasDevicePage::has_device){
        cheese_dark->setIcon(QIcon(":/image/shot-dark.png"));
    }
}

void Button::cheese_video_click(){
    //录像开始
    if(HasDevicePage::has_device){
        video_start = 1;
        cheese_stop->show();
        cheese_video->hide();
    }
}

void Button::cheese_stop_click(){
    video_start = 0;
    cheese_stop->hide();
    cheese_video->show();
    cheese_video->setIcon(QIcon(":/image/actiondark-click.svg"));
}

void Button::voice_click(){
    //关闭录音设备
    if(!video_start){
        if(voice_mode){
            voice->setIcon(QIcon(":/image/maike-off .svg"));
            voice_mode = false;
        }
        //打开录音设备
        else{
            voice->setIcon(QIcon(":/image/maike-on .svg"));
            voice_mode = true;
        }
    }
}
void Button::stop_click(){
    if(HasDevicePage::has_device && (video_start)){
    }
}
void Button::restore_click(){
}

void Button::picture_click(){
    if(!picturepage_show){
        picturepage_show = true;
#ifdef __V10__
        picture->setStyleSheet("QPushButton{background-color:#3DBE5;border-radius:4px;}");
#endif
    }
    else{
        picturepage_show = false;
#ifdef __V10__
        picture->setStyleSheet("QPushButton{background-color:#000000;}");
#endif
    }
}

//冗余代码 需优化
void Button::dealSystemGsettingChange(const QString themeNow){
    if(!video_mode){
        if(themeNow == "ukui-dark" || themeNow == "ukui-black"){
            cheese_dark->show();
            cheese->hide();
        }
        else{
            cheese->show();
            cheese_dark->hide();
        }
    }
}

bool Button::eventFilter(QObject *watched, QEvent *event){
    if(event->type() == QEvent::MouseButtonDblClick){
        return true;
    }
    if(watched == picture || watched == pictureVideo){
        if (event->type() == QEvent::MouseMove) {
            return true;
        }
        if(event->type() == QEvent::MouseButtonPress){
            if(static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton){
                gsettingItem->addThumbnailSDKPoint();
                QString fileName = gsettingItem->getFileName();
                QFileInfo info(fileName);
                if(info.exists()){
                    // 如果是播放视频，且距离上次播放时间过短，不播放
                    if (watched == pictureVideo && !m_canPlayVideo) {
                        qInfo() << "can't play video";
                        return true;
                    } else if (watched == pictureVideo && m_canPlayVideo) {
                        m_canPlayVideo = false;
                        QTimer::singleShot(m_playVideoInterval, [this]() { m_canPlayVideo = true; });
                    }
                    openFile();
                }
                else{
                    if(!video_mode){
                        openDir();
                    }
                    else{
                        openVideoDir();
                    }
                }
            }
        }
        if(watched == picture || watched == pictureVideo){
            if(event->type() == QEvent::MouseButtonPress){
                if(static_cast<QMouseEvent *>(event)->button() == Qt::RightButton){
                    albumRightMenu();
                }
            }
        }
        return QWidget::eventFilter(watched,event);
    }
}

void Button::openFile(){
    QString fileName = gsettingItem->getFileName();
    qDebug() << "latest filename" << fileName;
    QFileInfo info(fileName);
    if(info.exists()){
        QProcess proc;
        proc.startDetached("xdg-open " + fileName);
    }
}
void Button::openDir(){
    QString fileName = gsettingItem->getFileName();
    QString savePath;
    savePath = gsettingItem->getSavePath();
    if(!QDir(savePath).exists()){//路径不存在
        qDebug() << "存储路径不存在" << gsettingItem->getSavePath();
        QMessageBox mes;
        mes.warning(this,tr("waring"),tr("path does not exist, please set storage path"));
        mes.move(this->geometry().center()-mes.rect().center());
    }
    else{
        m_dbus_interface_peony->call("ShowFolders", QStringList{savePath}, "");
    }
}

void Button::openVideoDir(){
    QString videosavePath;
    videosavePath = gsettingItem->getVideoSavePath();
    if(!QDir(videosavePath).exists()){//路径不存在
        qDebug() << "存储路径不存在" << gsettingItem->getVideoSavePath();
        QMessageBox mes;
        mes.warning(this,tr("waring"),tr("path does not exist, please set storage path"));
        mes.move(this->geometry().center()-mes.rect().center());
    }
    else{
        m_dbus_interface_peony->call("ShowFolders", QStringList{videosavePath}, "");
    }
}

void Button::albumRightMenu()
{
    QMenu m_rightMenu;
    //定义action
    QAction openFilea(tr("open File"));
    QAction openDira(tr("open Dir"));
    QAction openVideoDira(tr("open Video Dir"));
    //action绑定槽函数
    connect(&openFilea, &QAction::triggered, this, &Button::openFile);
    connect(&openDira, &QAction::triggered, this, &Button::openDir);
    connect(&openVideoDira, &QAction::triggered, this, &Button::openVideoDir);
    //action添加到menu
    m_rightMenu.addAction(&openFilea);
    m_rightMenu.addAction(&openDira);
    m_rightMenu.addAction(&openVideoDira);
    //action添加到menu end
    m_rightMenu.exec(QCursor::pos());
}

void Button::onVideoGridClicked()
{
    QPushButton *obj = qobject_cast<QPushButton*>(sender());
    if(!obj->isChecked()){
        m_videoGrid->setChecked(false);
        m_captureGrid->setChecked(false);
        m_captureGrid->setIcon(QIcon::fromTheme("ukui-grid-close-symbolic"));
        m_videoGrid->setIcon(QIcon::fromTheme("ukui-grid-close-symbolic"));
        Q_EMIT showGirdLine(false);
    }else {
        m_videoGrid->setChecked(true);
        m_captureGrid->setChecked(true);
        m_captureGrid->setIcon(QIcon::fromTheme("ukui-grid-open-symbolic"));
        m_videoGrid->setIcon(QIcon::fromTheme("ukui-grid-open-symbolic"));
        Q_EMIT showGirdLine(true);
    }
}
void Button::onMirrorClicked()
{
    QPushButton *obj = qobject_cast<QPushButton*>(sender());
    if(!obj->isChecked()){
        m_captureMirror->setChecked(false);
        Q_EMIT mirror(false);
    }else {
        m_captureMirror->setChecked(true);
        Q_EMIT mirror(true);
    }
}
void Button::enableButtons(bool isEnable)
{
    if(!isEnable){
        m_captureDelay->setIgnorePress(true);
        m_videoDelay->setIgnorePress(true);
        m_captureMirror->setIgnorePress(true);
        m_captureGrid->setIgnorePress(true);
        m_videoGrid->setIgnorePress(true);
        m_captureBurst->setIgnorePress(true);
    }else{
        m_captureDelay->setIgnorePress(false);
        m_videoDelay->setIgnorePress(false);
        m_captureMirror->setIgnorePress(false);
        m_captureGrid->setIgnorePress(false);
        m_videoGrid->setIgnorePress(false);
        m_captureBurst->setIgnorePress(false);
    }
}

void Button::rmEventFilter()
{
    pictureVideo->removeEventFilter(this);
}

void Button::installEventFilter()
{
    pictureVideo->installEventFilter(this);
}

void Button::setBurstCount(int count)
{
    switch (count) {
    case 1:
        m_captureBurst->setIcon(QIcon::fromTheme("ukui-continuous-close-symbolic"));
        break;
    case 3:
        m_captureBurst->setIcon(QIcon::fromTheme("ukui-continuous-3p-symbolic"));
        break;
    case 5:
        m_captureBurst->setIcon(QIcon::fromTheme("ukui-continuous-5p-symbolic"));
        break;
    case 10:
        m_captureBurst->setIcon(QIcon::fromTheme("ukui-continuous-10p-symbolic"));
        break;
    case 20:
        m_captureBurst->setIcon(QIcon::fromTheme("ukui-continuous-20p-symbolic"));
        break;
    default:
        break;
    }
    if(count != 1){
        m_captureBurst->setChecked(true);
    }else{
        m_captureBurst->setChecked(false);
    }
}
void Button::onBurstClicked()
{
    QPushButton *obj = qobject_cast<QPushButton*>(sender());
    if(!obj->isChecked()){
        m_captureBurst->setChecked(false);
        Q_EMIT burst(false);
    }else {
        m_captureBurst->setChecked(true);
        Q_EMIT burst(true);
    }
}

void Button::onDelayClicked()
{
    QPushButton *obj = qobject_cast<QPushButton*>(sender());
    if(!obj->isChecked()){
        m_captureDelay->setChecked(false);
        m_videoDelay->setChecked(false);
        Q_EMIT delay(false);
    }else {
        m_captureDelay->setChecked(true);
        m_videoDelay->setChecked(true);
        Q_EMIT delay(true);
    }
}
