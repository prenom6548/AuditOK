#include "upgradefaillist.h"

upgradefaillist::upgradefaillist(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void upgradefaillist::initui()
{
    toplabel = new QLabel();
    toplabel1 = new QLabel();
    deblist = new QListWidget();
    confirm = new QPushButton();
    topwidget = new QWidget();

    topwidget->setFixedWidth(400);
    deblist->setFixedSize(400,320);

    toplabel->setFixedSize(16,16);
    toplabel->setPixmap(QPixmap(":/png/dialog-warning.png").scaled(QSize(16,16)));
    toplabel1->setText("以下应用未迁移成功：");
    confirm->setText("返回");
    confirm->setFixedSize(120,48);

    toplayout = new QHBoxLayout();
    mainlayout = new QVBoxLayout();

    topwidget->setLayout(toplayout);
    toplayout->addWidget(toplabel);
    toplayout->addWidget(toplabel1);

    mainlayout->addSpacing(30);
    mainlayout->addWidget(topwidget,0,Qt::AlignHCenter);
    mainlayout->addWidget(deblist,0,Qt::AlignHCenter);
    mainlayout->addSpacing(15);
    mainlayout->addWidget(confirm,0,Qt::AlignHCenter);
    mainlayout->addSpacing(40);

    QStringList list =getdeblist();
    deblist->addItems(list);

//    topwidget->setStyleSheet("QWidget{background: #FFFFFF}");
//    deblist->setStyleSheet("QListWidget{background: #FFFFFF}");
//    confirm->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF}");
    this->setLayout(mainlayout);
//    qDebug()<<"the failure list is "<<list;
//    QString var;
//    foreach (var, list) {
//        qDebug()<<"var deb is "<<var;
//        deblist->addItems();
//    }
}

void upgradefaillist::dbusconnection()
{
    connect(confirm,&QPushButton::clicked,this,&upgradefaillist::backtopage);
}

void upgradefaillist::backtopage()
{
    emit backtopagesignal();
}

QStringList upgradefaillist::getdeblist()
{
    QDBusInterface upgrade_interface( "com.kylin.RevisionsManager",
                                     "/com/kylin/RevisionsManager",
                                     "com.kylin.RevisionsManager.interface",
                                     QDBusConnection::systemBus() );
    QDBusMessage modelist = upgrade_interface.call("getFailedPackageList");
    qDebug()<<"the failed package list is"<<modelist;
    QStringList reason = modelist.arguments().value(0).toStringList();
    return reason;
}
