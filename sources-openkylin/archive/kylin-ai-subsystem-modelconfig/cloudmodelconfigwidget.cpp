#include "cloudmodelconfigwidget.h"

#include <ukui-log4qt.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QScrollArea>

#include "util.h"

CloudModelConfigWidget::CloudModelConfigWidget(QWidget *parent) : QDialog(parent)
{
    // 初始化布局
    initLayout();

    // 加载已配置的模型并显示
    loadModelConfig();

    // 初始化信号槽
    initConnect();
}

CloudModelConfigWidget::~CloudModelConfigWidget()
{
    m_nlpModelMap.clear();

    // 删除ModelItem对象
    for (ModelItem* item : m_nlpModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }
    m_nlpModelItemMap.clear();

    m_nlpModelItemList.clear();

    m_nlpModelGroup->removeAndDeleteAllWidget();

    m_nlpModelMapUI.clear();
}

void CloudModelConfigWidget::initLayout() {
    this->setFixedSize(544, 610);
    this->setWindowTitle(tr("Cloud Model Configuration"));

    m_mainVLayout = new QVBoxLayout();

    initModelTitleLayout();
    initModellayout();

    m_mainVLayout->setStretch(0,2);
    m_mainVLayout->setStretch(1,8);
    this->setLayout(m_mainVLayout);
}

void CloudModelConfigWidget::initModelTitleLayout() {
    QLabel* backgroundLabel = new QLabel(this);
    backgroundLabel->setFixedHeight(72);
    backgroundLabel->setPixmap(QPixmap(":/res/icon/background.svg"));
    backgroundLabel->setAlignment(Qt::AlignCenter);
    backgroundLabel->setScaledContents(false);
    backgroundLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_backgroundLabel");

    m_titleHLayout = new QHBoxLayout(backgroundLabel);
    m_titleHLayout->setContentsMargins(24, 0, 24, 0);

    m_vLeftLayout = new QVBoxLayout();
    m_iconLabel = new QLabel(backgroundLabel);
    m_iconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_iconLabel");
    QPixmap iconPixmap(":/res/icon/ukui-cloud-model-symbolic.svg");
    if(!iconPixmap.isNull())
    {   iconPixmap = iconPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_iconLabel->setPixmap(iconPixmap);
    }
    m_iconLabel->setFixedSize(24, 24);
    m_vLeftLayout->addWidget(m_iconLabel);

    m_cloudHLayout = new QVBoxLayout();
    m_cloudHLayout->setContentsMargins(0, 12, 0, 16);

    m_cloudLabel = new QLabel(backgroundLabel);
    m_cloudLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_cloudLabel");
    m_cloudLabel->setMinimumSize(216, 24);
    m_cloudLabel->setText(tr("Cloud Model"));
    QFont cloudFont;
    cloudFont.setBold(true);
    m_cloudLabel->setFont(cloudFont);
    m_cloudLabel->setStyleSheet("color: #3790FA; background: transparent;");

    m_descripLabel = new kdk::KLabel(backgroundLabel);
    m_descripLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_descripLabel");
    m_descripLabel->setMinimumSize(450, 20);

    m_descripLabel->setText(tr(
        "Configure a cloud model certified by Kylin(Deployed on the cloud server, access and use over the network)"));
    QFont cloudDescripFont;
    cloudDescripFont.setPointSize(cloudFont.pointSize() - 2);
    m_descripLabel->setFont(cloudDescripFont);
    setKLabelFontColor(m_descripLabel, "#3790FA");

    m_cloudHLayout->addWidget(m_cloudLabel);
    m_cloudHLayout->addWidget(m_descripLabel);

    m_titleHLayout->addLayout(m_vLeftLayout);
    m_titleHLayout->addLayout(m_cloudHLayout);

    m_mainVLayout->addWidget(backgroundLabel);
}


void CloudModelConfigWidget::initModellayout()
{
    m_baseVLayout = new QVBoxLayout;
    m_baseVLayout->setContentsMargins(0, 30, 0, 0);

    // NLP大语言模型
    m_nlpWidget = new QWidget;
    m_nlpWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_nlpWidget");

    m_nlpVLayout = new QVBoxLayout;
    m_nlpVLayout->setContentsMargins(0, 0, 0, 0);
    m_nlpVLayout->setSpacing(8);
    m_nlpHLayout = new QHBoxLayout;
    m_nlpHLayout->setContentsMargins(16, 0, 16, 0);
    m_nlpLabel = new QLabel;
    m_nlpLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_nlpLabel");
    m_nlpLabel->setText(tr("NLP Model"));

    // NLP描述
    m_nlpIconLabel = new QLabel(this);
    m_nlpIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    m_nlpIconLabel->setProperty("useIconHighlightEffect", 0x2);
    m_nlpIconLabel->setToolTip(
        tr("Processing and generating text, such as understanding text and answering questions"));

    // 添加按钮
    m_nlpAddButton = new kdk::KBorderlessButton;
    m_nlpAddButton->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_m_nlpAddButton");
    m_nlpAddButton->setFixedSize(60, 28);
    m_nlpAddButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_nlpAddButton->setIconSize(QSize(16, 16));
    m_nlpAddButton->setText(tr("Add"));

    // NLP模型大号添加按钮
    m_nlpAddItemButton = new QPushButton(tr("+ Add"), this);
    m_nlpAddItemButton->setObjectName("NlpAddItemButton");
    m_nlpAddItemButton->setFixedSize(496, 56);
    m_nlpAddItemButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_nlpAddItemButton");

    // 已配置模型
    m_nlpModelGroup = new SettingGroup(m_nlpWidget);
    m_nlpModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_nlpModelGroup");

    m_freeTrialModelGroup = new SettingGroup(m_nlpWidget);
    m_freeTrialModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_freeTrialModelGroup");

    m_nlpHLayout->addWidget(m_nlpLabel);
    m_nlpHLayout->addWidget(m_nlpIconLabel);
    m_nlpHLayout->addStretch();
    m_nlpHLayout->setSpacing(4);
    m_nlpHLayout->addWidget(m_nlpAddButton);

    m_nlpVLayout->addLayout(m_nlpHLayout);
    m_nlpVLayout->addWidget(m_nlpAddItemButton);
    m_nlpVLayout->addWidget(m_freeTrialModelGroup);
    m_nlpVLayout->addWidget(m_nlpModelGroup);

    m_nlpWidget->setLayout(m_nlpVLayout);

    // 滚动区域
    m_scrollVLayout = new QVBoxLayout;
    m_scrollVLayout->setContentsMargins(16, 20, 16, 0);
    m_scrollVLayout->addWidget(m_nlpWidget);
    m_scrollVLayout->addStretch();

    m_scrollWidget = new QWidget;
    m_scrollWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_speechWidget");
    m_scrollWidget->setContentsMargins(0, 0, 0, 0);
    m_scrollWidget->setLayout(m_scrollVLayout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setAccessibleName("kylin-ai-subsystem-modelconfig_QScrollArea_m_scrollArea");
    m_scrollArea->setFixedHeight(420);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);  // 无边框
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 不显示水平滚动条
    m_scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);

    m_scrollArea->setWidget(m_scrollWidget);
    m_mainVLayout->addWidget(m_scrollArea);

    m_confirmWidget = new QWidget(this);
    m_confirmWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_confirmWidget");
    m_confirmButton = new QPushButton(tr("OK"), this);
    m_confirmButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_confirmButton");
    m_confirmButton->setProperty("isImportant", true);
    m_confirmButtoLayout = new QHBoxLayout();
    m_confirmButtoLayout->setContentsMargins(0, 0, 24, 24);
    m_confirmButtoLayout->addStretch();
    m_confirmButtoLayout->addWidget(m_confirmButton);
    m_confirmWidget->setLayout(m_confirmButtoLayout);
    m_mainVLayout->addStretch();
    m_mainVLayout->addWidget(m_confirmWidget);
}

// 更新NLP模型添加按钮的可见性
void CloudModelConfigWidget::updateNlpAddButtonVisibility() {
    bool hasNlpModels = !m_nlpModelItemMap.isEmpty();

    if (hasNlpModels) {
        // 有NLP模型时：隐藏大号按钮，显示小按钮
        m_nlpAddItemButton->hide();
        m_nlpAddButton->show();
    } else {
        // 没有NLP模型时：显示大号按钮，隐藏小按钮
        m_nlpAddItemButton->show();
        m_nlpAddButton->hide();
    }
}

void CloudModelConfigWidget::initConnect()
{
    connect(m_nlpAddButton, &QPushButton::clicked, this, [this]() { this->openSoftwareStore(); });
    connect(m_nlpAddItemButton, &QPushButton::clicked, this, [this]() { this->openSoftwareStore(); });
    connect(m_confirmButton, &QPushButton::clicked, this, [this](){
        this->close();
    });

}

bool CloudModelConfigWidget::eventFilter(QObject* watched, QEvent* event) {
    Q_UNUSED(watched);

    // 模型配置
    ModelItem* clickedModelItem = qobject_cast<ModelItem*>(watched);

    if (clickedModelItem) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            if (mouseEvent->button() == Qt::LeftButton) {  // 判断左键
                                                           // 切换模型
                clickCloudModel(clickedModelItem->getAICapability(), clickedModelItem->getModelName());
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CloudModelConfigWidget::clickCloudModel(AiCapability capability, QString modelName) {
    KyInfo() << "正要切换到" << static_cast<int>(capability) << modelName;
    KyInfo() << "当前正在回答的选中模型为" << nlpSelectedModel;
    bool ret = checkModelSelection(capability, modelName);
    if (ret == true) {
        return;
    } else {
        if ((capability == AiCapability::Nlp && (getModelStatus(nlpSelectedModel) == ModelStatus::Running)) ||
            (capability == AiCapability::Vision && (getModelStatus(visionSelectedModel) == ModelStatus::Running)) ||
            (capability == AiCapability::Speech && (getModelStatus(speechSelectedModel) == ModelStatus::Running))) {
            // NLP or VISION or SPEECH 正在使用
            KyInfo() << "正在使用" << static_cast<int>(capability);
            bool canProceed = showModelInUseMessage(this);
            if (canProceed == true) {
                clickCloudModel(capability, modelName);
            } else {
                KyInfo() << "取消本次操作";
                return;
            }
        } else {
            bool ret = setSelectCloudModelName(capability, DeployType::PublicCloud, modelName);
            if (ret == true) {
                switchModelItem(capability, modelName);
                KyInfo() << modelName << " 设置为选中模型成功";
            } else {
                showErrorMessage(tr("Model switching failed"), this);
                KyInfo() << modelName << " 设置为选中模型失败";
            }
        }
    }
}

void CloudModelConfigWidget::loadModelConfig() {
    // 清空modelmap所有控件
    m_nlpModelMap.clear();

    // 删除ModelItem对象
    for (ModelItem* item : m_nlpModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }
    m_nlpModelItemMap.clear();

    m_nlpModelItemList.clear();

    m_nlpModelGroup->removeAndDeleteAllWidget();

    m_nlpModelMapUI.clear();

    loadModels();

    updateNlpAddButtonVisibility();
}

void CloudModelConfigWidget::loadModels() { loadFreeTrialModels(); }

void CloudModelConfigWidget::loadNoFreeModels() {
    ModelConfig mc;

    for (auto capability : {AiCapability::Nlp, AiCapability::Vision, AiCapability::Speech}) {
        std::vector<ExtendedModel> models = mc.getExtendedModels(capability);
        processModelInfo(models, capability);

        if (capability == AiCapability::Nlp) {
            std::string nlpModel = mc.getCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud);
            KyInfo() << "loadModelConfig-------------getCurrentModelName" << "nlpModel"
                     << QString::fromStdString(nlpModel);
            nlpSelectedModel = QString::fromStdString(nlpModel);
        } else if (capability == AiCapability::Vision) {
            std::string visionModel = mc.getCurrentModelName(AiCapability::Vision, DeployType::PublicCloud);
            KyInfo() << "loadModelConfig-------------getCurrentModelName" << "visionModel"
                     << QString::fromStdString(visionModel);
            visionSelectedModel = QString::fromStdString(visionModel);
        } else if (capability == AiCapability::Speech) {
            std::string speechModel = mc.getCurrentModelName(AiCapability::Speech, DeployType::PublicCloud);
            KyInfo() << "loadModelConfig-------------getCurrentModelName" << "speechModel"
                     << QString::fromStdString(speechModel);
            speechSelectedModel = QString::fromStdString(speechModel);
        }

        for (const auto &model : models) {
            PublicCloudModel publicCloudModel;
            publicCloudModel.capability = model.capability;
            publicCloudModel.vendor = QString::fromStdString(model.vendor);
            publicCloudModel.modelName = QString::fromStdString(model.modelName);
            publicCloudModel.multiAuths  = mc.getModelAuthentications(model.modelName);

            KyInfo() << "loadModelConfig模型名称" <<  QString::fromStdString(model.modelName);
            KyInfo() <<  "loadModelConfig厂商" << QString::fromStdString(model.vendor);
            for (const auto &authen : publicCloudModel.multiAuths) {
                for (const auto &au : authen.authentications) {
                    if (au.key == "appIdContinuous" || au.key == "apiKeyContinuous" || au.key == "secretKeyContinuous") {
                        if (au.value != "") {
                            continuousSelected = true;
                        }
                    } else if (au.key == "appIdRealTime" || au.key == "apiKeyRealTime") {
                        if (au.value != "") {
                            realtimeSelected = true;
                        }
                    }
                }
            }

            switch (capability) {
            case AiCapability::Nlp:
                loadModelItem(publicCloudModel, nlpSelectedModel);
                break;
            case AiCapability::Vision:
                loadModelItem(publicCloudModel, visionSelectedModel);
                break;
            case AiCapability::Speech:
                loadModelItem(publicCloudModel, speechSelectedModel);
                break;
            default:
                break;
            }
        }
    }
}

void CloudModelConfigWidget::processModelInfo(std::vector<ExtendedModel>& models, AiCapability aiCapability) {
    std::vector<ExtendedModel> orderedList;
    std::vector<ExtendedModel> unorderedList;
    // 保存云端模型信息
    QMap<QString, ExtendedModel> unOrderedListMap;
    for (auto& model : models) {
        if (model.deployType == DeployType::PublicCloud && !keyIsEmpty(model.modelName.c_str())) {
            unOrderedListMap[model.modelName.c_str()] = model;
            unorderedList.push_back(model);
        }
    }
    if (unOrderedListMap.size() <= 1) {
        models = unorderedList;
        return;
    }
    QStringList modelNameOrderList = m_modelOrderManager.getModelOrder(aiCapability);
    if (modelNameOrderList.size() != unOrderedListMap.size()) {
        KyWarning() << "The number of models and the saved ones are different."
                    << modelNameOrderList << unOrderedListMap.keys();
        models = unorderedList;
        return;
    }
    // 按照配置文件保存的顺序显示模型
    for (QString& modelName : modelNameOrderList) {
        if (!unOrderedListMap.keys().contains(modelName)) {
            KyWarning() << "The saved list does not contain the name of this model: " << modelName;
            models = unorderedList;
            return;
        }
        orderedList.push_back(unOrderedListMap.value(modelName));
    }
    models = orderedList;
}

bool CloudModelConfigWidget::keyIsEmpty(const QString& modelName) {
    ModelConfig mc;
    std::vector<MultiAuthentication> multiAuths = mc.getModelAuthentications(modelName.toStdString());
    bool isValueEmpty = false;
    bool isContinuousValueEmpty = false;
    bool isRealtimeValueEmpty = false;

    for (const auto &authen : multiAuths) {
        for (const auto &au : authen.authentications) {
            if (au.key == "appIdRealTime" || au.key == "apiKeyRealTime") {
                if (au.value == "") {
                    isContinuousValueEmpty = true;
                }
            } else if (au.key == "appIdContinuous" || au.key == "apiKeyContinuous" || au.key == "secretKeyContinuous") {
                if (au.value == "") {
                    isRealtimeValueEmpty = true;
                }
            } else {
                if (au.value == "") {
                    isValueEmpty = true;
                }
            }
        }
    }

    if (modelName == "讯飞-语音大模型") {
        if (isContinuousValueEmpty && isRealtimeValueEmpty) {
            return true;
        }
    } else {
        if(isValueEmpty) {
           return true;
        }
    }
    return false;
}

void CloudModelConfigWidget::loadFreeTrialModels() {
    ModelConfig mc;
    std::vector<Model> models =
        mc.getFreeTrialModels(kyai::config::model::AiCapability::Nlp);
    QString curModelName =
        mc.getCurrentModelName(AiCapability::Nlp, DeployType::PublicCloud).c_str();

    nlpSelectedModel = curModelName;

    // 定义免费模型的期望顺序
    const QStringList FREE_MODEL_ORDER = {"通义千问（Qwen-Max-试用版）", "深度求索（DeepSeek-R1-试用版）",
                                          "深度求索（DeepSeek-V3-试用版）", "通义千问（QwQ-32B-试用版）",
                                          "通义千问（Qwen-Plus-试用版）"};

    // 将API返回的模型存入map便于快速查找
    QMap<QString, Model> modelMap;
    for (const auto& model : models) {
        QString modelName = QString::fromStdString(model.modelName);
        modelMap[modelName] = model;
    }

    // 按照预定义顺序收集需要显示的模型
    QList<PublicCloudModel> orderedModels;

    // 按固定顺序添加模型
    for (const QString& expectedName : FREE_MODEL_ORDER) {
        if (modelMap.contains(expectedName)) {
            const auto& model = modelMap[expectedName];
            PublicCloudModel publicCloudModel;
            publicCloudModel.modelName = QString::fromStdString(model.modelName);
            publicCloudModel.vendor = QString::fromStdString(model.vendor);
            orderedModels.append(publicCloudModel);
        }
    }

    // 添加不在预定义列表中的其他模型
    for (const auto& model : models) {
        QString modelName = QString::fromStdString(model.modelName);
        if (!FREE_MODEL_ORDER.contains(modelName)) {
            PublicCloudModel publicCloudModel;
            publicCloudModel.modelName = QString::fromStdString(model.modelName);
            publicCloudModel.vendor = QString::fromStdString(model.vendor);
            orderedModels.prepend(publicCloudModel);
        }
    }

    // 按顺序添加到UI
    for (const auto& orderedModel : orderedModels) {
        if (orderedModel.modelName.isEmpty()) {
            KyInfo() << "Free trial account model name is empty !";
            continue;
        }

        loadFreeTrialModelItem(orderedModel, orderedModel.modelName == curModelName);
        KyInfo() << "loadModelConfig模型名称" << orderedModel.modelName;
        KyInfo() << "loadModelConfig厂商" << orderedModel.vendor;
    }
}

void CloudModelConfigWidget::loadFreeTrialModelItem(PublicCloudModel model, bool isSelected) {
    QString modelName = model.modelName;
    QString vendor = model.vendor;
    UkccFrame *itemFrame = new UkccFrame(this);
    itemFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_UkccFrame_itemFrame");
    itemFrame->setMinimumWidth(496);
    itemFrame->setMaximumWidth(496);
    itemFrame->setFixedHeight(56);
    itemFrame->setObjectName(modelName);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setContentsMargins(0, 0, 0, 0);

    ModelItem* item = new ModelItem(AiCapability::Nlp, modelName, vendor, true, itemFrame);
    item->setAccessibleName("kylin-ai-subsystem-modelconfig_ModelItem_item");
    if (isSelected) {
        item->showSelectedIcon(true);
    }
    item->installEventFilter(this);
    connect(item, &ModelItem::sigUpdate, this, &CloudModelConfigWidget::onUpdateModelItem);
    connect(item, &ModelItem::sigUninstall, this, &CloudModelConfigWidget::onUninstallModelItem);
    layItem->addWidget(item);

    // 删除被选中模型后,剩下模型中设置列表第一个为选中
    // 点击免费账号显示隐藏按钮,从列表获取免费模型，设置显示与隐藏
    m_nlpModelItemList.append(item);
    // 点击切换模型时，需要通过m_nlpModelItemMap保存的modelName进行确认
    m_nlpModelItemMap[modelName] = item;

    m_freeTrialModelGroup->addWidget(itemFrame);
}

void CloudModelConfigWidget::loadModelItem(PublicCloudModel model, QString curSelectedName) {
    KyInfo() << "loadModelConfig modelName:" <<  model.modelName;

    UkccFrame *itemFrame = new UkccFrame(this);
    itemFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_UkccFrame_itemFrame");
    itemFrame->setMinimumWidth(496);
    itemFrame->setMaximumWidth(496);
    itemFrame->setFixedHeight(56);
    itemFrame->setObjectName(model.modelName);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setContentsMargins(0, 0, 0, 0);

    ModelItem* item = new ModelItem(model.capability, model.modelName, model.vendor, false, itemFrame);
    item->setAccessibleName("kylin-ai-subsystem-modelconfig_ModelItem_item");
    item->installEventFilter(this);
    connect(item, &ModelItem::sigUpdate, this, &CloudModelConfigWidget::onUpdateModelItem);
    connect(item, &ModelItem::sigUninstall, this, &CloudModelConfigWidget::onUninstallModelItem);

    layItem->addWidget(item);

    if (model.capability == AiCapability::Nlp)
    {
        m_nlpModelItemList.append(item);
        m_nlpModelMap[model.modelName] = model;
        m_nlpModelItemMap[model.modelName] = item;
        m_nlpModelMapUI[model.modelName] = itemFrame;
        m_nlpModelGroup->addWidget(itemFrame);

        // 设置为选中
        if (model.modelName == curSelectedName) {
            item->showSelectedIcon(true);
            nlpSelectedModel = model.modelName;
        }
    }
    else if (model.capability == AiCapability::Vision)
    {
        m_visionModelItemList.append(item);
        m_visionModelMap[model.modelName] = model;
        m_visionModelItemMap[model.modelName] = item;
        m_visionModelMapUI[model.modelName] = itemFrame;
        m_visionModelGroup->addWidget(itemFrame);

        // 设置为选中
        if (model.modelName == curSelectedName) {
            item->showSelectedIcon(true);
            visionSelectedModel = model.modelName;
        }
    }
    else if (model.capability == AiCapability::Speech)
    {
        m_speechModelItemList.append(item);
        m_speechModelMap[model.modelName] = model;
        m_speechModelItemMap[model.modelName] = item;
        m_speechModelMapUI[model.modelName] = itemFrame;
        m_speechModelGroup->addWidget(itemFrame);

        // 设置为选中
        if (model.modelName == curSelectedName) {
            item->showSelectedIcon(true);
            speechSelectedModel = model.modelName;
        }
    }

}

void CloudModelConfigWidget::addModelItem(PublicCloudModel model)
{
    UkccFrame *itemFrame = new UkccFrame(this);
    itemFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_UkccFrame_itemFrame");
    itemFrame->setMinimumWidth(496);
    itemFrame->setMaximumWidth(496);
    itemFrame->setFixedHeight(56);
    itemFrame->setObjectName(model.modelName);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setContentsMargins(0, 0, 0, 0);

    ModelItem* item = new ModelItem(model.capability, model.modelName, model.vendor, false, itemFrame);
    item->setAccessibleName("kylin-ai-subsystem-modelconfig_ModelItem_item");
    item->installEventFilter(this);
    connect(item, &ModelItem::sigUpdate, this, &CloudModelConfigWidget::onUpdateModelItem);
    connect(item, &ModelItem::sigUninstall, this, &CloudModelConfigWidget::onUninstallModelItem);

    layItem->addWidget(item);

    if (model.capability == AiCapability::Nlp)
    {
        m_nlpModelItemList.append(item);
        m_nlpModelMap[model.modelName] = model;
        m_nlpModelItemMap[model.modelName] = item;
        m_nlpModelMapUI[model.modelName] = itemFrame;
        m_nlpModelGroup->addWidget(itemFrame);

        // 第一个添加的模型，设置为选中
        if (m_nlpModelItemMap.count() == 1) {
            item->showSelectedIcon(true);
            nlpSelectedModel = model.modelName;
        }
    }
    else if (model.capability == AiCapability::Vision)
    {
        m_visionModelItemList.append(item);
        m_visionModelMap[model.modelName] = model;
        m_visionModelItemMap[model.modelName] = item;
        m_visionModelMapUI[model.modelName] = itemFrame;
        m_visionModelGroup->addWidget(itemFrame);

        // 第一个添加的模型，设置为选中
        if (m_visionModelItemMap.count() == 1) {
            item->showSelectedIcon(true);
            visionSelectedModel = model.modelName;
        }
    }
    else if (model.capability == AiCapability::Speech)
    {
        m_speechModelItemList.append(item);
        m_speechModelMap[model.modelName] = model;
        m_speechModelItemMap[model.modelName] = item;
        m_speechModelMapUI[model.modelName] = itemFrame;
        m_speechModelGroup->addWidget(itemFrame);

        // 第一个添加的模型，设置为选中
        if (m_speechModelItemMap.count() == 1) {
            item->showSelectedIcon(true);
            speechSelectedModel = model.modelName;
        }
    }
    m_modelOrderManager.addModel(model.capability, model.modelName);
}

void CloudModelConfigWidget::onDeleteModelItem(AiCapability capability, QString modelName) {
    if (getModelStatus(modelName) == ModelStatus::Running) {
        // NLP or VISION or SPEECH 正在使用，重试弹窗
        KyInfo() << "正在使用" << static_cast<int>(capability) << modelName;
        bool canProceed = showModelInUseMessage(this);
        if (canProceed == true) {
            onDeleteModelItem(capability, modelName);
        }
    } else {
        // 未使用，确认弹窗
        bool canProceed = showConfirmMessage(this);
        if (canProceed == true) {
            bool ret = clearModelMultiAuths(capability, modelName);
            if (ret == true) {
                deleteModelItem(capability, modelName);
                m_modelOrderManager.deleteModel(capability, modelName);
                KyInfo() << modelName << " 验证信息clear成功";
            } else {
                showErrorMessage(tr("Failed to delete model"), this);
                KyInfo() << modelName << " 验证信息clear失败, 删除失败";
            }
        }
    }
}

void CloudModelConfigWidget::deleteModelItem(AiCapability capability, QString modelName) {
    ModelConfig mc;
    QString curModelName = QString::fromStdString(mc.getCurrentModelName(capability, DeployType::PublicCloud));
    bool isCheckSelected = checkModelSelection(capability, modelName);
    KyInfo() << "currentmodelname" << curModelName << "要删除的modelname" << modelName << "isCheckSelected"
             << isCheckSelected;

    if (capability == AiCapability::Nlp) {
        // 手动删除 ModelItem 对象
        if (m_nlpModelItemMap.contains(modelName)) {
            if (m_nlpModelItemMap[modelName]) {
                m_nlpModelItemList.removeOne(m_nlpModelItemMap[modelName]);

                delete m_nlpModelItemMap[modelName];
                m_nlpModelItemMap[modelName] = nullptr;
            }
        }
        m_nlpModelMap.remove(modelName);
        m_nlpModelItemMap.remove(modelName);
        m_nlpModelGroup->removeWidget(m_nlpModelMapUI[modelName]);
        m_nlpModelMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if (curModelName == modelName || isCheckSelected) {
            if (m_nlpModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中
                ModelItem* firstItem = m_nlpModelItemList.first();
                QString firstModelName = firstItem->getModelName();

                KyInfo() << "============删除后 nlp firstModelName" << firstModelName;

                bool ret = setSelectCloudModelName(capability, DeployType::PublicCloud, firstModelName);  // set sdk
                if (ret == true) {
                    switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                    KyInfo() << modelName << " 设置为选中模型成功";
                } else {
                    showErrorMessage(tr("Model setup failed"), this);
                    KyInfo() << modelName << " 设置为选中模型失败";
                    return;
                }

            } else {
                // 删除后无模型
                KyInfo() << " nlpModelItemMap为空";
                nlpSelectedModel = "";
            }
        }
    } else if (capability == AiCapability::Vision) {
        // 手动删除 ModelItem 对象
        auto item = m_visionModelItemList.takeAt(m_visionModelItemList.indexOf(m_visionModelItemMap[modelName]));
        if (m_visionModelItemMap.contains(modelName)) {
            if (m_visionModelItemMap[modelName]) {
                delete m_visionModelItemMap[modelName];
                m_visionModelItemMap[modelName] = nullptr;
            }
        }
        m_visionModelMap.remove(modelName);
        m_visionModelItemMap.remove(modelName);
        m_visionModelGroup->removeWidget(m_visionModelMapUI[modelName]);
        m_visionModelMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if (curModelName == modelName || isCheckSelected) {
            if (m_visionModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中
                ModelItem* firstItem = m_visionModelItemList.first();
                QString firstModelName = firstItem->getModelName();

                KyInfo() << "============删除后 vision firstModelName" << firstModelName;

                bool ret = setSelectCloudModelName(capability, DeployType::PublicCloud, firstModelName);  // set sdk
                if (ret == true) {
                    switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                    KyInfo() << modelName << " 设置为选中模型成功";
                } else {
                    showErrorMessage(tr("Model setup failed"), this);
                    KyInfo() << modelName << " 设置为选中模型失败";
                    return;
                }

            } else {
                // 删除后无模型
                KyInfo() << " visionModelItemMap为空";
                visionSelectedModel = "";
            }
        }

    } else if (capability == AiCapability::Speech) {
        // 手动删除 ModelItem 对象
        auto item = m_speechModelItemList.takeAt(m_speechModelItemList.indexOf(m_speechModelItemMap[modelName]));
        if (m_speechModelItemMap.contains(modelName)) {
            if (m_speechModelItemMap[modelName]) {
                delete m_speechModelItemMap[modelName];
                m_speechModelItemMap[modelName] = nullptr;
            }
        }
        m_speechModelMap.remove(modelName);
        m_speechModelItemMap.remove(modelName);
        m_speechModelGroup->removeWidget(m_speechModelMapUI[modelName]);
        m_speechModelMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if (curModelName == modelName || isCheckSelected) {
            if (m_speechModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中
                ModelItem* firstItem = m_speechModelItemList.first();
                QString firstModelName = firstItem->getModelName();

                KyInfo() << "============删除后 speech firstModelName" << firstModelName;

                bool ret = setSelectCloudModelName(capability, DeployType::PublicCloud, firstModelName);  // set sdk
                if (ret == true) {
                    switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                    KyInfo() << modelName << " 设置为选中模型成功";
                } else {
                    showErrorMessage(tr("Model setup failed"), this);
                    KyInfo() << modelName << " 设置为选中模型失败";
                    return;
                }

            } else {
                // 删除后无模型
                KyInfo() << " speechModelItemMap为空";
                speechSelectedModel = "";
            }
        }
    }
}

bool CloudModelConfigWidget::checkModelSelection(AiCapability capability, QString modelName) {
    if (capability == AiCapability::Nlp) {
        if (modelName == nlpSelectedModel) {
            return true;
        } else {
            return false;
        }

    } else if (capability == AiCapability::Vision) {
        if (modelName == visionSelectedModel) {
            return true;
        } else {
            return false;
        }

    } else if (capability == AiCapability::Speech) {
        if (modelName == speechSelectedModel) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool CloudModelConfigWidget::setSelectCloudModelConfig(PublicCloudModel model) {
    KyInfo() << "setSelectCloudModelConfig----" << model.modelName;

    bool isValueEmpty = false;

    for (const auto& authen : model.multiAuths) {
        for (const auto& au : authen.authentications) {
            if (au.key == "appIdRealTime" || au.key == "apiKeyRealTime") {
                if (au.value == "" && realtimeSelected == true) {
                    isValueEmpty = true;
                }
            } else if (au.key == "appIdContinuous" || au.key == "apiKeyContinuous" || au.key == "secretKeyContinuous") {
                if (au.value == "" && continuousSelected == true) {
                    isValueEmpty = true;
                }
            } else {
                if (au.value == "") {
                    isValueEmpty = true;
                }
            }
        }
    }

    if (isValueEmpty == true) {
        KyInfo() << model.modelName << "验证信息为空，设置失败";
        return false;
    }

    ModelConfig mc;
    bool ret1 = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);

    if (ret1 == true) {
        bool ret2 = setSelectCloudModelName(model.capability, DeployType::PublicCloud, model.modelName);
        if (ret2 == false) {
            return false;
        }
    } else {
        KyInfo() << "setModelAuthentications failed:" << ret1;
        return false;
    }
    return true;
}

bool CloudModelConfigWidget::setSelectCloudModelName(AiCapability capability, DeployType deployType,
                                                     QString modelName) {
    KyInfo() << "setSelectCloudModelName" << modelName;
    ModelConfig mc;
    bool ret2 = mc.setCurrentModelName(capability, deployType, modelName.toStdString());

    if (ret2 == true) {
        KyInfo() << "setCurrentModelName success:" << ret2;
    } else {
        KyInfo() << "setCurrentModelName failed:" << ret2;
    }
    return ret2;
}

void CloudModelConfigWidget::switchModelItem(AiCapability capability, QString modelName)
{
    QMap<QString, ModelItem*> *modelItemMap = nullptr;

    if (capability == AiCapability::Nlp) {
        modelItemMap = &m_nlpModelItemMap;

    } else if (capability == AiCapability::Vision) {
        modelItemMap = &m_visionModelItemMap;

    } else if (capability == AiCapability::Speech) {
        modelItemMap = &m_speechModelItemMap;
    }

    if (modelItemMap)
    {
        foreach (const auto& modelItemName, modelItemMap->keys()) {
            ModelItem* modelItem = modelItemMap->value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);

                if (capability == AiCapability::Nlp) {
                    nlpSelectedModel = modelName;

                } else if (capability == AiCapability::Vision) {
                    visionSelectedModel = modelName;

                } else if (capability == AiCapability::Speech) {
                    speechSelectedModel = modelName;
                }

            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
    }
}

bool CloudModelConfigWidget::clearModelMultiAuths(AiCapability capability, QString modelName)
{
    ModelConfig mc;
    bool ret = mc.clearModelAuthentications(modelName.toStdString());

    return ret;

}

/* 通过polkit提权 */
bool CloudModelConfigWidget::polkit() {
    PolkitQt1::Authority::Result result;
    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.kylin-ai-subsystem-modelconfig.policy",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);
    KyInfo() << "PolkitQt1 Authority result:" << result;
    if (result == PolkitQt1::Authority::Yes) { //认证通过
        return true;
    }
    return false;
}

QMap<QString, ModelItem*>& CloudModelConfigWidget::getModelItemMap(AiCapability capability)
{
    if (capability == AiCapability::Nlp) {
        return m_nlpModelItemMap;
    } else if (capability == AiCapability::Vision) {
        return m_visionModelItemMap;
    } else if (capability == AiCapability::Speech) {
        return m_speechModelItemMap;
    }
}

ModelStatus CloudModelConfigWidget::getModelStatus(QString modelName)
{
    ModelConfig mc;
    ModelStatus ret = mc.getModelStatus(modelName.toStdString());
    KyInfo() << QString("%1 ModelStatus = %2").arg(modelName).arg((qint32)ret);
    return ret;
}

void CloudModelConfigWidget::showCentered()
{
   if (QWidget* mainWindow = QApplication::activeWindow())
   {

       QRect mainGeom = mainWindow->geometry();
       move(mainGeom.x() + (mainGeom.width() - width()) / 2,
       mainGeom.y() + (mainGeom.height() - height()) / 2+ 15);
   }
   adjustPosition(this); // 调用内部调整逻辑
}

void CloudModelConfigWidget::openSoftwareStore() {
    QString program = "/usr/bin/kylin-software-center";
    QStringList arguments;
    arguments << "-menuid=2" << "-tagid=712";
    QProcess::startDetached(program, arguments);
}

void CloudModelConfigWidget::onUpdateModelItem(AiCapability capability, QString modelName) {
    KyInfo() << QString("Update AiCapability=%0, ModelName=%1").arg((qint32)capability).arg(modelName);

    if (getModelStatus(modelName) == ModelStatus::Running) {
        // 模型正在使用，显示重试弹窗
        KyInfo() << "模型正在使用" << static_cast<int>(capability) << modelName;
        bool canProceed = showModelInUseMessage(this);
        if (canProceed == true) {
            onUpdateModelItem(capability, modelName);
        } else {
            KyInfo() << "取消更新操作";
            return;
        }
    } else {
        // 模型未在使用，跳转到商店
        openSoftwareStore();
    }
}

void CloudModelConfigWidget::onUninstallModelItem(AiCapability capability, QString modelName) {
    KyInfo() << QString("Uninstall AiCapability=%0, ModelName=%1").arg((qint32)capability).arg(modelName);

    if (getModelStatus(modelName) == ModelStatus::Running) {
        // 模型正在使用，显示重试弹窗
        KyInfo() << "模型正在使用" << static_cast<int>(capability) << modelName;
        bool canProceed = showModelInUseMessage(this);
        if (canProceed == true) {
            onUninstallModelItem(capability, modelName);
        } else {
            KyInfo() << "取消卸载操作";
            return;
        }
    } else {
        // 模型未在使用，跳转到商店
        openSoftwareStore();
    }
}
