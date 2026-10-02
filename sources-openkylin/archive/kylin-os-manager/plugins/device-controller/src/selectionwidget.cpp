#include "selectionwidget.h"
#include "include/common.h"
#include "include/globalsignal.h"
#include <QScrollBar>

SelectionWidget::SelectionWidget(QWidget *parent) :
        QWidget(parent),
        m_tabBtnBar(new kdk::KTabBar(kdk::KTabBarStyle::SegmentLight, this)),
        m_stackWidget(new QStackedWidget()),
        m_driverInstallWidget(new DriverInstallMainWidget()),
        m_hardwareInfoMainWidget(new HardwareInfoMainWidget()),
        m_tabBtnLayout(new QHBoxLayout()),
        m_mainLayout(new QVBoxLayout()) {
    initGui();
    initConnect();
}

void SelectionWidget::initGui() {
    this->setProperty("useSystemStyleBlur", true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    m_tabBtnBar->addTab(tr("Hardware Info"));
    m_tabBtnBar->addTab(tr("Driver Manager"));
    m_tabBtnBar->setFixedWidth(200);
    m_tabBtnLayout->addWidget(m_tabBtnBar);

    m_stackWidget->addWidget(m_hardwareInfoMainWidget);
    m_stackWidget->addWidget(m_driverInstallWidget);
    m_stackWidget->setCurrentWidget(m_hardwareInfoMainWidget);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addLayout(m_tabBtnLayout);
    m_mainLayout->addSpacing(24);
    m_mainLayout->addWidget(m_stackWidget);
    m_mainLayout->setAlignment(Qt::AlignCenter);

    this->setLayout(m_mainLayout);
}

void SelectionWidget::initConnect() {
    connect(m_tabBtnBar, &kdk::KTabBar::currentChanged, this, [this](int index) {
        m_stackWidget->setCurrentIndex(index);
    });

    connect(g_signal, &GlobalSignal::jumpToHardWareInfoPage, [=]() { m_tabBtnBar->setCurrentIndex(0); });
    connect(g_signal, &GlobalSignal::jumpToDriverManagerPage, [=]() { m_tabBtnBar->setCurrentIndex(1); });
}
