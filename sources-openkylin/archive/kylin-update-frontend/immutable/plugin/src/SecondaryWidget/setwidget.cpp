#include "setwidget.h"
#include "src/xatom-helper.h"
SetWidget::SetWidget(QWidget* parent):QDialog(parent)
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowFlag(Qt::Tool);
    this->setFixedWidth(550);
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
//    this->setFixedSize(450,560);
    showAreaWidget = new QWidget(this);
    showAreaWidget->setObjectName("showArea");
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("scroll");
    this->setFixedHeight(560);
//    this->setFixedSize(450,560);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->horizontalScrollBar()->setEnabled(false);
    scrollArea->setFixedHeight(this->height());
    scrollArea->setWidget(showAreaWidget);
    scrollArea->setWidgetResizable(true);
//    scrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    scrollArea->setStyleSheet("QScrollArea#scroll{background-color: transparent;}");
    scrollArea->viewport()->setStyleSheet("QWidget#showArea{background-color: transparent;}");
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);
    QVBoxLayout *mainlyt = new QVBoxLayout();
    QHBoxLayout *titleLyt = initTitleBar();
    QVBoxLayout *bodylyt = initBody();
    QHBoxLayout *btnlyt = initButton();
    mainlyt->setMargin(0);
    mainlyt->addLayout(titleLyt);
    mainlyt->addWidget(scrollArea);
    mainlyt->addLayout(btnlyt);
    mainlyt->addSpacing(25);
    mainlyt->addStretch();
    mainlyt->setSpacing(0);
    mainlyt->setContentsMargins(0,0,0,0);
    this->setLayout(mainlyt);
    showAreaWidget->setLayout(bodylyt);
//    showAreaWidget->setFixedWidth(this->width());
    scrollArea->setFixedHeight(this->height() - cancelBtn->height() - 30 -10- 25);
    getInitialData();
}


QHBoxLayout* SetWidget::initTitleBar()
{
    titleText = new QLabel(showAreaWidget);
    titleIcon = new QPushButton(showAreaWidget);
    titleBtnClose = new QPushButton(showAreaWidget);
    titleBtnClose->setToolTip(tr("Close"));
    titleIcon->setObjectName("titleIcon");
    titleIcon->setFixedSize(24,24);
    titleIcon->setIconSize(QSize(25,25));
    //TODO：直接从主题调图标，不会QIcon转qpixmap所以暂时从本地拿
//    titleIcon->setPixmap(QPixmap::fromImage(QImage(iconPath)));
    titleIcon->setIcon(QIcon::fromTheme("ukui-control-center"));
    titleIcon->setFlat(true);
//    titleIcon->setProperty("useButtonPalette", true);
    titleIcon->setFocusPolicy(Qt::NoFocus);
    titleIcon->setStyleSheet("QPushButton#titleIcon{border:0px;border-radius:4px;background:transparent;}"
                             "QPushButton#titleIcon::hover{border:0px;border-radius:4px;background:transparent;}"
                             "QPushButton#titleIcon::pressed{border:0px;border-radius:4px;background:transparent;}");
    titleBtnClose->setFixedSize(30,30);
    titleBtnClose->setToolTip(tr("Close"));
    titleBtnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    titleBtnClose->setProperty("isWindowButton",0x2);
    titleBtnClose->setProperty("useIconHighlightEffect",0x8);
    titleBtnClose->setFlat(true);
    connect(titleBtnClose,&QPushButton::clicked,[=](){this->close();});
    QHBoxLayout *hlyt = new QHBoxLayout;
    titleText->setText(tr("Advanced Option"));
    hlyt->setSpacing(0);
    hlyt->setMargin(4);
    hlyt->addSpacing(4);
    hlyt->addWidget(titleIcon); //居下显示
    hlyt->addSpacing(8);
    hlyt->addWidget(titleText);//,0,Qt::AlignBottom
    hlyt->addStretch();
    hlyt->addWidget(titleBtnClose);
    return hlyt;
}

QVBoxLayout* SetWidget::initBody()
{
    QVBoxLayout *vLayout4 = initServerAddress();
    QVBoxLayout *vlyt = new QVBoxLayout;
    vlyt->setMargin(0);
    vlyt->setSpacing(0);
    vlyt->addLayout(vLayout4);
    vlyt->addStretch();
    vlyt->setContentsMargins(24,10,24,10);
    return vlyt;
}

QVBoxLayout* SetWidget::initServerAddress()
{
    serverAddressTitleLabel = new QLabel(showAreaWidget);
    serverAddressTitleLabel->setAlignment(Qt::AlignLeft);
    serverAddressTitleLabel->setText(tr("Server address settings"));
    tipsTextLabel = new QLabel(showAreaWidget);
    tipsTextLabel->setAlignment(Qt::AlignLeft);
    tipsTextLabel->setText(tr("If internal services, change the server address."));
//    ProtocolLabel = new QLabel(showAreaWidget);
//    ProtocolLabel->setAlignment(Qt::AlignLeft);
//    ProtocolLabel->setFixedWidth(80);
//    ProtocolLabel->setText(tr("Protocal"));
    ProtocolComboBox = new QComboBox(showAreaWidget);
    ProtocolComboBox->setFixedWidth(112);
    ProtocolComboBox->setEditable(false);
    ProtocolComboBox->setObjectName("comboBox1");
    ProtocolComboBox->addItem("https://");
    ProtocolComboBox->addItem("http://");
    PortLabel = new QLabel(showAreaWidget);
    PortLabel->setAlignment(Qt::AlignLeft);
    PortLabel->setFixedWidth(80);
    PortLabel->setText(tr("Port  ID "));
    PortEdit = new QLineEdit(showAreaWidget);
    PortEdit->setFixedWidth(112);
    PortEdit->setObjectName("PortEdit");
    QRegExp regx("[0-9]+$");
    QValidator* validator = new QRegExpValidator(regx, PortEdit);
    PortEdit->setValidator(validator);
    AddressLabel = new QLabel(showAreaWidget);
    AddressLabel->setAlignment(Qt::AlignLeft);
    AddressLabel->setText(tr("Address"));
    AddressLabel->setFixedWidth(80);
    IPEdit = new QLineEdit(showAreaWidget);
    IPEdit->setObjectName("IPEdit");
    IPEdit->setFixedWidth(250);

    QHBoxLayout *hlayout1 = new QHBoxLayout();
    hlayout1->addWidget(serverAddressTitleLabel,0,Qt::AlignLeft);
    hlayout1->addStretch();
    hlayout1->setSpacing(0);
    hlayout1->setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout2 = new QHBoxLayout();
    hlayout2->addWidget(tipsTextLabel,0,Qt::AlignLeft);
    hlayout2->addStretch();
    hlayout2->setSpacing(0);
    hlayout2->setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout3 = new QHBoxLayout();
    hlayout3->addWidget(AddressLabel,0,Qt::AlignLeft);
    hlayout3->addSpacing(4);
//    hlayout3->addWidget(ProtocolLabel,0,Qt::AlignLeft);
//    hlayout3->addSpacing(4);
    hlayout3->addWidget(ProtocolComboBox,0,Qt::AlignLeft);
    hlayout3->addSpacing(4);
    hlayout3->addWidget(IPEdit);
    hlayout3->addStretch();
    hlayout3->setSpacing(0);
    hlayout3->setContentsMargins(0,0,0,0);

    QHBoxLayout *hlayout4 = new QHBoxLayout();
    hlayout4->addWidget(PortLabel);
    hlayout4->addSpacing(4);
    hlayout4->addWidget(PortEdit);
    hlayout4->setSpacing(0);
    hlayout4->addStretch();
    hlayout4->setContentsMargins(0,0,0,0);

//    QHBoxLayout *hlayout5 = new QHBoxLayout();
//    hlayout5->addWidget(AddressLabel);
//    hlayout5->addSpacing(4);
//    hlayout5->addWidget(IPEdit);
//    hlayout5->addStretch();
//    hlayout5->setSpacing(0);
//    hlayout5->setContentsMargins(0,0,0,0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout1);
    vlayout->addSpacing(12);
    vlayout->addLayout(hlayout2);
    vlayout->addSpacing(8);
    vlayout->addLayout(hlayout3);
    vlayout->addSpacing(8);
    vlayout->addLayout(hlayout4);
//    vlayout->addSpacing(8);
//    vlayout->addLayout(hlayout5);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);
    return vlayout;
}


QHBoxLayout *SetWidget::initButton()
{
    resetBtn = new QPushButton(this);
    resetBtn->setText(tr("reset"));
//    resetBtn->setFlat(true);
//    resetBtn->setText(tr("Restore default settings"));
    resetBtn->setObjectName("resetBtn");
    connect(resetBtn,SIGNAL(clicked()),this,SLOT(slotRestoreDefaultSettings()));
    cancelBtn = new QPushButton(this);
    cancelBtn->setText(tr("Cancel"));
    connect(cancelBtn,&QPushButton::clicked,[=](){this->close();});
    determineBtn = new QPushButton(this);
    determineBtn->setText(tr("OK"));
    connect(determineBtn,SIGNAL(clicked(bool)),this,SLOT(slotModifysettings()));
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(resetBtn,0,Qt::AlignVCenter);
    hlayout->addStretch();
    hlayout->addWidget(cancelBtn,0,Qt::AlignVCenter);
    hlayout->addSpacing(10);
    hlayout->addWidget(determineBtn,0,Qt::AlignVCenter);
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(24,0,24,0);

    return hlayout;

}


/*****************获取数据********************/
void SetWidget::getInitialData()
{
    QString filename = "/var/lib/kylin-software-properties/template/kylin-source-update.conf";
    QSettings *setting = new QSettings(filename, QSettings::IniFormat);
    setting->setIniCodec("utf8");
    QStringList group = setting->childGroups();
    int num = group.count();
    int Priority;
    int tmp = 0;
    int i;
    for(i=0; i<num; )
    {
        QString Service = group.at(i);
        qInfo()<<"The Service is "<< Service;
        setting->beginGroup(Service);
        Priority = setting->value("Priority").toInt();
        qInfo()<<"the Priority is "<<Priority;
        setting->endGroup();
        if(Priority > tmp)
        {
            tmp = Priority;
            flag = Service;
        }
        i++;
    }
    if(Priority == 0)
    {
        qInfo()<<"Priority is 0,find priority";
        qInfo()<<"==================";
        for(i=0; i<num; )
        {
            QString Service = group.at(i);
            qDebug()<<"The Service is "<< Service;
            setting->beginGroup(Service);
            Priority = setting->value("priority").toInt();
            qDebug()<<"the priority is "<<Priority;
            setting->endGroup();
            if(Priority > tmp)
            {
                tmp = Priority;
                flag = Service;
            }
            i++;
        }
        qInfo()<<"==================";
    }
    qDebug()<<"the tmp is" <<tmp <<flag;
    /*Port ID initial*/
    setting->beginGroup(flag);
    QString Port = setting->value("Port").toString();
    qDebug()<<"the Port is "<< Port;
    PortEdit->setText(Port);
    /*IP Address initial*/
    QString fullname = setting->value("Address").toString();
    qDebug()<<"the fullname is "<<fullname;
    QStringList namelist = fullname.split("://");
    qDebug()<<"the namelist is "<< namelist;
    QString networkprotocal = namelist.at(0);
    qDebug()<<"the networkprotocal is "<<networkprotocal;
    if(networkprotocal != "http" && networkprotocal != "https")
    {
        qDebug()<<"the network protocal is error";
        ProtocolComboBox->setCurrentText("https://");
    }
    else
    {
        ProtocolComboBox->setCurrentText(networkprotocal+"://");
    }
    QString domainname = namelist.at(1);
    qDebug()<<"the domainname is "<< domainname;
    IPEdit->setText(domainname);
}



/**********************************槽函数***********************************/
void SetWidget::slotRestoreDefaultSettings()
{
    qDebug()<<"reset the default setting";
//    QDBusInterface modify_interface( "com.kylin.software.properties",
//                                 "/com/kylin/software/properties",
//                                 "com.kylin.software.properties.interface",
//                                 QDBusConnection::systemBus() );
    ProtocolComboBox->setCurrentText("http://");
    PortEdit->setText("59546");
    IPEdit->setText("archive1.kylinos.cn");
//    QDBusMessage modelist = modify_interface.call("setServerAddressConf","Server","http://archive1.kylinos.cn","59546");
//    QDBusMessage modelist1 = modify_interface.call("setServerAddressConf","Server1","https://archive1.kylinos.cn","59546");
}

void SetWidget::slotModifysettings()
{
    QString group = flag;
    QString protocol = ProtocolComboBox->currentText();
    QString domainname = IPEdit->text();
    QString IP = protocol+domainname;
    QString port = PortEdit->text();
    qDebug()<<group<<IP<<port;

    QDBusInterface modify_interface( "com.kylin.software.properties",
                                 "/com/kylin/software/properties",
                                 "com.kylin.software.properties.interface",
                                 QDBusConnection::systemBus() );
    QDBusMessage modelist = modify_interface.call("setServerAddressConf",group,IP,port);
    int result =modelist.arguments().value(0).toInt();
    qDebug()<<result;
    if(result)
    {
        qDebug()<<"error in modify!";
        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("Modification failed!"));
        msgBox.setWindowTitle(tr("Attention"));
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.addButton(tr("OK"), QMessageBox::YesRole);
    }
    else
    {
        this->close();
    }
}
