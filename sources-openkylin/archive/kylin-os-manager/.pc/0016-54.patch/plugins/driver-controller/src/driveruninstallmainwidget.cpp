#include "driveruninstallmainwidget.h"
#include "include/globalsignal.h"

DriverUninstallMainWidget::DriverUninstallMainWidget(QWidget *parent)
        : QWidget(parent),
          m_stackWidget(new QStackedWidget),
          m_noDriverUninstallWidget(new NoDriverUninstallPage),
          m_driverUninstallWidget(new DriverUninstallWidget),
          m_driverGetFailWidget(new DriverGetFailWidget),
          m_mainLayout(new QHBoxLayout) {
    initGui();
    initConnection();
}

void DriverUninstallMainWidget::initGui() {
    m_stackWidget->addWidget(m_noDriverUninstallWidget);
    m_stackWidget->addWidget(m_driverUninstallWidget);
    m_stackWidget->addWidget(m_driverGetFailWidget);
    m_stackWidget->setCurrentWidget(m_noDriverUninstallWidget);

    m_mainLayout->addWidget(m_stackWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    this->setLayout(m_mainLayout);
}

void DriverUninstallMainWidget::initConnection() {
    connect(g_signal, &GlobalSignal::switchUninstallPage, this, &DriverUninstallMainWidget::switchToUninstallWidget);
    connect(g_signal, &GlobalSignal::switchNoUninstallPage, this,
            &DriverUninstallMainWidget::switchToNoDriverUnistallWidget);
    connect(g_signal, &GlobalSignal::getHwInfoFailed, [=]() { m_stackWidget->setCurrentWidget(m_driverGetFailWidget); });
}

void DriverUninstallMainWidget::switchToUninstallWidget() {
    m_stackWidget->setCurrentWidget(m_driverUninstallWidget);
}

void DriverUninstallMainWidget::switchToNoDriverUnistallWidget() {
    m_stackWidget->setCurrentWidget(m_noDriverUninstallWidget);
}
