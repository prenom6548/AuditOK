#include "upgradepage1.h"

upgradepage1::upgradepage1(QWidget *parent) : QWidget(parent)
{
    initui();
    dbusconnection();
}

void upgradepage1::initui()
{
    mainlayout = new QVBoxLayout(this);

//    QWidget *topwidget = new QWidget(this);
//    QWidget *midwidget = new QWidget(this);
    QWidget *bottomwidget = new QWidget(this);
    QWidget *filewidget = new QWidget(this);
    QWidget *stepwidget = new QWidget(this);
    QWidget *suggestwidget = new QWidget(this);

//    QVBoxLayout *toplayout;
    QHBoxLayout *filelayout = new QHBoxLayout(this);
    QHBoxLayout *steplayout = new QHBoxLayout(this);
    QHBoxLayout *suggestlayout = new QHBoxLayout(this);
    QHBoxLayout *bottomlayout = new QHBoxLayout(this);

    bottomwidget->setLayout(bottomlayout);
    bottomwidget->setFixedWidth(432);

    filewidget->setLayout(filelayout);
    filewidget->setFixedWidth(360);
    stepwidget->setLayout(steplayout);
    steplayout->setContentsMargins(0,0,0,0);
    suggestwidget->setLayout(suggestlayout);
    suggestlayout->setContentsMargins(0,0,0,0);

    iconlabel = new QLabel(this);
    version = new QLabel(this);
    dbusconnect *dbus = dbusconnect::getInstance();
    QDBusReply<QString> versionStr = dbus->interface->call("getCurOsVersion");
    iconlabel->setFixedSize(93,93);
    iconlabel->setPixmap(QPixmap(":/kylin.png").scaled(QSize(93,93)));
    qDebug() << "str is " << versionStr.value();
    if (versionStr.value().contains("openkylin", Qt::CaseInsensitive))

    {
        iconlabel->setFixedSize(200,52);
        iconlabel->setPixmap(QPixmap(":/openKylin.png").scaled(QSize(200,52)));
    }
    version->setText(QString("当前版本：%1").arg(versionStr.value()));

    QLabel *filelabel = new QLabel(this);
    filelabel->setText("选择ISO文件");
    fileline = new QLineEdit(this);
    fileline->setReadOnly(true);
    filebtn = new KToolButton(this);
    filebtn->setType(KToolButtonType::SemiFlat);
    QIcon icon(":/png/document-open-symbolic.svg");
    filebtn->setIcon(icon);
    filelayout->addWidget(filelabel);
    filelayout->addWidget(fileline);
    filelayout->addWidget(filebtn);

    upgradebtn = new QPushButton(this);
    backbtn = new QPushButton(this);
    upgradebtn->setText("下一步");
    upgradebtn->setFixedHeight(40);
    upgradebtn->setEnabled(false);
    backbtn->setText("上一步");
    backbtn->setFixedHeight(40);
    steplayout->addWidget(backbtn);
    steplayout->addWidget(upgradebtn);

    tiplabel = new QLabel(this);
    tiplabel->setText("注意：升级前请备份系统并关闭其他应用");
    QFont tiplabelfont = QFont();
    tiplabelfont.setPixelSize(13);
    tiplabel->setFont(tiplabelfont);
    attentionbtn = new KBorderlessButton(this);
    attentionbtn->setContentsMargins(0,0,0,0);
    attentionbtn->setText("查看");
    suggestlayout->addWidget(tiplabel);
    suggestlayout->addWidget(attentionbtn);

    switchAppMoveCheckbox = new QCheckBox(this);
    switchAppMoveCheckbox->setText("新系统将兼容第三方应用");
    switchAppMoveCheckbox->setChecked(true);
    advancedoptionsbutton = new KBorderlessButton(this);
    advancedoptionsbutton->setText("高级设置");
    advancedoptionsbutton->setFlat(true);
    advancedoptionsbutton->setFocusPolicy(Qt::NoFocus);
    bottomlayout->addWidget(switchAppMoveCheckbox);
    bottomlayout->addStretch();
    bottomlayout->addWidget(advancedoptionsbutton);

    mainlayout->addSpacing(40);
    mainlayout->addWidget(iconlabel);
    mainlayout->addSpacing(5);
    mainlayout->addWidget(version);
    mainlayout->addSpacing(40);
    mainlayout->addWidget(filewidget);
    mainlayout->addSpacing(20);
    mainlayout->addWidget(stepwidget);
    mainlayout->addWidget(suggestwidget);
    mainlayout->addStretch();
    mainlayout->addWidget(bottomwidget);
    mainlayout->addSpacing(4);
    mainlayout->setAlignment(iconlabel,Qt::AlignHCenter);
    mainlayout->setAlignment(version,Qt::AlignHCenter);
    mainlayout->setAlignment(filewidget,Qt::AlignHCenter);
    mainlayout->setAlignment(stepwidget,Qt::AlignHCenter);
    mainlayout->setAlignment(suggestwidget,Qt::AlignHCenter);
    mainlayout->setAlignment(bottomwidget,Qt::AlignHCenter);





//    exitbuttonlayout = new QHBoxLayout(this);
//    iconlayout = new QHBoxLayout(this);
//    versionlayout = new QHBoxLayout(this);
//    tiplayout = new QHBoxLayout(this);
//    filelayout = new QHBoxLayout(this);
//    upgradelayout = new QHBoxLayout(this);
//    bottomlayout = new QHBoxLayout(this);
////    switchapplayout = new QHBoxLayout(this);
//    iconlabel = new QLabel(this);
//    version = new QLabel(this);
//    fileline = new QLineEdit(this);
//    filebtn = new QPushButton(this);
//    upgradebtn = new QPushButton(this);
//    backbtn = new QPushButton(this);
//    tiplabel = new QLabel(this);
//    attentionbtn = new QPushButton(this);
//    exitbtn = new QPushButton(this);
//    switchAppMoveCheckbox = new QCheckBox(this);
//    switchAppMoveCheckbox->setText("新系统将兼容第三方应用");
//    switchAppMoveCheckbox->setChecked(true);

//    advancedoptionsbutton = new QPushButton(this);
////    switchapplayout->addWidget(switchAppMoveCheckbox,0,Qt::AlignLeft);
//    midwidget = new QWidget(this);
//    midwidget->setFixedWidth(386);
//    midwidget->setFixedHeight(210);
//    QWidget *topwidget = new QWidget(this);
//    QVBoxLayout *toplayout =new QVBoxLayout(this);
//    topwidget->setLayout(toplayout);
//    midlayout = new QVBoxLayout(this);
//    midwidget->setLayout(midlayout);
//    bigmidlayout = new QHBoxLayout(this);

//    dbusconnect *dbus = dbusconnect::getInstance();
//    QDBusReply<QString> versionStr = dbus->interface->call("getCurOsVersion");
//    version->setText(QString("当前版本：%1").arg(versionStr.value()));
//    iconlabel->setFixedSize(93,93);
//    iconlabel->setPixmap(QPixmap(":/kylin.png").scaled(QSize(93,93)));
//    qDebug() << "str is " << versionStr.value();
//    if (versionStr.value().contains("openkylin", Qt::CaseInsensitive))
//    {
//        iconlabel->setFixedSize(200,52);
//        iconlabel->setPixmap(QPixmap(":/openKylin.png").scaled(QSize(200,52)));
//    }

//    fileline->setReadOnly(true);
//    fileline->setFixedHeight(40);
//    filebtn->setText("选择ISO文件");
//    filebtn->setFixedHeight(40);
//    filebtn->setFixedWidth(120);

//    upgradebtn->setText("下一步");
//    upgradebtn->setFixedHeight(40);
//    upgradebtn->setEnabled(false);
//    backbtn->setText("上一步");
//    backbtn->setFixedHeight(40);

//    tiplabel->setText("注意：升级前请备份系统并关闭其他应用");
//    QFont tiplabelfont = QFont();
//    tiplabelfont.setPixelSize(13);
//    tiplabel->setFont(tiplabelfont);

//    attentionbtn->setText("查看");
//    QPalette palette_attention = attentionbtn->palette();
//    palette_attention.setColor(QPalette::ButtonText,QColor(55,144,250));
//    attentionbtn->setPalette(palette_attention);

//    attentionbtn->setFlat(true);
//    attentionbtn->setFont(tiplabelfont);
//    attentionbtn->setContentsMargins(0,0,0,0);
//    exitbtn->setText("x");

//    exitbtn->setFlat(true);
//    exitbtn->setFixedSize(13,13);
//    advancedoptionsbutton->setText("高级设置");
//    advancedoptionsbutton->setFlat(true);
//    advancedoptionsbutton->setFocusPolicy(Qt::NoFocus);
////    advancedoptionsbutton->setStyleSheet("QPushButton{background: #FFFFFF}");
//    exitbuttonlayout->addWidget(exitbtn,0,Qt::AlignRight);
//    iconlayout->addStretch();
//    iconlayout->addWidget(iconlabel);
//    iconlayout->addStretch();
//    versionlayout->addStretch();
//    versionlayout->addWidget(version);
//    versionlayout->addStretch();
//    filelayout->addWidget(fileline);
//    filelayout->addSpacing(10);
//    filelayout->addWidget(filebtn);
//    upgradelayout->addWidget(backbtn);
//    upgradelayout->addWidget(upgradebtn);

//    tiplayout->addWidget(tiplabel);
//    tiplayout->addWidget(attentionbtn);
//    bottomlayout->addWidget(advancedoptionsbutton);

//    mainlayout->addLayout(exitbuttonlayout);

//    mainlayout->addLayout(iconlayout);
//    mainlayout->addLayout(versionlayout);

//    midlayout->addSpacing(5);
//    midlayout->addLayout(filelayout);
//    midlayout->addSpacing(10);
//    midlayout->addLayout(upgradelayout);
//    midlayout->addSpacing(5);
//    midlayout->addLayout(tiplayout);
//    midlayout->addSpacing(5);
////    midlayout->addLayout(switchapplayout);

//    bottomlayout->addWidget(switchAppMoveCheckbox);
//    bigmidlayout->addWidget(midwidget,Qt::AlignCenter);
//    mainlayout->addLayout(bigmidlayout);
//    mainlayout->addSpacing(170);
//    mainlayout->addLayout(bottomlayout);
//    mainlayout->addSpacing(20);

////    fileline->setStyleSheet("QLineEdit{background: #E6E6E6}");
////    iconlabel->setStyleSheet("QLabel{background: #FFFFFF}");
////    version->setStyleSheet("QLabel{background: #FFFFFF}");
////    midwidget->setStyleSheet("QWidget{background: #FFFFFF}");
////    filebtn->setStyleSheet("QPushButton{background: #E6E6E6}");
//    filebtn->setFocusPolicy(Qt::NoFocus);
////    upgradebtn->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
//    upgradebtn->setFocusPolicy(Qt::NoFocus);
////    backbtn->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
//    backbtn->setFocusPolicy(Qt::NoFocus);
////    attentionbtn->setStyleSheet("QPushButton{background: #FFFFFF}");
//    attentionbtn->setFocusPolicy(Qt::NoFocus);

//    exitbtn->setFocusPolicy(Qt::NoFocus);
    this->setLayout(mainlayout);
}

void upgradepage1::dbusconnection()
{
    connect(fileline,SIGNAL(textEdited(QString)),this,SLOT(isocontentedited(QString)));
    connect(filebtn,SIGNAL(clicked(bool)),this,SLOT(showfile(bool)));
    connect(upgradebtn,SIGNAL(clicked(bool)),this,SLOT(emitsignal(bool)));
    connect(backbtn,SIGNAL(clicked(bool)),this,SLOT(emitbacksignal(bool)));
    connect(attentionbtn,SIGNAL(clicked(bool)),this,SLOT(showattention(bool)));
//    connect(exitbtn,SIGNAL(clicked(bool)),this,SLOT(exitapp(bool)));
    connect(switchAppMoveCheckbox,SIGNAL(stateChanged(int)),this,SLOT(switchappmovecheck(int)));
    connect(advancedoptionsbutton,SIGNAL(clicked(bool)),this,SLOT(showadvancedoptions(bool)));
    dbus = dbusconnect::getInstance();
    if(switchAppMoveCheckbox->isChecked())
    {
        dbus->interface->call("switchAppMove",1);
    }
    else
    {
        dbus->interface->call("switchAppMove",0);
    }
    qDebug()<<"第一页dbus链接";
}

void upgradepage1::switchappmovecheck(int status)
{
    if(status)
    {
        dbus->interface->call("switchAppMove",1);
    }
    else
    {
        dbus->interface->call("switchAppMove",0);
    }
}

void upgradepage1::showfile(bool)
{
    s = QFileDialog::getOpenFileName(this,"选择镜像","/","ISO files(*.iso)");
    if(s.isEmpty()==false)
    {
        fileline->setText(s);
        upgradebtn->setEnabled(true);
//        upgradebtn->setStyleSheet("QPushButton{background: #3790FA;color:#FFFFFF;font-size:18px}");
    }
//    else
//    {
//        upgradebtn->setEnabled(false);
//        upgradebtn->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
//    }
}

void upgradepage1::emitsignal(bool)
{
    qDebug()<<"upgrade";

//    dbus = dbusconnect::getInstance();
//    qDebug()<<"upgrade"<<s;

//    dbus->interface->call("preUpgradeCheck",s);
        emit nextpage();

}

void upgradepage1::emitbacksignal(bool)
{
    qDebug()<<"emitbacksignal";

//    dbus = dbusconnect::getInstance();
//    qDebug()<<"upgrade"<<s;

//    dbus->interface->call("preUpgradeCheck",s);
    emit backpage();

}

void upgradepage1::showattention(bool)
{
    qDebug()<<"upgradepage1 showattention";

    this->close();
    emit showattentionsignal();
}

void upgradepage1::exitapp(bool)
{
    qDebug()<<"exitapp";
    emit exitappsignal();
}

void upgradepage1::showadvancedoptions(bool)
{
    emit showadvancedoptionspage();
}

void upgradepage1::isocontentedited(const QString &text)
{
    if(text.isEmpty())
    {
        upgradebtn->setEnabled(false);
//        upgradebtn->setStyleSheet("QPushButton{background: #E6E6E6;color:#262626;font-size:18px}");
    }
}
