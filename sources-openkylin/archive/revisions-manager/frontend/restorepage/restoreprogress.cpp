#include "restoreprogress.h"

restoreprogress::restoreprogress(QWidget *parent) : QWidget(parent)
{
    initialdbus();
    initui();
    dbusconnection();
}

void restoreprogress::initui()
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

    title->setText("正在还原，请勿关闭电脑。");
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

void restoreprogress::dbusconnection()
{
    dbus = dbusconnect::getInstance();
    qDebug()<<"还原信号建立";
    connect(dbus->interface,SIGNAL(fallbackProcessSig(QString,int,int,QString)),this,SLOT(showcondition(QString,int,int,QString)));
    //connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
}

void restoreprogress::initialdbus()
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

void restoreprogress::showcondition(QString itermsNum,int state,int process,QString suggestion)
{
    qDebug()<<itermsNum<<state<<process<<suggestion;
    qDebug()<<"收到还原进度信号";
    progress->setValue(process);
    progress->update();
    if(state == -1)
    {
        qDebug()<<"upgradefailsignal";
        failure.append(QString("%1").arg(itermsNum));
        qDebug()<<"failure list is "<<failure;
        Suggest.append(QString("%1").arg(suggestion));
        emit upgradefailsignal();
        return;
    }
    else
    {
        static int pro = 0;
        if(process != pro)
        {
            bottom->setText(QString("%1").arg(itermsNum));
//            middle->setText(QString("%1").arg(process)+"%");
    //        conditionbar->addScrollBarWidget(label,Qt::AlignHCenter | Qt::AlignTop);
            if(state != 0)//对于安装，state完全不一样
            {
//                label->setEnabled(false);
                failure.append(QString("%1").arg(itermsNum));
                Suggest.append(QString("%1").arg(suggestion));
                qDebug()<<"failure list is "<<failure;
            }
            else//还原失败
            {
                failure.append(QString("%1").arg(itermsNum));
                Suggest.append(QString("%1").arg(suggestion));
                qDebug()<<"restore failure";
                qDebug()<<"failure lsit is "<<failure;
            }
            pro = process;
        }
        if(process == 100)
        {
            qDebug()<<"emit upgradesuccesssignal，进度完成至100%";
            emit upgradesuccesssignal();
        }
    }
}
