#include "localmodelitem.h"

LocalModelItem::LocalModelItem(PublicLocalModel model, SystemVersion curSystemVersion, QWidget *parent)
{
    m_curSystemVersion = curSystemVersion;
    m_capability = model.capability;
    m_modelName = model.modelName;
    m_vendor = model.vendor;
    m_model = model;
    initLayout();
    initConnect();
    initMenu();
    if (m_curSystemVersion == SystemVersion::Version_2503) {
        initInstallSuccessMsgBox();
        // 如果存在临时文件，提示用户需要重启电脑
        if (checkHasRebootTempFile()) {
            showMessageTips(MessageTipType::PersistentMessage, tr("Effective after reboot"), 0);
        }
    }
}

LocalModelItem::~LocalModelItem() {}

void LocalModelItem::showSelectedIcon(bool flag) {
    if (flag) {
        m_selectedIconLabel->setPixmap(m_selectedPixmap);  // 显示选中图标
    } else {
        m_selectedIconLabel->setPixmap(m_unselectedPixmap);  // 显示未选中图标
    }
}

AiCapability LocalModelItem::getAICapability()
{
    return m_capability;
}

QString LocalModelItem::getModelName()
{
    return m_modelName;
}

QString LocalModelItem::getModelDebName()
{
    return m_model.debName;
}

bool LocalModelItem::getIsStartDownloading()
{
    return isStartDownloading;
}

void LocalModelItem::showMessageTips(MessageTipType type, const QString &msg, const int &duration)
{
    // 重置定时器
    if (type == MessageTipType::InfoMessage && m_infoTimer) {
        m_infoTimer->stop();
        m_infoTimer->deleteLater();
        m_infoTimer = nullptr;
    }
    else if (type == MessageTipType::WarningMessage && m_warningTimer) {
        m_warningTimer->stop();
        m_warningTimer->deleteLater();
        m_warningTimer = nullptr;
    }

    //设置状态
    if(m_tipsLabel->isVisible() == false)
    {
        m_tipsLabel->setVisible(true);
    }
    switch (type) {
    case MessageTipType::InfoMessage:
        m_tipsLabel->setStyleSheet("color:#808080;");
        m_tipsLabel->setText(msg);
        m_currentInfoMessage = msg;

        m_infoTimer = new QTimer(this);
        m_infoTimer->setSingleShot(true);
        connect(m_infoTimer, &QTimer::timeout, this, [this, msg](){
            KyInfo() << "===========showMessageTips===========InfoMessage============QTimer==========text:" << m_tipsLabel->text() << "==========msg===========" << m_currentInfoMessage;
            if (m_tipsLabel->text() == m_currentInfoMessage) {
                m_tipsLabel->setStyleSheet("color:black");
                m_tipsLabel->setText("");
                m_tipsLabel->setVisible(false);
            }
        });
        m_infoTimer->start(duration);

        break;
    case MessageTipType::WarningMessage:
        m_tipsLabel->setStyleSheet("color:red;");
        m_tipsLabel->setText(msg);
        m_currentWarningMessage = msg;

        m_warningTimer = new QTimer(this);
        m_warningTimer->setSingleShot(true);
        connect(m_warningTimer, &QTimer::timeout, this, [this, msg](){
            KyInfo() << "===========showMessageTips===========WarningMessage============QTimer==========text:" << m_tipsLabel->text() << "=========msg===========" << m_currentWarningMessage;
            if (m_tipsLabel->text() == m_currentWarningMessage) {
                m_tipsLabel->setStyleSheet("color:black");
                m_tipsLabel->setText("");
                m_tipsLabel->setVisible(false);
            }
        });
        m_warningTimer->start(duration);

        break;
    case MessageTipType::PersistentMessage:
        m_tipsLabel->setStyleSheet("color:#808080;");
        m_tipsLabel->setText(msg);
        break;
    default:
        break;
    }
}

void LocalModelItem::initModelDownloadLayout() {
    //模型面板
    m_panelHLayout = new QHBoxLayout();

    //模型信息
    m_messageHLayout = new QHBoxLayout();
    m_messageWidget = new QWidget(this);
    m_messageWidget->setAccessibleName("kylin-ai-subsystem-modelconfig_QWidget_m_messageWidget");
    //更新和状态信息提示
    m_tipsLabel = new QLabel(m_messageWidget);
    m_tipsLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_tipsLabel");
    m_tipsLabel->setVisible(false);
    if(checkHasRebootTempFile()) {
        m_tipsLabel->setVisible(true);
        showMessageTips(MessageTipType::PersistentMessage,tr("Effective after reboot ."),0);
    }
    m_messageHLayout->addWidget(m_tipsLabel);

    //更多按钮
    m_moreButton = new QToolButton(m_messageWidget);
    m_moreButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_moreButton");
    m_moreButton->setFixedSize(32, 32);
    m_moreButton->setProperty("useButtonPalette", true);
    m_moreButton->setAutoRaise(true);
    m_moreButton->setPopupMode(QToolButton::InstantPopup);
    m_moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_moreButton->setFocusPolicy(Qt::NoFocus);
    m_moreButton->setVisible(true);
    m_messageHLayout->addWidget(m_moreButton);

    //下载按钮
    m_downButton = new QToolButton(m_messageWidget);
    m_downButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_downButton");
    m_downButton->setFixedSize(32, 32);
    m_downButton->setProperty("useButtonPalette", true);
    m_downButton->setAutoRaise(true);
    m_downButton->setPopupMode(QToolButton::InstantPopup);
    m_downButton->setIcon(QIcon::fromTheme("browser-download-symbolic"));
    m_downButton->setFocusPolicy(Qt::NoFocus);
    m_downButton->hide();
    connect(m_downButton,&KToolButton::clicked,this,&LocalModelItem::sldownModel);
    m_messageHLayout->addWidget(m_downButton);

    m_loadButton = new KToolButton(m_messageWidget);
    m_loadButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_loadButton");
    m_loadButton->setLoading(true);
    m_loadButton->setFixedSize(38,38);
    m_loadButton->hide();
    m_messageHLayout->addWidget(m_loadButton);

    m_messageWidget->setLayout(m_messageHLayout);

    m_messageWidget->setVisible(true);
    m_panelHLayout->addWidget(m_messageWidget);


    //进度条
    m_progressHLayout = new QHBoxLayout();
    updateProgressBar = new UpdateNotificationWidget(m_model, this);
    updateProgressBar->setAccessibleName("kylin-ai-subsystem-modelconfig_UpdateNotificationWidget_updateProgressBar");
    updateProgressBar->setVisible(false);
    m_progressHLayout->addWidget(updateProgressBar);

    m_panelHLayout->addLayout(m_progressHLayout);
}

void LocalModelItem::initLayout()
{
    if (m_curSystemVersion == SystemVersion::Version_2503) {
        initModelDownloadLayout();
    }
    m_mainHLayout = new QHBoxLayout();

    initIconLabel();
    initCertificationLabel();

    //模型名称
    m_modelNameLabel = new QLabel(this);
    m_modelNameLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_m_modelNameLabel");
    m_modelNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_modelNameLabel->setText(m_modelName);

    //选中判断框
    m_selectedIconLabel = new QLabel(this);
    m_selectedIconLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_selectedIconLabel");

    // 加载选中和未选中状态的图标
    QIcon unselectedIcon = QIcon(":/res/icon/unselected.svg");
    m_unselectedPixmap = unselectedIcon.pixmap(unselectedIcon.actualSize(QSize(16, 16)));

    QIcon selectedIcon = QIcon(":/res/icon/selected.svg");
    m_selectedPixmap = selectedIcon.pixmap(selectedIcon.actualSize(QSize(16, 16)));

    // 默认显示未选中状态
    m_selectedIconLabel->setPixmap(m_unselectedPixmap);

    // 适配主题
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
    frame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_frame");
    frame->setFixedSize(18, 18);
    QHBoxLayout *frameLayout = new QHBoxLayout;
    frameLayout->setMargin(0);
    frame->setLayout(frameLayout);
    frameLayout->addWidget(m_selectedIconLabel);
    m_selectedIconLabel->setVisible(true);
    m_selectedIconLabel->setAlignment(Qt::AlignCenter);

    // 确保在所有系统版本下都创建m_moreButton
    if (!m_moreButton) {
        m_moreButton = new QToolButton(this);
        m_moreButton->setAccessibleName("kylin-ai-subsystem-modelconfig_QToolButton_m_moreButton");
        m_moreButton->setFixedSize(32, 32);
        m_moreButton->setProperty("useButtonPalette", true);
        m_moreButton->setAutoRaise(true);
        m_moreButton->setPopupMode(QToolButton::InstantPopup);
        m_moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
        m_moreButton->setFocusPolicy(Qt::NoFocus);
        m_moreButton->setVisible(true);
    }

    // 拔出后摩显卡后模型不可用的提示文本
    m_deviceStatusLabel = new QLabel(this);
    m_deviceStatusLabel->setText(tr("Dependent device is not connected"));
    m_deviceStatusLabel->setStyleSheet("color: #808080;");
    m_deviceStatusLabel->setVisible(false);

    m_mainHLayout->setContentsMargins(16,0,16,0);
    m_mainHLayout->addWidget(m_iconLabel);
    m_mainHLayout->addWidget(m_modelNameLabel);
    m_mainHLayout->addWidget(m_certificationLabel);
    m_mainHLayout->addStretch();
    if (m_curSystemVersion == SystemVersion::Version_2503) {
        m_mainHLayout->addLayout(m_panelHLayout);
    }
    m_mainHLayout->addWidget(m_deviceStatusLabel);
    m_mainHLayout->addSpacing(8);
    m_mainHLayout->addWidget(m_moreButton);
    m_mainHLayout->addSpacing(8);
    m_mainHLayout->addWidget(frame);
    this->setLayout(m_mainHLayout);
}

void LocalModelItem::initMenu(){
    // 添加空指针检查，避免在m_moreButton未初始化时调用setMenu导致崩溃
    if (!m_moreButton) {
        return;
    }

    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-ai-subsystem-modelconfig_QMenu_m_menu");
    m_moreButton->setMenu(m_menu);

    if (m_curSystemVersion == SystemVersion::Version_V11) {
        QAction* actionUpdate = new QAction(tr("Detect Updates"));
        connect(actionUpdate, &QAction::triggered, [this] { Q_EMIT sigUpdate(m_capability, m_modelName); });
        QAction* actionUninstall = new QAction(tr("Uninstall"));
        connect(actionUninstall, &QAction::triggered, [this] {
            Q_EMIT sigUninstall(m_capability, m_modelName);
        });
        m_menu->addAction(actionUpdate);
        m_menu->addAction(actionUninstall);
    }

    if (m_curSystemVersion == SystemVersion::Version_2503) {
        // 保留原有的检测更新功能
        QAction* actionCheckUpdates = new QAction(tr("Detect Updates"));  // 检测更新
        connect(actionCheckUpdates, &QAction::triggered, this, &LocalModelItem::checkHasNewVersion);
        m_menu->addAction(actionCheckUpdates);
    }
}

void LocalModelItem::initInstallSuccessMsgBox()
{
    m_installSuccessMsgBox = new QMessageBox(this);
    m_installSuccessMsgBox->setAccessibleName("kylin-ai-subsystem-modelconfig_QMessageBox_m_installSuccessMsgBox");
    m_installSuccessMsgBox->setIconPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(24,24));

    // 标题加粗并为黑色字体（使用 HTML）
    QString title = "<b><font color='black'>" + tr("Install success!") + "</font></b>";
    m_installSuccessMsgBox->setText(title);

    // 说明文字也设为黑色
    QString infoText = "<font color='black'>" + tr("The ondevice model has been installed and needs to be restarted "
                                                   "before it can take effect. Do you want to restart it?") + "</font>";
    m_installSuccessMsgBox->setInformativeText(infoText);

    QPushButton *rebootLater =  m_installSuccessMsgBox->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
    QPushButton *rebootImmediately = m_installSuccessMsgBox->addButton(tr("Reboot Immediately"), QMessageBox::AcceptRole);
    m_installSuccessMsgBox->setDefaultButton(rebootImmediately);

    // 立即重启
    connect(rebootImmediately, &QPushButton::clicked, this, [=]{
        QDBusInterface interface("org.gnome.SessionManager",
                                 "/org/gnome/SessionManager",
                                 "org.gnome.SessionManager",
                                 QDBusConnection::sessionBus());
        interface.call("reboot");
    });

    // 稍后重启
    connect(rebootLater, &QPushButton::clicked, this, [=]{
        m_modelStatus = LocalModelInstallStatus::RebootLater;
        m_messageWidget->setVisible(true);
        showMessageTips(MessageTipType::PersistentMessage,tr("Take effect after rebooting"),0);
        m_moreButton->setVisible(true);
    });

    m_installSuccessMsgBox->setVisible(false);
}

void LocalModelItem::initConnect()
{
    connect(updateProgressBar,&UpdateNotificationWidget::sigShowIsInstalling,this,[&]{
        m_messageWidget->setVisible(true);
        showMessageTips(MessageTipType::PersistentMessage,tr("installing..."),0);
    });
}

void LocalModelItem::checkHasNewVersion()
{
    m_moreButton->hide();
    m_tipsLabel->hide();
    m_loadButton->setVisible(true);
    QCoreApplication::processEvents();
    //断网检查
    bool isOnline = updateProgressBar->checkNetWorkOnline();
    if(!isOnline)
    {
        m_loadButton->setVisible(false);
        m_moreButton->setVisible(true);
        showMessageTips(MessageTipType::WarningMessage,tr("Net is error，Please check and try again"),6000);
        m_downButton->setVisible(false);
        return;
    }
    //进行新版本检查并切换到下载界面
    KyInfo() << "current download model deb Name:" << this->getModelDebName();
    QString pkgName = this->getModelDebName();
    QString tips;
    PkgInstallStatus status = ModelDownload::getInstance()->getPkgStatus(pkgName);
    switch (status) {
    //安装包未找到
    case PkgInstallStatus::PkgNotFound:
        tips = QString(tr("The installation package for the specified model was not found:% 1. Please check the software source and network status and try again!")).arg(pkgName);
        m_loadButton->setVisible(false);
        m_tipsLabel->setVisible(false);
        m_downButton->setVisible(false);
        m_moreButton->setVisible(true);
        break;
    //安装包已找到未安装
    case PkgInstallStatus::PkgFoundNotInstall:
        tips = QString(tr("The current model installation package is not installed, it can be downloaded and installed"));
        m_loadButton->setVisible(false);
        m_tipsLabel->setVisible(true);
        m_downButton->setVisible(true);
        break;
    //安装包发现新版本需要更新
    case PkgInstallStatus::PkgFoundNewVersion:
        tips = QString(tr("New version needs to be updated upon discovery"));
        m_loadButton->setVisible(false);
        showMessageTips(MessageTipType::InfoMessage,tr("New version needs to be updated upon discovery"),6000);
        m_downButton->setVisible(true);
        break;
    //安装包已经是最新版本
    case PkgInstallStatus::PkgLastestVersion:
        tips = QString(tr("The current model is already the latest model"));
        m_loadButton->setVisible(false);
        showMessageTips(MessageTipType::InfoMessage,tr("Current model version is the lastest version"),6000);
        m_downButton->setVisible(false);
        m_moreButton->setVisible(true);
        break;
    default:
        break;
    }
    KyInfo() << "current check deb result:" <<tips;

}

bool LocalModelItem::createRebootTempFile()
{
    QString  filePath = TEMP_REBOOT_FILE;
    QFile file(filePath);
    if(file.exists())
    {
        return true;
    }
    if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << "create tmp reboot test file." << endl;
            file.close();
            KyInfo() << "Reboot check File created successfully!";
            return true;
        } else {
            KyInfo() << "Failed to create Reboot check file!";
            return false;
        }
}

bool LocalModelItem::checkHasRebootTempFile()
{
    QString  filePath = TEMP_REBOOT_FILE;
    QFile file(filePath);
    return file.exists();
}

void LocalModelItem::sldownModel()
{
    m_downButton->setVisible(false);
    m_messageWidget->setVisible(false);
    //如果成功开始下载则跳转到下载界面
    if(ModelDownload::getInstance()->downloadModelPkg(m_model.debName))
    {
        updateProgressBar->setVisible(true);
        updateProgressBar->setProgressBarVisible(true);
        updateProgressBar->setCancelDownloadButtenEnabled(false);
        //设置开始下载标志
        isStartDownloading = true;
        ModelDownload::getInstance()->setIsStartDownloading(true);
        //更新下载进度
        connect(ModelDownload::getInstance(),&ModelDownload::sigUpdateModelInstallStatus,this,&LocalModelItem::slUpdateModelInstallStatus,Qt::UniqueConnection);
        //更新下载状态
        connect(ModelDownload::getInstance(),&ModelDownload::sigUpdateModelInstallFinished,this,&LocalModelItem::slUpdateModelInstallFinished,Qt::UniqueConnection);
        //更新取消下载按钮
        connect(ModelDownload::getInstance(),&ModelDownload::sigCancelExitButton,this,&LocalModelItem::slUpdateCancelButton,Qt::UniqueConnection);
    }
}

void LocalModelItem::startNetErrorTips()
{
    if(m_tipsLabel->isVisible() == true)
    {
        m_tipsLabel->setStyleSheet("color:red;");
        m_timer = new QTimer(this);
        m_timer->setInterval(3000);
        m_timer->start();
        connect(m_timer, &QTimer::timeout, this, [&]{
            m_timer->stop();
            m_tipsLabel->setStyleSheet("color:black;");
            m_tipsLabel->setText("");
            m_tipsLabel->setVisible(false);
        });
    }
}

void LocalModelItem::slUpdateModelInstallStatus(int value, QString status, QString details)
{
    if(isStartDownloading)
    {
        KyInfo() << "clear start downloading flag:" << m_downButton->isVisible();
        isDownLoading = true;
        ModelDownload::getInstance()->setDownloadingStatus(true);
        updateProgressBar->setCancelDownloadButtenEnabled(true);
        ModelDownload::getInstance()->setIsStartDownloading(false);
    }
    //下载过程中时再进行下载进度条的更新
    if(isDownLoading)
    {
        updateProgressBar->slUpdateProgressValue(value);
    }
}

void LocalModelItem::slUpdateModelInstallFinished(bool isSuccess, QStringList upgradeGroup, QString errorStr, QString errorDesc)
{
    //安装成功
    if(isSuccess)
    {
        //创建临时文件，检查是否重启过电脑
        createRebootTempFile();
        //如果存在临时文件，提示用户需要重启电脑
        if(checkHasRebootTempFile())
        {
            showMessageTips(MessageTipType::PersistentMessage,tr("Effective after reboot"),0);
        }

        //下载完成，如果当前界面显示时则显示是否重启对话框
        if(this->isVisible())
        {
            m_installSuccessMsgBox->setVisible(true);
        }
    }
    //安装失败
    else {
        QString errorTips = QString("current model deb:%0 install failed.\n errorStr:%1.\n errorDesc:%2")
                                .arg(upgradeGroup.at(0))
                                .arg(errorStr)
                                .arg(errorDesc);
        KyInfo() << errorTips;
        showMessageTips(MessageTipType::WarningMessage,tr("Current model install failed,please install manually."),6000);
    }
    //设置下载状态
    isDownLoading = false;
    ModelDownload::getInstance()->setDownloadingStatus(false);
    //设置安装状态
    ModelDownload::getInstance()->setInstallingStatus(false);
    //清除下载进度
    updateProgressBar->setVisible(false);
    updateProgressBar->resetProgressBar();
    m_messageWidget->setVisible(true);
    m_moreButton->setVisible(true);
    isStartDownloading = false;
    //断开信号槽的链接
    //更新下载进度
    disconnect(ModelDownload::getInstance(),&ModelDownload::sigUpdateModelInstallStatus,this,&LocalModelItem::slUpdateModelInstallStatus);
    //更新下载状态
    disconnect(ModelDownload::getInstance(),&ModelDownload::sigUpdateModelInstallFinished,this,&LocalModelItem::slUpdateModelInstallFinished);
    //更新取消下载按钮
    disconnect(ModelDownload::getInstance(),&ModelDownload::sigCancelExitButton,this,&LocalModelItem::slUpdateCancelButton);

}

void LocalModelItem::slUpdateCancelButton(bool isInstalling)
{
    if(isInstalling)
    {
        updateProgressBar->setCancelDownloadButtonVisible(false);
    }
}

void LocalModelItem::initIconLabel() {
    m_iconLabel = new QLabel(this);
    m_iconLabel->setScaledContents(true);

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
        // 默认图标
        m_iconLabel->setFixedSize(16, 16);
        updateVendorIcon(m_iconLabel, ":/res/icon/unknown-model.svg",
                         ":/res/icon/unknown-model-light.svg");
    }

    // 设置主题监听
    setupThemeListener();
}

void LocalModelItem::initCertificationLabel() {
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

// 用于设置后摩显卡拔出后的置灰状态
void LocalModelItem::setGrayedOut(bool available) {
    if (available) {
        m_modelNameLabel->setPalette(QPalette());
        m_iconLabel->setGraphicsEffect(nullptr);
        setKLabelFontColor(m_certificationLabel, "#3790FA");
        m_deviceStatusLabel->setVisible(false);

    } else {
        m_modelNameLabel->setStyleSheet("color: #808080;");
        m_iconLabel->setGraphicsEffect(createGrayEffect());
        setKLabelFontColor(m_certificationLabel, "#A5CDFD");
        m_deviceStatusLabel->setVisible(true);
    }
}

// 模型前的图标置灰
QGraphicsEffect* LocalModelItem::createGrayEffect() {
    QGraphicsColorizeEffect* colorizeEffect = new QGraphicsColorizeEffect();
    colorizeEffect->setColor(QColor(128, 128, 128));
    colorizeEffect->setStrength(0.8);

    return colorizeEffect;
}

void LocalModelItem::setupThemeListener() {
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

QString LocalModelItem::getCurrentTheme() {
    if (m_themeSettings) {
        return m_themeSettings->get("styleName").toString();
    }
    return "";
}

void LocalModelItem::updateIconForTheme(const QString& theme) {
    // 根据供应商确定基本图标路径
    if (m_vendor == "deepseek") {
        updateVendorIcon(m_iconLabel, ":/res/icon/deepseek.svg",
                         ":/res/icon/deepseek-light.svg");
    } else if (m_vendor == "qwen") {
        updateVendorIcon(m_iconLabel, ":/res/icon/qwen.svg",
                         ":/res/icon/qwen-light.svg");
    } else {
        // 如果没有匹配的供应商，使用空图标
        updateVendorIcon(m_iconLabel, ":/res/icon/unknown-model.svg",
                         ":/res/icon/unknown-model-light.svg");
        return;
    }
}

void LocalModelItem::updateVendorIcon(QLabel* label,
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
