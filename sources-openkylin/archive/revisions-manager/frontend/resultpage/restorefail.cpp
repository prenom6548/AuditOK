#include "restorefail.h"

RestoreFail::RestoreFail(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void RestoreFail::initui()
{
    mainlayout = new QVBoxLayout();

    iconlabel = new QLabel();
    iconlabel->setFixedSize(50,50);
    iconlabel->setPixmap(QPixmap(":/png/dialog-error.png").scaled(QSize(48,48)));

    warning = new QLabel();
    warning->setText("还原失败");
//    warning->setStyleSheet("font-size:22px;font-weight:500;line-height: 26px;");
//    warning->setStyleSheet("font-size:20px;line-height: 24px;");

    content = new QTextEdit();
    content->setFrameShape(QFrame::NoFrame);
    content->setTextInteractionFlags(Qt::NoTextInteraction);
    content->append("详情：");
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
//    btn->setStyleSheet("QPushButton{background: #3790FA;color:#black}");
    this->setLayout(mainlayout);

}

void RestoreFail::dbusconnection()
{
    connect(btn,&QPushButton::clicked,this,&RestoreFail::exitapp);
}

void RestoreFail::exitapp()
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}
