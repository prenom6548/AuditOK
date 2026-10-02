#include "cloudmodelitem.h"

#include "util.h"

ModelItem::ModelItem(kyai::config::model::AiCapability capability, QString modelName, QString vendor,
                     bool isTrialAccount, QWidget* parent)
    : QWidget(parent),
      m_capability(capability),
      m_modelName(modelName),
      m_vendor(vendor),
      m_isTrialAccount(isTrialAccount) {
    initLayout();
    initMenu();

    // 设置主题监听
    setupThemeListener();
}

ModelItem::~ModelItem() {}

void ModelItem::initLayout() {
    initIconLabel();
    initNameLabel();
    initCertificationLabel();
    initMoreButton();
    initSelectedIconLabel();

    QFrame *frame = new QFrame(this);
    frame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_frame");
    frame->setFixedSize(18, 18);
    QHBoxLayout *frameLayout = new QHBoxLayout(frame);
    frameLayout->setMargin(0);
    frame->setLayout(frameLayout);
    frameLayout->addWidget(m_selectedIconLabel);
    m_selectedIconLabel->setVisible(true);
    m_selectedIconLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(16, 10, 16, 10);
    hLayout->addWidget(m_iconLabel);
    hLayout->addWidget(m_nameLabel);
    hLayout->addWidget(m_certificationLabel);
    hLayout->addStretch();
    hLayout->addWidget(m_moreButton);
    hLayout->addSpacing(8);
    hLayout->addWidget(frame);
    this->setLayout(hLayout);
}

void ModelItem::initNameLabel() {
    m_nameLabel = new QLabel(this);
    m_nameLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_nameLabel");
    m_nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_nameLabel->setText(m_modelName);
}

void ModelItem::initCertificationLabel() {
    m_certificationLabel = new kdk::KLabel(this);
    m_certificationLabel->setObjectName("certificationLabel");

    // 设置标签文本为"麒麟认证"
    m_certificationLabel->setText(tr("Kylin Certification"));

    // 设置标签样式
    m_certificationLabel->setStyleSheet(
        "QLabel { "
        "background-color: #E8F3FF; "  // 淡蓝色背景
        "border-radius: 3px; "         // 圆角
        "padding: 2px 6px; "           // 内边距
        "font-size: 11px; "            // 字体大小
        "min-height: 16px; "           // 最小高度
        "max-height: 16px; "           // 最大高度
        "}");

    m_certificationLabel->setAlignment(Qt::AlignCenter);

    // 设置字体颜色为蓝色
    setKLabelFontColor(m_certificationLabel, "#3790FA");
}

void ModelItem::initIconLabel() {
    m_iconLabel = new QLabel(this);
    m_iconLabel->setScaledContents(true);  // 自动缩放图片以适应标签大小

    // 根据模型名称设置图标和相应尺寸
    if (m_vendor == "deepseek") {
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/deepseek.svg",
                         ":/res/icon/deepseek-light.svg");
    } else if (m_vendor == "qwen") {
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/qwen.svg",
                         ":/res/icon/qwen-light.svg");
    } else {
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/unknown-model.svg",
                         ":/res/icon/unknown-model-light.svg");
    }
}

void ModelItem::initMoreButton() {
    m_moreButton = new QToolButton(this);
    m_moreButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_moreButton");
    m_moreButton->setFixedSize(38, 38);
    m_moreButton->setProperty("useButtonPalette", true);
    m_moreButton->setAutoRaise(true);
    m_moreButton->setPopupMode(QToolButton::InstantPopup);
    m_moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_moreButton->setFocusPolicy(Qt::NoFocus);
}

void ModelItem::initSelectedIconLabel() {
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
}

void ModelItem::initMenu() {
    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-ai-subsystem-modelconfig_QMenu_m_menu");
    m_moreButton->setMenu(m_menu);

    QAction* actionUpdate = new QAction(tr("Detect Updates"));
    connect(actionUpdate, &QAction::triggered, [this] { Q_EMIT sigUpdate(m_capability, m_modelName); });

    QAction* actionUninstall = new QAction(tr("Uninstall"));
    connect(actionUninstall, &QAction::triggered, [this] { Q_EMIT sigUninstall(m_capability, m_modelName); });

    m_menu->addAction(actionUpdate);
    m_menu->addAction(actionUninstall);
}

void ModelItem::showSelectedIcon(bool flag) {
    if (flag) {
        m_selectedIconLabel->setPixmap(m_selectedPixmap);
    } else {
        m_selectedIconLabel->setPixmap(m_unselectedPixmap);
    }
}

AiCapability ModelItem::getAICapability() {
    return m_capability;
}

QString ModelItem::getModelName() {
    return m_modelName;
}

bool ModelItem::isTrialModel() {
    return m_isTrialAccount;
}

void ModelItem::setupThemeListener() {
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

QString ModelItem::getCurrentTheme() {
    if (m_themeSettings) {
        return m_themeSettings->get("styleName").toString();
    }
    return "";
}

void ModelItem::updateIconForTheme(const QString& theme) {
    // 根据供应商设置图标和相应尺寸
    if (m_vendor == "deepseek") {
        updateVendorIcon(m_iconLabel, ":/res/icon/deepseek.svg",
                         ":/res/icon/deepseek-light.svg");
    } else if (m_vendor == "qwen") {
        updateVendorIcon(m_iconLabel, ":/res/icon/qwen.svg",
                         ":/res/icon/qwen-light.svg");
    } else {
        updateVendorIcon(m_iconLabel, ":/res/icon/unknown-model.svg",
                         ":/res/icon/unknown-model-light.svg");
    }
}

void ModelItem::updateVendorIcon(QLabel* label, const QString& defaultIconPath,
                                 const QString& lightIconPath) {
    // 如果没有默认图标路径，则不设置图标
    if (defaultIconPath.isEmpty()) {
        label->setPixmap(QPixmap());
        return;
    }

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