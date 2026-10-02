#include <QDebug>
#include <QProcess>
#include <QCoreApplication>

#include "choosewidget.h"


ChooseWidget::ChooseWidget(QWidget *parent)
{
    initUI();
}

ChooseWidget::~ChooseWidget()
{

}

void ChooseWidget::initUI()
{
    this->setFixedSize(GlobalData::g_sizeWidget);
    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
    m_stackedWid = new QStackedWidget(this);
    m_chooseDevice = new ChooseDeviceWidget(this);
    m_chooseSys = new ChooseSysWidget(this);
    m_sendWidget = new SendDataWidget(this);
    m_acceptWidget = new AcceptDataWidget(this);

    m_layoutVMain = new QVBoxLayout(this);
    m_layoutVMain->addWidget(m_stackedWid);
    m_layoutVMain->setSpacing(0);
    m_layoutVMain->setMargin(0);

    m_stackedWid->addWidget(m_chooseDevice);
    m_stackedWid->addWidget(m_chooseSys);
    m_stackedWid->addWidget(m_sendWidget);
    m_stackedWid->addWidget(m_acceptWidget);
    m_stackedWid->setCurrentIndex(0);

    connect(m_chooseDevice, &ChooseDeviceWidget::sigNextPage, this, &ChooseWidget::slotNextPage);
    connect(m_chooseSys, &ChooseSysWidget::sigNextPage, this, &ChooseWidget::slotNextPage);
    connect(m_chooseSys, &ChooseSysWidget::sigBackPage, this, &ChooseWidget::slotBackPage);
    connect(m_sendWidget, &SendDataWidget::sigBackPage, this, &ChooseWidget::slotBackPage);
    connect(m_acceptWidget, &AcceptDataWidget::sigBackPage, this, [=](){
        m_stackedWid->setCurrentIndex(1);
    });

}

ChooseWidget *ChooseWidget::getInstance()
{
    static ChooseWidget *instance = nullptr;
    if (nullptr == instance) {
        instance = new ChooseWidget();
    }
    return instance;
}

void ChooseWidget::slotNextPage(SysType type)
{
    QProcess process;
    switch (type)
    {
    case SysType::NewSys:
        m_stackedWid->setCurrentIndex(1);
        break;
    case SysType::OldSys:
        m_stackedWid->setCurrentIndex(2);
        break;
    case SysType::LinuxSys:
        m_stackedWid->setCurrentIndex(3);
        m_acceptWidget->ipAutoDiscovery();
        break;
    case SysType::WinSys:
        // process.start("/usr/share/kylin-os-manager/win-data-migration/win-data-migration");
        // process.waitForFinished();
        // QString sAppName = "BaseSetTool";
        QProcess::startDetached("/usr/share/kylin-os-manager/treasure-chest/kylin-win-data-migration/kylin-win-data-migration");
        QCoreApplication::exit();
        break;
    default:
        break;
    }

}

void ChooseWidget::slotBackPage()
{
    m_stackedWid->setCurrentIndex(0);
}
