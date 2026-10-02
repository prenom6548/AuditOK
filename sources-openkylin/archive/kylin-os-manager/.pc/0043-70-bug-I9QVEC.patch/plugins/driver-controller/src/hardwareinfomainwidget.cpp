#include "hardwareinfomainwidget.h"

#include "include/globalsignal.h"

HardwareInfoMainWidget::HardwareInfoMainWidget(QWidget *parent)
        : QWidget(parent),
          m_mainLayout(new QHBoxLayout()),
          m_mainWidget(new QStackedWidget),
          m_hardwareInfoSrollArea(new QScrollArea()),
          m_hardwareinfoWidget(new HardWareInfoWidget),
          m_hardwareinfoGetFailWidget(new HardwareInfoGetFailWidget) {
    initGui();
    initConnect();
}

void HardwareInfoMainWidget::initGui() {

    m_hardwareInfoSrollArea->setWidget(m_hardwareinfoWidget);
    m_hardwareInfoSrollArea->setWidgetResizable(true);
    m_hardwareInfoSrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_hardwareInfoSrollArea->setFrameShape(QFrame::NoFrame);
    m_hardwareInfoSrollArea->setContentsMargins(0, 0, 0, 0);
    m_hardwareinfoWidget->adjustSize();

    m_mainWidget->addWidget(m_hardwareInfoSrollArea);
    m_mainWidget->addWidget(m_hardwareinfoGetFailWidget);
    m_mainWidget->setCurrentWidget(m_hardwareInfoSrollArea);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_mainWidget);
    this->setLayout(m_mainLayout);
}

void HardwareInfoMainWidget::initConnect() {
    connect(g_signal, &GlobalSignal::getHwInfoComplete, [=]() {
        if (g_infogetter->m_getsucceed) {
            m_mainWidget->setCurrentWidget(m_hardwareInfoSrollArea);
        }
    });
    connect(g_signal, &GlobalSignal::getHwInfoFailed,
            [=]() { m_mainWidget->setCurrentWidget(m_hardwareinfoGetFailWidget); });
}
