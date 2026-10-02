#include "mainstackwidget.h"
#include "include/common.h"
#include "include/globalsignal.h"

MainStackWidget::MainStackWidget(QWidget *parent)
        : QStackedWidget(parent),
        m_selectionWidget(new SelectionWidget()),
        m_loadingWidget(new LoadingWidget()){
    initGsetting();
    initConnection();
    initGui();
}

void MainStackWidget::initGsetting() {
    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
    }
}

void MainStackWidget::initConnection() {
    connect(m_themeData, &QGSettings::changed, g_signal, &GlobalSignal::themeChangeSignal);
}

void MainStackWidget::initGui() {
    this->addWidget(m_loadingWidget);
    this->addWidget(m_selectionWidget);
    this->setCurrentWidget(m_loadingWidget);
}
