#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDateTime>

#include "kom-configure.h"
#include "senddatawidget.h"
#include "sendoldsysinfo.h"
#include "dbus_args.h"

OldSysInfo::OldSysInfo(QWidget *parent)
{
    initUI();
    getOldInfo();
}

OldSysInfo::~OldSysInfo()
{
    m_dbus->deleteLater();

}

void OldSysInfo::initUI()
{
    this->setFixedSize(600, 264);

    m_labelId = new K::TextLabel(this);
    m_labelName = new K::TextLabel(this);
    m_labelPwd = new K::TextLabel(this);
    m_labelTip = new K::TextLabel(this);

    m_editId = new QLineEdit(this);
    m_editName = new QLineEdit(this);
    m_editPwd = new QLineEdit(this);

    m_btnShareData = new QPushButton(this);
    m_layoutVMain = new QVBoxLayout();
    m_layoutHMain = new QHBoxLayout();
    m_layoutF = new QFormLayout();

    m_layoutF->setWidget(0,QFormLayout::LabelRole, m_labelId);
    m_layoutF->setWidget(0, QFormLayout::FieldRole, m_editId);
    m_layoutF->setWidget(1,QFormLayout::LabelRole, m_labelName);
    m_layoutF->setWidget(1, QFormLayout::FieldRole, m_editName);
    m_layoutF->setWidget(2,QFormLayout::LabelRole, m_labelPwd);
    m_layoutF->setWidget(2, QFormLayout::FieldRole, m_editPwd);
    m_layoutF->setWidget(3, QFormLayout::FieldRole, m_labelTip);
    m_layoutF->setWidget(4, QFormLayout::FieldRole, m_btnShareData);
    m_layoutF->setHorizontalSpacing(16);
    m_layoutF->setVerticalSpacing(8);

    m_layoutHMain->addStretch();
    m_layoutHMain->addLayout(m_layoutF);
    m_layoutHMain->addStretch();

    m_layoutVMain->addStretch();
    m_layoutVMain->addLayout(m_layoutHMain);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);
    this->setLayout(m_layoutVMain);

    m_labelName->SetText(GlobalData::g_textOldSysUserName);
    m_labelId->SetText(GlobalData::g_textOldSysSignage);
    m_labelPwd->SetText(GlobalData::g_textOldSysCheckCode);
    m_labelTip->SetText(GlobalData::g_textTip);
    m_labelTip->SetFontColor(Qt::red);
    m_labelTip->hide();

    m_editId->setFixedSize(310, 36);
    m_editId->setReadOnly(true);
    m_editName->setFixedSize(310, 36);
    m_editPwd->setFixedSize(310, 36);

    m_btnShareData->setFixedSize(110, 36);
    m_btnShareData->setText(GlobalData::g_textBtnStartSharing);
    m_btnShareData->setText(GlobalData::g_textBtnSharing);
    GlobalData::getInstance()->setHighColorBtn(m_btnShareData);

    m_dbus = new QDBusInterface(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                QDBusConnection::systemBus());

    connect(m_btnShareData, &QPushButton::clicked, this, &OldSysInfo::slotStarShare);
    connect(m_editPwd, &QLineEdit::textChanged, this, &OldSysInfo::changeBtnType);
    connect(m_editName, &QLineEdit::textChanged, this, &OldSysInfo::changeBtnType);
}

void OldSysInfo::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = QApplication::palette().window().color();
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    QColor highLightColor = opt.palette.highlight().color();
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 16, 16); // 绘制圆角
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);    
}

void OldSysInfo::getOldInfo()
{
    QFile file(machineId_path);
    //设置打开方式
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << " ===== oldSysInfo ===== Can't open /etc/machine_id! === " <<endl;
    }
    //用QByteArray类去接收读取的信息
    QByteArray array=file.readAll();
    QString allStr = QString(array).trimmed();
    m_editId->setText(allStr);
    file.close();

    QString name = qgetenv("USER");
    m_editName->setText(name);


	kom::Configure configure;

    QString code = configure.value(CONFIG_PLUGIN_REPLACEMENT_ASSISTANT, CONFIG_PLUGIN_REPLACEMENT_ASSISTANT_CODE, "").toString();
    if (code == "") {
        qsrand(QDateTime::currentMSecsSinceEpoch() / 1000);
        createRandomCode();
        configure.setValue(CONFIG_PLUGIN_REPLACEMENT_ASSISTANT, CONFIG_PLUGIN_REPLACEMENT_ASSISTANT_CODE, m_code);
    } else {
        m_code = code;
    }
    m_editPwd->setText(m_code);
}

void OldSysInfo::createRandomCode()
{
    m_code.clear();
    for (int i = 0; i < m_num; ++i) {
        int num = qrand() % 3;
        if (num == 0) {
            m_code += QString::number(qrand() % 10);
        } else if (num == 1) {
            int temp = 'A';
            m_code += static_cast<QChar>(temp + qrand() % 26);
        } else if (num == 2) {
            int temp = 'a';
            m_code += static_cast<QChar>(temp + qrand() % 26);
        }
    }

    return;
}
void OldSysInfo::changeBtnType()
{
    if (m_editName->text() == m_nameStr && m_editPwd->text() == m_code) {
        return;
    }
    m_btnShareData->setText(GlobalData::g_textBtnStartSharing);
    m_btnShareData->setEnabled(true);
    slotStopShare();
}

bool OldSysInfo::regularTestCode(QString string)
{
    QRegExp rx("^[a-zA-Z0-9\u4e00-\u9fa5]+$"); //判断是否有特殊字符
    if (!rx.exactMatch(m_editPwd->text())) {
        return false;
    } else {
        return true;
    }
}

void OldSysInfo::initZeroConf()
{
    connect(&m_zeroConf, &QZeroConf::serviceAdded, this, [=](QZeroConfService zcs){
        qDebug() << "serviceAdded: " << zcs->name()<<"|"<< zcs->ip().toString() <<"|"<<zcs->type();
    });
    
    QString nameStr = m_editId->text() + "###" +  m_editName->text();
    QByteArray byteArrayName = nameStr.toUtf8(); 
    const char* name = byteArrayName.constData();

    m_zeroConf.clearServiceTxtRecords();
    m_zeroConf.startServicePublish(name, "_komt_replacement._tcp", "", 11437);
    // m_zeroConf.startBrowser("_komt_replacement._tcp");
}

void OldSysInfo::slotStarShare()
{
    m_code = m_editPwd->text();
    if (!regularTestCode(m_code)) {
        m_labelTip->show();
        return;
    }

    // 验证码检测，若改变，更改配置文件的验证码
	kom::Configure configure;
    QString code = configure.value(CONFIG_PLUGIN_REPLACEMENT_ASSISTANT, CONFIG_PLUGIN_REPLACEMENT_ASSISTANT_CODE, "").toString();
    if (code != m_code) {
        configure.setValue(CONFIG_PLUGIN_REPLACEMENT_ASSISTANT, CONFIG_PLUGIN_REPLACEMENT_ASSISTANT_CODE, m_code);
    }
    // 改变按钮状态
    m_btnShareData->setEnabled(false);
    m_btnShareData->setText(GlobalData::g_textBtnSharing);

    // 启动共享(rsync)
    m_nameStr = m_editName->text();
    QString senderName = m_editId->text() + m_nameStr;
    QDBusMessage msg = m_dbus->call("senderStartServer", senderName, m_code);
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : senderStartServer" << msg.errorMessage();
    }
        

    // 启动共享(ip共享)
    initZeroConf();
}

void OldSysInfo::slotStopShare()
{
    // IP发现断开
    m_zeroConf.stopServicePublish();
    // dbus执行关闭迁移
    
    QDBusMessage msg = m_dbus->call("senderStopServer");
    if (msg.errorMessage() != "") {
        qDebug() << "dbus interface error : senderStopServer" << msg.errorMessage();
    }
    m_btnShareData->setText(GlobalData::g_textBtnStartSharing);
    m_btnShareData->setEnabled(true);
}