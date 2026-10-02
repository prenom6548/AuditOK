#include "upgradepage2.h"

upgradepage2::upgradepage2(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void upgradepage2::initui()
{
    title = new QLabel();
    bottom = new QLabel();
    progress = new KProgressCircle();

    main = new QVBoxLayout();

    midwidget = new QWidget();
    boxlayout = new QHBoxLayout();
    midwidget->setLayout(boxlayout);

    topwidget = new QWidget();
    titlelayout = new QHBoxLayout();
    topwidget->setLayout(titlelayout);

    title->setText("正在检测是否符合升级条件");
    titlelayout->addWidget(title);

    boxlayout->addWidget(progress);

    main->addStretch();
    main->addWidget(midwidget);
    main->addWidget(topwidget,0,Qt::AlignHCenter);//topwidget改title
    main->addWidget(bottom,0,Qt::AlignHCenter);
    main->addStretch();

//    topwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    icon->setStyleSheet("QLabel{background: #FFFFFF}");
//    title->setStyleSheet("QLabel{background: #FFFFFF}");
//    midwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    bottom->setStyleSheet("QLabel{background: #FFFFFF}");
    finishsignalsent = false;
    this->setLayout(main);

}

void upgradepage2::dbusconnection()
{
    dbus = dbusconnect::getInstance();
    qDebug()<<"第二页dbus链接";
    connect(dbus->interface,SIGNAL(upgradeCheckSig(QString,int,int,QString)),this,SLOT(showcondition(QString,int,int,QString)));
    //connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
}

void upgradepage2::showcondition(QString itermsNum,int state,int process,QString suggestion)
{
    qDebug()<<itermsNum<<state<<process<<suggestion;
    qDebug()<<"接收到预检测信号";
    progress->setValue(process);
        bottom->setText(QString("%1").arg(itermsNum));
        if(state != 0)
        {           
            failure.append(QString("%1").arg(itermsNum));
            Suggest.append(QString("%1").arg(suggestion));
            qDebug()<<"failure list is "<<failure;
        }
    if(process == 100)
    {
        qDebug()<<"checkupdatefinishsignal";
        if(finishsignalsent)
        {
            ;
        }
        else
        {
            finishsignalsent = true;
            emit checkupdatefinishsignal();
        }
    }
}
