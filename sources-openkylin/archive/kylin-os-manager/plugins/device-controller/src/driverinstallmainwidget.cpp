#include "driverinstallmainwidget.h"

#include "include/globalsignal.h"

DriverInstallMainWidget::DriverInstallMainWidget(QWidget *parent)
        : QWidget(parent),
          m_mainLayout(new QHBoxLayout()),
          m_mainWidget(new QStackedWidget),
          m_driverInstallWidget(new DriverInstallWidget),
          m_driverIstallTopWidget(new QWidget),
          m_driverInstallLayout(new QHBoxLayout),
          m_failWidget(new DriverGetFailWidget),
          m_driverInstallSrollArea(new QScrollArea()) {
    initGui();
    initConnect();
}

void DriverInstallMainWidget::initGui() {

    m_driverInstallLayout->addWidget(m_driverInstallWidget);
    m_driverIstallTopWidget->setLayout(m_driverInstallLayout);
    m_driverInstallSrollArea->setWidget(m_driverIstallTopWidget);
    m_driverInstallSrollArea->setWidgetResizable(true);
    m_driverInstallSrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_driverInstallSrollArea->setFrameShape(QFrame::NoFrame);
    m_driverInstallSrollArea->setContentsMargins(0, 0, 0, 0);
    m_driverInstallWidget->adjustSize();

    m_mainWidget->addWidget(m_driverInstallSrollArea);
    m_mainWidget->addWidget(m_failWidget);
    m_mainWidget->setCurrentWidget(m_driverInstallSrollArea);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_mainWidget);
    this->setLayout(m_mainLayout);
}

void DriverInstallMainWidget::initConnect() {

    connect(g_signal, &GlobalSignal::updateInstallContentSucceed,
            [=]() { m_mainWidget->setCurrentWidget(m_driverInstallSrollArea); });
    connect(g_signal, &GlobalSignal::getHwInfoFailed, [=]() { m_mainWidget->setCurrentWidget(m_failWidget); });
}
