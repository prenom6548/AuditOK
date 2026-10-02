#include "computepowersharedialog.h"

#include <aiconfig/modelconfig.h>
#include <ukui-log4qt.h>

#include <QIcon>
#include <QProcess>

#include "util.h"

ComputePowerShareDialog::ComputePowerShareDialog(QWidget *parent) : kdk::KDialog(parent) {
    this->setFixedSize(544, 610);
    this->setWindowTitle(tr("Computing Power Sharing"));
    this->setWindowIcon(QIcon::fromTheme("ukui-control-center"));

    m_mainLayout = mainLayout();
    dialogCloseBtn = closeButton();

    QWidget *contentWidget = new QWidget;
    contentWidget->setContentsMargins(24, 0, 24, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFixedHeight(546);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);  // 无边框
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 不显示水平滚动条
    m_scrollArea->setWidget(contentWidget);

    m_mainLayout->addWidget(m_scrollArea);
    m_mainLayout->addStretch();

    m_mainVLayout = new QVBoxLayout();
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->setSpacing(8);
    contentWidget->setLayout(m_mainVLayout);

    m_computePowerListVLayout = new QVBoxLayout();
    m_computePowerListVLayout->setContentsMargins(0, 0, 0, 0);
    m_computePowerListVLayout->setSpacing(0);

    initComponent();
    m_roleMainVLayout = new QVBoxLayout();
    m_roleMainVLayout->setSpacing(1);
    m_roleMainVLayout->addWidget(m_roleSelectionFrame);
    m_roleMainVLayout->addWidget(m_verificationCodeFrame);

    m_mainVLayout->addWidget(m_mainTitleLabel);
    m_mainVLayout->addWidget(m_computePowerSharingFrame);
    m_mainVLayout->addLayout(m_roleMainVLayout);
    m_mainVLayout->addWidget(m_serviceOpengingWidget);
    m_mainVLayout->addLayout(m_computePowerListVLayout);
    m_mainVLayout->addStretch();

    initUIState();
    initConnections();
}

void ComputePowerShareDialog::initComponent() {
    initMainTitleLabel();
    initEnableComputePowerShareWidget();
    initRoleSelectionWidget();
    initVerificationCodeWidget();
    initServiceOpeningTipWidget();
    initSharedComputePowerList();
    initRequestComputePowerList();
}

void ComputePowerShareDialog::initConnections() {
    connect(m_toggleEnableButton, &kdk::KSwitchButton::stateChanged, this,
            &ComputePowerShareDialog::onToggleEnableButtonStatusChanged);
    connect(m_roleSelectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxCurrentIndexChanged(int)));
    connect(dialogCloseBtn, &QPushButton::clicked, [this]() {
        if (m_requestComputePowerList) {
            m_requestComputePowerList->deleteLater();
            m_requestComputePowerList = nullptr;
        }
        if (m_sharedComputePowerList) {
            m_sharedComputePowerList->deleteLater();
            m_sharedComputePowerList = nullptr;
        }
    });
    connect(m_serviceOpenLoadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingImageIndex += 1;
        int index = m_loadingImageIndex % 8;
        m_serviceOpenLoadingLabel->setPixmap(
            QIcon::fromTheme(QString("ukui-loading-%1-symbolic").arg(index)).pixmap(16, 16));
    });
    connect(m_sharedComputePowerList, &RequestComputePowerList::requestConnectionToggled, this,
            &ComputePowerShareDialog::requestConnectionToggled);
}

void ComputePowerShareDialog::initUIState() {
    if (!m_configManager.getEnableComputePowerSharing()) {
        return;
    }
    m_toggleEnableButton->setChecked(true);
    m_roleSelectionFrame->show();

    QString role = m_configManager.getComputePowerSharingRole();
    m_enableSharedComputePower = m_configManager.getEnableSharedComputePower();
    m_enableRequestComputePower = m_configManager.getEnableRequestComputePower();
    if (role == m_pleaseSelectKey) {
        m_curRole = RoleSelection::PleaseSelect;
        setComboBoxIndex(m_curRole);
    } else if (role == m_sharedComputingPowerKey) {
        QTimer::singleShot(10, this, [this]() {
            m_curRole = RoleSelection::SharedComputingPower;
            setComboBoxIndex(m_curRole);
            showSharedComputePowerUI();
        });
    } else if (role == m_requestComputingPowerKey) {
        QTimer::singleShot(10, this, [this]() {
            m_curRole = RoleSelection::RequestComputingPower;
            setComboBoxIndex(m_curRole);
            showRequestComputePowerUI();
        });
    }
}

void ComputePowerShareDialog::showRequestComputePowerUI() {
    if (!m_enableRequestComputePower) {
        return;
    }
    showRequestComputePowerOpenedUI();
}

void ComputePowerShareDialog::showRequestComputePowerOpenedUI() {
    m_sharedComputePowerList->show();

    setComponentsEnabled(false);
    setLocalIpLabelText(m_sharedComputePowerList->getLocalIp());
    m_sharedComputePowerList->updateIpList();
    setComponentsEnabled(true);
}

void ComputePowerShareDialog::setComponentsEnabled(bool enable) {
    m_roleSelectionComboBox->setEnabled(enable);
    m_verificationCodePassword->setEnabled(enable);
}

void ComputePowerShareDialog::showSharedComputePowerUI() {
    showVerificationCodeFrame();

    if (!m_enableSharedComputePower) {
        QString savedCode = m_configManager.getVerificationCode();
        if (savedCode.isEmpty()) {
            return;
        }
        m_verificationCodePassword->setText(savedCode);
        if (!isReboot()) {
            showRebootTipMessageBox();
            return;
        }

        openSharedServiceAndSettingCode(savedCode);
        return;
    }
    showSharedComputePowerOpenedUI();
}

bool ComputePowerShareDialog::openSharedServiceAndSettingCode(const QString &code) {
    setComponentsEnabled(false);
    showServiceOpeningTipWidget();
    if (!m_requestComputePowerList->openService()) {
        hideServiceOpeningTipWidget();

        while (1) {
            if (m_requestComputePowerList->disableService()) {
                break;
            }
        }
        resetToPleaseSelect();
        createTmpFile();

        setComponentsEnabled(true);
        return false;
    }
    hideServiceOpeningTipWidget();

    m_enableSharedComputePower = true;
    m_configManager.setEnableSharedComputePower(true);
    checkLocalModelEnable();

    setLocalIpLabelText(m_requestComputePowerList->getLocalIp());
    if (m_requestComputePowerList->setVerificationCode(code)) {
        m_requestComputePowerList->show();
        m_requestComputePowerList->updateIpList();
    }
    setComponentsEnabled(true);
    return true;
}

void ComputePowerShareDialog::showSharedComputePowerOpenedUI() {
    checkLocalModelEnable();
    setComponentsEnabled(false);
    setLocalIpLabelText(m_requestComputePowerList->getLocalIp());
    // 如果设置过验证码，更新ip列表
    if (isSetVerificationCode()) {
        m_requestComputePowerList->show();
        m_requestComputePowerList->updateIpList();
    }
    setComponentsEnabled(true);

    kyai::config::model::ModelConfig mc;
    mc.setSharedModelName(
        mc.getCurrentModelName(kyai::config::model::AiCapability::Nlp, kyai::config::model::DeployType::Ondevice));
}

void ComputePowerShareDialog::initMainTitleLabel() {
    // 算力共享
    m_mainTitleLabel = new kdk::KLabel(this);
    m_mainTitleLabel->setText(tr("Computing Power Sharing"));
    m_mainTitleLabel->setContentsMargins(16, 8, 0, 8);
    m_mainTitleLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_mainTitleLabel");
}

void ComputePowerShareDialog::initEnableComputePowerShareWidget() {
    m_computePowerSharingFrame = createFrame();
    m_computePowerSharingFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_computePowerSharingFrame");

    QHBoxLayout *frameMainHLayout = new QHBoxLayout();
    frameMainHLayout->setContentsMargins(16, 0, 16, 0);
    frameMainHLayout->setSpacing(16);
    m_computePowerSharingFrame->setLayout(frameMainHLayout);

    // 启用算力共享
    m_enableTitleLabel = new kdk::KLabel(m_computePowerSharingFrame);
    m_enableTitleLabel->setText(tr("Enable Computing Power Sharing"));
    m_enableTitleLabel->setMinimumWidth(398);
    m_enableTitleLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_enableTitleLabel");

    // 多设备分配和整合计算资源，实现无缝的高性能计算体验
    m_detailLabel = new kdk::KLabel(m_computePowerSharingFrame);
    m_detailLabel->setText(
        tr("Multi device allocation and integration of computing resources to achieve seamless high-performance "
           "computing experience"));
    m_detailLabel->setMinimumWidth(398);
    setKLabelFontColor(m_detailLabel, "#808080");
    QFont detailLabelFont;
    detailLabelFont.setPointSize(m_enableTitleLabel->font().pointSize() - 2);
    m_detailLabel->setFont(detailLabelFont);
    m_detailLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_detailLabel");

    QVBoxLayout *frameLableVLayout = new QVBoxLayout();
    frameLableVLayout->setSpacing(0);
    frameLableVLayout->setContentsMargins(0, 0, 0, 0);
    frameLableVLayout->addStretch();
    frameLableVLayout->addWidget(m_enableTitleLabel);
    frameLableVLayout->addWidget(m_detailLabel);
    frameLableVLayout->addStretch();

    //算力共享开关
    m_toggleEnableButton = new kdk::KSwitchButton;
    m_toggleEnableButton->setObjectName("serviceStatusSwitch");
    m_toggleEnableButton->setFocusPolicy(Qt::NoFocus);
    m_toggleEnableButton->setAccessibleName("kylin-ai-subsystem-modelconfig_KSwitchButton_toggleEnableButton");

    frameMainHLayout->addLayout(frameLableVLayout);
    frameMainHLayout->addStretch();
    frameMainHLayout->addWidget(m_toggleEnableButton);
}

void ComputePowerShareDialog::initRoleSelectionWidget() {
    m_roleSelectionFrame = createFrame();
    m_roleSelectionFrame->hide();
    m_roleSelectionFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_roleSelectionFrame");

    QHBoxLayout *frameMainHLayout = new QHBoxLayout();
    frameMainHLayout->setSpacing(16);
    frameMainHLayout->setContentsMargins(16, 0, 16, 0);
    m_roleSelectionFrame->setLayout(frameMainHLayout);

    // 身份选择
    m_roleSelectionLabel = new kdk::KLabel(m_roleSelectionFrame);
    m_roleSelectionLabel->setText(tr("Role Selection"));
    m_roleSelectionLabel->setMinimumWidth(268);
    m_roleSelectionLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_roleSelectionLabel");

    // 本机IP
    m_localIpLabel = new kdk::KLabel(m_roleSelectionFrame);
    m_localIpLabel->setText(tr("Local IP: "));
    setKLabelFontColor(m_localIpLabel, "#808080");
    m_localIpLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_localIpLabel");

    // IP
    m_localIpValueLabel = new kdk::KLabel(m_roleSelectionFrame);
    m_localIpValueLabel->setText(tr("Service not enabled"));
    setKLabelFontColor(m_localIpValueLabel, "#808080");
    m_localIpValueLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_localIpValueLabel");

    QFont ipLabelFont;
    ipLabelFont.setPointSize(m_roleSelectionLabel->font().pointSize() - 2);
    m_localIpLabel->setFont(ipLabelFont);
    m_localIpValueLabel->setFont(ipLabelFont);

    m_tipButton = new QPushButton(m_roleSelectionFrame);
    m_tipButton->setIcon(QIcon::fromTheme("help-about-symbolic"));
    m_tipButton->setFixedSize(16, 16);
    m_tipButton->setEnabled(false);
    m_tipButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    m_tipButton->setToolTip(
        tr("After changing the local IP address, it is necessary to re enable computing power sharing, otherwise it "
           "may not function properly"));
    m_tipButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_tipButton");

    m_ipHLayout = new QHBoxLayout();
    m_ipHLayout->addWidget(m_localIpLabel);
    m_ipHLayout->addWidget(m_localIpValueLabel);
    m_ipHLayout->addSpacing(5);
    m_ipHLayout->addWidget(m_tipButton);
    m_ipHLayout->addStretch();

    m_roleSelectionComboBox = new QComboBox(m_roleSelectionFrame);
    m_roleSelectionComboBox->setFixedSize(180, 36);
    m_roleSelectionComboBox->addItem(tr("Please select"));
    m_roleSelectionComboBox->insertSeparator(1);
    m_roleSelectionComboBox->addItem(tr("Shared computing power"));
    m_roleSelectionComboBox->addItem(tr("Request computing power"));
    m_roleSelectionComboBox->setAccessibleName("kylin-ai-subsystem-modelconfig_QComboBox_roleSelectionComboBox");

    QVBoxLayout *frameLableVLayout = new QVBoxLayout();
    frameLableVLayout->setSpacing(0);
    frameLableVLayout->setContentsMargins(0, 0, 0, 0);
    frameLableVLayout->addStretch();
    frameLableVLayout->addWidget(m_roleSelectionLabel);
    frameLableVLayout->addLayout(m_ipHLayout);
    frameLableVLayout->addStretch();

    frameMainHLayout->addLayout(frameLableVLayout);
    frameMainHLayout->addStretch();
    frameMainHLayout->addWidget(m_roleSelectionComboBox);
}

void ComputePowerShareDialog::initVerificationCodeWidget() {
    m_verificationCodeFrame = createFrame();
    m_verificationCodeFrame->hide();
    m_verificationCodeFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_m_verificationCodeFrame");

    QHBoxLayout *frameMainHLayout = new QHBoxLayout();
    frameMainHLayout->setContentsMargins(16, 0, 16, 0);
    frameMainHLayout->setSpacing(16);
    m_verificationCodeFrame->setLayout(frameMainHLayout);

    // 设置验证码
    m_setVerificationCodeLabel = new kdk::KLabel(m_verificationCodeFrame);
    m_setVerificationCodeLabel->setText(tr("Set Device Verification Code (4 digits)"));
    m_setVerificationCodeLabel->setMinimumWidth(268);
    m_setVerificationCodeLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_setVerificationCodeLabel");

    // 他人在使用你的共享算力时需要验证
    m_verificationCodeDetailLabel = new kdk::KLabel(m_verificationCodeFrame);
    m_verificationCodeDetailLabel->setText(tr("Others need to verify when using your shared computing power."));
    m_verificationCodeDetailLabel->setMinimumWidth(268);
    setKLabelFontColor(m_verificationCodeDetailLabel, "#808080");
    QFont detailLabelFont;
    detailLabelFont.setPointSize(m_setVerificationCodeLabel->font().pointSize() - 2);
    m_verificationCodeDetailLabel->setFont(detailLabelFont);
    m_verificationCodeDetailLabel->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_QLabel_verificationCodeDetailLabel");

    QVBoxLayout *frameLableVLayout = new QVBoxLayout();
    frameLableVLayout->setSpacing(0);
    frameLableVLayout->setContentsMargins(0, 0, 0, 0);
    frameLableVLayout->addStretch();
    frameLableVLayout->addWidget(m_setVerificationCodeLabel);
    frameLableVLayout->addWidget(m_verificationCodeDetailLabel);
    frameLableVLayout->addStretch();

    // 验证码输入框
    m_verificationCodePassword = new kdk::KPasswordEdit(m_verificationCodeFrame);
    connect(m_verificationCodePassword, &QLineEdit::editingFinished, [this]() {
        QString code = m_verificationCodePassword->text();
        if (code.length() == 4) {
            resetVerificationCode(code);
        }
    });

    m_verificationCodePassword->setFocusPolicy(Qt::WheelFocus);
    m_verificationCodePassword->setFixedSize(180, 36);

    // 只允许数字
    QRegExp regExp("\\d*");
    QRegExpValidator *validator = new QRegExpValidator(regExp, m_verificationCodePassword);
    m_verificationCodePassword->setValidator(validator);
    m_verificationCodePassword->setMaxLength(4);

    //输入验证码
    QString placeholderText = tr("Enter the verification code");
    m_verificationCodePassword->setPlaceholderText(placeholderText);
    m_verificationCodePassword->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_kdk::KPasswordEdit_verificationCodePassword");

    frameMainHLayout->addLayout(frameLableVLayout);
    frameMainHLayout->addStretch();
    frameMainHLayout->addWidget(m_verificationCodePassword);
}

void ComputePowerShareDialog::resetVerificationCode(const QString &code) {
    QString savedCode = m_configManager.getVerificationCode();
    if (savedCode == code) {
        return;
    }
    if (!savedCode.isEmpty() && !confirmResetVerificationCode()) {
        m_verificationCodePassword->setText(savedCode);
        return;
    }
    m_configManager.setVerificationCode(code);
    if (!m_enableSharedComputePower) {
        while (1) {
            if (m_requestComputePowerList->enableService()) {
                break;
            }
        }

        createTmpFile();
        showRebootTipMessageBox();
        return;
    }
    setSharedComputePowerVerificationCode(code);
}

bool ComputePowerShareDialog::confirmResetVerificationCode() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(
        tr("Replacing the device verification code will interrupt the current computing power connection. Are you sure "
           "about the replacement?"));

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *changeButton = msgBox.addButton(QObject::tr("Change"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(changeButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

void ComputePowerShareDialog::initServiceOpeningTipWidget() {
    m_serviceOpengingWidget = new QWidget(this);
    m_serviceOpengingWidget->setFixedSize(QSize(496, 60));
    m_serviceOpengingWidget->hide();

    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setAlignment(Qt::AlignHCenter);
    mainHLayout->setSpacing(8);
    m_serviceOpengingWidget->setLayout(mainHLayout);

    m_serviceOpenLoadingLabel = new QLabel();
    m_serviceOpenLoadingLabel->setPixmap(QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(16, 16));
    m_serviceOpenLoadingLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_serviceOpenLoadingLabel");

    m_serviceOpeningLabel = new kdk::KLabel();
    m_serviceOpeningLabel->setText(tr("Service is starting, please wait a moment..."));
    m_serviceOpeningLabel->setMaximumWidth(470);
    m_serviceOpeningLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_serviceLoadingLabel");

    m_serviceOpenLoadingTimer = new QTimer(m_serviceOpengingWidget);
    m_serviceOpenLoadingTimer->setObjectName("kylin-ai-subsystem-modelconfig_QTimer_serviceOpenLoadingTimer");

    mainHLayout->addWidget(m_serviceOpenLoadingLabel);
    mainHLayout->addWidget(m_serviceOpeningLabel);
}

void ComputePowerShareDialog::showServiceOpeningTipWidget() {
    m_serviceOpengingWidget->show();
    m_serviceOpenLoadingTimer->start(100);
}

void ComputePowerShareDialog::hideServiceOpeningTipWidget() {
    m_serviceOpengingWidget->hide();
    m_serviceOpenLoadingTimer->stop();
}

void ComputePowerShareDialog::setSharedComputePowerVerificationCode(const QString &code) {
    if (m_verificationCodeStr == code) {
        return;
    }

    setComponentsEnabled(false);
    bool success = m_requestComputePowerList->setVerificationCode(code);
    if (!success) {
        setComponentsEnabled(true);
        return;
    }
    m_verificationCodeStr = code;

    m_requestComputePowerList->show();
    m_requestComputePowerList->updateIpList();
    setComponentsEnabled(true);
}

void ComputePowerShareDialog::setComboBoxIndex(RoleSelection roleSelection) {
    m_roleSelectionComboBox->blockSignals(true);
    m_roleSelectionComboBox->setCurrentIndex(roleSelection);
    m_roleSelectionComboBox->blockSignals(false);
}

void ComputePowerShareDialog::createTmpFile() {
    QFile file(m_tmpFilePath);
    if (file.exists()) {
        return;
    }
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
    } else {
        qWarning() << "unable to create file:" << m_tmpFilePath;
    }
}

bool ComputePowerShareDialog::isReboot() { return !QFile::exists(m_tmpFilePath); }

void ComputePowerShareDialog::initRequestComputePowerList() {
    if (m_requestComputePowerList) {
        return;
    }
    m_requestComputePowerList = new RequestComputePowerList(this);
    m_requestComputePowerList->hide();
    m_computePowerListVLayout->addWidget(m_requestComputePowerList);
}

void ComputePowerShareDialog::initSharedComputePowerList() {
    if (m_sharedComputePowerList) {
        return;
    }
    m_sharedComputePowerList = new SharedComputePowerList(this);
    m_sharedComputePowerList->hide();
    m_computePowerListVLayout->addWidget(m_sharedComputePowerList);
}

bool ComputePowerShareDialog::isSetVerificationCode() {
    if (m_computePowerInterface.getVerificationCode() != ComputePowerShare::ErrorCode::NoError) {
        return false;
    }
    QString verificationCodeStr = m_computePowerInterface.getVerificationCodeValue();
    if (verificationCodeStr.isEmpty()) {
        return false;
    }
    m_verificationCodePassword->setText(verificationCodeStr);
    m_verificationCodeStr = verificationCodeStr;
    return true;
}

void ComputePowerShareDialog::onToggleEnableButtonStatusChanged(bool isEnable) {
    if (isEnable) {
        enableComputePowerSharing();
    } else {
        disableComputePowerSharing();
    }
}

void ComputePowerShareDialog::enableComputePowerSharing() {
    m_roleSelectionFrame->show();
    m_configManager.setEnableComputePowerSharing(true);
}

void ComputePowerShareDialog::disableComputePowerSharing() {
    setToggleEnableButtonState(true);
    if (m_serviceOpengingWidget->isVisible()) {
        showServiceOpeningTipMessageBox();
        return;
    }

    // 服务都未启用，当前的选项可能是共享算力
    QString savedCode = m_configManager.getVerificationCode();
    if (!m_enableSharedComputePower && m_curRole == RoleSelection::SharedComputingPower && !savedCode.isEmpty()) {
        if (!m_requestComputePowerList->disableService()) {
            return;
        }
    }
    if (m_enableRequestComputePower && !closeRequestComputePowerService()) {
        return;
    }
    if (m_enableSharedComputePower && !closeSharedComputePowerService()) {
        return;
    }

    m_verificationCodeFrame->hide();
    // 关闭服务需要将roleSelectionFrame隐藏
    m_roleSelectionFrame->hide();
    setComboBoxIndex(RoleSelection::PleaseSelect);
    m_curRole = RoleSelection::PleaseSelect;
    m_configManager.setComputePowerSharingRole(roleKeyMap.value(RoleSelection::PleaseSelect));
    // 服务关闭成功再将按钮关闭
    setToggleEnableButtonState(false);
    m_configManager.setEnableComputePowerSharing(false);
    m_configManager.setLocalIp("");
}

bool ComputePowerShareDialog::closeRequestComputePowerService() {
    if (!confirmCloseRequestComputePowerService()) {
        return false;
    }

    setComponentsEnabled(false);
    if (!closeService(m_sharedComputePowerList)) {
        setComponentsEnabled(true);
        return false;
    }

    m_roleSelectionFrame->hide();
    m_sharedComputePowerList->hide();

    m_enableRequestComputePower = false;
    m_configManager.setEnableRequestComputePower(false);

    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::confirmCloseRequestComputePowerService() {
    QString closeTip(
        tr("Closing the computing power sharing will disconnect the current connection. Are you sure you want to close "
           "it?"));
    if (!showCloseServiceTipMessageBox(closeTip)) {
        return false;
    }
    return true;
}

bool ComputePowerShareDialog::closeSharedComputePowerService() {
    if (!confirmCloseSharedComputePowerService()) {
        return false;
    }

    setComponentsEnabled(false);
    if (!closeService(m_requestComputePowerList)) {
        setComponentsEnabled(true);
        return false;
    }

    m_verificationCodeFrame->hide();
    m_roleSelectionFrame->hide();
    m_requestComputePowerList->hide();

    m_enableSharedComputePower = false;
    m_configManager.setEnableSharedComputePower(false);

    // 再次打开服务要执行脚本，提示重启
    createTmpFile();

    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::confirmCloseSharedComputePowerService() {
    QString closeTip(
        tr("Closing the computing power sharing will clear the connection configuration for this sharing. Are you "
           "sure you want to close it?"));
    if (!showCloseServiceTipMessageBox(closeTip)) {
        return false;
    }
    return true;
}

bool ComputePowerShareDialog::closeService(ComputePowerList *computePowerList) {
    if (!computePowerList->closeService()) {
        return false;
    }
    while (1) {
        if (computePowerList->disableService()) {
            break;
        }
    }
    return true;
}

void ComputePowerShareDialog::resetToPleaseSelect() {
    m_curRole = RoleSelection::PleaseSelect;
    m_verificationCodeFrame->hide();
    setComboBoxIndex(RoleSelection::PleaseSelect);
    m_configManager.setComputePowerSharingRole(roleKeyMap.value(RoleSelection::PleaseSelect));
}

void ComputePowerShareDialog::setToggleEnableButtonState(bool enable) {
    m_toggleEnableButton->blockSignals(true);
    m_toggleEnableButton->setChecked(enable);
    m_toggleEnableButton->blockSignals(false);
}

void ComputePowerShareDialog::onComboBoxCurrentIndexChanged(int currentIndex) {
    setComboBoxIndex(m_curRole);
    bool success = false;
    if (currentIndex == RoleSelection::PleaseSelect) {
        success = switchToPleaseSelect();
    } else if (currentIndex == RoleSelection::RequestComputingPower) {
        success = switchToRequestComputingPower();
    } else if (currentIndex == RoleSelection::SharedComputingPower) {
        success = switchToSharedComputingPower();
    }

    if (success) {
        m_curRole = (RoleSelection)currentIndex;
        setComboBoxIndex(m_curRole);
        m_configManager.setComputePowerSharingRole(roleKeyMap.value(m_curRole));
        return;
    }
}

bool ComputePowerShareDialog::switchToPleaseSelect() {
    QString savedCode = m_configManager.getVerificationCode();

    // 服务都未启用，当前的选项可能是共享算力
    // 未启用的共享算力切换到请选择，需要隐藏验证框frame
    // 共享算力验证码设置后，就会执行打开服务脚本，切换到请选择需要执行脚本关闭服务
    if (!m_enableSharedComputePower && m_curRole == RoleSelection::SharedComputingPower && !savedCode.isEmpty()) {
        if (!m_requestComputePowerList->disableService()) {
            return false;
        }
    }
    if (m_enableSharedComputePower && !switchToPleaseSelectFromShared()) {
        return false;
    }
    if (m_enableRequestComputePower && !switchToPleaseSelectFromRequest()) {
        return false;
    }

    m_verificationCodeFrame->hide();
    return true;
}

bool ComputePowerShareDialog::switchToPleaseSelectFromShared() {
    if (!confirmSwitchRole()) {
        return false;
    }
    setComponentsEnabled(false);
    if (!m_requestComputePowerList->closeService()) {
        setComponentsEnabled(true);
        return false;
    }

    m_verificationCodeFrame->hide();
    m_requestComputePowerList->hide();
    // 服务已关闭，必须执行脚本的关闭服务
    while (1) {
        if (m_requestComputePowerList->disableService()) {
            break;
        }
    }

    m_enableSharedComputePower = false;
    m_configManager.setEnableSharedComputePower(false);

    // 再次打开服务要执行脚本，提示重启
    createTmpFile();
    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::switchToPleaseSelectFromRequest() {
    if (!confirmSwitchRole()) {
        return false;
    }
    setComponentsEnabled(false);
    if (!m_sharedComputePowerList->closeService()) {
        setComponentsEnabled(true);
        return false;
    }
    m_sharedComputePowerList->hide();

    // 服务已关闭，必须执行脚本的关闭服务
    while (1) {
        if (m_sharedComputePowerList->disableService()) {
            break;
        }
    }

    m_enableRequestComputePower = false;
    m_configManager.setEnableRequestComputePower(false);
    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::switchToRequestComputingPower() {
    // 域控开启，本地模型是启用状态，本地模型与共享模型无法同时使用
    if (isEnableDomainControl() && isEnableLocalModel()) {
        showCannotOpenRequestServiceTipMessageBox();
        return false;
    }

    // 服务都未开启可能是请选择切换到请求算力,也可能是共享算力切换到请求算力
    if (!m_enableSharedComputePower) {
        if (m_curRole == RoleSelection::SharedComputingPower) {
            return switchToRequestFromUnopenedShare();
        } else {
            return switchToRequestFromPleaseSelect();
        }
    }
    if (m_enableSharedComputePower && !switchToRequestFromOpenedShare()) {
        return false;
    }
    return true;
}

bool ComputePowerShareDialog::switchToRequestFromPleaseSelect() {
    if (!m_sharedComputePowerList->enableService()) {
        return false;
    }
    setComboBoxIndex(RoleSelection::RequestComputingPower);
    return openRequestService();
}

bool ComputePowerShareDialog::switchToRequestFromUnopenedShare() {
    QString savedCode = m_configManager.getVerificationCode();
    if (savedCode.isEmpty()) {
        // 共享服务未打开，执行打开请求端服务的脚本
        if (!m_sharedComputePowerList->enableService()) {
            return false;
        }
    } else {
        // 共享算力验证码设置后，就会执行打开服务脚本，如果要切换到请求算例需要执行脚本关闭服务
        if (!m_requestComputePowerList->restartInferenceService()) {
            return false;
        }
    }
    setComboBoxIndex(RoleSelection::RequestComputingPower);
    m_verificationCodeFrame->hide();
    return openRequestService();
}

bool ComputePowerShareDialog::openRequestService() {
    setComponentsEnabled(false);
    showServiceOpeningTipWidget();
    if (!m_sharedComputePowerList->openService()) {
        hideServiceOpeningTipWidget();
        while (1) {
            if (m_sharedComputePowerList->disableService()) {
                break;
            }
        }

        resetToPleaseSelect();
        setComponentsEnabled(true);
        return false;
    }
    hideServiceOpeningTipWidget();

    m_sharedComputePowerList->show();

    // 获取Ip,设置Ip
    setLocalIpLabelText(m_sharedComputePowerList->getLocalIp());
    m_sharedComputePowerList->updateIpList();

    m_enableRequestComputePower = true;
    m_configManager.setEnableRequestComputePower(true);

    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::switchToRequestFromOpenedShare() {
    if (!confirmSwitchRole()) {
        return false;
    }
    setComponentsEnabled(false);
    if (!m_requestComputePowerList->resetService()) {
        setComponentsEnabled(true);
        return false;
    }

    m_verificationCodeFrame->hide();
    m_requestComputePowerList->hide();

    m_enableSharedComputePower = false;
    m_configManager.setEnableSharedComputePower(false);
    createTmpFile();

    while (1) {
        if (m_requestComputePowerList->restartInferenceService()) {
            break;
        }
    }

    m_sharedComputePowerList->show();
    m_sharedComputePowerList->updateIpList();

    m_enableRequestComputePower = true;
    m_configManager.setEnableRequestComputePower(true);

    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::switchToSharedComputingPower() {
    if (!isExistLocalModel()) {
        showNoLocalModelTipMessageBox();
        return false;
    }
    // 域控开启，本地模型是禁用状态，无法共享模型
    if (isEnableDomainControl() && !isEnableLocalModel()) {
        showCannotOpenSharedServiceTipMessageBox();
        return false;
    }
    QString savedCode = m_configManager.getVerificationCode();
    if (m_enableRequestComputePower && !switchToSharedFromRequest(savedCode)) {
        return false;
    }
    // 切换到共享算力成功
    showVerificationCodeFrame();
    if (savedCode.isEmpty()) {
        return true;
    }
    if (m_curRole == RoleSelection::PleaseSelect && !m_requestComputePowerList->enableService()) {
        m_verificationCodeFrame->hide();
        return false;
    }
    // 先将ComboBox设置为共享算力，再提示重启
    setComboBoxIndex(RoleSelection::SharedComputingPower);
    m_verificationCodePassword->setText(savedCode);
    createTmpFile();
    if (!isReboot()) {
        showRebootTipMessageBox();
        return true;
    }

    return openSharedServiceAndSettingCode(savedCode);
}

bool ComputePowerShareDialog::switchToSharedFromRequest(const QString &code) {
    if (!confirmSwitchRole()) {
        return false;
    }
    // 没有设置过验证码、或者设置验证码后没有重启，表示共享者服务未开启
    if (code.isEmpty() || !isReboot()) {
        if (!switchToUnopenedShareFromRequest(code)) {
            return false;
        }
    } else {
        if (!switchToOpenedSharedFromRequest(code)) {
            return false;
        }
    }
    return true;
}

bool ComputePowerShareDialog::switchToUnopenedShareFromRequest(const QString &code) {
    setComponentsEnabled(false);
    // 共享算力未设置验证码，关闭请求端服务
    if (!m_sharedComputePowerList->closeService()) {
        setComponentsEnabled(true);
        return false;
    }
    while (1) {
        if (code.isEmpty()) {
            // 执行脚本：关闭请求端服务
            if (m_sharedComputePowerList->disableService()) {
                break;
            }
        } else {
            // 执行脚本：关闭请求端服务，打开共享端服务
            if (m_sharedComputePowerList->restartInferenceService()) {
                break;
            }
        }
    }

    showVerificationCodeFrame();
    m_sharedComputePowerList->hide();

    m_enableRequestComputePower = false;
    m_configManager.setEnableRequestComputePower(false);
    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::switchToOpenedSharedFromRequest(const QString &code) {
    setComponentsEnabled(false);
    if (!m_sharedComputePowerList->resetService()) {
        setComponentsEnabled(true);
        return false;
    }
    m_sharedComputePowerList->hide();
    m_enableRequestComputePower = false;
    m_configManager.setEnableRequestComputePower(false);

    // 请求端切换到共享端成功，必须执行脚本重启推理服务
    while (1) {
        if (m_sharedComputePowerList->restartInferenceService()) {
            break;
        }
    }

    showVerificationCodeFrame();
    m_enableSharedComputePower = true;
    m_configManager.setEnableSharedComputePower(true);

    setLocalIpLabelText(m_requestComputePowerList->getLocalIp());
    if (m_requestComputePowerList->setVerificationCode(code)) {
        m_requestComputePowerList->show();
        m_requestComputePowerList->updateIpList();
    }
    setComponentsEnabled(true);
    return true;
}

bool ComputePowerShareDialog::confirmSwitchRole() {
    QString resetTip(tr(
        "Switching identities will interrupt the current computing power connection. Are you sure about the switch?"));
    if (!showResetServiceTipMessageBox(resetTip)) {
        return false;
    }
    return true;
}

void ComputePowerShareDialog::setLocalIpLabelText(const QString &localIp) {
    if (!localIp.isEmpty()) {
        m_localIpValueLabel->setText(localIp);
        m_configManager.setLocalIp(localIp);
        return;
    }
    QString savedIp = m_configManager.getLocalIp();
    if (!savedIp.isEmpty()) {
        m_localIpValueLabel->setText(savedIp);
    }
}

void ComputePowerShareDialog::checkLocalModelEnable() {
    kyai::config::model::ModelConfig mc;
    bool localModelEnable = mc.getModelDeployEnabled(kyai::config::model::DeployType::Ondevice);
    if (localModelEnable) {
        return;
    }
    emit localModelEnableStateChanged(true);
    showOpenLocalModelTipMessageBox();
}

bool ComputePowerShareDialog::isExistLocalModel() {
    kyai::config::model::ModelConfig mc;
    const auto models = mc.getExtendedModels(kyai::config::model::AiCapability::Nlp);
    for (const auto &model : models) {
        if (model.deployType == kyai::config::model::DeployType::Ondevice) {
            return true;
        }
    }
    return false;
}

void ComputePowerShareDialog::showOpenLocalModelTipMessageBox() {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(16, 16)));
    msgBox.setText(tr("When enabling computing power sharing, the local model is automatically enabled."));

    QPushButton *sureButton = msgBox.addButton(QObject::tr("Confirm"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    msgBox.exec();
}

void ComputePowerShareDialog::showNoLocalModelTipMessageBox() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(
        tr("The local model has not been installed on this machine, so it cannot support computing power sharing."));

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *installButton = msgBox.addButton(QObject::tr("Install Model"), QMessageBox::AcceptRole);
    connect(installButton, &QPushButton::clicked, this, &ComputePowerShareDialog::openSoftwareStore);
    msgBox.setDefaultButton(installButton);
    msgBox.exec();
}

void ComputePowerShareDialog::showCannotOpenSharedServiceTipMessageBox() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("Unable to enable computing power sharing"));
    msgBox.setInformativeText(
        tr("Computing power sharing requires the activation of the local model. However, the current device is already "
           "under unified management through domain control, preventing the activation of the local model."));

    QPushButton *sureButton = msgBox.addButton(QObject::tr("Confirm"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    msgBox.exec();
}

void ComputePowerShareDialog::showCannotOpenRequestServiceTipMessageBox() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("Unable to enable computing power sharing"));
    msgBox.setInformativeText(
        tr("The current equipment is managed uniformly by the domain controller, and the local model has been forced "
           "to be enabled. The local model and computing power sharing cannot be used simultaneously."));

    QPushButton *sureButton = msgBox.addButton(QObject::tr("Confirm"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    msgBox.exec();
}

bool ComputePowerShareDialog::showCloseServiceTipMessageBox(const QString &message) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(message);

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *closeButton = msgBox.addButton(QObject::tr("Close"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(closeButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

bool ComputePowerShareDialog::showResetServiceTipMessageBox(const QString &message) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(message);

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *switchButton = msgBox.addButton(QObject::tr("Switch"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(switchButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

void ComputePowerShareDialog::showRebootTipMessageBox() {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QIcon::fromTheme("dialog-info").pixmap(QSize(16, 16)));
    // 算力共享已启用，重启后将正式生效
    msgBox.setText(tr("Computing power sharing has been enabled and will officially take effect after restarting"));

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *sureButton = msgBox.addButton(QObject::tr("Reboot Now"), QMessageBox::AcceptRole);

    connect(sureButton, &QPushButton::clicked, this, &ComputePowerShareDialog::reboot);

    msgBox.setDefaultButton(sureButton);
    msgBox.exec();
}

void ComputePowerShareDialog::reboot() {
    QDBusInterface interface("org.gnome.SessionManager", "/org/gnome/SessionManager", "org.gnome.SessionManager",
                             QDBusConnection::sessionBus());
    interface.call("reboot");
}

void ComputePowerShareDialog::showServiceOpeningTipMessageBox() {
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QIcon::fromTheme("dialog-info").pixmap(QSize(16, 16)));
    msgBox.setText(
        tr("Computing power sharing is currently starting and cannot be turned off. Please try again later."));

    QPushButton *sureButton = msgBox.addButton(QObject::tr("Confirm"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    msgBox.exec();
}

void ComputePowerShareDialog::showVerificationCodeFrame() {
    m_verificationCodePassword->setText("");
    m_verificationCodePassword->setEchoMode(QLineEdit::Password);
    m_verificationCodeFrame->show();
}

bool ComputePowerShareDialog::isEnableDomainControl() { return m_configManager.getEnableDomainControl(); }

bool ComputePowerShareDialog::isEnableLocalModel() {
    kyai::config::model::ModelConfig mc;
    return mc.getModelDeployEnabled(kyai::config::model::DeployType::Ondevice);
}

void ComputePowerShareDialog::openSoftwareStore() {
    //打开软件商店
    QString cmd = QString("/usr/bin/kylin-software-center -menuid=2 -tagid=712");
    QProcess::startDetached(cmd);
}

UkccFrame *ComputePowerShareDialog::createFrame() {
    UkccFrame *frame = new UkccFrame();
    frame->setFixedSize(QSize(496, 60));
    return frame;
}
