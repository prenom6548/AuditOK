#include "nodriveruninstallpage.h"
#include <kysdk/applications/gsettingmonitor.h>
#include "include/common.h"

NoDriverUninstallPage::NoDriverUninstallPage(QWidget *parent)
        : QWidget(parent),
          m_nodriverIcon(new QLabel()),
          m_nodriverText(new QLabel()),
          m_mainVboxLayout(new QVBoxLayout()) {
    initGui();
    initConnect();
    initTheme();
}

void NoDriverUninstallPage::initGui() {
    m_nodriverIcon->setFixedSize(NoDriverIconSize);

    m_nodriverText->setEnabled(false);
    m_nodriverText->setText(tr("Unmountable Driver"));

    m_mainVboxLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVboxLayout->setSpacing(0);
    m_mainVboxLayout->addStretch();
    m_mainVboxLayout->addWidget(m_nodriverIcon, 0, Qt::AlignCenter);
    m_mainVboxLayout->addSpacing(8);
    m_mainVboxLayout->addWidget(m_nodriverText, 0, Qt::AlignCenter);
    m_mainVboxLayout->addStretch();

    this->setLayout(m_mainVboxLayout);
}

void NoDriverUninstallPage::initTheme() {
    QString theme = kdk::GsettingMonitor::getSystemTheme().toString();
    if(theme == "ukui-dark" || theme == "ukui-black"){
        m_nodriverIcon->setPixmap(QPixmap(":/nodevice_dark.svg").scaled(NoDriverIconSize));
    }else{
        m_nodriverIcon->setPixmap(QPixmap(":/nodevice.svg").scaled(NoDriverIconSize));
    }
}

void NoDriverUninstallPage::initConnect() {
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &NoDriverUninstallPage::initTheme);
}
