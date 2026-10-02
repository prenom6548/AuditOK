#include "sharedmodelconfigwidget.h"

#include "aiconfig/modelconfig.h"
#include "ukccframe.h"
#include "util.h"

SharedModelConfigWidget::SharedModelConfigWidget(QWidget *parent) : kdk::KDialog(parent) { initLayout(); }

void SharedModelConfigWidget::initLayout() {
    this->setFixedSize(544, 610);
    this->setWindowTitle(tr("Computing Power Sharing Model"));

    m_mainLayout = this->mainLayout();
    m_mainVLayout = new QVBoxLayout();
    m_mainLayout->addLayout(m_mainVLayout);

    initModelTitleLayout();
    initModellayout();
    initConnect();

    m_mainVLayout->setStretch(0, 2);
    m_mainVLayout->setStretch(1, 8);
}

void SharedModelConfigWidget::initModelTitleLayout() {
    QLabel *backgroundLabel = new QLabel(this);
    backgroundLabel->setFixedHeight(72);
    backgroundLabel->setPixmap(QPixmap(":/res/icon/background.svg"));
    backgroundLabel->setAlignment(Qt::AlignCenter);
    backgroundLabel->setScaledContents(false);
    backgroundLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_backgroundLabel");

    // 创建水平布局
    QHBoxLayout *m_titleHLayout = new QHBoxLayout(backgroundLabel);
    m_titleHLayout->setContentsMargins(40, 0, 40, 0);

    // 左侧图标布局
    QLabel *m_iconLabel = new QLabel(backgroundLabel);
    m_iconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_iconLabel");
    QPixmap iconPixmap(":/res/icon/ukui-shared-model-symbolic.svg");
    if (!iconPixmap.isNull()) {
        iconPixmap = iconPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_iconLabel->setPixmap(iconPixmap);
    }
    m_iconLabel->setFixedSize(24, 24);

    // 右侧文本布局
    QVBoxLayout *m_localHLayout = new QVBoxLayout();
    m_localHLayout->setContentsMargins(0, 12, 0, 16);
    m_localHLayout->setSpacing(0);

    QLabel *m_localLabel = new QLabel(backgroundLabel);
    m_localLabel->setMinimumSize(216, 26);
    m_localLabel->setText(tr("Computing Power Sharing Model"));
    QFont localFont;
    localFont.setBold(true);
    m_localLabel->setFont(localFont);
    m_localLabel->setStyleSheet("color: #3790FA; background: transparent;");
    m_localLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_localLabel");

    kdk::KLabel *m_descripLabel = new kdk::KLabel(backgroundLabel);
    m_descripLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_descripLabel");
    m_descripLabel->setMinimumSize(450, 20);
    m_descripLabel->setText(
        tr("Configure models from computing power sharing services (run through shared computing power nodes, "
           "obtain computing resources on demand, and complete inference"));
    QFont localDescripFont;
    localDescripFont.setPointSize(localFont.pointSize() - 2);
    m_descripLabel->setFont(localDescripFont);
    setKLabelFontColor(m_descripLabel, "#3790FA");

    m_localHLayout->addWidget(m_localLabel);
    m_localHLayout->addWidget(m_descripLabel);

    // 组合布局
    m_titleHLayout->addWidget(m_iconLabel);
    m_titleHLayout->addLayout(m_localHLayout);

    // 添加到主垂直布局
    m_mainVLayout->addWidget(backgroundLabel);
}

void SharedModelConfigWidget::initModellayout() {
    m_baseVLayout = new QVBoxLayout(this);
    m_baseVLayout->setContentsMargins(24, 0, 24, 0);

    QFont font;
    font.setBold(true);  // 设置字体加粗
    QLabel *m_nlpLabel = new QLabel();
    m_nlpLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_nlpLabel");
    m_nlpLabel->setText(tr("NLP Model"));
    m_nlpLabel->setFont(font);

    QPushButton *tipButton = new QPushButton();
    tipButton->setIcon(QIcon::fromTheme("help-about-symbolic"));
    tipButton->setFixedSize(16, 16);
    tipButton->setEnabled(false);
    tipButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    tipButton->setToolTip(tr("Process and generate text, such as comprehending text and answering questions."));
    tipButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_tipButton");

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(16, 28, 0, 5);
    hLayout->addWidget(m_nlpLabel);
    hLayout->addSpacing(5);
    hLayout->addWidget(tipButton);
    hLayout->addStretch();

    UkccFrame *itemFrame = new UkccFrame(this);
    itemFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_UkccFrame_itemFrame");
    itemFrame->setMinimumWidth(496);
    itemFrame->setMaximumWidth(496);
    itemFrame->setFixedHeight(56);
    QHBoxLayout *layItem = new QHBoxLayout(itemFrame);
    layItem->setSpacing(0);
    layItem->setContentsMargins(16, 0, 16, 0);

    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/res/icon/qwen.svg"));

    QLabel *m_otherFrameLabelText = new QLabel(itemFrame);
    m_otherFrameLabelText->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_frameLabel");

    kyai::config::model::ModelConfig mc;
    m_otherFrameLabelText->setText(mc.getSharedModelName().c_str());

    layItem->addWidget(iconLabel);
    layItem->addSpacing(4);
    layItem->addWidget(m_otherFrameLabelText);
    layItem->addStretch();

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(24, 0, 24, 24);

    m_openComputePowerShareBtn = new kdk::KBorderlessButton();
    m_openComputePowerShareBtn->setText(tr("Set up computing power sharing"));

    m_confirmLocalButton = new QPushButton();
    m_confirmLocalButton->setText(tr("Confirm"));
    m_confirmLocalButton->setProperty("isImportant", true);

    bottomLayout->addWidget(m_openComputePowerShareBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_confirmLocalButton);

    m_baseVLayout->addLayout(hLayout);
    m_baseVLayout->addWidget(itemFrame);
    m_baseVLayout->addStretch();
    m_mainVLayout->addLayout(m_baseVLayout);
    m_mainVLayout->addStretch();
    m_mainLayout->addLayout(bottomLayout);
}

void SharedModelConfigWidget::initConnect() {
    connect(m_openComputePowerShareBtn, &kdk::KBorderlessButton::clicked, [this]() {
        this->close();
        emit openComputePowerShare();
    });

    connect(m_confirmLocalButton, &QPushButton::clicked, [this]() { this->close(); });
}
