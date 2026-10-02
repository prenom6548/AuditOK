#include "expandablemodelgroup.h"

#include <QApplication>
#include <QCoreApplication>
#include <QVariant>

#include "util.h"

ExpandableModelGroup::ExpandableModelGroup(const QString& title, const QString& subtitle,
                                           kyai::config::model::DeployType deployType, QWidget* parent)
    : QWidget(parent), m_deployType(deployType), m_isExpanded(false) {
    initLayout();

    // 设置标题
    m_titleLabel->setText(title);

    // 设置描述文本
    m_descriptionLabel->setText(subtitle);
}

void ExpandableModelGroup::initLayout() {
    // 创建主框架，设置固定大小和样式
    m_mainFrame = new QWidget(this);
    m_mainFrame->setMaximumWidth(496);
    m_mainFrame->setFixedHeight(76);  // 初始高度只包含标题
    m_mainFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_mainFrame");
    m_mainFrame->setStyleSheet(
        "background-color: palette(window);"
        "border-radius: 6px;");

    // 主垂直布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_mainFrame);

    // 标题区域
    m_titleFrame = new QFrame(m_mainFrame);
    m_titleFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_m_titleFrame");

    // 标题区域的水平布局
    m_titleLayout = new QHBoxLayout(m_titleFrame);
    m_titleLayout->setContentsMargins(16, 8, 16, 8);  // 上下边距8px
    m_titleLayout->setSpacing(0);

    // 创建一个垂直布局容器来放置标题和描述文本
    QWidget* titleAndDescriptionWidget = new QWidget(m_titleFrame);
    QVBoxLayout* titleAndDescriptionLayout = new QVBoxLayout(titleAndDescriptionWidget);
    titleAndDescriptionLayout->setContentsMargins(0, 3, 0, 0);
    titleAndDescriptionLayout->setSpacing(4);  // 设置上下组件间距为4px

    // 标题标签
    m_titleLabel = new QLabel(m_titleFrame);
    m_titleLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_titleLabel");
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    // 添加描述文本标签
    m_descriptionLabel = new QLabel(m_titleFrame);
    m_descriptionLabel->setObjectName("kylin-ai-subsystem-modelconfig_QLabel_descriptionLabel");
    m_descriptionLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_descriptionLabel");
    m_descriptionLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_descriptionLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    m_descriptionLabel->setWordWrap(true);  // 允许文本换行
    m_descriptionLabel->setMinimumWidth(350);
    m_descriptionLabel->setMaximumWidth(400);
    m_descriptionLabel->setStyleSheet("color: #808080;");

    // 设置描述标签的字体比标题标签小2号
    QFont descriptionFont = m_titleLabel->font();
    int titleFontSize = descriptionFont.pointSize();
    if (titleFontSize > 2) {
        descriptionFont.setPointSize(titleFontSize - 2);
    } else {
        descriptionFont.setPointSize(1);  // 确保字体大小至少为1
    }
    m_descriptionLabel->setFont(descriptionFont);

    // 将标题和描述文本添加到垂直布局中
    titleAndDescriptionLayout->addWidget(m_titleLabel);
    titleAndDescriptionLayout->addWidget(m_descriptionLabel);

    // 展开/收起按钮，类似CloudModelItem的更多按钮
    m_expandButton = new QToolButton(m_titleFrame);
    m_expandButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_expandButton");
    m_expandButton->setFixedSize(38, 38);
    m_expandButton->setProperty("useButtonPalette", QVariant(true));
    m_expandButton->setAutoRaise(true);
    m_expandButton->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    m_expandButton->setIconSize(QSize(16, 16));
    m_expandButton->setFocusPolicy(Qt::NoFocus);
    m_expandButton->setStyleSheet(
        "QToolButton {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        "QToolButton:hover {"
        "    background-color: palette(button);"
        "}"
        "QToolButton:pressed {"
        "    background-color: palette(midlight);"
        "}"
        "QToolTip {"
        "    border-radius: 4px;"
        "    padding: 3px;"
        "}");
    m_expandButton->setToolTip(tr("Expand"));

    // 添加到标题布局
    m_titleLayout->addWidget(titleAndDescriptionWidget);
    m_titleLayout->addStretch();  // 弹性空间
    m_titleLayout->addWidget(m_expandButton);

    // 主框架的垂直布局，包含标题和内容区域
    auto* mainFrameLayout = new QVBoxLayout(m_mainFrame);
    mainFrameLayout->setContentsMargins(0, 0, 0, 0);
    mainFrameLayout->setSpacing(0);
    mainFrameLayout->addWidget(m_titleFrame);

    // 创建内容区域
    m_contentWidget = new QWidget(m_mainFrame);
    m_contentWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_contentWidget");
    m_contentWidget->setVisible(false);  // 默认隐藏内容区域

    // 内容区域的布局
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(8, 8, 8, 8);  // 内容区域边距8px
    m_contentLayout->setSpacing(8);                   // 设置组件间距为8px

    // 将内容区域添加到主框架布局
    mainFrameLayout->addWidget(m_contentWidget);

    // 连接展开/收起按钮信号
    connect(m_expandButton, &QToolButton::clicked, this, &ExpandableModelGroup::toggleContent);
}

void ExpandableModelGroup::addContentWidget(QWidget* widget) {
    m_contentLayout->addWidget(widget);
    updateContentHeight();
}

void ExpandableModelGroup::setContentLayout(QLayout* contentLayout) {
    // 清除现有的内容布局
    if (m_contentLayout->count() > 0) {
        QLayoutItem* child;
        while ((child = m_contentLayout->takeAt(0)) != nullptr) {
            delete child->widget();
        }
    }

    // 添加新的内容布局
    if (contentLayout) {
        m_contentLayout->addLayout(contentLayout);
    }

    // 更新高度
    updateContentHeight();
}

void ExpandableModelGroup::updateContentHeight() {
    if (m_isExpanded) {
        // 如果当前是展开状态，则重新计算并设置高度
        int newHeight = 76 + m_contentWidget->sizeHint().height();
        m_mainFrame->setFixedHeight(newHeight);
    }
    // 如果是收起状态，则保持原来的固定高度
    else {
        m_mainFrame->setFixedHeight(76);
    }
}

void ExpandableModelGroup::toggleContent() {
    m_isExpanded = !m_isExpanded;

    if (m_isExpanded) {
        m_expandButton->setIcon(QIcon::fromTheme("ukui-up-symbolic"));  // 展开时显示向上箭头
        m_expandButton->setToolTip(tr("Fold"));
        m_contentWidget->show();

        // 更新高度以适应内容
        updateContentHeight();
    } else {
        m_expandButton->setIcon(QIcon::fromTheme("ukui-down-symbolic"));  // 收起时显示向下箭头
        m_expandButton->setToolTip(tr("Expand"));
        m_contentWidget->hide();

        // 收起时恢复主框架高度
        m_mainFrame->setFixedHeight(76);
    }
}

void ExpandableModelGroup::increaseContentHeight() {
    if (m_isExpanded) {
        // 增加一个模型项的高度
        int increment = 57;
        int currentHeight = m_mainFrame->height();
        int newHeight = currentHeight + increment;

        // 确保最小高度为76（只有标题的高度）
        if (newHeight < 76) {
            newHeight = 76;
        }

        m_mainFrame->setFixedHeight(newHeight);
    }
}

void ExpandableModelGroup::decreaseContentHeight() {
    if (m_isExpanded) {
        // 减少一个模型项的高度
        int decrement = 57;
        int currentHeight = m_mainFrame->height();
        int newHeight = currentHeight - decrement;

        // 确保最小高度为76（只有标题的高度）
        if (newHeight < 76) {
            newHeight = 76;
        }

        m_mainFrame->setFixedHeight(newHeight);
    }
}

void ExpandableModelGroup::setExpanded(bool expanded) {
    if (m_isExpanded == expanded) {
        return;  // 如果状态相同，则不执行任何操作
    }

    m_isExpanded = expanded;

    if (expanded) {
        m_expandButton->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
        m_expandButton->setToolTip(tr("Fold"));
        m_contentWidget->show();
        updateContentHeight();
    } else {
        m_expandButton->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
        m_expandButton->setToolTip(tr("Expand"));
        m_contentWidget->hide();
        m_mainFrame->setFixedHeight(76);
    }
}