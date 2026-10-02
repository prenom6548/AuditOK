#include "permissionwidget.h"
#include <QDebug>
#include <QScrollArea>

PermissionWidget::PermissionWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void PermissionWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建 QScrollArea
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QLabel *globalHint = new QLabel(
        tr("💡 Please enable the following permissions according to the actual needs of the application. Unauthorized access will prohibit the application from using the corresponding functions."),
        this
    );
    globalHint->setStyleSheet(
        "background-color: #e3f2fd;"      // 浅蓝色背景
        "padding: 12px 15px;"            // 内边距
        "border-radius: 6px;"             // 圆角
        "border: 2px solid #90caf9;"     // 蓝色边框
        "color: #0d47a1;"
        "font-size: 14px;"
        "margin-bottom: 14px;"
        "box-shadow: 0 2px 4px rgba(0,0,0,0.1);" // 添加轻微阴影
    );
    globalHint->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(globalHint);

    // 创建容器 Widget，用于存放所有内容
    QWidget *containerWidget = new QWidget;
    QVBoxLayout *containerLayout = new QVBoxLayout(containerWidget);

    // 文件系统权限
    QGroupBox *filesystemGroupBox = new QGroupBox(tr("filesystem"), this);
    QGridLayout *filesystemLayout = new QGridLayout;

    m_xdgUserDirsCheckBox = new QCheckBox(tr("xdg-user-dirs"), this);
    filesystemLayout->addWidget(m_xdgUserDirsCheckBox, 0, 0);

    // 在复选框右侧添加提示标签
    QLabel *xdgLabel = new QLabel(tr("Only mount standard user directories (desktop, documents, images, music, videos, downloads, etc.)"), this);
    xdgLabel->setStyleSheet("color: #e74c3c;");
    filesystemLayout->addWidget(xdgLabel, 0, 1);

    // 挂载家目录选项
    m_mountHomeCheckBox = new QCheckBox(tr("Mount home directory"), this);
    filesystemLayout->addWidget(m_mountHomeCheckBox, 1, 0);

    // 家目录提示
    QLabel *homeLabel = new QLabel(tr("If not checked, the host home directory cannot be used"), this);
    homeLabel->setStyleSheet("color: #e74c3c;");
    filesystemLayout->addWidget(homeLabel, 1, 1);

    // 互斥逻辑
    connect(m_mountHomeCheckBox, &QCheckBox::stateChanged, this, [=](int state){
        if(state == Qt::Checked) {
            m_xdgUserDirsCheckBox->setChecked(false);
        }
    });

    connect(m_xdgUserDirsCheckBox, &QCheckBox::stateChanged, this, [=](int state){
        if(state == Qt::Checked) {
            m_mountHomeCheckBox->setChecked(false);
        }
    });

    QLabel *filesystemDirsLabel = new QLabel(tr("filesystem-dirs"), this);
    filesystemLayout->addWidget(filesystemDirsLabel, 2, 0);
    m_filesystemDirsLineEdit = new QTextEdit(this);
    m_filesystemDirsLineEdit->setPlaceholderText(tr("User-specified directory paths. "
                                    "Example: "
                                    "/path/a, /path/b"));
    m_filesystemDirsLineEdit->setToolTip(tr("User-specified directory paths, Separated with commas.\n"
                            "Example:\n"
                            "/path/a, /path/b"));
    filesystemLayout->addWidget(m_filesystemDirsLineEdit, 2, 1);
    filesystemGroupBox->setLayout(filesystemLayout);
    containerLayout->addWidget(filesystemGroupBox);

    // 网络权限
    QGroupBox *m_netGroupBox = new QGroupBox(tr("net"), this);
    QGridLayout *netLayout = new QGridLayout;
    m_networkCheckBox = new QCheckBox(tr("network"), this);
    netLayout->addWidget(m_networkCheckBox, 0, 0);

    m_scanningCheckBox = new QCheckBox(tr("scanning"), this);
    netLayout->addWidget(m_scanningCheckBox, 0, 1);
    m_netGroupBox->setLayout(netLayout);
    containerLayout->addWidget(m_netGroupBox);

    // 设备权限
    QGroupBox *deviceGroupBox = new QGroupBox(tr("device"), this);
    QGridLayout *deviceLayout = new QGridLayout;
    m_bluetoothCheckBox = new QCheckBox(tr("bluetooth"), this);
    deviceLayout->addWidget(m_bluetoothCheckBox, 0, 0);

    m_microphoneCheckBox = new QCheckBox(tr("microphone"), this);
    deviceLayout->addWidget(m_microphoneCheckBox, 0, 1);

    m_cameraCheckBox = new QCheckBox(tr("camera"), this);
    deviceLayout->addWidget(m_cameraCheckBox, 1, 0);

    m_usbCheckBox = new QCheckBox(tr("usb"), this);
    deviceLayout->addWidget(m_usbCheckBox, 1, 1);

    m_serialCheckBox = new QCheckBox(tr("serial"), this);
    deviceLayout->addWidget(m_serialCheckBox, 2, 0);
    deviceGroupBox->setLayout(deviceLayout);
    containerLayout->addWidget(deviceGroupBox);

    // 进程管理
    QGroupBox *m_processGroupBox = new QGroupBox(tr("processManager"), this);
    QHBoxLayout *hLayout = new QHBoxLayout();  // 改用水平布局

    m_autostartCheckBox = new QCheckBox(tr("autostart"), this);
    QLabel *autoLabel = new QLabel(tr("If not checked, the application settings will not take effect upon self startup"), this);
    autoLabel->setStyleSheet("color: #e74c3c;");

    hLayout->addWidget(m_autostartCheckBox);
    hLayout->addWidget(autoLabel);
    hLayout->addStretch();  // 添加伸缩因子使内容左对齐
    hLayout->setSpacing(5);  // 设置控件间距

    m_processGroupBox->setLayout(hLayout);
    containerLayout->addWidget(m_processGroupBox);

    // 其他权限
    QGroupBox *othersGroupBox = new QGroupBox(tr("others"), this);
    QGridLayout *othersLayout = new QGridLayout;
    m_systemNoticeCheckBox = new QCheckBox(tr("system-notice"), this);
    othersLayout->addWidget(m_systemNoticeCheckBox, 0, 0);

    m_systemTimeCheckBox = new QCheckBox(tr("system-time"), this);
    othersLayout->addWidget(m_systemTimeCheckBox, 0, 1);

    m_systemServerCheckBox = new QCheckBox(tr("system-server"), this);
    othersLayout->addWidget(m_systemServerCheckBox, 1, 0);

    m_preventSleepCheckBox = new QCheckBox(tr("prevent-sleep"), this);
    othersLayout->addWidget(m_preventSleepCheckBox, 1, 1);

    m_systemDisplayResolutionCheckBox = new QCheckBox(tr("system-display-resolution"), this);
    othersLayout->addWidget(m_systemDisplayResolutionCheckBox, 2, 0);

    m_systemDisplayBrightnessCheckBox = new QCheckBox(tr("system-display-brightness"), this);
    othersLayout->addWidget(m_systemDisplayBrightnessCheckBox, 2, 1);

    m_systemAudioCheckBox = new QCheckBox(tr("system-audio-setting"), this);
    othersLayout->addWidget(m_systemAudioCheckBox, 3, 0);

    m_systemCrontabCheckBox = new QCheckBox(tr("system-crontab"), this);
    othersLayout->addWidget(m_systemCrontabCheckBox, 3, 1);

    m_systemPowerModeCheckBox = new QCheckBox(tr("system-power-mode"), this);
    othersLayout->addWidget(m_systemPowerModeCheckBox, 4, 0);

    m_systemPowerPolicyCheckBox = new QCheckBox(tr("system-power-policy"), this);
    othersLayout->addWidget(m_systemPowerPolicyCheckBox, 4, 1);

    othersGroupBox->setLayout(othersLayout);
    containerLayout->addWidget(othersGroupBox);

    // 将容器 Widget 设置为 QScrollArea 的内容
    scrollArea->setWidget(containerWidget);
    mainLayout->addWidget(scrollArea); // 将 QScrollArea 添加到主布局

    // 确定按钮
//    QPushButton *okButton = new QPushButton(tr("Confirm"), this);
//    connect(okButton, &QPushButton::clicked, this, [=](){
//         QDialog::accept();
//    });
//    mainLayout->addWidget(okButton);
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton(tr("Back"));
    connect(backButton, &QPushButton::clicked, this, [=](){
        emit this->back();
    });
    QPushButton *nextButton = new QPushButton(tr("Next"));
    connect(nextButton, &QPushButton::clicked, this, [=](){
        emit this->next();
    });

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(backButton);
    buttonsLayout->addWidget(nextButton);
    buttonsLayout->setSpacing(8);
//    buttonsLayout->addStretch();

    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->setContentsMargins(24, 0, 24, 24);
//    mainLayout->setSpacing(24);
}

void PermissionWidget::setPermissionData(const KMBuilderManifest::Permission &permission)
{
    // 加载文件系统权限
    QStringList dirs = permission.m_filesystem.m_filesystemDirs;
    bool hasHomeDir = dirs.contains("~");

    // 设置复选框状态
    if (hasHomeDir) {
        m_mountHomeCheckBox->setChecked(true);
        m_xdgUserDirsCheckBox->setChecked(false);
        dirs.removeAll("~");  // 从显示中移除~
    } else {
        m_mountHomeCheckBox->setChecked(false);
        m_xdgUserDirsCheckBox->setChecked(permission.m_filesystem.m_xdgUserDirs);
    }

    // 设置目录编辑框内容
    m_filesystemDirsLineEdit->setText(dirs.join(","));

    // 加载网络权限
    m_networkCheckBox->setChecked(permission.m_net.m_network);
    m_scanningCheckBox->setChecked(permission.m_net.m_scanning);

    // 加载设备权限
    m_bluetoothCheckBox->setChecked(permission.m_device.m_bluetooth);
    m_microphoneCheckBox->setChecked(permission.m_device.m_microphone);
    m_cameraCheckBox->setChecked(permission.m_device.m_camera);
    m_usbCheckBox->setChecked(permission.m_device.m_usb);
    m_serialCheckBox->setChecked(permission.m_device.m_serial);

    //加载进程管理
    m_autostartCheckBox->setChecked(permission.m_processManager.m_autostart);

    // 加载其他权限
    m_systemNoticeCheckBox->setChecked(permission.m_others.m_systemNotice);
    m_systemTimeCheckBox->setChecked(permission.m_others.m_systemTime);
    m_systemServerCheckBox->setChecked(permission.m_others.m_systemServer);
    m_preventSleepCheckBox->setChecked(permission.m_others.m_preventSleep);
    m_systemDisplayResolutionCheckBox->setChecked(permission.m_others.m_systemDisplayResolution);
    m_systemDisplayBrightnessCheckBox->setChecked(permission.m_others.m_systemDisplayBrightness);
    m_systemAudioCheckBox->setChecked(
        permission.m_others.m_systemAudioSetting ||
        permission.m_others.m_systemAudioSize
    );
    m_systemCrontabCheckBox->setChecked(permission.m_others.m_systemCrontab);
    m_systemPowerModeCheckBox->setChecked(permission.m_others.m_systemPowerMode);
    m_systemPowerPolicyCheckBox->setChecked(permission.m_others.m_systemPowerPolicy);
}

KMBuilderManifest::Permission PermissionWidget::getPermissionData() const
{
   KMBuilderManifest::Permission permission;
   // 文件系统权限
   permission.m_filesystem.m_xdgUserDirs = m_xdgUserDirsCheckBox->isChecked();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList dirs = m_filesystemDirsLineEdit->toPlainText().split(",", Qt::SkipEmptyParts);
#else
    QStringList dirs = m_filesystemDirsLineEdit->toPlainText().split(",", QString::SkipEmptyParts);
#endif
    // 如果选择了挂载家目录，自动添加 ~ 到目录列表
    if (m_mountHomeCheckBox->isChecked() && !dirs.contains("~")) {
        dirs.prepend("~");  // 添加到列表开头
    }
    permission.m_filesystem.m_filesystemDirs = dirs;

   // 网络权限
   permission.m_net.m_network = m_networkCheckBox->isChecked();
   permission.m_net.m_scanning = m_scanningCheckBox->isChecked();

   // 设备权限
   permission.m_device.m_bluetooth = m_bluetoothCheckBox->isChecked();
   permission.m_device.m_microphone = m_microphoneCheckBox->isChecked();
   permission.m_device.m_camera = m_cameraCheckBox->isChecked();
   permission.m_device.m_usb = m_usbCheckBox->isChecked();
   permission.m_device.m_serial = m_serialCheckBox->isChecked();

   // 进程管理
   permission.m_processManager.m_autostart = m_autostartCheckBox->isChecked();

   // 其他权限
   permission.m_others.m_systemNotice = m_systemNoticeCheckBox->isChecked();
   permission.m_others.m_systemTime = m_systemTimeCheckBox->isChecked();
   permission.m_others.m_systemServer = m_systemServerCheckBox->isChecked();
   permission.m_others.m_preventSleep = m_preventSleepCheckBox->isChecked();
   permission.m_others.m_systemDisplayResolution = m_systemDisplayResolutionCheckBox->isChecked();
   permission.m_others.m_systemDisplayBrightness = m_systemDisplayBrightnessCheckBox->isChecked();
   permission.m_others.m_systemAudioSetting = m_systemAudioCheckBox->isChecked();
   permission.m_others.m_systemAudioSize = m_systemAudioCheckBox->isChecked();
   permission.m_others.m_systemCrontab = m_systemCrontabCheckBox->isChecked();
   permission.m_others.m_systemPowerMode = m_systemPowerModeCheckBox->isChecked();
   permission.m_others.m_systemPowerPolicy = m_systemPowerPolicyCheckBox->isChecked();

   return permission ;
}

