#include "privatemodelitemwidget.h"

PrivateModelItemWidget::PrivateModelItemWidget(kyai::config::model::AiCapability capability, QString modelName,
                                               QString vendor, bool hasMoreButton, QWidget* parent)
    : QWidget(parent) {
    m_capability = capability;
    m_modelName = modelName;
    m_vendor = vendor;  // 存储供应商信息
    initIconLabel();    // 初始化图标
    initLayout(hasMoreButton);
    if (hasMoreButton) {
        initMenu();
    }

    // 设置主题监听
    setupThemeListener();
}

PrivateModelItemWidget::~PrivateModelItemWidget()
{

}

void PrivateModelItemWidget::initIconLabel() {
    m_iconLabel = new QLabel(this);
    m_iconLabel->setScaledContents(true);  // 自动缩放图片以适应标签大小

    // 根据供应商设置图标和相应尺寸
    if (m_vendor == "ernie") {  // 百度-文生图
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/baidu.svg",
                         ":/res/icon/baidu-light.svg");
    } else if (m_vendor == "baidu") {  // 百度-语音
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/baidu.svg",
                         ":/res/icon/baidu-light.svg");
    } else if (m_vendor == "spark") {  // 讯飞-文生图
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/xunfei.svg",
                         ":/res/icon/xunfei-light.svg");
    } else if (m_vendor == "xunfei") {  // 讯飞-语音
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/xunfei.svg",
                         ":/res/icon/xunfei-light.svg");
    } else {
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(
            m_iconLabel, ":/res/icon/custom-model.svg",
            ":/res/icon/custom-model-light.svg");  // 默认使用自定义模型图标
    }
}

void PrivateModelItemWidget::initLayout(bool hasMoreButton) {
    m_nameLabel = new QLabel;
    m_nameLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_nameLabel");
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_nameLabel->setText(m_modelName);

    if (hasMoreButton) {
        m_moreButton = new QToolButton;
        m_moreButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_moreButton");

        m_moreButton->setFixedSize(38, 38);
        m_moreButton->setProperty("useButtonPalette", true);
        m_moreButton->setAutoRaise(true);
        m_moreButton->setPopupMode(QToolButton::InstantPopup);
        m_moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
        m_moreButton->setFocusPolicy(Qt::NoFocus);
        m_moreButton->setStyleSheet(
            "QToolButton {"
            "    border: none;"
            "    background-color: transparent;"
            "}"
            "QToolButton::menu-indicator {"
            "    image: none;"
            "}"
            "QToolButton:hover {"
            "    background-color: palette(button);"
            "}"
            "QToolButton:pressed {"
            "    background-color: palette(midlight);"
            "}");
    }

    m_selectedIconLabel = new QLabel();
    m_selectedIconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_selectedIconLabel");

    // 加载选中和未选中状态的图标
    QIcon unselectedIcon = QIcon(":/res/icon/unselected.svg");
    m_unselectedPixmap = unselectedIcon.pixmap(unselectedIcon.actualSize(QSize(16, 16)));

    QIcon selectedIcon = QIcon(":/res/icon/selected.svg");
    m_selectedPixmap = selectedIcon.pixmap(selectedIcon.actualSize(QSize(16, 16)));

    // 默认显示未选中状态
    m_selectedIconLabel->setPixmap(m_unselectedPixmap);

    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);
    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName") {
            // 更新图标以适应主题变化
            QIcon unselectedIcon = QIcon(":/res/icon/unselected.svg");
            m_unselectedPixmap = unselectedIcon.pixmap(unselectedIcon.actualSize(QSize(16, 16)));

            QIcon selectedIcon = QIcon(":/res/icon/selected.svg");
            m_selectedPixmap = selectedIcon.pixmap(selectedIcon.actualSize(QSize(16, 16)));

            // 如果当前是选中状态则显示选中图标，否则显示未选中图标
            if (m_selectedIconLabel->pixmap() &&
                m_selectedIconLabel->pixmap()->cacheKey() == m_selectedPixmap.cacheKey()) {
                m_selectedIconLabel->setPixmap(m_selectedPixmap);
            } else {
                m_selectedIconLabel->setPixmap(m_unselectedPixmap);
            }
        }
    });
    QFrame *frame = new QFrame(this);
    frame->setFixedSize(18, 18);
    QHBoxLayout *frameLayout = new QHBoxLayout;
    frameLayout->setMargin(0);
    frame->setLayout(frameLayout);
    frameLayout->addWidget(m_selectedIconLabel);
    m_selectedIconLabel->setVisible(true);
    m_selectedIconLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(16, 10, 16, 10);
    // 将图标标签添加到布局中，在名称标签之前
    hLayout->addWidget(m_iconLabel);
    hLayout->addSpacing(6);  // 图标与文字之间的间距
    hLayout->addWidget(m_nameLabel);
    hLayout->addStretch();

    if (hasMoreButton && m_moreButton) {
        hLayout->addWidget(m_moreButton);
        hLayout->addSpacing(8);  // 在moreButton和选中图标之间添加8像素间距
    }

    hLayout->addWidget(frame);
    this->setLayout(hLayout);
}

void PrivateModelItemWidget::initMenu()
{
    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-ai-subsystem-modelconfig_QMenu_m_menu");
    m_moreButton->setMenu(m_menu);

    QAction* actionEdit = new QAction(tr("Edit"));
    //actionEdit->setDisabled(true);
    connect(actionEdit, &QAction::triggered, [this]{Q_EMIT sigEdit(m_capability, m_modelName);});

    QAction* actionDelete = new QAction(tr("Delete"));
    connect(actionDelete, &QAction::triggered, [this]{Q_EMIT sigDelete(m_capability, m_modelName);});

    m_menu->addAction(actionEdit);
    m_menu->addAction(actionDelete);
}

void PrivateModelItemWidget::showSelectedIcon(bool flag)
{
    if (flag) {
        m_selectedIconLabel->setPixmap(m_selectedPixmap);  // 显示选中图标
    } else {
        m_selectedIconLabel->setPixmap(m_unselectedPixmap);  // 显示未选中图标
    }
}

AiCapability PrivateModelItemWidget::getAICapability()
{
    return m_capability;
}

QString PrivateModelItemWidget::getModelName() { return m_modelName; }

void PrivateModelItemWidget::setupThemeListener() {
    // 创建 QGSettings 对象来监听主题变化
    const QByteArray themeSchema(THEME_QT_SCHEMA);
    if (QGSettings::isSchemaInstalled(themeSchema)) {
        m_themeSettings = new QGSettings(themeSchema, QByteArray(), this);

        // 初始化时获取当前主题并更新图标
        QString currentTheme = getCurrentTheme();
        updateIconForTheme(currentTheme);

        // 连接信号槽监听主题变化
        connect(m_themeSettings, &QGSettings::changed, this,
                [this](const QString& key) {
                    if (key == "styleName" || key == "style-name") {
                        QString newTheme = getCurrentTheme();
                        updateIconForTheme(newTheme);
                    }
                });
    }
}

QString PrivateModelItemWidget::getCurrentTheme() {
    if (m_themeSettings) {
        return m_themeSettings->get("styleName").toString();
    }
    return "";
}

void PrivateModelItemWidget::updateIconForTheme(const QString& theme) {
    // 根据供应商设置图标和相应尺寸
    if (m_vendor == "ernie") {  // 百度-文生图
        updateVendorIcon(m_iconLabel, ":/res/icon/baidu.svg",
                         ":/res/icon/baidu-light.svg");
    } else if (m_vendor == "baidu") {  // 百度-语音
        updateVendorIcon(m_iconLabel, ":/res/icon/baidu.svg",
                         ":/res/icon/baidu-light.svg");
    } else if (m_vendor == "spark") {  // 讯飞-文生图
        updateVendorIcon(m_iconLabel, ":/res/icon/xunfei.svg",
                         ":/res/icon/xunfei-light.svg");
    } else if (m_vendor == "xunfei") {  // 讯飞-语音
        updateVendorIcon(m_iconLabel, ":/res/icon/xunfei.svg",
                         ":/res/icon/xunfei-light.svg");
    } else {
        updateVendorIcon(
            m_iconLabel, ":/res/icon/custom-model.svg",
            ":/res/icon/custom-model-light.svg");  // 默认使用自定义模型图标
    }
}

void PrivateModelItemWidget::updateVendorIcon(QLabel* label,
                                              const QString& defaultIconPath,
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

    label->setPixmap(QPixmap(iconPath));
}