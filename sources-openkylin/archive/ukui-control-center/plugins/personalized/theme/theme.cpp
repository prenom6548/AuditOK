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
#include "theme.h"
#include "ui_theme.h"
#include "radioproxystyle.h"
#include "myqradiobutton.h"

#include <QGSettings>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtConcurrent>
#include <QRadioButton>
#include <QButtonGroup>
#include <QScopedValueRollback>
#include "cursor/xcursortheme.h"
#include "../../../shell/customstyle.h"
#include "ukcccommon.h"
#include <kysdk/applications/accessinfohelper.h>
using namespace ukcc;

// GTK主题
#define THEME_GTK_SCHEMA "org.mate.interface"
#define MODE_GTK_KEY     "gtk-theme"
#define ICON_GTK_KEY     "icon-theme"

#define BG_MATE_SCHEMA   "org.mate.background"
#define BG_FILE_KEY      "picture-filename"

#define SOUND_UKUI_SCHEMA "org.ukui.sound"
#define SOUND_THEME_KEY   "theme-name"

// QT主题
#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"
#define THEME_TRAN_KEY   "menu-transparency"
#define PEONY_TRAN_KEY   "peony-side-bar-transparency"
#define ICON_QT_KEY      "icon-theme-name"
#define WIDGET_QT_KEY    "widget-theme-name"
#define COLOR_QT_KEY     "theme-color"
#define WINDOW_RAIDUS    "window-radius"

#define GLOBAL_SCHEMA    "org.ukui.globaltheme.settings"
#define GLOBAL_NAME_KEY  "global-theme-name"
#define ISMODIFY_KEY     "is-modified"

#define ICONTHEMEPATH       "/usr/share/icons/"
#define SYSTHEMEPATH        "/usr/share/themes/"
#define CURSORS_THEMES_PATH "/usr/share/icons/"

#define CURSOR_THEME_SCHEMA "org.ukui.peripherals-mouse"
#define CURSOR_THEME_KEY    "cursor-theme"

// 透明度设置
#define PERSONALSIE_SCHEMA        "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY      "transparency"
#define PERSONALSIE_BLURRY_KEY    "blurry"
#define PERSONALSIE_EFFECT_KEY    "effect"
#define PERSONALSIE_SAVE_TRAN_KEY "save-transparency"
#define PERSONALSIE_RADIUS_KEY    "radius"

// 自动主题设置
#define AUTO_SCHEMA               "org.ukui.SettingsDaemon.plugins.color"
#define COLOR_KEY_AUTO_THEME      "theme-schedule-automatic"

#define SYSTEM_THEME_CONF QString("/usr/share/config/globaltheme/globaltheme.conf")

const QString kDefCursor =           "DMZ-White";
const QString UbuntuVesionEnhance =  "22.04";
const QString kXder =                "XRender";

const static int CORLOR_COLUMN = 4;
const int transparency = 85;
//保存关闭特效模式之前的透明度
int save_trans = 0;

const QStringList cursorBlackList {"Adwaita", "DMZ-Black"};
const QStringList blackList       {"Adwaita", "bloom", "default", "hicolor", "HighContrast", "locolor", "LoginIcons", "ubuntu-mono-dark", "ubuntu-mono-light"};
const QStringList effectList      {"blur", "kwin4_effect_maximize", "zoom"};
const QStringList kIconsList      {"computer.png", "user-trash.png", "system-file-manager.png", "ukui-control-center.png", "kylin-software-center.png", "kylin-video.png", "kylin-assistant.png"};
const QStringList kIntelIconList  {"computer.png", "kylin-software-center.png", "ukui-control-center.png", "kylin-weather.png", "calendar-app.png", "kylin-camera.png"};

const QStringList defaultIconList {"kylin-startmenu.png", "kylin-software-center.png", "indicator-china-weather.png", "burner.png", "kylin-music.png", "kylin-photo-viewer.png"};
const QStringList heyinIconList   {"kylin-alarm-clock.png", "kylin-clipboard.png", "ukui-control-center.png", "kylin-calendar.png", "kylin-ipmsg.png", "kylin-photo-viewer.png"};
const QStringList classicIconList {"kylin-startmenu.png", "kylin-software-center.png", "kylin-alarm-clock.png", "indicator-china-weather.png", "yhkylin-backup-tools.png", "kylin-clipboard.png"};
namespace {

    // Preview cursors
    const char * const cursor_names[] =
    {
        "bd_double_arrow",
        "dnd-none",
        "left_ptr",
        "hand2",
        "watch",
        "crosshair",
    };

    const int numCursors = 6;     // The number of cursors from the above list to be previewed
}

Theme::Theme() : mFirstLoad(true)
{
    pluginName = tr("Theme");
    pluginType = PERSONALIZED;
}

Theme::~Theme()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString Theme::plugini18nName() {
    return pluginName;
}

int Theme::pluginTypes() {
    return pluginType;
}

QWidget *Theme::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Theme;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        m_themeInterface = new QDBusInterface("org.ukui.ukcc.session",
                                              "/Theme",
                                              "org.ukui.ukcc.session.Theme",
                                              QDBusConnection::sessionBus(), this);
        m_wlcomInterface = new QDBusInterface("com.kylin.Wlcom",
                                          "/com/kylin/Wlcom/Effect",
                                          "com.kylin.Wlcom.Effect",
                                          QDBusConnection::sessionBus(), this);
        m_wlcomViewInterface = new QDBusInterface("com.kylin.Wlcom",
                                          "/com/kylin/Wlcom/View",
                                          "com.kylin.Wlcom.View",
                                          QDBusConnection::sessionBus(), this);
        // 先 GSettings，再按页面视觉自上而下创建，保证默认 Tab 顺序正确
        setupGSettings();
        isUpgrade();

        initGlobalTheme();
        initThemeUi();
        mIsOpenkylin = UkccCommon::isOpenkylin();
        mDarkBtn->setVisible(!UkccCommon::isCommunity());
        mDefaultBtn->setProperty("value", "ukui-default");
        mLightBtn->setProperty("value", "ukui-light");
        mDarkBtn->setProperty("value", "ukui-dark");
        mAutoBtn->setProperty("value", "ukui-auto");
        initControlTheme();
        initIconTheme();
        initCursorTheme();
        initEffectSettingUi();
#if SP2
        initJumpTheme();
#endif
        setupSettings();
        initThemeMode();
        initEffectMode();
        initConnection();
        initSearchText();
        securityControl();
    }
    return pluginWidget;
}

const QString Theme::name() const {

    return QStringLiteral("Theme");
}

bool Theme::isShowOnHomePage() const
{
    return true;
}

QIcon Theme::icon() const
{
    return QIcon::fromTheme("ukui-note-color-symbolic");
}

bool Theme::isEnable() const
{
    return true;
}

void Theme::initCorner() {
    QString systemPath = SYSTEM_THEME_CONF;
    if (!QFile::exists(systemPath)) {
        qDebug() << Q_FUNC_INFO << "initCorner failed, path not exist" << systemPath;
        return;
    }
    QSettings* settings = new QSettings(systemPath, QSettings::NativeFormat, this);
    settings->setIniCodec(QTextCodec::codecForName("utf-8"));

    settings->beginGroup("window-radius");
    for (const QString &key : settings->childKeys()) {
        qDebug() << Q_FUNC_INFO << "initCorlor key" << key << settings->value(key);
        QString value = settings->value(key).toString();
        QString trans = getThemeTrans(key);
        if (trans.isEmpty()) {
            qDebug() << Q_FUNC_INFO << "get corner trans failed, skip" << key;
            continue;
        }
        mCornerWidget->comboBox()->addItem(trans, value);
        qDebug() << Q_FUNC_INFO << "add Corner suc" << key << value << trans;
    }
    settings->endGroup();
}

void Theme::initWindowMinimize() {
    m_windowMinimizeWidget->comboBox()->addItem(tr("Scale"), 0);
    m_windowMinimizeWidget->comboBox()->addItem(tr("MagicLamp"), 1);

    if (!m_wlcomViewInterface) {
        qDebug() << Q_FUNC_INFO << "com.kylin.Wlcom is null";
        return;
    }
    QDBusReply<uint32_t> isWlcomReply = m_wlcomViewInterface->call("GetMinimizeEffect");
    if (isWlcomReply.isValid()) {
        uint32_t index = isWlcomReply.value();
        qDebug() << Q_FUNC_INFO << "GetMinimizeEffect" << index;
        if (index >= 0 && index <= 1) {
            m_windowMinimizeWidget->comboBox()->setCurrentIndex(index);
        }
    } else {
        qDebug() << Q_FUNC_INFO << "GetMinimizeEffect return invalid";
    }
}

void Theme::initThemeUi()
{
    mThemeModeFrame = new QFrame(pluginWidget);
    mThemeModeFrame->setMinimumHeight(156);
    mThemeModeFrame->setFrameShape(QFrame::Shape::Box);

    FlowLayout *hLyt = new FlowLayout(mThemeModeFrame, 20, 16, 16);
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        hLyt->setRightToLeft(true);
    }
    mThemeModeLabel = new KLabel(mThemeModeFrame);
    mThemeModeLabel->setContentsMargins(16, 0, 0, 0);
    // 勿 KDK_EXTEND：会冲掉 setBuddy 的 LABELLED_BY（光标标题无 KDK 故绑定正常）

    mLightBtn = new ThemeButton(tr("Light"), QPixmap(QString("://img/plugins/theme/%1.png").arg("light")), mThemeModeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mLightBtn, name(), "", "");
    mLightBtn->setObjectName("lighttheme");
    mDarkBtn = new ThemeButton(tr("Dark"),QPixmap(QString("://img/plugins/theme/%1.png").arg("dark")), mThemeModeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mDarkBtn, name(), "", "");
    mDarkBtn->setObjectName("darktheme");
    mDefaultBtn = new ThemeButton(tr("Default"), QPixmap(QString("://img/plugins/theme/%1.png").arg("default")),mThemeModeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mDefaultBtn, name(), "", "");
    mDefaultBtn->setObjectName("defaulttheme");
    mAutoBtn = new ThemeButton(tr("Auto"), QPixmap(QString("://img/plugins/theme/%1.png").arg("auto")),mThemeModeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mAutoBtn, name(), "", "");
    mAutoBtn->setObjectName("autotheme");

    mThemeModeBtnGroup = new QButtonGroup(this);

    mThemeModeBtnGroup->addButton(mLightBtn);
    mThemeModeBtnGroup->addButton(mDarkBtn);
    mThemeModeBtnGroup->addButton(mDefaultBtn);
    mThemeModeBtnGroup->addButton(mAutoBtn);

    buildThemeModeBtn(mLightBtn);
    buildThemeModeBtn(mDarkBtn);
    buildThemeModeBtn(mDefaultBtn);
    buildThemeModeBtn(mAutoBtn);

    hLyt->addWidget(mLightBtn);
    hLyt->addWidget(mDarkBtn);
    hLyt->addWidget(mAutoBtn);

    ui->thememodeLayout->addWidget(mThemeModeLabel);
    ui->thememodeLayout->addWidget(mThemeModeFrame);
}

void Theme::initEffectSettingUi()
{
    mEffectLabel = new KLabel(pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(mEffectLabel, name(), "", "");
    mEffectLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Theme/Effect Setting
    mEffectLabel->setText(tr("Effect Setting"));

    mEffectGroup = new SettingGroup(pluginWidget);

    //~ contents_path /Theme/Performance mode
    mEffectWidget = new SwitchWidget(tr("Performance mode"), mEffectGroup);
    KDK_EXTEND_ALL_INFO_FORMAT(mEffectWidget->switchButton(), name(), "", "");
    //~ contents_path /Theme/Transparency
    mTranWidget   = new SliderWidget(tr("Transparency"), false, mEffectGroup);
    KDK_EXTEND_ALL_INFO_FORMAT(mTranWidget->slider(), name(), "", "");
    //~ contents_path /Theme/Corner
    mCornerWidget = new ComboxWidget(tr("Corner"), mEffectGroup);
    KDK_EXTEND_ALL_INFO_FORMAT(mCornerWidget->comboBox(), name(), "", "");

    //~ contents_path /Theme/Window Minimize
    m_windowMinimizeWidget = new ComboxWidget(tr("Window Minimize"), mEffectGroup);
    KDK_EXTEND_ALL_INFO_FORMAT(m_windowMinimizeWidget->comboBox(), name(), "", "");
    mTranWidget->leftLabel()->setFixedWidth(24);
    mTranWidget->rightLabel()->setFixedWidth(24);
    mTranWidget->setLeftPixmap(QPixmap("://img/plugins/theme/opacity-currency.svg"));
    mTranWidget->setRightPixmap(QPixmap("://img/plugins/theme/opaque-light.svg"));
    mTranWidget->slider()->setRange(35, 100);
    KSlider* transSlider = mTranWidget->slider();
    KDK_EXTEND_ALL_INFO_FORMAT(transSlider, name(), "", "");

    initCorner();
    initWindowMinimize();

    mEffectGroup->addWidget(mEffectWidget);
    mEffectGroup->addWidget(mTranWidget);
    mEffectGroup->addWidget(mCornerWidget);
    mEffectGroup->addWidget(m_windowMinimizeWidget);

    ui->effectLyt->addWidget(mEffectLabel);
    ui->effectLyt->addWidget(mEffectGroup);
    ui->effectLyt->addStretch();

#ifndef Nile
    mCornerWidget->setVisible(false);
#endif
}

void Theme::initSearchText() {
    //~ contents_path /Theme/Window Theme
    mThemeModeLabel->setText(tr("Window Theme"));
    //~ contents_path /Theme/Icon Theme
    mThemeIconLabel->setText(tr("Icon Theme"));
    if (mThemeCursorLabel != nullptr) {
        //~ contents_path /Theme/Cursor Theme
        mThemeCursorLabel->setText(tr("Cursor theme"));
    }

    mEffectWidget->setAccessibleName(QString());
    mEffectWidget->switchButton()->setAccessibleName(tr("Performance mode"));
    mTranWidget->slider()->setAccessibleName(tr("Transparency"));

    // 文案就绪后再 setBuddy（只绑当前选中项，Tab 进组会落在该项）
    // ThemeButton 在 QFrame 内时 AT-SPI 常无 LABELLED_BY，故把行标题写入 accessibleName
    const QString windowSection = tr("Window Theme");
    mThemeModeLabel->setAccessibleName(windowSection);
    mLightBtn->setAccessibleOptionName(windowSection);
    mDarkBtn->setAccessibleOptionName(windowSection);
    mDefaultBtn->setAccessibleOptionName(windowSection);
    mAutoBtn->setAccessibleOptionName(windowSection);
    if (mThemeModeBtnGroup) {
        QAbstractButton *modeBuddy = mThemeModeBtnGroup->checkedButton();
        mThemeModeLabel->setBuddy(modeBuddy ? modeBuddy : mLightBtn);
        mLightBtn->refreshGroupKeyboardNav();
    }
    if (mThemeIconLabel && mThemeIconBtnGroup) {
        const QList<QAbstractButton *> iconBtns = mThemeIconBtnGroup->buttons();
        if (!iconBtns.isEmpty()) {
            const QString iconSection = tr("Icon Theme");
            mThemeIconLabel->setAccessibleName(iconSection);
            for (QAbstractButton *btn : iconBtns) {
                if (auto *tb = qobject_cast<ThemeButton *>(btn)) {
                    tb->setAccessibleOptionName(iconSection);
                }
            }
            QAbstractButton *iconBuddy = mThemeIconBtnGroup->checkedButton();
            mThemeIconLabel->setBuddy(iconBuddy ? iconBuddy : iconBtns.first());
            if (auto *tb = qobject_cast<ThemeButton *>(iconBtns.first())) {
                tb->refreshGroupKeyboardNav();
            }
        }
    }
    if (mThemeCursorLabel && mThemeCursorBtnGroup) {
        const QList<QAbstractButton *> cursorBtns = mThemeCursorBtnGroup->buttons();
        if (!cursorBtns.isEmpty()) {
            const QString cursorSection = tr("Cursor theme");
            mThemeCursorLabel->setAccessibleName(cursorSection);
            for (QAbstractButton *btn : cursorBtns) {
                if (auto *tb = qobject_cast<ThemeButton *>(btn)) {
                    tb->setAccessibleOptionName(cursorSection);
                }
            }
            QAbstractButton *cursorBuddy = mThemeCursorBtnGroup->checkedButton();
            mThemeCursorLabel->setBuddy(cursorBuddy ? cursorBuddy : cursorBtns.first());
            if (auto *tb = qobject_cast<ThemeButton *>(cursorBtns.first())) {
                tb->refreshGroupKeyboardNav();
            }
        }
    }
    if (mControlLabel && !mRadioBtnList.isEmpty()) {
        QRadioButton *accentBuddy = nullptr;
        for (QRadioButton *btn : mRadioBtnList) {
            if (btn->isChecked()) {
                accentBuddy = btn;
                break;
            }
        }
        mControlLabel->setBuddy(accentBuddy ? accentBuddy : mRadioBtnList.first());
        if (auto *rb = qobject_cast<MyQRadioButton *>(mRadioBtnList.first())) {
            rb->refreshGroupKeyboardNav();
        }
    }
    if (mEffectLabel && mEffectWidget && mEffectWidget->switchButton()) {
        mEffectLabel->setBuddy(mEffectWidget->switchButton());
    }
    if (mSwitchLabel && mThemeBtnGroup) {
        const QList<QAbstractButton *> themeBtns = mThemeBtnGroup->buttons();
        if (!themeBtns.isEmpty()) {
            // 与窗口外观相同：无 LABELLED_BY 时把「主题」写入 accessibleName
            const QString themeSection = tr("Theme");
            mSwitchLabel->setAccessibleName(themeSection);
            for (QAbstractButton *btn : themeBtns) {
                if (auto *tb = qobject_cast<ThemeButton *>(btn)) {
                    tb->setAccessibleOptionName(themeSection);
                }
            }
            QAbstractButton *themeBuddy = mThemeBtnGroup->checkedButton();
            mSwitchLabel->setBuddy(themeBuddy ? themeBuddy : themeBtns.first());
            if (auto *tb = qobject_cast<ThemeButton *>(themeBtns.first())) {
                tb->refreshGroupKeyboardNav();
            }
        }
    }
    if (mJumpLabel && mWallWidget && mWallWidget->pushButton()) {
        mJumpLabel->setBuddy(mWallWidget->pushButton());
    }
}

void Theme::setupSettings() {
    if (UkccCommon::isOpenkylin()) {
       m_kwinFile = QDir::homePath() + "/.config/kwinrc";
    } else {
       m_kwinFile = QDir::homePath() + "/.config/ukui-kwinrc";
    }

    kwinSettings = new QSettings(m_kwinFile, QSettings::IniFormat, this);

    QStringList keys = kwinSettings->childGroups();
    kwinSettings->beginGroup("Plugins");
    bool kwin = kwinSettings->value("blurEnabled", kwin).toBool();

    if (!kwinSettings->childKeys().contains("blurEnabled") || isBlurEffect()) {
        kwin = true;
    } else {
        kwin = false;
    }

    kwinSettings->endGroup();

    bool effectEnabled = personliseGsettings->get(PERSONALSIE_EFFECT_KEY).toBool();
    mEffectWidget->setChecked(effectEnabled);
    mCornerWidget->setVisible(effectEnabled);
    m_windowMinimizeWidget->setVisible(effectEnabled);

    if (mEffectWidget->isChecked()) {
        mTranWidget->setVisible(true);
    } else {
        mTranWidget->setVisible(false);
    }

    if (keys.contains("Compositing")) {
        kwinSettings->beginGroup("Compositing");
        QString xder;
        bool kwinOG = false;
        bool kwinEN = true;
        xder = kwinSettings->value("Backend", xder).toString();
        kwinOG = kwinSettings->value("OpenGLIsUnsafe", kwinOG).toBool();
        kwinEN = kwinSettings->value("Enabled", kwinEN).toBool();
        if (xder == kXder || kwinOG || !kwinEN) {
            mEffectGroup->setVisible(false);
            mEffectLabel->setVisible(false);
            setEffectStatus(false);
        } else {
        }
        kwinSettings->endGroup();
    }

    if (m_themeInterface) {
        QDBusReply<bool> isRenderingReply = m_themeInterface->call("isHardwareRendering");
        qDebug() << Q_FUNC_INFO << "isHardwareRendering" << isRenderingReply.isValid() << isRenderingReply.value();
        if (isRenderingReply.isValid()) {
            bool isRendering = isRenderingReply.value();
            if (!isRendering) {
                mEffectGroup->setVisible(false);
                mEffectLabel->setVisible(false);
                qDebug() << Q_FUNC_INFO << "isHardwareRendering is false, set effect invisible";
            }
        }
    }
}

void Theme::setupComponent() {

    mIsOpenkylin = UkccCommon::isOpenkylin();
    mDarkBtn->setVisible(!UkccCommon::isCommunity());

    mDefaultBtn->setProperty("value", "ukui-default");
    mLightBtn->setProperty("value", "ukui-light");
    mDarkBtn->setProperty("value", "ukui-dark");
    mAutoBtn->setProperty("value", "ukui-auto");

    initGlobalTheme();
    initControlTheme();
}

void Theme::buildThemeModeBtn(ThemeButton *button)
{
    connect(mThemeModeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        emit button->btnCheckedSignals(eBtn == button ? true : false);
    });
}

void Theme::initThemeMode() {
    // 获取当前主题
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    bool isAutoTheme = getAutoTheme();

    if (isAutoTheme) {
        mThemeModeBtnGroup->buttonClicked(mAutoBtn);
        emit mAutoBtn->clicked();
    } else if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
        mThemeBtnGroup->buttonClicked(mDefaultBtn);
        emit mDefaultBtn->clicked();
    } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode) {
        mThemeModeBtnGroup->buttonClicked(mDarkBtn);
        emit mDarkBtn->clicked();
    } else {
        mThemeModeBtnGroup->buttonClicked(mLightBtn);
        emit mLightBtn->clicked();
    }
}

void Theme::initIconTheme() {
    mThemeIconFrame = new QFrame(pluginWidget);
    mThemeIconFrame->setMinimumHeight(156);
    mThemeIconFrame->setFrameShape(QFrame::Shape::Box);

    FlowLayout *fLyt = new FlowLayout(mThemeIconFrame, 0, 16, 16);
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        fLyt->setRightToLeft(true);
    }
    fLyt->setContentsMargins(16, 16, 0, 4);
    mThemeIconLabel = new KLabel(mThemeIconFrame);
    mThemeIconLabel->setContentsMargins(16, 0, 0, 0);
    // 勿 KDK_EXTEND：与光标标题一致，保证 setBuddy 建立 LABELLED_BY

    // 构建图标主题Widget Group，方便更新选中/非选中状态
    mThemeIconBtnGroup = new QButtonGroup;

    //构建图标主题QDir
    QStringList iconThemes = getSystemIconThemes();

    foreach (QString themedir, iconThemes) {
        if ("ukui-icon-theme-dawnlight" == themedir) {
            initIconThemeWidget(themedir, fLyt);
            break;
        }
    }

    foreach (QString themedir, iconThemes) {
        if (("ukui-icon-theme-default" == themedir) || ("ukui-hp" == themedir)) {
            initIconThemeWidget(themedir, fLyt);
            break;
        }
    }
    foreach (QString themedir, iconThemes) {
        if (("ukui-icon-theme-fashion" == themedir)) {
            initIconThemeWidget(themedir, fLyt);
            break;
        }
    }
    foreach (QString themedir, iconThemes) {
        if (("ukui-icon-theme-classical" == themedir)) {
            initIconThemeWidget(themedir, fLyt);
            break;
        }
    }
    foreach (QString themedir, iconThemes) {
        if ((UkccCommon::isCommunity() && (!themedir.compare("ukui") || !themedir.compare("ukui-classical") || !themedir.compare("ukui-fashion"))) ||
            (!UkccCommon::isCommunity()) ||
            (UkccCommon::isTablet() && (!themedir.compare("ukui") || themedir.startsWith("ukui-classical")))) {

            if (("ukui-icon-theme-basic" == themedir)  || ("ukui-icon-theme-default" == themedir) || ("ukui-hp" == themedir)
                ||("ukui-icon-theme-fashion" == themedir) || ("ukui-icon-theme-classical" == themedir) ||("ukui-icon-theme-dawnlight" == themedir)) {
                  continue;
            }
            if (blackList.contains(themedir))
                continue;
            initIconThemeWidget(themedir, fLyt);
        }
    }

    connect(mThemeIconBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(iconThemeBtnClickSlot(QAbstractButton*)));

    ui->iconverLyt->addWidget(mThemeIconLabel);
    ui->iconverLyt->addWidget(mThemeIconFrame);
    // 分区标题关联首个图标主题
    const QList<QAbstractButton *> iconBtns = mThemeIconBtnGroup->buttons();
    if (!iconBtns.isEmpty()) {
        mThemeIconLabel->setBuddy(iconBtns.first());
    }
}

void Theme::initCorlor() {
    QString systemPath = SYSTEM_THEME_CONF;
    if (!QFile::exists(systemPath)) {
        qDebug() << Q_FUNC_INFO << "initCorner failed, path not exist" << systemPath;
        return;
    }
    QSettings* settings = new QSettings(systemPath, QSettings::NativeFormat, this);
    settings->setIniCodec(QTextCodec::codecForName("utf-8"));

    settings->beginGroup("theme-color");
    for (const QString &key : settings->childKeys()) {
        qDebug() << Q_FUNC_INFO << "initCorlor key" << key << settings->value(key);
        QString value = settings->value(key).toString();
        m_controlMap.insert(key, value);
    }
    settings->endGroup();
}

QString Theme::getThemeTrans(QString key) {
    QString value;
    QString systemPath = SYSTEM_THEME_CONF;
    if (!QFile::exists(systemPath)) {
        qDebug() << Q_FUNC_INFO << "initCorner failed, path not exist" << systemPath;
        return value;
    }
    QSettings* settings = new QSettings(systemPath, QSettings::NativeFormat, this);
    settings->setIniCodec(QTextCodec::codecForName("utf-8"));

    settings->beginGroup("translation");
    QVariant var = settings->value(QString("%1[%2]").arg(key).arg(QLocale::system().name()));
    if (var.isValid()) {
        value = var.toString();
    }
    settings->endGroup();
    qDebug() << Q_FUNC_INFO << "corlor name" << key << "to trans" << value;
    return value;
}

void Theme::initControlTheme()
{
    if (!qtSettings->keys().contains("themeColor")) {
        return;
    }

    QString colorName = qtSettings->get(COLOR_QT_KEY).toString();
    initCorlor();

    mControlLabel = new KLabel();
    mControlLabel->setContentsMargins(16, 0, 0, 0);
    mControlLabel->setMaximumWidth(100);
    //~ contents_path /Theme/Corlor
    mControlLabel->setText(tr("Corlor"));

    mControlHLyt = new QHBoxLayout();
    mControlHLyt->addWidget(mControlLabel);
    mControlHLyt->addSpacing(32);

    mControlFrame = new QFrame(pluginWidget);
    mControlFrame->setMinimumHeight(60);
    mControlFrame->setFrameShape(QFrame::Shape::Box);

    // 强调色互斥：autoExclusive 关闭以便方向键只移焦，由 ButtonGroup 在点击/空格时互斥
    QButtonGroup *accentGroup = new QButtonGroup(mControlFrame);
    accentGroup->setExclusive(true);

    QMap<QString, QString>::const_iterator it = m_controlMap.constBegin();
    for (; it != m_controlMap.constEnd(); ++it) {
        QString value = it.value();
        QStringList colorList = value.split(",");
        if (colorList.size() != CORLOR_COLUMN) {
            qDebug() << Q_FUNC_INFO << "colorList not equal 4, skip" << value;
            continue;
        }
        QString trans = getThemeTrans(it.key());
        if (trans.isEmpty()) {
            qDebug() << Q_FUNC_INFO << "get corlor key" << it.key() << "trans failed";
            continue;
        }
        MyQRadioButton *radioBtn = new MyQRadioButton(mControlFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(radioBtn, "Theme", "", "corlor button of" + trans);
        radioBtn->setAccessibleName(trans);
        mRadioBtnList.append(radioBtn);
        accentGroup->addButton(radioBtn);

        radioBtn->setToolTip(trans);
        radioBtn->setProperty("key", value);
        int red = colorList.at(0).toInt();
        int green = colorList.at(1).toInt();
        int blue = colorList.at(2).toInt();
        float alpha = colorList.at(3).toFloat();
        RadioProxystyle *style = new RadioProxystyle(QColor(red, green, blue, alpha * 255));
        radioBtn->setStyle(style);
        style->setParent(mControlFrame);

        if (!getRealColorName(colorName).compare(value)) {
            radioBtn->setChecked(true);
            qDebug() << Q_FUNC_INFO << "color set checked" << value << colorName;
        }
        qDebug() << Q_FUNC_INFO << "create corlor radioBtn info" << it.key() << value << trans << colorName;
        mControlHLyt->addWidget(radioBtn);

        connect(radioBtn, &QRadioButton::clicked, this, [=] {
            radioBtnClickSlot(radioBtn->property("key").toString());
            mControlLabel->setBuddy(radioBtn);
        });
    }

    mControlHLyt->addStretch();
    mControlFrame->setLayout(mControlHLyt);
    ui->controlLyt->addWidget(mControlFrame);
    // 分区标题只绑当前选中项；Tab 进组落在选中强调色
    QRadioButton *accentBuddy = nullptr;
    for (QRadioButton *btn : mRadioBtnList) {
        if (btn->isChecked()) {
            accentBuddy = btn;
            break;
        }
    }
    if (!mRadioBtnList.isEmpty()) {
        mControlLabel->setBuddy(accentBuddy ? accentBuddy : mRadioBtnList.first());
        if (auto *rb = qobject_cast<MyQRadioButton *>(mRadioBtnList.first())) {
            rb->refreshGroupKeyboardNav();
        }
    }
}

void Theme::radioBtnClickSlot(const QString& value) {
    qtSettings->set(COLOR_QT_KEY, value);
    revokeGlobalThemeSlot("getQtAccentRole", value);
    if (UkccCommon::isTablet()) {
        //大屏版本控制面板自己设置强调色需要延迟才能防卡死，bug#183191
        QTimer::singleShot(800, this,[=](){
            qApp->setStyle(new InternalStyle("ukui"));
        });
    }
    UkccCommon::buriedSettings(name(), "set accent color", QString("settings"), value);
}

void Theme::initCursorTheme(){
    if (UkccCommon::isTablet())
        return;

    mThemeCursorFrame = new QFrame(pluginWidget);
    mThemeCursorFrame->setMinimumHeight(156);
    mThemeCursorFrame->setFrameShape(QFrame::Shape::Box);

    FlowLayout *fLyt = new FlowLayout(mThemeCursorFrame, 0, 16, 16);
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        fLyt->setRightToLeft(true);
    }
    fLyt->setContentsMargins(16, 16, 0, 4);

    mThemeCursorLabel = new KLabel(mThemeCursorFrame);
    mThemeCursorLabel->setContentsMargins(16, 0, 0, 0);

    QStringList cursorThemes = getSystemCursorThemes();

    mThemeCursorBtnGroup = new QButtonGroup(this);

    for (QString cursor : cursorThemes) {
        if (cursor == "dark-sense") {
            initCursorThemeWidget(cursor , fLyt);
            break;
        }
    }
    for (QString cursor : cursorThemes) {
        if (cursor == "DMZ-White") {
            initCursorThemeWidget(cursor , fLyt);
            break;
        }
    }
    for (QString cursor : cursorThemes) {
        if (cursor == "dark-sense" || cursor == "DMZ-White" || (cursorBlackList.contains(cursor))) {
            continue;
        }
        initCursorThemeWidget(cursor, fLyt);
    }

    connect(mThemeCursorBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(cursorThemeBtnClickSlot(QAbstractButton*)));

    ui->cursorVerLayout->addWidget(mThemeCursorLabel);
    ui->cursorVerLayout->addWidget(mThemeCursorFrame);
    // 分区标题关联首个光标主题
    const QList<QAbstractButton *> cursorBtns = mThemeCursorBtnGroup->buttons();
    if (!cursorBtns.isEmpty()) {
        mThemeCursorLabel->setBuddy(cursorBtns.first());
    }
}

void Theme::initEffectMode()
{
    if (qtSettings->keys().contains("windowRadius")) {
        QString radius = qtSettings->get(WINDOW_RAIDUS).toString();
        qDebug() << Q_FUNC_INFO << "init mCornerWidget combox current" << radius;
        mCornerWidget->setCurrentIndex(mCornerWidget->comboBox()->findData(radius));
    }
    qDebug() << Q_FUNC_INFO << "init trans" << personliseGsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 100.0;
    mTranWidget->slider()->setValue(static_cast<int>(personliseGsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 100.0));
}

void Theme::initGlobalTheme()
{
    if (mGlobalSettings == nullptr) {
        return;
    }
    const QString globalName = mGlobalSettings->get("global-theme-name").toString();

    mSwitchFrame = new QFrame(pluginWidget);
    mSwitchFrame->setMinimumHeight(196);
    mSwitchFrame->setFrameShape(QFrame::Shape::Box);

    mFLyt = new FlowLayout(mSwitchFrame, 0, 16, 16);
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        mFLyt->setRightToLeft(true);
    }
    mFLyt->setContentsMargins(16, 16, 0, 4);
    mSwitchLabel = new KLabel();
    mSwitchLabel->setContentsMargins(16, 0, 0, 0);
    mSwitchLabel->setText(tr("Theme"));
    // 勿 KDK_EXTEND：会冲掉 setBuddy 的 LABELLED_BY

    ui->switchLyt->addWidget(mSwitchLabel);
    mGlobalthemehelper = GlobalThemeHelper::getInstance();

    mThemeBtnGroup = new QButtonGroup(this);

    QList<GlobalTheme *> globalThemes = mGlobalthemehelper->getAllGlobalThemes();
    QList<GlobalTheme *> sorthemes = sortGlobalTheme(globalThemes);

    int index = 0;
    ThemeButton *firstGlobalBtn = nullptr;
    for (GlobalTheme *theme: sorthemes) {

        mGlobalthemehelper->loadThumbnail(theme);

        QString themeName = theme->getThemeName();
        QString i18nName  = theme->getLocaleThemeName();

        ThemeButton *btn = new ThemeButton(i18nName, mSwitchFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(btn, name(), "globalThemeButton" + QString::number(index), "");
        btn->setAccessibleOptionName(tr("Theme"));
        btn->setObjectName(themeName);
        btn->setProperty("value", themeName);
        mThemeBtnGroup->addButton(btn);
        if (!firstGlobalBtn) {
            firstGlobalBtn = btn;
        }

        mGlobalMap.insert(i18nName, btn);

        if (globalName == themeName) {
            mPrePicUnit = btn;
            btn->setBtnClicked(true);
            mThemeBtnGroup->buttonClicked(btn);
        }

        if (!themeName.compare("custom")) {
            mCustomPicUnit = btn;
        }

        mFLyt->addWidget(btn);

        connect(theme, &GlobalTheme::thumbnailLoaded, this, [=]{
            btn->setPixmap(theme->getThumbnail());
            btn->update();
        });
        ++index;
    }
    if (firstGlobalBtn) {
        // 分区标题只绑当前选中（无选中则首项）；Tab 进组落在选中项
        QAbstractButton *globalBuddy = mThemeBtnGroup->checkedButton();
        mSwitchLabel->setBuddy(globalBuddy ? globalBuddy : firstGlobalBtn);
        firstGlobalBtn->refreshGroupKeyboardNav();
    }
    ui->switchLyt->addWidget(mSwitchFrame);
    connect(mThemeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(changeGlobalThemeSlot(QAbstractButton*)));
    connect(mGlobalthemehelper, &GlobalThemeHelper::globalThemesChanged, this, &Theme::changeGlobalDirSlot);
}

QList<GlobalTheme *> Theme::sortGlobalTheme(const QList<GlobalTheme *> &globalThemes)
{
    QList<GlobalTheme *> tmpGlogbalThemes;
    GlobalTheme *heyin        = nullptr;
    GlobalTheme *lightSeeking = nullptr;
    GlobalTheme *dawnlight    = nullptr;

    for (GlobalTheme *theme: globalThemes) {
        if (!theme->getThemeName().compare("DawnLight", Qt::CaseInsensitive)) {
            dawnlight = theme;
        } else if (!theme->getThemeName().compare("Light-Seeking", Qt::CaseInsensitive)) {
            lightSeeking = theme;
        } else if (!theme->getThemeName().compare("HeYin", Qt::CaseInsensitive)){
            heyin = theme;
        } else {
            tmpGlogbalThemes.append(theme);
        }
    }

    if (heyin != nullptr) tmpGlogbalThemes.prepend(heyin);
    if (lightSeeking != nullptr) tmpGlogbalThemes.prepend(lightSeeking);
    if (dawnlight != nullptr) tmpGlogbalThemes.prepend(dawnlight);


    return tmpGlogbalThemes;
}

void Theme::initJumpTheme()
{
    mJumpLabel  = new KLabel(pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(mJumpLabel, "Theme", "", "");
    mJumpLabel->setContentsMargins(16, 0, 0, 0);

    //~ contents_path /Theme/Other
    mJumpLabel->setText(tr("Other"));
    mRelatedGroup = new SettingGroup(pluginWidget);
    //~ contents_path /Theme/Wallpaper
    mWallWidget = new PushButtonWidget(tr("Wallpaper"), mRelatedGroup);
    KDK_EXTEND_ALL_INFO_FORMAT(mWallWidget, "Theme", "", "");
    mWallWidget->setObjectName("Wallpaper");
    mWallWidget->setButtonText(tr("Set"));
    //~ contents_path /Theme/Beep
    mBeepWidget = new PushButtonWidget(tr("Beep"), mRelatedGroup);
    KDK_EXTEND_ALL_INFO_FORMAT(mBeepWidget, "Theme", "", "");
    mBeepWidget->setObjectName("Beep");
    mBeepWidget->setButtonText(tr("Set"));
    mRelatedGroup->addWidget(mWallWidget);
    mRelatedGroup->addWidget(mBeepWidget);

    ui->jumpLyt->addWidget(mJumpLabel);
    ui->jumpLyt->addWidget(mRelatedGroup);
    mJumpLabel->setBuddy(mWallWidget->pushButton());

    connect(mWallWidget, &PushButtonWidget::clicked, this, [=]{
        UkccCommon::buriedSettings(name(), "set wallpaper", QString("clicked"));
        jumpFunctionSlot("wallpaper");
    });

    connect(mBeepWidget, &PushButtonWidget::clicked, this, [=]{
        UkccCommon::buriedSettings(name(), "set beep", QString("clicked"));
        jumpFunctionSlot("audio");
    });
}

void Theme::effectChangedSlot(bool status) {
    qDebug() << Q_FUNC_INFO << "effectChangedSlot status" << status;
    mEffectWidget->setChecked(status);
    m_windowMinimizeWidget->setVisible(status);
    qDebug() << Q_FUNC_INFO << "effectChangedSlot after set mEffectWidget" << status;
}

void Theme::initConnection()
{
    connect(mEffectWidget, &SwitchWidget::stateChanged, [this](bool checked) {
        UkccCommon::buriedSettings(name(), "whether open effect mode", QString("settings"), checked ? "true" : "false");
        revokeGlobalThemeSlot("getBlurEnabled", checked ? "true" : "false");
        revokeGlobalThemeSlot("getSupportBlur", "true");

        changeEffectSlot(checked);
        setEffectStatus(checked);

        revokeGlobalThemeSlot("getTransparencyBlur", checked ? QString::number(save_trans) : "100");
        revokeGlobalThemeSlot("getSupportTransparency", "true");
    });
    connect(mMateBgSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "pictureFilename" && mGlobalthemehelper != nullptr) {
            if (m_isApplyingGlobalTheme) {
                return;
            }
            if (!mGlobalSettings || mGlobalSettings->get(GLOBAL_NAME_KEY).toString() != "custom") {
                return;
            }
            const QString& value = mMateBgSettings->get(key).toString();
            mGlobalthemehelper->updateCustomThemeSetting("getWallPaperPath", value);
        }
    });

    auto updateCustomSoundTheme = [=](QGSettings *soundSettings, const QString &key) {
        if ((key != "themeName" && key != SOUND_THEME_KEY) || !soundSettings || !mGlobalthemehelper) {
            return;
        }
        if (m_isApplyingGlobalTheme) {
            return;
        }
        if (!mGlobalSettings || mGlobalSettings->get(GLOBAL_NAME_KEY).toString() != "custom") {
            return;
        }
        mGlobalthemehelper->updateCustomThemeSetting("getSoundThemeName", soundSettings->get(SOUND_THEME_KEY).toString());
    };

    if (m_ukuiSoundSettings) {
        connect(m_ukuiSoundSettings, &QGSettings::changed, this, [=](const QString &key) {
            updateCustomSoundTheme(m_ukuiSoundSettings, key);
        });
    }

    // 监听主题改变
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        QString currentThemeMode = qtSettings->get(key).toString();
        if (key == "styleName") {
            // 获取当前主题
            if (mAutoSettings) {
                setThemeBtnStatus();
            }

            bool isAuto = mAutoSettings->get(COLOR_KEY_AUTO_THEME).toBool();
            qDebug() << Q_FUNC_INFO << "m_clickTheme" << m_clickTheme << "isAuto" << isAuto;
            if (!m_clickTheme && !isAuto) {
                qDebug() << Q_FUNC_INFO << "currentThemeMode changed" << currentThemeMode;
                if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
                    mThemeBtnGroup->buttonClicked(mDefaultBtn);
                    emit mDefaultBtn->clicked();
                } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode) {
                    mThemeModeBtnGroup->buttonClicked(mDarkBtn);
                    emit mDarkBtn->clicked();
                } else {
                    mThemeModeBtnGroup->buttonClicked(mLightBtn);
                    emit mLightBtn->clicked();
                }
                m_clickTheme = false;
            } else {
                m_clickTheme = false;
            }
        }

        if (key == "iconThemeName") {
            QString icoName = qtSettings->get(ICON_QT_KEY).toString();
            foreach (QAbstractButton *btn, mThemeIconBtnGroup->buttons()) {
                if (btn->property("value").isValid() && btn->property("value") == icoName) {
                    disconnect(mThemeIconBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(iconThemeBtnClickSlot(QAbstractButton*)));
                    btn->click();
                    connect(mThemeIconBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(iconThemeBtnClickSlot(QAbstractButton*)));
                }
            }
        }

        if (key == "themeColor") {
            updateAccentColorSlot(currentThemeMode);
//            qApp->setStyle(new InternalStyle("ukui")); // 为解决bug#360709设置，当前已无破晓主题，注释后之前bug不复现
        }

        if (key == "windowRadius") {
            updateRadiusStatus(qtSettings->get(key).toString());
        }
    });

    connect(gtkSettings,&QGSettings::changed,this,[=] (const QString &key) {
       if(key == "iconTheme") {
           QString icoName = qtSettings->get(ICON_QT_KEY).toString();
           foreach (QAbstractButton *btn, mThemeIconBtnGroup->buttons()) {
               if (btn->property("value").isValid() && btn->property("value") == icoName) {
                   disconnect(mThemeIconBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(iconThemeBtnClickSlot(QAbstractButton*)));
                   btn->click();
                   connect(mThemeIconBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(iconThemeBtnClickSlot(QAbstractButton*)));
               }
           }
       }
    });

    connect(curSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "cursorTheme") {
            QString cursorTheme = curSettings->get(CURSOR_THEME_KEY).toString();
            foreach (QAbstractButton *btn, mThemeCursorBtnGroup->buttons()) {
                if (btn->property("value").isValid() && btn->property("value") == cursorTheme) {
                    disconnect(mThemeCursorBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(cursorThemeBtnClickSlot(QAbstractButton*)));
                    btn->click();
                    connect(mThemeCursorBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(cursorThemeBtnClickSlot(QAbstractButton*)));
                }
            }
        }
    });

    connect(personliseGsettings, &QGSettings::changed,this,[=] (const QString &key) {
        if (key == "effect") {
            if (UkccCommon::isOpenkylin())
                qApp->setStyle(new InternalStyle("ukui"));
            bool effectEnabled = personliseGsettings->get(PERSONALSIE_EFFECT_KEY).toBool();
            bool effectBtnStatus = mEffectWidget->isChecked();
            if (effectEnabled != effectBtnStatus) {
                changeEffectSlot(effectEnabled);
                mEffectWidget->blockSignals(true);
                mEffectWidget->setChecked(effectEnabled);
                mEffectWidget->blockSignals(false);
            }
       }
    });

    if (mAutoSettings) {
        connect(mAutoSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "themeScheduleAutomatic") {
                bool isAutoTheme = getAutoTheme();
                if (isAutoTheme) {
                    if (!mAutoBtn->isChecked()) {
                        emit mAutoBtn->click();
                    }
                } else {
                    setThemeBtnStatus();
                }
             }
        });
    }

    connect(mThemeModeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(themeBtnClickSlot(QAbstractButton*)));

    connect(mTranWidget->slider(), &KSlider::valueChanged, this, [=](int value) {
        changeTranpancySlot(value);
    });

    connect(mTranWidget->slider(), &KSlider::sliderReleased, this, [=]() {
        revokeGlobalThemeSlot("getTransparencyBlur", QString::number(mTranWidget->slider()->value(), 10));
        revokeGlobalThemeSlot("getSupportTransparency", "true");
        UkccCommon::buriedSettings(name(), "set transparency", QString("select"), QString::number(mTranWidget->slider()->value()));
    });

    connect(mCornerWidget, &ComboxWidget::currentIndexChanged, this, [=](int index) {
        CornerComboxIndexChangedSlot(index);
    });

    connect(m_windowMinimizeWidget, &ComboxWidget::currentIndexChanged, this, [=](int index) {
        windowMinimizeComboxIndexChangedSlot(index);
    });
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Theme",
                                          "org.ukui.ukcc.session.Theme",
                                          "effectChanged",
                                          this,
                                          SLOT(effectChangedSlot(bool)));
}

void Theme::CornerComboxIndexChangedSlot(int index) {
    QString value = mCornerWidget->comboBox()->itemData(index).toString();
    qDebug() << Q_FUNC_INFO << "mCornerWidget combox currentIndexChanged" << index << value;
    setRadiusSlot(value);
    revokeGlobalThemeSlot("getRadiusValue", value);
    UkccCommon::buriedSettings(name(), "set radiusValue", QString("settings"), value);
}

void Theme::windowMinimizeComboxIndexChangedSlot(int index) {
    if (!m_wlcomViewInterface) {
        qDebug() << Q_FUNC_INFO << "com.kylin.Wlcom is null";
        return;
    }
    uint32_t value = m_windowMinimizeWidget->comboBox()->itemData(index).toUInt();
    m_wlcomViewInterface->call("SetMinimizeEffect", value);
    qDebug() << Q_FUNC_INFO << "windowMinimizeWidget combox currentIndexChanged" << index << value;
    UkccCommon::buriedSettings(name(), "set minimizeEffect", QString("settings"), QString::number(value));
}

void Theme::initIconThemeWidget(QString themedir , FlowLayout *horLyt)
{
    // 获取当前图标主题(以QT为准，后续可以对比GTK两个值)
    QString currentIconTheme = qtSettings->get(ICON_QT_KEY).toString();

    IconTheme curIconTheme(themedir);

    QDir appsDir = QDir(ICONTHEMEPATH + themedir + "/32x32/apps/");
    QDir placesDir = QDir(ICONTHEMEPATH + themedir + "/32x32/places/");
    QDir devicesDir = QDir(ICONTHEMEPATH + themedir + "/32x32/devices/");
    appsDir.setFilter(QDir::Files | QDir::NoSymLinks);
    devicesDir.setFilter(QDir::Files | QDir::NoSymLinks);
    placesDir.setFilter(QDir::Files | QDir::NoSymLinks);

    QStringList showIconsList;
    QStringList realIconsList;

    if (!UkccCommon::isTablet()) {
         realIconsList = defaultIconList;
    } else {
        realIconsList = kIntelIconList;
    }

    for (int i = 0; i < realIconsList.count(); i++) {
        QString iconPath = curIconTheme.iconPath(realIconsList.at(i), 32, qApp->devicePixelRatio());

        if (!iconPath.isEmpty()) {
            showIconsList.append(iconPath);
        } else {
            iconPath = curIconTheme.iconPath("application-x-desktop.png", 32, qApp->devicePixelRatio());
            if (iconPath.isEmpty())
                return;
            showIconsList.append(iconPath);
        }

    }
    QString iconThemeName = dullTranslation(themedir.section("-", -1, -1, QString::SectionSkipEmpty));
    ThemeButton *button = new ThemeButton(iconThemeName, showIconsList, pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(button, "Theme", "", "icon theme of" + QString(ICONTHEMEPATH) + themedir);
    button->setAccessibleOptionName(tr("Icon Theme"));
    button->setObjectName(themedir);
    button->setProperty("value", themedir);
     mThemeIconBtnGroup->addButton(button);
    connect(mThemeIconBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        emit button->btnCheckedSignals(eBtn == button ? true : false);
        // 图标主题标题只绑定当前选中项
        if (mThemeIconLabel && eBtn) {
            mThemeIconLabel->setBuddy(eBtn);
        }
    });
    // 加入Layout
    horLyt->addWidget(button);

    if (themedir == currentIconTheme){
        button->clicked();
        mThemeIconBtnGroup->buttonClicked(button);
    }
}

void Theme::initCursorThemeWidget(QString themedir, FlowLayout *fLyt)
{
    // 获取当前指针主题
    QString currentCursorTheme;
    currentCursorTheme = curSettings->get(CURSOR_THEME_KEY).toString();
    if (themedir == "DMZ-Black"  && !mIsUpgrade) return;

    QList<QPixmap> cursorVec;
    QString path = CURSORS_THEMES_PATH + themedir;
    XCursorTheme *cursorTheme = new XCursorTheme(path);

    for(int i = 0; i < numCursors; i++){
        int size = qApp->devicePixelRatio() * 8;
        QImage image = cursorTheme->loadImage(cursor_names[i],size);
        cursorVec.append(QPixmap::fromImage(image));
    }

    QString cursorThemeName = dullCursorTranslation(themedir);
    ThemeButton *button = new ThemeButton(cursorThemeName, cursorVec, pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(button, "Theme", "", "cursor theme of" + QString(CURSORS_THEMES_PATH) + themedir);
    button->setAccessibleOptionName(tr("Cursor theme"));
    button->setObjectName(themedir);
    button->setProperty("value", themedir);
    mThemeCursorBtnGroup->addButton(button);
    connect(mThemeCursorBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        emit button->btnCheckedSignals(eBtn == button ? true : false);
        // 光标主题标题只绑定当前选中项
        if (mThemeCursorLabel && eBtn) {
            mThemeCursorLabel->setBuddy(eBtn);
        }
    });
    // 加入Layout
    fLyt->addWidget(button);

    if (themedir == currentCursorTheme){
        button->clicked();
        mThemeCursorBtnGroup->buttonClicked(button);
    }
}

QStringList Theme::getSystemCursorThemes() {
    QStringList themes;
    QDir themesDir(CURSORS_THEMES_PATH);

    if (themesDir.exists()){
        foreach (QString dirname, themesDir.entryList(QDir::Dirs)){
            if (dirname == "." || dirname == "..")
                continue;
            QDir themeDir(CURSORS_THEMES_PATH + dirname + "/cursors/");
            if (themeDir.exists())
                themes.append(dirname);
        }
    }
    return themes;
}

void Theme::setThemeBtnStatus()
{
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    bool isAutoTheme = getAutoTheme();

    for (QAbstractButton * button : mThemeModeBtnGroup->buttons()) {
        QString propertyName = button->property("value").toString();
        if ("ukui-black" == currentThemeMode) {
            currentThemeMode = "ukui-dark";
        } else if ("ukui-white" == currentThemeMode) {
            currentThemeMode = "ukui-light";
        } else if ("ukui-auto" == propertyName && isAutoTheme && button->isChecked()) {
            return ;
        }

        if (propertyName == currentThemeMode && !isAutoTheme) {
            disconnect(mThemeModeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(themeBtnClickSlot(QAbstractButton*)));
            button->click();
            connect(mThemeModeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(themeBtnClickSlot(QAbstractButton*)));
        }
    }
    // Tab 落当前选中；标题 buddy 跟着选中项走（仍只绑一个）
    if (QAbstractButton *checked = mThemeModeBtnGroup->checkedButton()) {
        mThemeModeLabel->setBuddy(checked);
        if (auto *tb = qobject_cast<ThemeButton *>(checked)) {
            tb->refreshGroupKeyboardNav();
        }
    } else if (mLightBtn) {
        mLightBtn->refreshGroupKeyboardNav();
    }
}

QStringList Theme::readFile(QString filepath) {
    QStringList fileCont;
    QFile file(filepath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            QString line= textStream.readLine();
            line.remove('\n');
            fileCont<<line;
        }
        file.close();
        return fileCont;
    } else {
        qWarning() << filepath << " not found"<<endl;
        return QStringList();
    }
}

void Theme::setupGSettings() {
    const QByteArray id(THEME_GTK_SCHEMA);
    const QByteArray idd(THEME_QT_SCHEMA);
    const QByteArray iid(CURSOR_THEME_SCHEMA);
    const QByteArray iiid(PERSONALSIE_SCHEMA);
    const QByteArray globalID(GLOBAL_SCHEMA);
    const QByteArray bgID(BG_MATE_SCHEMA);
    const QByteArray ukuiSoundID(SOUND_UKUI_SCHEMA);
    const QByteArray autoID(AUTO_SCHEMA);

    gtkSettings         = new QGSettings(id,   QByteArray(), this);
    qtSettings          = new QGSettings(idd,  QByteArray(), this);
    curSettings         = new QGSettings(iid,  QByteArray(), this);
    personliseGsettings = new QGSettings(iiid, QByteArray(), this);
    mMateBgSettings     = new QGSettings(bgID, QByteArray(), this);
    if (QGSettings::isSchemaInstalled(ukuiSoundID)) {
        m_ukuiSoundSettings = new QGSettings(ukuiSoundID, QByteArray(), this);
    }

    if (QGSettings::isSchemaInstalled(autoID)) {
        mAutoSettings = new QGSettings(autoID, QByteArray(), this);
    }

    if (QGSettings::isSchemaInstalled(globalID)) {
        mGlobalSettings = new QGSettings(globalID, QByteArray(), this);
        connect(mGlobalSettings, &QGSettings::changed, this, &Theme::setCustomGlobalSlot);
    }
}

void Theme::setEffectStatus(bool status)
{
    // 特效开关只持久化到 GSettings
    personliseGsettings->set(PERSONALSIE_EFFECT_KEY, status);
}

void Theme::kwinCursorSlot(QString value) {

    QString filename = QDir::homePath() + "/.config/kcminputrc";
    QSettings *mouseSettings = new QSettings(filename, QSettings::IniFormat);

    mouseSettings->beginGroup("Mouse");
    mouseSettings->setValue("cursorTheme", value);
    mouseSettings->endGroup();

    delete mouseSettings;
    mouseSettings = nullptr;

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(5);
    args.append(0);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

QString Theme::dullCursorTranslation(QString str) {
    if (!QString::compare(str, "blue-crystal")){
        return tr("Blue-Crystal");
    } else if (!QString::compare(str, "lightseeking")) {
        return tr("Light-Seeking");
    } else if (!QString::compare(str, "DMZ-Black")) {
        return tr("DMZ-Black");
    } else if (!QString::compare(str, "DMZ-White")) {
        return tr("DMZ-White");
    } else if (!QString::compare(str, "dark-sense")) {
        return tr("Dark-Sense");
    } else {
        return str;
    }
}

QString Theme::getCursorName() {
    return curSettings->get(CURSOR_THEME_KEY).toString();
}

QString Theme::getGlobalName()
{
    mGlobalName = mGlobalSettings->get(GLOBAL_NAME_KEY).toString();
    return mGlobalName;
}

bool Theme::getAutoTheme()
{
    return mAutoSettings->get(COLOR_KEY_AUTO_THEME).toBool();
}

void Theme::setVisibleBySecurity() {
    QVariantMap moduleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = moduleMap.value(name().toLower() + "Settings").toString();

    QStringList setItems = moduleSettings.split(",");
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        if (item.at(0) == "themeModeFrame") {
            mThemeModeFrame->setVisible(item.at(1) == "true");
            mThemeModeLabel->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "globalModeFrame") {
            mSwitchFrame->setVisible(item.at(1) == "true");
            mSwitchLabel->setVisible(item.at(1) == "true");
        }
    }
}

void Theme::setEnableBySecurity() {
    QVariantMap moduleMap = UkccCommon::getModuleHideStatus();
    QString moduleEnables  = moduleMap.value(name().toLower() + "Enable").toString();
    QStringList enableItems = moduleEnables.split(",");
    foreach (QString enableItem, enableItems) {
        QStringList item = enableItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        if (item.at(0) == "themeModeFrame") {
            mThemeModeFrame->setEnabled(item.at(1) == "true");
            mThemeModeLabel->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "globalModeFrame") {
            mSwitchFrame->setEnabled(item.at(1) == "true");
            mSwitchLabel->setEnabled(item.at(1) == "true");
            foreach (auto key, mGlobalMap.keys()) {
                mGlobalMap[key]->setEnabled(item.at(1) == "true");
            }
        }
        if (item.at(0) == "themeAndEffectFrame") {
            mEffectGroup->setEnabled(item.at(1) == "true");
            mEffectLabel->setEnabled(item.at(1) == "true");

            mThemeModeFrame->setEnabled(item.at(1) == "true");
            mThemeModeLabel->setEnabled(item.at(1) == "true");

            mSwitchFrame->setEnabled(item.at(1) == "true");
            mSwitchLabel->setEnabled(item.at(1) == "true");
            foreach (auto key, mGlobalMap.keys()) {
                mGlobalMap[key]->setEnabled(item.at(1) == "true");
            }

            mThemeCursorFrame->setEnabled(item.at(1) == "true");
            mThemeCursorLabel->setEnabled(item.at(1) == "true");

            mThemeIconFrame->setEnabled(item.at(1) == "true");
            mThemeIconLabel->setEnabled(item.at(1) == "true");

            mControlFrame->setEnabled(item.at(1) == "true");
            mControlLabel->setEnabled(item.at(1) == "true");
        }
    }
}

void Theme::securityControl() {
    mDefaultBtn->setVisible(false);

    if (UkccCommon::isTablet()) {
        mTranWidget->setVisible(false);
    }

    setVisibleBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setVisibleBySecurity()));
    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void Theme::clickedCustomTheme()
{
    if (mCustomPicUnit && !mCustomPicUnit->isChecked()) {
        disconnect(mThemeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(changeGlobalThemeSlot(QAbstractButton*)));
        mCustomPicUnit->setBtnClicked(true);
        mThemeBtnGroup->buttonClicked(mCustomPicUnit);
        connect(mThemeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(changeGlobalThemeSlot(QAbstractButton*)));
    }

    if (mPrePicUnit && mPrePicUnit != mCustomPicUnit) {
        mPrePicUnit->setBtnClicked(false);
        mPrePicUnit = mCustomPicUnit;
    }
}

void Theme::updateRadiusStatus(QString radius)
{
    qDebug() << Q_FUNC_INFO << "updateRadiusStatus" << radius;
    mCornerWidget->blockSignals(true);
    mCornerWidget->setCurrentIndex(mCornerWidget->comboBox()->findData(radius));
    mCornerWidget->blockSignals(false);
}

bool Theme::isBlurEffect()
{
    QDBusInterface ifc(!mIsOpenkylin ? "org.ukui.KWin" : "org.kde.KWin",
                       "/Effects",
                       !mIsOpenkylin ? "org.ukui.kwin.Effects" : "org.kde.kwin.Effects",
                       QDBusConnection::sessionBus());
    QStringList effects = ifc.property("activeEffects").toStringList();
    return effects.contains("blur");
}

bool Theme::isUpgrade()
{
    version_t ver= kdk_system_get_version_detaile();
    QString version = QString(ver.update_version);
    mIsUpgrade = (version != "2303");
    return mIsUpgrade;
}



QString Theme::dullTranslation(QString str) {
    if (!QString::compare(str, "basic")){
        return tr("basic");
    } else if (!QString::compare(str, "classical")) { // 启典
        return UkccCommon::isTablet() ? tr("classic") : tr("Classic");
    } else if (!QString::compare(str, "default") || !QString::compare(str, "xunguang")) { // 寻光
        return UkccCommon::isTablet() ? tr("basic") : tr("Light-Seeking");
    } else if (!QString::compare(str, "fashion") || !QString::compare(str, "heyin")) {   // 和印
        return tr("HeYin");
    } else if (!QString::compare(str, "hp")) {
        return tr("hp");
    } else if (!QString::compare(str, "ukui")) {
        return UkccCommon::isTablet() ? tr("basic") : tr("ukui");
    } else if (!QString::compare(str, "daybreakBlue")) {
        return tr("daybreakBlue");
    } else if (!QString::compare(str, "jamPurple")) {
        return tr("jamPurple");
    } else if (!QString::compare(str, "magenta")) {
        return tr("magenta");
    } else if (!QString::compare(str, "sunRed")) {
        return tr("sunRed");
    } else if (!QString::compare(str, "sunsetOrange")) {
        return tr("sunsetOrange");
    } else if (!QString::compare(str, "dustGold")) {
        return tr("dustGold");
    } else if (!QString::compare(str, "polarGreen")) {
        return tr("polarGreen");
    } else if (!QString::compare(str, "dawnlight")) {
        return tr("dawnlight");
    }  else {
        return str;
    }
}

QStringList Theme::getSystemIconThemes()
{
    QStringList themes;
    QDir themesDir(ICONTHEMEPATH);

    if (themesDir.exists()){
        foreach (QString dirname, themesDir.entryList(QDir::Dirs)){
            if (dirname == "." || dirname == "..")
                continue;
            QFile iconthemefile(ICONTHEMEPATH + dirname + "/icon-theme.cache");
            if (iconthemefile.exists())
                themes.append(dirname);
        }
    }
    return themes;
}

// 重置设置
void Theme::resetBtnClickSlot() {

    emit mThemeModeBtnGroup->buttonClicked(mDefaultBtn);

    curSettings->reset(CURSOR_THEME_KEY);
    QString cursorTheme = kDefCursor;
    QString defaultCursor = getCursorName();
    if (defaultCursor.isEmpty()) {
        curSettings->set(CURSOR_THEME_KEY, kDefCursor);
    } else {
        cursorTheme = defaultCursor;
    }
    kwinCursorSlot(cursorTheme);

    qtSettings->reset(ICON_QT_KEY);
    gtkSettings->reset(ICON_GTK_KEY);

    if (mEffectWidget->isVisible()) {
        mEffectWidget->setChecked(true);
        qtSettings->reset(THEME_TRAN_KEY);
        qtSettings->reset(PEONY_TRAN_KEY);
        personliseGsettings->reset(PERSONALSIE_TRAN_KEY);
        mTranWidget->slider()->setValue(transparency);
    }
}

void Theme::writeKwinSettings(bool change) {

    if (UkccCommon::isWayland()) {
        qDebug() << Q_FUNC_INFO << "write wlcom config";
        writeWlcomSettings(change);
        return;
    }
    if (!QFile::exists(m_kwinFile)) {
        return;
    }

    if (!change) {
        kwinSettings->beginGroup("Plugins");
        kwinSettings->setValue("kwin4_effect_maximizeEnabled", false);
        kwinSettings->setValue("blurEnabled", false);
        kwinSettings->endGroup();

        for (int i = 0; i < effectList.length(); i++) {
            QDBusMessage message = QDBusMessage::createMethodCall(!mIsOpenkylin ? "org.ukui.KWin" : "org.kde.KWin",
                                                       "/Effects",
                                                       !mIsOpenkylin ? "org.ukui.kwin.Effects" : "org.kde.kwin.Effects",
                                                       "unloadEffect");
            message << effectList.at(i);
            QDBusConnection::sessionBus().send(message);
        }
    } else {
        kwinSettings->beginGroup("Plugins");
        kwinSettings->setValue("kwin4_effect_maximizeEnabled", true);
        kwinSettings->setValue("blurEnabled", true);
        kwinSettings->endGroup();
        for (int i = 0; i < effectList.length(); i++) {

            QDBusMessage message = QDBusMessage::createMethodCall(!mIsOpenkylin ? "org.ukui.KWin" : "org.kde.KWin",
                                                                  "/Effects",
                                                                  !mIsOpenkylin ? "org.ukui.kwin.Effects" : "org.kde.kwin.Effects",
                                                                  "loadEffect");
            message << effectList.at(i);
            QDBusConnection::sessionBus().send(message);
        }
    }
    kwinSettings->sync();
}

void Theme::writeWlcomSettings(bool change)
{
    if (m_wlcomInterface) {
        m_wlcomInterface->asyncCall("EnableEffect", "blur", change);
    }
    if (!change) {
        setRadiusSlot("0, 0");
        qDebug() << Q_FUNC_INFO << "writeWlcomSettings change" << change << "0,0";
        personliseGsettings->set(PERSONALSIE_RADIUS_KEY, mCornerWidget->comboBox()->currentData().toString());
    } else {
        setRadiusSlot(personliseGsettings->get(PERSONALSIE_RADIUS_KEY).toString());
    }
}

void Theme::themeBtnClickSlot(QAbstractButton *button) {
    // 设置主题
    // 标题只 buddy 当前选中项，便于 Tab 再次进入时读到「窗口外观」
    if (mThemeModeLabel && button) {
        mThemeModeLabel->setBuddy(button);
    }
    QString themeMode = button->property("value").toString();
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    QString tmpMode;
    UkccCommon::buriedSettings(name(), "set theme mode", QString("settings"), themeMode);
    qDebug() << Q_FUNC_INFO << "themeMode" << themeMode << "gsetting currentThemeMode" << currentThemeMode;
    if ("ukui-dark" == themeMode) {
        tmpMode = "ukui-black";
        revokeGlobalThemeSlot("getDefaultLightDarkMode", "dark");
        revokeGlobalThemeSlot("getQtDarkThemeName", themeMode);
        revokeGlobalThemeSlot("getGtkDarkThemeName", tmpMode);
        mAutoSettings->set(COLOR_KEY_AUTO_THEME, false);
    } else if ("ukui-light" == themeMode) {
        tmpMode = "ukui-white";
        revokeGlobalThemeSlot("getDefaultLightDarkMode", "light");
        revokeGlobalThemeSlot("getQtLightThemeName", themeMode);
        revokeGlobalThemeSlot("getGtkLightThemeName", tmpMode);
        mAutoSettings->set(COLOR_KEY_AUTO_THEME, false);
    } else if ("ukui-default" == themeMode) {
        revokeGlobalThemeSlot("getDefaultLightDarkMode", "light");
        revokeGlobalThemeSlot("getQtLightThemeName", themeMode);
        revokeGlobalThemeSlot("getGtkLightThemeName", tmpMode);
        mAutoSettings->set(COLOR_KEY_AUTO_THEME, false);
    } else if (themeMode == "ukui-auto" && button->isChecked()) {
        revokeGlobalThemeSlot("getDefaultLightDarkMode", "auto");
        clickedCustomTheme();
        qDebug() << Q_FUNC_INFO << "themeMode is ukui-auto set DefaultLightDarkMode as auto";
        mAutoSettings->set(COLOR_KEY_AUTO_THEME, true);
        return;
    }
    gtkSettings->set(MODE_GTK_KEY, tmpMode);
    QString styleName = qtSettings->get(MODE_QT_KEY).toString();
    if (styleName != themeMode) {
        m_clickTheme = true;
    } else {
        m_clickTheme = false;
    }
    qDebug() << Q_FUNC_INFO << "set style-name" << themeMode << "m_clickTheme" << m_clickTheme;
    qtSettings->set(MODE_QT_KEY, themeMode);

    qApp->setStyle(new InternalStyle("ukui"));
}

void Theme::changeGlobalThemeSlot(QAbstractButton *button)
{
    QScopedValueRollback<bool> applyingGlobalThemeGuard(m_isApplyingGlobalTheme, true);
    // 主题区标题只 buddy 当前选中项
    if (mSwitchLabel && button) {
        mSwitchLabel->setBuddy(button);
    }
    QVariant value = button->property("value");
    GlobalTheme *theme = nullptr;
    for (GlobalTheme *globaltheme: mGlobalthemehelper->getAllGlobalThemes()) {
        if (globaltheme->getThemeName() == value)
            theme = globaltheme;
    }
    if (!theme) return;
    UkccCommon::buriedSettings(name(), "set theme", QString("settings"), value.toString());

    if (mPrePicUnit != nullptr) {
        if (mPrePicUnit->property("value") != value) {
            mPrePicUnit->setBtnClicked(false);
        }
    }

    mPrePicUnit = static_cast<ThemeButton *>(button);

    bool supportBlur  = mEffectWidget->isVisible();
    bool blurEnable   = theme->getBlurEnabled();

    int transparency = theme->getTransparencyBlur();
    QString radiusValue = theme->getRadiusValue();

    QString globalName = theme->getThemeName();
    QString wallpaper  = theme->getWallPaperPath();
    QString iconName   = theme->getIconThemeName();
    QString cursorName = theme->getCursorThemeName();
    QString widgetName = theme->getWidgetStyleName();
    QString colorTheme = theme->getDefaultLightDarkMode();
    QString accentColor= theme->getQtAccentRole();
    QString gtkColorTheme;

    qDebug() << "changeGlobalThemeSlot value" << value << "colorTheme" << colorTheme;
    if (!colorTheme.compare("light")) {
        colorTheme = theme->getQtLightThemeName();
        gtkColorTheme = theme->getGtkLightThemeName();
    } else if (!colorTheme.compare("dark")) {
        colorTheme = theme->getQtDarkThemeName();
        gtkColorTheme = theme->getGtkDarkThemeName();
    } else if (!colorTheme.compare("mixed")) {
        colorTheme = theme->getQtMixedThemeName();
        gtkColorTheme = theme->getGtkLightThemeName();
    }

    QDBusInterface defaultUkccDbus("org.ukui.ukcc.session",
                                    "/Wallpaper",
                                    "org.ukui.ukcc.session.Wallpaper",
                                    QDBusConnection::sessionBus());
    defaultUkccDbus.call("setNeedChangeCustomTheme", false);
    qDebug() << Q_FUNC_INFO << "setNeedChangeCustomTheme set false";
    mMateBgSettings->set(BG_FILE_KEY, wallpaper);
    mGlobalSettings->set(GLOBAL_NAME_KEY, globalName);
    mGlobalSettings->set(ISMODIFY_KEY, false);

    qtSettings->set(ICON_QT_KEY, iconName);
    if (qtSettings->keys().contains("widgetThemeName")) {
        qtSettings->set(WIDGET_QT_KEY, widgetName);
    }
    QString realColor = getRealColorName(accentColor);
    qDebug() << Q_FUNC_INFO << "accentColor" << accentColor << "realColor" << realColor;
    qtSettings->set(COLOR_QT_KEY, realColor);
    gtkSettings->set(ICON_GTK_KEY, iconName);
    if (colorTheme.compare("auto")) {
        QString styleName = qtSettings->get(MODE_QT_KEY).toString();
        if (styleName != colorTheme) {
            m_clickTheme = true;
        } else {
            m_clickTheme = false;
        }
        qDebug() << Q_FUNC_INFO << "set style-name" << colorTheme << "styleName" << styleName << "m_clickTheme" << m_clickTheme;
        qtSettings->set(MODE_QT_KEY, colorTheme);
        gtkSettings->set(MODE_GTK_KEY, gtkColorTheme);
    } else {
        qDebug() << Q_FUNC_INFO << "colorTheme is auto, change to ukui-auto";
        mAutoSettings->set(COLOR_KEY_AUTO_THEME, true);
        for (QAbstractButton * button : mThemeModeBtnGroup->buttons()) {
            QString propertyName = button->property("value").toString();
            qDebug() << Q_FUNC_INFO << "button propertyName" << propertyName;
            if ("ukui-auto" != propertyName) {
                continue;
            }
            disconnect(mThemeModeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(themeBtnClickSlot(QAbstractButton*)));
            button->click();
            connect(mThemeModeBtnGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(themeBtnClickSlot(QAbstractButton*)));
        }
    }

    curSettings->set(CURSOR_THEME_KEY, cursorName);
    const QString soundTheme = theme->getSoundThemeName();
    if (m_ukuiSoundSettings) {
        m_ukuiSoundSettings->set(SOUND_THEME_KEY, soundTheme);
    }

    if (mAutoSettings && value != "custom")  {
        mAutoSettings->set(COLOR_KEY_AUTO_THEME, false);
    }

#ifdef OPENKYLIN
    curSettings->set(CURSOR_THEME_KEY, cursorName);
#endif

    if (supportBlur) {
        qDebug() << Q_FUNC_INFO << "blurEnable" << blurEnable;
        changeEffectBtnSlot(blurEnable);

        qtSettings->set(WINDOW_RAIDUS, radiusValue);   // 设置主题的圆角必须在changeEffectBtnSlot之后，否则特效关闭，然后切换主题，会设置成直角
        qDebug() << Q_FUNC_INFO << "changeGlobalThemeSlot windowRadius" << radiusValue;

        qDebug() << Q_FUNC_INFO << "transparency" << transparency;
        changeTranpancySliderSlot(transparency);
    }

    qApp->setStyle(new InternalStyle("ukui"));
}

void Theme::revokeGlobalThemeSlot(const QString &funcName, const QString &value)
{
    if (mGlobalSettings == nullptr) {
        return;
    }

    QString currentThemeName = mGlobalSettings->get(GLOBAL_NAME_KEY).toString();
    if (!currentThemeName.isEmpty() && currentThemeName != "custom") {
        QList<GlobalTheme *> themes = mGlobalthemehelper->getAllGlobalThemes();
        for (GlobalTheme *theme : themes) {
            if (theme->getThemeName() == currentThemeName) {
                QString currentValue;
                if (QMetaObject::invokeMethod(theme, funcName.toUtf8().constData(),
                                              Q_RETURN_ARG(QString, currentValue))) {
                    if (currentValue == value) {
                        return;
                    }
                }
                break;
            }
        }
    }

    mGlobalSettings->set(ISMODIFY_KEY, true);
    mGlobalSettings->set(GLOBAL_NAME_KEY, "custom");

    clickedCustomTheme();

    mGlobalthemehelper->updateCustomThemeSetting(funcName, value);
}

void Theme::changeGlobalDirSlot()
{
    GlobalThemeHelper *globalHelper = GlobalThemeHelper::getInstance();
    QMap<QString, GlobalTheme*> actGlobal;
    const QStringList &mapKeys = mGlobalMap.keys();
    for (GlobalTheme *theme: globalHelper->getAllGlobalThemes()) {
        globalHelper->loadThumbnail(theme);
        actGlobal.insert(theme->getLocaleThemeName(), theme);
    }

    QTimer::singleShot(500, this, [=]{
        if (actGlobal.keys().length() > mapKeys.length()) {
            for (QString themeName : actGlobal.keys()) {
                if (!mapKeys.contains(themeName)) {
                    ThemeButton *btn = new ThemeButton(themeName, actGlobal[themeName]->getThumbnail(), mSwitchFrame);
                    QString property = actGlobal[themeName]->getThemeName();
                    btn->setProperty("value", property);
                    btn->setAccessibleOptionName(tr("Theme"));
                    mThemeBtnGroup->addButton(btn);
                    mFLyt->addWidget(btn);
                    mGlobalMap.insert(themeName, btn);
                }
            }
        } else {
            for (QString themeName : mapKeys) {
                if (!actGlobal.keys().contains(themeName)) {
                    ThemeButton *gloablBtn = mGlobalMap[themeName];
                    if (gloablBtn->property("value") == getGlobalName()) {
                        mCustomPicUnit->setBtnClicked(true);
                        mThemeBtnGroup->buttonClicked(mCustomPicUnit);
                    }
                    mThemeBtnGroup->removeButton(gloablBtn);
                    mGlobalMap.remove(themeName);
                    delete gloablBtn;
                }
            }
        }
    });
}

void Theme::changeEffectSlot(bool checked)
{
    if (!checked) {
        save_trans = static_cast<int>(personliseGsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 100.0);
        personliseGsettings->set(PERSONALSIE_SAVE_TRAN_KEY, save_trans);
        personliseGsettings->set(PERSONALSIE_TRAN_KEY, 1.0);
        qtSettings->set(THEME_TRAN_KEY, 100);
        qtSettings->set(PEONY_TRAN_KEY, 100);
        changeTranpancySliderSlot(100);

        QTimer::singleShot(100, this, [=] {
            writeKwinSettings(checked);
        });
    } else {
        save_trans = personliseGsettings->get(PERSONALSIE_SAVE_TRAN_KEY).toInt();
        changeTranpancySliderSlot(save_trans);
        writeKwinSettings(checked);
    }

    if (m_wlcomInterface) {
        m_wlcomInterface->asyncCall("EnableEffect", "magic_lamp", checked);
        m_wlcomInterface->asyncCall("EnableEffect", "scale", checked);
        qDebug() << Q_FUNC_INFO << "EnableEffect set scale and magic_lamp to" << checked;
    }

    // 提供给外部监听特效接口
    setEffectStatus(checked);
    QTimer::singleShot(100, this, [=, checked] {
        pluginWidget->setUpdatesEnabled(false);
        mTranWidget->setVisible(checked && !UkccCommon::isTablet());
    #ifdef Nile
        mCornerWidget->setVisible(checked);
    #endif
        m_windowMinimizeWidget->setVisible(checked);
        pluginWidget->setUpdatesEnabled(true);
        pluginWidget->update();
        qDebug() << Q_FUNC_INFO << "update changeEffectSlot";
    });

}

void Theme::changeEffectBtnSlot(bool checked)
{
    mEffectWidget->blockSignals(true);
    mEffectWidget->setChecked(checked);
    mEffectWidget->blockSignals(false);
    changeEffectSlot(checked);
}

void Theme::changeTranpancySlot(int value)
{
    personliseGsettings->set(PERSONALSIE_TRAN_KEY,(static_cast<int>(value) / 100.0));
    qtSettings->set(THEME_TRAN_KEY, value);
    qtSettings->set(PEONY_TRAN_KEY, value);
}

void Theme::changeTranpancySliderSlot(int value)
{
    mTranWidget->slider()->blockSignals(true);
    mTranWidget->slider()->setValue(value);
    mTranWidget->slider()->blockSignals(false);
    changeTranpancySlot(value);
}

void Theme::jumpFunctionSlot(const QString &arg)
{
    QProcess::startDetached("ukui-control-center", QStringList() << "-m" << arg);
}

void Theme::iconThemeBtnClickSlot(QAbstractButton *button)
{
    // 设置图标主题
    QVariant valueVariant = button->property("value");
    UkccCommon::buriedSettings(name(), "set icon theme", QString("settings"), valueVariant.toString());
    qtSettings->set(ICON_QT_KEY, valueVariant);
    gtkSettings->set(ICON_GTK_KEY, valueVariant);

    revokeGlobalThemeSlot("getIconThemeName", valueVariant.toString());
}

void Theme::cursorThemeBtnClickSlot(QAbstractButton *button)
{
    // 设置光标主题
    QVariant valueVariant = button->property("value");
    UkccCommon::buriedSettings(name(), "set cursor theme", QString("settings"), valueVariant.toString());
    curSettings->set(CURSOR_THEME_KEY, valueVariant);
    kwinCursorSlot(valueVariant.toString());
    revokeGlobalThemeSlot("getCursorThemeName", valueVariant.toString());
}

QString Theme::getRealColorName(const QString &color)
{
    if (color == "daybreakBlue") {
        return "55,144,250,1";
    } else if (color == "jamPurple") {
        return "137,38,235,1";
    } else if (color == "magenta") {
        return "228,74,232,1";
    } else if (color == "sunRed") {
        return "242,181,39,1";
    } else if (color == "sunsetOrange") {
        return "20,166,33,1";
    } else if (color == "dustGold") {
        return "240,73,67,1";
    } else if (color == "polarGreen") {
        return "250,125,15,1";
    } else {
        return color;
    }
}

void Theme::updateAccentColorSlot(const QString &color)
{
    QString realColorName = getRealColorName(color);
    qDebug() << Q_FUNC_INFO << "updateAccentColorSlot color" << color << "real color name" << realColorName;
    for (QRadioButton *btn : mRadioBtnList) {
        if (btn->property("key") == realColorName) {
            btn->blockSignals(true);
            btn->setChecked(true);
            btn->blockSignals(false);
            if (mControlLabel) {
                mControlLabel->setBuddy(btn);
            }
            if (auto *rb = qobject_cast<MyQRadioButton *>(btn)) {
                rb->refreshGroupKeyboardNav();
            }
        }
    }
}

void Theme::setCustomGlobalSlot(const QString &key)
{
    QString value = mGlobalSettings->get(key).toString();

    if (mCustomPicUnit != nullptr && value == "custom") {
        clickedCustomTheme();
    }
}

void Theme::setRadiusSlot(QString radius)
{
    if (qtSettings->keys().contains("windowRadius")) {
        qtSettings->set(WINDOW_RAIDUS, radius);
    }
}

double Theme::convertToTran(const int value) {
    switch (value) {
    case 1:
        return 0.2;
        break;
    case 2:
        return 0.4;
        break;
    case 3:
        return 0.6;
        break;
    case 4:
        return 0.8;
        break;
    case 5:
        return 1.0;
        break;
    default:
        return 1.0;
        break;
    }
}

int Theme::tranConvertToSlider(const double value) {
    if (0.2 ==  value) {
        return 1;
    } else if (0.4 ==  value) {
        return 2;
    } else if (0.6 ==  value) {
        return 3;
    } else if (0.8 ==  value) {
        return 4;
    } else if (1.0 ==  value) {
        return 5;
    } else {
        return 5;
    }
}
