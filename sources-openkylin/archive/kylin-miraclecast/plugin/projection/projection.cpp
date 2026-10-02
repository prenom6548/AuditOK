/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "projection.h"
#include "ui_projection.h"
#include <QProcess>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>
#include <QMouseEvent>

#define ITEMFIXEDHEIGH 58
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"

#define SYSTEM_CMD_ERROR    -1
enum {
    NOT_SUPPORT_P2P = 0,
    SUPPORT_P2P_WITHOUT_DEV,
    SUPPORT_P2P_PERFECT,
    OP_NO_RESPONSE,
    NO_SERVICE
};

enum {
    PROJECTION_RUNNING = 256,
    DAEMON_NOT_RUNNING = 512
};

Projection::Projection()
{
    qDebug()<<"123456456";
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/projection/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);
    pluginName = tr("Projection");
    pluginType = DEVICES;
    ui = new Ui::Projection;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);
    QPalette pa;
    pa.setColor(QPalette::WindowText, Qt::red);
    ui->tipLabel->setPalette(pa);
    ui->tipLabel2->setPalette(pa);
    ui->tipLabel2->setFixedWidth(ui->widget_7->width()-ui->label_5->width()-48);
    projectionBtn = new KSwitchButton(pluginWidget);
    pincodeBtn = new KSwitchButton(pluginWidget);
    miracastBtn = new KSwitchButton(pluginWidget);
    dropDownLabel = new DrownLabel(pluginWidget);
    ui->horizontalLayout_12->addWidget(dropDownLabel);

    //   ui->pinframe->hide();
    init_button_status(get_process_status());
    //    qDebug()<<"---- Projection contructed, then subscribe slot func ";
    connect(projectionBtn, SIGNAL(stateChanged(bool)), this, SLOT(projectionButtonClickSlots(bool)));
    connect(pincodeBtn, SIGNAL(stateChanged(bool)), this, SLOT(pincodeButtonClickSlots(bool)));
    // connect(miracastBtn, SIGNAL(stateChanged(bool)), this, SLOT(miracastsourceButtonClickSlots(bool)));
    connect(ui->pushButton, &QPushButton::clicked, this, [=](){
        pushButtonClickSlots();
    });
    connect(dropDownLabel, &DrownLabel::labelClicked, [=](){
        debusFileConfig();
        onDrownLabelClicked();
    });
    // m_pin = new QLabel(pluginWidget);
    // ui->label->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
    ui->label->setStyleSheet("QLabel{color: palette(windowText);}");
    //~ contents_path /projection/Open Projection
    ui->titleLabel->setStyleSheet("QLabel{color: palette(windowText);}");
    pa.setColor(QPalette::WindowText, Qt::gray);
    ui->label_2->setPalette(pa);
    ui->label_4->setPalette(pa);
    ui->pushButton->setEnabled(false);
    ui->pushButton->setStyleSheet("color: gray");
    ui->pushButton->setText(tr("Find device"));
    ui->titleLabel_3->setStyleSheet("QLabel{color:palette(windowText);}");

    m_pServiceInterface = new QDBusInterface("org.freedesktop.miracleagent",
                                             "/org/freedesktop/miracleagent",
                                             "org.freedesktop.miracleagent.op",
                                             QDBusConnection::sessionBus());
    QString path=QDir::homePath()+"/.config/miracast.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    bool bo=setting->contains("host");
    qDebug()<<bo<<"bo";
    if (!bo) {
        QDBusInterface *hostInterface = new QDBusInterface("org.freedesktop.hostname1",
                                                           "/org/freedesktop/hostname1",
                                                           "org.freedesktop.hostname1",
                                                           QDBusConnection::systemBus());
        hostName = hostInterface->property("Hostname").value<QString>();
        setting->setValue("host",hostName);
        setting->sync();
        setting->endGroup();
        initComponent();
    }else {
        hostName = setting->value("host").toString();
    }
    //ui->projectionNameWidget->setFixedHeight(40);
    // setLabelText(ui->projectionName,hostName);
    QFontMetrics  fontMetrics(ui->projectionName->font());
    int fontSize = fontMetrics.width(hostName);
    if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) <= fontSize){
        ui->projectionName->setFixedWidth(ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46);
    }
    if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) > fontSize){
        ui->projectionName->setFixedWidth(fontSize);
    }
    ui->projectionName->setText(hostName);
    getSystemFontFamily();//建立GSetting监听系统字体
    ui->projectionNameChange->setProperty("useIconHighlightEffect", 0x8);
    ui->projectionNameChange->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ui->projectionNameChange->size()));
    ui->projectionNameWidget->installEventFilter(this);
    ui->horizontalLayout->addWidget(projectionBtn);
    ui->horizontalLayout_17->addWidget(pincodeBtn);
    ui->horizontalLayout_6->addWidget(miracastBtn);
    initComponent();
    debusFileConfig();
    ui->widget_4->installEventFilter(this);
    ui->widget_5->installEventFilter(this);
    ui->widget_6->installEventFilter(this);
    ui->widget_7->installEventFilter(this);
    ui->widget_9->installEventFilter(this);
    QLocale locale;
    if (locale.language() == QLocale::English){
        connect(this,&Projection::resize,[=](){
            setLabelText(ui->label_2,"Wireless LAN connection may be interrupted during projection.");
            setLabelText(ui->titleLabel_3,"You need to match the PIN code when projecting the screen to the pc");
            setLabelText(ui->label_3,"Open(PIN authentication is not supported temporarily.)");
            setLabelText(ui->label_4,"When enabled,the project to this PC feature stops.");
        });
    }
}
void Projection::debusFileConfig()
{
    QDBusInterface*mInterface = new QDBusInterface("org.freedesktop.miracle.wifi",
                                                   "/org/freedesktop/miracle/wifi/ui",
                                                   "org.freedesktop.miracle.wifi.ui",
                                                   QDBusConnection::systemBus());
    QDBusMessage result  = mInterface->call("QueryConnectedDevice");

    qDebug()<<"----------"<<result;
    deleteAllitemsOfLayout( ui->verticalLayout_2);

    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QMap<QString,QString>deviceName;
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    dbusArgs.beginMap();
    while (!dbusArgs.atEnd()) {
        QString key;
        QString value;
        dbusArgs.beginMapEntry();
        dbusArgs >> key >> value ;
        qDebug() << key<< "数值" <<value;
        deviceName.insert(key,value);
        dbusArgs.endMapEntry();
    }
    dbusArgs.endMap();

    QMap<QString, QString>::iterator iter = deviceName.begin();
    while (iter != deviceName.end())
    {
        QString value = iter.value();
        QHBoxLayout *addLyt = new QHBoxLayout;
        addLyt->setContentsMargins(0,17,0,17);
        QLabel *iconLabel = new QLabel(ui->widget_2);
        QLabel *textLabel = new QLabel(ui->widget_2);
        QPushButton *iconBtn = new QPushButton(ui->widget_2);
        textLabel->setText(iter.key());
        iconLabel->setPixmap(QIcon::fromTheme("phone-apple-iphone-symbolic").pixmap(QSize(16,16)));
        iconLabel->setProperty("useIconHighlightEffect", 0x8);   
        iconBtn->setMaximumSize(QSize(36,36));
        iconBtn->setIconSize(QSize(16, 16));
        iconBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
        iconBtn->setProperty("useIconHighlightEffect", 0x8);
        iconBtn->setProperty("isWindowButton", 0x1);
        iconBtn->setFlat(true);
        QPalette paletteLan = iconBtn->palette();
        paletteLan.setColor(QPalette::Highlight, Qt::transparent);
        paletteLan.setBrush(QPalette::Button, QBrush(QColor(1,1,1,0)));
        iconBtn->setPalette(paletteLan);
        addLyt->addItem(new QSpacerItem(16,10,QSizePolicy::Fixed));
        addLyt->addWidget(iconLabel);
        addLyt->addItem(new QSpacerItem(13,10,QSizePolicy::Fixed));
        addLyt->addWidget(textLabel);
        addLyt->addItem(new QSpacerItem(712,10,QSizePolicy::Expanding));
        addLyt->addWidget(iconBtn);
        addLyt->addItem(new QSpacerItem(16,10,QSizePolicy::Fixed));
        QFrame *line = new QFrame(ui->widget_2);
        line->setLineWidth(0);
        line->setFixedHeight(1);
        line->setFrameShape(QFrame::Shape::HLine);
        line->setStyleSheet("color: rgb(244, 244, 244);opacity:1;");
        ui->verticalLayout_2->addWidget(line);
        ui->verticalLayout_2->addLayout(addLyt);
        if (iter == deviceName.begin()){
            delete line;
        }
        connect(iconBtn, &QPushButton::clicked, this, [=] {

            QDBusInterface*mInterface = new QDBusInterface("org.freedesktop.miracle.wifi",
                                                           "/org/freedesktop/miracle/wifi/ui",
                                                           "org.freedesktop.miracle.wifi.ui",
                                                           QDBusConnection::systemBus());
            QDBusMessage result  = mInterface->call("DeleteConnectedDevice",value);
            QList<QVariant> outArgs = result.arguments();
            bool res = outArgs.at(0).value<int>();
            if (!res){
                debusFileConfig();
            }
        });
        iter++;
    }
    ui->widget_2->update();
}

void Projection::deleteAllitemsOfLayout(QLayout *layout)
{
    QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr)
        {
            ///setParent为NULL，防止删除之后界面不消失
            if(child->widget())
            {
                child->widget()->setParent(nullptr);
            }else if(child->layout()){
                deleteAllitemsOfLayout(child->layout());
            }
            delete child;
        }
}

void Projection::changeProjectionName(QString name){
    qDebug() << name;
    QString path=QDir::homePath()+"/.config/miracast.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    setting->setValue("host",name);
    setting->sync();
    setting->endGroup();
    m_pServiceInterface->call("UiSetName",name);
    QFontMetrics  fontMetrics(ui->projectionName->font());
    int fontSize = fontMetrics.width(name);
    if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) <= fontSize){
        ui->projectionName->setFixedWidth(ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46);
    }
    if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) > fontSize){
        ui->projectionName->setFixedWidth(fontSize);
    }
    // setLabelText(ui->projectionName,name);
    ui->projectionName->setText(name);
}

void Projection::getSystemFontFamily()
{
    const QByteArray id("org.ukui.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    connect(fontSetting, &QGSettings::changed,[=](QString key) {
        if ("systemFont" == key || "systemFontSize" ==key) {
            QString path=QDir::homePath()+"/.config/miracast.ini";
            QSettings *setting=new QSettings(path,QSettings::IniFormat);
            setting->beginGroup("projection");
            QString name = setting->value("host").toString();
            setting->sync();
            setting->endGroup();
            QFontMetrics  fontMetrics(ui->projectionName->font());
            int fontSize = fontMetrics.width(name);
            if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) <= fontSize){
                ui->projectionName->setFixedWidth(ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46);
            }
            if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) > fontSize){
                ui->projectionName->setFixedWidth(fontSize);
            }
            // setLabelText(ui->projectionName,name);
            ui->projectionName->setText(name);
        }
    });
}

void Projection::showChangeProjectionNameDialog(){

    ChangeProjectionName * dialog = new ChangeProjectionName();

    connect(dialog, &ChangeProjectionName::sendNewProjectionName, [=](QString name){
        changeProjectionName(name);
    });
    dialog->exec();
}

bool Projection::getWifiStatus() {
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );
    // 获取当前wifi是否打开
    QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");
    if (m_result.isValid()) {
        bool status = m_result.value().toBool();
        return status;
    } else {
        qDebug()<<"org.freedesktop.NetworkManager get invalid"<<endl;
        return false;
    }
}

void Projection::setWifiStatus(bool status) {

    QString wifiStatus = status ? "on" : "off";
    QString program = "nmcli";
    QStringList arg;
    arg << "radio" << "wifi" << wifiStatus;
    QProcess *nmcliCmd = new QProcess(this);
    nmcliCmd->start(program, arg);
    nmcliCmd->waitForStarted();
}

void Projection::setLabelText(QLabel *label, QString text)
{
    QFontMetrics  fontMetrics(label->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > label->width()) {
        label->setText(fontMetrics.elidedText(text, Qt::ElideRight, label->width()));
        label->setToolTip(text);
    } else {
        label->setText(text);
        label->setToolTip("");
    }
}

bool Projection::eventFilter(QObject *watched, QEvent *event){
    QLocale locale;
    QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
    if (watched == ui->projectionNameWidget){
        if (event->type() == QEvent::MouseButtonPress){
            if (mouseEvent->button() == Qt::LeftButton ){
                showChangeProjectionNameDialog();
            }
        }
    }
    else if (watched == ui->widget_9 ) {
        if (event->type() == QEvent::Resize) {
            QFontMetrics  fontMetrics(ui->projectionName->font());
            int fontSize = fontMetrics.width(ui->projectionName->text());
            if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) <= fontSize){
                ui->projectionName->setFixedWidth(ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46);
                emit resize();
            }
            if((ui->widget_9->width()-ui->titleLabel->width()-projectionBtn->width()-46) > fontSize){
                ui->projectionName->setFixedWidth(fontSize);
                emit resize();
            }
            ui->label_2->setFixedWidth(ui->widget_9->width()-106);
            emit resize();
        }
    }
    else if (watched == ui->widget_4 ) {
        if (event->type() == QEvent::Resize) {
            ui->titleLabel_3->setFixedWidth(ui->widget_4->width()-106);
            emit resize();
        }
    } 
    else if (watched == ui->widget_5 ) {
        if (event->type() == QEvent::Resize) {
            ui->label_3->setFixedWidth(ui->widget_5->width()-246);
            ui->label_4->setFixedWidth(ui->widget_5->width()-246);
            emit resize();
        }
    } 
    else if (watched == ui->widget_6 ) {
        if (event->type() == QEvent::Resize) {
            ui->tipLabel->setFixedWidth(ui->widget_5->width()-256);
            emit resize();
        }
    } 
    else if (watched == ui->widget_7 ) {
        if (event->type() == QEvent::Resize && locale.language() == QLocale::English) {
            ui->tipLabel2->setFixedWidth(ui->widget_5->width()-326);
            emit resize();
        }
        else if (event->type() == QEvent::Resize && locale.language() == QLocale::Chinese) {
            ui->tipLabel2->setFixedWidth(ui->widget_5->width()-ui->label_5->width()-48);
        }
    } 
    return QObject::eventFilter(watched, event);
}

void Projection::onDrownLabelClicked()
{
    if (dropDownLabel->isChecked) {
        ui->widget_2->hide();
        dropDownLabel->setDropDownStatus(false);
    } else {
        ui->widget_2->show();
        dropDownLabel->setDropDownStatus(true);
    }
}

void Projection::catchsignal()
{
    while (1)
    {
        m_pServiceInterface = new QDBusInterface("org.freedesktop.miracle.wifi",
                                                 "/org/freedesktop/miracle/wifi/ui",
                                                 "org.freedesktop.miracle.wifi.ui",
                                                 QDBusConnection::systemBus());
        if (m_pServiceInterface->isValid()) {
            connect(m_pServiceInterface,SIGNAL(PinCode(QString, QString)),this,SLOT(projectionPinSlots(QString,QString)));
            return;
        }else {
            delete m_pServiceInterface;
            delaymsec(1000);
        }
    }
    \
}
void Projection::delaymsec(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

Projection::~Projection()
{
    delete ui;
    delete m_pServiceInterface;
}

QString Projection::plugini18nName(){
    QFile server("/usr/bin/miracle-wifid");
    QFile agent("/usr/bin/miracle-agent");

    int projectionstatus;
    QDBusMessage result = m_pServiceInterface->call("PreCheck");
    QList<QVariant> outArgs = result.arguments();
    projectionstatus = outArgs.at(0).value<int>();

    if(!server.exists() || !agent.exists() || NO_SERVICE == projectionstatus)
        return NULL;

    return pluginName;
}

int Projection::pluginTypes(){
    return pluginType;
}

int Projection::get_process_status(void)
{
    int res;

    do{
        res = system("checkDaemonRunning.sh");
    }while(SYSTEM_CMD_ERROR == res);

    return res;
}
int Projection::get_process_source_status(void)
{
    QString path=QDir::homePath()+"/.config/miracastsource.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    QString method;

    bool bo=setting->contains("miracastsource");
    if (!bo) {
        method = "false";
        return 0;
    }else {
        method = setting->value("miracastsource").toString();
        if (method == "true")
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void Projection::init_button_status(int status)
{
    //    qDebug()<<"---- button init status "<<status;
    if (status == PROJECTION_RUNNING) {
        projectionBtn->setChecked(true);
    }
    else {
        projectionBtn->setChecked(false);//projection not switch-on, or daemon programs not running at all
    }
}

void Projection::init_source_button_status(int status)
{
    //    qDebug()<<"---- button init status "<<status;
    if (status == 1) {
        // miracastBtn->setChecked(true);
    }
    else {
        // miracastBtn->setChecked(false);//projection not switch-on, or daemon programs not running at all
    }
}


int Projection::get_pin_button_status(void)
{
    QString path=QDir::homePath()+"/.config/miracast.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    QString method;

    bool bo=setting->contains("config_method");
    if (!bo) {
        method = "display";
        m_pServiceInterface->call("UiSetConfigMethods","display");
        return 1;
    }else {
        method = setting->value("config_method").toString();
        m_pServiceInterface->call("UiSetConfigMethods",method);
        if (method == "display")
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void Projection::init_pin_button_status(int status)
{
    qDebug() << status;
    if (status) {
        pincodeBtn->setChecked(true);
    }
    else {
        pincodeBtn->setChecked(false);
    }
}

QWidget *Projection::pluginUi(){
    int res;
    int projectionstatus;
    int source_res;
    int pin_res;


    res = get_process_status();
    source_res=get_process_source_status();
    pin_res = get_pin_button_status();

    init_button_status(res);
    init_pin_button_status(pin_res);
    init_source_button_status(source_res);

    if (res == DAEMON_NOT_RUNNING){
        projectionstatus = NO_SERVICE;
    }
    else{
        QDBusMessage result = m_pServiceInterface->call("PreCheck");
        QList<QVariant> outArgs = result.arguments();
        projectionstatus = outArgs.at(0).value<int>();
        qDebug() << "---->" << projectionstatus;
    }
   
    ui->widget_3->show();
    ui->widget_4->show();
    ui->widget_5->hide();
    ui->widget_6->show();
    ui->widget_7->hide();
    ui->widget_9->show();
    ui->label->show();
    QLocale locale;
    if (locale.language() == QLocale::Chinese){
        ui->tipLabel->setText(tr("(No wireless network card is detected or the network card driver does not support it.)"));
        ui->tipLabel2->setText(tr("(No wireless network card is detected or the network card driver does not support it.)"));
    }
    else if (locale.language() == QLocale::English){
        setLabelText(ui->tipLabel,"(No wireless network card is detected or the network card driver does not support it.)");
        setLabelText(ui->tipLabel2,"(No wireless network card is detected or the network card driver does not support it.)");
        connect(this,&Projection::resize,[=](){
            setLabelText(ui->tipLabel,"(No wireless network card is detected or the network card driver does not support it.)");
            setLabelText(ui->tipLabel2,"(No wireless network card is detected or the network card driver does not support it.)");
        });
    }
    ui->tipLabel->show();
    ui->tipLabel2->show();
    if (dropDownLabel->isChecked){
        ui->widget_2->show();
    }
    else{
        ui->widget_2->hide();
    }

    //First, we check whether service process is running
    if (NO_SERVICE == projectionstatus) {
        QLocale locale;
        if (locale.language() == QLocale::Chinese){
            ui->tipLabel->setText(tr("(No wireless network card is detected or the network card driver does not support it.)"));
            ui->tipLabel2->setText(tr("(No wireless network card is detected or the network card driver does not support it.)"));
        }
        else if (locale.language() == QLocale::English){
            setLabelText(ui->tipLabel,"(No wireless network card is detected or the network card driver does not support it.)");
            setLabelText(ui->tipLabel2,"(No wireless network card is detected or the network card driver does not support it.)");
            connect(this,&Projection::resize,[=](){
                setLabelText(ui->tipLabel,"(No wireless network card is detected or the network card driver does not support it.)");
                setLabelText(ui->tipLabel2,"(No wireless network card is detected or the network card driver does not support it.)");
            });
        }
        ui->tipLabel->show();
        ui->tipLabel2->show();
        projectionBtn->setChecked(false);
        // miracastBtn->setChecked(false);
        projectionBtn->setCheckable(false);
        // miracastBtn->setCheckable(false);
    }
    //Then let's check whether hardware is ok
    else if (NOT_SUPPORT_P2P == projectionstatus) {
        projectionBtn->setChecked(false);
        // miracastBtn->setChecked(false);
        projectionBtn->setCheckable(false);
        // miracastBtn->setCheckable(false);
    }
    else if (SUPPORT_P2P_PERFECT == projectionstatus) {
        if(getWifiStatus())
        {
            qDebug()<<"wifi is on now";
            ui->tipLabel->hide();
            ui->tipLabel2->hide();
            projectionBtn->setCheckable(true);
            // miracastBtn->setCheckable(true);
        }
        else
        {
            qDebug()<<"wifi is off now";
            QLocale locale;
            if (locale.language() == QLocale::Chinese){
                ui->tipLabel->setText(tr("(WLAN is not turned on. You need to turn on WLAN to use this function.)"));
                ui->tipLabel2->setText(tr("(WLAN is not turned on. You need to turn on WLAN to use this function.)"));
            }
            else if (locale.language() == QLocale::English){
                setLabelText(ui->tipLabel,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                setLabelText(ui->tipLabel2,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                connect(this,&Projection::resize,[=](){
                    setLabelText(ui->tipLabel,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                    setLabelText(ui->tipLabel2,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                });
            }
            ui->tipLabel->show();
            ui->tipLabel2->show();
            projectionBtn->setChecked(false);
            // miracastBtn->setChecked(false);
            projectionBtn->setCheckable(false);
            // miracastBtn->setCheckable(false);
            ui->widget_6->update();
            ui->widget_7->update();
        }
    }
    else if (SUPPORT_P2P_WITHOUT_DEV == projectionstatus) {
        if(getWifiStatus())
        {
            qDebug()<<"wifi is on now";
            ui->tipLabel->hide();
            ui->tipLabel2->show();
            projectionBtn->setCheckable(true);
            // miracastBtn->setCheckable(false);
        }
        else
        {
            qDebug()<<"wifi is off now";
            QLocale locale;
            if (locale.language() == QLocale::Chinese){
                ui->tipLabel->setText(tr("(WLAN is not turned on. You need to turn on WLAN to use this function.)"));
                ui->tipLabel2->setText(tr("(WLAN is not turned on. You need to turn on WLAN to use this function.)"));
            }
            else if (locale.language() == QLocale::English){
                setLabelText(ui->tipLabel,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                setLabelText(ui->tipLabel2,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                connect(this,&Projection::resize,[=](){
                    setLabelText(ui->tipLabel,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                    setLabelText(ui->tipLabel2,"(WLAN is not turned on. You need to turn on WLAN to use this function.)");
                });
            }
            ui->tipLabel->show();
            ui->tipLabel2->show();
            projectionBtn->setChecked(false);
            projectionBtn->setCheckable(false);
            // miracastBtn->setCheckable(false);
        }
    }
    else if (OP_NO_RESPONSE == projectionstatus) {
        projectionBtn->setCheckable(false);
        // miracastBtn->setCheckable(false);
    }
    if (res == PROJECTION_RUNNING)//如果接收端已开启
    {
        // miracastBtn->setChecked(false);
        // miracastBtn->setCheckable(false);
    }
    if(source_res==1)//如果投射端已开启
    {
        projectionBtn->setChecked(false);
        projectionBtn->setCheckable(false);
    }


    //监听WLAN开关
    QDBusConnection::systemBus().connect(QString(), QString("/org/freedesktop/NetworkManager"), "org.freedesktop.NetworkManager", "PropertiesChanged", this, SLOT(netPropertiesChangeSlot(QMap<QString,QVariant>)));
    return pluginWidget;
}
void Projection::netPropertiesChangeSlot(QMap<QString, QVariant> property) {
    if (property.keys().contains("WirelessEnabled")) {
        qDebug()<<"WLAN status changed";
        pluginUi();
    }
}

const QString Projection::name() const {
    return QStringLiteral("projection");
}

bool Projection::isShowOnHomePage() const
{
    return false;
}

QIcon Projection::icon() const
{
    return QIcon::fromTheme("ukui-miracast-symbolic");
}

bool Projection::isEnable() const
{
    return true;
}

void Projection::projectionPinSlots(QString type, QString pin) {
    if (type.contains("clear")) {
        //m_pin->clear();
    } else {
        qDebug()<<pin;
        //m_pin->setText(pin);
    }
}

void Projection::projectionButtonClickSlots(bool status) {

    int config_method = pinCodeStatus(2);

    if (status){
        QDBusMessage result;
        if (config_method){
            result = m_pServiceInterface->call("Start",ui->projectionName->text(),"display");

        }
        else{
            result = m_pServiceInterface->call("Start",ui->projectionName->text(),"pbc");
        }
        QList<QVariant> outArgs = result.arguments();
        int res = outArgs.at(0).value<int>();
        if(!res)
        {
            // miracastBtn->setCheckable(false);
        }
    } else {
        m_pServiceInterface->call("Stop");
        int projectionstatus;
        QDBusMessage result = m_pServiceInterface->call("PreCheck");
        QList<QVariant> outArgs = result.arguments();
        projectionstatus = outArgs.at(0).value<int>();
        if (SUPPORT_P2P_PERFECT == projectionstatus){
            // miracastBtn->setCheckable(true);
        }
    }
}

void Projection::pincodeButtonClickSlots(bool status) {
    QDBusMessage result;
    QString path=QDir::homePath()+"/.config/miracast.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");
    
    if (status){
        pinCodeStatus(1);
        result = m_pServiceInterface->call("UiSetConfigMethods","display");
        setting->setValue("config_method","display");
    } else {
        pinCodeStatus(0);
        result = m_pServiceInterface->call("UiSetConfigMethods","pbc");
        setting->setValue("config_method","pbc");
    }
    setting->sync();
    setting->endGroup();
    QList<QVariant> outArgs = result.arguments();
    int res = outArgs.at(0).value<int>();
    if(res)
        qDebug()<<"Fail.";
    else
        qDebug()<<"Success.";
}

void Projection::pushButtonClickSlots() {

    QDBusInterface interface_2( "com.operate.wirelessscreen",
                              "/",
                              "com.operate.wirelessscreen",
                              QDBusConnection::sessionBus() );


    QDBusReply<int> m_result = interface_2.call("operate_wireless_screen",1);

    if (m_result.isValid()) {
        qDebug()<<"open res is"<<m_result<<endl;
    } else {
        qDebug()<<"open failed"<<endl;
    }

    if(m_result==1)
    {
        projectionBtn->setCheckable(false);
    }
}

void Projection::miracastsourceButtonClickSlots(bool status)
{
    QString path=QDir::homePath()+"/.config/miracastsource.ini";
    QSettings *setting=new QSettings(path,QSettings::IniFormat);
    setting->beginGroup("projection");

    if (status)
    {
        ui->pushButton->setEnabled(true);
        QString styleSheetDark = QString ("background:rgba(44,50,57,%1);color:white;");
        QString styleSheetLight = QString ("background:rgba(255,255,255,%1);color:rgba(58,67,78,0.25);");
        QGSettings gsetting(QString("org.ukui.style").toLocal8Bit());
        if (gsetting.get("style-name").toString() != "ukui-light"){
            ui->pushButton->setStyleSheet(styleSheetLight);
        }
        else{
            ui->pushButton->setStyleSheet(styleSheetDark);
        }
        projectionBtn->setCheckable(false);
        setting->setValue("miracastsource","true");
    }
    else
    {
        ui->pushButton->setEnabled(false);
        ui->pushButton->setStyleSheet("color: gray");
        projectionBtn->setCheckable(true);
        setting->setValue("miracastsource","false");

        QDBusInterface interface_2( "com.operate.wirelessscreen",
                                    "/",
                                    "com.operate.wirelessscreen",
                                    QDBusConnection::sessionBus() );

        interface_2.call("operate_wireless_screen",0);
    }
    setting->sync();
    setting->endGroup();
}

int Projection::pinCodeStatus(int key)
{
    if (key == 0){
        config_status = false;
        return 2;
    }
    else if (key == 1){
        config_status = true;
        return 2;
    }
    else{
        if (config_status)
            return 1;
        else
            return 0;
    }
}

void Projection::initComponent(){

    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
    addWgt->setMinimumSize(QSize(580, 64));
    addWgt->setMaximumSize(QSize(16777215, 64));
    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(base); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #2FB3E8; border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    QLabel * textLabel = new QLabel(tr("Add Bluetooths"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addItem(new QSpacerItem(8,10,QSizePolicy::Fixed));
    addLyt->addWidget(iconLabel);
    addLyt->addItem(new QSpacerItem(16,10,QSizePolicy::Fixed));
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addWgt->setLayout(addLyt);
    addWgt->hide();
}

QString Projection::translationPath() const
{
    return "/usr/share/projection/translations/%1.ts";
}

void Projection::initSearchText()
{
    //~ contents_path /projection/Open projection
}
