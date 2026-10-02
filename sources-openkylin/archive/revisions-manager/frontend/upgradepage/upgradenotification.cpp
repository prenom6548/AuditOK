#include "upgradenotification.h"
#include "dbusconnect.h"

upgradenotification::upgradenotification(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void upgradenotification::initui()
{
    main = new QVBoxLayout();
    QWidget *titlewidget = new QWidget(this);
    titlelayout = new QHBoxLayout();
    btnwidget = new QWidget();
    btnlayout = new QHBoxLayout();
    title = new QLabel();
    content = new QTextEdit();
    nextbtn = new QPushButton();
    backbtn = new QPushButton();
    check = new QCheckBox();


    title->setText("升级须知");
//    title->setStyleSheet("font-size:22px;font-weight:500;line-height: 26px;");
    nextbtn->setText("下一步");
    nextbtn->setFixedSize(120,48);
    nextbtn->setEnabled(false);
    backbtn->setText("上一步");
    backbtn->setFixedSize(120,48);

    // dbusconnect *dbus = dbusconnect::getInstance();
    // QDBusReply<QString> versionStr = dbus->interface->call("getCurOsVersion");
    // QString path = ":/res/EULA-kylin";
    // if (versionStr.value().contains("openkylin", Qt::CaseInsensitive))

    QString path = ":/res/EULA-openKylin";

    /*load license*/
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        qDebug() << "license Can not open";
    QTextStream in(&file);
    QString m_fileStr = in.readAll();
    content->setText(m_fileStr);
    file.close();

    content->setFixedSize(400,360);
    content->setReadOnly(true);
    content->viewport()->setAutoFillBackground(true);
    content->viewport()->setBackgroundRole(QPalette::Window);
    check->setText("已阅读并同意协议内容");
    titlewidget->setLayout(titlelayout);
    titlelayout->setContentsMargins(0,0,0,0);
    titlelayout->addStretch();
    titlelayout->addWidget(title);
    titlelayout->addStretch();
    btnwidget->setFixedWidth(260);

    btnwidget->setLayout(btnlayout);
    btnlayout->addWidget(backbtn);
    btnlayout->addSpacing(16);
    btnlayout->addWidget(nextbtn);

//    main->addSpacing(40);
    main->addWidget(titlewidget);
//    main->addStretch();
//    main->addSpacing(20);
    main->addWidget(content,0,Qt::AlignHCenter);
//    main->addSpacing(20);
//    main->addStretch();
//    main->setMargin(0);
//    main->addSpacing(16);
    main->addWidget(check,0,Qt::AlignHCenter);
//    main->addSpacing(16);
    main->addStretch();
    main->addWidget(btnwidget,0,Qt::AlignHCenter);
    main->addSpacing(30);

//    btnwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    title->setStyleSheet("QLabel{background: #FFFFFF;font-size:22px;font-weight:500;line-height: 26px;}");
//    check->setStyleSheet("QCheckBox{background: #FFFFFF}");
    check->setFocusPolicy(Qt::NoFocus);
//    backbtn->setStyleSheet("QPushButton{background: #F2F2F2}");
//    backbtn->setStyleSheet("QPushButton{background: #F2F2F2}");
    backbtn->setFocusPolicy(Qt::NoFocus);
    nextbtn->setFocusPolicy(Qt::NoFocus);
    this->setLayout(main);
}

void upgradenotification::dbusconnection()
{
    connect(backbtn,&QPushButton::clicked,this,&upgradenotification::gotofirstpage);
    connect(nextbtn,&QPushButton::clicked,this,&upgradenotification::nextpage);
    connect(check,SIGNAL(stateChanged(int)),this,SLOT(statechange(int)));
//    connect()
}

void upgradenotification::statechange(int num)
{
    qDebug()<<num<<"!!!!!!";
    if(num == 2)
    {
        nextbtn->setEnabled(true);
//        nextbtn->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
    }
    else
    {
        nextbtn->setEnabled(false);
//        nextbtn->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626}");
    }
}


void upgradenotification::gotofirstpage()
{
    qDebug()<<"notification to page1";
    emit gotofistpagesignal();
}

void upgradenotification::nextpage()
{
    qDebug()<<"notification to nextpage";
    emit nextpagesignal();
}
