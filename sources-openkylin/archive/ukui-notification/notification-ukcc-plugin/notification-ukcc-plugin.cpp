/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "notification-ukcc-plugin.h"
#include "notice-menu.h"
#include <QFileDialog>
#include <QToolButton>
#include <QFrame>

#include <QtConcurrent/QtConcurrent>
#include <mutex>
#include "security-control-config.h"
#include "utils.h"
#include <ukcc/widgets/lightlabel.h>
#include "notification-settings-item.h"

using namespace UkuiNotification;

#define THEME_QT_SCHEMA  "org.ukui.style"
static std::once_flag onceFlag;
UkuiSearch::ApplicationInfo& appInfo() {
    return Utils::applicationInfo();
}

Notice::Notice()
{
    QTranslator* translator = new QTranslator(this);
    if(!translator->load("/usr/share/ukui-notification/notification-ukcc-plugin/translations/" + QLocale::system().name())) {
        qWarning() << "/usr/share/ukui-notification/notification-ukcc-plugin/translations/" + QLocale::system().name() << "load failed";
    }
    qApp->installTranslator(translator);

    m_pluginName = tr("Notice");
    m_pluginType = SYSTEM;
}

Notice::~Notice()
{
    if (m_globalSettings) {
        delete m_globalSettings;
        m_globalSettings = nullptr;
    }
}

QString Notice::plugini18nName()
{
    return m_pluginName;
}

int Notice::pluginTypes()
{
    return m_pluginType;
}

QWidget *Notice::pluginUi()
{
    std::call_once(onceFlag, [ & ] {
        if (QGSettings::isSchemaInstalled(THEME_QT_SCHEMA)) {
            QByteArray id(THEME_QT_SCHEMA);
            m_themeSetting = new QGSettings(id, QByteArray(), this);
        }
        if (!m_globalSettings) {
            m_globalSettings = new NotificationGlobalSettings();
        }

        m_pluginWidget = new QWidget;
        m_pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        initUi(m_pluginWidget);
        initSearchText();
        initNoticeStatus();
        initConnection();
        initListUI();
    });
    return m_pluginWidget;
}

const QString Notice::name() const
{
    return QStringLiteral("Notice");
}

QString Notice::translationPath() const
{
    return QStringLiteral("/usr/share/ukui-notification/notification-ukcc-plugin/translations/%1.ts");
}

bool Notice::isShowOnHomePage() const
{
    return false;
}

QIcon Notice::icon() const
{
    return QIcon::fromTheme("ukui-tool-symbolic");
}

bool Notice::isEnable() const
{
    return true;
}

QString Notice::baseAccessibleName()
{
    return tr("Do not disturb mode");
}

void Notice::initUi(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *notFazeFrame = new QFrame(widget);
    initNotFaze(notFazeFrame);

    QWidget *Noticewidget = new QWidget(widget);
    Noticewidget->setMinimumSize(QSize(550, 0));
    Noticewidget->setMaximumSize(QSize(16777215, 16777215));

    QVBoxLayout *NoticeLayout = new QVBoxLayout(Noticewidget);
    NoticeLayout->setContentsMargins(0, 0, 0, 0);
    NoticeLayout->setSpacing(8);

    m_noticeLabel = new TitleLabel(Noticewidget);
    m_notFazeLabel = new TitleLabel();
    //~ contents_path /Notice/Do not disturb mode Mode
    m_notFazeLabel->setText(baseAccessibleName());

    LightLabel *notFazeLabel = new LightLabel(tr("(Notification banners, prompts will be hidden, and notification sounds will be muted)"));
    QFrame *distrubFrame = new QFrame(widget);
    distrubFrame->setMinimumWidth(550);
    distrubFrame->setMaximumWidth(16777215);

    QHBoxLayout *distrubLyt = new QHBoxLayout(distrubFrame);
    distrubLyt->setContentsMargins(0, 0, 0, 0);
    distrubLyt->addWidget(m_notFazeLabel);
    distrubLyt->addSpacing(4);
    distrubLyt->addWidget(notFazeLabel, Qt::AlignLeft);

    m_getNoticeFrame  = new QFrame(Noticewidget);
    m_getNoticeFrame->setMinimumSize(QSize(550, 60));
    m_getNoticeFrame->setMaximumSize(QSize(16777215, 60));
    m_getNoticeFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mGetNoticeLayout = new QHBoxLayout(m_getNoticeFrame);
    mGetNoticeLayout->setContentsMargins(16,0,16,0);

    m_getNoticeLabel = new QLabel(m_getNoticeFrame);
    m_getNoticeLabel->setFixedWidth(550);
    m_getNoticeLabel->setText(tr("Get notifications from the app"));
    m_enableSwitchBtn = new KSwitchButton(m_getNoticeFrame);
    m_enableSwitchBtn->setObjectName("getnoticeinfo");
    m_enableSwitchBtn->setAccessibleName(m_getNoticeLabel->text() + tr("switch"));

    mGetNoticeLayout->addWidget(m_getNoticeLabel,Qt::AlignLeft);
    mGetNoticeLayout->addStretch();
    mGetNoticeLayout->addWidget(m_enableSwitchBtn);

    m_noticeAppFrame = new QFrame(Noticewidget);
    m_noticeAppFrame->setMinimumSize(QSize(550, 0));
    m_noticeAppFrame->setMaximumSize(QSize(16777215, 16777215));
    m_noticeAppFrame->setFrameShape(QFrame::Box);

    m_appListVerticalLayout = new QVBoxLayout(m_noticeAppFrame);
    m_appListVerticalLayout->setContentsMargins(0, 0, 0, 0);
    m_appListVerticalLayout->setSpacing(0);

    NoticeLayout->addWidget(m_noticeLabel);
    NoticeLayout->addWidget(m_getNoticeFrame);
    NoticeLayout->addWidget(m_noticeAppFrame);
    NoticeLayout->addStretch();

    if (1) {
        mverticalLayout->addWidget(distrubFrame);
        mverticalLayout->addSpacing(8);
        mverticalLayout->addWidget(notFazeFrame);
        mverticalLayout->addSpacing(32);
    } else {
        distrubFrame->hide();
        notFazeFrame->hide();
    }
    mverticalLayout->addWidget(Noticewidget);
    mverticalLayout->setSizeConstraint(QLayout::SetMinimumSize);
}

void Notice::initNotFaze(QFrame *frame)
{
    frame->setMinimumSize(QSize(550, 0));
    frame->setMaximumSize(QSize(16777215, 16777215));
    frame->setFrameShape(QFrame::Box);

    QVBoxLayout *notFazeLyt = new QVBoxLayout(frame);
    notFazeLyt->setContentsMargins(0, 0, 0, 0);
    notFazeLyt->setSpacing(0);

    QFrame *line_1 = NoticeMenu::setLine();
    QFrame *line_2 = NoticeMenu::setLine();
    QFrame *line_3 = NoticeMenu::setLine();

    QFrame *autoOpenFrame = new QFrame(frame);
    autoOpenFrame->setMinimumSize(550, 60);
    autoOpenFrame->setMaximumSize(16777215, 60);
    QHBoxLayout *autoLyt = new QHBoxLayout(autoOpenFrame);
    autoLyt->setContentsMargins(16, 0, 16, 0);
    //~ contents_path /Notice/Do not disturb mode Mode/Automatically turn on
    QLabel *autoOpenLabel = new QLabel(tr("Automatically turn on"), autoOpenFrame);
    autoOpenLabel->setAccessibleName(baseAccessibleName() + tr("auto open time hour"));

    m_openTimeHComboBox = new QComboBox(autoOpenFrame);
    m_openTimeHComboBox->setObjectName("opentimehour");
    m_openTimeHComboBox->setFixedWidth(64);
    autoOpenLabel->setBuddy(m_openTimeHComboBox);
    m_openTimeMComboBox = new QComboBox(autoOpenFrame);
    m_openTimeMComboBox->setObjectName("opentimeminute");
    m_openTimeMComboBox->setFixedWidth(64);

    m_closeTimeHComboBox = new QComboBox(autoOpenFrame);
    m_closeTimeHComboBox->setObjectName("closetimehour");
    m_closeTimeHComboBox->setFixedWidth(64);
    m_closeTimeMComboBox = new QComboBox(autoOpenFrame);
    m_closeTimeMComboBox->setObjectName("closetimeminute");
    m_closeTimeMComboBox->setFixedWidth(64);

    for (int i = 0; i < 24; i++) {
        m_openTimeHComboBox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        m_openTimeHComboBox->setItemData(i, tr("%1 o'clock").arg(i), Qt::AccessibleTextRole);
        m_closeTimeHComboBox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        m_closeTimeHComboBox->setItemData(i, tr("%1 o'clock").arg(i), Qt::AccessibleTextRole);
    }


    for (int i = 0; i < 60; i++) {
        m_openTimeMComboBox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        m_openTimeMComboBox->setItemData(i, tr("%1 minute").arg(i), Qt::AccessibleTextRole);
        m_closeTimeMComboBox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        m_closeTimeMComboBox->setItemData(i, tr("%1 minute").arg(i), Qt::AccessibleTextRole);
    }

    // hh:mm to hh:mm
    QLabel *openMinuteSeparatorLabel = new QLabel(autoOpenFrame);
    openMinuteSeparatorLabel->setFixedWidth(4);
    openMinuteSeparatorLabel->setText(":");
    openMinuteSeparatorLabel->setAccessibleName(baseAccessibleName() + tr("auto open time minute"));
    openMinuteSeparatorLabel->setBuddy(m_openTimeMComboBox);

    QLabel * timeRangeLabel = new QLabel(autoOpenFrame);
    timeRangeLabel->setFixedWidth(20);
    timeRangeLabel->setText(tr("to"));
    timeRangeLabel->setAccessibleName(baseAccessibleName() + tr("auto close time hour"));
    timeRangeLabel->setBuddy(m_closeTimeHComboBox);

    QLabel *closeMinuteSeparatorLabel = new QLabel(autoOpenFrame);
    closeMinuteSeparatorLabel->setFixedWidth(4);
    closeMinuteSeparatorLabel->setText(":");
    closeMinuteSeparatorLabel->setAccessibleName(baseAccessibleName() + tr("auto close time minute"));
    closeMinuteSeparatorLabel->setBuddy(m_closeTimeMComboBox);

    m_autoOpenSwitchBtn = new KSwitchButton(autoOpenFrame);
    m_autoOpenSwitchBtn->setObjectName("autoopen");
    m_autoOpenSwitchBtn->setAccessibleName(baseAccessibleName() + autoOpenLabel->text() + tr("switch"));

    autoLyt->addWidget(autoOpenLabel);
    autoLyt->addStretch();
    autoLyt->addWidget(m_openTimeHComboBox);
    autoLyt->addWidget(openMinuteSeparatorLabel);
    autoLyt->addWidget(m_openTimeMComboBox);
    autoLyt->addWidget(timeRangeLabel);
    autoLyt->addWidget(m_closeTimeHComboBox);
    autoLyt->addWidget(closeMinuteSeparatorLabel);
    autoLyt->addWidget(m_closeTimeMComboBox);
    autoLyt->addSpacing(24);
    autoLyt->addWidget(m_autoOpenSwitchBtn);

    QFrame *multiScreenFrame = new QFrame(frame);
    const QString multiScreenText = tr("Automatically turn on when multiple screens are connected");
    m_multiScreenSwitchBtn= new KSwitchButton();
    m_multiScreenSwitchBtn->setObjectName("multiscreen");
    m_multiScreenSwitchBtn->setAccessibleName(baseAccessibleName() + multiScreenText);
    //~ contents_path /Notice/Do not disturb mode/Automatically turn on when multiple screens are connected
    setFrame(multiScreenFrame, m_multiScreenSwitchBtn, multiScreenText);

    QFrame *fullScreenFrame = new QFrame(frame);
    const QString fullScreenText = tr("Automatically open in full screen mode");
    m_fullScreenSwitchBtn= new KSwitchButton();
    m_fullScreenSwitchBtn->setObjectName("fullscreen");
    m_fullScreenSwitchBtn->setAccessibleName(baseAccessibleName() + fullScreenText);
    //~ contents_path /Notice/Do not disturb mode/Automatically open in full screen mode
    setFrame(fullScreenFrame, m_fullScreenSwitchBtn, fullScreenText);

    QFrame *allowAlarmrRemindersFrame = new QFrame(frame);
    const QString allowAlarmText = tr("Allow automatic alarm reminders in Do Not Disturb mode");
    m_allowAlarmSwitchBtn= new KSwitchButton();
    m_allowAlarmSwitchBtn->setObjectName("allowAlarmr");
    m_allowAlarmSwitchBtn->setAccessibleName(allowAlarmText);
    //~ contents_path /Notice/Do not disturb mode/Allow automatic alarm reminders in Do Not Disturb mode
    setFrame(allowAlarmrRemindersFrame, m_allowAlarmSwitchBtn, allowAlarmText);

    notFazeLyt->addWidget(autoOpenFrame);
    notFazeLyt->addWidget(line_1);
    notFazeLyt->addWidget(multiScreenFrame);
    notFazeLyt->addWidget(line_2);
    notFazeLyt->addWidget(fullScreenFrame);
    notFazeLyt->addWidget(line_3);
    notFazeLyt->addWidget(allowAlarmrRemindersFrame);
}

void Notice::setFrame(QFrame *frame, KSwitchButton *btn, QString str)
{
    frame->setMinimumSize(QSize(550, 60));
    frame->setMaximumSize(QSize(16777215, 60));
    frame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *hLyt = new QHBoxLayout(frame);
    hLyt->setContentsMargins(16, 0, 16, 0);
    QLabel *label = new QLabel(str, frame);

    hLyt->addWidget(label);
    hLyt->addStretch();
    hLyt->addWidget(btn);
}


void Notice::initSearchText()
{
    m_noticeLabel->setText(tr("Notice Settings"));
    //~ contents_path /notice/Get notifications from the app
    m_getNoticeLabel->setText(tr("Get notifications from the app"));
}

void Notice::initConnection()
{
    if (m_globalSettings) {
        connect(m_openTimeHComboBox, &QComboBox::currentTextChanged, this, [&](const QString &text){
            //Utils::buriedSettings(name(), "open time(hour) when auto turn on do not disturb mode", QString("settings"), text);
            m_globalSettings->setScheduleTurnOnDNDTime(QTime::fromString(text + ":" + m_openTimeMComboBox->currentText(), "hh:mm"));
        });

        connect(m_openTimeMComboBox, &QComboBox::currentTextChanged, this, [&](const QString &text){
            //Utils::buriedSettings(name(), "open time(minute) when auto turn on do not disturb mode", QString("settings"), m_openTimeMCombox->currentText());
            m_globalSettings->setScheduleTurnOnDNDTime(QTime::fromString(text + ":" + m_openTimeHComboBox->currentText(), "mm:hh"));
        });

        connect(m_closeTimeHComboBox, &QComboBox::currentTextChanged, this, [&](const QString &text){
            //Utils::buriedSettings(name(), "close time(hour) when auto turn on do not disturb mode", QString("settings"), m_closeTimeMCombox->currentText());
            m_globalSettings->setScheduleTurnOffDNDTime(QTime::fromString(text + ":" + m_closeTimeMComboBox->currentText(), "hh:mm"));
        });

        connect(m_closeTimeMComboBox, &QComboBox::currentTextChanged, this, [&](const QString &text){
            //Utils::buriedSettings(name(), "close time(minute) when auto turn on do not disturb mode", QString("settings"), m_closeTimeMCombox->currentText());
            m_globalSettings->setScheduleTurnOffDNDTime(QTime::fromString(text + ":" + m_closeTimeHComboBox->currentText(), "mm:hh"));
        });

        connect(m_autoOpenSwitchBtn, &KSwitchButton::stateChanged, [&](bool state) {
            //Utils::buriedSettings(name(), "whether auto turn on do not disturb mode", QString("settings"), state ? "true" : "false");
            m_globalSettings->setScheduleTurnOnDND(state);
            setComBoxStatus(state);
        });
        connect(m_multiScreenSwitchBtn, &KSwitchButton::stateChanged, [&](bool state) {
            //Utils::buriedSettings(name(), "whether auto turn on do not disturb mode", QString("settings"), state ? "true" : "false");
            m_globalSettings->setDNDWhileMultiScreen(state);
        });
        connect(m_fullScreenSwitchBtn, &KSwitchButton::stateChanged, [&](bool state) {
            //Utils::buriedSettings(name(), "whether auto turn on do not disturb mode", QString("settings"), state ? "true" : "false");
            m_globalSettings->setDNDWhileFullScreen(state);
        });
        connect(m_allowAlarmSwitchBtn, &KSwitchButton::stateChanged, [&](bool state) {
            //Utils::buriedSettings(name(), "whether auto turn on do not disturb mode", QString("settings"), state ? "true" : "false");
            m_globalSettings->setNotifyAlarmWhileDND(state);
        });
        connect(m_enableSwitchBtn, &KSwitchButton::stateChanged, [&](bool state){
            //Utils::buriedSettings(name(), "whether to get the notification from the app", QString("settings"), state ? "true" : "false");
            m_globalSettings->setReceiveNotificationsFromApps(state);
            setHiddenNoticeApp(state);
        });

        connect(&SecurityControlConfig::instance(), &SecurityControlConfig::configChanged, this, [this](SettingsProperty::Property property) {
            if (property == SettingsProperty::ReceiveNotificationsFromApps) {
                m_enableSwitchBtn->blockSignals(true);
                m_enableSwitchBtn->setChecked(m_globalSettings->receiveNotificationsFromApps());
                m_enableSwitchBtn->blockSignals(false);
                m_enableSwitchBtn->setEnabled(SecurityControlConfig::instance().isEnabledAllNotification());
            }
        });


        connect(m_globalSettings, &NotificationGlobalSettings::scheduleTurnOnDNDChanged, this, [&](bool state) {
            m_autoOpenSwitchBtn->blockSignals(true);
            m_autoOpenSwitchBtn->setChecked(state);
            m_autoOpenSwitchBtn->blockSignals(false);
            setComBoxStatus(state);
        });
        connect(m_globalSettings, &NotificationGlobalSettings::scheduleTurnOnDNDTimeChanged, this, [&](QTime time) {
            m_openTimeHComboBox->blockSignals(true);
            m_openTimeHComboBox->setCurrentText(time.toString("hh"));
            m_openTimeHComboBox->blockSignals(false);

            m_openTimeMComboBox->blockSignals(true);
            m_openTimeMComboBox->setCurrentText(time.toString("mm"));
            m_openTimeMComboBox->blockSignals(false);
        });
        connect(m_globalSettings, &NotificationGlobalSettings::scheduleTurnOffDNDTimeChanged, this, [&](QTime time) {
            m_closeTimeHComboBox->blockSignals(true);
            m_closeTimeHComboBox->setCurrentText(time.toString("hh"));
            m_closeTimeHComboBox->blockSignals(false);

            m_closeTimeMComboBox->blockSignals(true);
            m_closeTimeMComboBox->setCurrentText(time.toString("mm"));
            m_closeTimeMComboBox->blockSignals(false);
        });
        connect(m_globalSettings, &NotificationGlobalSettings::DNDWhileMultiScreenChanged, this, [&](bool state) {
            m_multiScreenSwitchBtn->blockSignals(true);
            m_multiScreenSwitchBtn->setChecked(state);
            m_multiScreenSwitchBtn->blockSignals(false);
        });
        connect(m_globalSettings, &NotificationGlobalSettings::DNDWhileFullScreenChanged, this, [&](bool state) {
            m_fullScreenSwitchBtn->blockSignals(true);
            m_fullScreenSwitchBtn->setChecked(state);
            m_fullScreenSwitchBtn->blockSignals(false);
        });
        connect(m_globalSettings, &NotificationGlobalSettings::notifyAlarmWhileDNDChanged, this, [&](bool state) {
            m_allowAlarmSwitchBtn->blockSignals(true);
            m_allowAlarmSwitchBtn->setChecked(state);
            m_allowAlarmSwitchBtn->blockSignals(false);
        });
        connect(m_globalSettings, &NotificationGlobalSettings::receiveNotificationsFromAppsChanged, this, [&](bool state) {
            Q_UNUSED(state);
            m_enableSwitchBtn->blockSignals(true);
            m_enableSwitchBtn->setChecked(m_globalSettings->receiveNotificationsFromApps());
            m_enableSwitchBtn->blockSignals(false);
            setHiddenNoticeApp(m_globalSettings->receiveNotificationsFromApps());
        });
    }
}

void Notice::initNoticeStatus()
{
    if (m_globalSettings) {
        m_autoOpenSwitchBtn->blockSignals(true);
        m_autoOpenSwitchBtn->setChecked(m_globalSettings->scheduleTurnOnDND());
        setComBoxStatus(m_autoOpenSwitchBtn->isChecked());
        m_autoOpenSwitchBtn->blockSignals(false);

        QTime openTime = m_globalSettings->scheduleTurnOnDNDTime();
        m_openTimeHComboBox->blockSignals(true);
        m_openTimeHComboBox->setCurrentText(openTime.toString("hh"));
        m_openTimeHComboBox->blockSignals(false);
        m_openTimeMComboBox->blockSignals(true);
        m_openTimeMComboBox->setCurrentText(openTime.toString("mm"));
        m_openTimeMComboBox->blockSignals(false);

        QTime closeTime = m_globalSettings->scheduleTurnOffDNDTime();
        m_closeTimeHComboBox->blockSignals(true);
        m_closeTimeHComboBox->setCurrentText(closeTime.toString("hh"));
        m_closeTimeHComboBox->blockSignals(false);
        m_closeTimeMComboBox->blockSignals(true);
        m_closeTimeMComboBox->setCurrentText(closeTime.toString("mm"));
        m_closeTimeMComboBox->blockSignals(false);

        m_multiScreenSwitchBtn->blockSignals(true);
        m_multiScreenSwitchBtn->setChecked(m_globalSettings->DNDWhileMultiScreen());
        m_multiScreenSwitchBtn->blockSignals(false);
        m_fullScreenSwitchBtn->blockSignals(true);
        m_fullScreenSwitchBtn->setChecked(m_globalSettings->DNDWhileFullScreen());
        m_fullScreenSwitchBtn->blockSignals(false);
        m_allowAlarmSwitchBtn->blockSignals(true);
        m_allowAlarmSwitchBtn->setChecked(m_globalSettings->notifyAlarmWhileDND());
        m_allowAlarmSwitchBtn->blockSignals(false);

        m_enableSwitchBtn->blockSignals(true);
        m_enableSwitchBtn->setChecked(m_globalSettings->receiveNotificationsFromApps());
        m_enableSwitchBtn->blockSignals(false);
        m_enableSwitchBtn->setEnabled(SecurityControlConfig::instance().isEnabledAllNotification());
    }

    setHiddenNoticeApp(m_enableSwitchBtn->isChecked());
}

void Notice::initListUI()
{
    const UkuiSearch::ApplicationProperties& properties = {
            UkuiSearch::ApplicationProperty::LocalName,
            UkuiSearch::ApplicationProperty::Icon,
            UkuiSearch::ApplicationProperty::NameEn
    };

    connect(this, &Notice::appDataInfo, this, &Notice::addItemUi, Qt::QueuedConnection);

    (void)QtConcurrent::run([=](){
        auto convertAppinfoMap = [](const UkuiSearch::ApplicationInfoMap& appInfoMap) -> UkuiSearch::ApplicationInfoMap
        {
            //全路径desktop map， 转换为desktop文件名的 map
            UkuiSearch::ApplicationInfoMap newAppInfoMap;
            for (auto it = appInfoMap.begin(); it != appInfoMap.end(); ++it) {
                QString newKey = Utils::desktopEntry2Name(it.key());
                if (!newAppInfoMap.contains(newKey)) {
                    newAppInfoMap.insert(newKey, it.value());
                }
            }
            return newAppInfoMap;
        };

        const UkuiSearch::ApplicationInfoMap& appInfoMap = appInfo().getInfo(properties);
        Q_EMIT appDataInfo(convertAppinfoMap(appInfoMap));
    });

    connect(ApplicationsSettings::self(), &ApplicationsSettings::applicationInstalled,
            this, [=](const QString &appDesktopName){

        const UkuiSearch::ApplicationProperties& properties = {
                UkuiSearch::ApplicationProperty::LocalName,
                UkuiSearch::ApplicationProperty::Icon,
                UkuiSearch::ApplicationProperty::NameEn

        };

        (void)QtConcurrent::run([=](){
            UkuiSearch::ApplicationPropertyMap dataMap = appInfo().getInfo(Utils::desktopEntryFromName(appDesktopName) , properties);
            UkuiSearch::ApplicationInfoMap appInfoMap = {{appDesktopName, dataMap}};
            Q_EMIT appDataInfo(appInfoMap);
        });
    });
}

void Notice::addItemUi(const UkuiSearch::ApplicationInfoMap &appInfoMap)
{
    if (appInfoMap.count() == 1) {
        QString desktopEntry = appInfoMap.keys().at(0);
        UkuiSearch::ApplicationPropertyMap dataMap = appInfoMap.value(desktopEntry);
        if (!m_appList.contains(desktopEntry)) {
            initItemUi(desktopEntry, dataMap,
                       ApplicationsSettings::self()->creatSettings(desktopEntry));
            m_appList.append(desktopEntry);
        }
    } else {
        ApplicationsSettingsMap dataMap = ApplicationsSettings::self()->getAllApplicationsSettings();
        ApplicationsSettingsMap::const_iterator i = dataMap.constBegin();
        m_appList.clear();
        while(i != dataMap.constEnd()) {
            const QString& desktopPath = i.key();

            if (appInfoMap.contains(desktopPath)) {
                initItemUi(desktopPath, appInfoMap[desktopPath], i.value());
                m_appList.append(desktopPath);
            } else {
                qWarning() << "Can't find application info for: " << desktopPath;
            }
            ++i;
        }
    }
}

void Notice::initItemUi(const QString &desktopPath, const UkuiSearch::ApplicationPropertyMap& properties, SingleApplicationSettings *settings)
{
    const QString localName = properties[UkuiSearch::ApplicationProperty::LocalName].toString();
    const QString iconName = properties[UkuiSearch::ApplicationProperty::Icon].toString();
    const QString nameEn = properties[UkuiSearch::ApplicationProperty::NameEn].toString();

    auto* notificationItem = new NotificationSettingsItem(localName, nameEn, settings, m_noticeAppFrame);
    notificationItem->setIcon(iconName);
    connect(m_themeSetting, &QGSettings::changed, notificationItem, [notificationItem, iconName](const QString &key){
        if (key == "iconThemeName") {
            notificationItem->setIcon(iconName);
        }
    });
    m_appListVerticalLayout->addWidget(notificationItem);

    if (m_appListVerticalLayout->count() >= 2) {
        QFrame *line = new QFrame(m_noticeAppFrame);
        line->setMinimumSize(QSize(0, 1));
        line->setMaximumSize(QSize(16777215, 1));
        line->setLineWidth(0);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        m_appListVerticalLayout->insertWidget(m_appListVerticalLayout->count() - 1, line);
    }

    connect(settings, &SingleApplicationSettings::uninstalled, this, [=]() {
        for (int i = 0; i < m_appList.length(); i++) {
            if (desktopPath == m_appList.at(i)) {
                m_appList.removeAt(i);
                if (i == 0) {
                    removeLayoutItem(i);
                    removeLayoutItem(i);
                } else {
                    removeLayoutItem(i * 2 - 1);
                    removeLayoutItem(i * 2 - 1);
                }
            }
        }
    });
}

void Notice::setHiddenNoticeApp(bool status)
{
    m_noticeAppFrame->setVisible(status);
}

void Notice::setComBoxStatus(bool status)
{
    m_openTimeHComboBox->setEnabled(status);
    m_closeTimeHComboBox->setEnabled(status);
    m_openTimeMComboBox->setEnabled(status);
    m_closeTimeMComboBox->setEnabled(status);
}

void Notice::removeLayoutItem(int i)
{
    if ((i >= 0)&&(m_appListVerticalLayout->count() > i)) {
        QWidget* widget = m_appListVerticalLayout->itemAt(i)->widget();
        if (widget) {
            m_appListVerticalLayout->removeWidget(widget);
            widget->deleteLater();
        }
    }
}