#include "localmodelconfigwidget.h"

LocalModelConfigWidget::LocalModelConfigWidget(QWidget *parent) : QDialog(parent)
{
    getCurSystemVersion();
    //初始化布局
    initLayout();
    //初始化本地模型能力
    loadNlpModelConfig();
    loadOtherModelConfig();
    //初始化信号槽
    initConnect();

    // 设置主题监听
    setupThemeListener();
}

void LocalModelConfigWidget::initLayout()
{
    this->setFixedSize(540,610);
    this->setWindowTitle(tr("Local Model Config"));

    m_mainVLayout = new QVBoxLayout();

    initModelTitleLayout();
    initModellayout();

    m_mainVLayout->setStretch(0,2);
    m_mainVLayout->setStretch(1,8);
    this->setLayout(m_mainVLayout);
}

void LocalModelConfigWidget::initModelTitleLayout() {
    QLabel* backgroundLabel = new QLabel(this);
    backgroundLabel->setFixedHeight(72);
    backgroundLabel->setPixmap(QPixmap(":/res/icon/background.svg"));
    backgroundLabel->setAlignment(Qt::AlignCenter);
    backgroundLabel->setScaledContents(false);
    backgroundLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_backgroundLabel");

    // 创建水平布局
    m_titleHLayout = new QHBoxLayout(backgroundLabel);
    m_titleHLayout->setContentsMargins(24, 0, 24, 0);

    // 左侧图标布局
    m_vLeftLayout = new QVBoxLayout();
    m_iconLabel = new QLabel(backgroundLabel);
    m_iconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_iconLabel");
    QPixmap iconPixmap(":/res/icon/ukui-local-model-symbolic.svg"); // 替换成本地模型图标
    if(!iconPixmap.isNull())
    {
        iconPixmap = iconPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_iconLabel->setPixmap(iconPixmap);
    }
    m_iconLabel->setFixedSize(24, 24);
    m_vLeftLayout->addWidget(m_iconLabel);

    // 右侧文本布局
    m_localHLayout = new QVBoxLayout();
    m_localHLayout->setContentsMargins(0, 12, 0, 16);

    m_localLabel = new QLabel(backgroundLabel);
    m_localLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_localLabel");
    m_localLabel->setMinimumSize(216, 24);
    m_localLabel->setText(tr("Local Model"));
    QFont localFont;
    localFont.setBold(true);
    m_localLabel->setFont(localFont);
    m_localLabel->setStyleSheet("color: #3790FA; background: transparent;");

    m_descripLabel = new kdk::KLabel(backgroundLabel);
    m_descripLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_descripLabel");
    m_descripLabel->setMinimumSize(450, 20);
    m_descripLabel->setText(
        tr("Configure a locally deployed model that has passed Kylin certification or is selected by the user "
           "(Installed on local equipment, it can run without network)"));
    QFont localDescripFont;
    localDescripFont.setPointSize(localFont.pointSize() - 2);
    m_descripLabel->setFont(localDescripFont);
    setKLabelFontColor(m_descripLabel, "#3790FA");

    m_localHLayout->addWidget(m_localLabel);
    m_localHLayout->addWidget(m_descripLabel);

    // 组合布局
    m_titleHLayout->addLayout(m_vLeftLayout);
    m_titleHLayout->addLayout(m_localHLayout);

    // 添加到主垂直布局
    m_mainVLayout->addWidget(backgroundLabel);
}

void LocalModelConfigWidget::getCurSystemVersion() {
    m_curSystemVersion = curSystemVersion();
}

void LocalModelConfigWidget::initOtherModelLayout() {
    // 系统内置模型 图像识别、向量化、语音处理等其他系统运行所需的模型
    m_otherVLayout = new QVBoxLayout();
    m_otherVLayout->setContentsMargins(0, 0, 0, 0);
    m_otherDescVLayout = new QVBoxLayout();
    m_otherDescVLayout->setSpacing(0);
    m_otherDescVLayout->setContentsMargins(16, 0, 16, 0);

    // 第一个新增静态模型组的标题和描述
    m_otherVisionLabel = new QLabel(m_nlpWidget);
    m_otherVisionLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_additionalLabel1");
    m_otherVisionLabel->setText(tr("Visual Model - Visual Perception Category"));

    // 添加图标到文字旁边
    otherVisionIconLabel = new QLabel(m_nlpWidget);
    otherVisionIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    otherVisionIconLabel->setProperty("useIconHighlightEffect", 0x2);
    otherVisionIconLabel->setToolTip(
        tr("·Analyze image content and extract key information, such as text recognition, object detection, and region "
           "segmentation.\n·Built in model of the system cannot be turned off or disabled"));

    // 第二个新增静态模型组的标题和描述
    m_otherSpeechLabel = new QLabel(m_nlpWidget);
    m_otherSpeechLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_additionalLabel2");
    m_otherSpeechLabel->setText(tr("Speech Model"));

    // 添加图标到文字旁边
    otherSpeechIconLabel = new QLabel(m_nlpWidget);
    otherSpeechIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    otherSpeechIconLabel->setProperty("useIconHighlightEffect", 0x2);
    otherSpeechIconLabel->setToolTip(tr("Process sound and speech, such as speech recognition and speech synthesis."));

    // 原有系统内置模型的标题和描述
    m_otherLabel = new QLabel(m_nlpWidget);
    m_otherLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_otherLabel");
    m_otherLabel->setText(tr("Other built in model of the system"));

    // 添加图标到文字旁边
    otherIconLabel = new QLabel(m_nlpWidget);
    otherIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    otherIconLabel->setProperty("useIconHighlightEffect", 0x2);
    otherIconLabel->setToolTip(
        tr("·Models required for the operation of other systems such as text vectorization and image "
           "vectorization\n·Built in model of the system cannot be turned off or disabled"));
}
void LocalModelConfigWidget::initModellayout()
{
    initOtherModelLayout();
    m_baseVLayout = new QVBoxLayout(this);
    m_baseVLayout->setContentsMargins(0, 0, 0, 0);

    // NLP大语言模型
    m_nlpWidget = new QWidget(this);
    m_nlpWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_nlpWidget");
    m_nlpWidget->setLayout(m_baseVLayout);

    m_nlpVLayout = new QVBoxLayout;
    m_nlpVLayout->setContentsMargins(0, 0, 0, 0);

    // NLP标题
    m_nlpLabel = new QLabel(m_nlpWidget);
    m_nlpLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_nlpLabel");
    m_nlpLabel->setText(tr("NLP Model"));

    // 添加图标到NLP标签旁边
    nlpIconLabel = new QLabel(m_nlpWidget);
    nlpIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    nlpIconLabel->setProperty("useIconHighlightEffect", 0x2);
    nlpIconLabel->setToolTip(
        tr("Process and generate text, such as understanding text and "
           "answering questions"));

    m_nlpWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_nlpWidget");
    m_nlpWidget->setLayout(m_baseVLayout);

    m_nlpVLayout = new QVBoxLayout;
    m_nlpVLayout->setContentsMargins(0, 0, 0, 0);

    // NLP大语言模型的水平布局（与系统内置模型对齐）
    m_nlpHLayout = new QHBoxLayout;
    m_nlpHLayout->setContentsMargins(16, 0, 16, 0);
    m_nlpHLayout->setSpacing(4);

    // 直接将标签和图标添加到水平布局中
    m_nlpHLayout->addWidget(m_nlpLabel);
    m_nlpHLayout->addWidget(nlpIconLabel);  // 添加图标
    m_nlpHLayout->addStretch();

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
    m_nlpAddItemButton->setMaximumHeight(56);
    m_nlpAddItemButton->setMinimumHeight(56);
    m_nlpAddItemButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_nlpAddItemButton");

    // 已配置模型
    m_nlpLocalModelGroup = new SettingGroup(m_nlpWidget);
    m_nlpLocalModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_nlpLocalModelGroup");

    m_nlpHLayout->addWidget(m_nlpAddButton);

    m_nlpVLayout->addLayout(m_nlpHLayout);
    m_nlpVLayout->addWidget(m_nlpAddItemButton);
    m_nlpVLayout->addWidget(m_nlpLocalModelGroup);
    m_baseVLayout->addLayout(m_nlpVLayout);

    m_otherModelGroup = new SettingGroup(m_nlpWidget);
    m_otherModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_otherModelGroup");

    m_otherVisionModelGroup = new SettingGroup(m_nlpWidget);
    m_otherSpeechModelGroup = new SettingGroup(m_nlpWidget);

    // 内置图像类模型组水平布局
    QHBoxLayout* m_otherVisionHLayout = new QHBoxLayout;
    m_otherVisionHLayout->setContentsMargins(16, 24, 16, 0);
    m_otherVisionHLayout->setSpacing(4);

    // 直接将标签和图标添加到水平布局中
    m_otherVisionHLayout->addWidget(m_otherVisionLabel);
    m_otherVisionHLayout->addWidget(otherVisionIconLabel);
    m_otherVisionHLayout->addStretch();

    m_otherVLayout->addLayout(m_otherVisionHLayout);
    m_otherVLayout->addWidget(m_otherVisionModelGroup);

    // 内置语音类模型组的水平布局
    QHBoxLayout* m_otherSpeechHLayout = new QHBoxLayout;
    m_otherSpeechHLayout->setContentsMargins(16, 24, 16, 0);
    m_otherSpeechHLayout->setSpacing(4);

    // 直接将标签和图标添加到水平布局中
    m_otherSpeechHLayout->addWidget(m_otherSpeechLabel);
    m_otherSpeechHLayout->addWidget(otherSpeechIconLabel);
    m_otherSpeechHLayout->addStretch();

    m_otherVLayout->addLayout(m_otherSpeechHLayout);
    m_otherVLayout->addWidget(m_otherSpeechModelGroup);

    // 原有系统内置模型的水平布局
    QHBoxLayout* m_otherHLayout = new QHBoxLayout;
    m_otherHLayout->setContentsMargins(16, 24, 16, 0);
    m_otherHLayout->setSpacing(4);

    // 直接将标签和图标添加到水平布局中
    m_otherHLayout->addWidget(m_otherLabel);
    m_otherHLayout->addWidget(otherIconLabel);
    m_otherHLayout->addStretch();

    m_otherVLayout->addLayout(m_otherHLayout);
    m_otherVLayout->addWidget(m_otherModelGroup);

    m_baseVLayout->addLayout(m_otherVLayout);

    // VISION视觉模型
    m_visionWidget = new QWidget;
    m_visionWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_visionWidget");
    m_visionVLayout = new QVBoxLayout;
    m_visionVLayout->setContentsMargins(0, 0, 0, 0);
    m_visionHLayout = new QHBoxLayout;
    m_visionHLayout->setContentsMargins(16, 0, 16, 0);

    // VISION标题
    m_visionLabel = new QLabel;
    m_visionLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_visionLabel");
    // 标题和描述放入布局
    m_visionDescVLayout  = new QVBoxLayout();
    m_visionDescVLayout->addWidget(m_visionLabel);
    m_visionDescVLayout->addWidget(m_visionDescrLabel);

    // 已配置模型
    m_visionLocalModelGroup = new SettingGroup;
    m_visionLocalModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_visionLocalModelGroup");
    m_visionHLayout->addLayout(m_visionDescVLayout);
    m_visionHLayout->addStretch();

    m_visionVLayout->addLayout(m_visionHLayout);
    m_visionVLayout->addWidget(m_visionLocalModelGroup);

    m_visionWidget->setLayout(m_visionVLayout);

    // SPEECH语音模型
    m_speechWidget = new QWidget;
    m_speechWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_speechWidget");
    m_speechVLayout = new QVBoxLayout;
    m_speechVLayout->setContentsMargins(0, 0, 0, 0);
    m_speechHLayout = new QHBoxLayout;
    m_speechHLayout->setContentsMargins(16, 0, 16, 0);

    // SPEECH标题
    m_speechLabel = new QLabel;
    m_speechLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_speechLabel");

    // 标题和描述放入布局
    m_speechDescVlayout  = new QVBoxLayout();
    m_speechDescVlayout->addWidget(m_speechLabel);
    m_speechDescVlayout->addWidget(m_speechDescLabel);

    // 已配置模型
    m_speechLocalModelGroup = new SettingGroup;
    m_speechLocalModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_speechLocalModelGroup");
    m_speechHLayout->addLayout(m_speechDescVlayout);
    m_speechHLayout->addStretch();

    m_speechVLayout->addLayout(m_speechHLayout);
    m_speechVLayout->addWidget(m_speechLocalModelGroup);

    m_speechWidget->setLayout(m_speechVLayout);

    // 滚动区域
    m_scrollVLayout = new QVBoxLayout;
    m_scrollVLayout->setContentsMargins(16, 20, 16, 0);
    m_scrollVLayout->addWidget(m_nlpWidget);
    m_scrollVLayout->addWidget(m_visionWidget);
    m_scrollVLayout->addWidget(m_speechWidget);
    m_scrollVLayout->addStretch();

    m_scrollWidget = new QWidget;
    m_scrollWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_scrollWidget");
    m_scrollWidget->setContentsMargins(0, 0, 0, 0);
    m_scrollWidget->setLayout(m_scrollVLayout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setAccessibleName("kylin-ai-subsystem-modelconfig_QScrollArea_m_scrollArea");
    m_scrollArea->setFixedHeight(420);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::NoFrame); // 无边框
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 不显示水平滚动条
    m_scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);

    m_scrollArea->setWidget(m_scrollWidget);
    m_mainVLayout->addWidget(m_scrollArea);

    m_confirmLocalButton = new QPushButton(tr("OK"), this);
    m_confirmLocalButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_confirmLocalButton");
    m_confirmLocalButton->setProperty("isImportant", true);
    m_confirmLocalButtoLayout = new QHBoxLayout();
    m_confirmLocalButtoLayout->setContentsMargins(0, 0, 24, 24);
    m_confirmLocalButtoLayout->addStretch();
    m_confirmLocalButtoLayout->addWidget(m_confirmLocalButton);
    m_mainVLayout->addStretch();
    m_mainVLayout->addLayout(m_confirmLocalButtoLayout);
}

void LocalModelConfigWidget::onUpdateModelItem(AiCapability capability, QString modelName) {
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

void LocalModelConfigWidget::onUninstallModelItem(AiCapability capability, QString modelName) {
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
        // 共享服务打开，如果要卸载本地模型，提示服务会被关闭, 是否要卸载
        if (capability == AiCapability::Nlp && m_configManager.getEnableSharedComputePower()
                && !confirmUnloadLocalModel()) {
            return;
        }
        emit modelUninstall(modelName);
        // 模型未在使用，跳转到商店
        openSoftwareStore();
    }
}

ModelStatus LocalModelConfigWidget::getModelStatus(QString modelName)
{
    ModelConfig mc;
    ModelStatus ret = mc.getModelStatus(modelName.toStdString());
    KyInfo() << QString("ModelStatus = %0").arg((qint32)ret);
    return ret;
}

bool LocalModelConfigWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 模型配置
    LocalModelItem *clickedModelItem = qobject_cast<LocalModelItem*>(watched);

    if (clickedModelItem) {

        if (event->type() == QEvent::MouseButtonRelease) {

            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::LeftButton) { // 判断左键
               // 切换模型
                clickLocalModel(clickedModelItem->getAICapability(), clickedModelItem->getModelName());
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched,event);
}

void LocalModelConfigWidget::closeEvent(QCloseEvent *event)
{
    bool isdownloading = ModelDownload::getInstance()->getDownloadingStatus();
    KyInfo() << " close event -----1----:current model is downloading :" << isdownloading;
    //检查是否正在初始化下载阶段
    bool isStartDownloading = ModelDownload::getInstance()->getStartDownloadingStatus();
    KyInfo() << " close event -----2----:current model is start downloading:" << isStartDownloading;
    //判断当前是否正在安装中，正在安装中则提示用户是否退出
    bool isInstalling = ModelDownload::getInstance()->getCurrentInstallStatus();
    KyInfo() << " close event -----3----:current  model is installing:" << isInstalling;

    //非正在下载状态则直接关闭界面
    if(isdownloading == false && isStartDownloading == false && isInstalling == false){
        event->accept();
        return;
    }else {
        //正在初始化安装中
        if(isStartDownloading)
        {
            //正在初始化下载过程中弹出二次确认窗口
            QMessageBox *msgBox = new QMessageBox(this);
            msgBox->setIcon(QMessageBox::Question);
            // 标题加粗并为黑色字体（使用 HTML）
            QString title = "<b><font color='black'>" + tr("Are you sure you want to exit?") + "</font></b>";
            msgBox->setText(title);
            // 说明文字也设为黑色
            QString infoText = "<font color='black'>" + tr("The model is currently being installed, exiting may result in installation failure. Are you sure you want to exit?") + "</font>";
            msgBox->setInformativeText(infoText);
            QPushButton *cancelButton = msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
            QPushButton *quitButton = msgBox->addButton(tr("Quit"), QMessageBox::AcceptRole);
            connect(msgBox,&QMessageBox::finished,this,[=](){
               msgBox->close();
               event->ignore();
               return;
            });
            connect(quitButton,&QPushButton::clicked,this,[=](){
                bool cancelFlag = ModelDownload::getInstance()->cancelDownloading();
                if(cancelFlag)
                {
                    event->accept();
                    this->hide();
                    return;
                }
                else{
                    event->ignore();
                    return;
                }
            });
            connect(cancelButton,&QPushButton::clicked,this,[=](){
               event->ignore();
               return;
            });
            msgBox->setDefaultButton(cancelButton);
            msgBox->exec();
            }
        //正在安装中
        else if(isInstalling)
        {
            QMessageBox *msgBox = new QMessageBox(this);
            msgBox->setIcon(QMessageBox::Question);
            // 标题加粗并为黑色字体（使用 HTML）
            QString title = "<b><font color='black'>" + tr("Exit current window?") + "</font></b>";
            msgBox->setText(title);
            QString infoText = "<font color='black'>" + tr("The model installation will continue to run in the background. A restart is required for the changes to take effect once completed.") + "</font>";
            msgBox->setInformativeText(infoText);

            QPushButton *cancelButton = msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
            QPushButton *quitButton = msgBox->addButton(tr("Quit"), QMessageBox::AcceptRole);
            connect(msgBox,&QMessageBox::finished,this,[=](){
                msgBox->close();
                event->ignore();
                return;
            });
            connect(quitButton,&QPushButton::clicked,this,[=](){
                event->accept();
                this->hide();
                return;
            });
            connect(cancelButton,&QPushButton::clicked,this,[=](){
               event->ignore();
               return;
            });
            msgBox->setDefaultButton(quitButton);
            msgBox->exec();

        }
        else if(isdownloading) {
        //正在下载过程中则弹出二次确认窗口
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Question);
        // 标题加粗并为黑色字体（使用 HTML）
        QString title = "<b><font color='black'>" + tr("Are you sure you want to exit?") + "</font></b>";
        msgBox->setText(title);
        // 说明文字也设为黑色
        QString infoText = "<font color='black'>" + tr("The model is currently being installed, exiting may result in installation failure. Are you sure you want to exit?") + "</font>";
        msgBox->setInformativeText(infoText);
        QPushButton *cancelButton = msgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
        QPushButton *quitButton = msgBox->addButton(tr("Quit"), QMessageBox::AcceptRole);
        connect(msgBox,&QMessageBox::finished,this,[=](){
           msgBox->close();
           event->ignore();
           return;
        });
        connect(quitButton,&QPushButton::clicked,this,[=](){
            bool cancelFlag = ModelDownload::getInstance()->cancelDownloading();
            if(cancelFlag)
            {
                event->accept();
                this->hide();
                return;
            }
            else{
                event->ignore();
                return;
            }
        });
        connect(cancelButton,&QPushButton::clicked,this,[=](){
           event->ignore();
           return;
        });
        msgBox->setDefaultButton(cancelButton);
        msgBox->exec();
        }

        }
}

void LocalModelConfigWidget::loadNlpModelConfig() {
    ModelConfig mc;
    bool isExistLocalModel = false;
    const auto models = mc.getExtendedModels(AiCapability::Nlp);

    // 获取当前记录的添加顺序
    QStringList currentOrder = m_configManager.getLocalNlpModelAddOrder();

    // 提取有效的本地模型名集合
    QSet<QString> validModelNames;
    QMap<QString, decltype(models)::value_type> modelDataMap;

    for (const auto& model : models) {
        if (model.deployType != DeployType::Ondevice || model.is3rdModel) {
            continue;
        }

        QString modelName = QString::fromStdString(model.modelName);
        validModelNames.insert(modelName);
        modelDataMap[modelName] = model;
    }

    // 清理已不存在的模型，保留仍存在的模型的顺序
    QStringList cleanedOrder;
    for (const QString& modelName : currentOrder) {
        if (validModelNames.contains(modelName)) {
            cleanedOrder.append(modelName);
        }
    }

    // 检查是否有新模型添加
    for (const auto &model : models) {
        if (model.deployType != DeployType::Ondevice || model.is3rdModel) {
            continue;
        }

        QString modelName = QString::fromStdString(model.modelName);
        if (!cleanedOrder.contains(modelName)) {
            // 新模型添加到顺序列表的开头
            cleanedOrder.prepend(modelName);
        }
    }

    // 保存更新后的顺序
    if (cleanedOrder != currentOrder) {
        m_configManager.setLocalNlpModelAddOrder(cleanedOrder);
    }

    // 清空现有UI组件 - 通过删除子部件的方式
    QLayout* layout = m_nlpLocalModelGroup->layout();
    if (layout) {
        // 删除布局中的所有子项
        while (layout->count() > 0) {
            QLayoutItem* item = layout->takeAt(0);
            delete item->widget();  // 删除部件
            delete item;            // 删除布局项
        }
    }

    m_nlpLocalModelItemMap.clear();  // 清空映射关系

    std::string curNlpModelName =
        mc.getCurrentModelName(AiCapability::Nlp, DeployType::Ondevice);
    nlpSelectedLocalModel = QString::fromStdString(curNlpModelName);

    for (const QString& orderedModelName : cleanedOrder) {
        auto modelData = modelDataMap[orderedModelName];

        UkccFrame *itemFrame = new UkccFrame(this);
        itemFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_UkccFrame_itemFrame");
        itemFrame->setMaximumWidth(496);
        itemFrame->setFixedHeight(56);
        itemFrame->installEventFilter(this);
        QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
        layItem->setContentsMargins(0, 0, 0, 0);

        PublicLocalModel publicLocalModel;
        publicLocalModel.capability = modelData.capability;
        publicLocalModel.vendor = QString::fromStdString(modelData.vendor);
        publicLocalModel.modelName =
            QString::fromStdString(modelData.modelName);
        publicLocalModel.multiAuths =
            mc.getModelAuthentications(modelData.modelName);
        publicLocalModel.debName = QString::fromStdString(modelData.debName);
        publicLocalModel.available = modelData.available;

        KyInfo() << "loadModelConfig模型名称"
                 << QString::fromStdString(modelData.modelName);
        KyInfo() << "loadModelConfig厂商"
                 << QString::fromStdString(modelData.vendor);
        KyInfo() << "模型可用性" << modelData.available;

        LocalModelItem *item = new LocalModelItem(publicLocalModel, m_curSystemVersion, itemFrame);
        item->setAccessibleName("kylin-ai-subsystem-modelconfig_LocalModelItem_item");
        item->installEventFilter(this);

        // 连接信号和槽
        connect(item, &LocalModelItem::sigUpdate, this, &LocalModelConfigWidget::onUpdateModelItem);
        connect(item, &LocalModelItem::sigUninstall, this, &LocalModelConfigWidget::onUninstallModelItem);

        // 设置模型项是否置灰
        item->setGrayedOut(publicLocalModel.available);

        layItem->addWidget(item);
        isExistLocalModel = true;

        m_nlpLocalModelGroup->addWidget(itemFrame);
        m_nlpLocalModelItemMap[publicLocalModel.modelName] = item;

        if (modelData.modelName == curNlpModelName) {
            item->showSelectedIcon(true);
        }
    }

    // 更新NLP模型添加按钮的可见性
    updateNlpAddButtonVisibility();
}

void LocalModelConfigWidget::loadOtherModelConfig() {
    // 内置图像模型组
    m_otherVisionItemFrame = new UkccFrame(this);
    m_otherVisionItemFrame->setMaximumWidth(496);
    m_otherVisionItemFrame->setFixedHeight(56);
    QHBoxLayout* otherVisionLayItem = new QHBoxLayout(m_otherVisionItemFrame);
    otherVisionLayItem->setContentsMargins(16, 0, 0, 0);

    // 创建图标标签
    m_otherVisionIconLabel = new QLabel(m_otherVisionItemFrame);
    updateSystemModelIcon(m_otherVisionIconLabel, ":/res/icon/system-model.svg",
                          ":/res/icon/system-model-light.svg");
    m_otherVisionIconLabel->setFixedSize(16, 16);

    m_otherVisionFrameLabelText = new QLabel(m_otherVisionItemFrame);
    m_otherVisionFrameLabelText->setText(
        tr("The system is equipped with built-in OCR, image clipping, and segmentation models"));

    otherVisionLayItem->addWidget(m_otherVisionIconLabel);       // 先添加图标
    otherVisionLayItem->addWidget(m_otherVisionFrameLabelText);  // 再添加文本
    m_otherVisionModelGroup->addWidget(m_otherVisionItemFrame);

    // 内置语音模型组
    m_otherSpeechItemFrame = new UkccFrame(this);
    m_otherSpeechItemFrame->setMaximumWidth(496);
    m_otherSpeechItemFrame->setFixedHeight(56);
    QHBoxLayout* otherSpeechLayItem = new QHBoxLayout(m_otherSpeechItemFrame);
    otherSpeechLayItem->setContentsMargins(16, 0, 0, 0);

    // 创建图标标签
    m_otherSpeechIconLabel = new QLabel(m_otherSpeechItemFrame);
    updateSystemModelIcon(m_otherSpeechIconLabel, ":/res/icon/system-model.svg",
                          ":/res/icon/system-model-light.svg");
    m_otherSpeechIconLabel->setFixedSize(16, 16);

    m_otherSpeechFrameLabelText = new QLabel(m_otherSpeechItemFrame);
    m_otherSpeechFrameLabelText->setText(tr("Built in speech model in the system"));

    otherSpeechLayItem->addWidget(m_otherSpeechIconLabel);       // 先添加图标
    otherSpeechLayItem->addWidget(m_otherSpeechFrameLabelText);  // 再添加文本
    m_otherSpeechModelGroup->addWidget(m_otherSpeechItemFrame);

    // 其他系统内置模型组
    UkccFrame *itemFrame = new UkccFrame(this);
    itemFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_UkccFrame_itemFrame");
    itemFrame->setMaximumWidth(496);
    itemFrame->setFixedHeight(56);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setContentsMargins(16, 0, 0, 0);

    // 创建图标标签
    m_otherIconLabel = new QLabel(itemFrame);
    updateSystemModelIcon(m_otherIconLabel, ":/res/icon/system-model.svg",
                          ":/res/icon/system-model-light.svg");
    m_otherIconLabel->setFixedSize(16, 16);

    m_otherFrameLabelText = new QLabel(itemFrame);
    m_otherFrameLabelText->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_frameLabel");
    m_otherFrameLabelText->setText(tr("Built in model group"));

    layItem->addWidget(m_otherIconLabel);       // 先添加图标
    layItem->addWidget(m_otherFrameLabelText);  // 再添加文本
    m_otherModelGroup->addWidget(itemFrame);
}

void LocalModelConfigWidget::initConnect()
{
    connect(m_confirmLocalButton, &QPushButton::clicked, this, [this](){
        this->close();
    });

    if (m_installButton) {
        connect(m_installButton, &QPushButton::clicked, this, &LocalModelConfigWidget::openSoftwareStore);
    }
    if (m_updateOrUninstallBtn) {
        connect(m_updateOrUninstallBtn, &QPushButton::clicked, this, &LocalModelConfigWidget::openSoftwareStore);
    }

    if (m_nlpAddButton) {
        connect(m_nlpAddButton, &kdk::KBorderlessButton::clicked, this, [this]() { this->openSoftwareStore(); });
    }
    if (m_nlpAddItemButton) {
        connect(m_nlpAddItemButton, &QPushButton::clicked, this, [this]() { this->openSoftwareStore(); });
    }
}

// 更新NLP模型添加按钮的可见性
void LocalModelConfigWidget::updateNlpAddButtonVisibility() {
    bool hasNlpModels = !m_nlpLocalModelItemMap.isEmpty();

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

void LocalModelConfigWidget::openSoftwareStore()
{
    //打开软件商店
    QString cmd = QString("/usr/bin/kylin-software-center -menuid=2 -tagid=712");
    QProcess::startDetached(cmd);
}

void LocalModelConfigWidget::deleteModelItem(AiCapability capability, QString modelName)
{
    ModelConfig mc;
    QString curModelName = QString::fromStdString(mc.getCurrentModelName(capability,DeployType::Ondevice));
    bool isCheckSelected = checkModelSelection(capability, modelName);
    //KyInfo() << "currentmodelname" << curModelName << "要删除的modelname" << modelName << "isCheckSelected" << isCheckSelected;

    if (capability == AiCapability::Nlp)
    {
        m_nlpLocalModelMap.remove(modelName);
        m_nlpLocalModelItemMap.remove(modelName);
        m_nlpLocalModelGroup->removeWidget(m_nlpLocalModelMapUI[modelName]);
        m_nlpLocalModelMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if(curModelName == modelName || isCheckSelected)
        {
            if (m_nlpLocalModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中
                auto remainingModelItems = m_nlpLocalModelItemMap.values();
                QString firstModelName = remainingModelItems.first()->getModelName();

                bool ret = setSelectLocalModelName(capability, DeployType::Ondevice, firstModelName); // set sdk
                if (ret == true) {
                    switchModelItem(remainingModelItems.first()->getAICapability(), remainingModelItems.first()->getModelName());
                    KyInfo() << modelName << " 设置为选中模型成功";
                } else {
                    showErrorMessage(tr("Model setup failed"), this);
                    KyInfo() << modelName << " 设置为选中模型失败";
                    return;
                }

            } else {
                // 删除后无模型
                KyInfo() << " nlpModelItemMap为空";
                nlpSelectedLocalModel = "";
            }
        }
    }
    else if (capability == AiCapability::Vision)
    {
        m_visionLocalModelMap.remove(modelName);
        m_visionLocalModelItemMap.remove(modelName);
        m_visionLocalModelGroup->removeWidget(m_visionLocalModelMapUI[modelName]);
        m_visionLocalModelMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if(curModelName == modelName || isCheckSelected)
        {
            if (m_visionLocalModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中
                auto remainingModelItems = m_visionLocalModelItemMap.values();
                QString firstModelName = remainingModelItems.first()->getModelName();

                bool ret = setSelectLocalModelName(capability, DeployType::Ondevice, firstModelName); // set sdk
                if (ret == true) {
                    switchModelItem(remainingModelItems.first()->getAICapability(), remainingModelItems.first()->getModelName());
                    KyInfo() << modelName << " 设置为选中模型成功";
                } else {
                    showErrorMessage(tr("Model setup failed"), this);
                    KyInfo() << modelName << " 设置为选中模型失败";
                    return;
                }

            } else {
                // 删除后无模型
                KyInfo() << " visionModelItemMap为空";
                visionSelectedLocalModel = "";
            }
        }

    }
    else if (capability == AiCapability::Speech)
    {
        m_speechLocalModelMap.remove(modelName);
        m_speechLocalModelItemMap.remove(modelName);
        m_speechLocalModelGroup->removeWidget(m_speechLocalModelMapUI[modelName]);
        m_speechLocalModelMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if(curModelName == modelName || isCheckSelected)
        {
            if (m_speechLocalModelItemMap.count() > 0) {

                // 删除后，设置剩下模型中的第一个为选中
                auto remainingModelItems = m_speechLocalModelItemMap.values();
                QString firstModelName = remainingModelItems.first()->getModelName();

                bool ret = setSelectLocalModelName(capability, DeployType::Ondevice, firstModelName); // set sdk
                if (ret == true) {
                    switchModelItem(remainingModelItems.first()->getAICapability(), remainingModelItems.first()->getModelName());
                    KyInfo() << modelName << " 设置为选中模型成功";
                } else {
                    showErrorMessage(tr("Model setup failed"), this);
                    KyInfo() << modelName << " 设置为选中模型失败";
                    return;
                }

            } else {
                // 删除后无模型
                KyInfo() << " speechModelItemMap为空";
                speechSelectedLocalModel = "";
            }
        }
    }
}

void LocalModelConfigWidget::switchModelItem(AiCapability capability, QString modelName)
{
    QMap<QString, LocalModelItem*> *localModelItemMap = nullptr;

    if (capability == AiCapability::Nlp) {
        localModelItemMap = &m_nlpLocalModelItemMap;
    } else if (capability == AiCapability::Vision) {
        localModelItemMap = &m_visionLocalModelItemMap;

    } else if (capability == AiCapability::Speech) {
        localModelItemMap = &m_speechLocalModelItemMap;
    }

    if (localModelItemMap)
    {
        foreach (const auto& modelItemName, localModelItemMap->keys()) {
            LocalModelItem* modelItem = localModelItemMap->value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);

                if (capability == AiCapability::Nlp) {
                    nlpSelectedLocalModel = modelName;

                } else if (capability == AiCapability::Vision) {
                    visionSelectedLocalModel = modelName;

                } else if (capability == AiCapability::Speech) {
                    speechSelectedLocalModel = modelName;
                }

            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
    }
}

bool LocalModelConfigWidget::setSelectLocalModelName(AiCapability capability, DeployType deployType, QString modelName)
{

    ModelConfig mc;
    bool ret2 = mc.setCurrentModelName(capability, deployType, modelName.toStdString());
    //bool isInstall = checkCurrentLocalModelInstall(kaglobal::ondeviceModelInstallPath) && modelName == "qwen_fastllm";
    //KyInfo() << "setSelectLocalModelName" << modelName;
    if ( ret2 == true )
    {
        KyInfo() << "setCurrentModelName success:"<< ret2 ;
    } else {
        KyInfo() << "setCurrentModelName failed:"<< ret2 ;
    }
    return ret2 ;
}

void LocalModelConfigWidget::clickLocalModel(AiCapability capability, QString modelName)
{
    //判断当前模型是否已选中
    bool ret = checkModelSelection(capability, modelName);
    if (ret == true) {
        return;
    }

    // 循环检查模型是否正在使用，直到用户选择继续或取消
    while ((capability == AiCapability::Nlp && (getModelStatus(nlpSelectedLocalModel) == ModelStatus::Running)) ||
           (capability == AiCapability::Vision && (getModelStatus(visionSelectedLocalModel) == ModelStatus::Running)) ||
           (capability == AiCapability::Speech && (getModelStatus(speechSelectedLocalModel) == ModelStatus::Running))) {
        // NLP or VISION or SPEECH 正在使用
        KyInfo() << "正在使用" << static_cast<int>(capability);
        bool canProceed = showModelInUseMessage(this);
        if (canProceed == false) {
            KyInfo() << "取消本次操作";
            return;
        }
        QApplication::processEvents();
    }

    // 当前模型未选中则进行切换
    KyInfo() << "aaa:" << static_cast<int>(capability) << modelName;
    // 共享服务打开，如果要切换本地模型，提示服务会被关闭, 是否要切换
    bool confirmSwitchModel = false;
    if (capability == AiCapability::Nlp && m_configManager.getEnableSharedComputePower()) {
        // 不切换直接返回
        if (!confirmSwitchLocalModel()) {
            return;
        }
        confirmSwitchModel = true;
    }
    // 判断当前模型是否已经安装可切换，否则直接返回
    bool ret1 = setSelectLocalModelName(capability, DeployType::Ondevice, modelName);
    if (ret1 == true) {
        switchModelItem(capability, modelName);
        KyInfo() << modelName << " 设置为选中模型成功";
        if (capability == AiCapability::Nlp && confirmSwitchModel) {
            emit switchModel();
        }
    } else {
        showErrorMessage(tr("Model switching failed"), this);
        KyInfo() << modelName << " 设置为选中模型失败";
    }
}

bool LocalModelConfigWidget::checkModelSelection(AiCapability capability, QString modelName)
{
    if (capability == AiCapability::Nlp) {
        if (modelName == nlpSelectedLocalModel) {
            return true;
        } else {
            return false;
        }

    } else if (capability == AiCapability::Vision) {
        if (modelName == visionSelectedLocalModel) {
            return true;
        } else {
            return false;
        }

    } else if (capability == AiCapability::Speech) {
        if (modelName == speechSelectedLocalModel) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool LocalModelConfigWidget::checkCurrentLocalModelInstall(const QString &filePath)
{
    //检查当前本地模型是否已经安装
    QFileInfo fileInfo(filePath);
    return  fileInfo.exists();
}

void LocalModelConfigWidget::showCentered()
{
   if (QWidget* mainWindow = QApplication::activeWindow())
   {
       QRect mainGeom = mainWindow->geometry();
       move(mainGeom.x() + (mainGeom.width() - width()) / 2,
       mainGeom.y() + (mainGeom.height() - height()) / 2 + 15);
   }
   adjustPosition(this); // 调用内部调整逻辑
}

bool LocalModelConfigWidget::confirmSwitchLocalModel() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Using computing power sharing"));
    msgBox.setInformativeText(tr("Switching to the local model will automatically disable computing power sharing "
                              "and disconnect devices that are currently connecting. Do you want to proceed with the switch?"));

    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *switchButton = msgBox.addButton(QObject::tr("Continue Switching"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(switchButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

bool LocalModelConfigWidget::confirmUnloadLocalModel() {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Using computing power sharing"));
    msgBox.setInformativeText(tr("Uninstalling this model will disable computing power sharing and disconnect the "
                                 "currently connected devices. Do you want to proceed with the uninstall?"));
    msgBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *uninstallButton = msgBox.addButton(QObject::tr("Continue Uninstalling"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(uninstallButton);
    int result = msgBox.exec();
    return result == QMessageBox::Accepted;
}

void LocalModelConfigWidget::setupThemeListener() {
    // 创建 QGSettings 对象来监听主题变化
    const QByteArray themeSchema(THEME_QT_SCHEMA);
    if (QGSettings::isSchemaInstalled(themeSchema)) {
        m_themeSettings = new QGSettings(themeSchema, QByteArray(), this);

        // 初始化时获取当前主题并更新图标
        QString currentTheme = getCurrentTheme();
        updateSystemModelIconsForTheme(currentTheme);

        // 连接信号槽监听主题变化
        connect(m_themeSettings, &QGSettings::changed, this,
                [this](const QString& key) {
                    if (key == "styleName" || key == "style-name") {
                        QString newTheme = getCurrentTheme();
                        updateSystemModelIconsForTheme(newTheme);
                    }
                });
    }
}

QString LocalModelConfigWidget::getCurrentTheme() {
    if (m_themeSettings) {
        return m_themeSettings->get("styleName").toString();
    }
    return "";
}

void LocalModelConfigWidget::updateSystemModelIconsForTheme(
    const QString& theme) {
    // 重新设置所有系统模型图标
    updateSystemModelIcon(m_otherVisionIconLabel, ":/res/icon/system-model.svg",
                          ":/res/icon/system-model-light.svg");
    updateSystemModelIcon(m_otherSpeechIconLabel, ":/res/icon/system-model.svg",
                          ":/res/icon/system-model-light.svg");
    updateSystemModelIcon(m_otherIconLabel, ":/res/icon/system-model.svg",
                          ":/res/icon/system-model-light.svg");
}

void LocalModelConfigWidget::updateSystemModelIcon(
    QLabel* label, const QString& defaultIconPath,
    const QString& lightIconPath) {
    QString iconPath;
    QString theme = getCurrentTheme();

    if (theme == "ukui-dark" || theme == "ukui-black") {
        // 深色主题使用带-light后缀的图标
        iconPath = lightIconPath;
    } else {
        // 浅色主题使用普通图标
        iconPath = defaultIconPath;
    }

    // 检查深色主题图标是否存在，如果不存在则使用原始图标
    QPixmap pixmap(iconPath);
    if (pixmap.isNull()) {
        // 如果深色主题图标不存在，回退到原来的图标
        iconPath = defaultIconPath;
    }

    QPixmap scaledPixmap(iconPath);
    if (!scaledPixmap.isNull()) {
        scaledPixmap = scaledPixmap.scaled(16, 16, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
        label->setPixmap(scaledPixmap);
    }
}
