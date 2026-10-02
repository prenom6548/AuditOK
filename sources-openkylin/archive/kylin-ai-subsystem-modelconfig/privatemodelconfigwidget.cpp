#include "privatemodelconfigwidget.h"

#include <QTimer>

#include "modelconfigdialog.h"
#include "util.h"

bool PrivateModelConfigWidget::GroupExpansionState::publicCloudExpanded = false;
bool PrivateModelConfigWidget::GroupExpansionState::localNetworkExpanded = false;
bool PrivateModelConfigWidget::GroupExpansionState::localExpanded = false;

PrivateModelConfigWidget::PrivateModelConfigWidget(QWidget *parent) :QDialog(parent)
{
    //初始化布局
    initLayout();
    // 初始化模型来源组配置
    initializeModelSourceGroups();
    //初始化私有模型能力
    loadModelConfig();
    //初始化信号槽
    initConnect();

    // 恢复之前保存的展开状态
    restoreExpansionStates();

    // 根据架构设置本地模型组的可见性
    updateLocalModelGroupVisibility();
}

// 保存展开状态
void PrivateModelConfigWidget::saveExpansionStates() {
    GroupExpansionState::publicCloudExpanded = m_publicCloudGroup->isExpanded();
    GroupExpansionState::localNetworkExpanded = m_localNetworkGroup->isExpanded();
    if (hasLocalModel()) {
        GroupExpansionState::localExpanded = m_localGroup->isExpanded();
    }
}

// 恢复展开状态
void PrivateModelConfigWidget::restoreExpansionStates() {
    m_publicCloudGroup->setExpanded(GroupExpansionState::publicCloudExpanded);
    m_localNetworkGroup->setExpanded(GroupExpansionState::localNetworkExpanded);
    if (hasLocalModel()) {
        m_localGroup->setExpanded(GroupExpansionState::localExpanded);
    }
}

void PrivateModelConfigWidget::closeEvent(QCloseEvent *event)
{
    // 在窗口关闭时保存当前的展开状态
    saveExpansionStates();
    // 调用父类的closeEvent以完成正常关闭流程
    QDialog::closeEvent(event);
}

void PrivateModelConfigWidget::initLayout()
{
    this->setFixedSize(544, 570);
    this->setWindowTitle(tr("Private Model Config"));
    m_mainVLayout = new QVBoxLayout();

    initModelTitleLayout();
    initExpandableModelGroups();

    m_mainVLayout->setStretch(0,2);
    m_mainVLayout->setStretch(1,8);
    this->setLayout(m_mainVLayout);
}

void PrivateModelConfigWidget::initModelTitleLayout() {
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
    QPixmap iconPixmap(":/res/icon/ukui-private-model-symbolic.svg"); // 替换成自定义模型图标
    if(!iconPixmap.isNull()){
        iconPixmap = iconPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_iconLabel->setPixmap(iconPixmap);
    }
    m_iconLabel->setFixedSize(24, 24);
    m_vLeftLayout->addWidget(m_iconLabel);

    // 右侧文本布局
    m_privateHLayout = new QVBoxLayout();
    m_privateHLayout->setContentsMargins(0, 12, 0, 16);

    m_privateLabel = new QLabel(backgroundLabel);
    m_privateLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_privatLabel");
    m_privateLabel->setMinimumSize(216, 24);
    m_privateLabel->setText(tr("Private Model"));
    QFont privateFont;
    privateFont.setBold(true);
    m_privateLabel->setFont(privateFont);
    m_privateLabel->setStyleSheet("color: #3790FA; background: transparent;");

    m_descripLabel = new kdk::KLabel(backgroundLabel);
    m_descripLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_descripLabel");
    m_descripLabel->setMinimumSize(450, 20);
    m_descripLabel->setText(
        tr("Configure local model files, LAN, and public cloud model "
           "configurations without Kylin authentication (we apologize for any "
           "inconvenience causeds.)"));
    QFont privateDescripFont;
    privateDescripFont.setPointSize(privateFont.pointSize() - 2);
    m_descripLabel->setFont(privateDescripFont);
    setKLabelFontColor(m_descripLabel, "#3790FA");

    m_privateHLayout->addWidget(m_privateLabel);
    m_privateHLayout->addWidget(m_descripLabel);

    // 组合布局
    m_titleHLayout->addLayout(m_vLeftLayout);
    m_titleHLayout->addLayout(m_privateHLayout);

    // 添加到主垂直布局
    m_mainVLayout->addWidget(backgroundLabel);
}

// 创建一个辅助函数来设置按钮样式
void PrivateModelConfigWidget::setupAddItemButtonStyle(QPushButton* button) {
    button->setStyleSheet(
        "QPushButton {"
        "  background-color: palette(base);"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: palette(text);"
        "}"
        "QPushButton:hover {"
        "  background-color: palette(button);"
        "}"
        "QPushButton:pressed {"
        "  background-color: palette(midlight);"
        "}");
}

// 新增函数：初始化可展开模型组
void PrivateModelConfigWidget::initExpandableModelGroups() {
    // 创建滚动区域（如果还没有的话）
    if (!m_scrollArea) {
        m_scrollArea = new QScrollArea(this);
        m_scrollArea->setAccessibleName("kylin-ai-subsystem-modelconfig_QScrollArea_m_scrollArea");
        m_scrollArea->setFixedHeight(389);
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setFrameStyle(QFrame::NoFrame);
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
        m_scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);

        // 创建滚动内容容器
        m_scrollWidget = new QWidget;
        m_scrollWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_scrollWidget");
        m_scrollVLayout = new QVBoxLayout;
        m_scrollVLayout->setContentsMargins(16, 0, 16, 0);
        m_scrollVLayout->setSpacing(8);
        m_scrollWidget->setLayout(m_scrollVLayout);
        m_scrollArea->setWidget(m_scrollWidget);
    }

    // 创建公有云模型组
    m_publicCloudGroup = new ExpandableModelGroup(
        tr("Public Cloud Models"),
        tr("Deployed on a third-party cloud platform, it can be accessed and used through an API Key or an address."),
        kyai::config::model::DeployType::Custom, this);

    // 文本模型组容器
    m_nlpPublicCloudModelGroup = new KBackgroundGroup;
    m_nlpPublicCloudModelGroup->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_SettingGroup_m_"
        "nlpPublicCloudModelGroup");
    m_nlpPublicCloudModelGroup->setStyleSheet("background-color: palette(Base);");
    m_nlpPublicCloudModelGroup->setBorderRadius(6);

    // 图像模型组容器
    m_visionPublicCloudModelGroup = new KBackgroundGroup;
    m_visionPublicCloudModelGroup->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_SettingGroup_m_"
        "visionPublicCloudModelGroup");
    m_visionPublicCloudModelGroup->setStyleSheet("background-color: palette(Base);");
    m_visionPublicCloudModelGroup->setBorderRadius(6);

    // 语音模型组容器
    m_speechPublicCloudModelGroup = new KBackgroundGroup;
    m_speechPublicCloudModelGroup->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_SettingGroup_m_"
        "speechPublicCloudModelGroup");
    m_speechPublicCloudModelGroup->setStyleSheet("background-color: palette(Base);");

    // 创建文本类模型区域（公有云）
    auto* nlpPublicCloudModelWidget = new QWidget(m_nlpPublicCloudModelGroup);
    nlpPublicCloudModelWidget->setMinimumHeight(60);
    nlpPublicCloudModelWidget->setMaximumHeight(60);
    nlpPublicCloudModelWidget->setMaximumWidth(475);
    nlpPublicCloudModelWidget->setStyleSheet("background-color: palette(Base);");

    // 文本类模型水平布局（标题+图标+按钮）
    auto* nlpPublicCloudModelHLayout = new QHBoxLayout(nlpPublicCloudModelWidget);
    nlpPublicCloudModelHLayout->setContentsMargins(16, 0, 16, 0);

    // 文本类模型标题
    auto* nlpPublicCloudModelLabel = new QLabel(tr("NLP Model"), nlpPublicCloudModelWidget);
    nlpPublicCloudModelLabel->setAlignment(Qt::AlignCenter);

    // 添加图标到文本标签旁边
    auto* nlpPublicCloudModelIconLabel = new QLabel(nlpPublicCloudModelWidget);
    nlpPublicCloudModelIconLabel->setStyleSheet("color: palette(Text);");
    nlpPublicCloudModelIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    nlpPublicCloudModelIconLabel->setProperty("useIconHighlightEffect", 0x2);
    nlpPublicCloudModelIconLabel->setToolTip(
        tr("Processing and generating text, such as understanding text and answering questions"));
    nlpPublicCloudModelIconLabel->setStyleSheet(
        "QToolTip {"
        "    border-radius: 4px;"
        "    padding: 3px;"
        "}");

    // 公有云NLP小添加按钮
    m_nlpPublicCloudAddButton = new kdk::KBorderlessButton;
    m_nlpPublicCloudAddButton->setFixedSize(60, 28);
    m_nlpPublicCloudAddButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_nlpPublicCloudAddButton->setIconSize(QSize(16, 16));
    m_nlpPublicCloudAddButton->setText(tr("Add"));

    nlpPublicCloudModelHLayout->addWidget(nlpPublicCloudModelLabel);
    nlpPublicCloudModelHLayout->addWidget(nlpPublicCloudModelIconLabel);
    nlpPublicCloudModelHLayout->addStretch();
    nlpPublicCloudModelHLayout->addWidget(m_nlpPublicCloudAddButton);

    // 大添加按钮（初始隐藏）
    m_nlpPublicCloudAddItemButton = new QPushButton(tr("+ Add"), m_nlpPublicCloudModelGroup);
    m_nlpPublicCloudAddItemButton->setObjectName("PublicCloudTextAddItemButton");
    m_nlpPublicCloudAddItemButton->setMinimumHeight(56);
    m_nlpPublicCloudAddItemButton->setMaximumHeight(56);
    m_nlpPublicCloudAddItemButton->setMaximumWidth(475);
    m_nlpPublicCloudAddItemButton->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_QPushButton_m_"
        "nlpPublicCloudAddItemButton");
    m_nlpPublicCloudAddItemButton->hide();
    setupAddItemButtonStyle(m_nlpPublicCloudAddItemButton);

    // 创建图像类模型区域
    auto* visionPublicCloudModelWidget = new QWidget(m_visionPublicCloudModelGroup);
    visionPublicCloudModelWidget->setMinimumHeight(60);
    visionPublicCloudModelWidget->setMaximumHeight(60);
    visionPublicCloudModelWidget->setMaximumWidth(475);
    visionPublicCloudModelWidget->setStyleSheet("background-color: palette(Base);");

    // 图像类模型标题
    auto* visionPublicCloudModelLabel = new QLabel(tr("Visual Model"), visionPublicCloudModelWidget);
    visionPublicCloudModelLabel->setAlignment(Qt::AlignCenter);

    // 添加图标到文本标签旁边
    auto* visionPublicCloudModelIconLabel = new QLabel(visionPublicCloudModelWidget);
    visionPublicCloudModelIconLabel->setStyleSheet("color: palette(Text);");
    visionPublicCloudModelIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    visionPublicCloudModelIconLabel->setProperty("useIconHighlightEffect", 0x2);
    visionPublicCloudModelIconLabel->setToolTip(
        tr("Processing and generating images, such as recognizing objects and "
           "analyzing scenes"));
    visionPublicCloudModelIconLabel->setStyleSheet(
        "QToolTip {"
        "    border-radius: 4px;"
        "    padding: 3px;"
        "}");

    // 小添加按钮
    m_visionPublicCloudAddButton = new kdk::KBorderlessButton;
    m_visionPublicCloudAddButton->setFixedSize(60, 28);
    m_visionPublicCloudAddButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_visionPublicCloudAddButton->setIconSize(QSize(16, 16));
    m_visionPublicCloudAddButton->setText(tr("Add"));

    // 图像类模型水平布局（标题+图标+按钮）
    auto* visionPublicCloudModelHLayout = new QHBoxLayout(visionPublicCloudModelWidget);
    visionPublicCloudModelHLayout->setContentsMargins(16, 0, 16, 0);
    visionPublicCloudModelHLayout->addWidget(visionPublicCloudModelLabel);
    visionPublicCloudModelHLayout->addWidget(visionPublicCloudModelIconLabel);
    visionPublicCloudModelHLayout->addStretch();
    visionPublicCloudModelHLayout->addWidget(m_visionPublicCloudAddButton);

    // 大添加按钮（初始隐藏）
    m_visionPublicCloudAddItemButton = new QPushButton(tr("+ Add"), m_visionPublicCloudModelGroup);
    m_visionPublicCloudAddItemButton->setObjectName("VisionPublicCloudAddItemButton");
    m_visionPublicCloudAddItemButton->setMinimumHeight(56);
    m_visionPublicCloudAddItemButton->setMaximumHeight(56);
    m_visionPublicCloudAddItemButton->setMaximumWidth(475);
    m_visionPublicCloudAddItemButton->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_QPushButton_m_"
        "visionPublicCloudAddItemButton");
    m_visionPublicCloudAddItemButton->hide();
    setupAddItemButtonStyle(m_visionPublicCloudAddItemButton);

    // 创建语音类模型区域
    auto* speechPublicCloudModelWidget = new QWidget(m_speechPublicCloudModelGroup);
    speechPublicCloudModelWidget->setMinimumHeight(60);
    speechPublicCloudModelWidget->setMaximumHeight(60);
    speechPublicCloudModelWidget->setMaximumWidth(475);
    speechPublicCloudModelWidget->setStyleSheet("background-color: palette(Base);");

    // 语音类模型标题
    auto* speechPublicCloudModelLabel = new QLabel(tr("Speech Model"), speechPublicCloudModelWidget);
    speechPublicCloudModelLabel->setAlignment(Qt::AlignCenter);

    // 添加图标到文本标签旁边
    auto* speechPublicCloudModelIconLabel = new QLabel(speechPublicCloudModelWidget);
    speechPublicCloudModelIconLabel->setStyleSheet("color: palette(Text);");
    speechPublicCloudModelIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    speechPublicCloudModelIconLabel->setProperty("useIconHighlightEffect", 0x2);
    speechPublicCloudModelIconLabel->setToolTip(
        tr("Processing sound and speech, such as recognizing speech and "
           "synthesizing speech"));
    speechPublicCloudModelIconLabel->setStyleSheet(
        "QToolTip {"
        "    border-radius: 4px;"
        "    padding: 3px;"
        "}");

    // 小添加按钮
    m_speechPublicCloudAddButton = new kdk::KBorderlessButton;
    m_speechPublicCloudAddButton->setFixedSize(60, 28);
    m_speechPublicCloudAddButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_speechPublicCloudAddButton->setIconSize(QSize(16, 16));
    m_speechPublicCloudAddButton->setText(tr("Add"));

    // 语音类模型水平布局（标题+图标+按钮）
    auto* speeechPublicCloudModelHLayout = new QHBoxLayout(speechPublicCloudModelWidget);
    speeechPublicCloudModelHLayout->setContentsMargins(16, 0, 16, 0);
    speeechPublicCloudModelHLayout->addWidget(speechPublicCloudModelLabel);
    speeechPublicCloudModelHLayout->addWidget(speechPublicCloudModelIconLabel);
    speeechPublicCloudModelHLayout->addStretch();
    speeechPublicCloudModelHLayout->addWidget(m_speechPublicCloudAddButton);

    // 大添加按钮（初始隐藏）
    m_speechPublicCloudAddItemButton = new QPushButton(tr("+ Add"), m_speechPublicCloudModelGroup);
    m_speechPublicCloudAddItemButton->setObjectName("SpeechPublicCloudAddItemButton");
    m_speechPublicCloudAddItemButton->setMinimumHeight(56);
    m_speechPublicCloudAddItemButton->setMaximumHeight(56);
    m_speechPublicCloudAddItemButton->setMaximumWidth(475);
    m_speechPublicCloudAddItemButton->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_QPushButton_m_"
        "speechPublicCloudAddItemButton");
    m_speechPublicCloudAddItemButton->hide();
    setupAddItemButtonStyle(m_speechPublicCloudAddItemButton);

    // 将公有云模型类别组件添加到公有云内容布局
    m_nlpPublicCloudModelGroup->addWidget(nlpPublicCloudModelWidget);
    m_nlpPublicCloudModelGroup->addWidget(m_nlpPublicCloudAddItemButton);

    m_visionPublicCloudModelGroup->addWidget(visionPublicCloudModelWidget);
    m_visionPublicCloudModelGroup->addWidget(m_visionPublicCloudAddItemButton);

    m_speechPublicCloudModelGroup->addWidget(speechPublicCloudModelWidget);
    m_speechPublicCloudModelGroup->addWidget(m_speechPublicCloudAddItemButton);

    // 将模型组添加到公有云布局
    m_publicCloudGroup->addContentWidget(m_nlpPublicCloudModelGroup);
    m_publicCloudGroup->addContentWidget(m_visionPublicCloudModelGroup);
    m_publicCloudGroup->addContentWidget(m_speechPublicCloudModelGroup);

    // 创建局域网模型组
    m_localNetworkGroup = new ExpandableModelGroup(
        tr("Local Network Models"),
        tr("Deployed on the enterprise intranet or local server, accessed through a local area network address."),
        kyai::config::model::DeployType::Custom, this);

    // 文本模型组容器（局域网）
    m_nlpLocalNetworkModelGroup = new KBackgroundGroup;
    m_nlpLocalNetworkModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_localNetworkGroup");
    m_nlpLocalNetworkModelGroup->setStyleSheet("background-color: palette(Base);");
    m_nlpLocalNetworkModelGroup->setBorderRadius(6);

    // 创建文本类模型区域（局域网）
    auto* nlpLocalNetworkModelWidget = new QWidget(m_nlpLocalNetworkModelGroup);
    nlpLocalNetworkModelWidget->setMinimumHeight(60);
    nlpLocalNetworkModelWidget->setMaximumHeight(60);
    nlpLocalNetworkModelWidget->setMaximumWidth(475);
    nlpLocalNetworkModelWidget->setStyleSheet("background-color: palette(Base);");

    // 文本类模型标题
    auto* nlpLocalNetworkModelLabel = new QLabel(tr("NLP Model"), nlpLocalNetworkModelWidget);
    nlpLocalNetworkModelLabel->setAlignment(Qt::AlignCenter);

    // 添加图标到文本标签旁边
    auto* nlpLocalNetworkModelIconLabel = new QLabel(nlpLocalNetworkModelWidget);
    nlpLocalNetworkModelIconLabel->setStyleSheet("color: palette(Text);");
    nlpLocalNetworkModelIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    nlpLocalNetworkModelIconLabel->setProperty("useIconHighlightEffect", 0x2);
    nlpLocalNetworkModelIconLabel->setToolTip(
        tr("Processing and generating text, such as understanding text and "
           "answering questions"));
    nlpLocalNetworkModelIconLabel->setStyleSheet(
        "QToolTip {"
        "    border-radius: 4px;"
        "    padding: 3px;"
        "}");

    // 局域网NLP小添加按钮
    m_nlpLocalNetworkAddButton = new kdk::KBorderlessButton;
    m_nlpLocalNetworkAddButton->setFixedSize(60, 28);
    m_nlpLocalNetworkAddButton->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_nlpLocalNetworkAddButton->setIconSize(QSize(16, 16));
    m_nlpLocalNetworkAddButton->setText(tr("Add"));

    // 局域网NLP模型水平布局（标题+图标+按钮）
    auto* nlpLocalNetworkModelHLayout = new QHBoxLayout(nlpLocalNetworkModelWidget);
    nlpLocalNetworkModelHLayout->setContentsMargins(16, 0, 16, 0);
    nlpLocalNetworkModelHLayout->addWidget(nlpLocalNetworkModelLabel);
    nlpLocalNetworkModelHLayout->addWidget(nlpLocalNetworkModelIconLabel);
    nlpLocalNetworkModelHLayout->addStretch();
    nlpLocalNetworkModelHLayout->addWidget(m_nlpLocalNetworkAddButton);

    // 局域网NLP大添加按钮（初始隐藏）
    m_nlpLocalNetworkAddItemButton = new QPushButton(tr("+ Add"), m_nlpLocalNetworkModelGroup);
    m_nlpLocalNetworkAddItemButton->setObjectName("LocalNetworkTextAddItemButton");
    m_nlpLocalNetworkAddItemButton->setMinimumHeight(56);
    m_nlpLocalNetworkAddItemButton->setMaximumHeight(56);
    m_nlpLocalNetworkAddItemButton->setMaximumWidth(475);
    m_nlpLocalNetworkAddItemButton->setAccessibleName(
        "kylin-ai-subsystem-modelconfig_QPushButton_m_"
        "nlpLocalNetworkAddItemButton");
    m_nlpLocalNetworkAddItemButton->hide();  // 初始隐藏
    setupAddItemButtonStyle(m_nlpLocalNetworkAddItemButton);

    // 将局域网NLP组件添加到局域网内容布局
    m_nlpLocalNetworkModelGroup->addWidget(nlpLocalNetworkModelWidget);
    m_nlpLocalNetworkModelGroup->addWidget(m_nlpLocalNetworkAddItemButton);

    // 将模型组添加到局域网云布局
    m_localNetworkGroup->addContentWidget(m_nlpLocalNetworkModelGroup);

    // 创建本地模型组
    m_localGroup = new ExpandableModelGroup(tr("Local Models"),
                                            tr("Installed and running on the user's local device, powered by local "
                                               "computing power, and can be used offline."),
                                            kyai::config::model::DeployType::Custom, this);

    // 文本模型组容器（本地）
    m_nlpLocalModelGroup = new KBackgroundGroup;
    m_nlpLocalModelGroup->setAccessibleName("kylin-ai-subsystem-modelconfig_SettingGroup_m_localNetworkGroup");
    m_nlpLocalModelGroup->setStyleSheet("background-color: palette(Base);");
    m_nlpLocalModelGroup->setBorderRadius(6);

    // 创建本地文本类模型区域
    auto* nlpLocalModelWidget = new QWidget(m_localGroup);
    nlpLocalModelWidget->setMinimumHeight(60);
    nlpLocalModelWidget->setMaximumHeight(60);
    nlpLocalModelWidget->setMaximumWidth(475);
    nlpLocalModelWidget->setStyleSheet("background-color: palette(Base);");

    // 文本类模型标题
    auto* nlpLocalModelLabel = new QLabel(tr("NLP Model"), nlpLocalModelWidget);
    nlpLocalModelLabel->setAlignment(Qt::AlignCenter);

    // 添加图标到文本标签旁边
    auto* nlpLocalModelIconLabel = new QLabel(nlpLocalModelWidget);
    nlpLocalModelIconLabel->setStyleSheet("color: palette(Text);");
    nlpLocalModelIconLabel->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(16, 16)));
    nlpLocalModelIconLabel->setProperty("useIconHighlightEffect", 0x2);
    nlpLocalModelIconLabel->setToolTip(
        tr("Processing and generating text, such as understanding text and "
           "answering questions"));
    nlpLocalModelIconLabel->setStyleSheet(
        "QToolTip {"
        "    border-radius: 4px;"
        "    padding: 3px;"
        "}");

    // 添加帮助链接按钮
    m_helpLinkButton = new kdk::KBorderlessButton(tr("Adding Method"));
    m_helpLinkButton->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_m_helpLinkButton");

    // 本地NLP模型水平布局（标题+图标+帮助按钮）
    auto* nlpLocalModelHLayout = new QHBoxLayout(nlpLocalModelWidget);
    nlpLocalModelHLayout->setContentsMargins(16, 0, 16, 0);
    nlpLocalModelHLayout->addWidget(nlpLocalModelLabel);
    nlpLocalModelHLayout->addWidget(nlpLocalModelIconLabel);
    nlpLocalModelHLayout->addStretch();
    nlpLocalModelHLayout->addWidget(m_helpLinkButton);

    // 将本地NLP组件添加到本地内容布局
    m_nlpLocalModelGroup->addWidget(nlpLocalModelWidget);

    // 将模型组添加到本地模型布局
    m_localGroup->addContentWidget(m_nlpLocalModelGroup);

    // 将所有模型组添加到滚动区域布局
    m_scrollVLayout->addWidget(m_publicCloudGroup);
    m_scrollVLayout->addWidget(m_localNetworkGroup);
    m_scrollVLayout->addWidget(m_localGroup);
    m_scrollVLayout->addStretch();

    // 添加到主布局
    m_mainVLayout->addWidget(m_scrollArea);

    // 添加确认按钮部分（如果还没有的话）
    if (!m_confirmWidget) {
        m_confirmWidget = new QWidget(this);
        m_confirmWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_confirmWidget");
        m_confirmPrivateButton = new QPushButton(tr("OK"), this);
        m_confirmPrivateButton->setAccessibleName(
            "kylin-ai-subsystem-modelconfig_QPushButton_m_"
            "confirmPrivateButton");
        m_confirmPrivateButton->setProperty("isImportant", true);
        m_confirmPrivateButtoLayout = new QHBoxLayout();
        m_confirmPrivateButtoLayout->setContentsMargins(0, 0, 24, 24);
        m_confirmPrivateButtoLayout->addStretch();
        m_confirmPrivateButtoLayout->addWidget(m_confirmPrivateButton);
        m_confirmWidget->setLayout(m_confirmPrivateButtoLayout);
        m_mainVLayout->addStretch();
        m_mainVLayout->addWidget(m_confirmWidget);
    }

    // 确保滚动区域内容初始时滚动到顶部
    m_scrollArea->verticalScrollBar()->setValue(0);

    // 更新按钮可见性状态
    updateNlpPublicCloudModelVisibility();
    updateVisionPublicCloudModelVisibility();
    updateSpeechPublicCloudModelVisibility();
    updateLocalNetworkNlpModelVisibility();
}

void PrivateModelConfigWidget::updateNlpPublicCloudModelVisibility() {
    bool hasNlpPublicCloudModels = !m_nlpPublicCloudModelItemMap.isEmpty();

    if (hasNlpPublicCloudModels) {
        // 有文本模型时：隐藏大按钮，显示小按钮
        m_nlpPublicCloudAddItemButton->hide();
        m_nlpPublicCloudAddButton->show();
    } else {
        // 没有文本模型时：显示大按钮，隐藏小按钮
        m_nlpPublicCloudAddItemButton->show();
        m_nlpPublicCloudAddButton->hide();
    }
}

void PrivateModelConfigWidget::updateVisionPublicCloudModelVisibility() {
    bool hasVisionPublicCloudModels = !m_visionPublicCloudModelItemMap.isEmpty();

    if (hasVisionPublicCloudModels) {
        // 有图像模型时：隐藏大按钮，显示小按钮
        m_visionPublicCloudAddItemButton->hide();
        m_visionPublicCloudAddButton->show();
    } else {
        // 没有图像模型时：显示大按钮，隐藏小按钮
        m_visionPublicCloudAddItemButton->show();
        m_visionPublicCloudAddButton->hide();
    }
}

void PrivateModelConfigWidget::updateSpeechPublicCloudModelVisibility() {
    bool hasSpeechPublicCloudModels = !m_speechPublicCloudModelItemMap.isEmpty();

    if (hasSpeechPublicCloudModels) {
        // 有语音模型时：隐藏大按钮，显示小按钮
        m_speechPublicCloudAddItemButton->hide();
        m_speechPublicCloudAddButton->show();
    } else {
        // 没有语音模型时：显示大按钮，隐藏小按钮
        m_speechPublicCloudAddItemButton->show();
        m_speechPublicCloudAddButton->hide();
    }
}

void PrivateModelConfigWidget::updateLocalNetworkNlpModelVisibility() {
    bool hasLocalNetworkNlpModels = !m_nlpLocalNetworkModelItemMap.isEmpty();

    if (hasLocalNetworkNlpModels) {
        // 有局域网NLP模型时：隐藏大按钮，显示小按钮
        m_nlpLocalNetworkAddItemButton->hide();
        m_nlpLocalNetworkAddButton->show();
    } else {
        // 没有局域网NLP模型时：显示大按钮，隐藏小按钮
        m_nlpLocalNetworkAddItemButton->show();
        m_nlpLocalNetworkAddButton->hide();
    }
}

ModelStatus PrivateModelConfigWidget::getModelStatus(const QString &modelName)
{
    ModelConfig mc;
    ModelStatus ret = mc.getModelStatus(modelName.toStdString());
    KyInfo() << QString("ModelStatus = %0").arg((qint32)ret);
    return ret;
}

void PrivateModelConfigWidget::onEditModelItem(AiCapability capability, QString modelName)
{
    if (!polkit()) {
        return;
    }

    KyInfo() << QString("currentEditModelItem:AiCapability=%0, ModelName=%1").arg((qint32)capability).arg(modelName);
    KyInfo() << "currentEditModelItem:modelName"<<modelName;
    if (capability == AiCapability::Nlp) {
        if (m_nlpLocalNetworkModelMap.contains(modelName)) {
            // 局域网NLP模型
            PrivateModel currentModel = m_nlpLocalNetworkModelMap[modelName];
            PrivateModelConfigDialog* settingDialog = new PrivateModelConfigDialog(currentModel, this);
            // 通过调用dialog中的方法设置来源组
            settingDialog->m_sourceGroup = "localNetwork";
            settingDialog->setModal(true);
            settingDialog->show();
            QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
        } else {
            // 公有云NLP模型
            PrivateModel currentModel = m_nlpPublicCloudModelMap[modelName];
            PrivateModelConfigDialog* settingDialog = new PrivateModelConfigDialog(currentModel, this);
            settingDialog->m_sourceGroup = "publicCloud";
            settingDialog->setModal(true);
            settingDialog->show();
            QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
        }
    } else if (capability == AiCapability::Vision) {
        // 随父窗口居中
        PrivateModel currentModel = m_visionPublicCloudModelMap[modelName];
        ModelConfigDialog* settingDialog = new ModelConfigDialog(m_visionPublicCloudModelMap[modelName], this);
        settingDialog->setModal(true);
        settingDialog->show();
        QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
    } else if (capability == AiCapability::Speech) {
        // 随父窗口居中
        PrivateModel currentModel = m_speechPublicCloudModelMap[modelName];
        ModelConfigDialog* settingDialog = new ModelConfigDialog(m_speechPublicCloudModelMap[modelName], this);
        settingDialog->setModal(true);
        settingDialog->show();
        QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
    }
}

void PrivateModelConfigWidget::onDeleteModelItem(AiCapability capability, QString modelName)
{
    ModelConfig mc;
    // 删除模型
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
            if (capability == AiCapability::Nlp) {
                // NLP模型：执行完整的删除操作
                bool ret = mc.deleteCustomModel(modelName.toStdString());
                if (ret == true) {
                    deleteModelItem(capability, modelName);
                    m_modelOrderManager.deleteModel(capability, modelName);
                    KyInfo() << modelName << " 删除成功";
                } else {
                    showErrorMessage(tr("Failed to delete model"), this);
                    KyInfo() << modelName << " 删除失败";
                }
            } else {
                // Vision/Speech模型：仅清除认证信息
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
}

bool PrivateModelConfigWidget::eventFilter(QObject *watched, QEvent *event)
{

    //模型配置
    PrivateModelItemWidget *clickedModelItem = qobject_cast<PrivateModelItemWidget*>(watched);
    if (clickedModelItem) {

        if (event->type() == QEvent::MouseButtonRelease) {

            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->button() == Qt::LeftButton) {  // 判断左键
                                                           // 切换模型
                clickPrivateModel(clickedModelItem->getAICapability(), clickedModelItem->getModelName());
                return true;
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

void PrivateModelConfigWidget::loadModelConfig() {
    // 清空modelmap所有控件
    m_nlpPublicCloudModelMap.clear();
    m_visionPublicCloudModelMap.clear();
    m_speechPublicCloudModelMap.clear();
    m_nlpLocalModelMap.clear();
    m_nlpLocalNetworkModelMap.clear();

    // 删除ModelItem对象
    for (PrivateModelItemWidget* item : m_nlpPublicCloudModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }
    for (PrivateModelItemWidget* item : m_visionPublicCloudModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }
    for (PrivateModelItemWidget* item : m_speechPublicCloudModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }

    for (PrivateModelItemWidget* item : m_nlpLocalNetworkModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }

    for (PrivateModelItemWidget* item : m_nlpLocalModelItemMap) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }

    // 清空所有映射和列表
    m_nlpPublicCloudModelItemMap.clear();
    m_visionPublicCloudModelItemMap.clear();
    m_speechPublicCloudModelItemMap.clear();
    m_nlpLocalNetworkModelItemMap.clear();
    m_nlpLocalModelItemMap.clear();

    m_nlpPublicCloudModelItemList.clear();
    m_visionPublicCloudModelItemList.clear();
    m_speechPublicCloudModelItemList.clear();
    m_nlpLocalNetworkModelItemList.clear();
    m_nlpLocalModelItemList.clear();

    // 清空UI映射
    m_nlpPublicCloudMapUI.clear();
    m_visionPublicCloudMapUI.clear();
    m_speechPublicCloudMapUI.clear();
    m_nlpLocalNetworkMapUI.clear();
    m_nlpLocalMapUI.clear();

    // 清空选中状态
    nlpSelectedModel.clear();
    visionSelectedPublicCloudModel.clear();
    speechSelectedPublicCloudModel.clear();

    // 加载模型来源组配置
    m_modelSourceGroups = loadModelSourceGroups();

    ModelConfig mc;

    std::string nlpCustomModel = mc.getCurrentModelName(AiCapability::Nlp, DeployType::Custom);
    std::string visionCustomModel = mc.getCurrentModelName(AiCapability::Vision, DeployType::Custom);
    std::string speechCustomModel = mc.getCurrentModelName(AiCapability::Speech, DeployType::Custom);

    nlpSelectedModel = QString::fromStdString(nlpCustomModel);
    visionSelectedPublicCloudModel = QString::fromStdString(visionCustomModel);
    speechSelectedPublicCloudModel = QString::fromStdString(speechCustomModel);

    // 获取所有公有云，局域网模型
    for (auto capability : {AiCapability::Nlp, AiCapability::Vision, AiCapability::Speech}) {
        std::vector<ExtendedModel> models = mc.getExtendedModels(capability);
        processModelInfo(models, capability);
        for (const auto& model : models) {
            QString modelName = QString::fromStdString(model.modelName);
            PrivateModel privateModel;
            privateModel.capability = model.capability;
            privateModel.vendor = QString::fromStdString(model.vendor);
            privateModel.modelName = QString::fromStdString(model.modelName);
            privateModel.modelVersion = QString::fromStdString(model.modelVersion);
            privateModel.apiUrl = QString::fromStdString(model.apiUrl);
            privateModel.multiAuths = mc.getModelAuthentications(model.modelName);
            privateModel.is3rdModel = false;


            std::vector<ExtendedMultiAuthentication> extendedAuths =
                mc.getModelExtendedAuthentications(model.modelName);
            KyInfo() << "getModelExtendedAuthentications returned" << extendedAuths.size()
                     << "items for model:" << QString::fromStdString(model.modelName);
            for (size_t i = 0; i < extendedAuths.size(); ++i) {
                const auto& extAuth = extendedAuths[i];
                KyInfo() << "Item" << i << ":" << QString::fromStdString(extAuth.itemName)
                         << "Enabled:" << extAuth.enabled;

                for (const auto& auth : extAuth.authentications) {
                    KyInfo() << "  Auth key:" << QString::fromStdString(auth.key)
                             << "friendlyKeyName:" << QString::fromStdString(auth.friendlyKeyName)
                             << "value length:" << auth.value.length();
                }
            }

            for (const auto& authen : privateModel.multiAuths) {
                KyInfo() << "loadModelConfig配置项名称" << QString::fromStdString(authen.itemName);
                for (const auto& au : authen.authentications) {
                    if (au.key == "apiKey") {
                        privateModel.apiKey = QString::fromStdString(au.value);
                    }
                }
            }


            for (const auto& extAuth : extendedAuths) {
                KyInfo() << "Extended auth item:" << QString::fromStdString(extAuth.itemName)
                         << "Enabled:" << extAuth.enabled;

                // 根据扩展认证信息中的启用状态设置开关状态
                if (extAuth.itemName == "语音听写&语音合成") {
                    continuousSelected = extAuth.enabled;
                } else if (extAuth.itemName == "实时语音转写") {
                    realtimeSelected = extAuth.enabled;
                }
            }

            // 根据来源组将模型加载到正确的数据结构
            QString sourceGroup = getModelSourceGroup(privateModel.modelName);

            switch (privateModel.capability) {
                case AiCapability::Nlp:
                    KyInfo() << "Loading model with vendor:" << privateModel.vendor
                             << "and name:" << privateModel.modelName;
                    loadModelItemWithSourceGroup(privateModel, sourceGroup, nlpSelectedModel, true);
                    break;
                case AiCapability::Vision:
                    KyInfo() << "Loading model with vendor:" << privateModel.vendor
                             << "and name:" << privateModel.modelName;
                    loadModelItemWithSourceGroup(privateModel, sourceGroup, visionSelectedPublicCloudModel, true);
                    break;
                case AiCapability::Speech:
                    KyInfo() << "Loading model with vendor:" << privateModel.vendor
                             << "and name:" << privateModel.modelName;
                    loadModelItemWithSourceGroup(privateModel, sourceGroup, speechSelectedPublicCloudModel, true);
                    break;
                default:
                    break;
            }
        }
    }

    // 获取所有本地模型（自选-本地模型在getExtendedModels接口返回的是Ondevice类型）
    const auto loaclModels = mc.getExtendedModels(AiCapability::Nlp);

    // 获取当前记录的私有NLP模型添加顺序
    QStringList currentOrder = m_configManager.getPrivateNlpModelAddOrder();

    // 提取有效的本地模型名集合
    QSet<QString> validModelNames;
    QMap<QString, decltype(loaclModels)::value_type> modelDataMap;

    for (const auto& model : loaclModels) {
        if (model.deployType == DeployType::Ondevice && model.is3rdModel) {
            QString modelName = QString::fromStdString(model.modelName);
            validModelNames.insert(modelName);
            modelDataMap[modelName] = model;
        }
    }

    // 清理已不存在的模型，保留仍存在的模型的顺序
    QStringList cleanedOrder;
    for (const QString& modelName : currentOrder) {
        if (validModelNames.contains(modelName)) {
            cleanedOrder.append(modelName);
        }
    }

    // 检查是否有新模型添加
    for (const auto &model : loaclModels) {
        if (model.deployType == DeployType::Ondevice && model.is3rdModel) {
            QString modelName = QString::fromStdString(model.modelName);
            if (!cleanedOrder.contains(modelName)) {
                // 新模型添加到顺序列表的末尾（表示最新添加）
                cleanedOrder.append(modelName);
            }
        }
    }

    // 保存更新后的顺序
    if (cleanedOrder != currentOrder) {
        m_configManager.setPrivateNlpModelAddOrder(cleanedOrder);
    }

    KyDebug() << "Processing local NLP models in order:" << cleanedOrder;

    // 按照排序后的顺序创建UI组件（正序处理，因为新模型已经在列表末尾）
    for (int i = 0; i < cleanedOrder.size(); ++i) {
        const QString& orderedModelName = cleanedOrder[i];
        KyDebug() << "Processing model at index" << i << ":" << orderedModelName;
        
        auto modelData = modelDataMap[orderedModelName];
        
        PrivateModel privateModel;
        privateModel.capability = modelData.capability;
        privateModel.vendor = QString::fromStdString(modelData.vendor);
        privateModel.modelName = QString::fromStdString(modelData.modelName);
        privateModel.modelVersion = QString::fromStdString(modelData.modelVersion);
        privateModel.apiUrl = QString::fromStdString(modelData.apiUrl);
        privateModel.multiAuths = mc.getModelAuthentications(modelData.modelName);
        privateModel.is3rdModel = modelData.is3rdModel;

        for (const auto& authen : privateModel.multiAuths) {
            KyInfo() << "loadModelConfig配置项名称" << QString::fromStdString(authen.itemName);
            for (const auto& au : authen.authentications) {
                if (au.key == "apiKey") {
                    privateModel.apiKey = QString::fromStdString(au.value);
                }
            }
        }

        QString sourceGroup = getModelSourceGroup(privateModel.modelName);
        KyDebug() << "Model" << privateModel.modelName << "assigned to source group:" << sourceGroup;
        
        switch (privateModel.capability) {
            case AiCapability::Nlp:
                KyInfo() << "Loading local model with vendor:" << privateModel.vendor
                         << "and name:" << privateModel.modelName;
                loadModelItemWithSourceGroup(privateModel, sourceGroup, nlpSelectedModel, false);
                break;
            default:
                break;
        }
    }
    // 检查是否没有选中的NLP模型，如果是，按优先级选择模型
    if (nlpSelectedModel.isEmpty()) {
        // 检查本地NLP模型是否有可选的
        if (!m_nlpLocalModelItemList.isEmpty()) {
            // 本地有NLP模型，选择第一个（新增在上面，所以选中.last）
            QString firstModelName = m_nlpLocalModelItemList.last()->getModelName();
            clickPrivateModel(AiCapability::Nlp, firstModelName);
        }
        // 检查公有云NLP模型是否有可选的
        else if (!m_nlpPublicCloudModelItemList.isEmpty()) {
            // 公有云有NLP模型，选择第一个（新增在上面，所以选中.last）
            QString firstModelName = m_nlpPublicCloudModelItemList.last()->getModelName();
            clickPrivateModel(AiCapability::Nlp, firstModelName);
        }
        // 检查局域网NLP模型是否有可选的
        else if (!m_nlpLocalNetworkModelItemList.isEmpty()) {
            // 局域网有NLP模型，选择第一个（新增在上面，所以选中.last）
            QString firstModelName = m_nlpLocalNetworkModelItemList.last()->getModelName();
            clickPrivateModel(AiCapability::Nlp, firstModelName);
        }
    }
}

void PrivateModelConfigWidget::loadModelItemWithSourceGroup(PrivateModel model, const QString& sourceGroup,
                                                            const QString& curSelectedName, bool isCustomModel) {
    QFrame* itemFrame = new QFrame(this);
    itemFrame->setStyleSheet("border-radius: 6px;");
    itemFrame->setMaximumWidth(475);
    itemFrame->setFixedHeight(56);
    itemFrame->setObjectName(model.modelName);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setContentsMargins(0, 0, 0, 0);

    PrivateModelItemWidget* item =
        new PrivateModelItemWidget(model.capability, model.modelName, model.vendor, isCustomModel, itemFrame);
    item->installEventFilter(this);
    if (isCustomModel) {
        connect(item, &PrivateModelItemWidget::sigEdit, this, &PrivateModelConfigWidget::onEditModelItem);
        connect(item, &PrivateModelItemWidget::sigDelete, this, &PrivateModelConfigWidget::onDeleteModelItem);
    }

    layItem->addWidget(item);

    if (model.capability == AiCapability::Nlp) {
        if (isCustomModel) {
            if (sourceGroup == "localNetwork") {
                // 添加到局域网NLP相关数据结构
                m_nlpLocalNetworkModelItemList.append(item);
                m_nlpLocalNetworkModelMap[model.modelName] = model;
                m_nlpLocalNetworkModelItemMap[model.modelName] = item;
                m_nlpLocalNetworkMapUI[model.modelName] = itemFrame;
                m_nlpLocalNetworkModelGroup->insertWidgetAt(1, itemFrame);

                // 当前模型是预设选中模型，则选中
                if (model.modelName == curSelectedName) {
                    item->showSelectedIcon(true);
                    nlpSelectedModel = model.modelName;
                }
                updateLocalNetworkNlpModelVisibility();
            } else {
                // 添加到公有云NLP相关数据结构
                m_nlpPublicCloudModelItemList.append(item);
                m_nlpPublicCloudModelMap[model.modelName] = model;
                m_nlpPublicCloudModelItemMap[model.modelName] = item;
                m_nlpPublicCloudMapUI[model.modelName] = itemFrame;
                m_nlpPublicCloudModelGroup->insertWidgetAt(1, itemFrame);

                // 当前模型是预设选中模型，则选中
                if (model.modelName == curSelectedName) {
                    item->showSelectedIcon(true);
                    nlpSelectedModel = model.modelName;
                }
                updateNlpPublicCloudModelVisibility();
            }
        } else {
            // 处理本地模型 (isCustomModel == false)
            // 添加到本地NLP相关数据结构
            m_nlpLocalModelItemList.append(item);
            m_nlpLocalModelMap[model.modelName] = model;
            m_nlpLocalModelItemMap[model.modelName] = item;
            m_nlpLocalMapUI[model.modelName] = itemFrame;
            m_nlpLocalModelGroup->insertWidgetAt(1, itemFrame);

            // 如果是本地模型的选中状态
            if (model.modelName == curSelectedName) {
                item->showSelectedIcon(true);
                nlpSelectedModel = model.modelName;
            }

            // 更新本地模型组
        }
    } else if (model.capability == AiCapability::Vision) {
        m_visionPublicCloudModelItemList.append(item);
        m_visionPublicCloudModelMap[model.modelName] = model;
        m_visionPublicCloudModelItemMap[model.modelName] = item;
        m_visionPublicCloudMapUI[model.modelName] = itemFrame;
        m_visionPublicCloudModelGroup->insertWidgetAt(1, itemFrame);

        // 设置为选中
        if (model.modelName == curSelectedName) {
            item->showSelectedIcon(true);
            visionSelectedPublicCloudModel = model.modelName;
        }

        // 更新按钮可见性
        updateVisionPublicCloudModelVisibility();
    } else if (model.capability == AiCapability::Speech) {
        m_speechPublicCloudModelItemList.append(item);
        m_speechPublicCloudModelMap[model.modelName] = model;
        m_speechPublicCloudModelItemMap[model.modelName] = item;
        m_speechPublicCloudMapUI[model.modelName] = itemFrame;
        m_speechPublicCloudModelGroup->insertWidgetAt(1, itemFrame);

        // 设置为选中
        if (model.modelName == curSelectedName) {
            item->showSelectedIcon(true);
            speechSelectedPublicCloudModel = model.modelName;
        }

        // 更新按钮可见性
        updateSpeechPublicCloudModelVisibility();
    }

    // 更新所有SettingGroup
    m_nlpPublicCloudModelGroup->update();
    m_visionPublicCloudModelGroup->update();
    m_speechPublicCloudModelGroup->update();
    m_nlpLocalNetworkModelGroup->update();
    m_nlpLocalModelGroup->update();
}

bool PrivateModelConfigWidget::hasLocalModel() {
#if (defined(Q_PROCESSOR_X86) || (defined(Q_PROCESSOR_ARM)))
    return true;
#else
    return false;
#endif
}

void PrivateModelConfigWidget::updateLocalModelGroupVisibility() {
    if (m_localGroup != nullptr) {
        bool shouldShowLocalModel = hasLocalModel();
        m_localGroup->setVisible(shouldShowLocalModel);

        if (!shouldShowLocalModel) {
            KyInfo() << "Local model group is now hidden.";
        } else {
            KyInfo() << "Local model group is now visible.";
        }
    }
}

void PrivateModelConfigWidget::initConnect() {
    connect(m_confirmPrivateButton, &QPushButton::clicked, this, [this]() { this->close(); });

    if (m_nlpPublicCloudAddButton) {
        connect(m_nlpPublicCloudAddButton, &kdk::KBorderlessButton::clicked, this,
                &PrivateModelConfigWidget::onNlpPublicCloudAddClicked);
    }
    if (m_nlpPublicCloudAddItemButton) {
        connect(m_nlpPublicCloudAddItemButton, &QPushButton::clicked, this,
                &PrivateModelConfigWidget::onNlpPublicCloudAddItemClicked);
    }

    if (m_visionPublicCloudAddButton) {
        connect(m_visionPublicCloudAddButton, &kdk::KBorderlessButton::clicked, this,
                &PrivateModelConfigWidget::onVisionPublicCloudAddClicked);
    }
    if (m_visionPublicCloudAddItemButton) {
        connect(m_visionPublicCloudAddItemButton, &QPushButton::clicked, this,
                &PrivateModelConfigWidget::onVisionPublicCloudAddItemClicked);
    }

    if (m_speechPublicCloudAddButton) {
        connect(m_speechPublicCloudAddButton, &kdk::KBorderlessButton::clicked, this,
                &PrivateModelConfigWidget::onSpeechPublicCloudAddClicked);
    }
    if (m_speechPublicCloudAddItemButton) {
        connect(m_speechPublicCloudAddItemButton, &QPushButton::clicked, this,
                &PrivateModelConfigWidget::onSpeechPublicCloudAddItemClicked);
    }

    if (m_nlpLocalNetworkAddButton) {
        connect(m_nlpLocalNetworkAddButton, &kdk::KBorderlessButton::clicked, this,
                &PrivateModelConfigWidget::onLocalNetworkNlpAddClicked);
    }
    if (m_nlpLocalNetworkAddItemButton) {
        connect(m_nlpLocalNetworkAddItemButton, &QPushButton::clicked, this,
                &PrivateModelConfigWidget::onLocalNetworkNlpAddItemClicked);
    }
    if (m_helpLinkButton) {
        connect(m_helpLinkButton, &QPushButton::clicked, [this]() { QDesktopServices::openUrl(kaglobal::openLink); });
    }
}

// 实现初始化函数
void PrivateModelConfigWidget::initializeModelSourceGroups() {
    // 设置配置文件路径到用户配置目录
    QString configDir = QDir::homePath() + "/.config/kylin-ai-subsystem-modelconfig/";
    m_configFilePath = configDir + "kylin-ai-model-location-config.json";

    // 确保目录存在
    QFileInfo fileInfo(m_configFilePath);
    QDir().mkpath(fileInfo.absolutePath());

    // 加载现有的模型来源组配置
    m_modelSourceGroups = loadModelSourceGroups();
}

QString PrivateModelConfigWidget::getModelSourceGroup(const QString& modelName) {
    return m_modelSourceGroups.value(modelName, "publicCloud");
}
void PrivateModelConfigWidget::saveModelSourceGroup(const QString& modelName, const QString& sourceGroup) {
    m_modelSourceGroups[modelName] = sourceGroup;

    // 将配置保存到文件
    QFile configFile(m_configFilePath);
    if (configFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc;
        QJsonObject obj;

        // 将哈希表转换为JSON对象
        for (auto it = m_modelSourceGroups.constBegin(); it != m_modelSourceGroups.constEnd(); ++it) {
            obj[it.key()] = it.value();
        }

        doc.setObject(obj);
        configFile.write(doc.toJson());
        configFile.close();
    }
}

QHash<QString, QString> PrivateModelConfigWidget::loadModelSourceGroups() {
    QHash<QString, QString> result;

    QFile configFile(m_configFilePath);
    if (configFile.exists() && configFile.open(QIODevice::ReadOnly)) {
        QByteArray data = configFile.readAll();
        configFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                result.insert(it.key(), it.value().toString());
            }
        }
    }

    return result;
}

void PrivateModelConfigWidget::addModelItem(PrivateModel model, const QString& sourceGroup) {
    // UkccFrame *itemFrame = new UkccFrame(this);
    QFrame* itemFrame = new QFrame(this);
    itemFrame->setStyleSheet("border-radius: 6px;");
    itemFrame->setMaximumWidth(475);
    itemFrame->setFixedHeight(56);
    itemFrame->setObjectName(model.modelName);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setContentsMargins(0, 0, 0, 0);

    PrivateModelItemWidget* item =
        new PrivateModelItemWidget(model.capability, model.modelName, model.vendor, true, itemFrame);
    item->installEventFilter(this);
    connect(item, &PrivateModelItemWidget::sigEdit, this, &PrivateModelConfigWidget::onEditModelItem);
    connect(item, &PrivateModelItemWidget::sigDelete, this, &PrivateModelConfigWidget::onDeleteModelItem);

    layItem->addWidget(item);
    if (model.capability == AiCapability::Nlp) {
        if (sourceGroup == "localNetwork") {
            // 添加到局域网NLP相关数据结构
            m_nlpLocalNetworkModelItemList.append(item);
            m_nlpLocalNetworkModelMap[model.modelName] = model;
            m_nlpLocalNetworkModelItemMap[model.modelName] = item;
            m_nlpLocalNetworkMapUI[model.modelName] = itemFrame;
            m_nlpLocalNetworkModelGroup->insertWidgetAt(1, itemFrame);

            if (m_nlpLocalNetworkModelItemMap.count() == 1) {
                switchModelItem(AiCapability::Nlp, model.modelName);
            }
            // 更新按钮可见性和高度
            if (m_nlpLocalNetworkModelItemMap.count() > 1) {
                m_localNetworkGroup->increaseContentHeight();
            }
            updateLocalNetworkNlpModelVisibility();
        } else {
            // 添加到公有云NLP相关数据结构
            m_nlpPublicCloudModelItemList.append(item);
            m_nlpPublicCloudModelMap[model.modelName] = model;
            m_nlpPublicCloudModelItemMap[model.modelName] = item;
            m_nlpPublicCloudMapUI[model.modelName] = itemFrame;
            m_nlpPublicCloudModelGroup->insertWidgetAt(1, itemFrame);

            if (m_nlpPublicCloudModelItemMap.count() == 1) {
                switchModelItem(AiCapability::Nlp, model.modelName);
            }
            // 更新按钮可见性和高度
            if (m_nlpPublicCloudModelItemMap.count() > 1) {
                m_publicCloudGroup->increaseContentHeight();
            }
            updateNlpPublicCloudModelVisibility();
        }

        // 保存模型来源组到配置文件
        saveModelSourceGroup(model.modelName, sourceGroup);
    } else if (model.capability == AiCapability::Vision) {
        m_visionPublicCloudModelItemList.append(item);
        m_visionPublicCloudModelMap[model.modelName] = model;
        m_visionPublicCloudModelItemMap[model.modelName] = item;
        m_visionPublicCloudMapUI[model.modelName] = itemFrame;
        m_visionPublicCloudModelGroup->insertWidgetAt(1, itemFrame);

        // 第一个添加的模型，设置为选中
        if (m_visionPublicCloudModelItemMap.count() == 1) {
            item->showSelectedIcon(true);
            visionSelectedPublicCloudModel = model.modelName;
        }
        // 更新按钮可见性和高度
        if (m_visionPublicCloudModelItemMap.count() > 1) {
            m_publicCloudGroup->increaseContentHeight();
        }
        updateVisionPublicCloudModelVisibility();
    } else if (model.capability == AiCapability::Speech) {
        m_speechPublicCloudModelItemList.append(item);
        m_speechPublicCloudModelMap[model.modelName] = model;
        m_speechPublicCloudModelItemMap[model.modelName] = item;
        m_speechPublicCloudMapUI[model.modelName] = itemFrame;
        m_speechPublicCloudModelGroup->insertWidgetAt(1, itemFrame);

        // 第一个添加的模型，设置为选中
        if (m_speechPublicCloudModelItemMap.count() == 1) {
            item->showSelectedIcon(true);
            speechSelectedPublicCloudModel = model.modelName;
        }
        // 更新按钮可见性和高度
        if (m_speechPublicCloudModelItemMap.count() > 1) {
            m_publicCloudGroup->increaseContentHeight();
        }
        updateSpeechPublicCloudModelVisibility();
    }
    m_modelOrderManager.addModel(model.capability, model.modelName);
}

void PrivateModelConfigWidget::deleteModelItem(AiCapability capability, QString modelName) {
    ModelConfig mc;
    DeployType deployType = (capability == AiCapability::Nlp) ? DeployType::Custom : DeployType::Custom;
    QString curModelName = QString::fromStdString(mc.getCurrentModelName(capability, deployType));
    bool isCheckSelected = checkModelSelection(capability, modelName);
    KyInfo() << "currentmodelname" << curModelName << "要删除的modelname" << modelName << "isCheckSelected" << isCheckSelected;

    if (capability == AiCapability::Nlp)
    {
        // 根据模型名称查找其来源组，确定从哪个数据结构删除
        if (m_nlpLocalNetworkModelMap.contains(modelName)) {
            KyInfo() << "============删除选中的局域网模型 localNetwork nlp "
                        "modelName (from local)"
                     << modelName;
            // 删除的是局域网NLP模型
            // 手动删除 ModelItem 对象
            auto item = m_nlpLocalNetworkModelItemList.takeAt(
                m_nlpLocalNetworkModelItemList.indexOf(m_nlpLocalNetworkModelItemMap[modelName]));
            if (m_nlpLocalNetworkModelItemMap.contains(modelName)) {
                if (m_nlpLocalNetworkModelItemMap[modelName]) {
                    delete m_nlpLocalNetworkModelItemMap[modelName];
                    m_nlpLocalNetworkModelItemMap[modelName] = nullptr;
                }
            }
            m_nlpLocalNetworkModelMap.remove(modelName);
            m_nlpLocalNetworkModelItemMap.remove(modelName);
            m_nlpLocalNetworkModelGroup->removeWidget(m_nlpLocalNetworkMapUI[modelName]);
            m_nlpLocalNetworkMapUI.remove(modelName);

            // 从模型来源组配置中移除该模型
            if (m_modelSourceGroups.contains(modelName)) {
                m_modelSourceGroups.remove(modelName);

                // 保存更新后的配置到文件
                QFile configFile(m_configFilePath);
                if (configFile.open(QIODevice::WriteOnly)) {
                    QJsonDocument doc;
                    QJsonObject obj;

                    // 将哈希表转换为JSON对象
                    for (auto it = m_modelSourceGroups.constBegin(); it != m_modelSourceGroups.constEnd(); ++it) {
                        obj[it.key()] = it.value();
                    }

                    doc.setObject(obj);
                    configFile.write(doc.toJson());
                    configFile.close();
                }
            }

            // 如果删除选中模型,需重新设置currentmodelname
            if (curModelName == modelName || isCheckSelected) {
                KyInfo() << "============删除选中的局域网模型 localNetwork nlp "
                            "modelName (from local)"
                         << modelName;
                if (m_nlpLocalNetworkModelItemMap.count() > 0) {
                    // 删除后，设置剩下模型中的第一个为选中(新增在上面，所以选中.last)
                    PrivateModelItemWidget* firstItem =
                        m_nlpLocalNetworkModelItemList.last();
                    QString firstModelName = firstItem->getModelName();

                    KyInfo() << "============删除选中的局域网模型后 "
                                "localNetwork nlp firstModelName (from local)"
                             << firstModelName;

                    bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
                    if (ret == true) {
                        switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                        KyInfo() << modelName << " 删除后设置为选中模型成功";
                    } else {
                        showErrorMessage(tr("Model setup failed"), this);
                        KyInfo() << modelName << " 删除后设置为选中模型失败";
                        return;
                    }
                } else if (m_nlpPublicCloudModelItemMap.count() > 0) {
                    // 当前组没模型了，从公有云组找第一个(新增在上面，所以选中.last)
                    PrivateModelItemWidget* firstItem =
                        m_nlpPublicCloudModelItemList.last();
                    QString firstModelName = firstItem->getModelName();

                    KyInfo() << "============删除后选中的局域网模型后 localNetwork "
                                "nlp firstModelName (from public cloud)"
                             << firstModelName;

                    bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
                    if (ret == true) {
                        switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                        KyInfo() << modelName << " 删除后设置为选中模型成功";
                    } else {
                        showErrorMessage(tr("Model setup failed"), this);
                        KyInfo() << modelName << " 删除后设置为选中模型失败";
                        return;
                    }
                } else if (m_nlpLocalModelItemMap.count() > 0) {
                    // 公有云组和局域网组都没有，从本地组找第一个(新增在上面，所以选中.last)
                    PrivateModelItemWidget* firstItem =
                        m_nlpLocalModelItemList.last();
                    QString firstModelName = firstItem->getModelName();

                    KyInfo() << "============删除后选中的局域网模型后 "
                                "localNetwork nlp firstModelName (from Local)"
                             << firstModelName;

                    bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
                    if (ret == true) {
                        switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                        KyInfo() << modelName << " 删除后设置为选中模型成功";
                    } else {
                        showErrorMessage(tr("Model setup failed"), this);
                        KyInfo() << modelName << " 删除后设置为选中模型失败";
                        return;
                    }
                } else {
                    // 删除后无模型
                    KyInfo() << " 所有mlp 模型已删除";
                    nlpSelectedModel = "";
                }
            }
            // 更新按钮可见性和高度
            updateLocalNetworkNlpModelVisibility();
            if (m_nlpLocalNetworkModelItemMap.count() >= 1) {
                m_localNetworkGroup->decreaseContentHeight();
            }
        } else if (m_nlpPublicCloudModelItemMap.contains(modelName)) {
            KyInfo() << "============删除公有云模型 publicCloud nlp modelName" << modelName;
            // 删除的是公有云NLP模型
            // 手动删除 ModelItem 对象
            auto item = m_nlpPublicCloudModelItemList.takeAt(
                m_nlpPublicCloudModelItemList.indexOf(m_nlpPublicCloudModelItemMap[modelName]));
            if (m_nlpPublicCloudModelItemMap.contains(modelName)) {
                if (m_nlpPublicCloudModelItemMap[modelName]) {
                    delete m_nlpPublicCloudModelItemMap[modelName];
                    m_nlpPublicCloudModelItemMap[modelName] = nullptr;
                }
            }
            m_nlpPublicCloudModelMap.remove(modelName);
            m_nlpPublicCloudModelItemMap.remove(modelName);
            m_nlpPublicCloudModelGroup->removeWidget(m_nlpPublicCloudMapUI[modelName]);
            m_nlpPublicCloudMapUI.remove(modelName);

            // 从模型来源组配置中移除该模型
            if (m_modelSourceGroups.contains(modelName)) {
                m_modelSourceGroups.remove(modelName);

                // 保存更新后的配置到文件
                QFile configFile(m_configFilePath);
                if (configFile.open(QIODevice::WriteOnly)) {
                    QJsonDocument doc;
                    QJsonObject obj;

                    // 将哈希表转换为JSON对象
                    for (auto it = m_modelSourceGroups.constBegin(); it != m_modelSourceGroups.constEnd(); ++it) {
                        obj[it.key()] = it.value();
                    }

                    doc.setObject(obj);
                    configFile.write(doc.toJson());
                    configFile.close();
                }
            }

            // 如果删除选中模型,需重新设置currentmodelname
            if (curModelName == modelName || isCheckSelected) {
                KyInfo() << "============删除选中的公有云模型 publicCloud nlp "
                            "modelName (from public)"
                         << modelName;
                if (m_nlpPublicCloudModelItemMap.count() > 0) {
                    // 删除后，设置剩下模型中的第一个为选中(新增在上面，所以选中.last)
                    PrivateModelItemWidget* firstItem =
                        m_nlpPublicCloudModelItemList.last();
                    QString firstModelName = firstItem->getModelName();

                    KyInfo() << "============删除后选中的公有云网模型后 nlp "
                                "firstModelName (from public)"
                             << firstModelName;

                    bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
                    if (ret == true) {
                        switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                        KyInfo() << modelName << " 设置为选中模型成功";
                    } else {
                        showErrorMessage(tr("Model setup failed"), this);
                        KyInfo() << modelName << " 设置为选中模型失败";
                        return;
                    }

                } else if (m_nlpLocalNetworkModelItemMap.count() > 0) {
                    // 当前组没模型了，从局域网组找第一个(新增在上面，所以选中.last)
                    PrivateModelItemWidget* firstItem =
                        m_nlpLocalNetworkModelItemList.last();
                    QString firstModelName = firstItem->getModelName();

                    KyInfo() << "============删除后选中的公有云网模型后 nlp "
                                "firstModelName (from loaclnetwork)"
                             << firstModelName;

                    bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
                    if (ret == true) {
                        switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                        KyInfo() << modelName << " 设置为选中模型成功";
                    } else {
                        showErrorMessage(tr("Model setup failed"), this);
                        KyInfo() << modelName << " 设置为选中模型失败";
                        return;
                    }
                } else if (m_nlpLocalModelItemMap.count() > 0) {
                    // 公有云组和局域网组都没有，从本地组找第一个(新增在上面，所以选中.last)
                    PrivateModelItemWidget* firstItem =
                        m_nlpLocalModelItemList.last();
                    QString firstModelName = firstItem->getModelName();

                    KyInfo() << "============删除后选中的公有云网模型后 "
                                "localNetwork nlp firstModelName (from local)"
                             << firstModelName;

                    bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
                    if (ret == true) {
                        switchModelItem(firstItem->getAICapability(), firstItem->getModelName());
                        KyInfo() << modelName << " 设置为选中模型成功";
                    } else {
                        showErrorMessage(tr("Model setup failed"), this);
                        KyInfo() << modelName << " 设置为选中模型失败";
                        return;
                    }
                }
            }

            // 更新按钮可见性和高度
            updateNlpPublicCloudModelVisibility();
            if (m_nlpPublicCloudModelItemMap.count() >= 1) {
                m_publicCloudGroup->decreaseContentHeight();
            }
        }
    }
    else if (capability == AiCapability::Vision)
    {
        // 手动删除 ModelItem 对象
        auto item = m_visionPublicCloudModelItemList.takeAt(
            m_visionPublicCloudModelItemList.indexOf(m_visionPublicCloudModelItemMap[modelName]));
        if (m_visionPublicCloudModelItemMap.contains(modelName)) {
            if (m_visionPublicCloudModelItemMap[modelName]) {
                delete m_visionPublicCloudModelItemMap[modelName];
                m_visionPublicCloudModelItemMap[modelName] = nullptr;
            }
        }
        m_visionPublicCloudModelMap.remove(modelName);
        m_visionPublicCloudModelItemMap.remove(modelName);
        m_visionPublicCloudModelGroup->removeWidget(m_visionPublicCloudMapUI[modelName]);
        m_visionPublicCloudMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if(curModelName == modelName || isCheckSelected)
        {
            if (m_visionPublicCloudModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中(新增在上面，所以选中.last)
                PrivateModelItemWidget* firstItem =
                    m_visionPublicCloudModelItemList.last();
                QString firstModelName = firstItem->getModelName();

                KyInfo() << "============删除后 vision firstModelName" << firstModelName;

                bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
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
                visionSelectedPublicCloudModel = "";
            }
        }

        // 更新按钮可见性和高度
        updateVisionPublicCloudModelVisibility();
        if (m_visionPublicCloudModelItemMap.count() >= 1) {
            m_publicCloudGroup->decreaseContentHeight();
        }
    }
    else if (capability == AiCapability::Speech)
    {
        // 手动删除 ModelItem 对象
        auto item = m_speechPublicCloudModelItemList.takeAt(
            m_speechPublicCloudModelItemList.indexOf(m_speechPublicCloudModelItemMap[modelName]));
        if (m_speechPublicCloudModelItemMap.contains(modelName)) {
            if (m_speechPublicCloudModelItemMap[modelName]) {
                delete m_speechPublicCloudModelItemMap[modelName];
                m_speechPublicCloudModelItemMap[modelName] = nullptr;
            }
        }
        m_speechPublicCloudModelMap.remove(modelName);
        m_speechPublicCloudModelItemMap.remove(modelName);
        m_speechPublicCloudModelGroup->removeWidget(m_speechPublicCloudMapUI[modelName]);
        m_speechPublicCloudMapUI.remove(modelName);

        // 如果删除选中模型,需重新设置currentmodelname
        if(curModelName == modelName || isCheckSelected)
        {
            if (m_speechPublicCloudModelItemMap.count() > 0) {
                // 删除后，设置剩下模型中的第一个为选中(新增在上面，所以选中.last)
                PrivateModelItemWidget* firstItem =
                    m_speechPublicCloudModelItemList.last();
                QString firstModelName = firstItem->getModelName();

                KyInfo() << "============删除后 speech firstModelName" << firstModelName;

                bool ret = setSelectPrivateModelName(capability, DeployType::Custom, firstModelName);
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
                speechSelectedPublicCloudModel = "";
            }
        }

        // 更新按钮可见性和高度
        updateSpeechPublicCloudModelVisibility();
        if (m_speechPublicCloudModelItemMap.count() >= 1) {
            m_publicCloudGroup->decreaseContentHeight();
        }
    }
}

void PrivateModelConfigWidget::switchModelItem(AiCapability capability, QString modelName)
{
    // QMap<QString, PrivateModelItemWidget*> *modelItemMap = nullptr;

    if (capability == AiCapability::Nlp) {
        if (m_nlpLocalNetworkModelMap.contains(modelName)) {
            // 局域网NLP模型
            nlpSelectedModel = modelName;
        } else if (m_nlpPublicCloudModelMap.contains(modelName)) {
            // 公有云NLP模型
            nlpSelectedModel = modelName;
        } else if (m_nlpLocalModelMap.contains(modelName)) {
            // 本地NLP模型
            nlpSelectedModel = modelName;
        }
        foreach (const auto& modelItemName, m_nlpPublicCloudModelItemMap.keys()) {
            PrivateModelItemWidget* modelItem = m_nlpPublicCloudModelItemMap.value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);
            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
        foreach (const auto& modelItemName, m_nlpLocalNetworkModelItemMap.keys()) {
            PrivateModelItemWidget* modelItem = m_nlpLocalNetworkModelItemMap.value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);
            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
        foreach (const auto& modelItemName, m_nlpLocalModelItemMap.keys()) {
            PrivateModelItemWidget* modelItem = m_nlpLocalModelItemMap.value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);
            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
    } else if (capability == AiCapability::Vision) {
        visionSelectedPublicCloudModel = modelName;
        // 处理Vision模型的UI状态更新
        foreach (const auto& modelItemName, m_visionPublicCloudModelItemMap.keys()) {
            PrivateModelItemWidget* modelItem = m_visionPublicCloudModelItemMap.value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);
            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
    } else if (capability == AiCapability::Speech) {
        speechSelectedPublicCloudModel = modelName;
        // 处理Speech模型的UI状态更新
        foreach (const auto& modelItemName, m_speechPublicCloudModelItemMap.keys()) {
            PrivateModelItemWidget* modelItem = m_speechPublicCloudModelItemMap.value(modelItemName);
            if (modelItem->getModelName() == modelName) {
                KyInfo() << "匹配成功，展示图标";
                modelItem->showSelectedIcon(true);
            } else {
                // 不匹配，隐藏图标
                KyInfo() << "不匹配，隐藏图标";
                modelItem->showSelectedIcon(false);
            }
        }
    }
}

// 添加模型并设为当前选中模型
bool PrivateModelConfigWidget::setSelectPrivateModelConfig(PrivateModel model)
{
    ModelConfig mc;
    bool ret1 = mc.addCustomModel(model.modelName.toStdString(), model.modelVersion.toStdString(),
                                  model.apiKey.toStdString(), model.apiUrl.toStdString());
    if (ret1 == true)
    {
        bool ret2 = setSelectPrivateModelName(model.capability, DeployType::Custom, model.modelName);
        if (ret2 == false)
        {
            KyInfo() << "setCurrentModelName failed:" << ret2;
            return false;
        }
    } else {
        KyInfo() << "addCustomModel failed:" << ret1;
        return false;
    }
    return true;
}

bool PrivateModelConfigWidget::setSelectCloudModelConfig(PrivateModel model, bool continuousSelectedState,
                                                         bool realtimeSelectedState) {
    KyInfo() << "setSelectCloudModelConfig----" << model.modelName;

    bool isValueEmpty = false;

    for (const auto& authen : model.multiAuths) {
        for (const auto& au : authen.authentications) {
            if (au.key == "appIdRealTime" || au.key == "apiKeyRealTime") {
                if (au.value == "" && realtimeSelectedState == true) {
                    isValueEmpty = true;
                }
            } else if (au.key == "appIdContinuous" || au.key == "apiKeyContinuous" || au.key == "secretKeyContinuous") {
                if (au.value == "" && continuousSelectedState == true) {
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
    bool ret1;

    if (model.capability == AiCapability::Speech) {
        // 区分不同语音模型类型，只对讯飞语音模型使用扩展认证信息
        if (model.modelName == "讯飞-语音大模型") {
            // 对于讯飞语音模型，需要保存认证信息和功能启用状态
            std::vector<ExtendedMultiAuthentication> extendedAuths;

            // 处理"语音听写&语音合成"项
            ExtendedMultiAuthentication continuousAuth;
            continuousAuth.itemName = "语音听写&语音合成";
            continuousAuth.enabled = continuousSelectedState;
            // 从当前模型认证信息中复制相关认证字段
            for (const auto& auth : model.multiAuths) {
                if (auth.itemName == "语音听写&语音合成") {
                    continuousAuth.authentications = auth.authentications;
                    break;
                }
            }
            extendedAuths.push_back(continuousAuth);

            // 处理"实时语音转写"项
            ExtendedMultiAuthentication realTimeAuth;
            realTimeAuth.itemName = "实时语音转写";
            realTimeAuth.enabled = realtimeSelectedState;
            // 从当前模型认证信息中复制相关认证字段
            for (const auto& auth : model.multiAuths) {
                if (auth.itemName == "实时语音转写") {
                    realTimeAuth.authentications = auth.authentications;
                    break;
                }
            }
            extendedAuths.push_back(realTimeAuth);

            // 保存扩展认证信息（含功能启用状态和认证字段）
            ret1 = mc.setModelAuthentications(model.modelName.toStdString(), extendedAuths);
            KyInfo() << "Saving extended authentication info for Xunfei model, success:" << ret1;
        } else {
            // 其他语音模型，使用setModelAuthentications
            ret1 = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
        }
    } else {
        // 非语音模型，使用setModelAuthentications
        ret1 = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
    }

    if (ret1 == true) {
        bool ret2 = setSelectCloudModelName(model.capability, DeployType::Custom, model.modelName);
        if (ret2 == false) {
            return false;
        }
    } else {
        KyInfo() << "setModelAuthentications failed:" << ret1;
        return false;
    }
    return true;
}

bool PrivateModelConfigWidget::setSelectCloudModelName(AiCapability capability, DeployType deployType,
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

// 设置选中的模型
bool PrivateModelConfigWidget::setSelectPrivateModelName(AiCapability capability, DeployType deployType, QString modelName)
{
    KyInfo() << "setSelectPrivateModelName" << modelName;
    ModelConfig mc;
    bool ret2 = mc.setCurrentModelName(capability, deployType, modelName.toStdString());

    if ( ret2 == true)
    {
        KyInfo() << "setCurrentModelName success:"<< ret2;
    } else {
        KyInfo() << "setCurrentModelName failed:"<< ret2;
    }
    return ret2;
}

// 编辑选中的模型
bool PrivateModelConfigWidget::editSelectPrivateModel(PrivateModel model, bool isContinuousSelected,
                                                      bool isRealTimeSelected, bool isValueChanged, QDialog *parentDialog) {
    bool isSwitchStateChanged = false;

    if (model.capability == AiCapability::Speech) {
        if (continuousSelected != isContinuousSelected) {
            isSwitchStateChanged = true;
        }
        if (realtimeSelected != isRealTimeSelected) {
            isSwitchStateChanged = true;
        }
    }

    while (getModelStatus(model.modelName) == ModelStatus::Running) {
        // NLP or VISION or SPEECH 正在使用，重试弹窗
        KyInfo() << "正在使用" << static_cast<int>(model.capability);

        if (isValueChanged == false && isSwitchStateChanged == false) {
            KyInfo() << "检查无修改内容，仅查看，关闭弹窗";
            return true;
        }

        bool canProceed = showModelInUseMessage(parentDialog);
        if (canProceed == true) {
            continue;
        } else {
            KyInfo() << "取消本次操作";
            return false;
        }
    }

    // 未使用
    // 当前选中的，无需再setCurrentModelName
    // 更新当前模型信息
    bool ret =  this->updatePrivateModel(model);
    if (ret == true) {
        // 发送信号告知已设置模型，重新创建会话
        if (model.capability == AiCapability::Nlp) {
            if (m_nlpLocalNetworkModelMap.contains(model.modelName)) {
                // 局域网NLP模型
                m_nlpLocalNetworkModelMap[model.modelName] = model;
                saveModelSourceGroup(model.modelName, "localNetwork");
            } else {
                // 公有云NLP模型
                m_nlpPublicCloudModelMap[model.modelName] = model;
                saveModelSourceGroup(model.modelName, "publicCloud");
            }
        } else if (model.capability == AiCapability::Vision) {
            m_visionPublicCloudModelMap[model.modelName] = model;

        } else if (model.capability == AiCapability::Speech) {
            m_speechPublicCloudModelMap[model.modelName] = model;
        }

        if (model.capability == AiCapability::Speech) {
            // 同步两个开关的状态
            continuousSelected = isContinuousSelected;
            realtimeSelected = isRealTimeSelected;
        }

    } else {
        showErrorMessage(tr("Model Editing failed"), parentDialog);
        KyInfo() << "编辑模型" << model.modelName << "失败";
        return false;
    }
    return true;
}
void PrivateModelConfigWidget::processModelInfo(std::vector<ExtendedModel>& models, AiCapability aiCapability) {
    std::vector<ExtendedModel> orderedList;
    std::vector<ExtendedModel> unorderedList;
    // 保存自选模型信息
    QMap<QString, ExtendedModel> unOrderedListMap;
    for (auto& model : models) {
        if (model.deployType == DeployType::Custom) {
            // 对于nlp模型不检查key是否为空
            bool shouldCheckKey = (aiCapability == AiCapability::Vision || aiCapability == AiCapability::Speech);
            if (!shouldCheckKey || (shouldCheckKey && !keyIsEmpty(model.modelName.c_str()))) {
                unOrderedListMap[model.modelName.c_str()] = model;
                unorderedList.push_back(model);
            }
        }
    }
    if (unOrderedListMap.size() <= 1) {
        models = unorderedList;
        return;
    }
    QStringList modelNameOrderList = m_modelOrderManager.getModelOrder(aiCapability);
    if (modelNameOrderList.size() != unOrderedListMap.size()) {
        KyWarning() << "The number of models and the saved ones are different." << modelNameOrderList
                    << unOrderedListMap.keys();
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

bool PrivateModelConfigWidget::keyIsEmpty(const QString& modelName) {
    ModelConfig mc;
    std::vector<MultiAuthentication> multiAuths = mc.getModelAuthentications(modelName.toStdString());
    bool isValueEmpty = false;
    bool isContinuousValueEmpty = false;
    bool isRealtimeValueEmpty = false;

    for (const auto& authen : multiAuths) {
        for (const auto& au : authen.authentications) {
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
        if (isValueEmpty) {
            return true;
        }
    }
    return false;
}

bool PrivateModelConfigWidget::editSelectCloudModel(PrivateModel model, bool isContinuousSelected,
                                                    bool isRealTimeSelected, bool isValueChanged,
                                                    QDialog* parentDialog) {
    bool isSwitchStateChanged = false;

    if (model.capability == AiCapability::Speech) {
        if (continuousSelected != isContinuousSelected) {
            isSwitchStateChanged = true;
        }
        if (realtimeSelected != isRealTimeSelected) {
            isSwitchStateChanged = true;
        }
    }

    while (getModelStatus(model.modelName) == ModelStatus::Running) {
        // NLP or VISION or SPEECH 正在使用，重试弹窗
        KyInfo() << "正在使用" << static_cast<int>(model.capability);

        if (isValueChanged == false && isSwitchStateChanged == false) {
            KyInfo() << "检查无修改内容，仅查看，关闭弹窗";
            return true;
        }

        bool canProceed = showModelInUseMessage(parentDialog);
        if (canProceed == true) {
            continue;
        } else {
            KyInfo() << "取消本次操作";
            return false;
        }
    }

    // 未使用
    ModelConfig mc;
    bool ret;

    if (model.capability == AiCapability::Speech) {
        // 区分不同语音模型类型，只对讯飞语音模型使用扩展认证信息
        if (model.modelName == "讯飞-语音大模型") {
            std::vector<ExtendedMultiAuthentication> extendedAuths;

            // 处理"语音听写&语音合成"项
            ExtendedMultiAuthentication continuousAuth;
            continuousAuth.itemName = "语音听写&语音合成";
            continuousAuth.enabled = isContinuousSelected;
            // 从当前模型认证信息中复制相关认证字段
            for (const auto& auth : model.multiAuths) {
                if (auth.itemName == "语音听写&语音合成") {
                    continuousAuth.authentications = auth.authentications;
                    break;
                }
            }
            extendedAuths.push_back(continuousAuth);

            // 处理"实时语音转写"项
            ExtendedMultiAuthentication realTimeAuth;
            realTimeAuth.itemName = "实时语音转写";
            realTimeAuth.enabled = isRealTimeSelected;
            // 从当前模型认证信息中复制相关认证字段
            for (const auto& auth : model.multiAuths) {
                if (auth.itemName == "实时语音转写") {
                    realTimeAuth.authentications = auth.authentications;
                    break;
                }
            }
            extendedAuths.push_back(realTimeAuth);

            // 保存扩展认证信息（含功能启用状态和认证字段）
            ret = mc.setModelAuthentications(model.modelName.toStdString(), extendedAuths);
            KyInfo() << "Saving extended authentication info for Xunfei model, success:" << ret;
        } else {
            // 其他语音模型，使用setModelAuthentications
            ret = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
        }
    } else {
        // 非语音模型，使用setModelAuthentications
        ret = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
    }

    if (ret == true) {
        // 发送信号告知已设置模型，重新创建会话
        if (model.capability == AiCapability::Nlp) {
            m_nlpPublicCloudModelMap[model.modelName] = model;

        } else if (model.capability == AiCapability::Vision) {
            m_visionPublicCloudModelMap[model.modelName] = model;

        } else if (model.capability == AiCapability::Speech) {
            m_speechPublicCloudModelMap[model.modelName] = model;
        }

        if (model.capability == AiCapability::Speech) {
            // 同步两个开关的状态
            continuousSelected = isContinuousSelected;
            realtimeSelected = isRealTimeSelected;
        }
    } else {
        showErrorMessage(tr("Model Editing failed"), parentDialog);
        KyInfo() << "编辑模型" << model.modelName << "失败";
        return false;
    }
    return true;
}

void PrivateModelConfigWidget::clickPrivateModel(AiCapability capability, QString modelName)
{
    KyInfo() << "正要切换到" << static_cast<int>(capability) << modelName ;
    KyInfo() << "当前正在回答的选中模型为" << nlpSelectedModel;

    bool ret = checkModelSelection(capability, modelName);

    if(ret == true){
        return;
    }
    else {
        if ((capability == AiCapability::Nlp && (getModelStatus(nlpSelectedModel) == ModelStatus::Running)) ||
            (capability == AiCapability::Vision &&
             (getModelStatus(visionSelectedPublicCloudModel) == ModelStatus::Running)) ||
            (capability == AiCapability::Speech &&
             (getModelStatus(speechSelectedPublicCloudModel) == ModelStatus::Running))) {
            // NLP or VISION or SPEECH 正在使用
            KyInfo() << "正在使用" << static_cast<int>(capability);
            bool canProceed = showModelInUseMessage(this);
            if (canProceed == true) {
                clickPrivateModel(capability, modelName);
            } else {
                 KyInfo() << "取消本次操作";
                return;
            }
        } else {
            bool ret = setSelectPrivateModelName(capability, DeployType::Custom, modelName);
            if (ret == true) {
                switchModelItem(capability, modelName);
                KyInfo() << modelName << " 设置为选中模型成功";
            } else {
                showErrorMessage(tr("Model switching failed"),this);
                KyInfo() << modelName << " 设置为选中模型失败";
            }
        }
    }
}

bool PrivateModelConfigWidget::clearModelMultiAuths(AiCapability capability, QString modelName)
{
    ModelConfig mc;
    bool ret = mc.clearModelAuthentications(modelName.toStdString());

    return ret;
}

bool PrivateModelConfigWidget::clearCustomModel(AiCapability capability, QString modelName)
{
    ModelConfig mc;
    bool ret = mc.deleteCustomModel(modelName.toStdString());

    return ret;
}

bool PrivateModelConfigWidget::addPrivateModel(PrivateModel model)
{
    ModelConfig mc;
    bool ret = mc.addCustomModel(model.modelName.toStdString(), model.modelVersion.toStdString(),
                                  model.apiKey.toStdString(), model.apiUrl.toStdString());
    if (ret == true)
    {
        return true;
    } else {
        KyInfo() << "addPrivateModel failed:" << ret;
        return false;
    }

}

bool PrivateModelConfigWidget::updatePrivateModel(PrivateModel model)
{
    ModelConfig mc;
    bool ret1 = mc.setCustomModelApiUrl(model.modelName.toStdString(), model.apiUrl.toStdString());
    bool ret2 = mc.setCustomModelVersion(model.modelName.toStdString(), model.modelVersion.toStdString());
    bool ret3 = mc.setModelAuthentications(model.modelName.toStdString(), model.multiAuths);
    if (ret1 == true && ret2 == true && ret3 == true)
    {
        return true;
    } else {
        KyInfo() << "updateCustomModel failed"<<"ret1"<<ret1<<"ret2"<<ret2<<"ret3"<<ret3;
        return false;
    }

}

bool PrivateModelConfigWidget::checkModelSelection(AiCapability capability, QString modelName)
{
    if (capability == AiCapability::Nlp) {
        return (modelName == nlpSelectedModel);
    } else if (capability == AiCapability::Vision) {
        return (modelName == visionSelectedPublicCloudModel);
    } else if (capability == AiCapability::Speech) {
        return (modelName == speechSelectedPublicCloudModel);
    } else {
        return false;
    }
}

bool PrivateModelConfigWidget::polkit()
{
    PolkitQt1::Authority::Result result;
    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.kylin-ai-subsystem-modelconfig.policy",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);
    KyInfo() << "PolkitQt1 Authority result:" << result;
    if (result == PolkitQt1::Authority::Yes) { //认证通过
        KyInfo() << QString("operation authorized");
        return true;
    } else {
        KyInfo() << QString("not authorized");
        return false;
    }

}

QMap<QString, PrivateModelItemWidget *> &PrivateModelConfigWidget::getModelItemMap(AiCapability capability)
{
    if (capability == AiCapability::Nlp) {
        return m_nlpPublicCloudModelItemMap;
    } else if (capability == AiCapability::Vision) {
        return m_visionPublicCloudModelItemMap;
    } else if (capability == AiCapability::Speech) {
        return m_speechPublicCloudModelItemMap;
    }
}

void PrivateModelConfigWidget::onNlpPublicCloudAddClicked() {
    PrivateModelConfigDialog* settingDialog = new PrivateModelConfigDialog(this);
    settingDialog->initUIData(AiCapability::Nlp, DeployType::Custom, "publicCloud");
    settingDialog->setModal(true);
    settingDialog->show();
    QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
}

void PrivateModelConfigWidget::onNlpPublicCloudAddItemClicked() { onNlpPublicCloudAddClicked(); }

void PrivateModelConfigWidget::onVisionPublicCloudAddClicked() {
    ModelConfigDialog* settingDialog = new ModelConfigDialog(this);
    settingDialog->initUIData(AiCapability::Vision, DeployType::Custom);
    settingDialog->setModal(true);
    settingDialog->show();
    QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
}

void PrivateModelConfigWidget::onVisionPublicCloudAddItemClicked() { onVisionPublicCloudAddClicked(); }

void PrivateModelConfigWidget::onSpeechPublicCloudAddClicked() {
    ModelConfigDialog* settingDialog = new ModelConfigDialog(this);
    settingDialog->initUIData(AiCapability::Speech, DeployType::Custom);
    settingDialog->setModal(true);
    settingDialog->show();
    QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
}

void PrivateModelConfigWidget::onSpeechPublicCloudAddItemClicked() { onSpeechPublicCloudAddClicked(); }

void PrivateModelConfigWidget::onLocalNetworkNlpAddClicked() {
    PrivateModelConfigDialog* settingDialog = new PrivateModelConfigDialog(this);
    settingDialog->initUIData(AiCapability::Nlp, DeployType::Custom, "localNetwork");
    settingDialog->setModal(true);
    settingDialog->show();
    QTimer::singleShot(30, settingDialog, SLOT(moveToCenter()));
}

void PrivateModelConfigWidget::onLocalNetworkNlpAddItemClicked() { onLocalNetworkNlpAddClicked(); }

void PrivateModelConfigWidget::showCentered()
{
   if (QWidget* mainWindow = QApplication::activeWindow())
   {
       QRect mainGeom = mainWindow->geometry();
       move(mainGeom.x() + (mainGeom.width() - width()) / 2,
       mainGeom.y() + (mainGeom.height() - height()) / 2 + 15);
   }
   adjustPosition(this); // 调用内部调整逻辑
}
