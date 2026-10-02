#include "mainwidget.h"
#include "include/common.h"
#include "include/globalsignal.h"

MainWidget::MainWidget(QWidget *parent)
        : QWidget(parent),
          m_stackWidget(new QStackedWidget()),
          m_selectionWidget(new SelectionWidget()),
          m_loadingWidget(new LoadingWidget()),
          m_mainLayout(new QHBoxLayout()) {
    m_getinfoThread = new GetInfoThread();
    m_getinfoThread->start();
    initGsetting();
    initConnection();
    initGui();
}

void MainWidget::initGsetting() {
    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }
}

void MainWidget::initConnection() {
    connect(m_themeData, &QGSettings::changed, g_signal, &GlobalSignal::themeChangeSignal);
    connect(g_signal, &GlobalSignal::getHwInfoComplete, m_getinfoThread, &QThread::quit);
    connect(g_signal, &GlobalSignal::getHwInfoComplete, m_getinfoThread, &QThread::deleteLater);
    connect(g_signal, &GlobalSignal::switchInstallPage, [=]() { m_stackWidget->setCurrentWidget(m_selectionWidget); });
}

void MainWidget::initGui() {
    this->setProperty("useSystemStyleBlur", true);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    m_stackWidget->addWidget(m_loadingWidget);
    m_stackWidget->addWidget(m_selectionWidget);
    m_stackWidget->setCurrentWidget(m_loadingWidget);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_stackWidget, Qt::AlignCenter);

    this->setLayout(m_mainLayout);
    this->setMinimumSize(MAINWIDGETSIZE);
}

