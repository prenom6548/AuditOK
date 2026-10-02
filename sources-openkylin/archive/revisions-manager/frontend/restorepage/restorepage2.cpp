#include "restorepage2.h"

restorepage2::restorepage2(QWidget *parent) : QWidget(parent)
{
    initialdbus();
    initui();
    dbusconnection();
}

void restorepage2::initui()
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

    title->setText("正在检测是否符合还原条件");

    titlelayout->addWidget(title);
    boxlayout->addWidget(progress);

    main->addStretch();
    main->addWidget(midwidget);
    main->addWidget(topwidget,0,Qt::AlignHCenter);
    main->addWidget(bottom,0,Qt::AlignHCenter);
    main->addStretch();

    this->setLayout(main);
}

void restorepage2::dbusconnection()
{
    dbus = dbusconnect::getInstance();
    qDebug()<<"第二页dbus链接";
    connect(dbus->interface,SIGNAL(fallbackCheckSig(QString,int,int,QString)),this,SLOT(showcondition(QString,int,int,QString)));
    //connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
}

void restorepage2::initialdbus()
{
//    dbus = dbusconnect::getInstance();
//    connect(dbus->interface,SIGNAL(upgradeCheckSig(int,int,int)),this,SLOT(showprogress(int,int,int)));
//    m_timer = new QTimer();
//    m_timer->setInterval(100);
//    connect(m_timer,&QTimer::timeout, [this] () {
////            QIcon icon;
//            QString str = QString(":/png/loading%1.svg").arg(i);
////            icon.addFile(str);
////            this->setIcon(icon);
//            icon->setPixmap(QPixmap(str).scaled(QSize(24,24)));
//            i++;
//            if(i==18){//8个图片
//                i=10;
//            }
//    });
}

void restorepage2::showcondition(QString itermsNum,int state,int process,QString suggestion)
{
    qDebug()<<itermsNum<<state<<process<<suggestion;
    qDebug()<<"第二页收到检测完成信号啦";
    progress->setValue(process);

    static int pro = 0;
    if(process != pro)
    {
        bottom->setText(QString("%1").arg(itermsNum));
//        middle->setText(QString("%1").arg(process)+"%");
        if(state != 0)
        {
            failure.append(QString("%1").arg(itermsNum));
            Suggest.append(QString("%1").arg(suggestion));
            qDebug()<<"failure list is "<<failure;
        }
//        else
//        {
//            failure.append(QString("%1").arg(itermsNum));
//            qDebug()<<"failure lsit is "<<failure;
//        }
        pro = process;
    }
    if(process == 100)
    {
        qDebug()<<"checkupdatefinishsignal";
        emit checkupdatefinishsignal();
    }
}
