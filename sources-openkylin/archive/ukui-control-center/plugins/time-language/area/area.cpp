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
#include "area.h"
#include "languagefeatureconfig.h"
#include <QCoreApplication>
#include <QPushButton>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

namespace {
const char kLanguageFeaturesGroup[] = "Area/LanguageFeatures";
const char kInstallLanguageFeaturesAction[] = "installLanguageFeatures";
const char kPendingRestartPromptKey[] = "pendingRestartPrompt";
const char kPendingRestartPromptBootIdKey[] = "pendingRestartPromptBootId";

// 获取当前系统启动 ID
QString currentBootId()
{
    QFile file(QStringLiteral("/proc/sys/kernel/random/boot_id"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromLatin1(file.readAll()).trimmed();
}

// 获取网络不可用提示文案
QString networkUnavailableText()
{
    return QCoreApplication::translate("LanguageFeatureItem",
                                       "Unable to connect to the network. Please reconnect and retry installation.");
}

// 转换保存的语言特性状态
LanguageFeatureState featureStateFromInt(int value)
{
    switch (value) {
    case static_cast<int>(LanguageFeatureState::Installing):
        return LanguageFeatureState::Installing;
    case static_cast<int>(LanguageFeatureState::Installed):
        return LanguageFeatureState::Installed;
    case static_cast<int>(LanguageFeatureState::Error):
        return LanguageFeatureState::Error;
    default:
        return LanguageFeatureState::NotInstalled;
    }
}

// 还原安装中状态为未安装
LanguageFeatureState normalizeRestoredFeatureState(LanguageFeatureState state)
{
    if (state == LanguageFeatureState::Installing) {
        return LanguageFeatureState::NotInstalled;
    }
    return state;
}

// 添加去重后的软件包名
void appendPackageName(QStringList *packageNames, const QString &packageName)
{
    if (!packageNames) {
        return;
    }
    const QString normalizedPackageName = packageName.trimmed();
    if (!normalizedPackageName.isEmpty()
            && !packageNames->contains(normalizedPackageName)) {
        packageNames->append(normalizedPackageName);
    }
}

// 获取实际使用的软件包名
QString effectivePackageName(const QString &resultPackageName,
                             const QString &optionPackageName)
{
    const QString normalizedResultPackageName = resultPackageName.trimmed();
    if (!normalizedResultPackageName.isEmpty()) {
        return normalizedResultPackageName;
    }
    return optionPackageName.trimmed();
}

// 统一语言编码格式
QString normalizedLanguageCode(QString languageCode)
{
    languageCode = languageCode.trimmed();
    languageCode.replace(QLatin1Char('-'), QLatin1Char('_'));
    const int encodingIndex = languageCode.indexOf(QLatin1Char('.'));
    if (encodingIndex > 0) {
        languageCode = languageCode.left(encodingIndex);
    }
    return languageCode;
}

// 判断软件包是否缺失
bool isMissingPackage(const QString &packageName)
{
    const QString normalizedPackageName = packageName.trimmed();
    return !normalizedPackageName.isEmpty()
            && !LanguageInstall::isPackageInstalled(normalizedPackageName);
}
} // namespace

Area::Area()
    : mFirstLoad(true)
{
    qRegisterMetaType<LanguageFeatureType>("LanguageFeatureType");
    qRegisterMetaType<PackageInstallResult>("PackageInstallResult");
    qRegisterMetaType<LanguageFeatureInstallTask>("LanguageFeatureInstallTask");
    m_languageInstall = new LanguageInstall(this);
    connect(m_languageInstall,
            &LanguageInstall::installFinished,
            this,
            &Area::onLanguageFeatureInstallFinished);
    pluginName = tr("Area");
    pluginType = DATETIME;
}

Area::~Area()
{
    if (areaWidget) {
        delete areaWidget;
        areaWidget = nullptr;
    }
    if (areaInterface) {
        delete areaInterface;
        areaInterface = nullptr;
    }
}

// 处理外部传入的插件事件
bool Area::event(QEvent *event)
{
    if (event->type() == UkccArgEvent::TYPE) {
        UkccArgEvent *argEvent = dynamic_cast<UkccArgEvent *>(event);
        if (argEvent != nullptr && argEvent->str() == QLatin1String(kInstallLanguageFeaturesAction)) {
            showCurrentLanguageFeatures();
            return true;
        }
    }
    return QObject::event(event);
}

void Area::cloudChangedSlot(const QString &key)
{
    if(key == QString("area")) {
        initCountry();
        initCalendar();
        initFirstDay();
#ifdef Nile
        initDateComboBox();
#else
        initDateFormat();
#endif
        initTimeFormat();
    }
}

void Area::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Area::cloudChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

QString Area::plugini18nName()
{
    return pluginName;
}

int Area::pluginTypes()
{
    return pluginType;
}

QWidget *Area::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad    = false;
        areaWidget    = new AreaUi;
        areaInterface = new QDBusInterface("org.ukui.ukcc.session",
                                           "/Area",
                                           "org.ukui.ukcc.session.Area",
                                           QDBusConnection::sessionBus(), this);

        if (areaInterface->isValid()) { // 判断服务是否存在
            QDBusMessage message = areaInterface->call("ping"); // 判断dbus路径是否存在
            if (message.type() == QDBusMessage::ErrorMessage && message.errorMessage().contains("No such object path", Qt::CaseInsensitive)) {
                qWarning()<<areaInterface<<":"<<message.errorMessage();
            } else {
                initContent();
                connectToServer();
                if (UkccCommon::isTablet()) {
                    areaWidget->settingForIntel();
                }
                QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                                      "/Area",
                                                      "org.ukui.ukcc.session.Area",
                                                      "changed",
                                                      this,
                                                      SLOT(dataChanged(QString)));
            }

        } else {
            qCritical() << "org.ukui.ukcc.session.Area DBus error:" << areaInterface->lastError();
        }

    } else {
        // 有可能修改了日期，因此重新加载日期格式
        qDebug() << "before initDateFormat" << areaInterface->isValid();
        if (areaInterface->isValid()) {
            initDateFormat(false);
            if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
                initLanguageFormat();
            }
        }
    }
    return areaWidget;
}

const QString Area::name() const
{
    return QStringLiteral("Area");
}

bool Area::isShowOnHomePage() const
{
    return true;
}

QIcon Area::icon() const
{
    return QIcon::fromTheme("ukui-area-symbolic");
}

bool Area::isEnable() const
{
    return true;
}

void Area::dataChanged(QString key)
{
    if (m_pendingSelfChangedKeys.removeOne(key)) {
        changeKey.clear();
        return;
    }

    if (changeKey != key) {
        if (key == QString("formatCountry")) {
            initCountry();
        } else if (key == QString("calendar")) {
            initCalendar();
        } else if (key == QString("firstDay")) {
            initFirstDay();
        } else if (key == QString("dateFormat")) {
#ifndef Nile
            initDateFormat();
#endif
        } else if (key == QString("timeFormat")) {
#ifndef Nile
            initTimeFormat();
#endif
        } else if (key == QString("showLanguageList") || key == QString("language")) {
            initLanguage();
        } else if (key == "iconThemeName") {
            areaWidget->reloadLanguageFrameIcon();
        }
    }
    changeKey = "";
}

// 忽略本次主动写入触发的数据变化
void Area::ignoreNextDataChanged(const QString &key)
{
    if (key.isEmpty()) {
        return;
    }

    m_pendingSelfChangedKeys.append(key);
    QTimer::singleShot(2000, this, [this, key]() {
        m_pendingSelfChangedKeys.removeOne(key);
        if (changeKey == key) {
            changeKey.clear();
        }
    });
}

void Area::initContent()
{
    initCountry();
    initCalendar();
    initFirstDay();
    initDateFormat();
    initTimeFormat(true);
    if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
        initLanguageFormat();
    }
    initLanguage();
    initAddLanguage();
#ifdef Nile
    initDateComboBox();
#endif
    initConnect();
}

InteractiveInfo Area::initInteractiveInfo(int orderNum)
{
    InteractiveInfo info;
    if (orderNum == AreaUi::COUNTRY) {
        info.listKey = QString("formatCountryList");
        info.key     = QString("formatCountry");
        info.setkey  = QString("setFormatCountry");
        info.list    = {
                        {QString("en_US.UTF-8"), tr("English  (US)")},
                        {QString("zh_CN.UTF-8"), tr("Simplified Chinese  (CN)")},
#ifndef Nile
                        {QString("bo_CN.UTF-8"), tr("Tibetan  (CN)")}
#else
                        {QString("bo_CN.UTF-8"), tr("Tibetan  (CN)")},
                        {QString("ug_CN.UTF-8"), tr("Uygur  (CN)")},
                        {QString("zh_HK.UTF-8"), tr("Traditional Chinese  (CN)")},
                        {QString("mn_MN.UTF-8"), tr("Mongolian  (CN)")},
                        {QString("kk_KZ.UTF-8"), tr("Kazaqa  (KZ)")},
                        {QString("ky_KG.UTF-8"), tr("Kirghiz  (KG)")},
                        {QString("de_DE.UTF-8"), tr("German  (DE)")},
                        {QString("es_ES.UTF-8"), tr("Spanish  (ES)")},
                        {QString("fr_FR.UTF-8"), tr("French  (FR)")}
#endif
                       };
        if (UkccCommon::isV11()) {
            info.list.append({QString("mn_MN.UTF-8"), tr("Mongolian  (CN)")});
            info.list.append({QString("zh_HK.UTF-8"), tr("Traditional Chinese  (CN)")});
            qDebug() << Q_FUNC_INFO << "add formatCountList" << info.list;
        }
    } else if (orderNum == AreaUi::CALENDAR) {
        info.listKey = QString("calendarList");
        info.key     = QString("calendar");
        info.setkey  = QString("setCalendar");
        info.list    = {{QString("solarlunar"), tr("Solar calendar")},
                        {QString("lunar"),      tr("Lunar")}};
    } else if (orderNum == AreaUi::FIRSTDAY) {
        info.listKey = QString("firstDayList");
        info.key     = QString("firstDay");
        info.setkey  = QString("setFirstDay");
        info.list    = {{QString("monday"), tr("Monday")},
                        {QString("sunday"), tr("Sunday")}};
    } else if (orderNum == AreaUi::DATEFORMAT) {
        info.listKey = QString("dateFormatList");
        info.key     = QString("dateFormat");
        info.setkey  = QString("setDateFormat");
        info.list    = {{QString("cn"), QDateTime::currentDateTime().toString("yyyy/MM/dd")},
                        {QString("en"), QDateTime::currentDateTime().toString("yyyy-MM-dd")}};
    } else if (orderNum == AreaUi::TIMEFORMAT) {
        info.listKey = QString("timeFormatList");
        info.key     = QString("timeFormat");
        info.setkey  = QString("setTimeFormat");
        info.list    = {{QString("12"), tr("12 Hours")},
                        {QString("24"), tr("24 Hours")}};
    }
    return info;
}

void Area::initComboBox(QComboBox *comboBox, InteractiveInfo info)
{
    if (!comboBox) {
        return;
    }
    comboBox->disconnect();
    comboBox->clear();
    QStringList dataList = {};
    for (pairString s : info.list) {
        dataList.append(s.first);
    }
    for (QString s : areaInterface->property(info.listKey.toUtf8().data()).toStringList()) {
        bool addFlag = false;
        for (int i = 0; i < dataList.size(); ++i) {
            if (dataList.at(i) == s) {
                comboBox->addItem(info.list.at(i).second, info.list.at(i).first);
                addFlag = true;
                break;
            }
        }
        if (!addFlag) {
             comboBox->addItem(s, s);
        }
    }
    int index = comboBox->findData(areaInterface->property(info.key.toUtf8().data()).toString());
    comboBox->setCurrentIndex(index < 0 ? 0 : index);
    connect(comboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            areaWidget,
            [=](int num) {
                Q_UNUSED(num);
                changeKey = info.key;
                areaInterface->call(info.setkey.toUtf8().data(), comboBox->currentData().toString());
                UkccCommon::buriedSettings(name(), info.setkey, QString("select"), comboBox->currentData().toString());
                if (info.setkey == QString("setFormatCountry")) {
                    showMessageBox(1);
                }
                if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
                    initLanguageFormat();
                }
    });
}

void Area::initCountry()
{
    initComboBox(areaWidget->countryComboBox(), initInteractiveInfo(AreaUi::COUNTRY));
}

void Area::initCalendar()
{
    initComboBox(areaWidget->calendarComboBox(), initInteractiveInfo(AreaUi::CALENDAR));
}

void Area::initFirstDay()
{
    initComboBox(areaWidget->firstDayComboBox(), initInteractiveInfo(AreaUi::FIRSTDAY));
}

void Area::initLanguageFormat() {
    QString enterTime = kdk_system_second();
    QString longWeek = kdk_system_longweek();
    QString calendar = areaWidget->calendarComboBox()->currentText();
    QString lunarDate = areaInterface->property("lunarDate").toString();
    if (calendar == tr("Solar calendar")) {
        lunarDate.clear();
    }
    if (!lunarDate.isEmpty()) {
        lunarDate.append("  ");
    }
    QString languageFormatLabel = areaWidget->longDateComboBox()->currentText() + "  "
            + areaWidget->shortDateComboBox()->currentText() + "  "
            + lunarDate + longWeek + "  " + enterTime;
    areaWidget->formatDateValueLabel()->setText(languageFormatLabel);
    qDebug() << Q_FUNC_INFO << "initLanguageFormat" << languageFormatLabel << calendar << lunarDate;
}

void Area::initDateFormat(bool firstLoad)
{
#ifdef Nile
    qDebug() << Q_FUNC_INFO << "initDateFormat firstLoad" << firstLoad;
    QString currentsecStr;
    QDateTime current = QDateTime::currentDateTime();
    int longIndex = -1;
    int shortIndex = -1;
    if (!firstLoad) {
        areaWidget->shortDateComboBox()->blockSignals(true);
        shortIndex = areaWidget->shortDateComboBox()->currentIndex();
        areaWidget->shortDateComboBox()->clear();
        qDebug() << Q_FUNC_INFO << "shortDateComboBox index" << shortIndex;
    }
    if (areaWidget->countryComboBox()->currentIndex() == 0) {
        currentsecStr = current.toString("M.d.yy");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("M/d/yy");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("M-d-yy");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("MM.dd.yyyy");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("MM/dd/yyyy");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("MM-dd-yyyy");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);
    } else {
        currentsecStr = current.toString("yy.M.d");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("yy/M/d");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("yy-M-d");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("yyyy.MM.dd");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("yyyy/MM/dd");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);

        currentsecStr = current.toString("yyyy-MM-dd");
        areaWidget->shortDateComboBox()->addItem(currentsecStr);
    }

    QLocale locale = QLocale::system();
    if ("zh_CN" == locale.name()){
        locale = QLocale::Chinese;
    } else if ("th_TH" == locale.name()) {
        locale = QLocale::Thai;
    } else if ("ms_MY" == locale.name()) {
        locale = QLocale::Malay;
    } else {
        locale = QLocale::English;
    }
    if ("ms_MY" == locale.name()) {
        currentsecStr = locale.toString(current, tr("dd MMMM, yyyy"));
    } else {
        currentsecStr = locale.toString(current, tr("MMMM dd, yyyy"));
    }
    qDebug() << Q_FUNC_INFO << "initDateFormat longDateComboBox item1" << currentsecStr << "locale.name" << locale.name();
    if (!firstLoad) {
        areaWidget->longDateComboBox()->blockSignals(true);
        longIndex = areaWidget->longDateComboBox()->currentIndex();
        areaWidget->longDateComboBox()->clear();
        qDebug() << Q_FUNC_INFO << "longDateComboBox index" << longIndex;
    }
    areaWidget->longDateComboBox()->addItem(currentsecStr);
    if ("ms_MY" == locale.name()) {
        currentsecStr = locale.toString(current, tr("d MMMM, yy"));
    } else {
        currentsecStr = locale.toString(current, tr("MMMM d, yy"));
    }
    qDebug() << Q_FUNC_INFO << "initDateFormat longDateComboBox item2" << currentsecStr << "locale.name" << locale.name();
    areaWidget->longDateComboBox()->addItem(currentsecStr);
    if (firstLoad) {
        initDateComboBox();
    } else {
        if (shortIndex >= 0 && shortIndex < areaWidget->shortDateComboBox()->count()) {
            areaWidget->shortDateComboBox()->setCurrentIndex(shortIndex);
        }
        if (longIndex >= 0 && longIndex < areaWidget->longDateComboBox()->count()) {
            areaWidget->longDateComboBox()->setCurrentIndex(longIndex);
        }
        qDebug() << Q_FUNC_INFO << "shortDate" << areaWidget->shortDateComboBox()->currentText() << "longDate" << areaWidget->longDateComboBox()->currentText();
        areaWidget->longDateComboBox()->blockSignals(false);
        areaWidget->shortDateComboBox()->blockSignals(false);
    }
#else
    initComboBox(areaWidget->dateComboBox(), initInteractiveInfo(AreaUi::DATEFORMAT));
#endif
}

void Area::initTimeFormat(bool firstLoad)
{
#ifndef Nile
    Q_UNUSED(firstLoad)
    initComboBox(areaWidget->timeComboBox(), initInteractiveInfo(AreaUi::TIMEFORMAT));
#else
    if(firstLoad) {
        areaWidget->timeComboBox()->addItem(tr("12 Hours"), "12");
        areaWidget->timeComboBox()->addItem(tr("24 Hours"), "24");
    }

    QString hourFormat;

    hourFormat = kdk_system_get_now_timeformat();
    qDebug() << Q_FUNC_INFO << "initTimeFormat kdk_system_get_now_timeformat" << hourFormat;
    if (hourFormat.contains("24")) {
        hourFormat = "24";
    } else {
        hourFormat = "12";
    }
    qDebug() << Q_FUNC_INFO << "initTimeFormat hourFormat" << hourFormat;

    if (firstLoad) {
        InteractiveInfo info = initInteractiveInfo(AreaUi::TIMEFORMAT);
        QString orgGsettingFormat = areaInterface->property(info.key.toUtf8().data()).toString().left(2);
        if (hourFormat != orgGsettingFormat) {
            hourFormat = orgGsettingFormat;
            if (hourFormat == "24") {
                kdk_system_set_24_timeformat();
            } else {
                kdk_system_set_12_timeformat();
            }
        }
    }


    if ("24" == hourFormat) {
        areaWidget->timeComboBox()->setCurrentIndex(1);
    } else {
        areaWidget->timeComboBox()->setCurrentIndex(0);
    }
    if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
        initLanguageFormat();
    }
#endif
}

void Area::initLanguage()
{
    areaWidget->clearShowLanguage();
    areaWidget->hideLanguageRestartTip();
    m_pendingInstallResults.clear();
    m_pendingRestartPromptInstallCounts.clear();
    if (m_languageInstall) {
        m_languageInstall->clear();
    }

    QStringList showLanguageList = areaInterface->property("showLanguageList" ).toStringList();
    QStringList languageList     = areaInterface->property("languageList").toStringList();
    QString     currentLanguage  = areaInterface->property("language" ).toString();
    // Gsetting为空，自动设置显示选中的语言
    if (showLanguageList.isEmpty()) {
        showLanguageList.append(currentLanguage);
        changeKey = QString("showLanguageList");
        areaInterface->call("setShowLanguageList", showLanguageList);
        areaInterface->blockSignals(false);
    }

    QStringList comboCodes;
    QStringList comboNames;
    for (const QString &language : showLanguageList) {
        if (!languageList.contains(language)) {
            continue;
        }
        comboCodes.append(language);
        comboNames.append(showLanguageName(language));
        restoreLanguageFrame(language, showLanguageName(language), language == currentLanguage);
    }
    areaWidget->updateCurrentLanguageCombo(comboCodes, comboNames, currentLanguage);
    areaWidget->collapseAllLanguageFrames();
}

// 保存语言特性安装状态
void Area::saveLanguageFeatureConfig(const QString &languageCode,
                                     const InstallLanguageResult &result,
                                     LanguageFeatureState fontState,
                                     LanguageFeatureState inputMethodState)
{
    QSettings settings;
    settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
    const QVariantMap oldMap = settings.value(languageCode).toMap();
    QVariantMap map;
    map.insert(QStringLiteral("languageName"), result.languageName);
    map.insert(QStringLiteral("setCurrentLanguage"), result.setCurrentLanguage);
    map.insert(QStringLiteral("installLanguageFont"), result.installLanguageFont);
    map.insert(QStringLiteral("installInputMethod"), result.installInputMethod);
    map.insert(QStringLiteral("inputMethodId"), result.inputMethodId);
    map.insert(QStringLiteral("inputMethodPackageName"), result.inputMethodPackageName);
    map.insert(QStringLiteral("fontPackageName"), result.fontPackageName);
    map.insert(QStringLiteral("fontState"), static_cast<int>(fontState));
    map.insert(QStringLiteral("inputMethodState"), static_cast<int>(inputMethodState));
    if (oldMap.value(QString::fromLatin1(kPendingRestartPromptKey)).toBool()) {
        map.insert(QString::fromLatin1(kPendingRestartPromptKey), true);
        const QString bootId = oldMap.value(QString::fromLatin1(kPendingRestartPromptBootIdKey)).toString();
        if (!bootId.isEmpty()) {
            map.insert(QString::fromLatin1(kPendingRestartPromptBootIdKey), bootId);
        }
    }
    settings.setValue(languageCode, map);
    settings.endGroup();
}

// 记录语言重启提示状态
void Area::setLanguageRestartPromptPending(const QString &languageCode, bool pending)
{
    if (languageCode.isEmpty()) {
        return;
    }

    QSettings settings;
    settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
    QVariantMap map = settings.value(languageCode).toMap();
    if (pending) {
        map.insert(QString::fromLatin1(kPendingRestartPromptKey), true);
        const QString bootId = currentBootId();
        if (!bootId.isEmpty()) {
            map.insert(QString::fromLatin1(kPendingRestartPromptBootIdKey), bootId);
        }
    } else {
        map.remove(QString::fromLatin1(kPendingRestartPromptKey));
        map.remove(QString::fromLatin1(kPendingRestartPromptBootIdKey));
    }
    if (map.isEmpty()) {
        settings.remove(languageCode);
    } else {
        settings.setValue(languageCode, map);
    }
    settings.endGroup();
}

// 判断语言重启提示是否待处理
bool Area::isLanguageRestartPromptPending(const QString &languageCode)
{
    if (languageCode.isEmpty()) {
        return false;
    }

    QSettings settings;
    settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
    const QVariantMap map = settings.value(languageCode).toMap();
    settings.endGroup();
    if (!map.value(QString::fromLatin1(kPendingRestartPromptKey)).toBool()) {
        return false;
    }

    const QString savedBootId = map.value(QString::fromLatin1(kPendingRestartPromptBootIdKey)).toString();
    const QString bootId = currentBootId();
    if (savedBootId.isEmpty() || (!bootId.isEmpty() && savedBootId != bootId)) {
        setLanguageRestartPromptPending(languageCode, false);
        return false;
    }

    return true;
}

// 判断语言重启是否已生效
bool Area::isLanguageRestartEffective(const QString &languageCode) const
{
    return normalizedLanguageCode(QLocale::system().name()) == normalizedLanguageCode(languageCode);
}

// 判断语言功能状态是否允许提示重启
bool Area::isLanguageRestartPromptReady(const LanguageFeatureOption &option,
                                        const InstallLanguageResult &result,
                                        LanguageFeatureState fontState,
                                        LanguageFeatureState inputMethodState) const
{
    if (option.showLanguageFont
            && result.installLanguageFont
            && (fontState == LanguageFeatureState::Installing
                || fontState == LanguageFeatureState::NotInstalled)) {
        return false;
    }

    if (option.showInputMethod
            && result.installInputMethod
            && (inputMethodState == LanguageFeatureState::Installing
                || inputMethodState == LanguageFeatureState::NotInstalled)) {
        return false;
    }

    return true;
}

// 恢复待展示的语言重启提示
void Area::updatePendingLanguageRestartTip(const QString &languageCode,
                                           const LanguageFeatureOption &option,
                                           const InstallLanguageResult &result,
                                           LanguageFeatureState fontState,
                                           LanguageFeatureState inputMethodState)
{
    if (!isLanguageRestartPromptPending(languageCode)) {
        return;
    }

    if (isLanguageRestartEffective(languageCode)) {
        setLanguageRestartPromptPending(languageCode, false);
        return;
    }

    if (isLanguageRestartPromptReady(option, result, fontState, inputMethodState)) {
        areaWidget->showLanguageRestartTip();
    }
}

// 更新已保存的单项语言特性状态
void Area::updateSavedFeatureState(const QString &languageCode,
                                   LanguageFeatureType type,
                                   LanguageFeatureState state)
{
    QSettings settings;
    settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
    const QVariantMap map = settings.value(languageCode).toMap();
    settings.endGroup();
    if (map.isEmpty()) {
        return;
    }

    InstallLanguageResult result;
    result.languageCode = languageCode;
    result.languageName = map.value(QStringLiteral("languageName")).toString();
    result.setCurrentLanguage = map.value(QStringLiteral("setCurrentLanguage")).toBool();
    result.installLanguageFont = map.value(QStringLiteral("installLanguageFont")).toBool();
    result.installInputMethod = map.value(QStringLiteral("installInputMethod")).toBool();
    result.inputMethodId = map.value(QStringLiteral("inputMethodId")).toString();
    result.inputMethodPackageName = map.value(QStringLiteral("inputMethodPackageName")).toString();
    result.fontPackageName = map.value(QStringLiteral("fontPackageName")).toString();

    LanguageFeatureState fontState = featureStateFromInt(map.value(QStringLiteral("fontState")).toInt());
    LanguageFeatureState inputMethodState = featureStateFromInt(map.value(QStringLiteral("inputMethodState")).toInt());

    if (type == LanguageFeatureType::Font) {
        fontState = state;
    } else {
        inputMethodState = state;
    }

    saveLanguageFeatureConfig(languageCode, result, fontState, inputMethodState);
}

// 删除语言特性保存配置
void Area::removeLanguageFeatureConfig(const QString &languageCode)
{
    QSettings settings;
    settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
    settings.remove(languageCode);
    settings.endGroup();
}

// 读取已保存的语言特性安装结果
InstallLanguageResult Area::savedLanguageFeatureResult(const QString &languageCode,
                                                       const QString &languageName) const
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(languageCode);
    InstallLanguageResult result = m_pendingInstallResults.value(languageCode);
    if (result.languageCode.isEmpty()) {
        QSettings settings;
        settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
        const QVariantMap map = settings.value(languageCode).toMap();
        settings.endGroup();

        if (!map.isEmpty()) {
            result.languageCode = languageCode;
            result.languageName = map.value(QStringLiteral("languageName"), languageName).toString();
            result.setCurrentLanguage = map.value(QStringLiteral("setCurrentLanguage")).toBool();
            result.installLanguageFont = map.value(QStringLiteral("installLanguageFont"),
                                                   option.checkLanguageFont).toBool();
            result.installInputMethod = map.value(QStringLiteral("installInputMethod"),
                                                  option.checkInputMethod).toBool();
            result.inputMethodId = map.value(QStringLiteral("inputMethodId"),
                                             option.inputMethodId).toString();
            result.inputMethodPackageName = map.value(QStringLiteral("inputMethodPackageName"),
                                                      option.inputMethodPackageName).toString();
            result.fontPackageName = map.value(QStringLiteral("fontPackageName"),
                                               option.fontPackageName).toString();
        }
    }

    if (result.languageCode.isEmpty()) {
        result.languageCode = languageCode;
        result.languageName = languageName;
        result.installLanguageFont = option.checkLanguageFont;
        result.installInputMethod = option.checkInputMethod;
        result.inputMethodId = option.inputMethodId;
        result.inputMethodPackageName = option.inputMethodPackageName;
        result.fontPackageName = option.fontPackageName;
    }

    result.inputMethodId = option.inputMethodId;
    result.inputMethodPackageName = option.inputMethodPackageName;
    result.fontPackageName = option.fontPackageName;

    return result;
}

// 查询语言字体特性状态
LanguageFeatureState Area::queryLanguageFontFeatureState(const InstallLanguageResult &result,
                                                         LanguageFeatureState fallbackState) const
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(result.languageCode);
    const QString packageName = effectivePackageName(result.fontPackageName, option.fontPackageName);
    if (packageName.isEmpty()) {
        return fallbackState;
    }
    return LanguageInstall::isPackageInstalled(packageName)
            ? LanguageFeatureState::Installed
            : LanguageFeatureState::NotInstalled;
}

// 查询输入法特性状态
LanguageFeatureState Area::queryInputMethodFeatureState(const InstallLanguageResult &result) const
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(result.languageCode);
    const QString packageName = effectivePackageName(result.inputMethodPackageName,
                                                     option.inputMethodPackageName);
    if (!packageName.isEmpty()) {
        return LanguageInstall::isPackageInstalled(packageName)
                ? LanguageFeatureState::Installed
                : LanguageFeatureState::NotInstalled;
    }

    QString inputMethodId = result.inputMethodId;
    if (inputMethodId.isEmpty()) {
        inputMethodId = option.inputMethodId;
    }
    if (!inputMethodId.isEmpty() && areaInterface) {
        QDBusReply<bool> added = areaInterface->call("isAddedInputMethod", inputMethodId);
        return added.isValid() && added.value()
                ? LanguageFeatureState::Installed
                : LanguageFeatureState::NotInstalled;
    }

    return LanguageFeatureState::NotInstalled;
}

// 判断语言是否缺少可安装特性包
bool Area::hasMissingLanguageFeaturePackage(const QString &languageCode) const
{
    if (languageCode.isEmpty()) {
        return false;
    }

    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(languageCode);
    const InstallLanguageResult result = savedLanguageFeatureResult(languageCode);

    if (option.showLanguageFont
            && result.installLanguageFont
            && isMissingPackage(effectivePackageName(result.fontPackageName, option.fontPackageName))) {
        return true;
    }

    if (option.showInputMethod
            && result.installInputMethod
            && isMissingPackage(effectivePackageName(result.inputMethodPackageName,
                                                    option.inputMethodPackageName))) {
        return true;
    }

    return false;
}

// 更新添加语言列表状态
void Area::updateAddLanguageRows()
{
    if (!areaWidget || !areaWidget->languageDialog() || !areaInterface) {
        return;
    }

    const QStringList languageList = areaInterface->property("languageList").toStringList();
    const QStringList showLanguageList = areaInterface->property("showLanguageList").toStringList();
    for (const QString &languageCode : languageList) {
        const bool isAdded = showLanguageList.contains(languageCode);
        const bool hasMissingPackage = isAdded && hasMissingLanguageFeaturePackage(languageCode);
        areaWidget->languageDialog()->setRowFlags(showLanguageName(languageCode),
                                                  isAdded && !hasMissingPackage);
    }
}

// 恢复语言展示项和特性状态
void Area::restoreLanguageFrame(const QString &languageCode, const QString &languageName, bool isCurrent)
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(languageCode);
    QSettings settings;
    settings.beginGroup(QString::fromLatin1(kLanguageFeaturesGroup));
    const QVariantMap map = settings.value(languageCode).toMap();
    settings.endGroup();

    InstallLanguageResult result = savedLanguageFeatureResult(languageCode, languageName);

    LanguageFeatureState fontState = normalizeRestoredFeatureState(
        featureStateFromInt(map.value(QStringLiteral("fontState"),
                                      static_cast<int>(LanguageFeatureState::NotInstalled)).toInt()));
    LanguageFeatureState inputMethodState = normalizeRestoredFeatureState(
        featureStateFromInt(map.value(QStringLiteral("inputMethodState"),
                                      static_cast<int>(LanguageFeatureState::NotInstalled)).toInt()));

    if (option.showLanguageFont && fontState != LanguageFeatureState::Installing) {
        fontState = queryLanguageFontFeatureState(result, fontState);
    }

    if (option.showInputMethod && inputMethodState != LanguageFeatureState::Installing) {
        inputMethodState = queryInputMethodFeatureState(result);
    }

    if (!map.isEmpty()) {
        saveLanguageFeatureConfig(languageCode, result, fontState, inputMethodState);
    }
    updatePendingLanguageRestartTip(languageCode, option, result, fontState, inputMethodState);

    m_pendingInstallResults.insert(languageCode, result);

    LanguageFrame *frame = areaWidget->addShowLanguageWithFeatures(languageCode,
                                                                 result.languageName,
                                                                 isCurrent);

    if (option.showLanguageFont) {
        frame->addLanguageFeature(
            LanguageFeatureType::Font,
            QCoreApplication::translate("InstallLanguageFeaturesDialog",
                                        "Corresponding language system font"),
            fontState);
    }

    if (option.showInputMethod) {
        QString inputMethodName = QCoreApplication::translate("InstallLanguageFeaturesDialog",
                                                              "Basic input method");
        if (!option.inputMethodName.isEmpty()) {
            inputMethodName = option.inputMethodName;
        }
        frame->addLanguageFeature(LanguageFeatureType::InputMethod,
                                  inputMethodName,
                                  inputMethodState);
    }
    frame->setExpanded(false);
}

void Area::initAddLanguage()
{
    if (!areaWidget->addbutton() || !areaWidget->languageDialog()) {
        return;
    }
    const QStringList languageList = areaInterface->property("languageList" ).toStringList();
    QStringList showLanguageList = areaInterface->property("showLanguageList" ).toStringList();
    for (QString s : languageList) {
        const bool isAdded = showLanguageList.contains(s);
        areaWidget->languageDialog()->addRow(showLanguageName(s),
                                             isAdded && !hasMissingLanguageFeaturePackage(s));
    }

    connect(areaWidget->addbutton(), &KPushButton::clicked, this, [=]() {
        updateAddLanguageRows();
        // showEvent 内会默认选中首个可选语言（同添加开机启动），确认键随选中启用
        areaWidget->languageDialog()->show();
        UkccCommon::buriedSettings(name(), "add system language", QString("clicked"));
    });
    connect(areaWidget->languageDialog(), &AddLanguageDialog::addShowLanguage, this, [=](QStringList selectedLanguageList) {
        for (QString selectedLanguage : selectedLanguageList) {
            for (int i = 0; i < languageList.size(); ++i) {
                if (selectedLanguage != showLanguageName(languageList.at(i))) {
                    continue;
                }
                QStringList showLanguageList = areaInterface->property("showLanguageList" ).toStringList();
                if (showLanguageList.contains(languageList.at(i))
                        && !hasMissingLanguageFeaturePackage(languageList.at(i))) {
                    break;
                }
                areaWidget->installLanguageFeaturesDialog()->setupLanguage(languageList.at(i), selectedLanguage);
                areaWidget->installLanguageFeaturesDialog()->show();
                break;
            }
        }
    });
    connect(areaWidget->installLanguageFeaturesDialog(), &InstallLanguageFeaturesDialog::installConfirmed,
            this, &Area::applyInstallLanguageResult);
    connect(areaWidget, &AreaUi::languageFeatureInstallRequested, this,
            &Area::onLanguageFeatureInstallRequested);
    connect(areaWidget->inputMethodDialog(), &AddInputMethodDialog::inputMethodAdded, this, [=](const QStringList methodList) {
        for (QString method : methodList) {
            qInfo() << "add input method:" << method;
            areaInterface->call("changeInputMethod", method, true);
        }
    });
}

// 展开当前语言的特性安装入口
void Area::showCurrentLanguageFeatures()
{
    if (mFirstLoad) {
        pluginUi();
    }
    if (!areaWidget || !areaInterface || !areaInterface->isValid()) {
        qWarning() << Q_FUNC_INFO << "Area UI is not ready";
        return;
    }

    const QString code = areaInterface->property("language").toString();
    if (code.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "language code is empty";
        return;
    }

    const QStringList languageList = areaInterface->property("languageList").toStringList();
    if (!languageList.contains(code)) {
        qWarning() << Q_FUNC_INFO << "unsupported language code:" << code;
        return;
    }

    const QString languageName = showLanguageName(code);
    QStringList showLanguageList = areaInterface->property("showLanguageList").toStringList();
    if (!showLanguageList.contains(code)) {
        showLanguageList.append(code);
        changeKey = QString("showLanguageList");
        ignoreNextDataChanged(changeKey);
        areaInterface->call("setShowLanguageList", showLanguageList);
    }

    if (areaWidget->languageDialog()) {
        areaWidget->languageDialog()->hide();
    }
    if (areaWidget->inputMethodDialog()) {
        areaWidget->inputMethodDialog()->hide();
    }
    if (areaWidget->installLanguageFeaturesDialog()) {
        areaWidget->installLanguageFeaturesDialog()->hide();
    }

    LanguageFrame *frame = areaWidget->languageFrame(code);
    if (!frame) {
        restoreLanguageFrame(code, languageName, code == areaInterface->property("language").toString());
        frame = areaWidget->languageFrame(code);
    } else if (!m_pendingInstallResults.contains(code)) {
        m_pendingInstallResults.insert(code, savedLanguageFeatureResult(code, languageName));
    }

    refreshLanguageCombo();
    areaWidget->collapseAllLanguageFrames();
    if (frame) {
        frame->setExpanded(true);
    }
    updateAddLanguageRows();
}

// 应用语言特性安装确认结果
void Area::applyInstallLanguageResult(const InstallLanguageResult &result)
{
    if (result.languageCode.isEmpty()) {
        return;
    }

    QStringList showLanguageList = areaInterface->property("showLanguageList").toStringList();
    const bool languageAlreadyShown = showLanguageList.contains(result.languageCode);
    if (!languageAlreadyShown) {
        showLanguageList.append(result.languageCode);
        areaWidget->languageDialog()->setRowFlags(result.languageName, true);
        changeKey = QString("showLanguageList");
        ignoreNextDataChanged(changeKey);
        areaInterface->call("setShowLanguageList", showLanguageList);
    }

    m_pendingInstallResults.insert(result.languageCode, result);

    const QString currentLanguage = areaInterface->property("language").toString();
    const bool isCurrent = result.setCurrentLanguage || result.languageCode == currentLanguage;
    if (result.setCurrentLanguage) {
        changeKey = QString("language");
        ignoreNextDataChanged(changeKey);
        areaInterface->call("setLanguage", result.languageCode);
    }

    LanguageFrame *frame = areaWidget->addShowLanguageWithFeatures(result.languageCode,
                                                                 result.languageName,
                                                                 isCurrent);
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(result.languageCode);

    LanguageFeatureItem *fontItem = nullptr;
    LanguageFeatureItem *inputMethodItem = nullptr;
    if (option.showLanguageFont) {
        fontItem = frame->addLanguageFeature(
            LanguageFeatureType::Font,
            QCoreApplication::translate("InstallLanguageFeaturesDialog",
                                        "Corresponding language system font"),
            result.installLanguageFont
                ? LanguageFeatureState::Installing
                : queryLanguageFontFeatureState(result, LanguageFeatureState::NotInstalled));
    }

    if (option.showInputMethod) {
        QString inputMethodName = QCoreApplication::translate("InstallLanguageFeaturesDialog",
                                                              "Basic input method");
        if (!option.inputMethodName.isEmpty()) {
            inputMethodName = option.inputMethodName;
        }
        inputMethodItem = frame->addLanguageFeature(
            LanguageFeatureType::InputMethod,
            inputMethodName,
            result.installInputMethod
                ? LanguageFeatureState::Installing
                : queryInputMethodFeatureState(result));
    }
    frame->setExpanded(true);

    refreshLanguageCombo(result.setCurrentLanguage ? result.languageCode : QString());
    if (result.setCurrentLanguage) {
        areaWidget->setCurrentLanguageCode(result.languageCode);
    }

    LanguageFeatureState initialFontState = LanguageFeatureState::NotInstalled;
    LanguageFeatureState initialInputMethodState = LanguageFeatureState::NotInstalled;
    if (result.installLanguageFont && option.showLanguageFont) {
        initialFontState = LanguageFeatureState::Installing;
    }
    if (result.installInputMethod && option.showInputMethod) {
        initialInputMethodState = LanguageFeatureState::Installing;
    }
    saveLanguageFeatureConfig(result.languageCode,
                              result,
                              initialFontState,
                              initialInputMethodState);
    if (result.setCurrentLanguage) {
        setLanguageRestartPromptPending(result.languageCode, true);
    }

    int restartPromptInstallCount = 0;
    if (result.setCurrentLanguage) {
        if (fontItem && result.installLanguageFont) {
            ++restartPromptInstallCount;
        }
        if (inputMethodItem && result.installInputMethod) {
            ++restartPromptInstallCount;
        }
        if (restartPromptInstallCount > 0) {
            m_pendingRestartPromptInstallCounts.insert(result.languageCode, restartPromptInstallCount);
        }
    }

    QList<QPair<LanguageFeatureType, LanguageFeatureItem *>> installFeatures;
    if (fontItem && result.installLanguageFont) {
        installFeatures.append(QPair<LanguageFeatureType, LanguageFeatureItem *>(
                                   LanguageFeatureType::Font, fontItem));
    }
    if (inputMethodItem && result.installInputMethod) {
        installFeatures.append(QPair<LanguageFeatureType, LanguageFeatureItem *>(
                                   LanguageFeatureType::InputMethod, inputMethodItem));
    }
    if (installFeatures.size() > 1 && m_languageInstall) {
        m_languageInstall->startInstallBatch(result.languageCode, installFeatures, result);
    } else {
        for (const QPair<LanguageFeatureType, LanguageFeatureItem *> &feature : installFeatures) {
            startLanguageFeatureInstall(result.languageCode, feature.first, feature.second, result);
        }
    }

    if (result.setCurrentLanguage && restartPromptInstallCount == 0) {
        promptLanguageRestartLater(result.languageCode);
    }
}

// 刷新当前语言下拉框
void Area::refreshLanguageCombo(const QString &currentLanguageOverride)
{
    const QStringList showLanguageList = areaInterface->property("showLanguageList").toStringList();
    const QStringList languageList = areaInterface->property("languageList").toStringList();
    const QString currentLanguage = currentLanguageOverride.isEmpty()
            ? areaInterface->property("language").toString()
            : currentLanguageOverride;
    QStringList comboCodes;
    QStringList comboNames;
    for (const QString &language : showLanguageList) {
        if (!languageList.contains(language)) {
            continue;
        }
        comboCodes.append(language);
        comboNames.append(showLanguageName(language));
    }
    areaWidget->updateCurrentLanguageCombo(comboCodes, comboNames, currentLanguage);
}

// 查询网络是否可用
bool Area::isNetworkAvailable() const
{
    return LanguageInstall::isNetworkAvailable();
}

// 安装语言字体特性
bool Area::installLanguageFontFeature(const InstallLanguageResult &result)
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(result.languageCode);
    if (option.fontName.isEmpty()) {
        return true;
    }
    QDBusInterface fontInterface("org.ukui.ukcc.session",
                                 "/Font",
                                 "org.ukui.ukcc.session.Font",
                                 QDBusConnection::sessionBus());
    if (!fontInterface.isValid()) {
        return false;
    }
    QDBusReply<void> reply = fontInterface.call("applyFont",
                                                option.fontName,
                                                option.fontPath,
                                                option.fontStyle);
    return reply.isValid();
}

// 安装输入法特性
bool Area::installInputMethodFeature(const InstallLanguageResult &result)
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(result.languageCode);
    const QString inputMethodId = result.inputMethodId.isEmpty()
            ? option.inputMethodId
            : result.inputMethodId;
    if (inputMethodId.isEmpty()) {
        return true;
    }
    QDBusReply<bool> added = areaInterface->call("isAddedInputMethod", inputMethodId);
    if (added.isValid() && added.value()) {
        return true;
    }
    areaInterface->call("changeInputMethod", inputMethodId, true);
    QDBusReply<bool> res = areaInterface->call("isAddedInputMethod", inputMethodId);
    return res.isValid() && res.value();
}

// 处理语言特性安装完成
void Area::onLanguageFeatureInstallFinished(const LanguageFeatureInstallTask &task,
                                            const PackageInstallResult &packageResult)
{
    LanguageFeatureInstallTask finishedTask = task;
    if (!m_pendingInstallResults.contains(task.languageCode)) {
        return;
    }
    if (!finishedTask.item && areaWidget) {
        if (LanguageFrame *frame = areaWidget->languageFrame(finishedTask.languageCode)) {
            finishedTask.item = frame->featureItem(finishedTask.type);
        }
    }

    bool success = packageResult.success;
    QString errorText = packageResult.errorText;
    if (success) {
        if (finishedTask.type == LanguageFeatureType::Font) {
            const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(finishedTask.result.languageCode);
            const QString packageName = effectivePackageName(finishedTask.result.fontPackageName,
                                                             option.fontPackageName);
            if (!packageName.isEmpty()) {
                installLanguageFontFeature(finishedTask.result);
                success = true;
            } else {
                success = installLanguageFontFeature(finishedTask.result);
            }
        } else {
            const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(finishedTask.result.languageCode);
            const QString packageName = effectivePackageName(finishedTask.result.inputMethodPackageName,
                                                             option.inputMethodPackageName);
            if (!packageName.isEmpty()) {
                installInputMethodFeature(finishedTask.result);
                success = true;
            } else {
                success = installInputMethodFeature(finishedTask.result);
            }
        }
        if (!success) {
            errorText = tr("Installation failed. Please retry later.");
        }
    }

    if (success) {
        if (finishedTask.item) {
            finishedTask.item->setState(LanguageFeatureState::Installed);
        }
        updateSavedFeatureState(finishedTask.languageCode,
                                finishedTask.type,
                                LanguageFeatureState::Installed);
    } else {
        if (finishedTask.item) {
            finishedTask.item->setState(LanguageFeatureState::Error, 0, errorText);
        }
        updateSavedFeatureState(finishedTask.languageCode,
                                finishedTask.type,
                                LanguageFeatureState::Error);
    }
    updateAddLanguageRows();
    finishLanguageRestartPromptInstall(finishedTask.languageCode);
}

// 卸载输入法特性
bool Area::uninstallInputMethodFeature(const QString &inputMethodId)
{
    if (inputMethodId.isEmpty()) {
        return true;
    }
    QDBusReply<bool> added = areaInterface->call("isAddedInputMethod", inputMethodId);
    if (!added.isValid() || !added.value()) {
        return true;
    }
    areaInterface->call("changeInputMethod", inputMethodId, false);
    QDBusReply<bool> res = areaInterface->call("isAddedInputMethod", inputMethodId);
    return res.isValid() && !res.value();
}

// 卸载语言字体特性
bool Area::uninstallLanguageFontFeature(const QString &languageCode)
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(languageCode);
    if (option.fontName.isEmpty() || option.fontPath.isEmpty()) {
        return true;
    }
    QDBusInterface fontInterface("org.ukui.ukcc.session",
                                 "/Font",
                                 "org.ukui.ukcc.session.Font",
                                 QDBusConnection::sessionBus());
    if (!fontInterface.isValid()) {
        return false;
    }
    const QStringList pathList = {option.fontPath};
    QDBusReply<bool> reply = fontInterface.call("uninstallFont", pathList, option.fontName);
    return reply.isValid() && reply.value();
}

// 卸载语言相关软件包
void Area::uninstallLanguagePackages(const QString &languageCode)
{
    if (m_languageInstall) {
        m_languageInstall->removeInstalls(languageCode);
    }

    LanguageFrame *frame = areaWidget->languageFrame(languageCode);
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(languageCode);
    const InstallLanguageResult pending = m_pendingInstallResults.value(languageCode);
    QStringList packageNamesToRemove;
    const QString fontPackageName = effectivePackageName(pending.fontPackageName,
                                                         option.fontPackageName);
    const QString inputMethodPackageName = effectivePackageName(pending.inputMethodPackageName,
                                                                option.inputMethodPackageName);

    const auto shouldUninstallFont = [&]() {
        if (frame) {
            LanguageFeatureItem *item = frame->featureItem(LanguageFeatureType::Font);
            return item && item->state() == LanguageFeatureState::Installed;
        }
        return option.showLanguageFont
                && !fontPackageName.isEmpty()
                && LanguageInstall::isPackageInstalled(fontPackageName);
    };
    const auto shouldUninstallInputMethod = [&]() {
        if (frame) {
            LanguageFeatureItem *item = frame->featureItem(LanguageFeatureType::InputMethod);
            return item && item->state() == LanguageFeatureState::Installed;
        }
        return option.showInputMethod
                && !inputMethodPackageName.isEmpty()
                && LanguageInstall::isPackageInstalled(inputMethodPackageName);
    };

    if (shouldUninstallFont()) {
        uninstallLanguageFontFeature(languageCode);
    }
    if (option.showLanguageFont
            && !fontPackageName.isEmpty()
            && LanguageInstall::isPackageInstalled(fontPackageName)) {
        appendPackageName(&packageNamesToRemove, fontPackageName);
    }

    const QString inputMethodId = pending.inputMethodId.isEmpty()
            ? option.inputMethodId
            : pending.inputMethodId;
    if (shouldUninstallInputMethod() && !inputMethodId.isEmpty()) {
        uninstallInputMethodFeature(inputMethodId);
    }
    if (option.showInputMethod
            && !inputMethodPackageName.isEmpty()
            && LanguageInstall::isPackageInstalled(inputMethodPackageName)) {
        appendPackageName(&packageNamesToRemove, inputMethodPackageName);
    }

    if (m_languageInstall) {
        m_languageInstall->removePackages(packageNamesToRemove);
    }

    m_pendingInstallResults.remove(languageCode);
    m_pendingRestartPromptInstallCounts.remove(languageCode);
}

// 确认并删除语言展示项
void Area::onLanguageDeleteRequested(const QString &languageCode, const QString &languageName)
{
    QMessageBox msg(areaWidget);
    msg.setWindowModality(Qt::ApplicationModal);
    msg.setIcon(QMessageBox::Warning);
    msg.setText(tr("Are you sure you want to remove %1?").arg(languageName));
    QPushButton *cancelBtn = msg.addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *removeBtn = msg.addButton(tr("Remove"), QMessageBox::AcceptRole);
    msg.setWindowTitle(tr("Are you sure you want to remove %1?").arg(languageName));
    cancelBtn->setAccessibleName(tr("Cancel"));
    removeBtn->setAccessibleName(tr("Remove"));
    Q_UNUSED(cancelBtn);
    msg.exec();
    if (msg.clickedButton() != removeBtn) {
        return;
    }

    uninstallLanguagePackages(languageCode);
    removeLanguageFeatureConfig(languageCode);
    areaWidget->removeLanguageFrame(languageCode);
    UkccCommon::buriedSettings(name(), QString("Delete"), QString("clicked"), languageCode);
}

// 启动单项语言特性安装
void Area::startLanguageFeatureInstall(const QString &languageCode,
                                       LanguageFeatureType type,
                                       LanguageFeatureItem *item,
                                       const InstallLanguageResult &result)
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(languageCode);
    const QString packageName = type == LanguageFeatureType::Font
            ? effectivePackageName(result.fontPackageName, option.fontPackageName)
            : effectivePackageName(result.inputMethodPackageName, option.inputMethodPackageName);
    if (item && isMissingPackage(packageName) && !isNetworkAvailable()) {
        item->setState(LanguageFeatureState::Error, 0, networkUnavailableText());
        updateSavedFeatureState(languageCode, type, LanguageFeatureState::Error);
        updateAddLanguageRows();
        finishLanguageRestartPromptInstall(languageCode);
        return;
    }

    if (m_languageInstall) {
        m_languageInstall->startInstall(languageCode, type, item, result);
    }
}

// 响应语言特性手动安装请求
void Area::onLanguageFeatureInstallRequested(const QString &languageCode, LanguageFeatureType type)
{
    if (!m_pendingInstallResults.contains(languageCode)) {
        return;
    }
    LanguageFrame *frame = areaWidget->languageFrame(languageCode);
    if (!frame) {
        return;
    }
    LanguageFeatureItem *item = frame->featureItem(type);
    if (!item) {
        return;
    }
    startLanguageFeatureInstall(languageCode, type, item, m_pendingInstallResults.value(languageCode));
}

void Area::initConnect()
{
    connect(areaWidget, &AreaUi::languageChanged, this, [=](QString languageCode) {
        changeKey = QString("language");
        areaInterface->call("setLanguage", languageCode);
        areaWidget->setCurrentLanguageCode(languageCode);
        UkccCommon::buriedSettings(name(), QString("currentLanguageCombox"), QString("select"), languageCode);
        promptLanguageRestartLater(languageCode);
    });
    connect(areaWidget, &AreaUi::languageRestartRequested, this, [=]() {
        system("ukui-session-tools --reboot");
    });
    connect(areaWidget, &AreaUi::languageDeleteRequested, this, &Area::onLanguageDeleteRequested);
    connect(areaWidget, &AreaUi::showLanguageListRemoved, this, [=](QString languageCode, QString languageName) {
        QStringList showLanguageList = areaInterface->property("showLanguageList" ).toStringList();
        showLanguageList.removeOne(languageCode);
        changeKey = QString("showLanguageList");
        areaInterface->call("setShowLanguageList", showLanguageList);
        areaWidget->languageDialog()->setRowFlags(languageName, false);
        const QString currentLanguage = areaInterface->property("language").toString();
        QStringList comboCodes;
        QStringList comboNames;
        const QStringList languageList = areaInterface->property("languageList").toStringList();
        for (const QString &language : showLanguageList) {
            if (!languageList.contains(language)) {
                continue;
            }
            comboCodes.append(language);
            comboNames.append(showLanguageName(language));
        }
        areaWidget->updateCurrentLanguageCombo(comboCodes, comboNames, currentLanguage);
    });

#ifdef Nile
    connect(areaWidget->shortDateComboBox(), static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=]{
        m_shortDateNeedChaned = false;
        kdk_system_set_short_dateformat(areaWidget->shortDateComboBox()->itemText(areaWidget->shortDateComboBox()->currentIndex()).toLatin1().data());
        UkccCommon::buriedSettings(name(), QString("Short Foramt Date"), QString("select"), areaWidget->shortDateComboBox()->currentData().toString());
    });

    connect(areaWidget->longDateComboBox(), static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=]{
        kdk_system_set_long_dateformat(areaWidget->longDateComboBox()->itemText(areaWidget->longDateComboBox()->currentIndex()).toLocal8Bit().data());
        UkccCommon::buriedSettings(name(), QString("Long Format Date"), QString("select"), areaWidget->longDateComboBox()->currentData().toString());
    });

    connect(areaWidget->timeComboBox(), static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            areaWidget,
            [=](int num) {
                Q_UNUSED(num);
                bool flag_24;
                if (0 == areaWidget->timeComboBox()->currentIndex()) {
                    flag_24 = false;
                    kdk_system_set_12_timeformat();
                } else {
                    flag_24 = true;
                    kdk_system_set_24_timeformat();
                }
                InteractiveInfo info = initInteractiveInfo(AreaUi::TIMEFORMAT);
                areaInterface->call(info.setkey.toUtf8().data(), areaWidget->timeComboBox()->currentData().toString());
                UkccCommon::buriedSettings(name(), info.setkey, QString("select"), areaWidget->timeComboBox()->currentData().toString());
    });

    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "TimeSignal",
                                          this,
                                          SLOT(TimeChanged(QString)));
    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "LongDateSignal",
                                          this,
                                          SLOT(refreshLongDate(QString)));
    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "ShortDateSignal",
                                          this,
                                          SLOT(refreshShortDate(QString)));
#endif
    setVisibleBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setVisibleBySecurity()));
    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void Area::setVisibleBySecurity()
{
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value(name().toLower() + "Settings").toString();
    QStringList setItems = moduleSettings.split(",");
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0) << item.at(1);
        bool visible = (item.at(1) == "true");
        if (item.at(0) == "addLanguageBtn") {
            areaWidget->addbutton()->setVisible(visible);
        }
        if (item.at(0) == "languageGroup") {
            if (areaWidget->addbutton()->isVisible()) {
                areaWidget->addbutton()->setVisible(visible);
            }
            areaWidget->getlanguageGroup()->setVisible(visible);
            areaWidget->getLanguageTitleLable()->setVisible(visible);
        }
    }
}

void Area::setEnableBySecurity() {
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("areaEnable").toString();
    QStringList setItems = moduleSettings.split(",");
    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0) << item.at(1);
        bool enable = (item.at(1) == "true");
        if (item.at(0) == "addLanguageBtn") {
            areaWidget->addbutton()->setEnabled(enable);
        }
        if (item.at(0) == "languageGroup") {
            if (areaWidget->addbutton()->isEnabled()) {
                areaWidget->addbutton()->setEnabled(enable);
            }
            areaWidget->getlanguageGroup()->setEnabled(enable);
            areaWidget->getLanguageTitleLable()->setEnabled(enable);
        }
    }
}

void Area::initDateComboBox() {
#ifdef Nile
        refreshShortDate(kdk_system_get_shortformat());
        refreshLongDate(kdk_system_get_longformat());
#endif
}

void Area::TimeChanged(QString time) {
    Q_UNUSED(time);
    initTimeFormat();
    InteractiveInfo info = initInteractiveInfo(AreaUi::TIMEFORMAT);
    areaInterface->call(info.setkey.toUtf8().data(), areaWidget->timeComboBox()->currentData().toString());
}

void Area::refreshLongDate(QString date) {
    QString mLongFormatDate = date;
    int longindex = 0;

    if ((mLongFormatDate.at(mLongFormatDate.length() - 4) == ',')
            || ((mLongFormatDate.at(2) < '0' || mLongFormatDate.at(2) > '9')
                && (mLongFormatDate.at(2) < 'a' || mLongFormatDate.at(2) > 'z')))
    {
        longindex = 1;
     } else {
        longindex = 0;
    }

    if (longindex == areaWidget->longDateComboBox()->currentIndex())
        return;

    areaWidget->longDateComboBox()->setCurrentIndex(longindex);
    if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
        initLanguageFormat();
    }
}

void Area::refreshShortDate(QString date) {
    if (!m_shortDateNeedChaned) {
        qDebug() << Q_FUNC_INFO << "refreshShortDate not need change date" << date;
        m_shortDateNeedChaned = true;
        return;
    }
    QString mShortFormatDate = date;
    int shortindex = 0;
    int ansindex = 0;

    if (areaWidget->countryComboBox()->currentIndex() == 0) {
        ansindex = mShortFormatDate.length() - 3;
    } else {
        ansindex = 2;
    }

    if (mShortFormatDate.at(ansindex) == 'M' ||
            mShortFormatDate.at(ansindex) == 'y') {
        shortindex = 3;
        switch (ansindex) {
        case 2:
            ansindex += 2;
            break;
        default:
            ansindex -= 2;
        }
    }

    if (mShortFormatDate.at(ansindex) == '/')
        shortindex += 1;
    if (mShortFormatDate.at(ansindex) == '-')
        shortindex += 2;
    if (shortindex == areaWidget->shortDateComboBox()->currentIndex())
        return;

    areaWidget->shortDateComboBox()->setCurrentIndex(shortindex);
    if (UkccCommon::isOpenkylinNile() || UkccCommon::isV11()) {
        initLanguageFormat();
    }
}

QString Area::showLanguageName(QString languageCode)
{
    QString languageName = "";
    if (languageCode == "zh_CN") {
        languageName = QString("简体中文 ") + QString("(%1)").arg(tr("Simplified Chinese"));
    } else if (languageCode == "en") {
        languageName = QString("English ") + QString("(%1)").arg(tr("English"));
    } else if (languageCode == "bo_CN") {
        languageName = QString("བོད་ཡིག ") + QString("(%1)").arg(tr("Tibetan"));
    } else if (languageCode == "kk_KZ") {
        languageName = QString("قازاق تىلى") + QString("(%1)").arg(tr("Kazaqa"));
    } else if (languageCode == "ug_CN") {
        languageName = QString("ئۇيغۇر") + QString("(%1)").arg(tr("Uygur"));
    } else if (languageCode == "ky_KG") {
        languageName = QString("قىرعىز تىلى") + QString("(%1)").arg(tr("Kirghiz"));
    } else if (languageCode == "zh_HK") {
        languageName = QString("繁體") + QString("(%1)").arg(tr("Traditional Chinese"));
    } else if (languageCode == "mn_MN") {
        languageName = QString("ᠮᠣᠩᠭᠣᠯ ᠬᠡᠯᠡ") + QString("(%1)").arg(tr("Mongolian"));
    } else if (languageCode == "de_DE") {
        languageName = QString("Deutsch") + QString("(%1)").arg(tr("German"));
    } else if (languageCode == "es_ES") {
        languageName = QString("Español") + QString("(%1)").arg(tr("Spanish"));
    } else if (languageCode == "fr_FR") {
        languageName = QString("Français") + QString("(%1)").arg(tr("French"));
    } else if (languageCode == "ab_GE") {
        languageName = QString("бызшәа") + QString("(%1)").arg(tr("Abkhazian"));
    } else if (languageCode == "ae") {
        languageName = QString("𐬀𐬌𐬙𐬌𐬚𐬀") + QString("(%1)").arg(tr("Avestan"));
    } else if (languageCode == "af_ZA") {
        languageName = QString("Afrikaans") + QString("(%1)").arg(tr("Afrikaans"));
    } else if (languageCode == "ar_AE") {
        languageName = QString("العربية") + QString("(%1)").arg(tr("Arabic"));
    } else if (languageCode == "ak_GH") {
        languageName = QString("Akan") + QString("(%1)").arg(tr("Akan"));
    } else if (languageCode == "az_AZ") {
        languageName = QString("dili") + QString("(%1)").arg(tr("Azerbaijani"));
    } else if (languageCode == "be_BY") {
        languageName = QString("мова") + QString("(%1)").arg(tr("Belarusian"));
    } else if (languageCode == "bg_BG") {
        languageName = QString("Български") + QString("(%1)").arg(tr("Bulgarian"));
    } else if (languageCode == "bn_BD") {
        languageName = QString("বাংলা") + QString("(%1)").arg(tr("Bengali"));
    } else if (languageCode == "bs_BA") {
        languageName = QString("Bosanski") + QString("(%1)").arg(tr("Bosnian"));
    } else if (languageCode == "ca_AD") {
        languageName = QString("Català") + QString("(%1)").arg(tr("Catalan"));
    } else if (languageCode == "ce_RU") {
        languageName = QString("Нохчийн") + QString("(%1)").arg(tr("Chechen"));
    } else if (languageCode == "cs_CZ") {
        languageName = QString("Čeština") + QString("(%1)").arg(tr("Czech"));
    } else if (languageCode == "cy_GB") {
        languageName = QString("Cymraeg") + QString("(%1)").arg(tr("Welsh"));
    } else if (languageCode == "da_DK") {
        languageName = QString("Dansk") + QString("(%1)").arg(tr("Danish"));
    } else if (languageCode == "el_CY") {
        languageName = QString("Ελληνικά") + QString("(%1)").arg(tr("Greek"));
    } else if (languageCode == "et_EE") {
        languageName = QString("keel") + QString("(%1)").arg(tr("Estonian"));
    } else if (languageCode == "eu_ES") {
        languageName = QString("Euskara") + QString("(%1)").arg(tr("Basque"));
    } else if (languageCode == "fa_IR") {
        languageName = QString("فارسی") + QString("(%1)").arg(tr("Persian"));
    } else if (languageCode == "fi_FI") {
        languageName = QString("Suomi") + QString("(%1)").arg(tr("Finnish"));
    } else if (languageCode == "fj") {
        languageName = QString("Vakaviti") + QString("(%1)").arg(tr("Fijian"));
    } else if (languageCode == "fo_FO") {
        languageName = QString("Føroyskt") + QString("(%1)").arg(tr("Faroese"));
    } else if (languageCode == "ga_IE") {
        languageName = QString("Gaeilge") + QString("(%1)").arg(tr("Irish"));
    } else if (languageCode == "gl_ES") {
        languageName = QString("Galego") + QString("(%1)").arg(tr("Galician"));
    } else if (languageCode == "gu_IN") {
        languageName = QString("ગુજરાતી") + QString("(%1)").arg(tr("Gujarati"));
    } else if (languageCode == "he_IL") {
        languageName = QString("עברית") + QString("(%1)").arg(tr("Hebrew"));
    } else if (languageCode == "hmn") {
        languageName = QString("Hmoob") + QString("(%1)").arg(tr("Hmong"));
    } else if (languageCode == "hi_IN") {
        languageName = QString("हिंदी") + QString("(%1)").arg(tr("Hindi"));
    } else if (languageCode == "hr_HR") {
        languageName = QString("Hrvatski") + QString("(%1)").arg(tr("Croatian"));
    } else if (languageCode == "hu_HU") {
        languageName = QString("Magyar") + QString("(%1)").arg(tr("Hungarian"));
    } else if (languageCode == "hy_AM") {
        languageName = QString("Հայերեն") + QString("(%1)").arg(tr("Armenian"));
    } else if (languageCode == "id_ID") {
        languageName = QString("Indonesia") + QString("(%1)").arg(tr("Indonesian"));
    } else if (languageCode == "is_IS") {
        languageName = QString("Íslenska") + QString("(%1)").arg(tr("Icelandic"));
    } else if (languageCode == "it_IT") {
        languageName = QString("Italiano") + QString("(%1)").arg(tr("Italian"));
    } else if (languageCode == "iu_CA") {
        languageName = QString("ᐃᓄᒃᑎᑐᑦ") + QString("(%1)").arg(tr("Inuktitut"));
    } else if (languageCode == "jv") {
        languageName = QString("ꦧꦱꦗꦮ") + QString("(%1)").arg(tr("Javanese"));
    } else if (languageCode == "ja_JP") {
        languageName = QString("日本語") + QString("(%1)").arg(tr("Japanese"));
    } else if (languageCode == "ka_GE") {
        languageName = QString("ქართული") + QString("(%1)").arg(tr("Georgian"));
    } else if (languageCode == "kg") {
        languageName = QString("Kongo") + QString("(%1)").arg(tr("Kongo"));
    } else if (languageCode == "kl_GL") {
        languageName = QString("Kalaallisut") + QString("(%1)").arg(tr("Greenlandic"));
    } else if (languageCode == "kn_IN") {
        languageName = QString("ಕನ್ನಡ") + QString("(%1)").arg(tr("Kannada"));
    } else if (languageCode == "ko_KR") {
        languageName = QString("한국어") + QString("(%1)").arg(tr("Korean"));
    } else if (languageCode == "kok_IN") {
        languageName = QString("कोंकणी") + QString("(%1)").arg(tr("Konkani"));
    } else if (languageCode == "la") {
        languageName = QString("Latina") + QString("(%1)").arg(tr("Latin"));
    } else if (languageCode == "lo_LA") {
        languageName = QString("ລາວ") + QString("(%1)").arg(tr("Lao"));
    } else if (languageCode == "lt_LT") {
        languageName = QString("Lietuvių") + QString("(%1)").arg(tr("Lithuanian"));
    } else if (languageCode == "lv_LV") {
        languageName = QString("Latviešu") + QString("(%1)").arg(tr("Latvian"));
    } else if (languageCode == "mg_MG") {
        languageName = QString("Malagasy") + QString("(%1)").arg(tr("Malagasy"));
    } else if (languageCode == "mi_NZ") {
        languageName = QString("Māori") + QString("(%1)").arg(tr("Māori"));
    } else if (languageCode == "mk_MK") {
        languageName = QString("Македонски") + QString("(%1)").arg(tr("Macedonian"));
    } else if (languageCode == "mr_IN") {
        languageName = QString("मराठी") + QString("(%1)").arg(tr("Marathi"));
    } else if (languageCode == "ms_MY") {
        languageName = QString("Melayu") + QString("(%1)").arg(tr("Malay"));
    } else if (languageCode == "mt_MT") {
        languageName = QString("Malti") + QString("(%1)").arg(tr("Maltese"));
    } else if (languageCode == "my_MM") {
        languageName = QString("မြန်မာဘာသာ") + QString("(%1)").arg(tr("Burmese"));
    } else if (languageCode == "na") {
        languageName = QString("Naoero") + QString("(%1)").arg(tr("Nauruan"));
    } else if (languageCode == "ne_NP") {
        languageName = QString("नेपाली") + QString("(%1)").arg(tr("Nepali"));
    } else if (languageCode == "nl_NL") {
        languageName = QString("Nederlands") + QString("(%1)").arg(tr("Dutch"));
    } else if (languageCode == "no") {
        languageName = QString("Norsk") + QString("(%1)").arg(tr("Norwegian"));
    } else if (languageCode == "ns") {
        languageName = QString("Sesotho") + QString("(%1)").arg(tr("Northern Sotho"));
    } else if (languageCode == "pa_IN") {
        languageName = QString("ਪੰਜਾਬੀ") + QString("(%1)").arg(tr("Punjabi"));
    } else if (languageCode == "pl_PL") {
        languageName = QString("Polski") + QString("(%1)").arg(tr("Polish"));
    } else if (languageCode == "pt_PT") {
        languageName = QString("Português") + QString("(%1)").arg(tr("Portuguese"));
    } else if (languageCode == "qu") {
        languageName = QString("Simi") + QString("(%1)").arg(tr("Quechua"));
    } else if (languageCode == "ro_RO") {
        languageName = QString("Română") + QString("(%1)").arg(tr("Romanian"));
    } else if (languageCode == "ru_RU") {
        languageName = QString("Русский") + QString("(%1)").arg(tr("Russian"));
    } else if (languageCode == "sa_IN") {
        languageName = QString("संस्कृत") + QString("(%1)").arg(tr("Sanskrit"));
    } else if (languageCode == "se_NO") {
        languageName = QString("Sámegiella") + QString("(%1)").arg(tr("Northern Sami"));
    } else if (languageCode == "sk_SK") {
        languageName = QString("Slovenčina") + QString("(%1)").arg(tr("Slovak"));
    } else if (languageCode == "sl_SI") {
        languageName = QString("Slovenščina") + QString("(%1)").arg(tr("Slovenian"));
    } else if (languageCode == "sq_AL") {
        languageName = QString("Shqip") + QString("(%1)").arg(tr("Albanian"));
    } else if (languageCode == "sv_FI") {
        languageName = QString("Svenska") + QString("(%1)").arg(tr("Swedish"));
    } else if (languageCode == "sw_KE") {
        languageName = QString("Kiswahili") + QString("(%1)").arg(tr("Swahili"));
    } else if (languageCode == "syr") {
        languageName = QString("ܣܘܪܝܝܐ") + QString("(%1)").arg(tr("Syriac"));
    } else if (languageCode == "ta_IN") {
        languageName = QString("தமிழ்") + QString("(%1)").arg(tr("Tamil"));
    } else if (languageCode == "te_IN") {
        languageName = QString("తెలుగు") + QString("(%1)").arg(tr("Telugu"));
    } else if (languageCode == "th_TH") {
        languageName = QString("ไทย") + QString("(%1)").arg(tr("Thai"));
    } else if (languageCode == "tk_TM") {
        languageName = QString("Türkmençe") + QString("(%1)").arg(tr("Turkmen"));
    } else if (languageCode == "tl_PH") {
        languageName = QString("Tagalog") + QString("(%1)").arg(tr("Tagalog"));
    } else if (languageCode == "tn_ZA") {
        languageName = QString("Setswana") + QString("(%1)").arg(tr("Tswana"));
    } else if (languageCode == "tr_TR") {
        languageName = QString("Türkçe") + QString("(%1)").arg(tr("Turkish"));
    } else if (languageCode == "tt_RU") {
        languageName = QString("теле") + QString("(%1)").arg(tr("Tatar"));
    } else if (languageCode == "uk_UA") {
        languageName = QString("Українська") + QString("(%1)").arg(tr("Ukrainian"));
    } else if (languageCode == "ur_IN") {
        languageName = QString("اردو") + QString("(%1)").arg(tr("Urdu"));
    } else if (languageCode == "uz_UZ") {
        languageName = QString("tili") + QString("(%1)").arg(tr("Uzbek"));
    } else if (languageCode == "vi_VN") {
        languageName = QString("Việt") + QString("(%1)").arg(tr("Vietnamese"));
    } else if (languageCode == "xh_ZA") {
        languageName = QString("isiXhosa") + QString("(%1)").arg(tr("Xhosa"));
    } else if (languageCode == "yue_HK") {
        languageName = QString("廣東話") + QString("(%1)").arg(tr("Cantonese"));
    } else if (languageCode == "za") {
        languageName = QString("壮语") + QString("(%1)").arg(tr("Zhuang"));
    } else if (languageCode == "zu_ZA") {
        languageName = QString("isiZulu") + QString("(%1)").arg(tr("Zulu"));
    } else if (languageCode == "ko_KP") {
        languageName = QString("조선어") + QString("(%1)").arg(tr("North Korea"));
    } else if (languageCode == "khb_CN") {
        languageName = QString("ᦅᧄᦺᦑᦟᦹᧉ") + QString("(%1)").arg(tr("Tainlwe"));
    } else if (languageCode == "ru_RU") {
        languageName = QString(" Русский") + QString("(%1)").arg(tr("Russian"));
    } else if (languageCode == "ur_PK") {
        languageName = QString(" اردو") + QString("(%1)").arg(tr("Urdu"));
    } else {
        languageName = QString("languageCode ") + QString("(%1)").arg("languageCode");
    }
    return languageName;
}

void Area::updateShortDate() {
    QString shortFormat = kdk_system_get_shortformat();
    if (shortFormat.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "kdk_system_get_shortformat return empty, skip";
        return;
    }
    QStringList enShortDateList = {"M.d.yy", "M/d/yy", "M-d-yy", "MM.dd.yyyy", "MM/dd/yyyy", "MM-dd-yyyy"};
    QStringList zhShortDateList = {"yy.M.d", "yy/M/d", "yy-M-d", "yyyy.MM.dd", "yyyy/MM/dd", "yyyy-MM-dd"};
    QString realShortFormt = shortFormat;
    if (areaWidget->countryComboBox()->currentIndex() == 0) {
        if (shortFormat.at(0) != 'M') {
            int index = zhShortDateList.indexOf(shortFormat);
            realShortFormt = enShortDateList.at(index);
            qDebug() << Q_FUNC_INFO << "updateShortDate en realShortFormt" << realShortFormt << "index" << index << "shortFormat" << shortFormat;
        }
    } else {
        if (shortFormat.at(0) != 'y') {
            int index = enShortDateList.indexOf(shortFormat);
            realShortFormt = zhShortDateList.at(index);
            qDebug() << Q_FUNC_INFO << "updateShortDate zh realShortFormt" << realShortFormt << "index" << index << "shortFormat" << shortFormat;
        }
    }
    qDebug() << Q_FUNC_INFO << "updateShortDate realShortFormt" << realShortFormt << "shortFormat" << shortFormat;
    m_shortDateNeedChaned = false;
    kdk_system_set_short_dateformat(realShortFormt.toLatin1().data());
}

// 延后提示语言重启
void Area::promptLanguageRestartLater(const QString &languageCode)
{
    const QString restartLanguageCode = languageCode.isEmpty() && areaInterface
            ? areaInterface->property("language").toString()
            : languageCode;
    setLanguageRestartPromptPending(restartLanguageCode, true);
    if (showLanguageRestartMessageBox()) {
        areaWidget->showLanguageRestartTip();
    }
}

// 结束语言特性安装后的重启提示等待
void Area::finishLanguageRestartPromptInstall(const QString &languageCode)
{
    if (!m_pendingRestartPromptInstallCounts.contains(languageCode)) {
        return;
    }

    const int remainingCount = m_pendingRestartPromptInstallCounts.value(languageCode) - 1;
    if (remainingCount > 0) {
        m_pendingRestartPromptInstallCounts.insert(languageCode, remainingCount);
        return;
    }

    m_pendingRestartPromptInstallCounts.remove(languageCode);
    promptLanguageRestartLater(languageCode);
}

// 显示语言重启确认弹窗
bool Area::showLanguageRestartMessageBox()
{
    QMessageBox msg(qApp->activeWindow());
    msg.setIcon(QMessageBox::Warning);
    msg.setParent(areaWidget);
    msg.setText(tr("Modify the first language need to reboot to take effect, whether to reboot?"));
    QPushButton *laterButton = msg.addButton(tr("Reboot later"), QMessageBox::NoRole);
    QPushButton *rebootButton = msg.addButton(tr("Reboot now"), QMessageBox::ApplyRole);
    msg.exec();

    if (msg.clickedButton() == rebootButton) {
        system("ukui-session-tools --reboot");
        return false;
    }

    return msg.clickedButton() == laterButton;
}

void Area::showMessageBox(int flag)
{
    if (flag == 2) {
        promptLanguageRestartLater();
        return;
    }

    QMessageBox msg(qApp->activeWindow());
    msg.setIcon(QMessageBox::Warning);
    msg.setParent(areaWidget);

    if (flag == 1) {
        msg.setText(tr("Modify the current region need to logout to take effect, whether to logout?"));
        msg.addButton(tr("Logout later"), QMessageBox::NoRole);
        msg.addButton(tr("Logout now"), QMessageBox::ApplyRole);
    }
    int ret = msg.exec();

    if (ret == 1) {
        if (flag == 1) {
            updateShortDate();
            system("ukui-session-tools --logout");
        }
    }
    return;
}
