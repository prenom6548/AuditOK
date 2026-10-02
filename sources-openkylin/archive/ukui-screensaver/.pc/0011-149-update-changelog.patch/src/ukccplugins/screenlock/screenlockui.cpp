/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "screenlockui.h"
#include <QImageReader>
#include <QScrollArea>
#include <QScrollBar>

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
    m_previewLayout->setContentsMargins(16, 12, 16, 16);

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
    QLabel* showPicLabel = new QLabel(tr("Show picture of screenlock on screenlogin"), showPicWidget);
    m_showPicSwitchButton = new KSwitchButton(showPicWidget);
    typeLayout->addWidget(showPicLabel);
    typeLayout->addStretch();
    typeLayout->addWidget(m_showPicSwitchButton);
    contentList.append(showPicWidget);

#ifdef Nile
    // 动态锁
    QWidget* btLeaveLockWidget = new QWidget();
    QHBoxLayout* btLeaveLockLayout = new QHBoxLayout(btLeaveLockWidget);
    btLeaveLockLayout->setContentsMargins(16, 0, 0, 0);
    btLeaveLockLayout->setSpacing(0);
    QLabel* btLeaveLockLabel = new QLabel(tr("Leave lock (System will be locked when the paired phone gone)"), btLeaveLockWidget);
    m_btLeaveLockSwitchButton = new KSwitchButton(btLeaveLockWidget);
    btLeaveLockLayout->addWidget(btLeaveLockLabel);
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

    m_noBtWidget = new UkccFrame(m_centerGroup, UkccFrame::None,true);
    contentList.append(m_noBtWidget);
#endif
    m_contentBackgroundGroup->addWidgetList(contentList);

    // 预览区域滚动条设置
    QScrollArea *previewScrollArea = new QScrollArea;
    previewScrollArea->setFixedHeight(192);
    previewScrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    previewScrollArea->setStyleSheet("QScrollArea{background-color: transparent;}");
    previewScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    previewScrollArea->verticalScrollBar()->setEnabled(false);
    previewScrollArea->setWidgetResizable(true);
    previewScrollArea->setWidget(m_previewFrame);

    QVBoxLayout* rightPreviewLayout = new QVBoxLayout();
    rightPreviewLayout->setContentsMargins(8, 0, 16, 0);
    rightPreviewLayout->setSpacing(0);
    rightPreviewLayout->addWidget(m_contentBackgroundGroup);
    rightPreviewLayout->addStretch();

    m_previewLayout->addWidget(m_previewLabel);
    m_previewLayout->addSpacing(0);
    m_previewLayout->addLayout(rightPreviewLayout);

    m_picBtnFrame = new UkccFrame(m_centerGroup);
    m_picBtnFrame->setLineWidth(0);

    m_pictureFrame = new UkccFrame(m_picBtnFrame);
    m_pictureFrame->setLineWidth(0);

    m_pictureLayout = new FlowLayout(m_pictureFrame, 8, -1, -1);
    m_pictureLayout->setContentsMargins(16, 0, 16, 0);

    m_bottomFrame = new UkccFrame(m_picBtnFrame);

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
    m_relateFrame = new UkccFrame(m_picBtnFrame);
    m_relateLayout = new QVBoxLayout(m_relateFrame);
    UkccFrame* titleFrame = new UkccFrame();
    QHBoxLayout* relateTitleLayout = new QHBoxLayout(titleFrame);

    relateTitleLayout->setContentsMargins(0, 0, 0, 0);
    relateTitleLabel  = new KLabel(m_relateFrame);
    relateTitleLabel->setContentsMargins(16, 0, 0, 0);
    relateTitleLayout->addWidget(relateTitleLabel);
    relateTitleLabel->setText(tr("Related Settings"));

    KBackgroundGroup* relateGroup = new KBackgroundGroup();
    monitorWidget = new PushButtonWidget(tr("Monitor Off"), m_relateFrame);
    monitorWidget->setButtonText(tr("Set"));
    screensaverWidget = new PushButtonWidget(tr("Screensaver"), m_relateFrame);
    screensaverWidget->setButtonText(tr("Set"));

    relateGroup->addWidget(titleFrame);
    relateGroup->addWidget(monitorWidget);
    relateGroup->addWidget(screensaverWidget);

    m_relateLayout->addWidget(relateGroup);

    m_picBtnLayout = new QVBoxLayout(m_picBtnFrame);
    m_picBtnLayout->setMargin(0);
    m_picBtnLayout->setSpacing(0);
    m_picBtnLayout->addWidget(m_bottomFrame);
    m_picBtnLayout->addWidget(m_pictureFrame);
    m_picBtnLayout->addWidget(m_relateFrame);

    m_centerGroup->addWidget(m_picBtnFrame);

    QScrollArea *contentScrollArea = new QScrollArea;
    contentScrollArea->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    contentScrollArea->setStyleSheet("QScrollArea{background-color: transparent;}");
    contentScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
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
    bool ispowered = isBluetoothPowered();
    bool leaveLockOn = getLeavLockOn();
    bool hasAdapter = hasBluetoothAdapter();
    QMap<QString, QString> btDevices = getBtPairedDevices();

    m_bluetoothComboBox->clear();

    if (leaveLockOn) {
        m_btLeaveLockSwitchButton->click();
    }

    canLeaveLock = !btDevices.isEmpty();
    m_bluetoothComboBox->addItem(tr("Please select device"));
    for (auto dev : btDevices.keys()) {
        m_bluetoothComboBox->addItem(btDevices.value(dev), dev);
    }
    if (canLeaveLock && ispowered) {
        lockdev = getLeaveLockDev();
        if (!lockdev.isEmpty() && canLeaveLock) {
            m_bluetoothComboBox->setCurrentIndex(m_bluetoothComboBox->findData(lockdev));
        }
    }
    if (!hasAdapter && leaveLockOn) {
        m_noBtWidget->setVisible(!hasAdapter);
        return;
    }
    if (m_bluetoothComboBox->count() == 1 && leaveLockOn) {
        m_setBtWidget->setVisible(true);
        m_noBtWidget->setVisible(false);
        m_bluetoothWidget->setVisible(false);
        return;
    }
    m_bluetoothWidget->setVisible(leaveLockOn && canLeaveLock && ispowered);
    m_setBtWidget->setVisible((!canLeaveLock || !ispowered) && leaveLockOn);
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
    if (!clicked) {
        m_noBtWidget->setVisible(false);
        m_setBtWidget->setVisible(false);
        m_bluetoothWidget->setVisible(false);
        if (!lockdev.isEmpty()) {
            setLeaveLock(false);
        }
        lockdev.clear();
        m_bluetoothComboBox->setCurrentIndex(0);
        return;
    }
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

void ScreenlockUi::setPicture(const QString &data)
{
    currentFile = data;
    QImageReader reader(data);
    reader.setDecideFormatFromContent(true);
    m_previewLabel->setPixmap(QPixmap::fromImage(reader.read()).scaled(m_previewLabel->size()));
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
    PictureUnit *picUnit = new PictureUnit(this);
    picUnit->setPixmap(pixmap);
    picUnit->setFilenameText(filename);
    if (clicked) {
        prePictureUnit = picUnit;
        picUnit->changeClickedFlag(true);
        setPicture(filename);
    }
    connect(picUnit, &PictureUnit::clicked, [=](QString file){
        if (prePictureUnit) {
            prePictureUnit->changeClickedFlag(false);
        }
        prePictureUnit = picUnit;
        setPicture(file);
        Q_EMIT pictureChanged(file);
    });
    m_pictureLayout->addWidget(picUnit);
}

void ScreenlockUi::resetClickedPic() {
    if (prePictureUnit != nullptr) {
        prePictureUnit->changeClickedFlag(false);
        prePictureUnit = nullptr;
    }
    for (int i = m_pictureLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *it      = m_pictureLayout->itemAt(i);
        PictureUnit *picUnit = static_cast<PictureUnit*>(it->widget());
        if (currentFile == picUnit->filenameText()) {
            picUnit->changeClickedFlag(true);
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

    QDBusMessage _message = QDBusMessage::createMethodCall("com.ukui.bluetooth",
                                                           "/com/ukui/bluetooth",
                                                           "com.ukui.bluetooth",
                                                           "getDevName");
    _message << dev;
    QDBusMessage _response = QDBusConnection::sessionBus().call(_message);
    if (_response.type() == QDBusMessage::ReplyMessage)
    {
        QString devName = _response.arguments().takeFirst().toString();
        if (devName != "") {
            m_bluetoothComboBox->addItem(devName, dev);
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
