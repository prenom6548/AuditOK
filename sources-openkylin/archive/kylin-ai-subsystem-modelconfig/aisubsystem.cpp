#include "aisubsystem.h"

#include <QtCore/qprocessordetection.h>
#include <kysdk/applications/kbackgroundgroup.h>
#include <kysdk/applications/klistwidget.h>
#include <kysdk/applications/kmessagebox.h>
#include <ukui-log4qt.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLibraryInfo>
#include <QString>
#include <QStringList>
#include <QTranslator>

static CloudModelConfigWidget* g_cloudModelConfigPage = nullptr;

static LocalModelConfigWidget* g_localModelConfigWidget = nullptr;

static PrivateModelConfigWidget* g_privateModelConfigWidget = nullptr;

TestAiSubSystem::TestAiSubSystem(QObject* parent) : QObject(parent) {}

CloudModelConfigWidget* TestAiSubSystem::getCloudModelConfigPage() { return g_cloudModelConfigPage; }

LocalModelConfigWidget* TestAiSubSystem::getLocalModelConfigPage() { return g_localModelConfigWidget; }

PrivateModelConfigWidget* TestAiSubSystem::getPrivateModelConfigPage() { return g_privateModelConfigWidget; }

QStringList translationsPaths() {
    QString binaryPath = QFileInfo(qApp->applicationDirPath()).absoluteFilePath();

    QString trPath = QDir::toNativeSeparators(binaryPath + "/translations");
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    return QStringList()
           << QStringLiteral(APP_PREFIX) +
                  "/share/kylin-ai-subsystem-plugin/kylin-ai-subsystem-modelconfig/translations"
           << trPath
           << QStringLiteral("/usr/share/kylin-ai-subsystem-plugin/kylin-ai-subsystem-modelconfig/translations")
           << QStringLiteral("/usr/local/share/kylin-ai-subsystem-plugin/kylin-ai-subsystem-modelconfig/translations");
#elif defined(Q_OS_WIN)
    return QStringList() << trPath;
#endif
}

QWidget* TestAiSubSystem::createModelConfigWidget() {
    initUkuiLog4qt("ukui-control-center");

    // 日志输出在 ~/.log/ukui-control-center.log
    KyInfo() << "Init AI model config.";

    m_translator = new QTranslator;
    QStringList trPaths = translationsPaths();

    for (const QString& path : trPaths) {
        KyInfo() << "binaryPath" << path;
        bool match = m_translator->load(QLocale(), QStringLiteral("Internationalization"), QStringLiteral("_"), path);
        if (match) {
            break;
        }
    }

    QCoreApplication::installTranslator(m_translator);

    m_mainWidget = new QWidget();
    m_mainWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_mainWidget");
    m_mainWidget->setContentsMargins(0, 0, 0, 0);

    // 模型配置
    m_modelConfigLabel = new QLabel(m_mainWidget);
    m_modelConfigLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_modelConfigLabel");
    m_modelConfigLabel->setContentsMargins(16, 8, 0, 8);
    m_modelConfigLabel->setText(tr("Model call priority"));

    // 云端模型
    m_publicCloudModel = new ModelWidget(
        tr("Cloud Model"),
        tr("Configure cloud models certified by Kylin (Deployed on the cloud server, access and use over the network)"),
        DeployType::PublicCloud);
    m_publicCloudModel->setAccessibleName("kylin-ai-subsystem-modelconfig_ModelWidget_m_publicCloudModel");
    // 设置父窗口为插件窗口类，然后调用exec实现模态窗口
    g_cloudModelConfigPage = new CloudModelConfigWidget(m_mainWidget);
    g_cloudModelConfigPage->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_CloudModelConfigWidget_g_cloudModelConfigPage");

    // 本地模型
    bool shouldShowLocalModel = hasLocalModel();

    if (shouldShowLocalModel == true) {
        m_ondeviceModel = new ModelWidget(
            tr("Local Model"),
            tr("Configure a local model certified by Kylin (Installed on local equipment, it can run without network)"),
            DeployType::Ondevice);
        m_ondeviceModel->setAccessibleName("kylin-ai-subsystem-modelconfig_ModelWidget_m_ondeviceModel");
        // 设置父窗口为插件窗口类，然后调用exec实现模态窗口
        g_localModelConfigWidget = new LocalModelConfigWidget(m_mainWidget);
        g_localModelConfigWidget->setAccessibleName(
            "kylin-ai-subsystem-modelconfig_LocalModelConfigWidget_g_localModelConfigWidget");

        QHBoxLayout* ondeviceHLayout = new QHBoxLayout(m_ondeviceModel);
        ondeviceHLayout->setContentsMargins(0, 0, 0, 0);
        ondeviceHLayout->setSpacing(0);
        ondeviceHLayout->addWidget(m_ondeviceModel);

        ModelConfig mc;
        const auto models = mc.getExtendedModels(AiCapability::Nlp);
        bool hasLocalNlpModel = false;
        for (const auto& model : models) {
            if (model.deployType == DeployType::Ondevice && !model.is3rdModel) {
                hasLocalNlpModel = true;
                break;
            }
        }

        // 根据是否有本地NLP模型来设置按钮文本
        if (hasLocalNlpModel) {
            m_ondeviceModel->setConfigButtonText(tr("Configure"));
        } else {
            m_ondeviceModel->setConfigButtonText(tr("Install"));
        }
    }
    // 自定义模型
    m_privateModel =
        new ModelWidget(tr("Custom Model"),
                        tr("Configure local model files, LAN, and public cloud model configurations without "
                           "Kylin authentication (we apologize for any inconvenience causeds.)"),
                        DeployType::Custom);
    // 设置父窗口为插件窗口类， 然后调用exec实现模态窗口
    g_privateModelConfigWidget = new PrivateModelConfigWidget(m_mainWidget);

    // 算力共享模型
    initComputePowerSharingMainWidget();

#ifdef ENABLE_COMPUTE_POWER_SHARING
    if (hasLocalModel()) {
        initComputePowerSharingFrame();
        if (isShowComputePowerSharingFrame()) {
            m_ondeviceModel->setModelSwitchBtnChecked(false);
            m_ondeviceModel->setModelSwitchBtnEnable(false);
            m_ondeviceModel->setTipButtonVisible(true);
            m_computePowerSharingModel->setModelSwitchBtnChecked(true);
            m_computePowerSharingModel->setModelSwitchBtnEnable(false);
            m_computePowerSharingModel->setVisible(true);
        }
    }
#endif

    m_modelGroup = new KBackgroundGroup;
    m_modelGroup->setMinimumWidth(495);
    m_modelGroup->setMaximumWidth(16777215);

    // 模型调度优先级
    m_modelCallPriorityFrame = new QFrame();
    m_modelCallPriorityFrame->setMinimumSize(QSize(495, 60));
    m_modelCallPriorityFrame->setMaximumSize(QSize(16777215, 60));
    m_modelCallPriorityFrame->setFrameShape(QFrame::Box);
    m_modelCallPriorityFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_modelCallPriorityFrame");
    m_modelCallPriorityWidget =
        new ModelCallPriority(hasLocalModel(), isShowComputePowerSharingFrame(), m_modelCallPriorityFrame);

    // 只有在有本地模型时才显示性能和资源相关UI
    if (hasLocalModel()) {
        m_perfResourceLabel = new QLabel(m_mainWidget);
        m_perfResourceLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_perfResourceLabel");
        m_perfResourceLabel->setContentsMargins(16, 8, 0, 8);
        m_perfResourceLabel->setText(tr("Performance and Resources"));

        // 性能与资源
        m_modelBootDelayLabelFrame = new QFrame();
        m_modelBootDelayLabelFrame->setMinimumSize(QSize(495, 65));
        m_modelBootDelayLabelFrame->setMaximumSize(QSize(16777215, 65));
        m_modelBootDelayLabelFrame->setFrameShape(QFrame::Box);
        m_modelBootDelayLabelFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_modelBootDelayLabelFrame");

        QHBoxLayout* modelBootDelayLayout = new QHBoxLayout(m_modelBootDelayLabelFrame);
        modelBootDelayLayout->setContentsMargins(16, 0, 16, 0);

        QVBoxLayout* labelsVLayout = new QVBoxLayout();
        labelsVLayout->setSpacing(0);
        labelsVLayout->setContentsMargins(0, 0, 0, 0);

        m_modelBootDelayLabel = new QLabel();
        m_modelBootDelayLabel->setText(tr("Automatically loading local speech model on system boot"));
        m_modelBootDelayLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_legalInformationLabel");

        kdk::KLabel* newKLabel = new kdk::KLabel();
        newKLabel->setText(
            tr("After being disabled, the voice function requires a model loading process upon its first use."));
        newKLabel->setAlignment(Qt::AlignLeft);
        setKLabelFontColor(newKLabel, QColor("#808080"));
        newKLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KLabel_newKLabel");

        labelsVLayout->addStretch();
        labelsVLayout->addWidget(m_modelBootDelayLabel);
        labelsVLayout->addWidget(newKLabel);
        labelsVLayout->addStretch();

        modelBootDelaySwitchBtn = new KSwitchButton();
        modelBootDelaySwitchBtn->setAccessibleName(
            "kylin-ai-subsystem-modelconfig_kdk::KSwitchButton_modelBootDelaySwitchBtn");
        modelBootDelaySwitchBtn->setCheckable(true);
        modelBootDelaySwitchBtn->setFixedSize(50, 24);

        ModelConfig mc;
        modelBootDelaySwitchBtn->setChecked(mc.getLoadOnBoot(speechToText));

        modelBootDelayLayout->addLayout(labelsVLayout);
        modelBootDelayLayout->addStretch();
        modelBootDelayLayout->addWidget(modelBootDelaySwitchBtn);
    }

    m_mainVLayout = new QVBoxLayout();
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->setSpacing(0);

    m_mainVLayout->addWidget(m_modelConfigLabel);

    initDomainControlTipWidget();
    m_mainVLayout->addWidget(m_domainControlTipWidget);
    m_mainVLayout->addSpacing(8);

    m_mainVLayout->addWidget(m_modelGroup);
    m_mainVLayout->addSpacing(8);
    m_mainVLayout->addWidget(m_modelCallPriorityFrame);

    // 只有在有本地模型时才添加性能和资源UI到布局
    if (hasLocalModel()) {
        m_mainVLayout->addSpacing(40);
        m_mainVLayout->addWidget(m_perfResourceLabel);
        m_mainVLayout->addWidget(m_modelBootDelayLabelFrame);
    }

    m_mainVLayout->addStretch();

    m_mainWidget->setLayout(m_mainVLayout);

    initConnect();
    initModelGroup();
    checkDomainControllerStatus();

    KyInfo() << "End AI model config.";

    return m_mainWidget;
}

QWidget* TestAiSubSystem::createComputePowerSharingWidget() {
#ifdef ENABLE_COMPUTE_POWER_SHARING
    if (!hasLocalModel()) {
        return m_computePowerShareMainWidget;
    }
    initComputePowerSharingMainWidget();
    // 打开共享算力服务，如果本地模型是禁用状态，启用本地模型
    connect(m_computePowerShareMainWidget, &ComputePowerShareWidget::localModelEnableStateChanged, [this](bool enable) {
        m_ondeviceModel->setModelSwitchBtnChecked(enable);
        setModelDeployEnable(DeployType::Ondevice, enable);
    });

    connect(m_computePowerShareMainWidget, &ComputePowerShareWidget::requestConnectionToggled, [this](bool connected) {
        // 请求端连接成功，模型配置显示共享算力分类，本地模型按钮禁止点击
        // 请求端连接断开，模型配置不显示共享算力分类，本地模型按钮可点击
        // 请求端连接成功/断开，只是设置本地模型按钮是否可点击，不改变本地模型启用/禁用状态
        if (connected) {
            handleRequestConnectionSuccess();
        } else {
            handleRequestDisconnected();
        }
    });

    // 点击模型配置本地模型启用/禁用按钮, 本地模型被禁用, 关闭共享端服务
    connect(m_ondeviceModel, &ModelWidget::closeModelSwitchBtn, [this]() {
        if (!m_computePowerShareMainWidget->isSharedServiceEnabled()) {
            setModelDeployEnable(DeployType::Ondevice, false);
            return;
        }
        m_ondeviceModel->setModelSwitchBtnChecked(true);
        if (confirmCloseSharedService()) {
            setModelDeployEnable(DeployType::Ondevice, false);
            m_computePowerShareMainWidget->closeSharedService();
            m_ondeviceModel->setModelSwitchBtnChecked(false);
        }
    });
#endif
    return m_computePowerShareMainWidget;
}

void TestAiSubSystem::handleRequestConnectionSuccess() {
    m_ondeviceModel->setModelSwitchBtnChecked(false);
    m_ondeviceModel->setModelSwitchBtnEnable(false);
    m_ondeviceModel->setTipButtonVisible(true);
    m_computePowerSharingModel->setModelSwitchBtnChecked(true);
    m_computePowerSharingModel->setModelSwitchBtnEnable(false);
    m_computePowerSharingModel->setVisible(true);
    setModelDeployEnable(DeployType::Shared, true);
    m_modelCallPriorityWidget->setSharedModelOptionVisible(true);
}

void TestAiSubSystem::handleRequestDisconnected() {
    kyai::config::model::ModelConfig mc;
    m_ondeviceModel->setModelSwitchBtnEnable(true);
    m_ondeviceModel->setTipButtonVisible(false);
    m_ondeviceModel->setModelSwitchBtnChecked(mc.getModelDeployEnabled(DeployType::Ondevice));
    m_computePowerSharingModel->setVisible(false);
    setModelDeployEnable(DeployType::Shared, false);
    m_modelCallPriorityWidget->setSharedModelOptionVisible(false);
}

void TestAiSubSystem::initComputePowerSharingFrame() {
    m_computePowerSharingModel = new ModelWidget(
        tr("Computing Power Sharing Model"),
        tr("Configure models from computing power sharing services (run through shared computing power nodes, "
           "obtain computing resources on demand, and complete inference)"),
        DeployType::Shared);
    m_computePowerSharingModel->setVisible(false);
}

bool TestAiSubSystem::isShowComputePowerSharingFrame() {
#ifdef ENABLE_COMPUTE_POWER_SHARING
    if (!hasLocalModel()) {
        return false;
    }
    kyai::config::model::ModelConfig mc;
    QString sharedModelName = mc.getSharedModelName().c_str();
    return m_computePowerShareMainWidget->isRequestServiceEnabled() && !sharedModelName.isEmpty();
#else
    return false;
#endif
}

void TestAiSubSystem::initComputePowerSharingMainWidget() {
    if (m_computePowerShareMainWidget != nullptr) {
        return;
    }
    m_computePowerShareMainWidget = new ComputePowerShareWidget();
}

void TestAiSubSystem::initModelGroup() {
    ModelConfig mc;
    int publicCloudValue = mc.getModelSchedulingPriority(DeployType::PublicCloud);
    int ondeviceValue = mc.getModelSchedulingPriority(DeployType::Ondevice);
    int customValue = mc.getModelSchedulingPriority(DeployType::Custom);
    int sharedValue = mc.getModelSchedulingPriority(DeployType::Shared);

    m_modelPriorityList.clear();
    m_modelPriorityList.append({publicCloudValue, DeployType::PublicCloud, m_publicCloudModel});
    if (hasLocalModel()) {
        m_modelPriorityList.append({ondeviceValue, DeployType::Ondevice, m_ondeviceModel});
#ifdef ENABLE_COMPUTE_POWER_SHARING
        m_modelPriorityList.append({sharedValue, DeployType::Shared, m_computePowerSharingModel});
#endif
    }
    m_modelPriorityList.append({customValue, DeployType::Custom, m_privateModel});

    sortItemsByPriority();
    updateUIOrder();
}

void TestAiSubSystem::checkDomainControllerStatus() {
    if (!isEnableDomainControl()) {
        return;
    }
    m_domainControlTipWidget->setVisible(true);
    m_publicCloudModel->setModelSwitchBtnEnable(false);
    m_privateModel->setModelSwitchBtnEnable(false);

    if (!hasLocalModel()) {
        return;
    }
    m_ondeviceModel->setModelSwitchBtnEnable(false);

#ifdef ENABLE_COMPUTE_POWER_SHARING
    kyai::config::model::ModelConfig mc;
    bool enableLocalModel = mc.getModelDeployEnabled(DeployType::Ondevice);

    if (!enableLocalModel && m_computePowerShareMainWidget->isSharedServiceEnabled()) {
        QTimer::singleShot(10, this, [this]() { m_computePowerShareMainWidget->closeSharedService(); });
    }
    if (enableLocalModel && m_computePowerShareMainWidget->isRequestServiceEnabled()) {
        m_computePowerSharingModel->setVisible(false);
        QTimer::singleShot(10, this, [this]() { m_computePowerShareMainWidget->closeRequestService(); });
    }
#endif
}

void TestAiSubSystem::initConnect() {
    // 云端模型
    connect(m_publicCloudModel, &ModelWidget::configClicked, this, [=] {
        // 重新初始化云端模型对话框界面
        g_cloudModelConfigPage->deleteLater();
        g_cloudModelConfigPage = nullptr;
        g_cloudModelConfigPage = new CloudModelConfigWidget(m_mainWidget);
        g_cloudModelConfigPage->setAccessibleName(
            "kylin-ai-subsystem-modelconfig_CloudModelConfigWidget_g_cloudModelConfigPage");
        g_cloudModelConfigPage->showCentered();
        TestAiSubSystem::getCloudModelConfigPage()->exec();
    });

    connect(m_publicCloudModel, &ModelWidget::sortClicked, this, [=] { moveItemUp(DeployType::PublicCloud); });

    // 本地模型
    if (hasLocalModel() == true) {
        connect(m_ondeviceModel, &ModelWidget::configClicked, this, [=] {
            // 重新初始化本地模型对话框界面
            g_localModelConfigWidget->deleteLater();
            g_localModelConfigWidget = nullptr;
            g_localModelConfigWidget = new LocalModelConfigWidget(m_mainWidget);
            g_localModelConfigWidget->setAccessibleName(
                "kylin-ai-subsystem-modelconfig_LocalModelConfigWidget_g_localModelConfigWidget");
            g_localModelConfigWidget->showCentered();

#ifdef ENABLE_COMPUTE_POWER_SHARING
            connect(g_localModelConfigWidget, &LocalModelConfigWidget::modelUninstall,
                    [this](const QString& modelName) {
                        kyai::config::model::ModelConfig mc;
                        QString curModelName = mc.getCurrentModelName(AiCapability::Nlp, DeployType::Ondevice).c_str();
                        if (curModelName == modelName && m_computePowerShareMainWidget->isSharedServiceEnabled()) {
                            m_computePowerShareMainWidget->closeSharedService();
                        }
                    });
            connect(g_localModelConfigWidget, &LocalModelConfigWidget::switchModel, [this]() {
                if (m_computePowerShareMainWidget->isSharedServiceEnabled()) {
                    m_computePowerShareMainWidget->closeSharedService();
                }
            });
#endif

            TestAiSubSystem::getLocalModelConfigPage()->exec();
        });

        connect(m_ondeviceModel, &ModelWidget::closeModelSwitchBtn, [this]() {
            m_ondeviceModel->setModelSwitchBtnChecked(true);
            if (confirmCloseLocalModel()) {
                setModelDeployEnable(DeployType::Ondevice, false);
                m_ondeviceModel->setModelSwitchBtnChecked(false);
            }
        });

        connect(m_ondeviceModel, &ModelWidget::sortClicked, this, [=] { moveItemUp(DeployType::Ondevice); });

        // 语音模型开机是否加载
        connect(modelBootDelaySwitchBtn, &kdk::KSwitchButton::clicked, this, &TestAiSubSystem::onModelBootDelayChanged);
    }

    // 自定义模型
    connect(m_privateModel, &ModelWidget::configClicked, this, [=] {
        // 重新初始化自定义模型对话框界面
        g_privateModelConfigWidget->deleteLater();
        g_privateModelConfigWidget = nullptr;
        g_privateModelConfigWidget = new PrivateModelConfigWidget(m_mainWidget);
        g_privateModelConfigWidget->showCentered();
        TestAiSubSystem::getPrivateModelConfigPage()->exec();
    });

    connect(m_privateModel, &ModelWidget::sortClicked, this, [=] { moveItemUp(DeployType::PrivateCloud); });

    connect(m_modelCallPriorityWidget, &ModelCallPriority::topModelPriorityChanged, this,
            &TestAiSubSystem::onModelPriorityChanged);

    // 共享模型
#ifdef ENABLE_COMPUTE_POWER_SHARING
    if (hasLocalModel()) {
        connect(m_computePowerSharingModel, &ModelWidget::configClicked, this, [=] {
            m_sharedModelConfigWidget->deleteLater();
            m_sharedModelConfigWidget = nullptr;
            m_sharedModelConfigWidget = new SharedModelConfigWidget(m_mainWidget);
            m_sharedModelConfigWidget->exec();
        });

        connect(m_computePowerSharingModel, &ModelWidget::sortClicked, this, [=] { moveItemUp(DeployType::Shared); });
    }
#endif
}

void TestAiSubSystem::moveItemUp(DeployType deployType) {
    int currentIndex = -1;
    for (int i = 0; i < m_modelPriorityList.size(); i++) {
        if (m_modelPriorityList[i].deployType == deployType) {
            currentIndex = i;
            break;
        }
    }
    if (currentIndex > 0) {
        // 交换优先级值
        std::swap(m_modelPriorityList[currentIndex].priority, m_modelPriorityList[currentIndex - 1].priority);
        // 重新排序
        sortItemsByPriority();
        // 更新UI
        updateUIOrder();
        saveModelPriority();
    }
}

void TestAiSubSystem::updateUIOrder() {
    QList<QWidget*> modelUIList;
    bool isFirstItem = true;
    for (const PriorityItem& item : m_modelPriorityList) {
        if (isFirstItem) {
            // 更新按钮状态
            item.modelWidget->m_sortButton->setEnabled(false);
            // 更新模型调度优先级
            m_modelCallPriorityWidget->setModelCallPriority(item.deployType);
            isFirstItem = false;
        } else {
            item.modelWidget->m_sortButton->setEnabled(true);
        }
        modelUIList.append(item.modelWidget);
    }
    m_modelGroup->removeWidgetList(m_modelGroup->widgetList());
    m_modelGroup->addWidgetList(modelUIList);
}

void TestAiSubSystem::onModelPriorityChanged(DeployType topPriorityModelType) {
    PriorityItem itemToMove;
    int index = -1;
    for (int i = 0; i < m_modelPriorityList.size(); i++) {
        if (m_modelPriorityList[i].deployType == topPriorityModelType) {
            itemToMove = m_modelPriorityList[i];
            index = i;
            break;
        }
    }
    if (index <= 0 || index >= m_modelPriorityList.size()) {
        return;
    }
    for (int i = index; i > 0; i--) {
        m_modelPriorityList[i] = m_modelPriorityList[i - 1];
        m_modelPriorityList[i].priority++;  // 优先级递增
    }

    itemToMove.priority = 1;
    m_modelPriorityList[0] = itemToMove;

    // 更新UI
    updateUIOrder();
    saveModelPriority();
}

void TestAiSubSystem::saveModelPriority() {
    QMap<DeployType, int> priorityMap;
    for (const PriorityItem& item : m_modelPriorityList) {
        priorityMap.insert(item.deployType, item.priority);
    }
    updateModelPriority(priorityMap);
}

void TestAiSubSystem::updateModelPriority(const QMap<DeployType, int>& priorityMap) {
    bool ret = false;
    bool shouldShowLocalModel = hasLocalModel();

    // 遍历 priorityMap，设置每个模型的优先级
    for (auto it = priorityMap.begin(); it != priorityMap.end(); ++it) {
        DeployType type = it.key();
        int priority = it.value();

        // 如果本地模型隐藏，则跳过
        if (shouldShowLocalModel == false && type == DeployType::Ondevice) {
            continue;
        }

        ModelConfig mc;
        ret = mc.setModelSchedulingPriority(type, priority);
        if (ret == false) {
            switch (type) {
                case DeployType::PublicCloud:
                    KyInfo() << "Failed to set the call priority for DeployType: PublicCloud with priority:"
                             << priority;
                    showErrorMessage(tr("Failed to set cloud model priority"), m_mainWidget);
                    break;
                case DeployType::Ondevice:
                    if (shouldShowLocalModel == true) {
                        KyInfo() << "Failed to set the call priority for DeployType: Ondevice with priority:"
                                 << priority;
                        showErrorMessage(tr("Failed to set local model priority"), m_mainWidget);
                    }
                    break;
                case DeployType::Custom:
                    KyInfo() << "Failed to set the call priority for DeployType: Custom with priority:" << priority;
                    showErrorMessage(tr("Failed to set custom model priority"), m_mainWidget);
                    break;
                default:
                    KyInfo() << "Unknown DeployType";
                    break;
            }
        } else {
            // 设置成功
            switch (type) {
                case DeployType::PublicCloud:
                    KyInfo() << "Successfully set the call priority for DeployType: PublicCloud with priority:"
                             << priority;
                    break;
                case DeployType::Ondevice:
                    if (shouldShowLocalModel == true) {
                        KyInfo() << "Successfully set the call priority for DeployType: Ondevice with priority:"
                                 << priority;
                    }
                    break;
                case DeployType::Custom:
                    KyInfo() << "Successfully set the call priority for DeployType: Custom with priority:" << priority;
                    break;
                default:
                    KyInfo() << "Successfully set the call priority for Unknown DeployType with priority:" << priority;
                    break;
            }
        }
    }
}

void TestAiSubSystem::sortItemsByPriority() {
    std::sort(m_modelPriorityList.begin(), m_modelPriorityList.end(),
              [](const PriorityItem& a, const PriorityItem& b) { return a.priority < b.priority; });
}

bool TestAiSubSystem::hasLocalModel() {
#if (defined(Q_PROCESSOR_X86) || (defined(Q_PROCESSOR_ARM)))
    return true;
#else
    return false;
#endif
}

bool TestAiSubSystem::confirmCloseSharedService() {
    QMessageBox msgBox(m_mainWidget);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(
        tr("After closing the local model, computing power sharing will be interrupted. Do you want to continue "
           "closing?"));

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton* closeButton = msgBox.addButton(QObject::tr("Close"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(closeButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

bool TestAiSubSystem::confirmCloseLocalModel() {
    QMessageBox msgBox(m_mainWidget);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("AI indexing relies on the local model"));
    msgBox.setInformativeText(
        tr("Closing the local model will disable AI indexing-related capabilities, including the generation and "
           "updating of smart tags and summary indexes. Do you wish to proceed with closing?"));
    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton* closeButton = msgBox.addButton(QObject::tr("Continue Closing"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(closeButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

void TestAiSubSystem::setModelDeployEnable(DeployType deployType, bool enable) {
    ModelConfig mc;
    bool ret = mc.setModelDeployEnabled(deployType, enable);
    if (!ret) {
        KyWarning() << "Failed to set deployType" << (int)deployType << enable;
    }
}

bool TestAiSubSystem::isEnableDomainControl() { return m_configManager.getEnableDomainControl(); }

void TestAiSubSystem::initDomainControlTipWidget() {
    m_domainControlTipWidget = new QWidget(m_mainWidget);
    m_domainControlTipWidget->setFixedHeight(40);
    m_domainControlTipWidget->setMinimumSize(QSize(495, 56));
    m_domainControlTipWidget->setMaximumSize(QSize(16777215, 56));
    m_domainControlTipWidget->setStyleSheet("background:rgba(55, 144, 250, 0.1); border-radius:6px;");
    m_domainControlTipWidget->setVisible(false);

    QHBoxLayout* hLayout = new QHBoxLayout(m_domainControlTipWidget);
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);

    // 图标
    QLabel* m_iconLabel = new QLabel();
    m_iconLabel->setPixmap(QIcon::fromTheme("dialog-info").pixmap(16, 16));
    m_iconLabel->setStyleSheet("background: transparent;");
    m_iconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_iconLabel");

    // 提示信息
    kdk::KLabel* m_tipsLabel = new kdk::KLabel;
    m_tipsLabel->setText(
        tr("The current device has been integrated into the unified management of the domain controller, "
           "and the startup and shutdown of cloud/local/optional models are controlled by the domain controller. "
           "Local modification is not supported at this time."));
    m_tipsLabel->setStyleSheet(
        "color: rgba(55, 144, 250, 1); background: transparent; padding-top:0px; padding-left:2px; padding-right:2px;");
    m_tipsLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_m_descripLabel");

    hLayout->addSpacing(16);
    hLayout->addWidget(m_iconLabel);
    hLayout->addSpacing(6);
    hLayout->addWidget(m_tipsLabel);
    hLayout->addStretch();
}

void TestAiSubSystem::onModelBootDelayChanged(bool checked) {
    ModelConfig mc;
    bool ret = mc.setLoadOnBoot(speechToText, checked);
    if (!ret) {
        KyWarning() << "Failed to set LoadOnBoot" << "speechToText" << checked;
    }
}
