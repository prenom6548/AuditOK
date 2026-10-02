#include <QDebug>
#include "../globaldata.h"
#include "acceptdatawidget.h"
#include "dbus_args.h"
#include <QMessageBox>

AcceptDataWidget::AcceptDataWidget(QWidget *parent)
{
    initUI();
    initConn();
}

AcceptDataWidget::~AcceptDataWidget()
{
}

void AcceptDataWidget::initUI()
{
    m_acceptStatus = new AcceptDataStatus(this);
    m_stackedWid = new QStackedWidget(this);
    m_accpetChooseWidget = new AcceptChooseDevice(this);
    m_accperChooseData = new AcceptChooseData(this);
    m_acceptMigration = new AcceptMigrationData(this);
    m_acceptReboot = new AcceptRebootWidget(this);
    m_rebootWidget = new QWidget(this);

    m_btnBack = new QPushButton(this);
    m_btnNext = new QPushButton(this);
    m_btnCannel = new QPushButton(this);
    m_btnReboot = new QPushButton(this);
    m_btnTip = new QPushButton(this);
    m_labelTip = new K::TextLabel(this);

    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                          QString("sigReceTestSuccess"), this, SLOT(ConnSuccess()));
  
    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                          QString("sigReceProcessFail"), this, SLOT(ConnFile(int)));

    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                          QString("sigMigrateDataInfo"), this, SLOT(slotMigrationDataInfo(QStringList)));
    
    QDBusConnection::systemBus().connect(dbus::DBUS_SERVICENAME, dbus::DBUS_PARH_DATA_MIGRATION, dbus::DBUS_INTERFACE_DATA_MIGRATION,
                                          QString("sigMigrateDataSize"), this, SLOT(slotMigrationDataSize(qint64)));

    m_btnBack->setText(GlobalData::g_textBackBtn);
    m_btnNext->setText(GlobalData::g_textNextBtn);
    m_btnCannel->setText(GlobalData::g_textRectMigrateCannel);
    m_btnCannel->hide();
    m_btnReboot->setText(GlobalData::g_textRectReboot);
    m_btnReboot->hide();

    m_labelTip->SetText(GlobalData::g_textRectTipReboot);
    m_labelTip->SetPixelSize(12);
    m_labelTip->SetFontColor(QColor(89, 89, 89));
    m_btnTip->setFixedSize(20, 20);
    m_btnTip->setIconSize(QSize(16, 16));               
    m_btnTip->setIcon(GlobalData::g_iconTips); 
    m_btnTip->setFlat(true);
    m_btnTip->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_btnReboot->setFixedSize(96, 36);

    GlobalData::getInstance()->setHighColorBtn(m_btnReboot);
    GlobalData::getInstance()->setHighColorBtn(m_btnNext);

    setRebootWidget();  
    setLayout();

}


void AcceptDataWidget::initConn()
{
    connect(m_btnBack, &QPushButton::clicked, this, &AcceptDataWidget::slotBackPage);
    connect(m_btnNext, &QPushButton::clicked, this, &AcceptDataWidget::slotNextPage);
    connect(m_btnCannel, &QPushButton::clicked, this, &AcceptDataWidget::slotCannel);
    connect(m_btnReboot, &QPushButton::clicked, this, &AcceptDataWidget::slotReboot);
    connect(m_accperChooseData, &AcceptChooseData::sigBackPage, this, &AcceptDataWidget::slotBackPage);
    connect(m_accperChooseData, &AcceptChooseData::sigChooseData, this, &AcceptDataWidget::slotMigrationData);
    connect(m_acceptMigration, &AcceptMigrationData::sigMigratDataFinish, this, &AcceptDataWidget::slotMigrationDataFinish);
}

void AcceptDataWidget::setRebootWidget()
{
    QHBoxLayout *m_hlayoutTip = new QHBoxLayout();
    m_hlayoutTip->addStretch();
    m_hlayoutTip->addWidget(m_btnTip);
    m_hlayoutTip->addSpacing(4);
    m_hlayoutTip->addWidget(m_labelTip);
    m_hlayoutTip->addStretch();

    QVBoxLayout *vRebootLayout = new QVBoxLayout(m_rebootWidget);
    vRebootLayout->addWidget(m_acceptReboot, 0, Qt::AlignHCenter);
    vRebootLayout->addSpacing(32);
    vRebootLayout->addWidget(m_btnReboot, 0, Qt::AlignHCenter);
    vRebootLayout->addSpacing(12);
    vRebootLayout->addLayout(m_hlayoutTip);
    vRebootLayout->addStretch();
    vRebootLayout->setMargin(0);
    vRebootLayout->setSpacing(0);
}

void AcceptDataWidget::setLayout()
{
    m_layoutVMain = new QVBoxLayout(this);
    QHBoxLayout *m_hlayout = new QHBoxLayout();

    m_hlayout->addStretch();
    m_hlayout->addWidget(m_btnBack);
    m_hlayout->addSpacing(16);
    m_hlayout->addWidget(m_btnNext);
    m_hlayout->addWidget(m_btnCannel);
    m_hlayout->addStretch();

    m_stackedWid->addWidget(m_accpetChooseWidget);
    m_stackedWid->addWidget(m_accperChooseData);
    m_stackedWid->addWidget(m_acceptMigration);
    m_stackedWid->addWidget(m_rebootWidget);
    m_stackedWid->setCurrentIndex(0);
    m_acceptStatus->setStatusStyle(AcceptDataStatus::FlowPart::FirstPart);

    m_layoutVMain->addWidget(m_acceptStatus, 0, Qt::AlignHCenter);
    m_layoutVMain->addWidget(m_stackedWid, 0, Qt::AlignHCenter);
    m_layoutVMain->addSpacing(32);
    m_layoutVMain->addLayout(m_hlayout);
    m_layoutVMain->addStretch();
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);
}

void AcceptDataWidget::slotBackPage()
{
    // 上一步，判断“当前界面”所在步骤
    if (m_isDataMigration) {
        m_stackedWid->setCurrentIndex(0);
        m_acceptStatus->setStatusStyle(AcceptDataStatus::FlowPart::FirstPart);
        m_btnNext->show();
        m_btnBack->show();
        m_accperChooseData->initAppWidget();
        m_isDataMigration = false;
    } else {
        m_accpetChooseWidget->stopZeroConf();
        m_btnNext->setEnabled(true);
        emit sigBackPage();
    }
}

void AcceptDataWidget::slotNextPage()
{
    if(!m_accpetChooseWidget->getAcceptDeviceInfo()) {
        return;
    }
    m_btnNext->setEnabled(false);
    QStringList list = m_accpetChooseWidget->getInfo();
    qDebug() << " === slotNextPage === " << list;

    // dbus 判断连接是否正确
    QString ip = list.at(0);
    QString name = list.at(1);
    QString pwd = list.at(2);
    GlobalData::getInstance()->dbusReceiverTestServer(pwd, ip, name);
    // 监听信号
}

// 开始迁移
void AcceptDataWidget::slotMigrationData(ChooseDataType type)
{
    m_stackedWid->setCurrentIndex(2);
    m_acceptStatus->setStatusStyle(AcceptDataStatus::FlowPart::ThirdPart);

    GlobalData::getInstance()->dbusReceiverMigrateData(type);
    m_btnCannel->show();
}

// 迁移数据信息
void AcceptDataWidget::slotMigrationDataInfo(QStringList list)
{
    m_acceptMigration->slotMigrationDataInfo(list);
}

// 获取迁移数据大小
void AcceptDataWidget::slotMigrationDataSize(qint64 num)
{
    m_accperChooseData->slotMigrationDataSize(num);
    m_acceptMigration->slotSetProgressRange(num);
}

// 取消迁移
void AcceptDataWidget::slotCannel()
{
    m_btnCannel->hide();
    m_stackedWid->setCurrentIndex(1);
    m_acceptStatus->setStatusStyle(AcceptDataStatus::FlowPart::SecondPart);
    // m_btnReboot->show();
    
    GlobalData::getInstance()->dbusReceiverStopRsync();

    this->update();
}

// dbus返回密码正确
void AcceptDataWidget::ConnSuccess()
{
    m_isDataMigration = true;
    m_stackedWid->setCurrentIndex(1);
    m_acceptStatus->setStatusStyle(AcceptDataStatus::FlowPart::SecondPart);
    m_btnNext->hide();
    m_btnBack->hide();
    m_btnNext->setEnabled(true);
    m_accpetChooseWidget->showOrHideTip(false);
}

// dbus返回错误信息
void AcceptDataWidget::ConnFile(int errorType)
{
    m_btnNext->setEnabled(true);
    QMessageBox msgBox;
    bool showMsgBox = false;
    switch (errorType)
    {
        // 用户名和密误填写错误
    case ErrorTypeEnum::ErrorRsyncPassWord:
        showMsgBox = false;
        m_accpetChooseWidget->showOrHideTip(true);
        break;
        // 对方没开机、防火墙阻挡
    case ErrorTypeEnum::ErrorConnectFailed:
        showMsgBox = true;
        msgBox.setText(GlobalData::g_textRectMsgNetErr3);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(GlobalData::g_textRectMsgSure , QMessageBox::ActionRole);
        break;
        // rsync服务没有正确启动
    case ErrorTypeEnum::ErrorServerNotStart:
        showMsgBox = true;
        msgBox.setText(GlobalData::g_textRectMsgNetErr3);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(GlobalData::g_textRectMsgSure , QMessageBox::ActionRole);
        break;
        // rsync命令语法错误
    case ErrorTypeEnum::ErrorRsyncCommand:
        showMsgBox = false;
        break;
        // 配置文件模块名称书写错误
    case ErrorTypeEnum::ErrorRsyncServerConfig:
        showMsgBox = false;
        break;
    
    default:
        break;
    }
    if (showMsgBox) {
        msgBox.exec();
    }

}

// 开启IP检测
void AcceptDataWidget::ipAutoDiscovery()
{
    m_accpetChooseWidget->startZeroConf();
}

// 重启
void AcceptDataWidget::slotReboot()
{
    GlobalData::getInstance()->dbusReceiverReboot();
}

void AcceptDataWidget::slotMigrationDataFinish()
{
    m_stackedWid->setCurrentIndex(3);
    m_acceptStatus->setStatusStyle(AcceptDataStatus::FlowPart::LastPart);
    m_acceptReboot->setLabelsText();
    m_btnReboot->show();
    m_btnCannel->hide();
}