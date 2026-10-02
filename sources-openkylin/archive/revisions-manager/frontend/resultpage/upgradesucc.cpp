#include "upgradesucc.h"
#include <QDBusReply>
UpgradeSucc::UpgradeSucc(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void UpgradeSucc::initui()
{
    mainlayout = new QVBoxLayout();
    dbus = dbusconnect::getInstance();
    QDBusReply<QStringList> ret = dbus->interface->call("getUncompatPackageList");
    iconlabel = new QLabel();
    iconlabel->setFixedSize(50,50);
    iconlabel->setPixmap(QPixmap(":/png/ukui-selected.png").scaled(QSize(48,48)));

    warning = new QLabel();
    warning->setText("升级成功");
//    warning->setStyleSheet("font-size:22px;font-weight:500;line-height: 26px;");
//    warning->setStyleSheet("font-size:20px;line-height: 24px;");

    content = new QTextEdit();
    content->setFrameShape(QFrame::NoFrame);
    content->setTextInteractionFlags(Qt::NoTextInteraction);
    QString compatlist(ret.value().join(","));
    qDebug()<<"uncompat package list:"<<compatlist;
    content->append(QString("新系统中不兼容以下应用：%1").arg(compatlist));
    QSizePolicy content_sizepolicy = content->sizePolicy();
    content_sizepolicy.setRetainSizeWhenHidden(true);
    content->setSizePolicy(content_sizepolicy);
    if(ret.value().length()==0){
        content->hide();
    }
    //这一块应该在接受到信号之后再跑。upgradepage2的showcondition函数之后
//    QStringList *list = upgradepage2.failure;
//    QString var;
//    foreach (var, list) {
//        qDebug()<<var<<"var is var"<<list;
//        content->setText(var);
//    }

    btn = new QPushButton();
    btn->setText("确定");
    btn->setFixedSize(120,48);

    mainlayout->addSpacing(155);
    mainlayout->addWidget(iconlabel,0,Qt::AlignHCenter);
    mainlayout->addSpacing(20);
    mainlayout->addWidget(warning,0,Qt::AlignHCenter);
    mainlayout->addSpacing(16);
    mainlayout->addWidget(content,0,Qt::AlignHCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(btn,0,Qt::AlignHCenter);
    mainlayout->addSpacing(40);

//    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    warning->setStyleSheet("QLabel{background: #FFFFFF}");
//    btn->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
    this->setLayout(mainlayout);
}

void UpgradeSucc::dbusconnection()
{
    connect(btn,&QPushButton::clicked,this,&UpgradeSucc::exitapp);
}

void UpgradeSucc::exitapp()
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}
