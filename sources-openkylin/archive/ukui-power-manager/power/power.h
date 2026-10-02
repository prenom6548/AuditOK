/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef POWER_H
#define POWER_H

#include <QObject>
#include <QtPlugin>
#include <QStyledItemDelegate>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRadioButton>
#include <QButtonGroup>

#include <kysdk/applications/klabel.h>

#include "interface.h"
#include "comboxframe.h"
#include "lightlabel.h"
#include <kysdk/kysdk-system/libkysysinfo.h>
#include "kswitchbutton.h"
#include "klineframe.h"
using namespace kdk;

namespace Ui {
class Power;
}

class Power : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit Power();
    ~Power();

public:
    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget *pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;

public:
    void initUI(QWidget *widget);
    void initInterface();
    void retranslateUi();
    void resetui();
    void setupComponent();
    void setupConnect();
    void initCustomPlanStatus();
    void isLidPresent();
    bool isHibernateSupply();
    bool isSuspend2HibernateSupply();
    bool isSuspendSupply();
    bool isExitBattery();
    bool isDesktop();
    bool QLabelSetText(QLabel *label, QString string);
    void clearAutoItem(QVBoxLayout *mLyt);
    void setFrame_Noframe(QFrame *frame);
    void setHintLable();

    void hideComponent();
    void hideComponentByConf2();

private:

    QWidget *pluginWidget;

    QGSettings *settings;
    QGSettings *sessionSetting;
    QGSettings *stylesettings;
    QGSettings *sessionsettings;
    QGSettings *screensettings;
    QGSettings *m_centerSettings;
    QGSettings *m_qsettings;
    QGSettings *mProcManagerSettings = nullptr;
    QDBusInterface *hostLightStripDbus = nullptr;
    bool isSupportHostLight = false;

    QString pluginName;
    int pluginType;

    KLabel *m_customTitleLabel = nullptr;
    KLabel *m_batteryPlanTitleLabel = nullptr;
    KLabel *m_acPowerSettingsLabel = nullptr;
    KLabel *m_batteryPowerSettingsTitleLabel = nullptr;
    QLabel *mSleepPwdLabel;
    QLabel *mWakenPwdLabel;
    QLabel *mPowerKeyLabel;
    QLabel *m_acCloseDisplayLabel = nullptr;
    QLabel *m_batteryCloseDisplayLabel = nullptr;
    QLabel *m_acCloseDisplayHintIcon = nullptr;
    QLabel *m_batteryCloseDisplayHintIcon = nullptr;
    LightLabel *m_acCloseDisplayHintLabel = nullptr;
    LightLabel *m_batteryCloseDisplayHintLabel = nullptr;
    QLabel *m_acSleepLabel = nullptr;
    QLabel *m_batterySleepLabel = nullptr;

    QLabel *m_acHibernateLabel = nullptr;
    QLabel *m_batteryHibernateLabel = nullptr;

    QLabel *mCloseLidLabel;
    QLabel *mHostLightLabel;
    QLabel *mResourceLabel;
    QLabel *m_remainingTimeLabel = nullptr;
    QLabel *m_acPowerLabel = nullptr;
    QLabel *m_batteryPowerLabel = nullptr;
    QLabel *mDarkenLabel;
    QLabel *mLowpowerLabel1;
    QLabel *mLowpowerLabel2;
    QLabel *mNoticeLabel;
    QLabel *mLowSaveLabel;
    QLabel *mBatterySaveLabel;

    LightLabel *m_resourceDetailLabel = nullptr;
    LightLabel *m_remainingTimeDetailLabel = nullptr;

    QFrame *Powerwidget;
    QFrame *BlankSleepwidget;
    QFrame *PowerPlanwidget;
    QFrame *Batterywidget;
    QFrame *mSleepPwdFrame;
    QFrame *mWakenPwdFrame;
    QFrame *mPowerKeyFrame;
    QFrame *m_acCloseDisplayFrame = nullptr;
    QFrame *m_batteryCloseDisplayFrame = nullptr;
    QFrame *m_acSleepFrame = nullptr;
    QFrame *m_batterySleepFrame = nullptr;
    QFrame *m_acHibernateFrame = nullptr;
    QFrame *m_batteryHibernateFrame = nullptr;
    QFrame *mCloseLidFrame;
    QFrame *mHostLightFrame;
    QFrame *mResourceFrame;
    QFrame *m_remainingTimeFrame = nullptr;
    QFrame *m_acPowerSettingsFrame = nullptr;
    QFrame *m_batteryPowerSettingsFrame = nullptr;
    QFrame *mBatteryPlanFrame;
    QFrame *m_acPowerFrame = nullptr;
    QFrame *m_batteryPowerFrame = nullptr;
    QFrame *mDarkenFrame;
    QFrame *mLowpowerFrame;
    QFrame *mNoticeLFrame;
    QFrame *mLowSaveFrame;
    QFrame *mBatterySaveFrame;

    QFrame* createAcSectionWidget(QWidget *parent);
    QFrame* createBatterySectionWidget(QWidget *parent);

    KHLineFrame *line_1;
    KHLineFrame *line_2;
    KHLineFrame *line_3;
    KHLineFrame *m_acCloseDisplayLine;
    KHLineFrame *m_acSleepLine;

    KHLineFrame *m_acLineHibernate = nullptr;
    KHLineFrame *m_batteryLineHibernate = nullptr;

    KHLineFrame *line_8;
    KHLineFrame *line_9;
    KHLineFrame *line_10;
    KHLineFrame *line_11;
    KHLineFrame *line_12;
    KHLineFrame *m_batteryCloseDisplayLine;
    KHLineFrame *m_batterySleepLine;

    QSpacerItem *mItem;

    QComboBox *mPowerKeyComboBox;
    QComboBox *m_acCloseDisplayComboBox = nullptr;
    QComboBox *m_batteryCloseDisplayComboBox = nullptr;
    QComboBox *m_acSleepComboBox = nullptr;
    QComboBox *m_batterySleepComboBox = nullptr;
    QComboBox *m_acHibernateComboBox = nullptr;
    QComboBox *m_batteryHibernateComboBox = nullptr;
    QComboBox *mCloseLidComboBox;
    QComboBox *m_acPowerComboBox = nullptr;
    QComboBox *m_batteryPowerComboBox = nullptr;
    QComboBox *mDarkenComboBox;
    QComboBox *m_lowpowerComboBox1;
    QComboBox *mLowpowerComboBox2;
    QComboBox *mNoticeComboBox;

    KSwitchButton *mSleepPwdBtn;
    KSwitchButton *mResourceSButton;
    KSwitchButton *m_remainingTimeSwitch = nullptr;
    KSwitchButton *mWakenPwdBtn;
    KSwitchButton *mHostLightSButton;
    KSwitchButton *mLowSaveBtn;
    KSwitchButton *mBatterySaveBtn;

    QButtonGroup *mPowerBtnGroup;

    QSpacerItem *verticalSpacer;
    QSpacerItem *verticalSpacer_1;
    QSpacerItem *verticalSpacer_2;

    QStringList buttonStringList;
    QStringList sleepStringList;
    QStringList sleepStringList_battery;
    QStringList closeStringList;
    QStringList closeStringList_battery;
    QStringList acHibernateStringList;
    QStringList batteryHibernateStringList;
    
    QStringList closeLidStringList;
    QStringList PowerplanStringList;
    QStringList BatteryplanStringList;
    QStringList DarkenStringList;
    QStringList LowpowerStringList;

    QStringList mKeys;

    QVBoxLayout *BatteryLayout;
    QVBoxLayout *PowerLayout;
    QVBoxLayout *m_acPowerSettingsLayout = nullptr;
    QVBoxLayout *m_batteryPowerSettingsLayout = nullptr;
    QVBoxLayout *mverticalLayout;

    QDBusInterface *mHibernateInterface = nullptr;

    bool mFirstLoad;
    bool mIsExistLid;
    bool m_isExistHibernate; // 休眠
    bool m_isExistSuspend;   // 睡眠
    bool mHasBat;
    bool m_isExistS2H;
private slots:
    void setVisibleBySecurity();
    void setEnableBySecurity();
    void setWidgetsEnabledByGSettings();

private:
    bool kconf2Enable(const QString &cloudPlatForm) const;
    bool isGSettingsKeyWritable(const QString &schema, const QString &key);
    void updateWidgetEnabledByKey(const QString &schema, const QString &key);
    QWidget* findWidgetByKey(const QString &schema, const QString &key);
    void initKeyWidgetMapping();
    static QString unqtify_name(const QString &name);
    
    // 统一的键-控件映射表结构
    struct KeyWidgetMapping {
        QString schema;
        QString key;
        QWidget *widget;  // 直接存储控件指针
    };
    QList<KeyWidgetMapping> m_keyWidgetMappings;
};

#endif // POWER_H
