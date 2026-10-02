#include "modelwidget.h"
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <aiconfig/modelconfig.h>
#include <ukui-log4qt.h>
#include <QFontMetrics>

#include "util.h"

#define THEME_QT_SCHEMA  "org.ukui.style"

ModelWidget::ModelWidget(const QString &modelName, const QString &modelIntro, DeployType deployType, QWidget *parent)
    : QWidget(parent), m_deployType(deployType)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    m_modelLayout = new QVBoxLayout;
    m_modelLayout->setSpacing(0);
    m_modelLayout->setContentsMargins(0, 0, 0, 0);

    m_sortButton = new QPushButton(this);
    m_sortButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_m_sortButton");
    m_sortButton->setFixedSize(QSize(36, 36));
    m_sortButton->setToolTip(tr("Increase Priority"));
    m_sortButton->setFocusPolicy(Qt::NoFocus);
    m_sortButton->setIcon(QIcon::fromTheme("go-up-symbolic"));
    m_sortButton->setProperty("useIconHighlightEffect", 0x2);
    m_sortButton->setFlat(true);

    modelNameLabel = new QLabel(modelName, this);
    modelNameLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_modelNameLabel");
    modelNameLabel->setAlignment(Qt::AlignLeft);

    modelIntroLabel = new kdk::KLabel(this);
    modelIntroLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KLabel_modelIntroLabel");
    modelIntroLabel->setText(modelIntro);
    modelIntroLabel->setAlignment(Qt::AlignLeft);
    setKLabelFontColor(modelIntroLabel, "#808080");

    // 创建认证标签
    modelAuthLabel = new kdk::KLabel(this);
    modelAuthLabel->setObjectName("modelAuthLabel");

    // 根据部署类型设置标签文本和样式
    QString labelText, styleSheet;
    if (m_deployType == DeployType::PublicCloud || m_deployType == DeployType::Ondevice) {
        // 公有云或本地模型 - 显示"麒麟认证"
        labelText = tr("Kylin Certification");
        styleSheet =
            "QLabel { "
            "background-color: #E8F3FF; "  // 淡蓝色背景
            "border-radius: 3px; "         // 圆角
            "padding: 2px 6px; "           // 内边距
            "font-size: 11px; "            // 字体大小
            "min-height: 16px; "           // 最小高度
            "max-height: 16px; "           // 最大高度
            "}";
    } else if (m_deployType == DeployType::Custom) {
        // 自定义模型 - 显示"用户自选"
        labelText = tr("User Selected");
        styleSheet =
            "QLabel { "
            "background-color: #F5F5F5; "  // 淡灰色背景
            "border-radius: 3px; "         // 圆角
            "padding: 2px 6px; "           // 内边距
            "font-size: 11px; "            // 字体大小
            "min-height: 16px; "           // 最小高度
            "max-height: 16px; "           // 最大高度
            "}";
    }

    modelAuthLabel->setText(labelText);
    modelAuthLabel->setStyleSheet(styleSheet);
    modelAuthLabel->setAlignment(Qt::AlignCenter);

    // 使用setKLabelFontColor函数设置字体颜色
    if (m_deployType == DeployType::PublicCloud || m_deployType == DeployType::Ondevice) {
        // 公有云或本地模型 - 蓝色文字
        setKLabelFontColor(modelAuthLabel, "#3790FA");
    } else if (m_deployType == DeployType::Custom) {
        // 自定义模型 - 灰色文字
        setKLabelFontColor(modelAuthLabel, "#808080");
    }

    // 修改模型名称部分的布局，使用水平布局将名称和认证标签并排
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(6);
    nameLayout->addWidget(modelNameLabel);
    nameLayout->addWidget(modelAuthLabel);
    nameLayout->addStretch();

    // 创建一个容器来包含名称和认证标签的水平布局
    QWidget* nameContainer = new QWidget(this);
    nameContainer->setLayout(nameLayout);

    QLayoutItem* child;
    while ((child = m_modelLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    m_modelLayout->addStretch();
    m_modelLayout->addWidget(nameContainer);  // 添加包含名称和认证标签的容器
    m_modelLayout->addWidget(modelIntroLabel);
    m_modelLayout->addStretch();

    tipButton = new QPushButton();
    tipButton->setIcon(QIcon::fromTheme("help-about-symbolic"));
    tipButton->setFixedSize(16, 16);
    tipButton->setEnabled(false);
    tipButton->setVisible(false);
    tipButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
    tipButton->setToolTip(
      tr("The computing power sharing model has been connected, and the local model has been automatically disabled"));
    tipButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QPushButton_tipButton");

    configButton = new kdk::KBorderlessButton; // bug #288117 加大小设置会导致切换字体/字体大小显示"..."
    configButton->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_configButton");
    configButton->setText(tr("Configure"));

    viewButton = new kdk::KBorderlessButton;
    viewButton->setText(tr("View"));
    viewButton->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KBorderlessButton_viewButton");

    modelSwitchBtn = new KSwitchButton(this);
    modelSwitchBtn->setAccessibleName("kylin-ai-subsystem-modelconfig_kdk::KSwitchButton_modelSwitchBtn");
    modelSwitchBtn->setCheckable(true);
    modelSwitchBtn->setFixedSize(50, 24);

    ModelConfig mc;
    // 获取当前模型的部署是否使能
    modelSwitchBtn->setChecked(mc.getModelDeployEnabled(deployType));

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 0, 16, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_sortButton);
    m_layout->addSpacing(8);
    m_layout->addLayout(m_modelLayout);
    m_layout->addStretch();
    m_layout->addSpacing(24);
    m_layout->addWidget(tipButton);
    m_layout->addSpacing(9);

    if (m_deployType == DeployType::Shared) {
        m_layout->addWidget(viewButton);
    } else {
        m_layout->addWidget(configButton);
    }

    m_layout->addSpacing(8);
    m_layout->addWidget(modelSwitchBtn);
    setLayout(m_layout);

    QFontMetrics modelNameLabelFontMetrics(modelNameLabel->font());
    QFontMetrics modelIntroLabelFontMetrics(modelIntroLabel->font());
    this->setFixedHeight(modelIntroLabelFontMetrics.height() + modelNameLabelFontMetrics.height() + 32);

    initConnect();
}

ModelWidget::~ModelWidget()
{

}

QSize ModelWidget::sizeHint() const
{
    return  this->size();
}

void ModelWidget::initConnect()
{
    connect(m_sortButton, &QPushButton::clicked, this, &ModelWidget::onSortButtonClicked);
    connect(modelSwitchBtn, &KSwitchButton::stateChanged, this, &ModelWidget::onTransModelDeployStatusChanged);
    connect(configButton, &QPushButton::clicked, this, [this](){
        emit configClicked(modelWidgetName);
    });
    connect(viewButton, &QPushButton::clicked, this, [this](){
        emit configClicked(modelWidgetName);
    });

    const QByteArray idd(THEME_QT_SCHEMA);
    if (!QGSettings::isSchemaInstalled(idd)) {
        qWarning() << idd << " is not installed.";
        return;
    }
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    connect(qtSettings, &QGSettings::changed, this, [this](const QString &key) {
        if (key == "systemFontSize") {
            QFontMetrics modelNameLabelFontMetrics(modelNameLabel->font());
            QFontMetrics modelIntroLabelFontMetrics(modelIntroLabel->font());
            this->setFixedHeight(modelIntroLabelFontMetrics.height() + modelNameLabelFontMetrics.height() + 32);
        }
    });
}

void ModelWidget::onTransModelDeployStatusChanged(bool enable) {
    if (!enable && m_deployType == DeployType::Ondevice) {
        emit closeModelSwitchBtn();
        return;
    }

    ModelConfig mc;
    // 设置部署策略并检查是否成功
    bool ret = mc.setModelDeployEnabled(m_deployType, enable);

    //错误处理
    if (!ret) {
        KyInfo() << "Error: Failed to modify PublicCloud model deployment strategy.";
    }

    return;
}

void ModelWidget::onSortButtonClicked() {
    emit sortClicked(); // 发送排序点击信号
}

void ModelWidget::setModelSwitchBtnChecked(bool checked) {
    modelSwitchBtn->blockSignals(true);
    modelSwitchBtn->setChecked(checked);
    modelSwitchBtn->blockSignals(false);
}

void ModelWidget::setModelSwitchBtnEnable(bool enable) {
    modelSwitchBtn->blockSignals(true);
    modelSwitchBtn->setEnabled(enable);
    modelSwitchBtn->blockSignals(false);
}

void ModelWidget::setConfigButtonText(const QString& text) {
    configButton->setText(text);
}

void ModelWidget::setTipButtonVisible(bool visible) {
    tipButton->setVisible(visible);
}
