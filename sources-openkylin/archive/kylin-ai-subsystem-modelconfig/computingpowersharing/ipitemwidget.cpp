#include "ipitemwidget.h"

#include "util.h"

IpItemWidget::IpItemWidget(const QString &ip, bool isConnected, QWidget *parent)
    : UkccFrame(parent), m_ip(ip), m_isConnected(isConnected) {
    this->setFixedSize(QSize(496, 60));

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 0, 16, 0);
    m_mainLayout->setSpacing(0);

    // Icon
    QPixmap pixmap = QIcon::fromTheme("computer-symbolic").pixmap(QSize(16, 16));
    m_iconLabel = new QLabel(this);
    m_iconLabel->setPixmap(pixmap);
    m_iconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_iconlabel");

    // IP Label
    m_ipLabel = new kdk::KLabel(this);
    m_ipLabel->setText(m_ip);
    m_ipLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_ipLabel");

    QHBoxLayout *ipHLayout = new QHBoxLayout();
    ipHLayout->setContentsMargins(0, 0, 0, 0);
    ipHLayout->setSpacing(16);
    ipHLayout->addWidget(m_iconLabel);
    ipHLayout->addWidget(m_ipLabel);

    m_separatorLabel = new kdk::KLabel();
    m_separatorLabel->setText("|");
    m_separatorLabel->hide();
    m_separatorLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_separatorLabel");

    m_modelLabel = new kdk::KLabel();
    m_modelLabel->setText(tr("Model: "));
    m_modelLabel->hide();
    m_modelLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_modelLabel");

    m_shareModelNameLabel = new kdk::KLabel();
    setKLabelFontColor(m_shareModelNameLabel, "#808080");
    m_shareModelNameLabel->hide();
    m_shareModelNameLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_shareModelNameLabel");

    QHBoxLayout *modelHLayout = new QHBoxLayout();
    modelHLayout->setContentsMargins(0, 0, 0, 0);
    modelHLayout->setSpacing(0);
    modelHLayout->addSpacing(2);
    modelHLayout->addWidget(m_separatorLabel);
    modelHLayout->addSpacing(2);
    modelHLayout->addWidget(m_modelLabel);
    modelHLayout->addWidget(m_shareModelNameLabel);
    modelHLayout->addSpacing(16);

    // Connected label
    m_connectLabel = new kdk::KLabel(this);
    m_connectLabel->setText(tr("Connected"));
    m_connectLabel->setVisible(m_isConnected);
    m_connectLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_connectLabel");

    // Connect button
    m_connectButton = new kdk::KBorderlessButton(this);
    m_connectButton->setText(tr("Request connection"));
    m_connectButton->setVisible(!m_isConnected);
    m_connectButton->setAccessibleName("kylin-ai-subsystem-modelconfig_KBorderlessButton_button");
    connect(m_connectButton, &kdk::KBorderlessButton::clicked, this, &IpItemWidget::onRequestConnectionClicked);

    m_mainLayout->addLayout(ipHLayout);
    m_mainLayout->addLayout(modelHLayout);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_connectButton);
    m_mainLayout->addWidget(m_connectLabel);
}

void IpItemWidget::setIp(const QString &ip) {
    if (m_ip != ip) {
        m_ip = ip;
    }
    m_ip = ip;
    m_ipLabel->setText(ip);
}

void IpItemWidget::setConnected(bool connected) {
    if (m_isConnected == connected) {
        return;
    }
    m_isConnected = connected;
    m_connectLabel->setVisible(connected);
    m_connectButton->setVisible(!connected);
    if (!connected) {
        m_separatorLabel->hide();
        m_modelLabel->hide();
        m_shareModelNameLabel->hide();
    }
}

void IpItemWidget::setSharedModelName(const QString &modelName) {
    m_shareModelNameLabel->setText(modelName);

    m_separatorLabel->show();
    m_modelLabel->show();
    m_shareModelNameLabel->show();
}

void IpItemWidget::onRequestConnectionClicked() { emit requestConnection(m_ip); }
