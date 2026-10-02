#include "hardwareinfogetfailwidget.h"


#include "include/globalsignal.h"
#include "include/common.h"

HardwareInfoGetFailWidget::HardwareInfoGetFailWidget(QWidget *parent)
        : QWidget(parent),
          m_failIcon(new QLabel()),
          m_failLabel(new QLabel()),
          m_retryBtn(new QPushButton()),
          m_iconLayout(new QHBoxLayout),
          m_mainLayout(new QVBoxLayout()) {
    initConnect();
    initGUi();
}

HardwareInfoGetFailWidget::~HardwareInfoGetFailWidget() {

}

void HardwareInfoGetFailWidget::initConnect() {
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &HardwareInfoGetFailWidget::themeSetting);
    connect(m_retryBtn, &QPushButton::clicked, g_signal, &GlobalSignal::reDetectSignal);

}

void HardwareInfoGetFailWidget::initGUi() {
    themeSetting();
    m_failLabel->setText(tr("Failed to obtain driver information, please try again later."));
    m_retryBtn->setText(tr("Retry"));
    m_retryBtn->setFixedSize(UPDATEBTNSIZE);

    m_iconLayout->addWidget(m_failIcon);
    m_iconLayout->setContentsMargins(0, 0, 0, 0);
    m_iconLayout->setAlignment(Qt::AlignCenter);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_iconLayout);
    m_mainLayout->addSpacing(8);
    m_mainLayout->addWidget(m_failLabel);
    m_mainLayout->addSpacing(24);
    m_mainLayout->addWidget(m_retryBtn, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(80);
    m_mainLayout->addStretch();

    this->setLayout(m_mainLayout);
}

void HardwareInfoGetFailWidget::themeSetting() {
    QString styleName = kdk::GsettingMonitor::getSystemTheme().toString();
    if (styleName == "ukui-black" || styleName == "ukui-dark") {
        QPixmap pixmap;
        pixmap.load(":/lodingerr-dark.png");
        pixmap = pixmap.scaled(PIXMAPSIZE, Qt::KeepAspectRatio);
        m_failIcon->setPixmap(pixmap);
    } else {
        QPixmap pixmap;
        pixmap.load(":/lodingerr-light.png");
        pixmap = pixmap.scaled(PIXMAPSIZE, Qt::KeepAspectRatio);
        m_failIcon->setPixmap(pixmap);
    }
}