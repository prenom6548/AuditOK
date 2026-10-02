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
#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QtPlugin>
#include <QLayout>
#include <QMap>
#include <QDir>
#include <QColor>
#include <QSettings>
#include <QtDBus/QDBusMessage>
#include <QGSettings>
#include <QtDBus/QtDBus>
#include <QAbstractButton>
#include <QButtonGroup>

#include <kysdk/kysdk-system/libkysysinfo.h>

#include "flowlayout.h"
#include "icontheme.h"

#include "shell/interface.h"
#include "themewidget.h"
#include "widgetgroup.h"
#include "kswitchbutton.h"
#include "globaltheme/globalthemehelper.h"
#include "globaltheme/globaltheme.h"
#include "themebutton.h"

#include "settinggroup.h"
#include "pushbuttonwidget.h"
#include "switchwidget.h"
#include "sliderwidget.h"
#include "comboxwidget.h"
#include <klabel.h>
#include <QDBusInterface>

using namespace kdk;

class QPushButton;
class QGSettings;
class WidgetGroup;

namespace Ui {
class Theme;
}

class Theme : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    enum ThemeType { ICON, CURSOR};

private:
    Ui::Theme *ui;
    bool mFirstLoad;

    QString pluginName;
    int pluginType;
    QWidget *pluginWidget;

    QGSettings *gtkSettings           = nullptr;
    QGSettings *qtSettings            = nullptr;
    QGSettings *curSettings           = nullptr;
    QGSettings *personliseGsettings   = nullptr;
    QGSettings *mGlobalSettings       = nullptr;
    QGSettings *mMateBgSettings       = nullptr;
    QGSettings *m_ukuiSoundSettings   = nullptr;
    QGSettings *mAutoSettings         = nullptr;

    QSettings  *kwinSettings          = nullptr;

    WidgetGroup *cursorThemeWidgetGroup;
    WidgetGroup *iconThemeWidgetGroup;

    KLabel *mSwitchLabel;
    KLabel *mControlLabel;
    KLabel *mJumpLabel;
    KLabel *mThemeModeLabel;
    KLabel *mThemeIconLabel;
    KLabel *mThemeCursorLabel         = nullptr;
    KLabel *mEffectLabel;

    ThemeButton *mCustomPicUnit           = nullptr;
    ThemeButton *mPrePicUnit              = nullptr;

    GlobalThemeHelper *mGlobalthemehelper = nullptr;

    FlowLayout   *mFLyt;
    QHBoxLayout  *mControlHLyt;
    QHBoxLayout  *mWallHLyt;
    QHBoxLayout  *mSoundHLyt;

    QFrame *mSwitchFrame;
    QFrame *mControlFrame;
    QFrame *mOtherFrame;
    QFrame *mThemeModeFrame;
    QFrame *mThemeIconFrame;
    QFrame *mThemeCursorFrame;

    ThemeButton *mDefaultBtn;
    ThemeButton *mDarkBtn;
    ThemeButton *mLightBtn;
    ThemeButton *mAutoBtn;

    QButtonGroup *mThemeBtnGroup;
    QButtonGroup *mThemeModeBtnGroup;
    QButtonGroup *mThemeIconBtnGroup;
    QButtonGroup *mThemeCursorBtnGroup;

    QMap<QString, QString> m_controlMap;
    QMap<QString, ThemeButton*> mGlobalMap;

    QList<QRadioButton *> mRadioBtnList;

    SettingGroup     *mRelatedGroup = nullptr;
    PushButtonWidget *mWallWidget   = nullptr;
    PushButtonWidget *mBeepWidget   = nullptr;

    SettingGroup     *mEffectGroup  = nullptr;
    SwitchWidget     *mEffectWidget = nullptr;
    SliderWidget     *mTranWidget   = nullptr;
    ComboxWidget     *mCornerWidget = nullptr;
    ComboxWidget     *m_windowMinimizeWidget = nullptr;

    QDBusInterface* m_themeInterface = nullptr;
    QDBusInterface* m_wlcomInterface = nullptr;
    QDBusInterface* m_wlcomViewInterface = nullptr;
    bool m_clickTheme = false;
    bool m_isApplyingGlobalTheme = false;

public:
    Theme();
    ~Theme();
    double i=0;
    QString plugini18nName()      Q_DECL_OVERRIDE;
    int pluginTypes()             Q_DECL_OVERRIDE;
    QWidget * pluginUi()          Q_DECL_OVERRIDE;
    const QString name() const    Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const            Q_DECL_OVERRIDE;
    bool isEnable() const         Q_DECL_OVERRIDE;

    void initThemeUi();
    void initEffectSettingUi();
    void initSearchText();
    void setupSettings();
    void setupComponent();
    void initThemeMode();
    void initIconTheme();
    void initControlTheme();
    void initCursorTheme();
    void initEffectMode();
    void initGlobalTheme();
    QList<GlobalTheme *> sortGlobalTheme(const QList<GlobalTheme *> &globalThemes);
    void initJumpTheme();
    void initConnection();
    void initIconThemeWidget(QString themedir, FlowLayout  *fLyt);
    void initCursorThemeWidget(QString themedir, FlowLayout  *fLyt);

    void buildThemeModeBtn(ThemeButton * button);

    QString dullTranslation(QString str);

    QStringList getSystemIconThemes();
    QStringList getSystemCursorThemes();
    void setThemeBtnStatus();
    QString getThemeTrans(QString key);

    QString mGlobalName;
    QString m_kwinFile;
    bool mIsUpgrade = false;

    bool mIsOpenkylin = false;

private:
    double convertToTran(const int value);
    int tranConvertToSlider(const double value);
    QStringList readFile(QString filepath);
    void setupGSettings();
    void setEffectStatus(bool status);
    void kwinCursorSlot(QString value);

    QString dullCursorTranslation(QString str);
    QString getCursorName();
    QString getGlobalName();

    bool getAutoTheme();

    void securityControl();
    void clickedCustomTheme();
    void updateRadiusStatus(QString radius);

    bool isBlurEffect();
    bool isUpgrade();
    void initCorner();
    void initCorlor();
    QString getRealColorName(const QString& color);
    void radioBtnClickSlot(const QString& value);
    void CornerComboxIndexChangedSlot(int index);
    void initWindowMinimize();
    void windowMinimizeComboxIndexChangedSlot(int index);

private slots:
    void resetBtnClickSlot();
    void writeKwinSettings(bool change);
    void writeWlcomSettings(bool change);
    void themeBtnClickSlot(QAbstractButton *button);
    void changeGlobalThemeSlot(QAbstractButton *button);
    void revokeGlobalThemeSlot(const QString& funcName, const QString& value);
    void changeGlobalDirSlot();
    void changeEffectSlot(bool checked);
    void changeEffectBtnSlot(bool checked);
    void changeTranpancySlot(int value);
    void changeTranpancySliderSlot(int value);
    void jumpFunctionSlot(const QString &arg);
    void iconThemeBtnClickSlot(QAbstractButton *button);
    void cursorThemeBtnClickSlot(QAbstractButton *button);
    void updateAccentColorSlot(const QString &color);
    void setCustomGlobalSlot(const QString &key);
    void setRadiusSlot(QString radius);
    void setVisibleBySecurity();
    void setEnableBySecurity();
    void effectChangedSlot(bool status);
};

#endif // THEME_H

