#include "restorepage1.h"

restorepage1::restorepage1(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void restorepage1::initui()
{
    mainlayout = new QVBoxLayout(this);
    iconlayout = new QHBoxLayout(this);
    versionlayout = new QHBoxLayout(this);
    tiplayout = new QHBoxLayout(this);
    upgradelayout = new QHBoxLayout(this);
    bottomlayout = new QHBoxLayout(this);

    iconlabel = new QLabel(this);
    version = new QLabel(this);
    restorebtn = new QPushButton(this);
    tiplabel = new QLabel(this);
    attentionbtn = new QPushButton(this);
//    exitbtn = new QPushButton(this);

    dbusconnect *dbus = dbusconnect::getInstance();
    QDBusReply<QString> versionStr = dbus->interface->call("getCurOsVersion");
    version->setText(QString("当前版本：%1").arg(versionStr.value()));

    // iconlabel->setFixedSize(93,93);
    // iconlabel->setPixmap(QPixmap(":/kylin.png").scaled(QSize(93,93)));
    // qDebug() << "str is " << versionStr.value();
    // if (versionStr.value().contains("openkylin", Qt::CaseInsensitive))
    {
        iconlabel->setFixedSize(200,52);
        iconlabel->setPixmap(QPixmap(":/openKylin.png").scaled(QSize(200,52)));
    }

    restorebtn->setText("还原");
    restorebtn->setFixedWidth(300);
    restorebtn->setFixedHeight(56);
//    restorebtn->setStyleSheet("background-color:#3790FA;font-size:22px;color:white");

    attentionbtn->setText("注意事项");
    attentionbtn->setFixedWidth(120);
    attentionbtn->setFlat(true);
//    exitbtn->setText("退出应用");
//    exitbtn->setFixedWidth(120);
//    exitbtn->setFlat(true);

    iconlayout->addStretch();
    iconlayout->addWidget(iconlabel);
    iconlayout->addStretch();
    versionlayout->addStretch();
    versionlayout->addWidget(version);
    versionlayout->addStretch();
    upgradelayout->addWidget(restorebtn);
    tiplayout->addStretch();
    tiplayout->addWidget(tiplabel);
    tiplayout->addStretch();
    bottomlayout->addStretch();
    bottomlayout->addWidget(attentionbtn,Qt::AlignHCenter);
    bottomlayout->addStretch();
//    bottomlayout->addWidget(exitbtn,Qt::AlignRight);

    mainlayout->addSpacing(90);
    mainlayout->addLayout(iconlayout);
    mainlayout->addLayout(versionlayout);
    mainlayout->addSpacing(40);
    mainlayout->addLayout(upgradelayout);
    mainlayout->addSpacing(5);
    mainlayout->addLayout(tiplayout);
    mainlayout->addSpacing(200);
    mainlayout->addLayout(bottomlayout);
    mainlayout->addSpacing(20);

//    tiplabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    version->setStyleSheet("QLabel{background: #FFFFFF}");
//    attentionbtn->setStyleSheet("QPushButton{background: #FFFFFF}");
//    exitbtn->setStyleSheet("QPushButton{background: #FFFFFF}");
    this->setLayout(mainlayout);

    dbus = dbusconnect::getInstance();
    QDBusReply<int> reply=dbus->interface->asyncCall("isOverTime");
    over=reply.value();
    if(over == 0)
        restorebtn->setEnabled(true);
    else
    {
        restorebtn->setEnabled(false);
        tiplabel->setText("该功能已过期");
//        tiplabel->setStyleSheet("color:red");
    }
}

void restorepage1::dbusconnection()
{
    connect(restorebtn,SIGNAL(clicked(bool)),this,SLOT(emitsignal(bool)));
    connect(attentionbtn,SIGNAL(clicked(bool)),this,SLOT(showattention(bool)));
//    connect(exitbtn,SIGNAL(clicked(bool)),this,SLOT(exitapp(bool)));

    dbus = dbusconnect::getInstance();
    qDebug()<<"第一页dbus链接";
}

void restorepage1::emitsignal(bool)
{
    qDebug()<<"restore";
    emit nextpage();

}

void restorepage1::showattention(bool)
{
    qDebug()<<"restorepage1 showattention";
    this->close();
    emit showattentionsignal();
}

void restorepage1::exitapp(bool)
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}
