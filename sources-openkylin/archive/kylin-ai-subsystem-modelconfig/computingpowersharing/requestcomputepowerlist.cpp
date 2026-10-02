#include "requestcomputepowerlist.h"

// 请求算力列表(消费者列表)
// 获取消费者列表,需要启动server服务
RequestComputePowerList::RequestComputePowerList(QWidget* parent) : ComputePowerList(parent) {
    initComputingPowerFrame();
    m_mainVLayout->addWidget(m_computingPowerFrame);
    m_mainVLayout->addLayout(m_ipVLayout);

    initConnections();
}

RequestComputePowerList::~RequestComputePowerList() { m_updateListTimer->stop(); }

bool RequestComputePowerList::enableService() { return m_computePowerInterface.enableProviderService(); }

bool RequestComputePowerList::disableService() { return m_computePowerInterface.disableProviderService(); }

bool RequestComputePowerList::restartInferenceService() {
    return m_computePowerInterface.disableProviderServiceAndRestartInferenceService();
}

void RequestComputePowerList::initConnections() {
    connect(m_refreshBtn, &kdk::KBorderlessButton::clicked, this, &RequestComputePowerList::updateIpList);
    connect(m_loadingTimer, &QTimer::timeout, this, &RequestComputePowerList::updateLoadingImage);
    connect(m_updateListTimer, &QTimer::timeout, this, &RequestComputePowerList::autoRefreshList);
}

void RequestComputePowerList::autoRefreshList() {
    if (m_isRefreshing) {
        return;
    }
    m_isRefreshing = true;
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.requestConsumerList();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        KyWarning() << __func__ << "Failed to get IP list.";
    }
    QList<QString> newConsumerIpList = m_computePowerInterface.getConsumerList();
    if (newConsumerIpList.isEmpty()) {
        clearIpList();
        showNoDeviceAccessFrame();
        m_isRefreshing = false;
        return;
    }
    if (newConsumerIpList == m_consumerList) {
        m_isRefreshing = false;
        return;
    }

    hideNoDeviceAccessFrame();
    updateIpListUi(newConsumerIpList);
    m_consumerList = newConsumerIpList;
    m_isRefreshing = false;
}

void RequestComputePowerList::updateIpListUi(const QList<QString>& newIpList) {
    int oldSize = m_consumerList.size();
    int newSize = newIpList.size();
    int minSize = qMin(oldSize, newSize);

    for (int i = 0; i < minSize; ++i) {
        QWidget* widget = m_ipVLayout->itemAt(i) ? m_ipVLayout->itemAt(i)->widget() : nullptr;
        IpItemWidget* ipWidget = qobject_cast<IpItemWidget*>(widget);
        if (ipWidget) {
            updateIpWidget(ipWidget, newIpList.at(i));
        }
    }

    for (int i = oldSize - 1; i >= minSize; --i) {
        QLayoutItem* item = m_ipVLayout->takeAt(i);
        if (item) {
            if (QWidget* widget = item->widget()) {
                delete widget;
            }
            delete item;
        }
    }

    for (int i = minSize; i < newSize; ++i) {
        m_ipVLayout->addWidget(createIpWidget(newIpList.at(i)));
    }
}

void RequestComputePowerList::updateIpWidget(IpItemWidget* ipWidget, const QString& newIp) { ipWidget->setIp(newIp); }

void RequestComputePowerList::updateLoadingImage() {
    m_loadingIndex += 1;
    int index = m_loadingIndex % 8;
    m_loadingLabel->setPixmap(QIcon::fromTheme(QString("ukui-loading-%1-symbolic").arg(index)).pixmap(16, 16));
}

bool RequestComputePowerList::openService() {
    m_refreshBtn->hide();
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.openServerService();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    mc.setSharedModelName(
        mc.getCurrentModelName(kyai::config::model::AiCapability::Nlp, kyai::config::model::DeployType::Ondevice));
    return true;
}

bool RequestComputePowerList::closeService() {
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.closeServerService();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    mc.stopModelSharing(
        mc.getCurrentModelName(kyai::config::model::AiCapability::Nlp, kyai::config::model::DeployType::Ondevice));
    serviceCloseCompleted();
    return true;
}

bool RequestComputePowerList::resetService() {
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.resetServerToEndSide();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    mc.stopModelSharing(
        mc.getCurrentModelName(kyai::config::model::AiCapability::Nlp, kyai::config::model::DeployType::Ondevice));
    serviceCloseCompleted();
    return true;
}

void RequestComputePowerList::getIpList() {
    m_isRefreshing = true;
    if (!m_updateListTimer->isActive()) {
        m_updateListTimer->start(updateTimer);
    }

    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.requestConsumerList();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        KyWarning() << __func__ << "Failed to get IP list.";
    }

    updateListCompleted();
    QList<QString> consumerIpList = m_computePowerInterface.getConsumerList();
    if (consumerIpList.isEmpty()) {
        showNoDeviceAccessFrame();
        m_isRefreshing = false;
        return;
    }
    m_consumerList = consumerIpList;
    for (const QString& ipString : consumerIpList) {
        m_ipVLayout->addWidget(createIpWidget(ipString));
    }
    m_isRefreshing = false;
}

void RequestComputePowerList::clearIpList() {
    m_consumerList.clear();
    clearIpLayout();
}

void RequestComputePowerList::initComputingPowerFrame() {
    m_computingPowerFrame = createFrame();
    m_computingPowerFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_requestComputingPowerFrame");

    QHBoxLayout* frameMainHLayout = new QHBoxLayout();
    frameMainHLayout->setContentsMargins(16, 0, 16, 0);
    frameMainHLayout->setSpacing(0);
    m_computingPowerFrame->setLayout(frameMainHLayout);

    kdk::KLabel* requestComputePowerLabel = new kdk::KLabel(m_computingPowerFrame);
    requestComputePowerLabel->setText(tr("Request Computing Power List"));
    requestComputePowerLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_requestComputePowerLabel");

    m_refreshBtn = new kdk::KBorderlessButton(m_computingPowerFrame);
    m_refreshBtn->setIcon(QIcon::fromTheme("software-update-available-symbolic"));
    m_refreshBtn->setIconSize(QSize(16, 16));
    m_refreshBtn->setText(tr("Refresh"));
    m_refreshBtn->hide();
    m_refreshBtn->setAccessibleName("kylin-ai-subsystem-modelconfig_KBorderlessButton_computingPowerListRefreshBtn");

    m_loadingLabel = new QLabel(m_computingPowerFrame);
    m_loadingLabel->hide();
    m_loadingLabel->setPixmap(QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(16, 16));
    m_loadingLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_loadingLabel");

    m_loadingTimer = new QTimer(m_computingPowerFrame);
    m_loadingTimer->setObjectName("kylin-ai-subsystem-modelconfig_QTimer_loadingTimer");

    m_updateListTimer = new QTimer(m_computingPowerFrame);
    m_updateListTimer->setObjectName("kylin-ai-subsystem-modelconfig_QTimer_updateListTimer");

    frameMainHLayout->addWidget(requestComputePowerLabel);
    frameMainHLayout->addSpacing(8);
    frameMainHLayout->addWidget(m_loadingLabel);
    frameMainHLayout->addStretch();
    frameMainHLayout->addWidget(m_refreshBtn);
}

bool RequestComputePowerList::setVerificationCode(const QString& code) {
    if (!m_computePowerInterface.isServiceEnabled()) {
        KyWarning() << __func__ << "Service not opened or request for service status failed.";
    }

    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.setVerificationCode(code);
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    return true;
}

QString RequestComputePowerList::getVerificationCode() {
    if (!m_computePowerInterface.isServiceEnabled()) {
        KyWarning() << __func__ << "Service not opened or request for service status failed.";
    }

    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.getVerificationCode();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return "";
    }
    return m_computePowerInterface.getVerificationCodeValue();
}

UkccFrame* RequestComputePowerList::createIpWidget(const QString& ipString) {
    IpItemWidget* ipWidget = new IpItemWidget(ipString, true);
    return ipWidget;
}

void RequestComputePowerList::updateIpList() {
    m_refreshBtn->show();
    m_refreshBtn->setEnabled(false);
    m_loadingLabel->show();
    m_loadingTimer->start(100);

    clearIpList();
    hideNoDeviceAccessFrame();

    if (m_isRefreshing) {
        updateListCompleted();
        return;
    }
    getIpList();
}

void RequestComputePowerList::updateListCompleted() {
    m_loadingTimer->stop();
    m_loadingLabel->hide();
    m_refreshBtn->show();
    m_refreshBtn->setEnabled(true);
}

void RequestComputePowerList::serviceCloseCompleted() {
    clearIpList();
    hideNoDeviceAccessFrame();
    m_updateListTimer->stop();
}
