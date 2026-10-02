#include "upgradeprocess.h"

upgradeprocess::upgradeprocess(QWidget *parent) : QWidget(parent)
{
    initialdbus();
    initui();
    dbusconnection();
}


void upgradeprocess::initui()
{
//    icon = new QLabel();
    title = new QLabel();
//    middle = new QLabel();
    bottom = new QLabel();
    progress = new KProgressCircle();

    main = new QVBoxLayout();

    midwidget = new QWidget();
    boxlayout = new QHBoxLayout();
    midwidget->setLayout(boxlayout);

    topwidget = new QWidget();
    titlelayout = new QHBoxLayout();
    topwidget->setLayout(titlelayout);

    title->setText("正在升级，请勿关闭电脑。");
//    icon->setFixedSize(24,24);
//    icon->setPixmap(QPixmap(":/png/loading10.svg").scaled(QSize(24,24)));

//    progress->setValue(0);
//    progress->setFixedHeight(6);
//    progress->setTextVisible(false);
//    progress->setStyleSheet("QProgressBar{background:white;} QProgressBar::chunk{background:blue}");

//    titlelayout->addWidget(icon);
    titlelayout->addWidget(title);

    boxlayout->addWidget(progress);
//    boxlayout->addWidget(middle);

    main->addStretch();
    main->addWidget(midwidget);
    main->addWidget(topwidget,0,Qt::AlignHCenter);

    main->addWidget(bottom,0,Qt::AlignHCenter);
    main->addStretch();

//    topwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    icon->setStyleSheet("QLabel{background: #FFFFFF}");
//    title->setStyleSheet("QLabel{background: #FFFFFF}");
//    midwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    bottom->setStyleSheet("QLabel{background: #FFFFFF}");
    this->setLayout(main);

}

void upgradeprocess::dbusconnection()
{
    dbus = dbusconnect::getInstance();
    qDebug()<<"第二页dbus链接";
    connect(dbus->interface,SIGNAL(upgradeProcessSig(QString,int,int,QString)),this,SLOT(showcondition(QString,int,int,QString)));
    //connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
}

void upgradeprocess::initialdbus()
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

void upgradeprocess::showcondition(QString itermsNum,int state,int process,QString suggestion)
{
    qDebug()<<itermsNum<<state<<process<<suggestion;
    qDebug()<<"收到完成进度信号";
    progress->setValue(process);
    progress->update();
    if(state == -1)
    {
        qDebug()<<"upgradefailsignal";
            if(tmp != process)
            {
                failure.append(QString("%1").arg(itermsNum));
                Suggest.append(QString("%1").arg(suggestion));
                tmp = process;
            }
        qDebug()<<"failure list is "<<failure;
        emit upgradefailsignal();
        disconnect(dbus->interface,SIGNAL(upgradeProcessSig(QString,int,int,QString)),this,SLOT(showcondition(QString,int,int,QString)));
        return;
    }
    else
    {
//        static int pro = 0;
//        if(process != pro)
//        {
            bottom->setText(QString("%1").arg(itermsNum));
//            middle->setText(QString("%1").arg(process)+"%");
            if(state != 0)//对于安装，state完全不一样
            {
//                label->setEnabled(false);
                failure.append(QString("%1").arg(itermsNum));
                Suggest.append(QString("%1").arg(suggestion));
                qDebug()<<"failure list is "<<failure;
            }
    //        else
    //        {
    //            failure.append(QString("%1").arg(itermsNum));
    //            qDebug()<<"failure lsit is "<<failure;
    //        }
//            pro = process;
//        }
        if(process == 100)
        {
            qDebug()<<"upgradesuccesssignal";
            emit upgradesuccesssignal();
        }
    }
}
