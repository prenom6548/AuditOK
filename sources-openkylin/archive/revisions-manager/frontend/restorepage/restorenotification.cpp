#include "restorenotification.h"

restorenotification::restorenotification(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void restorenotification::initui()
{
    main = new QVBoxLayout();
    titlelayout = new QHBoxLayout();
    btnlayout = new QHBoxLayout();
    title = new QLabel();
    content = new QTextEdit();
    nextbtn = new QPushButton();
    backbtn = new QPushButton();
    check = new QCheckBox();

    title->setText("还原须知");
    nextbtn->setText("下一步");
    nextbtn->setEnabled(false);
    backbtn->setText("返回首页");
    content->setText("啥东西也没有");
    check->setText("已阅读并同意协议内容");
    titlelayout->addStretch();
    titlelayout->addWidget(title);
    titlelayout->addStretch();

    main->addLayout(titlelayout);
    main->addWidget(content);
    main->addSpacing(10);
    main->addWidget(check,0,Qt::AlignHCenter);
    main->addLayout(btnlayout);
    btnlayout->addWidget(nextbtn);
    btnlayout->addWidget(backbtn);

//    btnwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    title->setStyleSheet("QLabel{background: #FFFFFF}");
//    check->setStyleSheet("QCheckBox{background: #FFFFFF}");
    check->setFocusPolicy(Qt::NoFocus);
//    backbtn->setStyleSheet("QPushButton{background: #F2F2F2}");
//    backbtn->setStyleSheet("QPushButton{background: #F2F2F2}");
    backbtn->setFocusPolicy(Qt::NoFocus);
    nextbtn->setFocusPolicy(Qt::NoFocus);
    this->setLayout(main);
}

void restorenotification::dbusconnection()
{
    connect(backbtn,&QPushButton::clicked,this,&restorenotification::gotofirstpage);
    connect(nextbtn,&QPushButton::clicked,this,&restorenotification::nextpage);
    connect(check,SIGNAL(stateChanged(int)),this,SLOT(statechange(int)));
//    connect()
}

void restorenotification::statechange(int num)
{
    qDebug()<<num<<"!!!!!!";
    if(num == 2)
        nextbtn->setEnabled(true);
    else
        nextbtn->setEnabled(false);
}


void restorenotification::gotofirstpage()
{
    qDebug()<<"notification to page1";
    emit gotofistpagesignal();
}

void restorenotification::nextpage()
{
    qDebug()<<"notification to nextpage";
    emit nextpagesignal();
}

