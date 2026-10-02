/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog.h"
#include "ui_dialog.h"
#include "worker.h"
#include "utils.h"
#include "get_userinfo.h"
#include "dbus_client.h"
#include "containerthread.h"
//#include "xatom-helper.h"

//#ifdef UKUI_WAYLAND
//#include "ukui-wayland/ukui-decoration-manager.h"
//#endif

#include <QThread>
#include <QString>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QX11Info>
#include <QPainter>
#include <QBitmap>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <pwd.h>
#include <unistd.h>

namespace {

const int dialogWidth = 400;
const int dialogHeight = 96;

}

Dialog::Dialog(const QString &tip, const QString &addition, QWidget *parent): QDialog(parent)
    , ui(new Ui::Dialog)
    , m_tipMSg(tip)
    , m_additionMsg(addition)
    , m_timer(new QTimer(this))
    , m_progressValue(0)
    , progressBar(new QProgressBar)
    , logo_label(new QLabel)
    , text_label(new QLabel)
{
    QStringList args = QApplication::arguments();
    if (args.size() > 1) {
        QString arg1 = args.at(1);
        if (arg1.compare("start_kmre_silently") == 0) {
            this->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
            this->setFixedSize(0, 0);

            QTimer::singleShot(7 * 1000, this, SLOT(onFinishApp()));
            return;
        }
    }

//    ui->setupUi(this);

    /*
    //this->setStyleSheet("QWidget{background-color:transparent;}");
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    */
//#ifdef KYLIN_V10
//    this->setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
//#else
//    this->setWindowFlags(Qt::Tool);//会导致XAtomHelper设置圆角无效
//#endif

    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);//QTool
    this->setWindowIcon(QIcon(":/image/images/kmre.svg"));
    //disable style window manager
    this->setProperty("useStyleWindowManager", false);//for UKUI 3.0
    this->setFixedSize(dialogWidth, dialogHeight);
//    this->setStyleSheet("QDialog{background: #FFFFFF; border-radius: 12px;}");
//    ui->progressBar->setFixedSize(dialogWidth, dialogHeight);
    progressBar->setFixedSize(280,8);
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);

//    progressBar->setStyleSheet("QProgressBar{background:rgba(0, 0, 0, 0.06); color:transparent;border-radius:4px;} QProgressBar::chunk{background:#3790FA;width:55px;border-radius:4px;}");
//    QPixmap pixmap = QPixmap(":/image/images/kmre.svg").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    QIcon kmre = QIcon::fromTheme("kylin-kmre");
    QPixmap pixmap = QPixmap(":/image/images/kylin-kmre.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logo_label->setFixedSize(48,48);
    logo_label->setScaledContents(true);
    logo_label->setPixmap(pixmap);
//    logo_label->setPixmap(kmre.pixmap(kmre.actualSize(QSize(48, 48))));
//    logo_label->setStyleSheet("QLabel{background:#DBDEE6; border-radius:12px;}");
    text_label->setFixedWidth(250);
    text_label->setWordWrap(true);
    text_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    text_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    text_label->setText(tr("Init Kmre Env"));

    QVBoxLayout *m_vlayout = new QVBoxLayout;
    m_vlayout->setMargin(0);
    m_vlayout->setSpacing(0);
    m_vlayout->addWidget(text_label);
    m_vlayout->addWidget(progressBar);
    QHBoxLayout *m_hlayout = new QHBoxLayout(this);
    m_hlayout->setMargin(24);
    m_hlayout->setSpacing(0);
    m_hlayout->addWidget(logo_label);
    m_hlayout->addSpacing(16);
    m_hlayout->addLayout(m_vlayout);
    m_hlayout->addStretch();

    this->setLayout(m_hlayout);

//    ui->logo_label->setFixedSize(64, 64);
//    ui->text_label->setFixedSize(342, 92);
//    ui->text_label->setWordWrap(true);
//    ui->text_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
//    ui->text_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    ui->text_label->setText(tr("Init Kmre Env"));//初始化移动应用环境

//    QFont curFont = ui->text_label->font();
//    curFont.setPixelSize(26);
//    curFont.setWeight(QFont::Light);
//    ui->text_label->setFont(curFont);

////    const auto ratio = qApp->devicePixelRatio();
////    qDebug()<<"ratio:"<<ratio;
////    if (ratio > 1) {
////    }
//    ui->logo_label->setScaledContents(true);
//    ui->logo_label->setPixmap(pixmap);

    this->setFixedPosition();

    this->showAnimation();
    this->doWork();

//#ifndef KYLIN_V10
////#ifndef UKUI_WAYLAND
//    // 添加窗管协议
//    if (QX11Info::isPlatformX11()) {
//        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
//        MotifWmHints hints;
//        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
//        hints.functions = MWM_FUNC_ALL;//& ~MWM_FUNC_MOVE
//        hints.decorations = MWM_DECOR_BORDER;
//        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
//    }
////#endif
//#endif
}

Dialog::~Dialog()
{
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }

    delete ui;
}

void Dialog::setFixedPosition()
{
    #define MARGIN 0
    QDBusInterface iface("org.ukui.panel",
                         "/panel/position",
                         "org.ukui.panel",
                         QDBusConnection::sessionBus());
    QDBusReply<QVariantList> reply=iface.call("GetPrimaryScreenGeometry");
    if (!iface.isValid() || !reply.isValid() || reply.value().size()<5) {
        //this->setGeometry(0,0,this->width(),this->height());
        this->move(desktop.availableGeometry().width() - this->width(), desktop.availableGeometry().height() - this->height());//move the window to right bottom
        return;
    }

    QVariantList position_list=reply.value();
    switch(reply.value().at(4).toInt()){
    //panel on top
    case 1:
        this->setGeometry(position_list.at(0).toInt()+position_list.at(2).toInt()-this->width()-MARGIN,
                          position_list.at(1).toInt()+MARGIN,
                          this->width(),this->height());
        break;
    //panel on left
    case 2:
        this->setGeometry(position_list.at(0).toInt()+MARGIN,
                          position_list.at(1).toInt()+reply.value().at(3).toInt()-this->height()-MARGIN,
                          this->width(),this->height());
        break;
    //panel on right
    case 3:
        this->setGeometry(position_list.at(0).toInt()+position_list.at(2).toInt()-this->width()-MARGIN,
                          position_list.at(1).toInt()+reply.value().at(3).toInt()-this->height()-MARGIN,
                          this->width(),this->height());
        break;
    //panel on bottom
    default:
        this->setGeometry(position_list.at(0).toInt()+position_list.at(2).toInt()-this->width()-MARGIN,
                          position_list.at(1).toInt()+reply.value().at(3).toInt()-this->height()-MARGIN,
                          this->width(),this->height());
        break;
    }
}

void Dialog::doWork()
{
    if (m_tipMSg == "incorrect_kernel" ||
        m_tipMSg == "unsupported_vm" ||
        m_tipMSg == "has_other_android_environment" ||
        m_tipMSg == "unsupported_root" ||
        m_tipMSg == "unsupported_os" ||
        m_tipMSg == "unsupported_gpu" ||
        m_tipMSg == "unsupported_cpu" ||
        m_tipMSg == "too_many_windows" ||
        m_tipMSg == "too_many_windows_for_cpu" ||
        m_tipMSg == "manager_service_exception" ||
        m_tipMSg == "too_many_windows_for_environment" ||
        m_tipMSg == "experience_effect") {
//        progressBar->setValue(0);
        progressBar->setVisible(false);

//        QFont font = ui->text_label->font();
//        font.setPixelSize(18);
//        font.setWeight(QFont::Light);
//        ui->text_label->setFont(font);

        if (m_tipMSg == "incorrect_kernel") {
            text_label->setText(tr("The kernel does not support Kmre"));//内核不支持移动应用环境
        }
        else if (m_tipMSg == "unsupported_vm") {
            text_label->setText(tr("Virtual machine is not supported in Kmre"));//移动应用环境暂不支持虚拟机
        }
        else if (m_tipMSg == "has_other_android_environment") {
            text_label->setText(tr("Kmre is corrupted, please check if other Android compatible env are installed"));//移动应用环境被破坏，请检查是否安装了其他安卓兼容环境
        }
        else if (m_tipMSg == "unsupported_root") {
            text_label->setText(tr("Please do not use root"));//请不要使用 root 用户
        }
        else if (m_tipMSg == "unsupported_gpu") {
            text_label->setText(tr("The current graphics card is not supported in Kmre"));//移动应用环境暂时不支持当前显卡
        }
        else if (m_tipMSg == "unsupported_os") {
            text_label->setText(tr("Kmre only supports Kylin OS for the time being"));//移动应用环境暂时只支持麒麟操作系统
        }
        else if (m_tipMSg == "unsupported_cpu") {
            text_label->setText(tr("Kmre does not support the current CPU temporarily") + " (" + m_additionMsg + ")");//移动应用环境暂时不支持当前CPU
        }
        else if (m_tipMSg == "too_many_windows") {
            text_label->setText(tr("The current graphics card cannot support more apps"));//当前显卡无法支持运行更多移动应用
        }
        else if (m_tipMSg == "too_many_windows_for_cpu") {
            text_label->setText(tr("The current CPU cannot support running more apps"));//当前CPU无法支持运行更多移动应用
        }
        else if (m_tipMSg == "too_many_windows_for_environment") {
            text_label->setText(tr("The current hardware environment cannot support running more apps"));//当前硬件环境无法支持运行更多移动应用
        }
        else if (m_tipMSg == "manager_service_exception") {
            text_label->setText(tr("Exception of dbus service interface to be accessed"));//需要访问的dbus服务接口异常
        }
        else if (m_tipMSg == "experience_effect") {
            text_label->setText(tr("For better experience, please close an android app and then open it"));//为了更好的体验效果，请关闭一个安卓应用后再开启
        }
        else {
            text_label->setText(tr("The current startup item does not support Kmre, please use default startup item"));//当前启动项不支持移动应用环境，请使用默认启动项
        }

        QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
    }
    else {
        //the operation when using correct kernel version
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerOut()));
        m_timer->start(20);

        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("Stopped"), this, SLOT(onShutDown(QString)));
        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("ServiceStartFailed"), this, SLOT(onServiceStartFailed(QString)));
        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("ServiceNotFound"), this, SLOT(onServiceNotFound(QString)));
        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("ImageConfNotFound"), this, SLOT(onImageConfNotFound()));
        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("ImageFileNotFound"), this, SLOT(onImageFileNotFound()));
        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("ImageLoadFailed"), this, SLOT(onImageLoadFailed()));
        QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                             QString("/cn/kylinos/Kmre"),
                                             QString("cn.kylinos.Kmre"),
                                             QString("ImageNotMatched"), this, SLOT(onImageNotMatched()));

        if (kmre::DBusClient::getInstance()->IsImageReady()) {
            qDebug() << "start Container";
            Utils::startContainer();

            m_worker = new Worker(m_tipMSg);
            m_thread = new QThread(this);
            m_worker->moveToThread(m_thread);
            m_thread->start();

            connect(m_worker, &Worker::androidRunCompleted, this, &Dialog::onAndroidRunCompleted, Qt::QueuedConnection);
        }
        else {
            //ui->text_label->setText(tr("Loading Android Image......"));//正在加载镜像......
            qDebug() << "Loading image......";
            kmre::DBusClient::getInstance()->LoadImage();

            QDBusConnection::systemBus().connect(QString("cn.kylinos.Kmre"),
                                                 QString("/cn/kylinos/Kmre"),
                                                 QString("cn.kylinos.Kmre"),
                                                 QString("ImageLoaded"), this, SLOT(onImageLoaded()));
        }

        QTimer::singleShot(240*1000, this, SLOT(onFinishApp())); //force to finish this progress
    }
}


void Dialog::onTimerOut()
{
    if (m_progressValue >= 100) {
        m_progressValue = 0;
    }
//    progressBar->setValue(m_progressValue++);
}

void Dialog::onAndroidRunCompleted()
{
    emit this->animaTimeOut();//emit signal to finish animation
}

//animation to show right bottom dialog
void Dialog::showAnimation()
{
    animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(700);//start or cloae animation time
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();

    connect(this, SIGNAL(animaTimeOut()), this,SLOT(closeAnimation() ));
}

//close animation
void Dialog::closeAnimation()
{
    disconnect(this,SIGNAL(animaTimeOut()),this,SLOT(closeAnimation()));

    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
    connect(animation,SIGNAL(finished()),this,SLOT(clearAll()));
    animation->start();
}

//clean up animation pointer
void Dialog::clearAll()
{
    disconnect(animation,SIGNAL(finished()),this,SLOT(clearAll()));
    delete animation;
    animation = NULL;

    //lixiang：环境首次启动后要关闭当前的 startapp 进程，而不是隐藏该进程的图形，否则程序一直还在后台
    closelog();
    QApplication::exit();
}

void Dialog::onImageLoaded()
{
    //qDebug() << "Android docker image had been loaded, will start android container";
    //ui->text_label->setText(tr("Init Kmre Env"));//初始化移动应用环境

    // 在线程中执行 Utils::startContainer() ,避免第一次加载容器后启动容器耗时太久导致界面卡顿
    ContainerThread *thrd = new ContainerThread;
    connect(thrd, &ContainerThread::finished, thrd, &ContainerThread::deleteLater, Qt::QueuedConnection);
    thrd->start();

    m_worker = new Worker(m_tipMSg);
    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);
    m_thread->start();

    connect(m_worker, &Worker::androidRunCompleted, this, &Dialog::onAndroidRunCompleted, Qt::QueuedConnection);
}

void Dialog::onShutDown(QString container)
{
    //qDebug() << "Android container had been closed!!!";

    struct str_info my_info;
    my_info = UserInfo::getuserinfo();

    QString name = "kmre-" + QString::number(my_info.uid_info) + "-" + my_info.name_info;
    if (name == container) {
        closelog();
        exit(0);
    }
}

void Dialog::onFinishApp()
{
    bool b = Utils::isAndroidDeskStart();
    if (!b) {
        //qDebug() << "Force to finish this process for waiting start kmre too long";
        closelog();
        QApplication::exit();
    }
}

void Dialog::onAnimationOver()
{
    emit this->animaTimeOut();//emit signal to closeAnimation()
}

void Dialog::onAppOver()
{
    closelog();
    QApplication::exit();
}

void Dialog::onServiceStartFailed(QString name)
{
    text_label->setText(tr("Service startup failed: %1").arg(name));
    QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
}

void Dialog::onServiceNotFound(QString name)
{
    text_label->setText(tr("No service found: %1").arg(name));
    QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
}

void Dialog::onImageConfNotFound()
{
    text_label->setText(tr("Image configuration not found"));
    QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
}

void Dialog::onImageFileNotFound()
{
    text_label->setText(tr("Image not found"));
    QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
}

void Dialog::onImageLoadFailed()
{
    text_label->setText(tr("Image loading failed"));
    QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
}

void Dialog::onImageNotMatched()
{
    text_label->setText(tr("Image mismatch"));
    QTimer::singleShot(7*1000, this, SLOT(onAnimationOver())); //finish animation after 7 seconds
}

void Dialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter painter(&bmp);
    painter.setRenderHint(QPainter::Antialiasing, true);//设置反走样，避免锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRoundedRect(bmp.rect(), 6, 6);
    setMask(bmp);
}
