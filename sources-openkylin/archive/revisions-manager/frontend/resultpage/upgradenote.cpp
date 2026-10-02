#include "upgradenote.h"

UpgradeNote::UpgradeNote(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void UpgradeNote::initui()
{
//    mainlayout = new QVBoxLayout();

//    iconlabel = new QLabel();
//    iconlabel->setFixedSize(50,50);
//    iconlabel->setPixmap(QPixmap(":/png/dialog-info.png").scaled(QSize(48,48)));

//    warning = new QLabel();
//    warning->setText("升级成功");
////    warning->setStyleSheet("font-size:22px;font-weight:500;line-height: 26px;");
////    warning->setStyleSheet("font-size:20px;line-height: 24px;");

//    content = new QTextEdit();
//    content->setFrameShape(QFrame::NoFrame);
//    content->setTextInteractionFlags(Qt::NoTextInteraction);
//    content->append("详情：");

////    deblist = new QPushButton();
////    deblist->setText("了解详情");
////    deblist->setFlat(true);
////    deblist->setStyleSheet("QPushButton{color: #3790FA;background: #FFFFFF}");

//    //这一块应该在接受到信号之后再跑。upgradepage2的showcondition函数之后
////    QStringList *list = upgradepage2.failure;
////    QString var;
////    foreach (var, list) {
////        qDebug()<<var<<"var is var"<<list;
////        content->setText(var);
////    }

//    btn = new QPushButton();
//    btn->setText("确定");
//    btn->setFixedSize(120,48);

//    mainlayout->addSpacing(155);
//    mainlayout->addWidget(iconlabel,0,Qt::AlignHCenter);
//    mainlayout->addSpacing(20);
//    mainlayout->addWidget(warning,0,Qt::AlignHCenter);
//    mainlayout->addSpacing(16);
//    mainlayout->addWidget(content,0,Qt::AlignHCenter);//deblist替换content
//    mainlayout->addStretch();
//    mainlayout->addWidget(btn,0,Qt::AlignHCenter);
//    mainlayout->addSpacing(40);

//    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    warning->setStyleSheet("QLabel{font-size:20px;line-height: 24px;background: #FFFFFF}");
//    btn->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
//    this->setLayout(mainlayout);

    mainlayout = new QVBoxLayout();

    iconlabel = new QLabel();
    iconlabel->setFixedSize(50,50);
    iconlabel->setPixmap(QPixmap(":/png/dialog-info.png").scaled(QSize(48,48)));

    warning = new QLabel();
    warning->setText("升级成功");

midwidget = new QWidget();
midlabel = new QLabel();
midbtn = new QPushButton();
midlayout = new QHBoxLayout();
midlabel->setText("存在未升级成功的应用 ");
midbtn->setText("查看详情");
midwidget->setLayout(midlayout);
midlayout->addWidget(midlabel);
midlayout->addWidget(midbtn);
midbtn->setFlat(true);

suggestionlabel = new QLabel();
suggestionlabel->setText("建议从软件商店安装最新版本");

    btn = new QPushButton();
    btn->setText("确定");
    btn->setFixedSize(120,48);

    mainlayout->addSpacing(155);
    mainlayout->addWidget(iconlabel,0,Qt::AlignHCenter);
    mainlayout->addSpacing(20);
    mainlayout->addWidget(warning,0,Qt::AlignHCenter);
    mainlayout->addSpacing(16);
    mainlayout->addWidget(midwidget,0,Qt::AlignHCenter);//midwidget替换content
    mainlayout->addSpacing(8);
    mainlayout->addWidget(suggestionlabel,0,Qt::AlignHCenter);
    mainlayout->addStretch();
    mainlayout->addWidget(btn,0,Qt::AlignHCenter);
    mainlayout->addSpacing(40);

//    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    suggestionlabel->setStyleSheet("QLabel{background: #FFFFFF}");
//    warning->setStyleSheet("QLabel{font-size:20px;line-height: 24px;background: #FFFFFF}");
//    btn->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
//    midwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    midbtn->setStyleSheet("QPushButton{color:#3790FA}");
    this->setLayout(mainlayout);

}

void UpgradeNote::showdeblist()
{
    qDebug()<<"UpgradeNote::showdeblist";
    emit showdeblistsignal();
}

void UpgradeNote::dbusconnection()
{
    connect(btn,&QPushButton::clicked,this,&UpgradeNote::exitapp);
    connect(midbtn,&QPushButton::clicked,this,&UpgradeNote::showdeblist);
}

void UpgradeNote::exitapp()
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}

