#include "widget.h"
#include "ui_widget.h"

#include <QDBusConnection>
#include <QTimer>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>

#include "../dbus/interfacedbusproxy.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), timer(nullptr), countTimer(10), connectDevice(QString())
{
    ui->setupUi(this);

//    setWindowFlags(Qt::FramelessWindowHint);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect position = screen->availableGeometry();
    move(position.x() + position.width()/2 - width()/2, position.y() + position.height()/2 -height()/2);
    // 初始化第一页的内容
    ui->deviceLabel->setFont(QFont("Timers", 24, QFont::Bold));
    ui->warningLabel->setFont(QFont("Timers", 12, QFont::Bold));
    QPalette pa;
    pa.setColor(QPalette::WindowText, Qt::red);
    ui->warningLabel->setPalette(pa);


    // 初始化第二页的内容
    ui->pinLabel->setFont(QFont("Timers", 20, QFont::Bold));

    // 初始化第三页的内容
    ui->mediaStatusLabel->setFont(QFont("Timers", 24, QFont::Bold));
    thirdPageReset(INIT);

    // 初始化第四页的内容
    ui->label->setFont(QFont("Timers", 24, QFont::Bold));

//    countDownWarning();

    ui->stackedWidget->setCurrentIndex(0);

    show();
}

Widget::~Widget()
{
    delete ui;
    if(timer){
        delete timer;
        timer = nullptr;
    }
}

void Widget::dbusSignalInit()
{
    //建立到session bus的连接
    QDBusConnection connection = QDBusConnection::sessionBus();

    // 收到移动设备的连接请求信号
    if(connection.connect(QString(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "Confirming", this, SLOT(getConfirmingSignal(QString deviceName))))
        qDebug() << "连接confirming信号成功！";
    if(connection.connect(QString(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "Formationing", this, SLOT(getFormationingSignal(QString deviceName))))
        qDebug() << "连接Formationing信号成功！";
    if(connection.connect(QString(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "FormationOver", this, SLOT(getFormationOverSignal(int status, QString failedReason))))
        qDebug() << "连接FormationOver信号成功！";
    if(connection.connect(QString(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "RtspOver", this, SLOT(getRtspOverSignal(int status))))
        qDebug() << "连接RtspOver信号成功！";
    if(connection.connect(QString(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "disconnected", this, SLOT(getdisconnectedSignal())))
        qDebug() << "连接disconnected信号成功！";
}

void Widget::getFormationingSignal(QString deviceName)
{
    if(ui->stackedWidget->currentIndex() != 1)
        ui->stackedWidget->setCurrentIndex(1);
    connectDevice = deviceName;
    FirstPageReset();
    secondPageReset(INITIALED);
    show();
}

void Widget::getConfirmingSignal(QString deviceName)
{
    if(ui->stackedWidget->currentIndex() != 0)
        ui->stackedWidget->setCurrentIndex(0);
    connectDevice = deviceName;
    QString device = QString("%1请求投屏").arg(connectDevice);
    FirstPageReset();
    ui->deviceLabel->setText(device);
    countDownWarning();
    show();
}

void Widget::getFormationOverSignal(int status, QString failedReason)
{
    Q_UNUSED(failedReason)
    if((status == 1) && (ui->stackedWidget->currentIndex() == 1)){
        thirdPageReset(INIT);
        ui->stackedWidget->setCurrentIndex(2);
    }
    else if((status == 0) && (ui->stackedWidget->currentIndex() == 1)){
        secondPageReset(FAIL);
    }
}

void Widget::getRtspOverSignal(int status)
{
    if(ui->stackedWidget->currentIndex() == 2)
    {
        if(status == 1)
            thirdPageReset(SUCCESS);
        else if(status == 0)
            thirdPageReset(FAILED);
    }
}

void Widget::getdisconnectedSignal()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void Widget::switchToPage2(bool verified)
{
    if(ui->stackedWidget->currentIndex() != 1){
        QString pincode;
        ui->stackedWidget->setCurrentIndex(1);
        if(verified)
            pincode = QString("请求设备已经认证过，无需再次输入PIN码!");
        else{
            QString string = INTERACTIONDBUS->getPinCode(1);
            if(string.isEmpty())
                pincode = QString("无需PIN码验证");
            else if(string == QString("getPinFailed"))
                pincode = QString("获取PIN码失败");
            else
                pincode = QString("请输入如下PIN码：%1").arg(string);
        }
        ui->pinLabel->setText(pincode);
    }
}

void Widget::countDownWarning()
{
    if(!timer){
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Widget::updateCountDownWaring);
        timer->start(1000);
    }
    else{
        timer->stop();
        countTimer = 10;
        timer->start(1000);
    }
}

void Widget::updateCountDownWaring()
{
    countTimer--;
    if(countTimer == 0){
        FirstPageReset();
        this->hide();
    }
    QString waring = QString("无操作%1S后自动退出").arg(countTimer);
    ui->warningLabel->setText(waring);
}

void Widget::FirstPageReset()
{
    if(timer){
        timer->stop();
        countTimer = 10;
    }
    ui->warningLabel->setText("无操作10S后自动退出");
}

void Widget::secondPageReset(PINSTATUS status)
{
    switch (status) {
    case INITIAL:
        qDebug() << "配对中";
        switchToPage2(false);
        ui->pinStatusLabel->setText(QString("%1配对中").arg(connectDevice));
        ui->labelTip->setText(QString());
        ui->exitBtn->hide();
        break;
    case INITIALED:
        qDebug() << "配对中";
        switchToPage2(true);
        ui->pinStatusLabel->setText(QString("%1配对中").arg(connectDevice));
        ui->labelTip->setText(QString());
        ui->exitBtn->hide();
        break;
    case FAIL:
        qDebug() << "配对失败";
        ui->pinStatusLabel->setText(QString("%1配对失败").arg(connectDevice));
        ui->labelTip->setText("请直接重新连接，或重新开关投屏功能后再次连接");
        ui->exitBtn->show();
        break;
    }
}

void Widget::thirdPageReset(PLAYERSTATUS status)
{
    switch (status) {
    case INIT:
        ui->exitBtn3->hide();
        ui->playerLabel->setText("请等待  。。。");
        ui->mediaStatusLabel->setText("多媒体协商中");
        break;
    case FAILED:
        ui->exitBtn3->show();
        ui->mediaStatusLabel->setText("多媒体协商失败");
        ui->playerLabel->setText("请直接重新连接，或重新开关投屏功能后再次连接");
        break;
    case SUCCESS:
        ui->exitBtn3->hide();
        ui->mediaStatusLabel->setText("多媒体协商成功");
        ui->playerLabel->setText("播放器准备启动");
        break;
    }
}


void Widget::on_permitBtn_clicked()
{
    secondPageReset(INITIAL);
    FirstPageReset();
}

void Widget::on_forbidBtn_clicked()
{
    this->hide();
    FirstPageReset();
    INTERACTIONDBUS->getPinCode(0);
}

void Widget::on_exitBtn_clicked()
{
    this->hide();
    FirstPageReset();
    ui->stackedWidget->setCurrentIndex(0);
}

void Widget::on_exitBtn3_clicked()
{
    this->hide();
    thirdPageReset(INIT);
    ui->stackedWidget->setCurrentIndex(0);
}

void Widget::on_exitBtn4_clicked()
{
    this->hide();
    ui->stackedWidget->setCurrentIndex(0);
}
