#include "updatewidget.h"
#include "ui_updatewidget.h"
#include <QPainterPath>
#include <QToolTip>
#include <QHoverEvent>
#include "ukuistylehelper/ukuistylehelper.h"
#include "windowmanager/windowmanager.h"

#define PANEL_DBUS_SERVICE "com.ukui.panel.desktop"
#define PANEL_DBUS_PATH "/"
#define PANEL_DBUS_INTERFACE "com.ukui.panel.desktop"
#define UKUI_PANEL_SETTING "org.ukui.panel.settings"

#define DBUS_NAME       "org.ukui.SettingsDaemon"
#define DBUS_PATH       "/org/ukui/SettingsDaemon/wayland"
#define DBUS_INTERFACE  "org.ukui.SettingsDaemon.wayland"

updatewidget::updatewidget(bool check_Tips, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    //设置出现在屏幕的位置以及大小
    listenPanelChange();
    QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
    if (xdg_session_type != "wayland"){
        initPanelDbusGsetting();
        GetsAvailableAreaScreen();
        qDebug()<<"not wayland";
        bIsWayland = false;
    } else {
        //注释掉的是原先的接口，已经废除。采用原本的获取方式
//        initset_window_position();
//        set_window_position();
        initPanelDbusGsetting();
        GetsAvailableAreaScreen();
        qDebug()<<"wayland";
        bIsWayland = true;
    }
    _check_Tips = check_Tips;
    //设置任务栏无显示
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    update_lab     = new QLabel();
    icon_lab     = new QLabel();

    tip_lab        = new QLabel();
    tip_lab->setText(tr("    Updatable app detected on your system!"));
    agreebtn       = new FixButton();
    delaybtn       = new FixButton();
    window_close = new QPushButton(this);

    window_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    window_close->setProperty("isWindowButton", 0x2);
    window_close->setProperty("useIconHighlightEffect", 0x8);
    window_close->setFlat(true);
    window_close->setToolTip(tr("Close"));
    window_close->installEventFilter(this);
    connect(window_close, &QPushButton::clicked, this, &updatewidget::onClick_close_btn);

    QPixmap icon(QIcon::fromTheme("ukui-control-center").pixmap(QSize(26,26)));

    m_pfirstlayout  = new QVBoxLayout();
    m_labellayout = new QHBoxLayout();
    m_psecondlayout = new QHBoxLayout();
    m_pselectlayout = new QHBoxLayout();
    comboBox = new QComboBox();
    comboBox->clear();
    strList<<tr("30 minutes later")<<tr("1 hour later")<<tr("5 hours later")<<tr("1 day later")<<tr("3 days later");
    comboBox->addItems(strList);
    comboBox->installEventFilter(this);

    labelwidget = new QWidget();
    selectWidget = new QWidget();
    statwidget = new QWidget();

    agreebtn->setText(tr("Update"),1);
    agreebtn->installEventFilter(this);
    delaybtn->setText(tr("Delay"),1);
    delaybtn->installEventFilter(this);

    connect(agreebtn,&QPushButton::clicked,this,&updatewidget::agreeBtnClicked);
    connect(comboBox,SIGNAL(activated(int)),this,SLOT(comboBoxClicked(int)));
    connect(delaybtn,&QPushButton::clicked,this,&updatewidget::delayBtnClicked);

    icon_lab->setPixmap(icon);
    icon_lab->setScaledContents(true);
    icon_lab->setMaximumSize(26,26);
    update_lab->setMaximumHeight(26);
    update_lab->setContentsMargins(0,0,0,3);
    window_close->setFixedSize(24,24);
    window_close->setContentsMargins(0,0,0,3);
    m_labellayout->addWidget(icon_lab);
    m_labellayout->addWidget(update_lab);
    m_labellayout->addStretch();
    m_labellayout->addWidget(window_close);
    m_labellayout->setContentsMargins(0,0,0,20);
    labelwidget->setLayout(m_labellayout);

    m_pselectlayout->addWidget(tip_lab);
    m_pselectlayout->setContentsMargins(0,0,0,25);
    selectWidget->setLayout(m_pselectlayout);

    m_psecondlayout->addStretch();
    comboBox->setFixedHeight(35);
    comboBox->setMinimumWidth(135);
    comboBox->setMaximumWidth(155);
    agreebtn->setFixedSize(100,35);
    if (_check_Tips)
        m_psecondlayout->addWidget(delaybtn);
    else
        m_psecondlayout->addWidget(comboBox);
    m_psecondlayout->addWidget(agreebtn);
    m_psecondlayout->setSpacing(15);                 //set space between comboBox and btn
    m_psecondlayout->setContentsMargins(0,0,12,12);
    statwidget->setLayout(m_psecondlayout);

    m_pfirstlayout->addWidget(labelwidget);
    m_pfirstlayout->addWidget(selectWidget);
    m_pfirstlayout->addWidget(statwidget);

    m_pfirstlayout->setSpacing(2);
    this->setLayout(m_pfirstlayout);
    this->setFixedSize(435,160);

//    QGSettings *settings;
//    if(QGSettings::isSchemaInstalled("org.ukui.style")){
//        settings = new QGSettings("org.ukui.style");
//        connect(settings,&QGSettings::changed,this,&ActiveConnectionWidget::GSettingsChanges);
//    }

//    const QByteArray transparency_id(TRANSPARENCY_SETTINGS);
//    if(QGSettings::isSchemaInstalled(transparency_id)) {
//        transparency = new QGSettings(transparency_id);
//    }
//    if(QGSettings::isSchemaInstalled(transparency_id)) {
//        getTransparency();
//        connect(transparency,&QGSettings::changed,this,&ActiveConnectionWidget::GSettingsChanges);
//    }

    this->setProperty("useStyleWindowManager", false);
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowOpacity(0.9);

//    labelwidget->setStyleSheet("background-color:green;");
//    update_lab->setStyleSheet("background-color:blue;");
//    selectWidget->setStyleSheet("background-color:red;");
//    statwidget->setStyleSheet("background-color:orange;");
//    comboBox->setStyleSheet("background-color:violet;");
//    agreebtn->setStyleSheet("background-color:yellow;");
}

updatewidget::~updatewidget()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    delete ui;
}

void updatewidget::updateWidgetShow()
{
    this->show();
    qDebug()<<"should show:"<<updateWidgetRect;

    if (bIsWayland)
        kdk::WindowManager::setGeometry(this->windowHandle(), updateWidgetRect);
    else
        this->setGeometry(updateWidgetRect);
}

void updatewidget::onClick_close_btn()
{
    // emit disAgreeBtnSignals();
    qDebug()<<"User Cancel reminder, exit tray program!";
    // exit(0);
    qApp->quit();
}

/**
 * @brief updatewidget::eventFilter
 * 事件过滤器
 */
bool updatewidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this->window_close) {           //指定window_close
        if (event->type() == QEvent::HoverMove) {
            QPushButton *close_btn = qobject_cast<QPushButton *>(obj);
            if (close_btn) {
                QToolTip::showText(QCursor::pos(), close_btn->toolTip());
            }
        }
    } else if (obj == this->agreebtn || obj == this->delaybtn) {
        if (event->type() == QEvent::HoverMove) {
            FixButton *btn = qobject_cast<FixButton *>(obj);
            if (btn) {
                QToolTip::showText(QCursor::pos(), btn->toolTip());
            }
        }
    } else if (obj == this->comboBox) {
        if (event->type() == QEvent::HoverMove) {
            QComboBox *comboBox = qobject_cast<QComboBox *>(obj);
            if (comboBox) {
                QToolTip::showText(QCursor::pos(), comboBox->toolTip());
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

/**
 * @brief updatewidget::initPanelDbusGsetting
 * 初始化与任务栏gsetting和dbus
 */
void updatewidget::initPanelDbusGsetting()
{
    /* 链接任务栏Dbus接口，获取任务栏高度和位置 */
    m_pServiceInterface = new QDBusInterface(PANEL_DBUS_SERVICE, PANEL_DBUS_PATH, PANEL_DBUS_INTERFACE, QDBusConnection::sessionBus());
    m_pServiceInterface->setTimeout(2147483647);
    connect(m_pPanelSetting, &QGSettings::changed,[=](QString key){
        if ("panelposition" == key | "panelsize" == key) {
             GetsAvailableAreaScreen();
        }
     });
}

void updatewidget::listenPanelChange()
{
    /* 链接任务栏dgsetting接口 */
    if(QGSettings::isSchemaInstalled(UKUI_PANEL_SETTING))
        m_pPanelSetting = new QGSettings(UKUI_PANEL_SETTING);
}

/**
 * @brief updatewidget::connectTaskBarDbus
 * 链接任务栏dbus获取高度的接口
 * @return
 */
int updatewidget::connectTaskBarDbus()
{
    int panelHeight = 46;
    if (m_pPanelSetting != nullptr) {
        QStringList keys = m_pPanelSetting->keys();
        if (keys.contains("panelsize")) {
            panelHeight = m_pPanelSetting->get("panelsize").toInt();
        }
    } else {
        QDBusMessage msg = m_pServiceInterface->call("GetPanelSize", QVariant("Hight"));
        panelHeight = msg.arguments().at(0).toInt();
        return panelHeight;
    }
    return panelHeight;
}

/**
 * @brief updatewidget::getPanelSite
 * 获取任务栏状态位置下上左右
 * @return
 * 返回任务栏位置
 */
int updatewidget::getPanelSite()
{
    int panelPosition = 0;
    if (m_pPanelSetting != nullptr) {
        QStringList keys = m_pPanelSetting->keys();
        if (keys.contains("panelposition")) {
            panelPosition = m_pPanelSetting->get("panelposition").toInt();
        }
    } else {
        QDBusMessage msg = m_pServiceInterface->call("GetPanelPosition", QVariant("Site"));
        panelPosition = msg.arguments().at(0).toInt();
    }
    qDebug() << "Current Position of ukui panel: " << panelPosition;
    return panelPosition;
}

/**
 * @brief updatewidget::GetsAvailableAreaScreen
 * 获取屏幕的可用区域高度和宽度并设置位置
 */
void updatewidget::GetsAvailableAreaScreen()
{
    //如果取不到任务栏的高度,还是优先获取桌面分辨率,可用区域
    if ((0 == connectTaskBarDbus()) && (0 == getPanelSite())) {
        QScreen* pScreen = QGuiApplication::primaryScreen();
        QRect DeskSize = pScreen->availableGeometry();
        m_nScreenWidth = DeskSize.width();                      //桌面分辨率的宽
        m_nScreenHeight = DeskSize.height();                    //桌面分辨率的高
    } else {
        //如果取到任务栏的高度,则取屏幕分辨率的高度
        int h = connectTaskBarDbus();
        QRect screenRect = QGuiApplication::primaryScreen()->geometry();
        m_nScreenWidth = screenRect.width();
        m_nScreenHeight = screenRect.height();
        m_pPeonySite = getPanelSite();
        switch (m_pPeonySite)
        {
            case updatewidget::PanelDown :
                {
                    qDebug()<<"Taskbar coordinates: [ '"<<m_nScreenWidth-445<<"','"<< m_nScreenHeight-h-170<<"','"<<300<<"','"<<150 - 100<<"']";
                    updateWidgetRect = QRect(m_nScreenWidth-445, m_nScreenHeight-h-170, 300, 150-100);
                }
                break;
            case updatewidget::PanelUp:
                {
                    qDebug()<<"任务栏在上方： 显示坐标为： "<<m_nScreenWidth - 445 << m_nScreenHeight - 170 << 300 << 150 - 100;
                    updateWidgetRect = QRect(m_nScreenWidth-445, m_nScreenHeight-170, 300, 150-100);
                }
                break;
            case updatewidget::PanelLeft:
                {
                    qDebug()<<"任务栏在左侧： 显示坐标为： "<<m_nScreenWidth - 445 << m_nScreenHeight - 170 << 300 << 150 - 100;
                    updateWidgetRect = QRect(m_nScreenWidth-445, m_nScreenHeight-170, 300, 150-100);
                }
                break;
            case updatewidget::PanelRight:
                {
                    qDebug()<<"任务栏在右侧： 显示坐标为： "<<m_nScreenWidth - 445 - h << m_nScreenHeight - 170 << 300 << 150 - 100;
                    updateWidgetRect = QRect(m_nScreenWidth-445, m_nScreenHeight-170, 300, 150-100);
                }
                break;
            default:
                break;
        }

    }
    qDebug() << "Current screen width:  " << m_nScreenWidth <<";  height:"<< m_nScreenHeight;
}


int updatewidget::getTaskbarPos(QString str)
{
    QDBusInterface interface( "com.ukui.panel.desktop",
                              "/",
                              "com.ukui.panel.desktop",
                              QDBusConnection::sessionBus() );

    QDBusReply<int> reply = interface.call("GetPanelPosition", str);
    return reply;
}

int updatewidget::getTaskbarHeight(QString str)
{
    QDBusInterface interface( "com.ukui.panel.desktop",
                              "/",
                              "com.ukui.panel.desktop",
                              QDBusConnection::sessionBus() );

    QDBusReply<int> reply = interface.call("GetPanelSize", str);
    return reply;
}


int updatewidget::getScreenGeometry(QString methodName)
{
    int res = 0;
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_NAME,
                               DBUS_PATH,
                               DBUS_INTERFACE,
                               methodName);
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        if(response.arguments().isEmpty() == false) {
            int value = response.arguments().takeFirst().toInt();
            res = value;
        }
    } else {
        qDebug()<<methodName<<"called failed";
    }
    return res;
}

void updatewidget::initset_window_position()
{
    connect(m_pPanelSetting, &QGSettings::changed,[=](QString key){
        if ("panelposition" == key | "panelsize" == key) {
             set_window_position();
        }
     }
    );
}

void updatewidget::set_window_position()
{
    QRect rect;
    int availableWidth,totalWidth;
    int availableHeight,totalHeight;
    rect = this->geometry();
    int priX = getScreenGeometry("x");
    int priY = getScreenGeometry("y");
    int priWid = getScreenGeometry("width");
    int priHei = getScreenGeometry("height");
    QRect screenGeometry = qApp->primaryScreen()->geometry();

    availableWidth = priX + priWid;
    availableHeight = priY + priHei;
    totalHeight = screenGeometry.height();
    totalWidth = screenGeometry.width();

    int distance = 4;
    int n = 0;
    int m = 46;

    n = getTaskbarPos("position");
    m = getTaskbarHeight("height");
    if(n == 0){
        //任务栏在下侧
        this->setGeometry(priWid-435-distance,availableHeight-160-m-distance,this->width(),this->height());
        qDebug()<<"任务栏在下侧"<<priWid-435-distance<<"  "<<availableHeight-160-m-distance <<" "<<this->width()<<" "<<this->height();
    }else if(n == 1){
        //任务栏在上侧
        this->setGeometry(priWid-435-distance,availableHeight-160-distance,this->width(),this->height());
        qDebug()<<"任务栏在上侧"<<priWid-435-distance<<"  "<<availableHeight-160-distance <<" "<<this->width()<<" "<<this->height();
    } else if (n == 2){
        //任务栏在左侧
        this->setGeometry(priWid-435-distance,availableHeight-160-distance,this->width(),this->height());
        qDebug()<<"任务栏在左侧"<<priWid-435-distance<<"  "<<availableHeight-160-distance <<" "<<this->width()<<" "<<this->height();
    } else if (n == 3){
        //任务栏在右侧
        this->setGeometry(availableWidth-435-m-distance,availableHeight-160-distance,this->width(),this->height());
        qDebug()<<"任务栏在右侧"<<availableWidth-435-m-distance<<"  "<<availableHeight-160-distance <<" "<<this->width()<<" "<<this->height();
    }
}

/**
 * @brief updatewidget::insideTime
 * 获取时间
 * @return
 */
char *updatewidget::insideTime()
{
    qDebug()<<"current select: "<<this->comboBox->currentText();
    QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
    QString str = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
    QByteArray str2char2 = str.toLatin1(); // must
    char *nowtime = str2char2.data();

    if(comboBox->currentText() == strList.at(0)) {
        QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
        QString str = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
        QDateTime bojb = QDateTime::fromString(str,"yyyy-MM-dd hh:mm:ss").addSecs(1800);
        QString str1 = bojb.toString("yyyy-MM-dd hh:mm:ss");
        QByteArray str2char2 = str1.toLatin1(); // must
        nowtime = str2char2.data();
        return nowtime;
    } else if (comboBox->currentText() == strList.at(1)) {
        QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
        QString str = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
        QDateTime bojb = QDateTime::fromString(str,"yyyy-MM-dd hh:mm:ss").addSecs(3600);
        QString str1 = bojb.toString("yyyy-MM-dd hh:mm:ss");
        QByteArray str2char2 = str1.toLatin1(); // must
        nowtime = str2char2.data();
        return nowtime;
    } else if (comboBox->currentText() == strList.at(2)) {
        QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
        QString str = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
        QDateTime bojb = QDateTime::fromString(str,"yyyy-MM-dd hh:mm:ss").addSecs(18000);
        QString str1 = bojb.toString("yyyy-MM-dd hh:mm:ss");
        QByteArray str2char2 = str1.toLatin1(); // must
        nowtime = str2char2.data();
        return nowtime;
    } else if (comboBox->currentText() == strList.at(3)) {
        QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
        QString str = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
        QDateTime bojb = QDateTime::fromString(str,"yyyy-MM-dd hh:mm:ss").addDays(1);
        QString str1 = bojb.toString("yyyy-MM-dd hh:mm:ss");
        QByteArray str2char2 = str1.toLatin1(); // must
        nowtime = str2char2.data();
        return nowtime;
    } else if (comboBox->currentText() == strList.at(4)) {
        QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
        QString str = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
        QDateTime bojb = QDateTime::fromString(str,"yyyy-MM-dd hh:mm:ss").addDays(3);
        QString str1 = bojb.toString("yyyy-MM-dd hh:mm:ss");
        QByteArray str2char2 = str1.toLatin1(); // must
        nowtime = str2char2.data();
        return nowtime;
    }
    return nowtime;

}

bool updatewidget::_is_openkylin()
{
    QString OPENKYLIN_DISTTRIBUTOR = "Openkylin";
    QString command = "lsb_release -i -s";
    QProcess process;
    process.start(command);
    process.waitForFinished();

    QString result = process.readAllStandardOutput();
    int ret = result.left(OPENKYLIN_DISTTRIBUTOR.length()).compare(OPENKYLIN_DISTTRIBUTOR.left(OPENKYLIN_DISTTRIBUTOR.length()));
    return (0 == ret)?true:false;
}

/**
 * @brief updatewidget::agreeBtnClicked
 * 同意更新按钮槽函数
 */
void updatewidget::agreeBtnClicked()
{
    emit agreeBtnSignals();
    bool _is_2107 = true;
    QFile file("/etc/os-release");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString line;
        QTextStream in(&file);
        line = in.readLine();
        while(!line.isNull()) {
            line = in.readLine();
            if(line.contains("KYLIN_RELEASE_ID")) {
                _is_2107 = false;
            }
        }
    }
    file.close();
    QProcess p(0);
    if (_is_2107&&!_is_openkylin()) {
        p.startDetached("ukui-control-center --upgrade");
        qDebug()<<"exec --upgrade";
    } else {
        p.startDetached("ukui-control-center -m upgrade");
        qDebug()<<"exec -m upgrade";
    }
    p.waitForStarted();
    qDebug()<<"Open the control panel and start updating immediately, exit tray program!";
    exit(0);
}

/**
 * @brief updatewidget::comboBoxClicked
 * 推迟框槽函数
 */
void updatewidget::comboBoxClicked(int num)
{
    //判断是否有kylin-background-upgrade目录，没有则创建
    QString toDir = QDir::homePath();
    toDir += "/.config/kylin-background-upgrade/";
    QDir dir(toDir);
    if(! dir.exists()) {
       dir.mkdir(toDir);//只创建一级子目录，即必须保证上级目录存在
    }

    toDir += "kylin-background-upgrade-conf.ini";
    QFileInfo fileInfo(toDir);
    if (!fileInfo.exists()) {
       qDebug()<<"The periodic configuration file fails to be opened, exit!";
       exit(0);
    } else {
        char *nowtime=insideTime();
        qDebug() << "current timing: "<<nowtime;
        timersettings = new QSettings(toDir,QSettings::IniFormat);
        timersettings->beginGroup(QString::fromLocal8Bit("Timer"));
        timersettings->setValue("CheckUpgradeTime",nowtime);
        timersettings->sync();
        timersettings->endGroup();
    }
    emit disAgreeBtnSignals();
    qDebug()<<"Users do not update, exit tray program!";
    exit(0);
}

/**
 * @brief updatewidget::delayBtnClicked
 * 延迟提醒，退出
 */
void updatewidget::delayBtnClicked()
{
    qDebug() << "User deferred updates, exit!";
    exit(0);
}

/**
 * @brief updatewidget::paintEvent
 * 样式绘画画家
 * @param e
 */
void updatewidget::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    /* 获取当前剪贴板中字体的颜色，作为背景色；
     * 白字体 --> 黑背景
     * 黑字体 --> 白字体
    */
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setOpacity(1);
    p.setPen(QPen(QColor(220,220,220)));
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,12,12);
    p.setRenderHint(QPainter::Antialiasing);                        //反锯齿
    p.drawRoundedRect(opt.rect,12,12);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QFontMetrics fontMetrics(this->font());
    QString current_str = comboBox->currentText();
    int fontSize = fontMetrics.width(current_str);
    int showTextSize = comboBox->width() - 32;
    if (fontSize > showTextSize) {
        QString elidedText = fontMetrics.elidedText(comboBox->currentText(), Qt::ElideRight, showTextSize);
        comboBox->setItemText(comboBox->currentIndex(), elidedText);
        comboBox->setToolTip(current_str);
    }
    QWidget::paintEvent(e);
}

/**
 * @brief updateTip::updateTip
 * 提示框
 * @param parent
 */
updateTip::updateTip(QWidget *parent) :
    QWidget(parent)
{
    //设置任务栏无显示
    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Popup);

    m_layout = new QVBoxLayout();
    tipLabel = new QLabel();
    tipLabel->setText(tr("The system is updating silently"));
    m_layout->addWidget(tipLabel);
    this->setLayout(m_layout);
    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowOpacity(0.9);
}

updateTip::~updateTip()
{

}

/**
 * @brief updateTip::paintEvent
 * 样式绘画画家
 * @param e
 */
void updateTip::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    /* 获取当前剪贴板中字体的颜色，作为背景色；
     * 白字体 --> 黑背景
     * 黑字体 --> 白字体
    */
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setOpacity(1);
    p.setPen(QPen(QColor(220,220,220)));
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,4,4);
    p.setRenderHint(QPainter::Antialiasing);                        //反锯齿
    p.drawRoundedRect(opt.rect,4,4);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}


