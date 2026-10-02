/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "screenlockui.h"
#include <QImageReader>
#include <QScrollBar>
#include "commonfunc.h"

ScreenlockUi::ScreenlockUi()
{
    setAttribute(Qt::WA_DeleteOnClose);
    m_screenlockLayout = new QVBoxLayout(this);
    m_screenlockLayout->setContentsMargins(0,0,0,0);
    initUi();


}

ScreenlockUi::~ScreenlockUi()
{

}

void ScreenlockUi::initUi()
{
    initScreenLock();
#ifdef Nile
    initLeavLockStatus();
#endif
    initSignals();
}

void ScreenlockUi::initScreenLock()
{
    // 1 锁屏
    m_wallpaperTitleLabel = new KLabel(this);
    m_wallpaperTitleLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Screenlock/Screenlock
    m_wallpaperTitleLabel->setText(tr("Screenlock"));

    // 2.1 预览图
    m_previewFrame = new UkccFrame(this);
    m_previewFrame->setFixedHeight(193);
    m_previewLayout = new QHBoxLayout(m_previewFrame);
    m_previewLayout->setContentsMargins(16, 8, 8, 8);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setFixedSize(280, 160);
    m_centerGroup = new SettingGroup(this);

    m_contentBackgroundGroup = new KBackgroundGroup(m_previewFrame);
    QList<QWidget*> contentList;

    // 登录界面显示锁屏壁纸
    QWidget* showPicWidget = new QWidget();
    QHBoxLayout* typeLayout = new QHBoxLayout(showPicWidget);
    typeLayout->setContentsMargins(16, 0, 0, 0);
    typeLayout->setSpacing(0);
    //~ contents_path /Screenlock/Show picture of screenlock on screenlogin
    KLabel* showPicLabel = new KLabel(showPicWidget);
    showPicLabel->setText(tr("Show picture of screenlock on screenlogin"));
    showPicLabel->setMinimumWidth(50);
    m_showPicSwitchButton = new KSwitchButton(showPicWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_showPicSwitchButton, "showPicWidget", "initScreenLock", "Toggle for show picture of screenlock on screenlogin");
    typeLayout->addWidget(showPicLabel);
    typeLayout->addStretch();
    typeLayout->addWidget(m_showPicSwitchButton);
    contentList.append(showPicWidget);

    QWidget* lockDatetimeEnableWidget = new QWidget();
    QHBoxLayout* datetimeeLayout = new QHBoxLayout(lockDatetimeEnableWidget);
    datetimeeLayout->setContentsMargins(16, 0, 0, 0);
    datetimeeLayout->setSpacing(0);
    //~ contents_path /Screenlock/The date and time are displayed
    KLabel* datetimeLabel = new KLabel(lockDatetimeEnableWidget);
    datetimeLabel->setText(tr("The date and time are displayed"));
    datetimeLabel->setMinimumWidth(50);
    m_lockDatetimeEnableBtn = new KSwitchButton(lockDatetimeEnableWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_lockDatetimeEnableBtn, "lockDatetimeEnableWidget", "initScreenLock", "Toggle for date and time are displayed");
    datetimeeLayout->addWidget(datetimeLabel);
    datetimeeLayout->addStretch();
    datetimeeLayout->addWidget(m_lockDatetimeEnableBtn);
    contentList.append(lockDatetimeEnableWidget);

#ifdef Nile
    // 动态锁
    QWidget* btLeaveLockWidget = new QWidget();
    QHBoxLayout* btLeaveLockLayout = new QHBoxLayout(btLeaveLockWidget);
    btLeaveLockLayout->setContentsMargins(16, 0, 0, 0);
    btLeaveLockLayout->setSpacing(0);
    KLabel* btLeaveLockLabel = new KLabel(btLeaveLockWidget);
    btLeaveLockLabel->setText(tr("Leave lock (System will be locked when the paired phone gone)"));
    btLeaveLockLabel->setMinimumWidth(50);
    m_btLeaveLockSwitchButton = new KSwitchButton(btLeaveLockWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(m_btLeaveLockSwitchButton, "btLeaveLockWidget", "initScreenLock", "Toggle for leave lock");
    btLeaveLockLayout->addWidget(btLeaveLockLabel);
    btLeaveLockLayout->addSpacing(40);
    btLeaveLockLayout->addStretch();
    btLeaveLockLayout->addWidget(m_btLeaveLockSwitchButton);
    contentList.append(btLeaveLockWidget);

    // 蓝牙设备
    m_bluetoothWidget = new QWidget();
    KLabel* bluetoothLabel = new KLabel(m_bluetoothWidget);
    bluetoothLabel->setText(tr("Specified device"));
    m_bluetoothComboBox = new QComboBox(m_bluetoothWidget);
    QHBoxLayout* bluetoothLayout = new QHBoxLayout(m_bluetoothWidget);
    bluetoothLayout->setContentsMargins(16, 0, 0, 0);
    bluetoothLayout->setSpacing(0);
    bluetoothLayout->addWidget(bluetoothLabel);
    bluetoothLayout->addStretch();
    bluetoothLayout->addWidget(m_bluetoothComboBox);
    m_bluetoothWidget->setVisible(false);
    contentList.append(m_bluetoothWidget);

    // 无配对手机去设置蓝牙
    m_setBtWidget = new QWidget();
    QHBoxLayout* setBtLayout = new QHBoxLayout(m_setBtWidget);
    setBtLayout->setContentsMargins(16, 0, 0, 0);
    setBtLayout->setSpacing(0);
    KLabel* setBtLabel = new KLabel(m_setBtWidget);
    setBtLabel->setText(tr("No paired phone. Please turn to 'Bluetooth' to pair."));
    setBtLabel->setMinimumWidth(50);
    QLabel* setBticonLabel = new QLabel(m_setBtWidget);
    QIcon icon = QIcon::fromTheme("kylin-dialog-warning");
    if (icon.isNull()) {
        icon = QIcon::fromTheme("dialog-warning");
    }
    setBticonLabel->setPixmap(icon.pixmap(16, 16));
    m_setBtPushButton = new KPushButton(m_setBtWidget);
    m_setBtPushButton->setText(tr("Bluetooth"));
    setBtLayout->addWidget(setBticonLabel);
    setBtLayout->addWidget(setBtLabel);
    setBtLayout->addStretch();
    setBtLayout->addWidget(m_setBtPushButton);
    m_setBtWidget->setVisible(false);
    contentList.append(m_setBtWidget);

    m_noBtWidget = new QWidget();
    contentList.append(m_noBtWidget);
#endif
    m_contentBackgroundGroup->addWidgetList(contentList);

    // 预览区域滚动条设置
    previewScrollArea = new QScrollArea;
    previewScrollArea->setFixedHeight(192);
    previewScrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    previewScrollArea->verticalScrollBar()->setEnabled(false);
    previewScrollArea->verticalScrollBar()->setVisible(false);
    previewScrollArea->horizontalScrollBar()->setEnabled(false);
    previewScrollArea->setWidgetResizable(true);
    previewScrollArea->setWidget(m_previewFrame);

    QVBoxLayout* rightPreviewLayout = new QVBoxLayout();
    rightPreviewLayout->setContentsMargins(8, 0, 16, 0);
    rightPreviewLayout->setSpacing(0);
    rightPreviewLayout->addWidget(m_contentBackgroundGroup);
    rightPreviewLayout->addStretch();

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 6, 0, 0);
    leftLayout->addWidget(m_previewLabel);
    leftLayout->addStretch();
    m_previewLayout->addLayout(leftLayout);
    m_previewLayout->addLayout(rightPreviewLayout);

    m_pictureFrame = new UkccFrame(m_centerGroup);
    m_pictureLayout = new FlowLayout(m_pictureFrame, -1, 8, 8);
    m_pictureLayout->setContentsMargins(12, 8, 12, 8);

    m_bottomFrame = new UkccFrame(m_centerGroup, UkccFrame::Top, false);
    m_bottomFrame->setFixedHeight(60);

    // 3.1 选择图片
    //~ contents_path /Wallpaper/Choose Picture
    QLabel* chooseLabel = new QLabel(tr("Choose Picture"), m_bottomFrame);

    // 3.2 本地图片
    //~ contents_path /Screenlock/Local Pictures
    m_localBtn = new QPushButton(tr("Local Pictures"), m_bottomFrame);

    // 3.3 在线图片
    //~ contents_path /Screenlock/Online Pictures
    m_onlineBtn = new QPushButton(tr("Online Pictures"), m_bottomFrame);

    // 3.4 恢复默认
    //~ contents_path /Screenlock/Reset To Default
    m_resetBtn = new QPushButton(tr("Reset To Default"), m_bottomFrame);

    m_bottomLayout = new QHBoxLayout(m_bottomFrame);
    m_bottomLayout->setContentsMargins(16, 8, 16, 8);
    m_bottomLayout->addWidget(chooseLabel);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_localBtn);
    m_bottomLayout->addWidget(m_onlineBtn);
    m_bottomLayout->addWidget(m_resetBtn);

    // 相关设置
    m_relateFrame = new UkccFrame(m_centerGroup, UkccFrame::None, false);
    m_relateFrame->setFixedHeight(60);
    relateTitleLabel  = new KLabel(m_relateFrame);
    QHBoxLayout *relateLayout = new QHBoxLayout(m_relateFrame);
    relateLayout->setContentsMargins(16, 16, 16, 16);
    relateTitleLabel->setText(tr("Related Settings"));
    relateLayout->addWidget(relateTitleLabel);
    relateLayout->addStretch();

    monitorWidget = new PushButtonWidget(tr("Monitor Off"), m_centerGroup);
    monitorWidget->setButtonText(tr("Set"));

    screensaverWidget = new PushButtonWidget(tr("Screensaver"), m_centerGroup);
    screensaverWidget->setButtonText(tr("Set"));

    m_centerGroup->addWidget(m_bottomFrame);
    m_centerGroup->addWidget(m_pictureFrame);
    m_centerGroup->addWidget(m_relateFrame);
    m_centerGroup->addWidget(monitorWidget);
    m_centerGroup->addWidget(screensaverWidget);

    QScrollArea *contentScrollArea = new QScrollArea;
    QPalette pal = contentScrollArea->palette();
    pal = contentScrollArea->palette();
    pal.setColor(QPalette::Active, QPalette::Base, QColor(Qt::transparent));
    pal.setColor(QPalette::Inactive, QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Disabled, QPalette::Base, Qt::transparent);
    contentScrollArea->setPalette(pal);
    contentScrollArea->setWidgetResizable(true);
    contentScrollArea->setFrameShape(QFrame::Box);
    contentScrollArea->setFrameShadow(QFrame::Plain);
    contentScrollArea->setWidget(m_centerGroup);

    m_screenlockLayout->addWidget(m_wallpaperTitleLabel);
    m_screenlockLayout->addWidget(previewScrollArea);
    m_screenlockLayout->addWidget(contentScrollArea);

#ifdef Nile
    QHBoxLayout *noBtVLayout = new QHBoxLayout();
    QLabel *noBtWarnIcon = new QLabel(m_noBtWidget);
    QLabel *noBtMessage = new QLabel(tr("No bluetooth adapter detected, can not use Leave Lock."),m_noBtWidget);

    noBtWarnIcon->setPixmap(icon.pixmap(18,18));
    noBtVLayout->setContentsMargins(16, 0, 16, 0);
    noBtVLayout->setSpacing(16);
    noBtVLayout->addWidget(noBtWarnIcon);
    noBtVLayout->addWidget(noBtMessage,Qt::AlignLeft);
    m_noBtWidget->setLayout(noBtVLayout);
    m_noBtWidget->setVisible(false);

    m_bluetoothWidget->setVisible(false);
#endif
}

void ScreenlockUi::initLeavLockStatus() {
    bool leaveLockPowerCtl = getLeaveLockPowerCtl();
    m_btLeaveLockSwitchButton->setEnabled(!leaveLockPowerCtl);
    bool ispowered = isBluetoothPowered();
    bool leaveLockOn = getLeavLockOn();//实际蓝牙维护的离座锁功能
    bool leaveLockPower = getLeaveLockPower();//离座锁按钮
    qDebug() << "leavelockpower" << leaveLockPower;
    bool hasAdapter = hasBluetoothAdapter();
    QMap<QString, QString> btDevices = getBtPairedDevices();

    m_bluetoothComboBox->clear();

    qDebug() << "checked" << m_btLeaveLockSwitchButton->isChecked();
    m_btLeaveLockSwitchButton->setChecked(leaveLockPower);
    if (leaveLockPower){
        m_previewFrame->setFixedHeight(245);
        previewScrollArea->setFixedHeight(244);
    }

    canLeaveLock = !btDevices.isEmpty();
    m_bluetoothComboBox->addItem(tr("Please select device"));
    for (auto dev : btDevices.keys()) {
        m_bluetoothComboBox->addItem(btDevices.value(dev), dev);
    }
    qDebug() << canLeaveLock << ispowered;
    if (canLeaveLock && ispowered) {
        lockdev = getLeaveLockDev();
        qDebug()<<  "lockdev" << lockdev;
        if (leaveLockOn && !lockdev.isEmpty() && canLeaveLock) {
            m_bluetoothComboBox->setCurrentIndex(m_bluetoothComboBox->findData(lockdev));
        } else {
            m_bluetoothComboBox->setCurrentIndex(0);
        }
    }
    if (!hasAdapter && leaveLockPower) {
        m_noBtWidget->setVisible(!hasAdapter);
        return;
    }
    if (m_bluetoothComboBox->count() == 1 && leaveLockPower) {
        m_setBtWidget->setVisible(true);
        m_noBtWidget->setVisible(false);
        m_bluetoothWidget->setVisible(false);
        return;
    }
    m_bluetoothWidget->setVisible(leaveLockPower && canLeaveLock && ispowered);
    m_setBtWidget->setVisible((!canLeaveLock || !ispowered) && leaveLockPower);
}

void ScreenlockUi::setBtPushButtonClickSlot() {
    QProcess *process = new QProcess();
    QString cmd = "ukui-control-center";
    QStringList arg;
    arg.clear();
    arg << "-m";
    arg << "Bluetooth";
    process->startDetached(cmd,arg);
}

void ScreenlockUi::btLeaveLockButtonClickSlot(bool clicked) {
    setLeaveLockPower(clicked);
    if (!clicked) {
        m_noBtWidget->setVisible(false);
        m_setBtWidget->setVisible(false);
        m_bluetoothWidget->setVisible(false);
        if (!lockdev.isEmpty()) {
            setLeaveLock(false);
        }
        lockdev.clear();
        m_bluetoothComboBox->setCurrentIndex(0);
        m_previewFrame->setFixedHeight(193);
        previewScrollArea->setFixedHeight(192);
        return;
    }
    m_previewFrame->setFixedHeight(245);
    previewScrollArea->setFixedHeight(244);
    m_bluetoothComboBox->setCurrentIndex(0);
    bool ispowered = isBluetoothPowered();
    bool hasAdapter = hasBluetoothAdapter();

    if (!hasAdapter) {
        m_noBtWidget->setVisible(!hasAdapter);
        return;
    }
    if (m_bluetoothComboBox->count() == 1) {
        m_setBtWidget->setVisible(true);

        m_noBtWidget->setVisible(false);
        m_bluetoothWidget->setVisible(false);
        return;
    }
    m_bluetoothWidget->setVisible(canLeaveLock && ispowered);
    m_setBtWidget->setVisible(!canLeaveLock || !ispowered);
}

void ScreenlockUi::bluetoothWidgetIdxChangeSlot(int index) {
    if (!lockdev.isEmpty()) {
        setLeaveLock(false);
    }
    if (index == 0) {
        lockdev.clear();
        return;
    }
    lockdev = m_bluetoothComboBox->currentData().toString();
    setLeaveLock(true);
}

void ScreenlockUi::initSignals()
{
    connect(m_showPicSwitchButton, &KSwitchButton::clicked, this, [=](){
        Q_EMIT showOnLoginChanged(m_showPicSwitchButton->isChecked());
    });
    connect(m_lockDatetimeEnableBtn, &KSwitchButton::clicked, this, [=](){
        Q_EMIT lockDatetimeEnableChanged(m_lockDatetimeEnableBtn->isChecked());
    });
#ifdef Nile
    connect(m_btLeaveLockSwitchButton, &KSwitchButton::clicked, this, [=](bool clicked){
        btLeaveLockButtonClickSlot(clicked);
    });

    connect(m_bluetoothComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        bluetoothWidgetIdxChangeSlot(index);
    });

    connect(m_setBtPushButton, &QPushButton::clicked, this, [=](){
        setBtPushButtonClickSlot();
    });
#endif
    connect(monitorWidget, &PushButtonWidget::clicked, this, [=](){
        Q_EMIT toSetMonitor();
    });
    connect(screensaverWidget, &PushButtonWidget::clicked, this, [=](){
        Q_EMIT toSetScreensaver();
    });
    connect(m_localBtn, &QPushButton::clicked, this, [=](){
        Q_EMIT localButtonClicked();
    });
    connect(m_onlineBtn, &QPushButton::clicked, this, [=](){
        Q_EMIT onlineButtonClicked();
    });
    connect(m_resetBtn, &QPushButton::clicked, this, [=](){
        Q_EMIT resetButtonClicked();
    });
#ifdef Nile
    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "leaveLockCtlChanged",
                                          this,
                                          SLOT(leaveLockCtlChanged(bool)));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "devConnectStatusSignal",
                                          this,
                                          SLOT(devConnectStatusChanged(QString, bool)));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "leaveLockSuc",
                                          this,
                                          SLOT(leaveLockSuc()));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "devPairSignal",
                                          this,
                                          SLOT(addPairedDev(QString,bool)));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "defaultAdapterPowerChanged",
                                          this,
                                          SLOT(powerChanged(bool)));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "defaultAdapterChanged",
                                          this,
                                          SLOT(defaultAdapterChanged(QString)));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "adapterAddSignal",
                                          this,
                                          SLOT(adapterAdded(QString)));

    QDBusConnection::sessionBus().connect("com.ukui.bluetooth",
                                          "/com/ukui/bluetooth",
                                          "com.ukui.bluetooth",
                                          "adapterRemoveSignal",
                                          this,
                                          SLOT(adapterRemoved(QString)));
#endif
}

void ScreenlockUi::setShowOnLogin(bool b)
{
    m_showPicSwitchButton->blockSignals(true);
    m_showPicSwitchButton->setChecked(b);
    m_showPicSwitchButton->blockSignals(false);
}

void ScreenlockUi::setLockDatetimeEnable(bool b)
{
    m_lockDatetimeEnableBtn->blockSignals(true);
    m_lockDatetimeEnableBtn->setChecked(b);
    m_lockDatetimeEnableBtn->blockSignals(false);
}

void ScreenlockUi::setPicture(const QString &data)
{
    currentFile = data;
    QImageReader reader(data);
    reader.setDecideFormatFromContent(true);
    m_previewLabel->setPixmap(PixmapToRound(QPixmap::fromImage(reader.read()).scaled(280, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation), 10, 10, 10, 10));
    resetClickedPic();
}

void ScreenlockUi::setPictures(const QStringList &picturePathList, const QStringList &sourcePathList)
{
    if (picturePathList.size() != sourcePathList.size()) {
        qWarning()<<"picturePathList.size():"<<picturePathList.size()<<" != sourcePathList.size():"<<sourcePathList.size();
        return;
    }

    for (int i = 0; i < picturePathList.size(); ++i) {
        QPixmap pixmap;
        pixmap.load(picturePathList.at(i));
        bool clicked = false;
        if (sourcePathList.at(i) == currentFile) {
            clicked = true;
        }
        createPictureUnit(pixmap, sourcePathList.at(i), clicked);
    }
}

void ScreenlockUi::createPictureUnit(const QPixmap &pixmap, const QString &filename, const bool &clicked)
{
    PhotoCorver *photoCorver = new PhotoCorver(this, true, false);
    KDK_EXTEND_ALL_INFO_FORMAT(photoCorver, "ScreenlockUi", "createPictureUnit", "Image for screensaver background");
    photoCorver->setBackground(pixmap);
    if (clicked) {
        prePictureUnit = photoCorver;
        photoCorver->setClickStatus(clicked);
        setPicture(filename);
    }
    photoCorver->setObjectName(filename);
    connect(photoCorver, &PhotoCorver::photoCorverLeftClicked, [=](){
        setPicture(filename);
        Q_EMIT pictureChanged(filename);
    });
    m_pictureLayout->addWidget(photoCorver);
}

void ScreenlockUi::resetClickedPic() {
    if (prePictureUnit != nullptr) {
        prePictureUnit->setClickStatus(false);
        prePictureUnit = nullptr;
    }
    for (int i = m_pictureLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *it      = m_pictureLayout->itemAt(i);
        PhotoCorver *picUnit = static_cast<PhotoCorver*>(it->widget());
        if (currentFile == picUnit->objectName()) {
            picUnit->setClickStatus(true);
            prePictureUnit = picUnit;
        }
    }
}

QMap<QString, QString> ScreenlockUi::getBtPairedDevices() {
    QMap<QString, QString> pairedList;
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth",
                                                      "getPairedPhoneAddr");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        QStringList ret = response.arguments().takeFirst().toStringList();
        if (ret.isEmpty())
            return pairedList;
        for (int i = 0; i < ret.size(); i++) {
            if (pairedList.contains(ret.at(i)))
                continue;
            QDBusMessage _message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                                   "/com/ukui/bluetooth",
                                                                   "com.ukui.bluetooth",
                                                                   "getDevName");
            _message << ret.at(i);
            QDBusMessage _response = QDBusConnection::sessionBus().call(_message);
            if (_response.type() == QDBusMessage::ReplyMessage)
            {
                QString devName = _response.arguments().takeFirst().toString();
                if (devName == "")
                    continue;
                pairedList.insert(ret.at(i), devName);
            }
        }
    }
    return pairedList;
}

bool ScreenlockUi::hasBluetoothAdapter() {
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth", \
                                                      "getAdapterDevAddressList");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        QStringList ret = response.arguments().takeFirst().toStringList();
        return !ret.isEmpty();
    } else
        return false;
}

bool ScreenlockUi::isBluetoothPowered() {
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth",
                                                      "getDefaultAdapterPower");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        return response.arguments().takeFirst().toBool();
    } else
        return false;
}

void ScreenlockUi::leaveLockCtlChanged(bool on) {
    qDebug() << "leaveLockCtlChanged" << on;
    m_btLeaveLockSwitchButton->setEnabled(!on);
}

bool ScreenlockUi::getLeaveLockPowerCtl() {
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                          "/com/ukui/bluetooth",
                                                          "com.ukui.bluetooth",
                                                          "getLeaveLockPowerCtl");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
        return response.arguments().takeFirst().toBool();
    else
        return false;
}

void ScreenlockUi::devConnectStatusChanged(QString dev, bool status) {
    qDebug() << "dev" << dev << "status" << status << "lockdev" << lockdev;
    if (dev == lockdev && !status) {
        return ;
    }
    if (dev != lockdev) {
        initLeavLockStatus();
    }
}

void ScreenlockUi::leaveLockSuc()
{
    setLeaveLock(false);
    initLeavLockStatus();
}

void ScreenlockUi::addPairedDev(QString dev, bool paired) {
    int index = m_bluetoothComboBox->findData(dev);
    if (paired && index >= 0)
        return;

    if (!paired && index >= 0) {
        m_bluetoothComboBox->removeItem(m_bluetoothComboBox->findData(dev));
        if (m_bluetoothComboBox->count() == 1) {
            m_noBtWidget->setVisible(false);
            m_bluetoothWidget->setVisible(false);
            m_setBtWidget->setVisible(true);
        }
        if (lockdev == dev)
            lockdev.clear();
        return;
    }

    // 鼠标等蓝牙设备不支持离座锁，需要从getPairedPhoneAddr获取
    QMap<QString, QString> btDevices = getBtPairedDevices();
    auto it = btDevices.find(dev);
    if (it == btDevices.end()) {
        return;
    }

    QString devName = it.value();
    if (!devName.isEmpty()) {
        m_bluetoothComboBox->addItem(devName, dev);
        if (getLeaveLockPower()) {
            m_setBtWidget->setVisible(false);
            m_bluetoothWidget->setVisible(true);
        }
    }
}

void ScreenlockUi::powerChanged(bool powered) {
    if (!m_btLeaveLockSwitchButton->isChecked())
        return;
    if (!hasBluetoothAdapter()) {
        m_noBtWidget->setVisible(true);
        m_setBtWidget->setVisible(false);
        m_bluetoothWidget->setVisible(false);
        return;
    }

    m_noBtWidget->setVisible(false);
    m_setBtWidget->setVisible(!powered);
    m_bluetoothWidget->setVisible(powered);
    setLeaveLock(false);
}

void ScreenlockUi::setLeaveLockPower(bool on)
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                          "/com/ukui/bluetooth",
                                                          "com.ukui.bluetooth",
                                                          "setLeaveLockPower");
    message << on;
    QDBusConnection::sessionBus().call(message);
}

bool ScreenlockUi::getLeaveLockPower()
{
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                          "/com/ukui/bluetooth",
                                                          "com.ukui.bluetooth",
                                                          "getLeaveLockPower");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
        return response.arguments().takeFirst().toBool();
    else
        return false;
}

void ScreenlockUi::setLeaveLock(bool on) {
    if (lockdev.isEmpty())
        return;
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth",
                                                      "setLeaveLock");
    message << lockdev << on;
    QDBusConnection::sessionBus().call(message);
}

bool ScreenlockUi::getLeavLockOn() {
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth",
                                                      "getLeaveLock");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
        return response.arguments().takeFirst().toBool();
    else
        return false;
}

QString ScreenlockUi::getLeaveLockDev() {
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth",
                                                      "getLeaveLockDev");
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
        return response.arguments().takeFirst().toString();
    else
        return QString("");
}

void ScreenlockUi::defaultAdapterChanged(QString address) {
    Q_UNUSED(address);
    if (!m_btLeaveLockSwitchButton->isChecked())
        return;
    bool ispowered = isBluetoothPowered();
    bool leaveLockOn = getLeavLockOn();
    bool hasAdapter = hasBluetoothAdapter();
    QMap<QString, QString> btDevices = getBtPairedDevices();

    lockdev.clear();
    m_bluetoothComboBox->clear();

    if (leaveLockOn && !m_btLeaveLockSwitchButton->isChecked()) {
        m_btLeaveLockSwitchButton->click();
    }

    canLeaveLock = !btDevices.isEmpty();
    m_bluetoothComboBox->addItem(tr("Please select device"));
    for (auto dev : btDevices.keys())
        m_bluetoothComboBox->addItem(btDevices.value(dev), dev);

    if (!hasAdapter && leaveLockOn) {
        m_noBtWidget->setVisible(!hasAdapter);
        return;
    }
    m_bluetoothWidget->setVisible(leaveLockOn && canLeaveLock && ispowered);
    m_setBtWidget->setVisible((!canLeaveLock || !ispowered) && leaveLockOn);
}

void ScreenlockUi::adapterAdded(QString address) {
    Q_UNUSED(address);
    if (!m_btLeaveLockSwitchButton->isChecked())
        return;
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth", \
                                                      "getAdapterDevAddressList");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        QStringList ret = response.arguments().takeFirst().toStringList();
        if (ret.size() > 1)
            return;
        else {
            bool ispowered = isBluetoothPowered();
            QMap<QString, QString> btDevices = getBtPairedDevices();

            lockdev.clear();
            m_bluetoothComboBox->clear();

            canLeaveLock = !btDevices.isEmpty();
            m_bluetoothComboBox->addItem(tr("Please select device"));
            for (auto dev : btDevices.keys()) {
                m_bluetoothComboBox->addItem(btDevices.value(dev), dev);
            }
            m_noBtWidget->setVisible(false);
            m_setBtWidget->setVisible(false);
            m_bluetoothWidget->setVisible(canLeaveLock && ispowered);
            m_setBtWidget->setVisible(!canLeaveLock || !ispowered);
        }
    }
}

void ScreenlockUi::adapterRemoved(QString address) {
    Q_UNUSED(address);
    if (!m_btLeaveLockSwitchButton->isChecked())
        return;
    QDBusMessage message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                      "/com/ukui/bluetooth",
                                                      "com.ukui.bluetooth", \
                                                      "getAdapterDevAddressList");

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        QStringList ret = response.arguments().takeFirst().toStringList();
        if (ret.size() < 1){
            lockdev.clear();
            m_bluetoothComboBox->clear();
            m_noBtWidget->setVisible(true);
            m_bluetoothWidget->setVisible(false);
            m_setBtWidget->setVisible(false);
        }
    }
}
