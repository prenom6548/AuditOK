#include "initpage.h"

initpage::initpage(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void initpage::initui()
{
//    this->setStyleSheet("QWidget{background: #FFFFFF}");

//    closeBtn = new QPushButton(this);
//    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
//    closeBtn->setFlat(true);
//    closeBtn->setFixedSize(QSize(30, 30));
//    closeBtn->setProperty("isWindowButton", 0x2);
//    closeBtn->setProperty("useIconHighlightEffect", 0x08);

    mainlayout = new QVBoxLayout(this);
    iconlabel = new QLabel(this);
    UpgradeBtn = new QPushButton(this);
    UpgradeBtn->setText("系统升级");
    UpgradeBtn->setFixedHeight(50);
    UpgradeBtn->setFixedWidth(300);
    UpgradeBtn->setFocusPolicy(Qt::NoFocus);
    RestoreBtn = new QPushButton(this);
    RestoreBtn->setText("系统还原");
    RestoreBtn->setEnabled(true);

    dbusconnect *dbus = dbusconnect::getInstance();
    QDBusReply<int> mode = dbus->interface->call("isOldOsExists");
    if (mode.isValid())
    {
        if(mode == false)
        {
            RestoreBtn->setEnabled(false);
        }
        else
        {
            RestoreBtn->setEnabled(true);
        }
    }
    {
        iconlabel->setFixedSize(200,52);
        iconlabel->setPixmap(QPixmap(":/openKylin.png").scaled(QSize(200,52)));
    }

    RestoreBtn->setFixedHeight(50);
    RestoreBtn->setFixedWidth(300);    
    RestoreBtn->setFocusPolicy(Qt::NoFocus);
//    mainlayout->addWidget(closeBtn);
    mainlayout->addStretch();
    mainlayout->addWidget(iconlabel);
    mainlayout->addSpacing(200);
    mainlayout->addWidget(UpgradeBtn);
    mainlayout->addSpacing(10);
    mainlayout->addWidget(RestoreBtn);
    mainlayout->addStretch();
//    mainlayout->setAlignment(closeBtn,Qt::AlignRight|Qt::AlignTop);
    mainlayout->setAlignment(iconlabel,Qt::AlignHCenter);
    mainlayout->setAlignment(UpgradeBtn,Qt::AlignHCenter);
    mainlayout->setAlignment(RestoreBtn,Qt::AlignHCenter);
}

void initpage::dbusconnection()
{
    connect(UpgradeBtn,SIGNAL(clicked(bool)),this,SLOT(Upgrade(bool)));
    connect(RestoreBtn,SIGNAL(clicked(bool)),this,SLOT(Restore(bool)));
//    connect(closeBtn, &QPushButton::clicked, this, [=]{
//        emit closesignal();
//    });
}

void initpage::Upgrade(bool)
{
    qDebug()<<"initpage showattention";
    this->close();
    emit Upgradesignal();
}

void initpage::Restore(bool)
{
    qDebug()<<"Restore";
    emit Restoresignal();
}
