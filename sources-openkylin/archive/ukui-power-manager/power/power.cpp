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

#include <QTranslator>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QSettings>
#include <QFormLayout>
#include <QFile>

#include "ukcccommon.h"
#include "dmi_chassis_type.h"
using namespace ukcc;

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <gio/gio.h>
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

int closeArray[7] = {5, 10, 15, 30, 60, 120, 0};

// 回退时在 combo 里逐项 findData；不支持的项会自动跳过
static const QStringList kCriticalBatteryActionFallback = {"hibernate", "suspend", "shutdown", "blank", "nothing"};
static const QStringList kLidActionFallback = {"suspend", "hibernate", "shutdown", "blank", "nothing"};
static const QStringList kPowerButtonActionFallback = {"interactive", "hibernate", "suspend", "shutdown"};

static int actionComboIndex(QComboBox *comboBox, const QString &storedAction,
                            const QStringList &fallbackOrder)
{
    const int storedIndex = comboBox->findData(storedAction);
    if (storedIndex >= 0) {
        return storedIndex;
    }

    for (const QString &action : fallbackOrder) {
        const int index = comboBox->findData(action);
        if (index >= 0) {
            return index;
        }
    }
    return comboBox->count() > 0 ? 0 : -1;
}

static void syncInvalidActionGsettings(QGSettings *settings, const char *key,
                                       QComboBox *comboBox,
                                       const QStringList &fallbackOrder)
{
    const QString storedAction = settings->get(key).toString();
    const int resolvedIndex = actionComboIndex(comboBox, storedAction, fallbackOrder);
    if (resolvedIndex >= 0
        && comboBox->itemData(resolvedIndex).toString() != storedAction) {
        settings->set(key, comboBox->itemData(resolvedIndex));
    }
}

Power::Power() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/ukui-power-manager/power/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("Power");
    pluginType = SYSTEM;
    initInterface();
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
        const QByteArray procID(PROC_MANAGER);
        if (QGSettings::isSchemaInstalled(procID)) {
            mProcManagerSettings = new QGSettings(procID, QByteArray(), this);
        }

        if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(styleID) && QGSettings::isSchemaInstalled(iid)
                && QGSettings::isSchemaInstalled(iiid) && QGSettings::isSchemaInstalled(IVd) ) {
            settings = new QGSettings(id, QByteArray(), this);
            stylesettings = new QGSettings(styleID, QByteArray(), this);
            sessionsettings = new QGSettings(iid, QByteArray(), this);
            screensettings = new QGSettings(iiid, QByteArray(), this);
            m_centerSettings = new QGSettings(IVd,QByteArray(), this);
            if (QGSettings::isSchemaInstalled(Vd) && UkccCommon::isTablet()) {
                m_qsettings = new QGSettings(Vd,QByteArray(), this);
            } else {
                m_qsettings = nullptr;
            }

            connect(stylesettings,&QGSettings::changed,[=](QString key) {
                if("systemFont" == key || "systemFontSize" == key) {
                    retranslateUi();
                }
            });
            mKeys = settings->keys();

            hostLightStripDbus =  new QDBusInterface("com.control.center.qt.systemdbus",
                                                    "/",
                                                    "com.control.center.interface",
                                                    QDBusConnection::systemBus(), this);
            if (!hostLightStripDbus->isValid()){
                qCritical() << "Create Client Interface Failed:" << QDBusConnection::systemBus().lastError();
            } else {
                QDBusReply<bool> ret = hostLightStripDbus->call("isSupportHostLightStrip");
                isSupportHostLight = ret;
            }

            isExitBattery();
            initUI(pluginWidget);
            isLidPresent();
            setupComponent();
            initCustomPlanStatus();
            setupConnect();
            resetui();

            hideComponent();
            hideComponentByConf2();
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
    return true;
}

void Power::initUI(QWidget *widget)
{
    // 整体布局
    mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);
    m_customTitleLabel = new KLabel(widget);
    m_customTitleLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Power/General
    m_customTitleLabel->setText(tr("General"));
    mverticalLayout->addWidget(m_customTitleLabel);

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
    QSize size1 = mSleepPwdBtn->size();
    qDebug() << "mSleepPwdBtn size:" << size1.width() << "x" << size1.height();
    mSleepPwdBtn->setObjectName("sleepwaken");
    mSleepPwdBtn->setAccessibleName("sleepwaken");

    mSleepPwdLayout->addWidget(mSleepPwdLabel);
    mSleepPwdLayout->addStretch();
    mSleepPwdLayout->addWidget(mSleepPwdBtn);
    PowerLayout->addWidget(mSleepPwdFrame);

    line_1 = new KHLineFrame(Powerwidget);
    PowerLayout->addWidget(line_1);


    mWakenPwdFrame = new QFrame(Powerwidget);
    setFrame_Noframe(mWakenPwdFrame);

    QHBoxLayout *mWakenPwdLayout = new QHBoxLayout(mWakenPwdFrame);
    mWakenPwdLayout->setContentsMargins(16, 0, 16, 0);

    mWakenPwdLabel = new QLabel(mWakenPwdFrame);
    mWakenPwdLabel->setFixedSize(550,49);///???????????

    mWakenPwdBtn = new KSwitchButton(mWakenPwdFrame);
    QSize size = mWakenPwdBtn->size();
    qDebug() << "mWakenPwdBtn size:" << size;

    mWakenPwdBtn->setObjectName("lockwaken");
    mWakenPwdBtn->setAccessibleName("lockwaken");

    mWakenPwdLayout->addWidget(mWakenPwdLabel);
    mWakenPwdLayout->addStretch();
    mWakenPwdLayout->addWidget(mWakenPwdBtn);
    PowerLayout->addWidget(mWakenPwdFrame);

    line_2 = new KHLineFrame(Powerwidget);
    PowerLayout->addWidget(line_2);

    mPowerKeyFrame = new QFrame(Powerwidget);
    // setFrame_Noframe(mSleepPwdFrame);
    setFrame_Noframe(mPowerKeyFrame);



    QHBoxLayout *mPowerKeyLayout = new QHBoxLayout(mPowerKeyFrame);
    mPowerKeyLayout->setContentsMargins(16, 0, 16, 0);

    mPowerKeyLabel = new QLabel(mPowerKeyFrame);
    mPowerKeyLabel->setFixedSize(550,60);//固定了高宽

    mPowerKeyComboBox = new QComboBox(mPowerKeyFrame);
    mPowerKeyComboBox->setObjectName("powerkey");//明天查一下
    mPowerKeyComboBox->setMinimumWidth(200);

    mPowerKeyLayout->addWidget(mPowerKeyLabel);
    mPowerKeyLayout->addWidget(mPowerKeyComboBox);

    PowerLayout->addWidget(mPowerKeyFrame);

    line_3 = new KHLineFrame(Powerwidget);
    PowerLayout->addWidget(line_3);

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

    mResourceFrame = new QFrame(widget);
    mResourceFrame->setMinimumSize(QSize(550, 80));
    mResourceFrame->setMaximumSize(QSize(16777215, 16777215));
    mResourceFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mResourceLayout = new QHBoxLayout(mResourceFrame);
    mResourceLayout->setContentsMargins(16, 0, 16, 0);

    mResourceLabel = new QLabel(mResourceFrame);
    mResourceLabel->setAlignment(Qt::AlignBottom);
    mResourceLabel->setFixedWidth(550);
    m_resourceDetailLabel = new LightLabel(widget);
    m_resourceDetailLabel->setText(tr("Reduce the occupation of backend running program resources and ensure smooth operation of key and focus applications."));
    m_resourceDetailLabel->setAlignment(Qt::AlignTop);

    QVBoxLayout *mLabelLayout = new QVBoxLayout(mResourceFrame);
    mLabelLayout->setSpacing(0);
    mLabelLayout->setContentsMargins(0,0,0,0);
    mLabelLayout->addWidget(mResourceLabel);
    mLabelLayout->addWidget(m_resourceDetailLabel);

    mResourceSButton = new KSwitchButton(mResourceFrame);
    mResourceSButton->setAccessibleName("Dynamic resource scheduling");
    mResourceSButton->setFixedSize(49, 24); 
    qDebug() << "mResourceSButton size:" << mResourceSButton->size();

    mResourceLayout->addLayout(mLabelLayout);
    mResourceLayout->addWidget(mResourceSButton);

    mverticalLayout->addWidget(Powerwidget);

    // 主机灯带设置布局
    mHostLightFrame = new QFrame(widget);
    mHostLightFrame->setMinimumSize(QSize(550, 0));
    mHostLightFrame->setMaximumSize(QSize(16777215, 16777215));
    mHostLightFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mHostLightLayout = new QHBoxLayout(mHostLightFrame);
    mHostLightLayout->setContentsMargins(16, 0, 16, 0);

    mHostLightLabel = new QLabel(mHostLightFrame);
    mHostLightLabel->setFixedSize(550,59);

    mHostLightSButton = new KSwitchButton(mHostLightFrame);
    mHostLightSButton->setAccessibleName("Host light strip");

    mHostLightLayout->addWidget(mHostLightLabel);
    mHostLightLayout->addStretch();
    mHostLightLayout->addWidget(mHostLightSButton);

    mverticalLayout->addWidget(mHostLightFrame);
    mHostLightFrame->setVisible(isSupportHostLight);

    mverticalLayout->addWidget(mResourceFrame);
    mverticalLayout->addSpacerItem(new QSpacerItem(20, 24, QSizePolicy::Fixed));
    // 使用电源时
    mverticalLayout->addWidget(createAcSectionWidget(widget));
    mverticalLayout->addSpacerItem(new QSpacerItem(20, 24, QSizePolicy::Fixed));

    // 使用电池时
    if(mHasBat){
        mverticalLayout->addWidget(createBatterySectionWidget(widget));
        mverticalLayout->addSpacerItem(new QSpacerItem(20, 24, QSizePolicy::Fixed));
    }

    m_batteryPlanTitleLabel = new KLabel(Powerwidget);
    m_batteryPlanTitleLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Power/Battery saving plan
    m_batteryPlanTitleLabel->setText((tr("Battery saving plan")));
    mverticalLayout->addWidget(m_batteryPlanTitleLabel);

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

    line_8 = new KHLineFrame(Batterywidget);
    BatteryLayout->addWidget(line_8);

    mLowpowerFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mLowpowerFrame);


    mLowpowerLabel1 = new QLabel(mLowpowerFrame);
    mLowpowerLabel1->setFixedSize(84,60);
    mLowpowerLabel2 = new QLabel(mLowpowerFrame);
    mLowpowerLabel2->setFixedSize(370,60);

    QHBoxLayout *mLowpowerLayout = new QHBoxLayout(mLowpowerFrame);
    mLowpowerLayout->setContentsMargins(16, 0, 16, 0);

    m_lowpowerComboBox1 = new QComboBox(mLowpowerFrame);
    m_lowpowerComboBox1->setObjectName("lowbatterylimit");
    m_lowpowerComboBox1->setFixedWidth(80);
    mLowpowerComboBox2 = new QComboBox(mLowpowerFrame);
    mLowpowerComboBox2->setObjectName("lowbatterysettings");
    mLowpowerComboBox2->setMinimumWidth(200);

    mLowpowerLayout->setSpacing(8);
    mLowpowerLayout->addWidget(mLowpowerLabel1);
    mLowpowerLayout->addWidget(m_lowpowerComboBox1);
    mLowpowerLayout->addWidget(mLowpowerLabel2);
    mLowpowerLayout->addWidget(mLowpowerComboBox2);

    BatteryLayout->addWidget(mLowpowerFrame);

    line_9 = new KHLineFrame(Batterywidget);
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

    line_10 = new KHLineFrame(Batterywidget);
    BatteryLayout->addWidget(line_10);

    mLowSaveFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mLowSaveFrame);

    QHBoxLayout *mLowSaveLayout = new QHBoxLayout(mLowSaveFrame);
    mLowSaveLayout->setContentsMargins(16, 0, 16, 0);

    mLowSaveLabel = new QLabel(mLowSaveFrame);
    mLowSaveLabel->setFixedSize(550,59);

    mLowSaveBtn = new KSwitchButton(mLowSaveFrame);
    mLowSaveBtn->setObjectName("lowbatterysave");
    mLowSaveBtn->setAccessibleName("lowbatterysave");

    mLowSaveLayout->addWidget(mLowSaveLabel);
    mLowSaveLayout->addStretch();
    mLowSaveLayout->addWidget(mLowSaveBtn);

    BatteryLayout->addWidget(mLowSaveFrame);

    line_11 = new KHLineFrame(Batterywidget);
    BatteryLayout->addWidget(line_11);

    mBatterySaveFrame = new QFrame(Batterywidget);
    setFrame_Noframe(mBatterySaveFrame);

    QHBoxLayout *mBatterySaveLayout = new QHBoxLayout(mBatterySaveFrame);
    mBatterySaveLayout->setContentsMargins(16, 0, 16, 0);

    mBatterySaveLabel = new QLabel(mBatterySaveFrame);
    mBatterySaveLabel->setFixedSize(550,59);

    mBatterySaveBtn = new KSwitchButton(mBatterySaveFrame);
    mBatterySaveBtn->setObjectName("batterysave");
    mBatterySaveBtn->setAccessibleName("batterysave");

    mBatterySaveLayout->addWidget(mBatterySaveLabel);
    mBatterySaveLayout->addStretch();
    mBatterySaveLayout->addWidget(mBatterySaveBtn);

    BatteryLayout->addWidget(mBatterySaveFrame);

    line_12 = new KHLineFrame(Batterywidget);
    BatteryLayout->addWidget(line_12);

    // 添加"显示剩余时间"设置框架
    if(mHasBat){
        m_remainingTimeFrame = new QFrame(Batterywidget);
        setFrame_Noframe(m_remainingTimeFrame);

        QHBoxLayout *remainingTimeLayout = new QHBoxLayout(m_remainingTimeFrame);
        remainingTimeLayout->setContentsMargins(16, 0, 16, 0);

        m_remainingTimeLabel = new QLabel(m_remainingTimeFrame);
        m_remainingTimeLabel->setAlignment(Qt::AlignBottom);
        m_remainingTimeLabel->setFixedWidth(550);
        m_remainingTimeDetailLabel = new LightLabel(Batterywidget);
        m_remainingTimeDetailLabel->setText(tr("Remaining time is estimated based on real-time hardware power and may be inaccurate."));
        m_remainingTimeDetailLabel->setAlignment(Qt::AlignTop);

        QVBoxLayout *LabelLayout = new QVBoxLayout(m_remainingTimeFrame);
        LabelLayout->setSpacing(0);
        LabelLayout->setContentsMargins(0,0,0,0);
        LabelLayout->addWidget(m_remainingTimeLabel);
        LabelLayout->addWidget(m_remainingTimeDetailLabel);

        m_remainingTimeSwitch = new KSwitchButton(m_remainingTimeFrame);
        m_remainingTimeSwitch->setAccessibleName("Display remaining charging time and usage time");
        m_remainingTimeSwitch->setFixedSize(49, 24); 

        remainingTimeLayout->addLayout(LabelLayout);
        remainingTimeLayout->addWidget(m_remainingTimeSwitch);

        BatteryLayout->addWidget(m_remainingTimeFrame);
    }

    mverticalLayout->addWidget(Batterywidget);
    mverticalLayout->addStretch();

    retranslateUi();
}

void Power::setVisibleBySecurity()
{
    // 安全管控 免密登录及自动登录设置项的显示与隐藏
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value(name().toLower() + "Settings").toString();
    qDebug()<<Q_FUNC_INFO<<"moduleSettings:"<<moduleSettings;
    QStringList setItems = moduleSettings.split(",");

    qDebug() << Q_FUNC_INFO << moduleSettings << "===" <<  name().toLower();

    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "CloseDisplayFrame") {
            m_acCloseDisplayFrame->setVisible(item.at(1) == "true");
            m_acCloseDisplayLine->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "SleepFrame") {
            m_acSleepFrame->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mSleepPwdFrame") {
            mSleepPwdFrame->setVisible(item.at(1) == "true");
            line_1->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mWakenPwdFrame") {
            mWakenPwdFrame->setVisible(item.at(1) == "true");
            line_2->setVisible(item.at(1) == "true");
        }
    }

    if (UkccCommon::getCpuInfo().contains("9006C", Qt::CaseInsensitive)) {
        mResourceFrame->setVisible(false);
    }
}

bool Power::kconf2Enable(const QString &cloudPlatForm) const
{
    if (cloudPlatForm == "huawei") {
        qDebug() << Q_FUNC_INFO << m_isExistHibernate << m_isExistHibernate;
        if ((!m_isExistHibernate) && (!m_isExistSuspend)) return false;
    }
    return true;
}

void Power::initInterface()
{
    if (mHibernateInterface == nullptr) {
        mHibernateInterface = new QDBusInterface("org.freedesktop.login1",
                                                 "/org/freedesktop/login1",
                                                 "org.freedesktop.login1.Manager",
                                                 QDBusConnection::systemBus(),this);

        isSuspendSupply();
        isHibernateSupply();
        isSuspend2HibernateSupply();
    }
}

bool Power::isSuspend2HibernateSupply()
{
    if (!mHibernateInterface || !mHibernateInterface->isValid()) {
        qWarning() << "login1 interface invalid for CanSuspendThenHibernate";
        m_isExistS2H = false;
        return m_isExistS2H;
    }
    QDBusReply<QString> reply = mHibernateInterface->call("CanSuspend2Hibernate");
    if (!reply.isValid()) {
        qWarning() << "CanSuspend2Hibernate call failed:" << QDBusConnection::systemBus().lastError();
        m_isExistS2H = false;
        return m_isExistS2H;
    }
    const QString v = reply.value();
    m_isExistS2H = (v == "yes");
    return m_isExistS2H;
}

void Power::retranslateUi()
{
    //~ contents_path /Power/Require password when suspend/hibernate
    QString str = (m_isExistHibernate && m_isExistSuspend) ? tr("Require password when suspend/hibernate")
                                                         : (m_isExistSuspend ? tr("Require password when suspend") :
                                                                              (m_isExistHibernate ? tr("Require password when hibernate") : ""));
    if (str.isEmpty()) {
        mSleepPwdFrame->setVisible(false);
    }
    if (QLabelSetText(mSleepPwdLabel, str)) {
        mSleepPwdLabel->setToolTip(str);
    }

    //~ contents_path /Power/Password required when waking up the screen
    if (QLabelSetText(mWakenPwdLabel, tr("Password required when waking up the screen"))) {
        mSleepPwdLabel->setToolTip(tr("Password required when waking up the screen"));
    }

    //~ contents_path /Power/Press the power button
    if (QLabelSetText(mPowerKeyLabel, tr("Press the power button"))) {
        mPowerKeyLabel->setToolTip("Press the power button");
    }

    //~ contents_path /Power/Time to close display
    if (QLabelSetText(m_acCloseDisplayLabel, tr("Time to close display"))) {
        m_acCloseDisplayLabel->setToolTip(tr("Time to close display"));
    }
    if(mHasBat) {
        //~ contents_path /Power/Time to close display
        if (QLabelSetText(m_batteryCloseDisplayLabel, tr("Time to close display"))) {
            m_batteryCloseDisplayLabel->setToolTip(tr("Time to close display"));
        }
        //~ contents_path /Power/Time to sleep
        if (QLabelSetText(m_batterySleepLabel, tr("Time to sleep"))) {
            m_batterySleepLabel->setToolTip(tr("Time to sleep"));
        }
        //~ contents_path /Power/Using battery
        if (QLabelSetText(m_batteryPowerLabel, tr("Power plan"))) {
            m_batteryPowerLabel->setToolTip(tr("Power plan"));
        }
        //~ contents_path /Power/Time to hibernate
        if (QLabelSetText(m_batteryHibernateLabel, tr("After this period, the system will transition from sleep to hibernation."))) {
            m_batteryHibernateLabel->setToolTip(tr("After this period, the system will transition from sleep to hibernation."));
        }
        if (QLabelSetText(m_remainingTimeLabel, tr("Display remaining charging time and usage time"))) {
            m_remainingTimeLabel->setToolTip(tr("Display remaining charging time and usage time"));
        }
    }

    //~ contents_path /Power/Time to sleep
    if (QLabelSetText(m_acSleepLabel, tr("Time to sleep"))) {
        m_acSleepLabel->setToolTip(tr("Time to sleep"));
    }

    //~ contents_path /Power/Time to hibernate
    if (QLabelSetText(m_acHibernateLabel, tr("After this period, the system will transition from sleep to hibernation."))) {
        m_acHibernateLabel->setToolTip(tr("After this period, the system will transition from sleep to hibernation."));
    }

    //~ contents_path /Power/Notebook cover
    if (QLabelSetText(mCloseLidLabel, tr("Notebook cover"))) {
        mCloseLidLabel->setToolTip(tr("Notebook cover"));
    }

    //~ contents_path /Power/Host light strip
    if (QLabelSetText(mHostLightLabel, tr("Host light strip"))) {
        mHostLightLabel->setToolTip(tr("Host light strip"));
    }

    //~ contents_path /Power/Dynamic resource scheduling
    if (QLabelSetText(mResourceLabel, tr("Dynamic resource scheduling"))) {
        mResourceLabel->setToolTip(tr("Dynamic resource scheduling"));
    }

    //~ contents_path /Power/Using power
    if (QLabelSetText(m_acPowerLabel, tr("Power plan"))) {
        m_acPowerLabel->setToolTip(tr("Power plan"));
    }

    if (QLabelSetText(mDarkenLabel, tr(" Time to darken"))) {
        mDarkenLabel->setToolTip(tr(" Time to darken"));
    }

    if (QLabelSetText(mLowpowerLabel1, tr("Battery level is lower than"))) {
        mLowpowerLabel1->setToolTip(tr("Battery level is lower than"));
    }

    mLowpowerLabel2->setText(tr("Run"));

    //~ contents_path /Power/Low battery notification
    if (QLabelSetText(mNoticeLabel, tr("Low battery notification"))) {
        mNoticeLabel->setToolTip(tr("Low battery notification"));
    }

    //~ contents_path /Power/"Automatically run saving mode when low battery
    if (QLabelSetText(mLowSaveLabel, tr("Automatically run saving mode when low battery"))) {
        mLowSaveLabel->setToolTip(tr("Automatically run saving mode when the low battery"));
    }

    if (QLabelSetText(mBatterySaveLabel, tr("Automatically run saving mode when using battery"))) {
        mBatterySaveLabel->setToolTip(tr("Automatically run saving mode when using battery"));
    }
}

void Power::resetui()
{
    mBatterySaveFrame->hide();

    mDarkenFrame->hide();
    line_8->hide();
    line_11->hide();
    //不存在盖子隐藏该项
    if (!mIsExistLid || isDesktop()) {
        mCloseLidFrame->hide();
    }
    
    // 不存在电池隐藏这些设置项
    if (!mHasBat) {
        m_batteryPlanTitleLabel->hide();
        clearAutoItem(BatteryLayout);
        Batterywidget->hide();
    }

    //Intel作如下处理
    if (UkccCommon::isTablet()) {
        mSleepPwdFrame->hide();
        mWakenPwdFrame->hide();
        mPowerKeyFrame->hide();
        mCloseLidFrame->hide();
        mDarkenFrame->hide();
        mLowpowerFrame->hide();
        mNoticeLFrame->hide();
        mItem->changeSize(0, 0);
        line_1->hide();
        line_2->hide();
        line_3->hide();
        line_8->hide();
        line_9->hide();
        line_10->hide();
        line_11->hide();
        line_12->hide();
    }
}

void Power::setupComponent()
{
    // 合盖
    closeLidStringList << tr("Nothing") << tr("Blank") << tr("Shutdown") << tr("Suspend") << tr("Hibernate");
    mCloseLidComboBox->insertItem(0, closeLidStringList.at(0), "nothing");
    mCloseLidComboBox->insertItem(1, closeLidStringList.at(1), "blank");
    mCloseLidComboBox->insertItem(2, closeLidStringList.at(2), "shutdown");
    if (m_isExistSuspend) mCloseLidComboBox->insertItem(3, closeLidStringList.at(3), "suspend");
    if (m_isExistHibernate) {
        mCloseLidComboBox->insertItem(4, closeLidStringList.at(4), "hibernate");
    }
    syncInvalidActionGsettings(settings, BUTTON_LID_AC_KEY, mCloseLidComboBox, kLidActionFallback);
    syncInvalidActionGsettings(settings, BUTTON_LID_BATT_KET, mCloseLidComboBox, kLidActionFallback);

    //按下电源键时
    buttonStringList << tr("Interactive") << tr("Shutdown") << tr("Suspend") << tr("Hibernate");
    mPowerKeyComboBox->insertItem(0, buttonStringList.at(0), "interactive");
    mPowerKeyComboBox->insertItem(1, buttonStringList.at(1), "shutdown");
    if (m_isExistSuspend) mPowerKeyComboBox->insertItem(2, buttonStringList.at(2), "suspend");
    if (m_isExistHibernate) mPowerKeyComboBox->insertItem(3, buttonStringList.at(3), "hibernate");
    syncInvalidActionGsettings(settings, BUTTON_POWER_KEY, mPowerKeyComboBox, kPowerButtonActionFallback);

    //关闭显示器
    closeStringList <<tr("1min")<< tr("5min") << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("Never");
    m_acCloseDisplayComboBox->insertItem(0, closeStringList.at(0), QVariant::fromValue(1)); 
    m_acCloseDisplayComboBox->insertItem(1, closeStringList.at(1), QVariant::fromValue(5));
    m_acCloseDisplayComboBox->insertItem(2, closeStringList.at(2), QVariant::fromValue(10));
    m_acCloseDisplayComboBox->insertItem(3, closeStringList.at(3), QVariant::fromValue(15));
    m_acCloseDisplayComboBox->insertItem(4, closeStringList.at(4), QVariant::fromValue(30));
    m_acCloseDisplayComboBox->insertItem(5, closeStringList.at(5), QVariant::fromValue(60)); 
    m_acCloseDisplayComboBox->insertItem(6, closeStringList.at(6), QVariant::fromValue(120));    
    m_acCloseDisplayComboBox->insertItem(7, closeStringList.at(7), QVariant::fromValue(0));

    if(mHasBat){
        //关闭显示器
        closeStringList_battery <<tr("1min")<< tr("5min") << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("Never");
        m_batteryCloseDisplayComboBox->insertItem(0, closeStringList.at(0), QVariant::fromValue(1)); 
        m_batteryCloseDisplayComboBox->insertItem(1, closeStringList.at(1), QVariant::fromValue(5));
        m_batteryCloseDisplayComboBox->insertItem(2, closeStringList.at(2), QVariant::fromValue(10));
        m_batteryCloseDisplayComboBox->insertItem(3, closeStringList.at(3), QVariant::fromValue(15));
        m_batteryCloseDisplayComboBox->insertItem(4, closeStringList.at(4), QVariant::fromValue(30));
        m_batteryCloseDisplayComboBox->insertItem(5, closeStringList.at(5), QVariant::fromValue(60)); 
        m_batteryCloseDisplayComboBox->insertItem(6, closeStringList.at(6), QVariant::fromValue(120));    
        m_batteryCloseDisplayComboBox->insertItem(7, closeStringList.at(7), QVariant::fromValue(0));
    }


    //睡眠
    sleepStringList << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("3h") << tr("Never");
    m_acSleepComboBox->insertItem(0, sleepStringList.at(0), QVariant::fromValue(10));
    m_acSleepComboBox->insertItem(1, sleepStringList.at(1), QVariant::fromValue(15));
    m_acSleepComboBox->insertItem(2, sleepStringList.at(2), QVariant::fromValue(30));
    m_acSleepComboBox->insertItem(3, sleepStringList.at(3), QVariant::fromValue(60));
    m_acSleepComboBox->insertItem(4, sleepStringList.at(4), QVariant::fromValue(120));
    m_acSleepComboBox->insertItem(5, sleepStringList.at(5), QVariant::fromValue(180));
    m_acSleepComboBox->insertItem(6, sleepStringList.at(6), QVariant::fromValue(0));

    //休眠
    acHibernateStringList << tr("30min") << tr("1h") << tr("2h") << tr("3h") << tr("4h") << tr("5h")<< tr("Never");
    m_acHibernateComboBox->insertItem(0, acHibernateStringList.at(0), QVariant::fromValue(30));
    m_acHibernateComboBox->insertItem(1, acHibernateStringList.at(1), QVariant::fromValue(60));
    m_acHibernateComboBox->insertItem(2, acHibernateStringList.at(2), QVariant::fromValue(120));
    m_acHibernateComboBox->insertItem(3, acHibernateStringList.at(3), QVariant::fromValue(180));
    m_acHibernateComboBox->insertItem(4, acHibernateStringList.at(4), QVariant::fromValue(240));
    m_acHibernateComboBox->insertItem(5, acHibernateStringList.at(5), QVariant::fromValue(300));
    m_acHibernateComboBox->insertItem(6, acHibernateStringList.at(6), QVariant::fromValue(0));

    if (mHasBat) {
        //睡眠
        sleepStringList_battery << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("3h") << tr("Never");
        m_batterySleepComboBox->insertItem(0, sleepStringList.at(0), QVariant::fromValue(10));
        m_batterySleepComboBox->insertItem(1, sleepStringList.at(1), QVariant::fromValue(15));
        m_batterySleepComboBox->insertItem(2, sleepStringList.at(2), QVariant::fromValue(30));
        m_batterySleepComboBox->insertItem(3, sleepStringList.at(3), QVariant::fromValue(60));
        m_batterySleepComboBox->insertItem(4, sleepStringList.at(4), QVariant::fromValue(120));
        m_batterySleepComboBox->insertItem(5, sleepStringList.at(5), QVariant::fromValue(180));
        m_batterySleepComboBox->insertItem(6, sleepStringList.at(6), QVariant::fromValue(0));
         //休眠
        batteryHibernateStringList << tr("30min") << tr("1h") << tr("2h") << tr("3h") << tr("4h") << tr("5h")<< tr("Never");
        m_batteryHibernateComboBox->insertItem(0, batteryHibernateStringList.at(0), QVariant::fromValue(30));
        m_batteryHibernateComboBox->insertItem(1, batteryHibernateStringList.at(1), QVariant::fromValue(60));
        m_batteryHibernateComboBox->insertItem(2, batteryHibernateStringList.at(2), QVariant::fromValue(120));
        m_batteryHibernateComboBox->insertItem(3, batteryHibernateStringList.at(3), QVariant::fromValue(180));
        m_batteryHibernateComboBox->insertItem(4, batteryHibernateStringList.at(4), QVariant::fromValue(240));
        m_batteryHibernateComboBox->insertItem(5, batteryHibernateStringList.at(5), QVariant::fromValue(300));
        m_batteryHibernateComboBox->insertItem(6, batteryHibernateStringList.at(6), QVariant::fromValue(0));
    }

    //电源计划
    PowerplanStringList << tr("Balance") << tr("Energy Efficiency")<<tr("Performance");
    m_acPowerComboBox->insertItem(0, PowerplanStringList.at(0), "Balance");
    m_acPowerComboBox->insertItem(1, PowerplanStringList.at(1), "Energy Efficiency");
    m_acPowerComboBox->insertItem(2, PowerplanStringList.at(2), "Performance");

    if (mHasBat) {
        BatteryplanStringList << tr("Balance") << tr("Energy Efficiency")<<tr("Performance");
        m_batteryPowerComboBox->insertItem(0, BatteryplanStringList.at(0), "Balance");
        m_batteryPowerComboBox->insertItem(1, BatteryplanStringList.at(1), "Energy Efficiency");
        m_batteryPowerComboBox->insertItem(2, BatteryplanStringList.at(2), "Performance");
    }

    //低电量时执行
    LowpowerStringList << tr("Nothing") << tr("Blank") << tr("Shutdown")  << tr("Suspend") << tr("Hibernate");
    mLowpowerComboBox2->insertItem(0, LowpowerStringList.at(0), "nothing");
    mLowpowerComboBox2->insertItem(1, LowpowerStringList.at(1), "blank");
    mLowpowerComboBox2->insertItem(2, LowpowerStringList.at(2), "shutdown");
    if (m_isExistSuspend) mLowpowerComboBox2->insertItem(3, LowpowerStringList.at(3), "suspend");
    if (m_isExistHibernate) mLowpowerComboBox2->insertItem(4, LowpowerStringList.at(4), "hibernate");
    syncInvalidActionGsettings(settings, ACTION_CRI_BTY, mLowpowerComboBox2, kCriticalBatteryActionFallback);

    //低电量通知
    for (int i = 1; i < 5; i++) {
        mNoticeComboBox->insertItem(i-1, QString("%1%").arg(i*10));
    }

    //电池低电量范围
    int batteryRemain = settings->get(PER_ACTION_CRI).toInt();
    for(int i = 0; i < batteryRemain; i++) {
        m_lowpowerComboBox1->insertItem(i, QString("%1%").arg(i+1));
    }
}

void Power::setupConnect()
{
    connect(mSleepPwdBtn,&KSwitchButton::stateChanged, [=](bool checked){
        UkccCommon::buriedSettings(name(), "whether password is required for wake-up after sleep", QString("settings"), checked ? "true" : "false");
        screensettings->set(SLEEP_ACTIVATION_ENABLED,checked);
    });

    if (screensettings->keys().contains("closeActivationEnabled")) {
        connect(mWakenPwdBtn,&KSwitchButton::stateChanged, [=](bool checked){
            UkccCommon::buriedSettings(name(), "whether password is required for wake-up screen", QString("settings"), checked ? "true" : "false");
            screensettings->set(CLOSE_ACTIVATION_ENABLED,checked);
        });
    }

    connect(mPowerKeyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        UkccCommon::buriedSettings(name(), "execute when pressing the power key", QString("select"), mPowerKeyComboBox->itemData(index).toString());
        settings->set(BUTTON_POWER_KEY, mPowerKeyComboBox->itemData(index));
    });

    connect(m_acCloseDisplayComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        UkccCommon::buriedSettings(name(), "turn off the display after this time period", QString("select"), m_acCloseDisplayComboBox->currentData().toString());
        if (m_acCloseDisplayComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(SLEEP_DISPLAY_AC_KEY, -1);
            qDebug()<<"sleep-display-ac : "<<settings->trySet(SLEEP_DISPLAY_AC_KEY, -1);
        } else {
            settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(m_acCloseDisplayComboBox->currentData(Qt::UserRole).toInt() * 60));
            qDebug()<<"sleep-display-ac : "<<settings->trySet(SLEEP_DISPLAY_AC_KEY, m_acCloseDisplayComboBox->currentData(Qt::UserRole).toInt() * 60);
        }
        setHintLable();
    });

    if(mHasBat) {
        connect(m_batteryCloseDisplayComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            Q_UNUSED(index)
            if (m_batteryCloseDisplayComboBox->currentData(Qt::UserRole).toInt() == 0) {
                settings->set(SLEEP_DISPLAY_BATT_KEY, -1);
                qDebug()<<"sleep-display-battery : "<<settings->trySet(SLEEP_DISPLAY_BATT_KEY, -1);
            } else {
                settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(m_batteryCloseDisplayComboBox->currentData(Qt::UserRole).toInt() * 60));
                qDebug()<<"sleep-display-battery : "<<settings->trySet(SLEEP_DISPLAY_BATT_KEY, m_batteryCloseDisplayComboBox->currentData(Qt::UserRole).toInt() * 60);
            }
            setHintLable();
        });

        connect(m_batterySleepComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            Q_UNUSED(index)
            if (m_batterySleepComboBox->currentData(Qt::UserRole).toInt() == 0) {
                settings->set(SLEEP_COMPUTER_BATT_KEY, -1);
                qDebug()<<"sleep-computer-battery : "<<settings->trySet(SLEEP_COMPUTER_BATT_KEY, -1);
            } else {
                settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(m_batterySleepComboBox->currentData(Qt::UserRole).toInt() * 60));
                qDebug()<<"sleep-computer-battery : "<<settings->trySet(SLEEP_COMPUTER_BATT_KEY, m_batterySleepComboBox->currentData(Qt::UserRole).toInt() * 60);
            }
            setHintLable();
        });

        connect(m_batteryHibernateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
            int value = m_batteryHibernateComboBox->currentData(Qt::UserRole).toInt();
            int seconds = (value == 0) ? -1 : value * 60;
            qDebug()<<"hibernate-computer-battery : " << settings->trySet(HIBERNATE_COMPUTER_BATT_KEY, seconds);
         });

        // 连接剩余时间显示开关信号
        connect(m_remainingTimeSwitch, &KSwitchButton::stateChanged, this, [=](bool checked) {
            UkccCommon::buriedSettings(name(), "Display remaining time", QString("settings"), checked ? "true" : "false");
            settings->set(DISPLAY_LEFT_TIME, checked);
            qDebug() << "Display remaining time feature is now" << (checked ? "enabled" : "disabled");
        });
    }


    connect(m_acSleepComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        UkccCommon::buriedSettings(name(), "the system go to sleep after this time period", QString("select"), m_acSleepComboBox->currentData().toString());
        if (m_acSleepComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(SLEEP_COMPUTER_AC_KEY, -1);
            qDebug()<<"sleep-computer-ac : "<<settings->trySet(SLEEP_COMPUTER_AC_KEY, -1);
        } else {
            settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(m_acSleepComboBox->currentData(Qt::UserRole).toInt() * 60));
            qDebug()<<"sleep-computer-ac : "<<settings->trySet(SLEEP_COMPUTER_AC_KEY, m_acSleepComboBox->currentData(Qt::UserRole).toInt() * 60);
        }
        setHintLable();
    });

    connect(m_acHibernateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        int value = m_acHibernateComboBox->currentData(Qt::UserRole).toInt();
        int seconds = (value == 0) ? -1 : value * 60;
        qDebug()<<"hibernate-computer-ac : " << settings->trySet(HIBERNATE_COMPUTER_AC_KEY, seconds);
     });

    connect(mCloseLidComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        UkccCommon::buriedSettings(name(), "execute when the notebook is closed", QString("select"), mCloseLidComboBox->currentData().toString());
        settings->set(BUTTON_LID_AC_KEY, mCloseLidComboBox->itemData(index));
        settings->set(BUTTON_LID_BATT_KET, mCloseLidComboBox->itemData(index));
    });

    connect(mHostLightSButton, &KSwitchButton::stateChanged, this, [=](bool checked){
        UkccCommon::buriedSettings(name(), "Host light strip", QString("settings"), checked ? "true":"false");
        QDBusReply<bool> ret = hostLightStripDbus->call("setHostLightStrip", checked);
        if (!ret) {
            qDebug() << "setHostLightStrip failed!";
            mHostLightSButton->blockSignals(true);
            mHostLightSButton->setChecked(!checked);
            mHostLightSButton->blockSignals(false);
        }
    });


    connect(mResourceSButton, &KSwitchButton::clicked, this, [=](bool checked){
        if (mProcManagerSettings != nullptr && mProcManagerSettings->keys().contains("resourceLimitEnabled")) {
            mProcManagerSettings->set(RESOURCE_LIMIT_ENABLED, checked);
        }
    });

    if (mKeys.contains("powerPolicyAc") && mKeys.contains("powerPolicyBattery")) {
        connect(m_acPowerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            UkccCommon::buriedSettings(name(), "mode when using power supply", QString("select"), m_acPowerComboBox->currentData().toString());
            if (index == 0) {
                settings->set(POWER_POLICY_AC, 1);
            }  else if (index == 1) {
                settings->set(POWER_POLICY_AC, 2);
            } else {
                settings->set(POWER_POLICY_AC, 0);
            }
        });

        if(mHasBat) {
            connect(m_batteryPowerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            UkccCommon::buriedSettings(name(), "mode when using battery supply", QString("select"), m_batteryPowerComboBox->currentData().toString());
            if (index == 0) {
                settings->set(POWER_POLICY_BATTARY, 1);
            } else if (index == 1) {
                settings->set(POWER_POLICY_BATTARY, 2);
            } else {
                settings->set(POWER_POLICY_BATTARY, 0);
            }
        });
        }
    }

    connect(mDarkenComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        UkccCommon::buriedSettings(name(), "reduce screen brightness after this time period", QString("select"), m_batteryPowerComboBox->currentData().toString());
        if (mDarkenComboBox->currentData(Qt::UserRole).toInt() == 0) {
            settings->set(IDLE_DIM_TIME_KEY, -1);
        } else {
            settings->set(IDLE_DIM_TIME_KEY, QVariant(mDarkenComboBox->currentData(Qt::UserRole).toInt() * 60));
        }
    });

    connect(m_lowpowerComboBox1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        UkccCommon::buriedSettings(name(), "low battery value", QString("select"), m_lowpowerComboBox1->currentData().toString());
       settings->set(PER_ACTION_KEY, index + 1); 
    });

    connect(mLowpowerComboBox2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        UkccCommon::buriedSettings(name(), "execute below low battery level", QString("select"), mLowpowerComboBox2->currentData().toString());
        settings->set(ACTION_CRI_BTY, mLowpowerComboBox2->itemData(index));
    });

    connect(mNoticeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        UkccCommon::buriedSettings(name(), "notify when the power is lower than this value", QString("select"), mNoticeComboBox->currentText());
        settings->set(PERCENTAGE_LOW, (index + 1)*10);
    });

    connect(mLowSaveBtn,&KSwitchButton::stateChanged, [=](bool checked){
        UkccCommon::buriedSettings(name(), "whether the saving mode is autoturned on when the battery is low", QString("select"), checked ? "true" : "false");
        settings->set(LOW_BATTERY_AUTO_SAVE,checked);
    });

    connect(settings, &QGSettings::changed, this, [=](const QString &key) {
        initCustomPlanStatus();
        // 检查该 key 的可写状态是否变化，动态更新控件
        updateWidgetEnabledByKey(POWERMANAGER_SCHEMA, key);
    });

    connect(screensettings, &QGSettings::changed, this, [=](const QString &key) {
        initCustomPlanStatus();
        // 检查该 key 的可写状态是否变化，动态更新控件
        updateWidgetEnabledByKey(SCREENSAVER_SCHEMA, key);
    });

    if (mProcManagerSettings != nullptr && mProcManagerSettings->keys().contains("resourceLimitEnabled")) {
        connect(mProcManagerSettings, &QGSettings::changed, this, [=](const QString &key) {
            initCustomPlanStatus();
            // 检查该 key 的可写状态是否变化，动态更新控件
            updateWidgetEnabledByKey(PROC_MANAGER, key);
        });
    }

    setVisibleBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setVisibleBySecurity()));
    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void Power::initCustomPlanStatus()
{
    // 信号阻塞
    mPowerKeyComboBox->blockSignals(true);
    m_acCloseDisplayComboBox->blockSignals(true);
    m_acSleepComboBox->blockSignals(true);
    m_acHibernateComboBox->blockSignals(true);
    mCloseLidComboBox->blockSignals(true);
    m_acPowerComboBox->blockSignals(true);
    if (mHasBat) {
        m_batteryPowerComboBox->blockSignals(true);
        m_batteryHibernateComboBox->blockSignals(true);
    }
    m_lowpowerComboBox1->blockSignals(true);
    mLowpowerComboBox2->blockSignals(true);
    mNoticeComboBox->blockSignals(true);
    mSleepPwdBtn->blockSignals(true);
    mWakenPwdBtn->blockSignals(true);
    mLowSaveBtn->blockSignals(true);
    mBatterySaveBtn->blockSignals(true);

    mPowerKeyComboBox->setCurrentIndex(actionComboIndex(mPowerKeyComboBox,
                                                        settings->get(BUTTON_POWER_KEY).toString(),
                                                        kPowerButtonActionFallback));
    if (settings->get(SLEEP_COMPUTER_AC_KEY).toInt() == -1) {
        m_acSleepComboBox->setCurrentIndex(m_acSleepComboBox->findData(0));
    } else {
        m_acSleepComboBox->setCurrentIndex(m_acSleepComboBox->findData(settings->get(SLEEP_COMPUTER_AC_KEY).toInt() / FIXES));
    }

    if (settings->get(SLEEP_DISPLAY_AC_KEY).toInt() == -1) {
        m_acCloseDisplayComboBox->setCurrentIndex(m_acCloseDisplayComboBox->findData(0));
    } else {
        m_acCloseDisplayComboBox->setCurrentIndex(m_acCloseDisplayComboBox->findData(settings->get(SLEEP_DISPLAY_AC_KEY).toInt() / FIXES));;
    }

    if (settings->get(HIBERNATE_COMPUTER_AC_KEY).toInt() == -1) {
        m_acHibernateComboBox->setCurrentIndex(m_acHibernateComboBox->findData(0));
    } else {
        m_acHibernateComboBox->setCurrentIndex(m_acHibernateComboBox->findData(settings->get(HIBERNATE_COMPUTER_AC_KEY).toInt() / FIXES));
    }

    if(mHasBat) {
        if (settings->get(SLEEP_COMPUTER_BATT_KEY).toInt() == -1) {
            m_batterySleepComboBox->setCurrentIndex(m_batterySleepComboBox->findData(0));
        } else {
            m_batterySleepComboBox->setCurrentIndex(m_batterySleepComboBox->findData(settings->get(SLEEP_COMPUTER_BATT_KEY).toInt() / FIXES));
        }

        if (settings->get(SLEEP_DISPLAY_BATT_KEY).toInt() == -1) {
            m_batteryCloseDisplayComboBox->setCurrentIndex(m_batteryCloseDisplayComboBox->findData(0));
        } else {
            m_batteryCloseDisplayComboBox->setCurrentIndex(m_batteryCloseDisplayComboBox->findData(settings->get(SLEEP_DISPLAY_BATT_KEY).toInt() / FIXES));
        }

        if (settings->get(HIBERNATE_COMPUTER_BATT_KEY).toInt() == -1) {
            m_batteryHibernateComboBox->setCurrentIndex(m_batteryHibernateComboBox->findData(0));
        } else {
            m_batteryHibernateComboBox->setCurrentIndex(m_batteryHibernateComboBox->findData(settings->get(HIBERNATE_COMPUTER_BATT_KEY).toInt() / FIXES));
        }

        if (settings->keys().contains(DISPLAY_LEFT_TIME)) {
            m_remainingTimeSwitch->blockSignals(true);
            m_remainingTimeSwitch->setChecked(settings->get(DISPLAY_LEFT_TIME).toBool());
            m_remainingTimeSwitch->blockSignals(false);
        }
    }

    setHintLable();

    mCloseLidComboBox->setCurrentIndex(actionComboIndex(mCloseLidComboBox,
                                                        settings->get(BUTTON_LID_AC_KEY).toString(),
                                                        kLidActionFallback));

    if (isSupportHostLight && hostLightStripDbus != nullptr) {
        mHostLightSButton->blockSignals(true);
        QDBusReply<bool> ret = hostLightStripDbus->call("getHostLightStripStatus");
        mHostLightSButton->setChecked(ret);
        mHostLightSButton->blockSignals(false);
    }

    if (mProcManagerSettings != nullptr && mProcManagerSettings->keys().contains("resourceLimitEnabled")) {
        mResourceSButton->blockSignals(true);
        mResourceSButton->setChecked(mProcManagerSettings->get(RESOURCE_LIMIT_ENABLED).toBool());
        mResourceSButton->blockSignals(false);
    }

    if (mKeys.contains("powerPolicyAc") && mKeys.contains("powerPolicyBattery")) {
        if (1 == settings->get(POWER_POLICY_AC).toInt()) {
            m_acPowerComboBox->setCurrentIndex(0);
        } else if (2 == settings->get(POWER_POLICY_AC).toInt()){
            m_acPowerComboBox->setCurrentIndex(1);
        } else {
            m_acPowerComboBox->setCurrentIndex(2);
        }
        if(mHasBat) {
            if (1 == settings->get(POWER_POLICY_BATTARY).toInt()) {
                m_batteryPowerComboBox->setCurrentIndex(0);
            } else if (2 == settings->get(POWER_POLICY_BATTARY).toInt()) {
                m_batteryPowerComboBox->setCurrentIndex(1);
            } else {
                m_batteryPowerComboBox->setCurrentIndex(2);
            }
        }
    } else {
        m_acPowerComboBox->setEnabled(false);
        if( mHasBat){
            m_batteryPowerComboBox->setEnabled(false);
        }

    }

    m_lowpowerComboBox1->setCurrentIndex(qBound(0, settings->get(PER_ACTION_KEY).toInt() - 1, m_lowpowerComboBox1->count() - 1));
    mLowpowerComboBox2->setCurrentIndex(actionComboIndex(mLowpowerComboBox2,
                                                         settings->get(ACTION_CRI_BTY).toString(),
                                                         kCriticalBatteryActionFallback));
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
    m_acCloseDisplayComboBox->blockSignals(false);
    m_acSleepComboBox->blockSignals(false);
    m_acHibernateComboBox->blockSignals(false);
    mCloseLidComboBox->blockSignals(false);
    m_acPowerComboBox->blockSignals(false);
    if (mHasBat) {
        m_batteryPowerComboBox->blockSignals(false);
        m_batteryHibernateComboBox->blockSignals(false);
    }
    m_lowpowerComboBox1->blockSignals(false);
    mLowpowerComboBox2->blockSignals(false);
    mNoticeComboBox->blockSignals(false);
    mSleepPwdBtn->blockSignals(false);
    mWakenPwdBtn->blockSignals(false);
    mLowSaveBtn->blockSignals(false);
    mBatterySaveBtn->blockSignals(false);

    // 初始化映射表（此时控件已全部初始化）
    initKeyWidgetMapping();
    
    // 根据 GSettings key 的可写性禁用/启用控件（管控功能）
    setWidgetsEnabledByGSettings();
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
    mIsExistLid = LidInfo.value().toBool();
}

bool Power::isHibernateSupply()
{
    if (!mHibernateInterface->isValid()) {
        qDebug() << "Create login1 Hibernate Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return false;
    }
    QDBusReply<QString> HibernateInfo;
    HibernateInfo = mHibernateInterface->call("CanHibernate");
    m_isExistHibernate = HibernateInfo == "yes" ? true : false;

    return m_isExistHibernate;
}

// 是否支持睡眠
bool Power::isSuspendSupply()
{
    if (!mHibernateInterface->isValid()) {
        qDebug() << "Create login1 CanSuspend Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return false;
    }
    QDBusReply<QString> HibernateInfo;
    HibernateInfo = mHibernateInterface->call("CanSuspend");
    if (HibernateInfo == "yes" || HibernateInfo == "challenge") { // bug#200148
        m_isExistSuspend = true;
    } else {
        m_isExistSuspend = false;
    }

    return m_isExistSuspend;
}

bool Power::isExitBattery()
{
    /* 默认机器没有电池 */
    mHasBat = false;
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
        mHasBat = true ;
    }

    return mHasBat;
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
    frame->setFrameShape(QFrame::NoFrame);
}

void Power::setHintLable()
{
    if ((((m_acCloseDisplayComboBox->currentData().toInt() > m_acSleepComboBox->currentData().toInt()) && m_acSleepComboBox->currentData().toInt() !=0) ||
         (m_acCloseDisplayComboBox->currentData().toInt() == 0 && m_acSleepComboBox->currentData().toInt() != 0)) && m_isExistSuspend) {

        m_acCloseDisplayHintIcon->setVisible(true);
        m_acCloseDisplayHintLabel->setVisible(true);
        m_acCloseDisplayFrame->setMinimumHeight(60);
    } else {
        m_acCloseDisplayHintIcon->setVisible(false);
        m_acCloseDisplayHintLabel->setVisible(false);
        m_acCloseDisplayFrame->setMinimumHeight(60);
    }

        // 电池区块
    if (m_batteryCloseDisplayComboBox && m_batterySleepComboBox && m_batteryCloseDisplayHintIcon && m_batteryCloseDisplayHintLabel && m_batteryCloseDisplayFrame) {
        if ((((m_batteryCloseDisplayComboBox->currentData().toInt() > m_batterySleepComboBox->currentData().toInt()) && m_batterySleepComboBox->currentData().toInt() !=0) ||
             (m_batteryCloseDisplayComboBox->currentData().toInt() == 0 && m_batterySleepComboBox->currentData().toInt() != 0)) && m_isExistSuspend) {

            m_batteryCloseDisplayHintIcon->setVisible(true);
            m_batteryCloseDisplayHintLabel->setVisible(true);
            m_batteryCloseDisplayFrame->setMinimumHeight(60);
        } else {
            m_batteryCloseDisplayHintIcon->setVisible(false);
            m_batteryCloseDisplayHintLabel->setVisible(false);
            m_batteryCloseDisplayFrame->setMinimumHeight(60);
        }
    }
}

void Power::hideComponent()
{
    m_acCloseDisplayLine->setVisible(m_isExistSuspend);
    m_acSleepFrame->setVisible(m_isExistSuspend);

    // S2H 相关仅由 CanSuspendThenHibernate 决定
    m_acHibernateFrame->setVisible(m_isExistS2H);
    m_acLineHibernate->setVisible(m_isExistS2H);

    if (mHasBat) {
        m_batterySleepFrame->setVisible(m_isExistSuspend);
        m_batteryCloseDisplayLine->setVisible(m_isExistSuspend);

        m_batteryHibernateFrame->setVisible(m_isExistS2H);
        m_batteryLineHibernate->setVisible(m_isExistS2H);
    }
}

void Power::hideComponentByConf2()
{
    QString cloudPlatForm = QString(QLatin1String(kdk_system_get_hostCloudPlatform()));
    if (cloudPlatForm == "huawei") {
        m_acPowerFrame->setVisible(false);
        m_batteryPowerFrame->setVisible(false);
        m_batteryPlanTitleLabel->setVisible(false);
        Batterywidget->setVisible(false);
        mWakenPwdFrame->setVisible(false);
        mPowerKeyFrame->setVisible(false);
        mCloseLidFrame->setVisible(false);
        m_acSleepFrame->setVisible(false);
        line_2->setVisible(false);
        line_3->setVisible(false);
        m_acCloseDisplayLine->setVisible(false);
        mResourceFrame->setVisible(false);
    }
}

void Power::setEnableBySecurity() {
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("powerEnable").toString();
    QStringList setItems = moduleSettings.split(",");
    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "mSleepPwdFrame") {
            mSleepPwdFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mWakenPwdFrame") {
            mWakenPwdFrame->setEnabled(item.at(1) == "true");
        }
    }
}

QString Power::translationPath() const { 
    return "/usr/share/ukui-power-manager/power/translations/%1.ts"; 
}

QFrame* Power::createAcSectionWidget(QWidget *parent)
{
    m_acPowerSettingsLabel = new KLabel(parent);
    m_acPowerSettingsLabel->setContentsMargins(16, 0, 0, 0);
    m_acPowerSettingsLabel->setText(tr("using AC power"));
    mverticalLayout->addWidget(m_acPowerSettingsLabel);

    m_acPowerSettingsFrame = new QFrame(parent);
    m_acPowerSettingsFrame->setMinimumSize(QSize(550, 0));
    m_acPowerSettingsFrame->setMaximumSize(QSize(16777215, 16777215));
    m_acPowerSettingsFrame->setFrameShape(QFrame::Box);

    m_acPowerSettingsLayout = new QVBoxLayout(m_acPowerSettingsFrame);
    m_acPowerSettingsLayout->setContentsMargins(0, 0, 0, 0);
    m_acPowerSettingsLayout->setSpacing(0);

    // 关闭显示器
    m_acCloseDisplayFrame = new QFrame(m_acPowerSettingsFrame);
    setFrame_Noframe(m_acCloseDisplayFrame);

    //最外层改成水平布局
    QHBoxLayout *vCloseLayout = new QHBoxLayout(m_acCloseDisplayFrame);
    vCloseLayout->setContentsMargins(16, 0, 16, 0);
   
    QVBoxLayout *mCloseLayout = new QVBoxLayout();
    mCloseLayout->setSpacing(0);
    mCloseLayout->setContentsMargins(0,0,0,0);
    mCloseLayout->addStretch();

    m_acCloseDisplayLabel = new QLabel(m_acCloseDisplayFrame);
    m_acCloseDisplayLabel->setFixedWidth(550);
    mCloseLayout->addWidget(m_acCloseDisplayLabel);

    QHBoxLayout* hintLayout = new QHBoxLayout();
    m_acCloseDisplayHintIcon = new QLabel;
    m_acCloseDisplayHintIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16, 16)));
    m_acCloseDisplayHintIcon->setAlignment(Qt::AlignVCenter); // 改为垂直居中对齐
    m_acCloseDisplayHintLabel = new LightLabel(tr("The system will sleep before turning off the display"));
    m_acCloseDisplayHintLabel->setAlignment(Qt::AlignVCenter); 
    hintLayout->addWidget(m_acCloseDisplayHintIcon);
    hintLayout->addWidget(m_acCloseDisplayHintLabel);
    hintLayout->addStretch();
    
    mCloseLayout->addLayout(hintLayout);
    mCloseLayout->addStretch();

    m_acCloseDisplayComboBox = new QComboBox(m_acCloseDisplayFrame);
    m_acCloseDisplayComboBox->setObjectName("close");
    m_acCloseDisplayComboBox->setMinimumWidth(200);

    vCloseLayout->addLayout(mCloseLayout);
    vCloseLayout->addWidget(m_acCloseDisplayComboBox,1);

    m_acPowerSettingsLayout->addWidget(m_acCloseDisplayFrame);
    m_acCloseDisplayLine = new KHLineFrame(m_acPowerSettingsFrame);
    m_acPowerSettingsLayout->addWidget(m_acCloseDisplayLine);

    // 睡眠
    m_acSleepFrame = new QFrame(m_acPowerSettingsFrame);
    setFrame_Noframe(m_acSleepFrame);

    QVBoxLayout *vSleepLayout = new QVBoxLayout(m_acSleepFrame);
    QHBoxLayout *mainSleepLayout = new QHBoxLayout();
    vSleepLayout->setContentsMargins(16, 0, 16, 0);

    m_acSleepLabel = new QLabel(m_acSleepFrame);
    m_acSleepLabel->setFixedWidth(550);

    m_acSleepComboBox = new QComboBox(m_acSleepFrame);
    m_acSleepComboBox->setObjectName("sleep");
    m_acSleepComboBox->setMinimumWidth(200);

    mainSleepLayout->addWidget(m_acSleepLabel);
    mainSleepLayout->addWidget(m_acSleepComboBox);
    vSleepLayout->addLayout(mainSleepLayout);

    m_acPowerSettingsLayout->addWidget(m_acSleepFrame);
    m_acSleepLine = new KHLineFrame(m_acPowerSettingsFrame);
    m_acPowerSettingsLayout->addWidget(m_acSleepLine);

    //S32S4
    m_acHibernateFrame = new QFrame(m_acPowerSettingsFrame);
    setFrame_Noframe(m_acHibernateFrame);

    QHBoxLayout *m_acHibernateLayout = new QHBoxLayout(m_acHibernateFrame);
    m_acHibernateLayout->setContentsMargins(16, 0, 16, 0);

    m_acHibernateLabel = new QLabel(m_acHibernateFrame);
    m_acHibernateLabel->setFixedSize(550,59);

    m_acHibernateComboBox = new QComboBox(m_acHibernateFrame);
    m_acHibernateComboBox->setObjectName("hibernate");
    m_acHibernateComboBox->setMinimumWidth(200);

    m_acHibernateLayout->addWidget(m_acHibernateLabel);
    m_acHibernateLayout->addWidget(m_acHibernateComboBox);

    m_acPowerSettingsLayout->addWidget(m_acHibernateFrame);

    m_acLineHibernate = new KHLineFrame(m_acPowerSettingsFrame);
    m_acPowerSettingsLayout->addWidget(m_acLineHibernate);

    // 电源计划
    m_acPowerFrame = new QFrame(m_acPowerSettingsFrame);
    setFrame_Noframe(m_acPowerFrame);

    QHBoxLayout *acPowerLayout = new QHBoxLayout(m_acPowerFrame);
    acPowerLayout->setContentsMargins(16, 0, 16, 0);

    m_acPowerLabel = new QLabel(m_acPowerFrame);
    m_acPowerLabel->setFixedSize(550,60);

    m_acPowerComboBox = new QComboBox(m_acPowerFrame);
    m_acPowerComboBox->setObjectName("powermode");
    m_acPowerComboBox->setMinimumWidth(200);

    acPowerLayout->addWidget(m_acPowerLabel);
    acPowerLayout->addWidget(m_acPowerComboBox);

    m_acPowerSettingsLayout->addWidget(m_acPowerFrame);
    return m_acPowerSettingsFrame;
}

QFrame* Power::createBatterySectionWidget(QWidget *parent)
{
    m_batteryPowerSettingsTitleLabel = new KLabel(parent);
    m_batteryPowerSettingsTitleLabel->setContentsMargins(16, 0, 0, 0);
    m_batteryPowerSettingsTitleLabel->setText(tr("using battery"));
    mverticalLayout->addWidget(m_batteryPowerSettingsTitleLabel);

    m_batteryPowerSettingsFrame = new QFrame(parent);
    m_batteryPowerSettingsFrame->setMinimumSize(QSize(550, 0));
    m_batteryPowerSettingsFrame->setMaximumSize(QSize(16777215, 16777215));
    m_batteryPowerSettingsFrame->setFrameShape(QFrame::Box);

    m_batteryPowerSettingsLayout = new QVBoxLayout(m_batteryPowerSettingsFrame);
    m_batteryPowerSettingsLayout->setContentsMargins(0, 0, 0, 0);
    m_batteryPowerSettingsLayout->setSpacing(0);

    // 关闭显示器
    m_batteryCloseDisplayFrame = new QFrame(m_batteryPowerSettingsFrame);
    setFrame_Noframe(m_batteryCloseDisplayFrame);

    QHBoxLayout *vCloseLayout = new QHBoxLayout(m_batteryCloseDisplayFrame);
    vCloseLayout->setContentsMargins(16, 0, 16, 0);

    QVBoxLayout *mCloseLayout = new QVBoxLayout();
    mCloseLayout->setSpacing(0);
    mCloseLayout->setContentsMargins(0,0,0,0);
    mCloseLayout->addStretch();

    m_batteryCloseDisplayLabel = new QLabel(m_batteryCloseDisplayFrame);
    m_batteryCloseDisplayLabel->setFixedWidth(550);
    mCloseLayout->addWidget(m_batteryCloseDisplayLabel);

    QHBoxLayout* hintLayout = new QHBoxLayout();
    m_batteryCloseDisplayHintIcon = new QLabel;
    m_batteryCloseDisplayHintIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16, 16)));
    m_batteryCloseDisplayHintIcon->setAlignment(Qt::AlignVCenter);
    m_batteryCloseDisplayHintLabel = new LightLabel(tr("The system will sleep before turning off the display"));
    m_batteryCloseDisplayHintLabel->setAlignment(Qt::AlignVCenter);
    hintLayout->addWidget(m_batteryCloseDisplayHintIcon);
    hintLayout->addWidget(m_batteryCloseDisplayHintLabel);
    hintLayout->addStretch();

    mCloseLayout->addLayout(hintLayout);
    mCloseLayout->addStretch();

    m_batteryCloseDisplayComboBox = new QComboBox(m_batteryCloseDisplayFrame);
    m_batteryCloseDisplayComboBox->setObjectName("close");
    m_batteryCloseDisplayComboBox->setMinimumWidth(200);

    vCloseLayout->addLayout(mCloseLayout);
    vCloseLayout->addWidget(m_batteryCloseDisplayComboBox,1);

    m_batteryPowerSettingsLayout->addWidget(m_batteryCloseDisplayFrame);
    m_batteryCloseDisplayLine = new KHLineFrame(m_batteryPowerSettingsFrame);
    m_batteryPowerSettingsLayout->addWidget(m_batteryCloseDisplayLine);

    // 睡眠
    m_batterySleepFrame = new QFrame(m_batteryPowerSettingsFrame);
    setFrame_Noframe(m_batterySleepFrame);

    QVBoxLayout *vSleepLayout = new QVBoxLayout(m_batterySleepFrame);
    QHBoxLayout *mainSleepLayout = new QHBoxLayout();
    vSleepLayout->setContentsMargins(16, 0, 16, 0);

    m_batterySleepLabel = new QLabel(m_batterySleepFrame);
    m_batterySleepLabel->setFixedWidth(550);

    m_batterySleepComboBox = new QComboBox(m_batterySleepFrame);
    m_batterySleepComboBox->setObjectName("sleep");
    m_batterySleepComboBox->setMinimumWidth(200);

    mainSleepLayout->addWidget(m_batterySleepLabel);
    mainSleepLayout->addWidget(m_batterySleepComboBox);
    vSleepLayout->addLayout(mainSleepLayout);

    m_batteryPowerSettingsLayout->addWidget(m_batterySleepFrame);
    m_batterySleepLine = new KHLineFrame(m_batteryPowerSettingsFrame);
    m_batteryPowerSettingsLayout->addWidget(m_batterySleepLine);

    //S32S4
    m_batteryHibernateFrame = new QFrame(m_batteryPowerSettingsFrame);
    setFrame_Noframe(m_batteryHibernateFrame);

    QHBoxLayout *m_batteryHibernateLayout = new QHBoxLayout(m_batteryHibernateFrame);
    m_batteryHibernateLayout->setContentsMargins(16, 0, 16, 0);

    m_batteryHibernateLabel = new QLabel(m_batteryHibernateFrame);
    m_batteryHibernateLabel->setFixedSize(550,59);

    m_batteryHibernateComboBox = new QComboBox(m_batteryHibernateFrame);
    m_batteryHibernateComboBox->setObjectName("hibernate");
    m_batteryHibernateComboBox->setMinimumWidth(200);

    m_batteryHibernateLayout->addWidget(m_batteryHibernateLabel);
    m_batteryHibernateLayout->addWidget(m_batteryHibernateComboBox);

    m_batteryPowerSettingsLayout->addWidget(m_batteryHibernateFrame);

    m_batteryLineHibernate = new KHLineFrame(m_batteryPowerSettingsFrame);
    m_batteryPowerSettingsLayout->addWidget(m_batteryLineHibernate);

    // 电池计划
    m_batteryPowerFrame = new QFrame(m_batteryPowerSettingsFrame);
    setFrame_Noframe(m_batteryPowerFrame);

    QHBoxLayout *batteryPowerLayout = new QHBoxLayout(m_batteryPowerFrame);
    batteryPowerLayout->setContentsMargins(16, 0, 16, 0);

    m_batteryPowerLabel = new QLabel(m_batteryPowerFrame);
    m_batteryPowerLabel->setFixedSize(550,59);

    m_batteryPowerComboBox = new QComboBox(m_batteryPowerFrame);
    m_batteryPowerComboBox->setObjectName("batterymode");
    m_batteryPowerComboBox->setMinimumWidth(200);

    batteryPowerLayout->addWidget(m_batteryPowerLabel);
    batteryPowerLayout->addWidget(m_batteryPowerComboBox);

    m_batteryPowerSettingsLayout->addWidget(m_batteryPowerFrame);

    return m_batteryPowerSettingsFrame;
}

bool Power::isDesktop()
{
    QFile file("/sys/class/dmi/id/chassis_type");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open chassis_type file";
        return false;
    }

    QString chassisTypeStr = file.readAll().trimmed();
    file.close();

    bool ok = false;
    int chassisTypeInt = chassisTypeStr.toInt(&ok);
    if (!ok) {
        qWarning() << "Invalid chassis_type format:" << chassisTypeStr;
        return false;
    }

    if (static_cast<DmiChassisType>(chassisTypeInt) == DmiChassisType::Desktop) {
        return true; // 台式机
    }
    return false; // 非台式机
}

QString Power::unqtify_name(const QString &name)
{
    QString result;
    result.reserve(name.length() + name.length() / 2); // 预分配空间，考虑可能添加的连字符

    for (int i = 0; i < name.length(); ++i) {
        QChar c = name.at(i);
        if (c.isUpper()) {
            if (!result.isEmpty()) {
                result.append('-');
            }
            result.append(c.toLower());
        } else {
            result.append(c);
        }
    }

    return result;
}

bool Power::isGSettingsKeyWritable(const QString &schema, const QString &key)
{
    QByteArray schemaBytes = schema.toUtf8();
    
    GSettings *gs = g_settings_new(schemaBytes.constData());
    if (!gs) {
        qWarning() << "Failed to create GSettings for schema" << schema;
        return false;
    }
    
    // 使用 unqtify_name 将 Qt 风格的键名转换为 GSettings 风格的键名
    QString newkey = Power::unqtify_name(key);
    QByteArray keyBytes = newkey.toUtf8();
    gboolean writable = g_settings_is_writable(gs, keyBytes.constData());
    
    g_object_unref(gs);
    
    return writable;
}

void Power::initKeyWidgetMapping()
{
    // 统一的键-控件映射表（所有需要管控的键都在这里）
    m_keyWidgetMappings = {
        {POWERMANAGER_SCHEMA, POWER_POLICY_AC, m_acPowerComboBox},
        {POWERMANAGER_SCHEMA, POWER_POLICY_BATTARY, m_batteryPowerComboBox},
        {POWERMANAGER_SCHEMA, BUTTON_POWER_KEY, mPowerKeyComboBox},
        {POWERMANAGER_SCHEMA, BUTTON_LID_AC_KEY, mCloseLidComboBox},
        {POWERMANAGER_SCHEMA, PER_ACTION_KEY, m_lowpowerComboBox1},
        {POWERMANAGER_SCHEMA, ACTION_CRI_BTY, mLowpowerComboBox2},
        {POWERMANAGER_SCHEMA, PERCENTAGE_LOW, mNoticeComboBox},
        {POWERMANAGER_SCHEMA, LOW_BATTERY_AUTO_SAVE, mLowSaveBtn},
        {POWERMANAGER_SCHEMA, DISPLAY_LEFT_TIME, m_remainingTimeSwitch},
        {POWERMANAGER_SCHEMA, SLEEP_DISPLAY_AC_KEY, m_acCloseDisplayComboBox},
        {POWERMANAGER_SCHEMA, SLEEP_DISPLAY_BATT_KEY, m_batteryCloseDisplayComboBox},
        {POWERMANAGER_SCHEMA, SLEEP_COMPUTER_AC_KEY, m_acSleepComboBox},
        {POWERMANAGER_SCHEMA, SLEEP_COMPUTER_BATT_KEY, m_batterySleepComboBox},
        {POWERMANAGER_SCHEMA, HIBERNATE_COMPUTER_AC_KEY, m_acHibernateComboBox},
        {POWERMANAGER_SCHEMA, HIBERNATE_COMPUTER_BATT_KEY, m_batteryHibernateComboBox},
        {SCREENSAVER_SCHEMA, SLEEP_ACTIVATION_ENABLED, mSleepPwdBtn},
        {SCREENSAVER_SCHEMA, CLOSE_ACTIVATION_ENABLED, mWakenPwdBtn},
        {PROC_MANAGER, RESOURCE_LIMIT_ENABLED, mResourceSButton},
    };
}

QWidget* Power::findWidgetByKey(const QString &schema, const QString &key)
{
    // 从统一的映射表中查找
    for (const auto &mapping : m_keyWidgetMappings) {
        if (mapping.schema == schema && mapping.key == key) {
            return mapping.widget;
        }
    }
    return nullptr;
}

void Power::updateWidgetEnabledByKey(const QString &schema, const QString &key)
{
    QWidget *widget = findWidgetByKey(schema, key);
    if (widget) {
        widget->setEnabled(isGSettingsKeyWritable(schema, key));
    }
}

void Power::setWidgetsEnabledByGSettings()
{    
    // 遍历统一的映射表，设置所有控件的启用状态
    for (const auto &mapping : m_keyWidgetMappings) {
        if (mapping.widget) {
            mapping.widget->setEnabled(isGSettingsKeyWritable(mapping.schema, mapping.key));
        }
    }
}