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
#include "power.h"
#include "powermacrodata.h"

#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QSettings>
#include <QFormLayout>
#include <QFile>

#include "../../../shell/utils/utils.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include "libupower-glib/upower.h"

typedef enum {
    BALANCE,
    SAVING,
    CUSTDOM
}MODE;

#define DISPLAY_BALANCE_BA 5 * 60
#define DISPLAY_BALANCE_AC 10 * 60
#define COMPUTER_BALANCE_BA 5 * 60
#define COMPUTER_BALANCE_AC 10 * 60
#define BRIGHTNESS_BALANCE 100
#define DISPLAY_SAVING 60
#define COMPUTER_SAVING 2 * 60
#define BRIGHTNESS_SAVING 20

Power::Power() : mFirstLoad(true)
{
    pluginName = tr("Power");
    pluginType = SYSTEM;
}

Power::~Power() {
    if (!mFirstLoad) {

    }
}

QString Power::plugini18nName() {
    return pluginName;
}

int Power::pluginTypes() {
    return pluginType;
}

QWidget * Power::pluginUi() {
    if (mFirstLoad) {
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        const QByteArray styleID(STYLE_FONT_SCHEMA);
        const QByteArray id(POWERMANAGER_SCHEMA);
        const QByteArray iid(SESSION_SCHEMA);
        const QByteArray iiid(SCREENSAVER_SCHEMA);
        const QByteArray IVd(PERSONALSIE_SCHEMA);
        const QByteArray Vd(UKUI_QUICK_OPERATION_PANEL);

        if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(styleID) && QGSettings::isSchemaInstalled(iid)
                && QGSettings::isSchemaInstalled(iiid) && QGSettings::isSchemaInstalled(IVd) ) {
            settings = new QGSettings(id, QByteArray(), this);
            stylesettings = new QGSettings(styleID, QByteArray(), this);
            sessionsettings = new QGSettings(iid, QByteArray(), this);
            screensettings = new QGSettings(iiid, QByteArray(), this);
            m_centerSettings = new QGSettings(IVd,QByteArray(), this);
            if (QGSettings::isSchemaInstalled(Vd) && Utils::isTablet()) {
                m_qsettings = new QGSettings(Vd,QByteArray(), this);
            } else {
                m_qsettings = nullptr;
            }


            connect(stylesettings,&QGSettings::changed,[=](QString key)
            {
                if("systemFont" == key || "systemFontSize" == key)
                {
                    retranslateUi();
                }
            });
            mKeys = settings->keys();

            InitUI(pluginWidget);
            initSearText();
            isLidPresent();
            isHibernateSupply();
            isExitBattery();
            setupComponent();
            initCustomPlanStatus();
            setupConnect();
            resetui();
        }
    }

    return pluginWidget;
}

const QString Power::name() const {

    return QStringLiteral("Power");
}

bool Power::isShowOnHomePage() const
{
    return true;
}

QIcon Power::icon() const
{
    return QIcon::fromTheme("system-shutdown-symbolic");
}

bool Power::isEnable() const
{
    return !(QString(QLatin1String(kdk_system_get_hostCloudPlatform())) == "huawei" ||
             QString(QLatin1String(kdk_system_get_hostVirtType())) != "none");
}

void Power::InitUI(QWidget *widget)
{
    // 整体布局
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    CustomTitleLabel = new TitleLabel(widget);

    mverticalLayout->addWidget(CustomTitleLabel);

    // 通用设置布局
    Powerwidget = new QFrame(widget);
    Powerwidget->setMinimumSize(QSize(550, 0));
    Powerwidget->setMaximumSize(QSize(16777215, 16777215));
    Powerwidget->setFrameShape(QFrame::Box);

    PowerLayout = new QVBoxLayout(Powerwidget);
    PowerLayout->setContentsMargins(0, 0, 0, 0);
    PowerLayout->setSpacing(0);

    mSleepPwdFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mSleepPwdFrame);

    QHBoxLayout *mSleepPwdLayout = new QHBoxLayout(mSleepPwdFrame);
    mSleepPwdLayout->setContentsMargins(16, 0, 16, 0);

    mSleepPwdLabel = new QLabel(mSleepPwdFrame);
    mSleepPwdLabel->setFixedSize(550,60);

    mSleepPwdBtn = new KSwitchButton(mSleepPwdFrame);
    mSleepPwdBtn->setObjectName("sleepwaken");

    mSleepPwdLayout->addWidget(mSleepPwdLabel);
    mSleepPwdLayout->addStretch();
    mSleepPwdLayout->addWidget(mSleepPwdBtn);

    PowerLayout->addWidget(mSleepPwdFrame);

    line_1 = setLine(Powerwidget);
    PowerLayout->addWidget(line_1);

    mWakenPwdFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mWakenPwdFrame);

    QHBoxLayout *mWakenPwdLayout = new QHBoxLayout(mWakenPwdFrame);
    mWakenPwdLayout->setContentsMargins(16, 0, 16, 0);

    mWakenPwdLabel = new QLabel(mWakenPwdFrame);
    mWakenPwdLabel->setFixedSize(550,49);

    mWakenPwdBtn = new KSwitchButton(mWakenPwdFrame);
    mWakenPwdBtn->setObjectName("lockwaken");

    mWakenPwdLayout->addWidget(mWakenPwdLabel);
    mWakenPwdLayout->addStretch();
    mWakenPwdLayout->addWidget(mWakenPwdBtn);

    PowerLayout->addWidget(mWakenPwdFrame);

    line_2 = setLine(Powerwidget);
    PowerLayout->addWidget(line_2);

    mPowerKeyFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mSleepPwdFrame);


    QHBoxLayout *mPowerKeyLayout = new QHBoxLayout(mPowerKeyFrame);
    mPowerKeyLayout->setContentsMargins(16, 0, 16, 0);

    mPowerKeyLabel = new QLabel(mPowerKeyFrame);
    mPowerKeyLabel->setFixedSize(550,60);

    mPowerKeyComboBox = new QComboBox(mPowerKeyFrame);
    mPowerKeyComboBox->setObjectName("powerkey");
    mPowerKeyComboBox->setMinimumWidth(200);

    mPowerKeyLayout->addWidget(mPowerKeyLabel);
    mPowerKeyLayout->addWidget(mPowerKeyComboBox);

    PowerLayout->addWidget(mPowerKeyFrame);

    line_3 = setLine(Powerwidget);
    PowerLayout->addWidget(line_3);

    mCloseFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mCloseFrame);

    QHBoxLayout *mCloseLayout = new QHBoxLayout(mCloseFrame);
    mCloseLayout->setContentsMargins(16, 0, 16, 0);

    mCloseLabel = new QLabel(mCloseFrame);
    mCloseLabel->setFixedSize(550,60);

    mCloseComboBox = new QComboBox(mCloseFrame);
    mCloseComboBox->setObjectName("close");
    mCloseComboBox->setMinimumWidth(200);

    mCloseLayout->addWidget(mCloseLabel);
    mCloseLayout->addWidget(mCloseComboBox);

    PowerLayout->addWidget(mCloseFrame);

    line_4 = setLine(Powerwidget);
    PowerLayout->addWidget(line_4);

    mSleepFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mSleepPwdFrame);

    QHBoxLayout *mSleepLayout = new QHBoxLayout(mSleepFrame);
    mSleepLayout->setContentsMargins(16, 0, 16, 0);

    mSleepLabel = new QLabel(mSleepFrame);
    mSleepLabel->setFixedSize(550,59);

    mSleepComboBox = new QComboBox(mSleepFrame);
    mSleepComboBox->setObjectName("sleep");
    mSleepComboBox->setMinimumWidth(200);

    mSleepLayout->addWidget(mSleepLabel);
    mSleepLayout->addWidget(mSleepComboBox);

    PowerLayout->addWidget(mSleepFrame);

    line_5 = setLine(Powerwidget);
    PowerLayout->addWidget(line_5);

    mCloseLidFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mCloseLidFrame);


    QHBoxLayout *mCloseLidLayout = new QHBoxLayout(mCloseLidFrame);
    mCloseLidLayout->setContentsMargins(16, 0, 16, 0);

    mCloseLidLabel = new QLabel(mCloseLidFrame);
    mCloseLidLabel->setFixedSize(550,59);

    mCloseLidComboBox = new QComboBox(mCloseLidFrame);
    mCloseLidComboBox->setObjectName("closelid");
    mCloseLidComboBox->setMinimumWidth(200);

    mCloseLidLayout->addWidget(mCloseLidLabel);
    mCloseLidLayout->addWidget(mCloseLidComboBox);

    PowerLayout->addWidget(mCloseLidFrame);

    mverticalLayout->addWidget(Powerwidget);
    mItem = new QSpacerItem(20, 24, QSizePolicy::Fixed);
    mverticalLayout->addSpacerItem(mItem);

    PowerPlanTitleLabel = new TitleLabel(widget);
    mverticalLayout->addWidget(PowerPlanTitleLabel);

    PowerPlanwidget = new QFrame(widget);
    PowerPlanwidget->setMinimumSize(QSize(550, 0));
    PowerPlanwidget->setMaximumSize(QSize(16777215, 16777215));
    PowerPlanwidget->setFrameShape(QFrame::Box);

    // 电源计划布局
    QVBoxLayout *PowerPlanLayout = new QVBoxLayout(PowerPlanwidget);
    PowerPlanLayout->setContentsMargins(0, 0, 0, 0);
    PowerPlanLayout->setSpacing(0);

    mPowerFrame = new QFrame(PowerPlanwidget);
    setFrame_Noframe(mPowerFrame);

    QHBoxLayout *mPowerLayout = new QHBoxLayout(mPowerFrame);
    mPowerLayout->setContentsMargins(16, 0, 16, 0);


    mPowerLabel = new QLabel(mPowerFrame);
    mPowerLabel->setFixedSize(550,60);

    mPowerComboBox = new QComboBox(mPowerFrame);
    mPowerComboBox->setObjectName("powermode");
    mPowerComboBox->setMinimumWidth(200);

    mPowerLayout->addWidget(mPowerLabel);
    mPowerLayout->addWidget(mPowerComboBox);

    PowerPlanLayout->addWidget(mPowerFrame);

    line_7 = setLine(PowerPlanwidget);
    PowerPlanLayout->addWidget(line_7);

    mBatteryFrame = new QFrame(PowerPlanwidget);
    setFrame_Noframe(mBatteryFrame);

    QHBoxLayout *mBatteryLayout = new QHBoxLayout(mBatteryFrame);
    mBatteryLayout->setContentsMargins(16, 0, 16, 0);

    mBatteryLabel = new QLabel(mBatteryFrame);
    mBatteryLabel->setFixedSize(550,59);

    mBatteryComboBox = new QComboBox(mBatteryFrame);
    mBatteryComboBox->setObjectName("batterymode");
    mBatteryComboBox->setMinimumWidth(200);

    mBatteryLayout->addWidget(mBatteryLabel);
    mBatteryLayout->addWidget(mBatteryComboBox);

    PowerPlanLayout->addWidget(mBatteryFrame);

    mverticalLayout->addWidget(PowerPlanwidget);
    mverticalLayout->addSpacing(24);

    BatteryPlanTitleLabel = new TitleLabel(Powerwidget);
    mverticalLayout->addWidget(BatteryPlanTitleLabel);

    // 电池节能计划
    Batterywidget = new QFrame(widget);
    Batterywidget->setMinimumSize(QSize(550, 0));
    Batterywidget->setMaximumSize(QSize(16777215, 16777215));
    Batterywidget->setFrameShape(QFrame::Box);


    BatteryLayout = new QVBoxLayout(Batterywidget);
    BatteryLayout->setContentsMargins(0, 0, 0, 0);
    BatteryLayout->setSpacing(0);

    mDarkenFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mDarkenFrame);

    QHBoxLayout *mDarkenLayout = new QHBoxLayout(mDarkenFrame);
    mDarkenLayout->setContentsMargins(16, 0, 16, 0);

    mDarkenLabel = new QLabel(mDarkenFrame);
    mDarkenLabel->setFixedSize(550,59);

    mDarkenComboBox = new QComboBox(mDarkenFrame);
    mDarkenComboBox->setObjectName("darken");
    mDarkenComboBox->setMinimumWidth(200);

    mDarkenLayout->addWidget(mDarkenLabel);
    mDarkenLayout->addWidget(mDarkenComboBox);

    BatteryLayout->addWidget(mDarkenFrame);

    line_8 = setLine(Batterywidget);
    BatteryLayout->addWidget(line_8);

    mLowpowerFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mLowpowerFrame);


    mLowpowerLabel1 = new QLabel(mLowpowerFrame);
    mLowpowerLabel1->setFixedSize(84,60);
    mLowpowerLabel2 = new QLabel(mLowpowerFrame);
    mLowpowerLabel2->setFixedSize(370,60);

    QHBoxLayout *mLowpowerLayout = new QHBoxLayout(mLowpowerFrame);
    mLowpowerLayout->setContentsMargins(16, 0, 16, 0);

    mLowpowerComboBox1 = new QComboBox(mLowpowerFrame);
    mLowpowerComboBox1->setObjectName("lowbatterylimit");
    mLowpowerComboBox1->setFixedWidth(80);
    mLowpowerComboBox2 = new QComboBox(mLowpowerFrame);
    mLowpowerComboBox2->setObjectName("lowbatterysettings");
    mLowpowerComboBox2->setMinimumWidth(200);

    mLowpowerLayout->setSpacing(8);
    mLowpowerLayout->addWidget(mLowpowerLabel1);
    mLowpowerLayout->addWidget(mLowpowerComboBox1);
    mLowpowerLayout->addWidget(mLowpowerLabel2);
    mLowpowerLayout->addWidget(mLowpowerComboBox2);

    BatteryLayout->addWidget(mLowpowerFrame);

    line_9 = setLine(Batterywidget);
    BatteryLayout->addWidget(line_9);

    mNoticeLFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mNoticeLFrame);


    QHBoxLayout *mNoticeLayout = new QHBoxLayout(mNoticeLFrame);
    mNoticeLayout->setContentsMargins(16, 0, 16, 0);

    mNoticeLabel = new QLabel(mNoticeLFrame);
    mNoticeLabel->setFixedSize(550,59);

    mNoticeComboBox = new QComboBox(mNoticeLFrame);
    mNoticeComboBox->setObjectName("noticesettings");
    mNoticeComboBox->setMinimumWidth(200);

    mNoticeLayout->addWidget(mNoticeLabel);
    mNoticeLayout->addWidget(mNoticeComboBox);

    BatteryLayout->addWidget(mNoticeLFrame);

    line_10 = setLine(Batterywidget);
    BatteryLayout->addWidget(line_10);

    mLowSaveFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mLowSaveFrame);

    QHBoxLayout *mLowSaveLayout = new QHBoxLayout(mLowSaveFrame);
    mLowSaveLayout->setContentsMargins(16, 0, 16, 0);

    mLowSaveLabel = new QLabel(mLowSaveFrame);
    mLowSaveLabel->setFixedSize(550,59);

    mLowSaveBtn = new KSwitchButton(mLowSaveFrame);
    mLowSaveBtn->setObjectName("lowbatterysave");

    mLowSaveLayout->addWidget(mLowSaveLabel);
    mLowSaveLayout->addStretch();
    mLowSaveLayout->addWidget(mLowSaveBtn);

    BatteryLayout->addWidget(mLowSaveFrame);

    line_11 = setLine(Batterywidget);
    BatteryLayout->addWidget(line_11);

    mBatterySaveFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mBatterySaveFrame);

    QHBoxLayout *mBatterySaveLayout = new QHBoxLayout(mBatterySaveFrame);
    mBatterySaveLayout->setContentsMargins(16, 0, 16, 0);

    mBatterySaveLabel = new QLabel(mBatterySaveFrame);
    mBatterySaveLabel->setFixedSize(550,59);

    mBatterySaveBtn = new KSwitchButton(mBatterySaveFrame);
    mBatterySaveBtn->setObjectName("batterysave");

    mBatterySaveLayout->addWidget(mBatterySaveLabel);
    mBatterySaveLayout->addStretch();
    mBatterySaveLayout->addWidget(mBatterySaveBtn);

    BatteryLayout->addWidget(mBatterySaveFrame);

    line_12 = setLine(Batterywidget);
    BatteryLayout->addWidget(line_12);

    mDisplayTimeFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mDisplayTimeFrame);

    QHBoxLayout *mDisplayTimeLayout = new QHBoxLayout(mDisplayTimeFrame);
    mDisplayTimeLayout->setContentsMargins(16, 0, 16, 0);

    mDisplayTimeLabel = new QLabel(mDisplayTimeFrame);
    mDisplayTimeLabel->setFixedSize(550,59);

    mDisplayTimeBtn = new KSwitchButton(mDisplayTimeFrame);
    mDisplayTimeBtn->setObjectName("powershowtime");

    mDisplayTimeLayout->addWidget(mDisplayTimeLabel);
    mDisplayTimeLayout->addStretch();
    mDisplayTimeLayout->addWidget(mDisplayTimeBtn);

    BatteryLayout->addWidget(mDisplayTimeFrame);

    mverticalLayout->addWidget(Batterywidget);
    mverticalLayout->addStretch();

    retranslateUi();
}

void Power::retranslateUi()
{
    if (QLabelSetText(mSleepPwdLabel, tr("Require password when sleep"))) {
        mSleepPwdLabel->setToolTip(tr("Require password when sleep"));
    }

    if (QLabelSetText(mWakenPwdLabel, tr("Password required when waking up the screen"))) {
        mSleepPwdLabel->setToolTip(tr("Password required when waking up the screen"));
    }

    if (QLabelSetText(mPowerKeyLabel, tr("Press the power button"))) {
        mPowerKeyLabel->setToolTip("Press the power button");
    }

    if (QLabelSetText(mCloseLabel, tr("Time to close display"))) {
        mCloseLabel->setToolTip(tr("Time to close display"));
    }

    if (QLabelSetText(mSleepLabel, tr("Time to sleep"))) {
        mSleepLabel->setToolTip(tr("Time to sleep"));
    }

    if (QLabelSetText(mCloseLidLabel, tr("Notebook cover"))) {
        mCloseLidLabel->setToolTip(tr("Notebook cover"));
    }

    if (QLabelSetText(mPowerLabel, tr("Using power"))) {
        mPowerLabel->setToolTip(tr("Using power"));
    }

    if (QLabelSetText(mBatteryLabel, tr("Using battery"))) {
        mBatteryLabel->setToolTip(tr("Using power"));
    }

    if (QLabelSetText(mDarkenLabel, tr(" Time to darken"))) {
        mDarkenLabel->setToolTip(tr(" Time to darken"));
    }

    if (QLabelSetText(mLowpowerLabel1, tr("Battery level is lower than"))) {
        mLowpowerLabel1->setToolTip(tr("Battery level is lower than"));
    }

    mLowpowerLabel2->setText(tr("Run"));

    if (QLabelSetText(mNoticeLabel, tr("Low battery notification"))) {
        mNoticeLabel->setToolTip(tr("Low battery notification"));
    }

    if (QLabelSetText(mLowSaveLabel, tr("Automatically run saving mode when low battery"))) {
        mLowSaveLabel->setToolTip(tr("Automatically run saving mode when the low battery"));
    }

    if (QLabelSetText(mBatterySaveLabel, tr("Automatically run saving mode when using battery"))) {
        mBatterySaveLabel->setToolTip(tr("Automatically run saving mode when using battery"));
    }

    if (QLabelSetText(mDisplayTimeLabel, tr("Display remaining charging time and usage time"))) {
        mDisplayTimeLabel->setToolTip(tr("Display remaining charging time and usage time"));
    }
}

void Power::resetui()
{
    //990隐藏这些设置项
    if (Utils::isWayland()) {
        line_9->hide();
        mNoticeLFrame->hide();
        line_10->hide();
        mLowSaveFrame->hide();
        line_11->hide();
        mBatterySaveFrame->hide();
        line_12->hide();
        mDisplayTimeFrame->hide();
    }

    mBatterySaveFrame->hide();
    mDarkenFrame->hide();
    mDisplayTimeFrame->hide();
    line_8->hide();
    line_11->hide();
    line_12->hide();
    //不存在盖子隐藏该项
    if (!isExitsLid) {
        mCloseLidFrame->hide();
        line_5->hide();
    }

    //不存在电池隐藏这些设置项
    if (!hasBat) {
        line_7->hide();
        mBatteryFrame->hide();
        BatteryPlanTitleLabel->hide();
        clearAutoItem(BatteryLayout);
        Batterywidget->hide();
    }

    //Intel作如下处理
    if (Utils::isTablet()) {
        mSleepPwdFrame->hide();
        mWakenPwdFrame->hide();
        mPowerKeyFrame->hide();
        mCloseLidFrame->hide();
        mDarkenFrame->hide();
        mLowpowerFrame->hide();
        mNoticeLFrame->hide();
        mDisplayTimeFrame->hide();
        mItem->changeSize(0, 0);
        line_1->hide();
        line_2->hide();
        line_3->hide();
        line_5->hide();
        line_8->hide();
        line_9->hide();
        line_10->hide();
        line_11->hide();
        line_12->hide();
    }
}

void Power::initSearText()
{
    //~ contents_path /Power/General
    CustomTitleLabel->setText(tr("General"));
    //~ contents_path /Power/Select Powerplan
    PowerPlanTitleLabel->setText(tr("Select Powerplan"));
    //~ contents_path /Power/Battery saving plan
    BatteryPlanTitleLabel->setText((tr("Battery saving plan")));
}

void Power::setupComponent()
{
    // 合盖
    closeLidStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
    mCloseLidComboBox->insertItem(0, closeLidStringList.at(0), "nothing");
    mCloseLidComboBox->insertItem(1, closeLidStringList.at(1), "blank");
    mCloseLidComboBox->insertItem(2, closeLidStringList.at(2), "suspend");
    mCloseLidComboBox->insertItem(3, closeLidStringList.at(3), "shutdown");
    if (!Utils::isWayland() && isExitHibernate){
        closeLidStringList << tr("hibernate");
        mCloseLidComboBox->insertItem(4, closeLidStringList.at(4), "hibernate");
     }

    //按下电源键时
    buttonStringList << tr("interactive") << tr("suspend") << tr("shutdown") << tr("hibernate");
    mPowerKeyComboBox->insertItem(0, buttonStringList.at(0), "interactive");
    mPowerKeyComboBox->insertItem(1, buttonStringList.at(1), "suspend");
    mPowerKeyComboBox->insertItem(2, buttonStringList.at(2), "shutdown");
    if (isExitHibernate) {
        mPowerKeyComboBox->insertItem(3, buttonStringList.at(3), "hibernate");
    }

    //关闭显示器
    closeStringList << tr("5min") << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("never");
    mCloseComboBox->insertItem(0, closeStringList.at(0), QVariant::fromValue(5));
    mCloseComboBox->insertItem(1, closeStringList.at(1), QVariant::fromValue(10));
    mCloseComboBox->insertItem(2, closeStringList.at(2), QVariant::fromValue(15));
    mCloseComboBox->insertItem(3, closeStringList.at(3), QVariant::fromValue(30));
    mCloseComboBox->insertItem(4, closeStringList.at(4), QVariant::fromValue(60));
    mCloseComboBox->insertItem(5, closeStringList.at(5), QVariant::fromValue(120));
    mCloseComboBox->insertItem(6, closeStringList.at(6), QVariant::fromValue(0));

    //睡眠
    sleepStringList << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("3h") << tr("never");
    mSleepComboBox->insertItem(0, sleepStringList.at(0), QVariant::fromValue(10));
    mSleepComboBox->insertItem(1, sleepStringList.at(1), QVariant::fromValue(15));
    mSleepComboBox->insertItem(2, sleepStringList.at(2), QVariant::fromValue(30));
    mSleepComboBox->insertItem(3, sleepStringList.at(3), QVariant::fromValue(60));
    mSleepComboBox->insertItem(4, sleepStringList.at(4), QVariant::fromValue(120));
    mSleepComboBox->insertItem(5, sleepStringList.at(5), QVariant::fromValue(180));
    mSleepComboBox->insertItem(6, sleepStringList.at(6), QVariant::fromValue(0));

    //电源计划
    PowerplanStringList << tr("Balance Model") << tr("Save Model")<<tr("Performance Model");
    mPowerComboBox->insertItem(0, PowerplanStringList.at(0), "Balance Model");
    mPowerComboBox->insertItem(1, PowerplanStringList.at(1), "Save Model");
    mPowerComboBox->insertItem(2, PowerplanStringList.at(2), "Performance Model");

    BatteryplanStringList << tr("Balance Model") << tr("Save Model")<<tr("Performance Model");
    mBatteryComboBox->insertItem(0, BatteryplanStringList.at(0), "Balance Model");
    mBatteryComboBox->insertItem(1, BatteryplanStringList.at(1), "Save Model");
     mBatteryComboBox->insertItem(2, BatteryplanStringList.at(2), "Performance Model");

    //低电量时执行
    LowpowerStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
    mLowpowerComboBox2->insertItem(0, LowpowerStringList.at(0), "nothing");
    mLowpowerComboBox2->insertItem(1, LowpowerStringList.at(1), "blank");
    mLowpowerComboBox2->insertItem(2, LowpowerStringList.at(2), "suspend");
    mLowpowerComboBox2->insertItem(3, LowpowerStringList.at(3), "shutdown");
    if (isExitHibernate){
        LowpowerStringList << tr("hibernate");
        mLowpowerComboBox2->insertItem(4, LowpowerStringList.at(4), "hibernate");
     }

    //低电量通知
    for (int i = 1; i < 5; i++) {
        mNoticeComboBox->insertItem(i-1, QString("%1%").arg(i*10));
    }

    //电池低电量范围
    int batteryRemain = settings->get(PER_ACTION_CRI).toInt();
    for(int i = 5; i < batteryRemain; i++) {
        mLowpowerComboBox1->insertItem(i - 5, QString("%1%").arg(i));
    }
}

void Power::setupConnect()
{
    connect(mSleepPwdBtn,&KSwitchButton::stateChanged, [=](bool checked){
        Utils::buriedSettings(name(), "whether password is required for wake-up after sleep", QString("settings"), checked ? "true" : "false");
        screensettings->set(SLEEP_ACTIVATION_ENABLED,checked);
    });

    if (screensettings->keys().contains("closeActivationEnabled")) {
        connect(mWakenPwdBtn,&KSwitchButton::stateChanged, [=](bool checked){
            Utils::buriedSettings(name(), "whether password is required for wake-up screen", QString("settings"), checked ? "true" : "false");
            screensettings->set(CLOSE_ACTIVATION_ENABLED,checked);
        });
    }

    connect(mPowerKeyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Utils::buriedSettings(name(), "execute when pressing the power key", QString("settings"), mPowerKeyComboBox->itemData(index).toString());
        settings->set(BUTTON_POWER_KEY, mPowerKeyComboBox->itemData(index));
    });

    connect(mCloseComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Utils::buriedSettings(name(), "turn off the display after this time period", QString("settings"), mCloseComboBox->currentData().toString());
        if (mCloseComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(SLEEP_DISPLAY_AC_KEY, -1);
            qDebug()<<"sleep-display-ac : "<<settings->trySet(SLEEP_DISPLAY_AC_KEY, -1);
        } else {
            settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(mCloseComboBox->currentData(Qt::UserRole).toInt() * 60));
            qDebug()<<"sleep-display-ac : "<<settings->trySet(SLEEP_DISPLAY_AC_KEY, mCloseComboBox->currentData(Qt::UserRole).toInt() * 60);
        }
    });

    connect(mCloseComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        if (mCloseComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(SLEEP_DISPLAY_BATT_KEY, -1);
            qDebug()<<"sleep-display-battery : "<<settings->trySet(SLEEP_DISPLAY_BATT_KEY, -1);
        } else {
            settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(mCloseComboBox->currentData(Qt::UserRole).toInt() * 60));
            qDebug()<<"sleep-display-battery : "<<settings->trySet(SLEEP_DISPLAY_BATT_KEY, mCloseComboBox->currentData(Qt::UserRole).toInt() * 60);
        }
    });

    connect(mSleepComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Utils::buriedSettings(name(), "the system go to sleep after this time period", QString("settings"), mSleepComboBox->currentData().toString());
        if (mSleepComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(SLEEP_COMPUTER_AC_KEY, -1);
            qDebug()<<"sleep-computer-ac : "<<settings->trySet(SLEEP_COMPUTER_AC_KEY, -1);
        } else {
            settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(mSleepComboBox->currentData(Qt::UserRole).toInt() * 60));
            qDebug()<<"sleep-computer-ac : "<<settings->trySet(SLEEP_COMPUTER_AC_KEY, mSleepComboBox->currentData(Qt::UserRole).toInt() * 60);
        }
    });

    connect(mSleepComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        if (mSleepComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(SLEEP_COMPUTER_BATT_KEY, -1);
            qDebug()<<"sleep-computer-battery : "<<settings->trySet(SLEEP_COMPUTER_BATT_KEY, -1);
        } else {
            settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(mSleepComboBox->currentData(Qt::UserRole).toInt() * 60));
            qDebug()<<"sleep-computer-battery : "<<settings->trySet(SLEEP_COMPUTER_BATT_KEY, mSleepComboBox->currentData(Qt::UserRole).toInt() * 60);
        }
    });

    connect(mCloseLidComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Utils::buriedSettings(name(), "execute when the notebook is closed", QString("settings"), mCloseLidComboBox->currentData().toString());
        settings->set(BUTTON_LID_AC_KEY, mCloseLidComboBox->itemData(index));
        settings->set(BUTTON_LID_BATT_KET, mCloseLidComboBox->itemData(index));
    });

    if (mKeys.contains("powerPolicyAc") && mKeys.contains("powerPolicyBattery")) {
        connect(mPowerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            Utils::buriedSettings(name(), "mode when using power supply", QString("settings"), mPowerComboBox->currentData().toString());
            if (index == 0) {
                settings->set(POWER_POLICY_AC, 1);
            }  else if (index == 1) {
                settings->set(POWER_POLICY_AC, 2);
            } else {
                settings->set(POWER_POLICY_AC, 0);
            }
        });

        connect(mBatteryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            Utils::buriedSettings(name(), "mode when using battery supply", QString("settings"), mBatteryComboBox->currentData().toString());
            //当开启了 低电量自动开启节能模式 时，在此低电量范围内调整电池计划，则自动关闭 低电量自动开启节能模式
            if (!Utils::isWayland() && mKeys.contains("lowBatteryAutoSave")) {
                if (mLowSaveBtn->isChecked() &&  getBattery() <= settings->get(PERCENTAGE_LOW).toDouble()) {
                    mLowSaveBtn->setChecked(false);
                }
            }
            if (index == 0) {
                settings->set(POWER_POLICY_BATTARY, 1);
            } else if (index == 1) {
                settings->set(POWER_POLICY_BATTARY, 2);
            } else {
                settings->set(POWER_POLICY_BATTARY, 0);
            }
        });
    }

    connect(mDarkenComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        Utils::buriedSettings(name(), "reduce screen brightness after this time period", QString("settings"), mBatteryComboBox->currentData().toString());
        if (mDarkenComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(IDLE_DIM_TIME_KEY, -1);
        } else {
            settings->set(IDLE_DIM_TIME_KEY, QVariant(mDarkenComboBox->currentData(Qt::UserRole).toInt() * 60));
        }
    });

    connect(mLowpowerComboBox1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Utils::buriedSettings(name(), "low battery value", QString("settings"), mLowpowerComboBox1->currentData().toString());
        settings->set(PER_ACTION_KEY, index + 5);
    });

    connect(mLowpowerComboBox2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Utils::buriedSettings(name(), "execute below low battery level", QString("settings"), mLowpowerComboBox2->currentData().toString());
        settings->set(ACTION_CRI_BTY, mLowpowerComboBox2->itemData(index));
    });

    connect(mNoticeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Utils::buriedSettings(name(), "notify when the power is lower than this value", QString("settings"), mNoticeComboBox->currentData().toString());
        settings->set(PERCENTAGE_LOW, (index + 1)*10);
    });

    connect(mLowSaveBtn,&KSwitchButton::stateChanged, [=](bool checked){
        Utils::buriedSettings(name(), "whether the saving mode is autoturned on when the battery is low", QString("settings"), checked ? "true" : "false");
        settings->set(LOW_BATTERY_AUTO_SAVE,checked);
    });

    connect(settings, &QGSettings::changed, this, &Power::initCustomPlanStatus);

    connect(screensettings,&QGSettings::changed,[=](){
        initCustomPlanStatus();
    });

}

void Power::initCustomPlanStatus()
{
    // 信号阻塞
    mPowerKeyComboBox->blockSignals(true);
    mCloseComboBox->blockSignals(true);
    mSleepComboBox->blockSignals(true);
    mCloseLidComboBox->blockSignals(true);
    mPowerComboBox->blockSignals(true);
    mBatteryComboBox->blockSignals(true);
    mLowpowerComboBox1->blockSignals(true);
    mLowpowerComboBox2->blockSignals(true);
    mNoticeComboBox->blockSignals(true);
    mSleepPwdBtn->blockSignals(true);
    mWakenPwdBtn->blockSignals(true);
    mLowSaveBtn->blockSignals(true);
    mBatterySaveBtn->blockSignals(true);

    mPowerKeyComboBox->setCurrentIndex(mPowerKeyComboBox->findData(settings->get(BUTTON_POWER_KEY).toString()));
    if (settings->get(SLEEP_COMPUTER_AC_KEY).toInt() == -1) {
        mSleepComboBox->setCurrentIndex(mSleepComboBox->findData(0));
    } else {
          mSleepComboBox->setCurrentIndex(mSleepComboBox->findData(settings->get(SLEEP_COMPUTER_AC_KEY).toInt() / FIXES));
    }

    if (settings->get(SLEEP_DISPLAY_AC_KEY).toInt() == -1) {
        mCloseComboBox->setCurrentIndex(mCloseComboBox->findData(0));
    } else {
         mCloseComboBox->setCurrentIndex(mCloseComboBox->findData(settings->get(SLEEP_DISPLAY_AC_KEY).toInt() / FIXES));;
    }

    mCloseLidComboBox->setCurrentIndex(mCloseLidComboBox->findData(settings->get(BUTTON_LID_AC_KEY).toString()));

    if (mKeys.contains("powerPolicyAc") && mKeys.contains("powerPolicyBattery")) {
        if (1 == settings->get(POWER_POLICY_AC).toInt()) {
            mPowerComboBox->setCurrentIndex(0);
        } else if (2 == settings->get(POWER_POLICY_AC).toInt()){
            mPowerComboBox->setCurrentIndex(1);
        } else {
            mPowerComboBox->setCurrentIndex(2);
        }
        if (1 == settings->get(POWER_POLICY_BATTARY).toInt()) {
            mBatteryComboBox->setCurrentIndex(0);
        } else if (2 == settings->get(POWER_POLICY_BATTARY).toInt()) {
            mBatteryComboBox->setCurrentIndex(1);
        } else {
            mBatteryComboBox->setCurrentIndex(2);
        }
    } else {
        mPowerComboBox->setEnabled(false);
        mBatteryComboBox->setEnabled(false);
    }

    mLowpowerComboBox1->setCurrentIndex(settings->get(PER_ACTION_KEY).toInt() - 5);
    mLowpowerComboBox2->setCurrentIndex(mLowpowerComboBox2->findData(settings->get(ACTION_CRI_BTY).toString()));
    mNoticeComboBox->setCurrentIndex(settings->get(PERCENTAGE_LOW).toInt()/10 - 1);

    if (screensettings->keys().contains("sleepActivationEnabled")) {
        mSleepPwdBtn->setChecked(screensettings->get(SLEEP_ACTIVATION_ENABLED).toBool());
    }

    if (screensettings->keys().contains("closeActivationEnabled"))
        mWakenPwdBtn->setChecked(screensettings->get(CLOSE_ACTIVATION_ENABLED).toBool());

    if (mKeys.contains("lowBatteryAutoSave")) {
        mLowSaveBtn->setChecked(settings->get(LOW_BATTERY_AUTO_SAVE).toBool());
    } else {
        mLowSaveFrame->hide();
        line_10->hide();
    }

    // 信号阻塞解除
    mPowerKeyComboBox->blockSignals(false);
    mCloseComboBox->blockSignals(false);
    mSleepComboBox->blockSignals(false);
    mCloseLidComboBox->blockSignals(false);
    mPowerComboBox->blockSignals(false);
    mBatteryComboBox->blockSignals(false);
    mLowpowerComboBox1->blockSignals(false);
    mLowpowerComboBox2->blockSignals(false);
    mNoticeComboBox->blockSignals(false);
    mSleepPwdBtn->blockSignals(false);
    mWakenPwdBtn->blockSignals(false);
    mLowSaveBtn->blockSignals(false);
    mBatterySaveBtn->blockSignals(false);
}

void Power::isLidPresent()
{
    QDBusInterface *LidInterface = new QDBusInterface("org.freedesktop.UPower",
                       "/org/freedesktop/UPower",
                       "org.freedesktop.DBus.Properties",
                        QDBusConnection::systemBus(),this);


    if (!LidInterface->isValid()) {
        qDebug() << "Create UPower Lid Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return;
    }
    QDBusReply<QVariant> LidInfo;
    LidInfo = LidInterface->call("Get", "org.freedesktop.UPower", "LidIsPresent");
    isExitsLid = LidInfo.value().toBool();
}

void Power::isHibernateSupply()
{
    QDBusInterface *HibernateInterface = new QDBusInterface("org.freedesktop.login1",
                       "/org/freedesktop/login1",
                       "org.freedesktop.login1.Manager",
                        QDBusConnection::systemBus(),this);
    if (!HibernateInterface->isValid()) {
        qDebug() << "Create login1 Hibernate Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return;
    }
    QDBusReply<QString> HibernateInfo;
    HibernateInfo = HibernateInterface->call("CanHibernate");
    isExitHibernate = HibernateInfo == "yes"?true:false;
}

bool Power::isExitBattery()
{
    /* 默认机器没有电池 */
    hasBat = false;
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus(), this);
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");

    if (briginfo.value().toBool()) {
        hasBat = true ;
    }

    return hasBat;

}

double Power::getBattery()
{
    QDBusInterface *BatteryInterface = new QDBusInterface("org.freedesktop.UPower",
                       "/org/freedesktop/UPower/devices/battery_BAT0",
                       "org.freedesktop.DBus.Properties",
                        QDBusConnection::systemBus(),this);


    if (!BatteryInterface->isValid()) {
        qDebug() << "Create UPower Battery Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return 0;
    }
    QDBusReply<QVariant> BatteryInfo;
    BatteryInfo = BatteryInterface->call("Get", "org.freedesktop.UPower.Device", "Percentage");
    return BatteryInfo.value().toDouble();
}

bool Power::QLabelSetText(QLabel *label, QString string)
{
    bool is_over_length = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);
    QString str = string;
    if (fontSize > (label->width()-5)) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, label->width());
        is_over_length = true;
    }
    label->setText(str);
    return is_over_length;
}

void Power::clearAutoItem(QVBoxLayout *mLyt)
{
    if (mLyt->layout() != NULL) {
        QLayoutItem *item;
        while ((item = mLyt->layout()->takeAt(0)) != NULL)
        {
            if(item->widget()) {
               item->widget()->setParent(NULL);
            }
            delete item;
            item = nullptr;
        }
    }
}

void Power::setFrame_Noframe(QFrame *frame)
{
    frame->setMinimumSize(QSize(550, 60));
    frame->setMaximumSize(QSize(16777215, 60));
    frame->setFrameShape(QFrame::NoFrame);
}

QFrame *Power::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}
