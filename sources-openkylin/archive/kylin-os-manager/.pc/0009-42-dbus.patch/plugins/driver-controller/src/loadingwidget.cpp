#include "loadingwidget.h"
#include "kysdk/applications/gsettingmonitor.h"
#include <QPixmap>

#include "include/globalsignal.h"
#include "include/common.h"

LoadingWidget::LoadingWidget(QWidget *parent)
        : QWidget(parent),
          m_loadingIcon(new QLabel()),
          m_loadingLabel(new QLabel()),
          m_iconLayout(new QHBoxLayout),
          m_mainLayout(new QVBoxLayout()) {
    initConnect();
    initGUi();
}

LoadingWidget::~LoadingWidget() {

}

void LoadingWidget::initConnect() {
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &LoadingWidget::themeSetting);
}

void LoadingWidget::initGUi() {
    themeSetting();
    m_loadingLabel->setText(tr("The driver information is being detected, please wait a moment."));

    m_iconLayout->addWidget(m_loadingIcon);
    m_iconLayout->setContentsMargins(0, 0, 0, 0);
    m_iconLayout->setAlignment(Qt::AlignCenter);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_iconLayout);
    m_mainLayout->addSpacing(8);
    m_mainLayout->addWidget(m_loadingLabel);
    m_mainLayout->addStretch();

    this->setLayout(m_mainLayout);
}

void LoadingWidget::themeSetting() {
    QString styleName = kdk::GsettingMonitor::getSystemTheme().toString();
    if (styleName == "ukui-black" || styleName == "ukui-dark") {
        QPixmap pixmap;
        pixmap.load(":/ukui-occupation-map-dark.png");
        pixmap = pixmap.scaled(PIXMAPSIZE, Qt::KeepAspectRatio);
        m_loadingIcon->setPixmap(pixmap);
    } else {
        QPixmap pixmap;
        pixmap.load(":/ukui-occupation-map.png");
        pixmap = pixmap.scaled(PIXMAPSIZE, Qt::KeepAspectRatio);
        m_loadingIcon->setPixmap(pixmap);
    }
}



