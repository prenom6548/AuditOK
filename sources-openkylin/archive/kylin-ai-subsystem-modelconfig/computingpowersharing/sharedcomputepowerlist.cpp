#include "sharedcomputepowerlist.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QRegExpValidator>
#include <QVBoxLayout>

#include "util.h"

// 共享算力列表(提供者列表)
// 获取提供者列表，需要启动endside服务
SharedComputePowerList::SharedComputePowerList(QWidget *parent) : ComputePowerList(parent) {
    initComputingPowerFrame();
    m_mainVLayout->addWidget(m_computingPowerFrame);
    m_mainVLayout->addLayout(m_ipVLayout);

    initConnections();
}

SharedComputePowerList::~SharedComputePowerList() { m_updateListTimer->stop(); }

bool SharedComputePowerList::enableService() { return m_computePowerInterface.enableConsumerService(); }

bool SharedComputePowerList::disableService() { return m_computePowerInterface.disableConsumerService(); }

bool SharedComputePowerList::restartInferenceService() {
    return m_computePowerInterface.enableProviderServiceAndRestartInferenceService();
}

void SharedComputePowerList::initConnections() {
    connect(m_refreshBtn, &kdk::KBorderlessButton::clicked, this, &SharedComputePowerList::updateIpList);
    connect(m_loadingTimer, &QTimer::timeout, this, &SharedComputePowerList::updateLoadingImage);
    connect(m_updateListTimer, &QTimer::timeout, this, &SharedComputePowerList::autoRefreshList);
}

void SharedComputePowerList::updateLoadingImage() {
    m_loadingIndex += 1;
    int index = m_loadingIndex % 8;
    m_loadingLabel->setPixmap(QIcon::fromTheme(QString("ukui-loading-%1-symbolic").arg(index)).pixmap(16, 16));
}

void SharedComputePowerList::autoRefreshList() {
    if (m_isRefreshing) {
        return;
    }
    m_isRefreshing = true;
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.requestProviderList();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        KyWarning() << __func__ << "Failed to get IP list.";
    }
    QList<QString> newConnectedProviderList = m_computePowerInterface.getConnectedProviderList();
    QList<QString> newDisconnectedProviderList = m_computePowerInterface.getDisconnectedProviderList();

    if (newConnectedProviderList.isEmpty() && newDisconnectedProviderList.isEmpty()) {
        requestConnectionToggled(false);
        clearIpList();
        showNoDeviceAccessFrame();
        m_isRefreshing = false;
        return;
    }
    if (newConnectedProviderList == m_connectedProviderList &&
        newDisconnectedProviderList == m_disconnectedProviderList) {
        m_isRefreshing = false;
        return;
    }
    hideNoDeviceAccessFrame();
    // 已连接列表为空，连接断开
    requestConnectionToggled(!newConnectedProviderList.empty());
    updateIpListUi(newConnectedProviderList, newDisconnectedProviderList);
    m_connectedProviderList = newConnectedProviderList;
    m_disconnectedProviderList = newDisconnectedProviderList;
    m_isRefreshing = false;
}

void SharedComputePowerList::updateIpListUi(const QList<QString> &newConnectedIpList,
                                            const QList<QString> &newDisconnectedIpList) {
    QList<QPair<QString, bool>> newIpList;
    for (const QString &ip : newConnectedIpList) {
        qDebug() << ip << newConnectedIpList << "connect";
        newIpList.append(qMakePair(ip, true));
    }
    for (const QString &ip : newDisconnectedIpList) {
        qDebug() << ip << newDisconnectedIpList << "disconnect";
        newIpList.append(qMakePair(ip, false));
    }
    int oldSize = m_connectedProviderList.size() + m_disconnectedProviderList.size();
    int newSize = newIpList.size();
    int minSize = qMin(oldSize, newSize);

    for (int i = 0; i < minSize; ++i) {
        QWidget *widget = m_ipVLayout->itemAt(i) ? m_ipVLayout->itemAt(i)->widget() : nullptr;
        IpItemWidget *ipWidget = qobject_cast<IpItemWidget *>(widget);
        if (ipWidget) {
            updateIpWidget(ipWidget, newIpList.at(i).first, newIpList.at(i).second);
        }
    }

    for (int i = oldSize - 1; i >= minSize; --i) {
        QLayoutItem *item = m_ipVLayout->takeAt(i);
        if (item) {
            if (QWidget *widget = item->widget()) {
                delete widget;
            }
            delete item;
        }
    }

    for (int i = minSize; i < newSize; ++i) {
        m_ipVLayout->addWidget(createIpWidget(newIpList.at(i).first, newIpList.at(i).second));
    }
}

void SharedComputePowerList::updateIpWidget(IpItemWidget *ipWidget, const QString &newIp, bool connectionState) {
    ipWidget->setIp(newIp);
    ipWidget->setConnected(connectionState);
    if (connectionState) {
        setIpWidgetSharedModelName(ipWidget, getSharedModelName());
    }
}

bool SharedComputePowerList::openService() {
    m_refreshBtn->hide();
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.openEndSideService();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    return true;
}

bool SharedComputePowerList::closeService() {
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.closeEndSideService();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    emit requestConnectionToggled(false);
    serviceCloseCompleted();
    return true;
}

bool SharedComputePowerList::resetService() {
    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.resetEndSideToServer();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        return false;
    }
    emit requestConnectionToggled(false);
    mc.setSharedModelName(
        mc.getCurrentModelName(kyai::config::model::AiCapability::Nlp, kyai::config::model::DeployType::Ondevice));
    serviceCloseCompleted();
    return true;
}

void SharedComputePowerList::getIpList() {
    m_isRefreshing = true;
    if (!m_updateListTimer->isActive()) {
        m_updateListTimer->start(updateTimer);
    }

    ComputePowerShare::ErrorCode errorCode = m_computePowerInterface.requestProviderList();
    if (errorCode != ComputePowerShare::ErrorCode::NoError) {
        showErrorMessageBoxByErrorCode(errorCode);
        KyWarning() << __func__ << "Failed to get IP list.";
    }

    updateListCompleted();
    QList<QString> connectedProviderList = m_computePowerInterface.getConnectedProviderList();
    QList<QString> disconnectedProviderList = m_computePowerInterface.getDisconnectedProviderList();
    qDebug() << __func__ << connectedProviderList << disconnectedProviderList;

    if (connectedProviderList.isEmpty() && disconnectedProviderList.isEmpty()) {
        emit requestConnectionToggled(false);
        showNoDeviceAccessFrame();
        m_isRefreshing = false;
        return;
    }
    m_connectedProviderList = connectedProviderList;
    m_disconnectedProviderList = disconnectedProviderList;

    // 已连接列表为空，连接断开
    requestConnectionToggled(!connectedProviderList.empty());
    for (const QString &connectedIpString : connectedProviderList) {
        m_ipVLayout->addWidget(createIpWidget(connectedIpString, true));
    }
    for (const QString &disconnectedIpString : disconnectedProviderList) {
        m_ipVLayout->addWidget(createIpWidget(disconnectedIpString, false));
    }
    m_isRefreshing = false;
}

void SharedComputePowerList::clearIpList() {
    m_connectedProviderList.clear();
    m_disconnectedProviderList.clear();
    clearIpLayout();
}

void SharedComputePowerList::initComputingPowerFrame() {
    m_computingPowerFrame = createFrame();
    m_computingPowerFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_sharedComputingPowerFrame");

    QHBoxLayout *frameMainHLayout = new QHBoxLayout();
    frameMainHLayout->setContentsMargins(16, 0, 16, 0);
    frameMainHLayout->setSpacing(0);
    m_computingPowerFrame->setLayout(frameMainHLayout);

    kdk::KLabel *sharedComputePowerLabel = new kdk::KLabel(m_computingPowerFrame);
    sharedComputePowerLabel->setText(tr("Shared Computing Power List"));
    sharedComputePowerLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_sharedComputePowerLabel");

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

    frameMainHLayout->addWidget(sharedComputePowerLabel);
    frameMainHLayout->addSpacing(8);
    frameMainHLayout->addWidget(m_loadingLabel);
    frameMainHLayout->addStretch();
    frameMainHLayout->addWidget(m_refreshBtn);
}

UkccFrame *SharedComputePowerList::createIpWidget(const QString &ipString, bool isConnected) {
    IpItemWidget *ipWidget = new IpItemWidget(ipString, isConnected);
    if (isConnected) {
        setIpWidgetSharedModelName(ipWidget, getSharedModelName());
    }
    connect(ipWidget, &IpItemWidget::requestConnection, this, [this](const QString &ip) {
        if (m_connectedProviderList.isEmpty()) {
            showVerificationDialog(ip);
        } else {
            showSwitchConnectionMessageBox(m_connectedProviderList.first(), ip);
        }
    });
    return ipWidget;
}

void SharedComputePowerList::setIpWidgetSharedModelName(IpItemWidget *ipWidget, const QString &modelName) {
    if (modelName.isEmpty()) {
        KyWarning() << __func__ << "The shared model name is empty.";
        return;
    }
    ipWidget->setSharedModelName(modelName);
}

QString SharedComputePowerList::getSharedModelName() { return mc.getSharedModelName().c_str(); }

void SharedComputePowerList::showVerificationDialog(const QString &newIp, const QString &oldIp) {
    m_verificationDialog->deleteLater();
    m_verificationDialog = nullptr;
    initVerificationDialog(newIp, oldIp);
    m_verificationPassword->setText("");
    m_verificationPassword->setEchoMode(QLineEdit::Password);
    m_verificationDialog->exec();
}

void SharedComputePowerList::initVerificationDialog(const QString &newIp, const QString &oldIp) {
    m_verificationDialog = new kdk::KDialog(this->parentWidget());
    m_verificationDialog->setFixedSize(376, 216);

    //请求算力验证
    QString title = tr("Request for computing power verification") + QString("(%1)").arg(newIp);
    m_verificationDialog->setWindowTitle(title);

    QVBoxLayout *frameMainVLayout = new QVBoxLayout();
    frameMainVLayout->setContentsMargins(24, 16, 24, 0);
    frameMainVLayout->setSpacing(0);
    m_verificationDialog->mainWidget()->setLayout(frameMainVLayout);

    //共享设备验证码（4 个数字）
    kdk::KLabel *verificationLabel = new kdk::KLabel();
    verificationLabel->setText(tr("Shared device verification code (4 digits)"));
    verificationLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_verificationLabel");

    m_verificationPassword = new kdk::KPasswordEdit(m_verificationDialog->mainWidget());
    m_verificationPassword->setFocusPolicy(Qt::WheelFocus);
    m_verificationPassword->setFixedSize(328, 40);
    // 只允许数字
    QRegExp regExp("\\d*");
    QRegExpValidator *validator = new QRegExpValidator(regExp, m_verificationPassword);
    m_verificationPassword->setValidator(validator);
    m_verificationPassword->setMaxLength(4);
    m_verificationPassword->setAccessibleName("kylin-ai-subsystem-modelconfig_KPasswordEdit_verificationPassword");

    m_cancelBtn = new QPushButton(tr("Cancel"));
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_cancelBtn");

    m_confirmBtn = new QPushButton(tr("Confirm"));
    m_confirmBtn->setFixedHeight(36);
    // 高亮按钮
    m_confirmBtn->setProperty("isImportant", true);
    m_confirmBtn->setEnabled(false);
    m_confirmBtn->setDefault(true);
    m_confirmBtn->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_confirmBtn");

    // 验证码错误，请重新输入
    m_errorLabel = new kdk::KLabel();
    m_errorLabel->setText(tr("Verification code error, please re-enter"));
    m_errorLabel->hide();
    setKLabelFontColor(m_errorLabel, "red");
    QFont font;
    font.setPointSize(verificationLabel->font().pointSize() - 2);
    m_errorLabel->setFont(font);
    m_errorLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_errorLabel");

    connect(m_verificationPassword, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_errorLabel->hide();
        m_verificationDialog->setFixedHeight(216);
        m_confirmBtn->setEnabled(text.length() == 4);
    });
    connect(m_cancelBtn, &QPushButton::clicked, this, [this]() { m_verificationDialog->close(); });
    connect(m_confirmBtn, &QPushButton::clicked, this, [newIp, oldIp, this]() {
        ComputePowerShare::ErrorCode errorCode =
            m_computePowerInterface.connectToProvider(newIp, m_verificationPassword->text(), oldIp);
        if (errorCode == ComputePowerShare::ErrorCode::VerificationCodeError) {
            m_errorLabel->show();
            m_verificationDialog->setFixedHeight(240);
        } else if (errorCode == ComputePowerShare::ErrorCode::NoError) {
            m_verificationDialog->close();
            updateIpList();
        } else {
            showErrorMessageBoxByErrorCode(errorCode);
        }
    });

    QHBoxLayout *btnHLayout = new QHBoxLayout();
    btnHLayout->setContentsMargins(0, 0, 0, 0);
    btnHLayout->setSpacing(0);
    btnHLayout->addStretch();
    btnHLayout->addWidget(m_cancelBtn);
    btnHLayout->addSpacing(8);
    btnHLayout->addWidget(m_confirmBtn);

    frameMainVLayout->addWidget(verificationLabel);
    frameMainVLayout->addSpacing(6);
    frameMainVLayout->addWidget(m_verificationPassword);
    frameMainVLayout->addSpacing(2);
    frameMainVLayout->addWidget(m_errorLabel);
    frameMainVLayout->addSpacing(24);
    frameMainVLayout->addLayout(btnHLayout);
    frameMainVLayout->addStretch();
}

void SharedComputePowerList::updateIpList() {
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

void SharedComputePowerList::showSwitchConnectionMessageBox(const QString &currentIp, const QString &newIp) {
    QMessageBox msgBox(this->parentWidget());
    msgBox.setIcon(QMessageBox::Question);
    QString message(
        tr("Do you want to disconnect the current computing power sharing device %1 and switch to the new device %2?")
            .arg(currentIp)
            .arg(newIp));
    msgBox.setText(message);

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *sureButton = msgBox.addButton(QObject::tr("Ok"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    int result = msgBox.exec();
    if (result == QMessageBox::Accepted) {
        showVerificationDialog(newIp, currentIp);
    }
}

void SharedComputePowerList::updateListCompleted() {
    m_loadingTimer->stop();
    m_loadingLabel->hide();
    m_refreshBtn->show();
    m_refreshBtn->setEnabled(true);
}

void SharedComputePowerList::serviceCloseCompleted() {
    clearIpList();
    hideNoDeviceAccessFrame();
    m_updateListTimer->stop();
}
