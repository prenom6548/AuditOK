#include "computepowersharewidget.h"

#include <aiconfig/modelconfig.h>
#include <klabel.h>

ComputePowerShareWidget::ComputePowerShareWidget(QWidget *parent) : QWidget(parent) {
    initComputePowerShareWidget();
    initConnections();
}

ComputePowerShareWidget::~ComputePowerShareWidget() {
    if (m_computePowerShareDialog) {
        delete m_computePowerShareDialog;
        m_computePowerShareDialog = nullptr;
    }
}

void ComputePowerShareWidget::closeSharedService() {
    m_computePowerInterface.closeServerService();
    while (1) {
        if (m_computePowerInterface.disableProviderService()) {
            break;
        }
    }

    kyai::config::model::ModelConfig mc;
    mc.stopModelSharing(
        mc.getCurrentModelName(kyai::config::model::AiCapability::Nlp, kyai::config::model::DeployType::Ondevice));
    m_configManager.setEnableSharedComputePower(false);
    m_configManager.setComputePowerSharingRole("");
    m_configManager.setEnableComputePowerSharing(false);
}

void ComputePowerShareWidget::closeRequestService() {
    m_computePowerInterface.closeEndSideService();
    while (1) {
        if (m_computePowerInterface.disableConsumerService()) {
            break;
        }
    }

    m_configManager.setEnableRequestComputePower(false);
    m_configManager.setComputePowerSharingRole("");
    m_configManager.setEnableComputePowerSharing(false);
}

void ComputePowerShareWidget::openComputePowerShare() { showComputePowerShareDialog(); }

bool ComputePowerShareWidget::isSharedServiceEnabled() { return m_configManager.getEnableSharedComputePower(); }

bool ComputePowerShareWidget::isRequestServiceEnabled() { return m_configManager.getEnableRequestComputePower(); }

void ComputePowerShareWidget::initComputePowerShareWidget() {
    m_mainVLayout = new QVBoxLayout(this);
    m_mainVLayout->setSpacing(0);
    m_mainVLayout->setContentsMargins(0, 40, 0, 0);

    m_advancedSettingsLabel = new QLabel();
    m_advancedSettingsLabel->setText(tr("Advanced Settings"));
    m_advancedSettingsLabel->setContentsMargins(16, 0, 16, 8);
    m_advancedSettingsLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_advancedSettingsLabel");

#ifdef ENABLE_COMPUTE_POWER_SHARING
    // 算力共享
    m_computePowerShareFrame = new QFrame(this);
    m_computePowerShareFrame->setMinimumSize(QSize(495, 60));
    m_computePowerShareFrame->setMaximumSize(QSize(16777215, 60));
    m_computePowerShareFrame->setFrameShape(QFrame::Box);
    m_computePowerShareFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_computePowerShareFrame");

    QHBoxLayout *computePowerShareLayout = new QHBoxLayout(m_computePowerShareFrame);
    computePowerShareLayout->setContentsMargins(16, 0, 16, 0);

    m_computePowerShareLabel = new QLabel();
    m_computePowerShareLabel->setText(tr("Computing Power Sharing"));
    m_computePowerShareLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_computePowerShareLabel");

    m_settingsBtn = new QPushButton();
    m_settingsBtn->setText(tr("Settings"));
    m_settingsBtn->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_settingsBtn");

    computePowerShareLayout->addWidget(m_computePowerShareLabel);
    computePowerShareLayout->addStretch();
    computePowerShareLayout->addWidget(m_settingsBtn);
#endif

    // 法律信息
    m_legalInformationFrame = new QFrame(this);
    m_legalInformationFrame->setMinimumSize(QSize(495, 60));
    m_legalInformationFrame->setMaximumSize(QSize(16777215, 60));
    m_legalInformationFrame->setFrameShape(QFrame::Box);
    m_legalInformationFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_legalInformationFrame");

    QHBoxLayout *legalInformationLayout = new QHBoxLayout(m_legalInformationFrame);
    legalInformationLayout->setContentsMargins(16, 0, 16, 0);

    m_legalInformationLabel = new QLabel();
    m_legalInformationLabel->setText(tr("Legal Information"));
    m_legalInformationLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_legalInformationLabel");

    m_detailsBtn = new QPushButton();
    m_detailsBtn->setText(tr("Details"));
    m_detailsBtn->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_detailsBtn");

    legalInformationLayout->addWidget(m_legalInformationLabel);
    legalInformationLayout->addStretch();
    legalInformationLayout->addWidget(m_detailsBtn);

    m_mainVLayout->addWidget(m_advancedSettingsLabel);
#ifdef ENABLE_COMPUTE_POWER_SHARING
    m_mainVLayout->addWidget(m_computePowerShareFrame);
    m_mainVLayout->addSpacing(10);
#endif
    m_mainVLayout->addWidget(m_legalInformationFrame);
    m_mainVLayout->addStretch();
}

void ComputePowerShareWidget::initConnections() {
#ifdef ENABLE_COMPUTE_POWER_SHARING
    connect(m_settingsBtn, &QPushButton::clicked, [this]() { showComputePowerShareDialog(); });
#endif

    connect(m_detailsBtn, &QPushButton::clicked, [this]() { showLegalInformationDialog(); });
}

void ComputePowerShareWidget::showComputePowerShareDialog() {
    if (m_computePowerShareDialog) {
        delete m_computePowerShareDialog;
        m_computePowerShareDialog = nullptr;
    }
    m_computePowerShareDialog = new ComputePowerShareDialog(this);
    connect(m_computePowerShareDialog, &ComputePowerShareDialog::localModelEnableStateChanged, this,
            &ComputePowerShareWidget::localModelEnableStateChanged);
    connect(m_computePowerShareDialog, &ComputePowerShareDialog::requestConnectionToggled, this,
            &ComputePowerShareWidget::requestConnectionToggled);
    m_computePowerShareDialog->exec();
}

void ComputePowerShareWidget::showLegalInformationDialog() {
    if (!m_legalInformationDailog) {
        initLegalInformationDialog();
    }
    m_legalInformationDailog->exec();
}

void ComputePowerShareWidget::initLegalInformationDialog() {
    m_legalInformationDailog = new QDialog(this);
    m_legalInformationDailog->setFixedSize(QSize(452, 304));
    m_legalInformationDailog->setWindowTitle(tr("Disclaimers"));
    m_legalInformationDailog->setAccessibleName("kylin-ai-subsystem-modelconfig_QDialog_legalInformationDailog");

    m_legalInformationVLayout = new QVBoxLayout(m_legalInformationDailog);
    m_legalInformationVLayout->setContentsMargins(24, 24, 24, 24);
    m_legalInformationVLayout->setSpacing(0);

    kdk::KLabel *titleKLabel = new kdk::KLabel();
    titleKLabel->setText(tr("Disclaimer for Local File Retrieval"));
    titleKLabel->setAlignment(Qt::AlignCenter);
    titleKLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_titleKLabel");

    kdk::KLabel *descriptionKLabel = new kdk::KLabel();
    descriptionKLabel->setAlignment(Qt::AlignTop);
    descriptionKLabel->setFixedSize(QSize(404, 128));
    descriptionKLabel->setWordWrap(true);
    descriptionKLabel->setText(tr(
        "The retrieval of local files without sensitive word filtering results in adverse consequences that are based "
        "on the original content of the local files. Users are responsible for the search results based on the content "
        "of the local files themselves, and Kirin Software does not assume legal responsibility for it."));
    descriptionKLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_descriptionKLabel");

    kdk::KLabel *kylinKLabel = new kdk::KLabel();
    kylinKLabel->setText(tr("KylinSoft Corporation"));
    kylinKLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_kylinKLabel");

    QHBoxLayout *lableHLayout = new QHBoxLayout();
    lableHLayout->addStretch();
    lableHLayout->addWidget(kylinKLabel);

    m_legalInformationVLayout->addWidget(titleKLabel);
    m_legalInformationVLayout->addSpacing(24);
    m_legalInformationVLayout->addWidget(descriptionKLabel);
    m_legalInformationVLayout->addSpacing(24);
    m_legalInformationVLayout->addLayout(lableHLayout);
}
