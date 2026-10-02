/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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
#include "accessibility.h"
#include "accessibilityui.h"

#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLayoutItem>
#include <QPluginLoader>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#define UKUI_STYLE_ID                    "org.ukui.style"
#define UKUI_STYLE_NAME                  "style-name"
#define UKCC_SCHEMA                      "org.ukui.control-center"
#define GNOME_A11Y_APPLICATIONS_SCHEMA   "org.gnome.desktop.a11y.applications"
#define SCREEN_READER_ENABLED_KEY        "screen-reader-enabled"
#define SCREEN_READER_ENABLED_UKUI_KEY   "screenReaderEnabled"
#define SCREEN_READER_VOLUME_KEY         "reader-volume"
#define SCREEN_READER_SPEED_KEY          "reader-speed"
#define SCREEN_READER_SPEED_MIN          30
#define SCREEN_READER_SPEED_MAX          100
#define SCREEN_READER_VOLUME_GET_KEY         "readerVolume"
#define SCREEN_READER_SPEED_GET_KEY          "readerSpeed"
#define SCREEN_READER_ORCA_VERIFY_MS         4000
#define SCREEN_READER_ORCA_VERIFY_MAX_ATTEMPTS 15
#define SCREEN_READER_START_GRACE_MS         60000

#define PLUGINS_SCHEMA                       "org.ukui.control-center.plugins"
#define SUB_PLUGINS_GSETTINGS_PREFIX         "/org/ukui/control-center/plugins/"
#define SUB_PLUGIN_GSETTINGS_NAME_KEY        "plugin-name"
#define SUB_PLUGIN_GSETTINGS_SHOW_KEY        "show"

static void suppressLabelForScreenReader(QLabel *label)
{
    if (!label) {
        return;
    }
    label->setFocusPolicy(Qt::NoFocus);
    // 非空 accessibleName 可避免 Qt 回退朗读可见文本
    label->setAccessibleName(QStringLiteral(" "));
}

static bool isOrcaInstalled()
{
    QProcess process;
    process.start(QStringLiteral("bash"),
                  QStringList() << QStringLiteral("-c") << QStringLiteral("dpkg -l | grep orca"));
    if (!process.waitForFinished(3000)) {
        qWarning() << Q_FUNC_INFO << "dpkg orca check timeout";
        process.kill();
        return false;
    }
    const QByteArray output = process.readAllStandardOutput();
    for (const QByteArray &line : output.split('\n')) {
        const QByteArray trimmed = line.trimmed();
        // dpkg -l 已安装包行首为 ii
        if (trimmed.startsWith("ii") && trimmed.contains(" orca")) {
            return true;
        }
    }
    qDebug() << Q_FUNC_INFO << "orca package not installed, dpkg output:" << output;
    return false;
}

static bool isXunfeiAikitAvailable()
{
#ifndef SUB_PLUGIN_INSTALL_DIRS
    qWarning() << Q_FUNC_INFO << "SUB_PLUGIN_INSTALL_DIRS undefined";
    return false;
#else
    const QString path = QDir(QStringLiteral(SUB_PLUGIN_INSTALL_DIRS))
                             .filePath(QStringLiteral("libaikit.so"));
    if (QFileInfo::exists(path)) {
        qDebug() << Q_FUNC_INFO << "found" << path;
        return true;
    }
    qDebug() << Q_FUNC_INFO << "libaikit.so not found under"
             << QStringLiteral(SUB_PLUGIN_INSTALL_DIRS)
             << ", hide screen reader UI";
    return false;
#endif
}

static bool readScreenReaderEnabled(QGSettings *settings)
{
    if (!settings) {
        return false;
    }
    const QStringList keys = settings->keys();
    bool enabled = false;
    if (keys.contains(SCREEN_READER_ENABLED_KEY)) {
        enabled = enabled || settings->get(SCREEN_READER_ENABLED_KEY).toBool();
    }
    if (keys.contains(SCREEN_READER_ENABLED_UKUI_KEY)) {
        enabled = enabled || settings->get(SCREEN_READER_ENABLED_UKUI_KEY).toBool();
    }
    return enabled;
}

bool Accessibility::isWithinScreenReaderStartGracePeriod() const
{
    if (m_screenReaderEnableRequestedAt <= 0) {
        return false;
    }
    return QDateTime::currentMSecsSinceEpoch() - m_screenReaderEnableRequestedAt
           < SCREEN_READER_START_GRACE_MS;
}

void Accessibility::scheduleOrcaVerify()
{
    QTimer::singleShot(SCREEN_READER_ORCA_VERIFY_MS, this, [this]() {
        verifyOrcaRunningAfterEnable();
    });
}

void Accessibility::verifyOrcaRunningAfterEnable()
{
    if (!m_pScreenReaderSwitchButton || !m_pScreenReaderSwitchButton->isChecked()) {
        m_orcaVerifyAttempt = 0;
        return;
    }
    if (!m_accessibilityInterface) {
        return;
    }

    QDBusReply<bool> isScreenReaderRunningReply = m_accessibilityInterface->call("isScreenReaderRunning");
    qDebug() << Q_FUNC_INFO << "attempt" << (m_orcaVerifyAttempt + 1)
             << "of" << SCREEN_READER_ORCA_VERIFY_MAX_ATTEMPTS
             << "isScreenReaderRunningReply" << isScreenReaderRunningReply.isValid()
             << isScreenReaderRunningReply.value();
    if (isScreenReaderRunningReply.isValid() && isScreenReaderRunningReply.value()) {
        m_orcaVerifyAttempt = 0;
        return;
    }

    m_orcaVerifyAttempt++;
    if (m_orcaVerifyAttempt < SCREEN_READER_ORCA_VERIFY_MAX_ATTEMPTS) {
        scheduleOrcaVerify();
        return;
    }

    m_orcaVerifyAttempt = 0;
    qDebug() << Q_FUNC_INFO << "orca not running after retries, disable screen reader switch";
    if (m_pScreenReaderSetting) {
        m_pScreenReaderSetting->set(SCREEN_READER_ENABLED_KEY, false);
    }
    syncScreenReaderSwitch(false);
    onScreenReaderSwitchToggled(false);
}

const QString m_filterPreview = ":/images/filterPreview.png";

Accessibility::Accessibility() : m_firstLoad(true)
{
    m_pluginName = tr("Accessibility");
    m_pluginType = COMMONINFO;
    m_accessibilityInterface = new QDBusInterface("org.ukui.ukcc.session",
                                                  "/Accessibility",
                                                  "org.ukui.ukcc.session.Accessibility",
                                                  QDBusConnection::sessionBus(), this);
    if (!m_accessibilityInterface) {
        qCritical() << Q_FUNC_INFO << "org.ukui.ukcc.session.Accessibility QDBusInterface is null";
        return;
    }

    QDBusReply<bool> effectReply = m_accessibilityInterface->call("isEffectSupported");
    m_isSupportEffect = effectReply.value();
    qDebug() << Q_FUNC_INFO << "isEffectSupported valid" << effectReply.isValid() << effectReply.value();
}

Accessibility::~Accessibility()
{
}

QString Accessibility::plugini18nName()
{
    return m_pluginName;
}

int Accessibility::pluginTypes()
{
    return m_pluginType;
}

QWidget *Accessibility::pluginUi()
{
    if (m_firstLoad) {
        m_firstLoad = false;

        ui = new Ui::Accessibility;
        m_pPluginWidget = new QWidget;
        m_pPluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(m_pPluginWidget);

        initUI();
        initQGSettingsAndDBus();
        initDefaultData();
        initConnection();
        initSubPluginArea(); // 加载子插件
    } else if (m_isOrcaInstalled && m_pScreenReaderSwitchButton
               && m_pScreenReaderSwitchButton->isChecked() && m_accessibilityInterface
               && !isWithinScreenReaderStartGracePeriod()) {
        QDBusReply<bool> isScreenReaderRunningReply = m_accessibilityInterface->call("isScreenReaderRunning");
        if (isScreenReaderRunningReply.isValid() && !isScreenReaderRunningReply.value()) {
            qDebug() << Q_FUNC_INFO << "re-enter accessibility page, orca not running, reset switch to false";
            if (m_pScreenReaderSetting) {
                m_pScreenReaderSetting->set(SCREEN_READER_ENABLED_KEY, false);
            }
            syncScreenReaderSwitch(false);
            onScreenReaderSwitchToggled(false);
        }
    }
    return m_pPluginWidget;
}

const QString Accessibility::name() const
{
    return QStringLiteral("Accessibility");
}

bool Accessibility::isShowOnHomePage() const
{
    return true;
}

QIcon Accessibility::icon() const
{
    return QIcon::fromTheme("preferences-desktop-accessibility-symbolic");
}

bool Accessibility::isEnable() const
{
    qDebug() << Q_FUNC_INFO << "isSupportEffect" << m_isSupportEffect;
    return m_isSupportEffect;
}

void Accessibility::initUI()
{
    const bool orcaInstalled = isOrcaInstalled();
    const bool aikitAvailable = isXunfeiAikitAvailable();
    // openKylin 默认 kylin_speech，装 Orca 即显示「屏幕朗读」；
    // 商业版默认讯飞，还需 libaikit，缺则隐藏整块界面。
    if (UkccCommon::isOpenkylin()) {
        m_isOrcaInstalled = orcaInstalled;
    } else {
        m_isOrcaInstalled = orcaInstalled && aikitAvailable;
    }
    qDebug() << Q_FUNC_INFO << "orca installed" << orcaInstalled
             << "xunfei libaikit" << aikitAvailable
             << "openkylin" << UkccCommon::isOpenkylin()
             << "screen reader ui" << m_isOrcaInstalled;
    if (!m_isOrcaInstalled) {
        ui->m_pScreenReaderFrame->setVisible(false);
    }

    m_pFilterSwitchButton   = new KSwitchButton(ui->m_pFilterFrame);
    m_pShortcutSwitchButton = new KSwitchButton(ui->m_pShortcutFrame);
    if (m_isOrcaInstalled) {
        m_pScreenReaderSwitchButton = ui->m_screenReaderSwitchWidget->switchButton();
    }
    KDK_EXTEND_ALL_INFO_FORMAT(m_pFilterSwitchButton, name(), "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_pShortcutSwitchButton, name(), "", "");

    ui->m_pFilterHorizontalLayout->addWidget(m_pFilterSwitchButton, 0, Qt::AlignRight);
    ui->m_pShortcutHorizontalLayout->addWidget(m_pShortcutSwitchButton, 0, Qt::AlignRight);

    m_pFilterSwitchButton->setAccessibleName(tr("Color Filter"));
    m_pShortcutSwitchButton->setAccessibleName(
        tr("Press Super + Ctrl + C to turn on or off color filter"));
    // 左侧说明文字不参与焦点链；开关已有完整 accessibleName，勿 setBuddy（会关联空格 label 盖掉 name）。
    suppressLabelForScreenReader(ui->m_pShortcutLabel);

    ui->m_zoomSwitchWidget->switchButton()->setAccessibleName(
        tr("Enable magnifying glass: Enlarge the content of the desktop"));

    if (m_isOrcaInstalled) {
        m_pScreenReaderSwitchButton->setAccessibleName(tr("Enable Screen Reader"));

        ui->m_screenReaderVolumeSliderWidget->slider()->setRange(0, 100);
        ui->m_screenReaderVolumeSliderWidget->slider()->setValue(50);
        ui->m_screenReaderVolumeSliderWidget->leftLabel()->setPixmap(
            QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(16, 16));
        ui->m_screenReaderVolumeSliderWidget->leftLabel()->setFixedWidth(16);

        ui->m_screenReaderSpeedSliderWidget->slider()->setRange(SCREEN_READER_SPEED_MIN, SCREEN_READER_SPEED_MAX);
        ui->m_screenReaderSpeedSliderWidget->slider()->setValue(50);
        ui->m_screenReaderSpeedSliderWidget->setLeftText(tr("Slow"));
        ui->m_screenReaderSpeedSliderWidget->setRightText(tr("Fast"));

        KSlider *volumeSlider = ui->m_screenReaderVolumeSliderWidget->slider();
        KSlider *speedSlider = ui->m_screenReaderSpeedSliderWidget->slider();
        ui->m_screenReaderVolumeSliderWidget->setFocusPolicy(Qt::NoFocus);
        ui->m_screenReaderSpeedSliderWidget->setFocusPolicy(Qt::NoFocus);
        volumeSlider->setFocusPolicy(Qt::StrongFocus);
        speedSlider->setFocusPolicy(Qt::StrongFocus);
        volumeSlider->setAccessibleName(tr("Reader Volume"));
        speedSlider->setAccessibleName(tr("Reader Speed"));

        ui->m_screenReaderVolumeSliderWidget->setEnabled(false);
        ui->m_screenReaderSpeedSliderWidget->setEnabled(false);
    }

    Q_ASSERT(!m_filterPreview.isNull());
    QPixmap filterViewPixmap(QIcon(m_filterPreview).pixmap(380,110));
    ui->m_pEffectPixmapLabel->setPixmap(filterViewPixmap);

    ui->m_pZoomComboBox->addItem(tr("Window Zoom"));
    ui->m_pZoomComboBox->addItem(tr("Full Screen Zoom"));

    ui->m_pFilterComboBox->addItem(tr("Red/Green Filter (Protanopia)"), QVariant("Protanopia"));
    ui->m_pFilterComboBox->addItem(tr("Green/Red Filter (Deuteranopia)"), QVariant("Deuteranopia"));
    ui->m_pFilterComboBox->addItem(tr("Blue/Yellow Filter (Tritanopia)"), QVariant("Tritanopia"));
    ui->m_pFilterComboBox->addItem(tr("Grayscale"), QVariant("GrayScale"));
    ui->m_pFilterComboBox->addItem(tr("Invert"), QVariant("Invert"));

    // setBuddy 建立 LABELLED_BY，读屏聚焦 combo 时 Orca 能读到行标题
    ui->m_pUseZoomLabel->setAccessibleName(tr("Zoom"));
    ui->m_pUseZoomLabel->setBuddy(ui->m_pZoomComboBox);
    ui->m_pZoomComboBox->setAccessibleName(tr("Zoom"));

    ui->m_pFilterLabel->setAccessibleName(tr("Color Filter"));
    ui->m_pFilterLabel->setBuddy(ui->m_pFilterComboBox);
    ui->m_pFilterComboBox->setAccessibleName(tr("Color Filter"));

    suppressLabelForScreenReader(ui->m_pPointerSetLbel);
    ui->m_pPointerSetBtn->setAccessibleName(
        tr("%1 %2").arg(tr("Pointer Size"), tr("Set")));
    suppressLabelForScreenReader(ui->m_pVocalSetLbel);
    ui->m_pVocalSetBtn->setAccessibleName(
        tr("%1 %2").arg(tr("Vocal Tract Regulation"), tr("Set")));

    initTabOrder();
}

void Accessibility::initTabOrder()
{
    KSwitchButton *zoomSwitch = ui->m_zoomSwitchWidget->switchButton();

    m_pFilterSwitchButton->setFocusPolicy(Qt::StrongFocus);
    m_pShortcutSwitchButton->setFocusPolicy(Qt::StrongFocus);

    // 按界面自上而下顺序设置 Tab 键访问链
    QWidget::setTabOrder(zoomSwitch, ui->m_pZoomComboBox);
    QWidget::setTabOrder(ui->m_pZoomComboBox, ui->m_pFilterComboBox);
    QWidget::setTabOrder(ui->m_pFilterComboBox, m_pFilterSwitchButton);
    QWidget::setTabOrder(m_pFilterSwitchButton, m_pShortcutSwitchButton);
    if (m_isOrcaInstalled) {
        ui->m_screenReaderSwitchWidget->setFocusPolicy(Qt::NoFocus);
        ui->m_screenReaderSwitchWidget->setFocusProxy(m_pScreenReaderSwitchButton);
        m_pScreenReaderSwitchButton->setFocusPolicy(Qt::StrongFocus);

        KSlider *volumeSlider = ui->m_screenReaderVolumeSliderWidget->slider();
        KSlider *speedSlider = ui->m_screenReaderSpeedSliderWidget->slider();
        QWidget::setTabOrder(m_pShortcutSwitchButton, m_pScreenReaderSwitchButton);
        QWidget::setTabOrder(m_pScreenReaderSwitchButton, volumeSlider);
        QWidget::setTabOrder(volumeSlider, speedSlider);
        QWidget::setTabOrder(speedSlider, ui->m_pPointerSetBtn);
    } else {
        QWidget::setTabOrder(m_pShortcutSwitchButton, ui->m_pPointerSetBtn);
    }
    QWidget::setTabOrder(ui->m_pPointerSetBtn, ui->m_pVocalSetBtn);
}

void Accessibility::initConnection() {
    connect(ui->m_pZoomComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setZoomMode(int)));
    connect(ui->m_pFilterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setFilterMode(int)));

    connect(ui->m_zoomSwitchWidget->switchButton(), &KSwitchButton::toggled, this, &Accessibility::setZoomSwitch);
    connect(m_pFilterSwitchButton, &KSwitchButton::toggled, this, &Accessibility::setFilterSwitch);
    connect(m_pShortcutSwitchButton, &KSwitchButton::toggled, this, &Accessibility::setShortcutSwitch);
    if (m_isOrcaInstalled) {
        connect(ui->m_screenReaderSwitchWidget, &SwitchWidget::stateChanged, this, &Accessibility::onScreenReaderStateChanged);
        connect(m_pScreenReaderSwitchButton, &KSwitchButton::toggled, this, &Accessibility::onScreenReaderSwitchToggled);
        connect(ui->m_screenReaderVolumeSliderWidget->slider(), &KSlider::valueChanged, this, &Accessibility::onScreenReaderVolumeChanged);
        connect(ui->m_screenReaderSpeedSliderWidget->slider(), &KSlider::valueChanged, this, &Accessibility::onScreenReaderSpeedChanged);
        if (m_pScreenReaderSetting) {
            connect(m_pScreenReaderSetting, &QGSettings::changed,
                    this, &Accessibility::onScreenReaderSettingChanged);
        }
    }

    connect(ui->m_pPointerSetBtn, &QPushButton::clicked, this, [=](){
        UkccCommon::buriedSettings(name(), "setBtn(Mouse)", QString("clicked"));
        QProcess::startDetached("ukui-control-center", QStringList() << "-m" << "Mouse");
    });
    connect(ui->m_pVocalSetBtn, &QPushButton::clicked, this, [=](){
        UkccCommon::buriedSettings(name(), "setBtn(Audio)", QString("clicked"));
        QProcess::startDetached("ukui-control-center", QStringList() << "-m" << "Audio");
    });

    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Accessibility",
                                          "org.ukui.ukcc.session.Accessibility",
                                          "statusChanged",
                                          this,
                                          SLOT(syncZoomSwitch(bool)));
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Accessibility",
                                          "org.ukui.ukcc.session.Accessibility",
                                          "modeChanged",
                                          this,
                                          SLOT(syncZoomComboBox(int)));
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Accessibility",
                                          "org.ukui.ukcc.session.Accessibility",
                                          "colorFilterTypeChanged",
                                          this,
                                          SLOT(syncFilterComboBox(QString)));
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Accessibility",
                                          "org.ukui.ukcc.session.Accessibility",
                                          "filterStatusChanged",
                                          this,
                                          SLOT(syncFilterSwitch(bool)));
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Accessibility",
                                          "org.ukui.ukcc.session.Accessibility",
                                          "colorFilterShortCutEnabledChanged",
                                          this,
                                          SLOT(syncShortcutSwitch(bool)));
}

void Accessibility::initQGSettingsAndDBus() {
    ui->m_zoomSwitchWidget->switchButton()->setChecked(false);
    QDBusReply<bool> isZoomingReply = m_accessibilityInterface->call("isZooming");
    qDebug() << Q_FUNC_INFO << "isZoomingReply" << isZoomingReply.isValid() << "isZooming" << isZoomingReply.value();
    if (isZoomingReply.isValid()) {
        ui->m_zoomSwitchWidget->switchButton()->setChecked(isZoomingReply.value());
        qDebug() << Q_FUNC_INFO << "ZoomSwitchButton" << isZoomingReply.value();
    }

    if (!m_isOrcaInstalled) {
        return;
    }

    bool screenReaderEnabled = false;
    if (QGSettings::isSchemaInstalled(QByteArray(GNOME_A11Y_APPLICATIONS_SCHEMA))) {
        m_pScreenReaderSetting = new QGSettings(QByteArray(GNOME_A11Y_APPLICATIONS_SCHEMA), QByteArray(), this);
        screenReaderEnabled = readScreenReaderEnabled(m_pScreenReaderSetting);
    }
    if (screenReaderEnabled && m_accessibilityInterface) {
        QDBusReply<bool> isScreenReaderRunningReply = m_accessibilityInterface->call("isScreenReaderRunning");
        qDebug() << Q_FUNC_INFO << "isScreenReaderRunningReply" << isScreenReaderRunningReply.isValid() << isScreenReaderRunningReply.value();
        if (isScreenReaderRunningReply.isValid() && !isScreenReaderRunningReply.value()) {
            // 会话初始化可能仍在 speechd 预热 / Orca 拉起中，保留开关并稍后重试校验。
            qDebug() << Q_FUNC_INFO << "screen reader enabled but orca not running yet, keep enabled and verify later";
            m_screenReaderEnableRequestedAt = QDateTime::currentMSecsSinceEpoch();
            m_orcaVerifyAttempt = 0;
            scheduleOrcaVerify();
        }
    }
    qDebug() << Q_FUNC_INFO << "screen reader switchbutton init status" << screenReaderEnabled;
    m_pScreenReaderSwitchButton->setChecked(screenReaderEnabled);

    if (QGSettings::isSchemaInstalled(QByteArray(UKCC_SCHEMA))) {
        m_pUkccSetting = new QGSettings(QByteArray(UKCC_SCHEMA), QByteArray(), this);
        const QStringList keys = m_pUkccSetting->keys();
        m_hasReaderVolumeKey = keys.contains(SCREEN_READER_VOLUME_GET_KEY);
        m_hasReaderSpeedKey = keys.contains(SCREEN_READER_SPEED_GET_KEY);
        qDebug() << Q_FUNC_INFO << "hasReaderVolumeKey" << m_hasReaderVolumeKey << "hasReaderSpeedKey" << m_hasReaderSpeedKey;
        int readerVolume = m_pUkccSetting->get(SCREEN_READER_VOLUME_KEY).toInt();
        qDebug() << Q_FUNC_INFO << "reader volume init value" << readerVolume;
        ui->m_screenReaderVolumeSliderWidget->slider()->setValue(readerVolume);
        ui->m_screenReaderVolumeSliderWidget->setRightText(QString("%1%").arg(readerVolume));

        const int storedSpeed = m_pUkccSetting->get(SCREEN_READER_SPEED_KEY).toInt();
        const int readerSpeed = qBound(SCREEN_READER_SPEED_MIN, storedSpeed, SCREEN_READER_SPEED_MAX);
        qDebug() << Q_FUNC_INFO << "reader speed init value" << readerSpeed;
        ui->m_screenReaderSpeedSliderWidget->slider()->setValue(readerSpeed);
        if (readerSpeed != storedSpeed) {
            m_pUkccSetting->set(SCREEN_READER_SPEED_KEY, readerSpeed);
        }

        ui->m_screenReaderVolumeSliderWidget->setEnabled(m_pScreenReaderSwitchButton->isChecked() && m_hasReaderVolumeKey);
        ui->m_screenReaderSpeedSliderWidget->setEnabled(m_pScreenReaderSwitchButton->isChecked() && m_hasReaderSpeedKey);
    }
}

void Accessibility::onScreenReaderStateChanged(bool checked)
{
    if (m_screenReaderSwitchSync) {
        m_screenReaderSwitchSync = false;
        return;
    }
    if (m_pScreenReaderSetting) {
        m_pScreenReaderSetting->set(SCREEN_READER_ENABLED_KEY, checked);
        qDebug() << Q_FUNC_INFO << "screen reader status" << checked;
    }
    if (!checked) {
        m_orcaVerifyAttempt = 0;
        m_screenReaderEnableRequestedAt = 0;
        return;
    }
    if (!m_accessibilityInterface) {
        return;
    }
    m_screenReaderEnableRequestedAt = QDateTime::currentMSecsSinceEpoch();
    m_orcaVerifyAttempt = 0;
    scheduleOrcaVerify();
}

void Accessibility::onScreenReaderSettingChanged(const QString &key)
{
    if (key != SCREEN_READER_ENABLED_KEY && key != SCREEN_READER_ENABLED_UKUI_KEY) {
        qDebug() << Q_FUNC_INFO << "onScreenReaderSettingChanged key return" << key;
        return;
    }

    if (!m_pScreenReaderSetting) {
        return;
    }
    syncScreenReaderSwitch(readScreenReaderEnabled(m_pScreenReaderSetting));
}

void Accessibility::syncScreenReaderSwitch(bool status)
{
    qDebug() << Q_FUNC_INFO << status;
    if (status == m_pScreenReaderSwitchButton->isChecked()) {
        return;
    }
    m_screenReaderSwitchSync = true;
    m_pScreenReaderSwitchButton->setChecked(status);
}

void Accessibility::onScreenReaderSwitchToggled(bool checked)
{
    qDebug() << Q_FUNC_INFO << "screen reader switchbutton status" << checked << m_hasReaderVolumeKey << m_hasReaderSpeedKey;
    ui->m_screenReaderVolumeSliderWidget->setEnabled(checked && m_hasReaderVolumeKey);
    ui->m_screenReaderSpeedSliderWidget->setEnabled(checked && m_hasReaderSpeedKey);
}

void Accessibility::onScreenReaderVolumeChanged(int value)
{
    ui->m_screenReaderVolumeSliderWidget->setRightText(QString("%1%").arg(value));
    m_pUkccSetting->set(SCREEN_READER_VOLUME_KEY, value);
    qDebug() << Q_FUNC_INFO << "volume set value" << value;
}

void Accessibility::onScreenReaderSpeedChanged(int value)
{
    m_pUkccSetting->set(SCREEN_READER_SPEED_KEY, value);
    qDebug() << Q_FUNC_INFO << "speed set value" << value;
}

void Accessibility::initDefaultData() {
    QDBusReply<int> zoomModeReply = m_accessibilityInterface->call("GetType");
    qDebug() << Q_FUNC_INFO << "getZoomMode" << zoomModeReply.isValid() << "isZooming" << zoomModeReply.value();
    if (zoomModeReply.isValid()) {
        int index = zoomModeReply.value();
        if (index >= 0 && index < ui->m_pZoomComboBox->count()) {
            if (index == 0) {
                ui->m_pZoomComboBox->setCurrentIndex(1);
            } else {
                ui->m_pZoomComboBox->setCurrentIndex(0);
            }
        }
        qDebug() << Q_FUNC_INFO << "ZoomComboBox set index" << index;
    }

    QDBusReply<bool> shortcutReply = m_accessibilityInterface->call("isColorFilterShortCutEnabled");
    qDebug() << Q_FUNC_INFO << "isColorFilterShortCutEnabled" << shortcutReply.value();
    if (shortcutReply.isValid()){
        m_pShortcutSwitchButton->setChecked(shortcutReply.value());
    }

    QDBusReply<bool> enableReply = m_accessibilityInterface->call("isColorFilterEnabled");
    qDebug() << Q_FUNC_INFO << "isColorFilterEnabled" << enableReply.value();
    if (enableReply.isValid()){
        m_pFilterSwitchButton->setChecked(enableReply.value());
        ui->m_pFilterComboBox->setEnabled(enableReply.value());
    } else {
        m_pFilterSwitchButton->setChecked(false);
        ui->m_pFilterComboBox->setEnabled(false);
    }

    QDBusReply<QString> modeReply = m_accessibilityInterface->call("getCurrentColorFilter");
    qDebug() << Q_FUNC_INFO << "getCurrentColorFilter" << modeReply.value();
    if (modeReply.isValid()){
        QString filterMode = modeReply.value();
        for (int i = 0; i < ui->m_pFilterComboBox->count(); i++) {
            if (ui->m_pFilterComboBox->itemData(i) != filterMode) {
                continue;
            } else {
                ui->m_pFilterComboBox->setCurrentIndex(i);
                break;
            }
        }
    }
}

void Accessibility::setZoomMode(int index) {
    qDebug() << Q_FUNC_INFO << "setZoomMode" << index;
    int dbusMode = 1;
    if (index == 1) {
        dbusMode = 0;
    }
    m_accessibilityInterface->asyncCall("setZoomMode", dbusMode);
}

void Accessibility::setFilterMode(int index) {
    qDebug() << Q_FUNC_INFO << "setColorFilter" << index << ui->m_pFilterComboBox->currentData();
    m_accessibilityInterface->asyncCall("setColorFilter", QVariant(ui->m_pFilterComboBox->currentData()));
}

void Accessibility::setZoomSwitch(bool toggled) {
    qDebug() << Q_FUNC_INFO << "setZoomSwitch" << toggled;
    m_accessibilityInterface->asyncCall("setZoomSwitch", toggled);
}

void Accessibility::setFilterSwitch(bool toggled) {
    qDebug() << Q_FUNC_INFO << "setFilterSwitch" << toggled;
    ui->m_pFilterComboBox->setEnabled(toggled);
    m_accessibilityInterface->asyncCall("setColorFilterSwitch", toggled);
}

void Accessibility::setShortcutSwitch(bool enabled) {
    qDebug() << Q_FUNC_INFO << "setColorFilterShortCutEnabled" << enabled;
    m_accessibilityInterface->asyncCall("setColorFilterShortCutEnabled", enabled);
}

void Accessibility::syncZoomSwitch(bool status) {
    qDebug() << Q_FUNC_INFO << status << __LINE__;
    if (status == ui->m_zoomSwitchWidget->switchButton()->isChecked()) {
        return;
    }
    m_zoomSwitchSync = true;
    ui->m_zoomSwitchWidget->switchButton()->setChecked(status);
}

void Accessibility::syncFilterSwitch(bool status) {
    qDebug() << Q_FUNC_INFO << __LINE__ << status;
    ui->m_pFilterComboBox->setEnabled(status);
    if (status == m_pFilterSwitchButton->isChecked()) {
        return;
    }
    m_pFilterSwitchButton->setChecked(status);
}

void Accessibility::syncShortcutSwitch(bool status) {
    qDebug() << Q_FUNC_INFO << __LINE__ << status;
    if (status == m_pShortcutSwitchButton->isChecked()) {
        return;
    }
    m_pShortcutSwitchButton->setChecked(status);
}

void Accessibility::syncZoomComboBox(int mode) {
    if (mode >= 0 && mode < ui->m_pZoomComboBox->count()) {
        const int targetIndex = (mode == 0) ? 1 : 0;
        if (ui->m_pZoomComboBox->currentIndex() == targetIndex) {
            return;
        }
        ui->m_pZoomComboBox->blockSignals(true);
        ui->m_pZoomComboBox->setCurrentIndex(targetIndex);
        ui->m_pZoomComboBox->blockSignals(false);
    }
}

void Accessibility::syncFilterComboBox(QString type) {
    for (int i = 0; i < ui->m_pFilterComboBox->count(); i++) {
        if (ui->m_pFilterComboBox->itemData(i) != type) {
            continue;
        } else {
            ui->m_pFilterComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void Accessibility::initSubPluginArea()
{
    loadSubPlugins();

    if (m_subPlugins.isEmpty()) {
        return;
    }

    int insertIndex = -1;
    for (int i = 0; i < ui->m_pVerticalLayout_3->count(); ++i) {
        QLayoutItem *item = ui->m_pVerticalLayout_3->itemAt(i);
        if (item && item->layout() == ui->m_pVerticalLayout) {
            insertIndex = i;
            break;
        }
    }

    if (insertIndex < 0) {
        insertIndex = ui->m_pVerticalLayout_3->count() - 1;
        if (insertIndex < 0) {
            insertIndex = 0;
        }
    }

    bool firstSubPlugin = true;
    for (SubPluginEntry &entry : m_subPlugins) {
        if (!entry.widget) {
            continue;
        }
        if (firstSubPlugin) {
            entry.topSpacing = new QWidget(m_pPluginWidget);
            entry.topSpacing->setFixedHeight(24);
            entry.topSpacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            ui->m_pVerticalLayout_3->insertWidget(insertIndex, entry.topSpacing);
            ++insertIndex;
            firstSubPlugin = false;
        }
        entry.widget->setParent(m_pPluginWidget);
        ui->m_pVerticalLayout_3->insertWidget(insertIndex, entry.widget);
        ++insertIndex;
        updateSubPluginVisible(entry);
    }
}

QGSettings *Accessibility::createSubPluginGsettings(const QString &subName, bool isEnable)
{
    if (!QGSettings::isSchemaInstalled(QByteArray(PLUGINS_SCHEMA))) {
        qWarning() << Q_FUNC_INFO << "schema not installed:" << PLUGINS_SCHEMA;
        return nullptr;
    }

    const QString path = QStringLiteral("%1%2/").arg(QString::fromUtf8(SUB_PLUGINS_GSETTINGS_PREFIX), subName);
    auto *settings = new QGSettings(QByteArray(PLUGINS_SCHEMA), path.toUtf8(), this);

    const QString existingName = settings->get(SUB_PLUGIN_GSETTINGS_NAME_KEY).toString();
    if (existingName.isEmpty()) {
        settings->set(SUB_PLUGIN_GSETTINGS_NAME_KEY, subName);
        settings->set(SUB_PLUGIN_GSETTINGS_SHOW_KEY, isEnable);
    } else {
        settings->set(SUB_PLUGIN_GSETTINGS_SHOW_KEY, isEnable);
    }

    qDebug() << Q_FUNC_INFO << subName << "show" << settings->get(SUB_PLUGIN_GSETTINGS_SHOW_KEY).toBool()
             << "isEnable" << isEnable;
    return settings;
}

void Accessibility::updateSubPluginVisible(SubPluginEntry &entry)
{
    if (!entry.widget || !entry.sub) {
        qDebug() << Q_FUNC_INFO << "skip invalid entry"
                 << "widget" << entry.widget << "sub" << entry.sub;
        return;
    }

    const bool isEnable = entry.sub->isEnable();
    bool show = true;
    if (entry.settings) {
        show = entry.settings->get(SUB_PLUGIN_GSETTINGS_SHOW_KEY).toBool();
    }
    const bool visible = isEnable && show;

    qDebug() << Q_FUNC_INFO << entry.sub->name()
             << "isEnable" << isEnable
             << "show" << show
             << "visible" << visible;

    if (entry.topSpacing) {
        entry.topSpacing->setVisible(visible);
    }
    entry.widget->setVisible(visible);
}

void Accessibility::onSubPluginSettingChanged(const QString &key, SubPluginEntry *entry)
{
    if (key != SUB_PLUGIN_GSETTINGS_SHOW_KEY || !entry || !entry->sub || !entry->settings) {
        qDebug() << Q_FUNC_INFO << "onSubPluginSettingChanged key" << key << "skip";
        return;
    }

    qDebug() << Q_FUNC_INFO << entry->sub->name()
             << "show" << entry->settings->get(SUB_PLUGIN_GSETTINGS_SHOW_KEY).toBool()
             << "isEnable" << entry->sub->isEnable();

    updateSubPluginVisible(*entry);
}

void Accessibility::connectSubPluginSettings(int index)
{
    if (index < 0 || index >= m_subPlugins.size() || !m_subPlugins[index].settings) {
        return;
    }

    QGSettings *settings = m_subPlugins[index].settings;
    connect(settings, &QGSettings::changed, this,
            [this, index](const QString &key) {
                onSubPluginGsettingsChanged(key, index);
            });
}

void Accessibility::onSubPluginGsettingsChanged(const QString &key, int index)
{
    if (index < 0 || index >= m_subPlugins.size()) {
        return;
    }
    onSubPluginSettingChanged(key, &m_subPlugins[index]);
}

void Accessibility::loadSubPlugins()
{
    const QString pluginDir = QDir(QStringLiteral(SUB_PLUGIN_INSTALL_DIRS))
                                  .filePath(name().toLower() + QStringLiteral("-plugin"));

    QDir dir(pluginDir);
    if (!dir.exists()) {
        qWarning() << Q_FUNC_INFO << "Sub-plugin directory not found:" << pluginDir;
        return;
    }

    const QFileInfoList plugins = dir.entryInfoList({QStringLiteral("*.so")}, QDir::Files);
    for (const QFileInfo &fi : plugins) {
        auto *loader = new QPluginLoader(fi.absoluteFilePath(), this);
        QObject *obj = loader->instance();
        if (!obj) {
            qWarning() << Q_FUNC_INFO << "Failed to load sub-plugin:"
                       << fi.absoluteFilePath() << loader->errorString();
            loader->deleteLater();
            continue;
        }

        auto *sub = qobject_cast<SubPluginInterface *>(obj);
        if (!sub) {
            qWarning() << Q_FUNC_INFO << "Invalid sub-plugin:" << fi.absoluteFilePath();
            loader->unload();
            loader->deleteLater();
            continue;
        }

        QWidget *widget = sub->pluginUi();
        if (!widget) {
            qWarning() << Q_FUNC_INFO << "Sub-plugin returned null widget:" << sub->name();
            loader->unload();
            loader->deleteLater();
            continue;
        }

        SubPluginEntry entry;
        entry.loader = loader;
        entry.widget = widget;
        entry.sub = sub;
        entry.settings = createSubPluginGsettings(sub->name(), sub->isEnable());

        m_subPlugins.append(entry);
        connectSubPluginSettings(m_subPlugins.size() - 1);

        qDebug() << Q_FUNC_INFO << "Loaded sub-plugin:" << sub->name()
                 << "isEnable" << sub->isEnable()
                 << "visible" << widget->isVisible()
                 << "from" << fi.absoluteFilePath();
        break; // 当前只需要加载一个，没有多个需求
    }
}
