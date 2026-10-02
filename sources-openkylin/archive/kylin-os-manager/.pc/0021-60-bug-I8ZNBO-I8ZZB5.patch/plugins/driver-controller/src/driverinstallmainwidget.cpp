#include "driverinstallmainwidget.h"

#include "include/globalsignal.h"

DriverInstallMainWidget::DriverInstallMainWidget(QWidget *parent)
        : QWidget(parent),
          m_mainLayout(new QHBoxLayout()),
          m_mainWidget(new QStackedWidget),
          m_driverInstallWidget(new DriverInstallWidget),
          m_failWidget(new DriverGetFailWidget) {
    initGui();
    initConnect();
}

void DriverInstallMainWidget::initGui() {
    m_mainWidget->addWidget(m_driverInstallWidget);
    m_mainWidget->addWidget(m_failWidget);
    m_mainWidget->setCurrentWidget(m_driverInstallWidget);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_mainWidget);
    this->setLayout(m_mainLayout);
}

void DriverInstallMainWidget::initConnect() {

    connect(g_signal, &GlobalSignal::updateInstallContentSucceed, [=]() { m_mainWidget->setCurrentWidget(m_driverInstallWidget); });
    connect(g_signal, &GlobalSignal::getHwInfoFailed, [=]() { m_mainWidget->setCurrentWidget(m_failWidget); });
}
