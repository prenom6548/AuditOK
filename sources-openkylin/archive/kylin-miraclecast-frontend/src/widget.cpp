#include "widget.h"
#include "ui_widget.h"
#include "dialog.h"
#include <QDBusConnection>
#include <QTimer>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QPalette>
#include <QScreen>
#include "../dialog.h"
#include "../dbus/interfacedbusproxy.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), timer(nullptr), countTimer(10), connectDevice(QString())
{
    ui->setupUi(this);
    b =new Dialog ;
    this->discon_flag=1;



    ui->permitBtn->setVisible(0);
    ui->forbidBtn->setVisible(0);
    this->setFixedSize(1,1);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect position = screen->availableGeometry();

    ui->deviceLabel->setFont(QFont("Timers", 24, QFont::Bold));
    ui->warningLabel->setFont(QFont("Timers", 12, QFont::Bold));
    QPalette pa;
    pa.setColor(QPalette::WindowText, Qt::red);
    ui->warningLabel->setPalette(pa);


    ui->pinLabel->setFont(QFont("Timers", 20, QFont::Bold));

    ui->mediaStatusLabel->setFont(QFont("Timers", 24, QFont::Bold));
    thirdPageReset(INIT);

    ui->label->setFont(QFont("Timers", 24, QFont::Bold));

    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->setStyleSheet("background-color:#fcfcfc");

    connect(b,SIGNAL(refuse_btn_signal()),this,SLOT(on_forbidBtn_clicked()));
    connect(b,SIGNAL(allow_btn_signal()),this,SLOT(on_permitBtn_clicked()));

    dbusSignalInit();    

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
    QDBusConnection connection = QDBusConnection::sessionBus();

    if(connection.connect(getServiceName(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "Formationing", this, SLOT(getFormationingSignal(QString))))
        qDebug() << "连接Formationing信号成功！";
    if(connection.connect(getServiceName(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "FormationOver", this, SLOT(getFormationOverSignal(int, QString))))
        qDebug() << "连接FormationOver信号成功！";
    if(connection.connect(getServiceName(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "RtspOver", this, SLOT(getRtspOverSignal(int))))
        qDebug() << "连接RtspOver信号成功！";
    if(connection.connect(getServiceName(), getDbusSiganlPath(), getDbusInterfaceName(),
                       "disconnected", this, SLOT(getdisconnectedSignal())))
        qDebug() << "连接disconnected信号成功！";
}

void Widget::showInitWin(void)
{
    if(INIT_WIDGET_WITH_CONFIRM == _iInitWinType)
        initConfirmingWin();
    else if(INIT_WIDGET_WITHOUT_CONFIRM == _iInitWinType)
        initFormationingWin();
    else
        qDebug() << "invalid window type "<<_iInitWinType;
}

void Widget::initConfirmingWin(void)
{
    qDebug() << __func__;
    if(ui->stackedWidget->currentIndex() != 0)
        ui->stackedWidget->setCurrentIndex(0);
    QString device = QString("%1 %2").arg(connectDevice).arg(tr("request cast."));//请求投屏
    FirstPageReset();
    ui->deviceLabel->setText(device);
    countDownWarning();
    activateWindow();
    show();
    b->set_label_text(device);
    b->set_label_pin_text("");
    b->set_button(1);

}

void Widget::initFormationingWin(void)
{
    qDebug() << __func__;
    if(ui->stackedWidget->currentIndex() != 1)
        ui->stackedWidget->setCurrentIndex(1);
    FirstPageReset();
    secondPageReset(INITIALED);
    activateWindow();
    show();
}

void Widget::getFormationingSignal(QString deviceName)
{
    qDebug() << __func__;
    if(ui->stackedWidget->currentIndex() != 1)
        ui->stackedWidget->setCurrentIndex(1);
    connectDevice = deviceName;
    FirstPageReset();
    secondPageReset(INITIALED);
    activateWindow();
    show();
}

void Widget::getFormationOverSignal(int status, QString failedReason)
{
    qDebug() << __func__ << status << failedReason;
    Q_UNUSED(failedReason)
    if((status == 1) && (ui->stackedWidget->currentIndex() == 1)){
        thirdPageReset(INIT);
        ui->stackedWidget->setCurrentIndex(2);
        this->hide();
    }
    else if((status == 0) && (ui->stackedWidget->currentIndex() == 1)){
        secondPageReset(FAIL);
    }
}

void Widget::getRtspOverSignal(int status)
{
    qDebug() << __func__;

        if(status == 1)
            thirdPageReset(SUCCESS);
        else if(status == 0)
            thirdPageReset(FAILED);
}

void Widget::getdisconnectedSignal()
{
    if(this->discon_flag==1)
    {
        this->discon_flag=0;
    }
    else{
        qDebug()<<"Recv second disc-signal,so ignore it";
        return ;
    }


    qDebug() << __func__;
    this->show();
    ui->stackedWidget->setCurrentIndex(3);
    this->hide();
    b->set_visible(1);
    b->set_label_text(tr("The device is disconnected！"));
    b->set_button_2();
}

void Widget::switchToPage2(bool verified)
{
    QString pincode;
    QString msg_pin="";
    ui->stackedWidget->setCurrentIndex(1);
    this->hide();
    if(verified)
    {
        pincode = QString("请求设备已经认证过，无需再次输入PIN码!");
        ui->pinLabel->setText(pincode);
        this->hide();
        return;
    }
    else{
        msg_pin=INTERACTIONDBUS->getPinCode(1);
        if(msg_pin.isEmpty())
            pincode = QString("无需PIN码验证");
        else if(msg_pin == QString("getPinFailed"))
            pincode = QString("获取PIN码失败");
        else
            pincode = QString("%1 请在手机端输入以下PIN码：%2").arg(connectDevice).arg(msg_pin);
    }
    ui->pinLabel->setText(msg_pin);
    if(msg_pin.isEmpty())
        b->set_label_text(QString("%1%2%3").arg(connectDevice).arg(" ").arg(tr("request cast.")));
    else
        b->set_label_text(QString("%1%2%3：").arg(connectDevice).arg(" ").arg(tr("request cast.Please enter the pin code on mobile")));
    b->set_label_pin_text(msg_pin);
    b->set_button(0);
    b->show();
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
    QLocale locale;
    QString waring;
    countTimer--;

    if(countTimer == 0){
        FirstPageReset();
        this->hide();
    }
    if (locale.language() == QLocale::Chinese){
        waring = QString("无操作%1S后自动退出。").arg(countTimer);
    }else if (locale.language() == QLocale::English){
        waring = QString("Exit after %1S without operation.").arg(countTimer);
    }
    ui->warningLabel->setText(waring);
    b->set_warningLabel_text(waring);
}

void Widget::FirstPageReset()
{
    if(timer){
        timer->stop();
        countTimer = 10;
    }
    ui->warningLabel->setText("无操作10S后自动退出。");
}

void Widget::secondPageReset(PINSTATUS status)
{
    b->set_countdown(0);
    switch (status) {
    case INITIAL:
        qDebug() << "配对中 INITIAL";
        switchToPage2(false);
        this->hide();
        ui->pinStatusLabel->setText(QString("%1配对中").arg(connectDevice));
        ui->labelTip->setText(QString());
        ui->exitBtn->hide();
        break;
    case INITIALED:
        qDebug() << "配对中 INITIALED";
        switchToPage2(true);
        this->hide();
        ui->pinStatusLabel->setText(QString("%1配对中").arg(connectDevice));
        ui->labelTip->setText(QString());
        ui->exitBtn->hide();
        break;
    case FAIL:
        qDebug() << "配对失败";
        ui->pinStatusLabel->setText(QString("%1配对失败").arg(connectDevice));
        ui->labelTip->setText("请直接重新连接，或重新开关投屏功能后再次尝试");
        ui->exitBtn->show();
        b->set_label_text(QString(tr("Pairing failed, please reconnect...")));
        b->set_label_pin(0);
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
        if(connectDevice==NULL)//未获取到设备名称时将提示信息改为正在初始化
        {
            b->set_label_text(QString(tr("The environment is initializing, please wait")));
        }
        else
        {
            b->set_label_text(QString("%1%2%3").arg(connectDevice).arg(" ").arg(tr("request cast.Connecting......")));
        }
        b->set_label_pin_text("");
        b->set_button(0);
        b->show();
        this->setVisible(0);
        break;
    case FAILED:
        ui->exitBtn3->show();
        ui->mediaStatusLabel->setText("多媒体协商失败");
        ui->playerLabel->setText("请直接重新连接，或重新开关投屏功能后再次连接");
        b->set_label_text(QString("%1%2%3").arg(connectDevice).arg(" ").arg(tr("request cast.Multimedia connect failed")));
        break;
    case SUCCESS:
        ui->exitBtn3->hide();
        ui->mediaStatusLabel->setText("多媒体协商成功");
        ui->playerLabel->setText("开始投屏");
        b->set_label_text(QString("%1%2%3").arg(connectDevice).arg(" ").arg(tr("request cast.Multimedia connect succeeded")));
        b->set_visible(0);
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
    secondPageReset(INITIAL);
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
