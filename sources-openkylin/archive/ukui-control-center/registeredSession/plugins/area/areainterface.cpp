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
#include "areainterface.h"
#include <unistd.h>
#include <QDBusConnection>
#include <QProcess>

AreaInterface::AreaInterface():UKUI::ServiceObject(nullptr)
{
    ukccPanelGsettings = UniversalInterface::self()->ukccPanelGsettings();
    accountInterface   = UniversalInterface::self()->accountUserDbus();
    fcitxInterface     = UniversalInterface::self()->fcitxInputMethodDbus();
    styleGsettings     = UniversalInterface::self()->ukuiStyleGsettings();
    m_calendarGsettings  = UniversalInterface::self()->calendarGsettings();

    initConnect();
}

AreaInterface::~AreaInterface()
{

}

void AreaInterface::initConnect()
{
    connect(ukccPanelGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == CALENDAR_KEY) {
            Q_EMIT changed("calendar");
        } else if (key == FIRST_DAY_KEY) {
            Q_EMIT changed("firstDay");
        } else if (key == DATE_FORMATE_KEY) {
            Q_EMIT changed("dateFormat");
        } else if (key == TIME_FORMAT_KEY) {
            Q_EMIT changed("timeFormat");
        } else if (key == SHOW_LANGUAGE_KEY) {
            Q_EMIT changed("showLanguageList");
        }
    });

    connect(styleGsettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName") {
            Q_EMIT changed("iconThemeName");
        }
    });
    if (m_calendarGsettings) {
        connect(m_calendarGsettings, &QGSettings::changed, this, [=](QString key) {
            if (key == LUNAR_DATE_KEY) {
                Q_EMIT changed("lunarDate");
            }
        });
    }
}

QString AreaInterface::gsettingCalendarGet(const QString &key) const
{
    if (!m_calendarGsettings) {
        return "";
    }
    if (m_calendarGsettings->keys().contains(key)) {
        return m_calendarGsettings->get(key).toString();
    } else {
        qCritical() << "calendarGsettings not contains the key: " << key;
        return "";
    }
}

QString AreaInterface::gsettingGet(const QString &key) const
{
    if (ukccPanelGsettings->keys().contains(key)) {
        return ukccPanelGsettings->get(key).toString();
    } else {
        qCritical() << "ukccPanelGsettings not contains the key: " << key;
        return "";
    }
}

void AreaInterface::gsettingSet(const QString &key, const QString &value)
{
    if (ukccPanelGsettings->keys().contains(key)) {
        ukccPanelGsettings->set(key, value);
    } else {
        qCritical() << "ukccPanelGsettings not contains the key: " << key;
    }
}

QStringList AreaInterface::getFormatCountryList()
{
    QStringList formatList;
    formatList << QString("en_US.UTF-8") << QString("zh_CN.UTF-8") << QString("bo_CN.UTF-8");
    if (isOpenkylin()) {
        formatList.removeLast();
#ifdef Nile
    formatList << QString("bo_CN.UTF-8") << QString("ug_CN.UTF-8") << QString("ky_KG.UTF-8")
               << QString("zh_HK.UTF-8") << QString("kk_KZ.UTF-8") << QString("mn_MN.UTF-8")
               << QString("de_DE.UTF-8") << QString("es_ES.UTF-8") << QString("fr_FR.UTF-8");
#endif
    }

    if (isV11()) {
        formatList << QString("mn_MN.UTF-8") << QString("zh_HK.UTF-8");
        qDebug() << Q_FUNC_INFO << "getFormatCountryList" << formatList;
    }

    return formatList;
}

QStringList AreaInterface::getCalendarList()
{
    QStringList list = {};
    list.append(QString("solarlunar"));
    QString locale = QLocale::system().name();
    if ("zh_CN" == locale) {
        list.append(QString("lunar"));
    }
    return list;
}

QStringList AreaInterface::getFirstDayList()
{
    return QStringList() << QString("monday") << QString("sunday");
}

QStringList AreaInterface::getDateFormatList()
{
    return QStringList() << QString("cn") << QString("en");
}

QStringList AreaInterface::getTimeFormatList()
{
    return QStringList() << QString("12") << QString("24");
}

QStringList AreaInterface::getShowLanguageList()
{
    if (ukccPanelGsettings->keys().contains(SHOW_LANGUAGE_KEY)) {
        return ukccPanelGsettings->get(SHOW_LANGUAGE_KEY).toStringList();;
    } else {
        qCritical() << "ukccPanelGsettings not contains the key: " << SHOW_LANGUAGE_KEY;
    }
    return QStringList();
}

QStringList AreaInterface::getLanguageList()
{
    QStringList formatList;
    formatList << QString("zh_CN") << QString("en") << QString("bo_CN");

    if (isOpenkylin()) {
        formatList.removeLast();
#ifdef Nile
    formatList << QString("bo_CN") << QString("ug_CN") << QString("ky_KG")
               << QString("zh_HK") << QString("kk_KZ") << QString("mn_MN")
               << QString("de_DE") << QString("es_ES") << QString("fr_FR");
#endif
        formatList << QString("ar_AE") << QString("ab_GE") << QString("ae") << QString("af_ZA") << QString("ak_GH") << QString("az_AZ") << QString("be_BY") << QString("bg_BG") << QString("bn_BD") 
                   << QString("bs_BA") << QString("ca_AD") << QString("ce_RU") << QString("cs_CZ") << QString("cy_GB") << QString("da_DK") << QString("de_DE") << QString("el_CY") << QString("en_US") 
                   << QString("es_ES") << QString("et_EE") << QString("eu_ES") << QString("fa_IR") << QString("fi_FI") << QString("fj") << QString("fo_FO") << QString("fr_FR") << QString("ga_IE") 
                   << QString("gl_ES") << QString("gu_IN") << QString("he_IL") << QString("hmn") << QString("hi_IN") << QString("hr_HR") << QString("hu_HU") << QString("hy_AM") << QString("id_ID") 
                   << QString("is_IS") << QString("it_IT") << QString("iu_CA") << QString("jv") << QString("ja_JP") << QString("ka_GE") << QString("kg") << QString("kl_GL") << QString("kn_IN") 
                   << QString("ko_KR") << QString("kok_IN") << QString("la") << QString("lo_LA") << QString("lt_LT") << QString("lv_LV") << QString("mg_MG") << QString("mi_NZ") << QString("mk_MK") 
                   << QString("mr_IN") << QString("ms_MY") << QString("mt_MT") << QString("my_MM") << QString("na") << QString("ne_NP") << QString("nl_NL") << QString("no") << QString("ns") 
                   << QString("pa_IN") << QString("pl_PL") << QString("pt_PT") << QString("qu") << QString("ro_RO") << QString("ru_RU") << QString("sa_IN") << QString("se_No") << QString("sk_SK") 
                   << QString("sl_SI") << QString("sq_AL") << QString("sv_FI") << QString("sw_KE") << QString("syr") << QString("ta_IN") << QString("te_IN") << QString("th_TH") << QString("tk_TM") 
                   << QString("tl_PH") << QString("tn_ZA") << QString("tr_TR") << QString("tt_RU") << QString("uk_UA") << QString("ur_IN") << QString("uz_UZ") << QString("vi_VN") << QString("xh_ZA") 
                   << QString("yue_HK") << QString("za") << QString("zu_ZA")   << QString("ko_KP") << QString("khb_CN") << QString("ru_RU") << QString("ur_PK")  ;
        qDebug() << Q_FUNC_INFO << "getLanguageList" << formatList;
        return formatList;
    }

    if (isV11()) {
        formatList << QString("mn_MN") << QString("zh_HK") << QString("ug_CN");
    }

#ifdef CZDZDYY
    formatList << QString("kk_KZ") << QString("ug_CN") << QString("ky_KG");
#endif

#ifdef CXDZDYY
    formatList << QString("vi_VN") << QString("ms_MY") << QString("th_TH");
#endif

    qDebug() << Q_FUNC_INFO << "getLanguageList" << formatList;

    return formatList;
}

bool AreaInterface::isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool AreaInterface::isV11()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("VERSION_ID").toString();

    if (versionID.compare("v11", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

QString AreaInterface::getFormatCountry()
{
    return accountInterface->property("FormatsLocale").toString();;
}

QString AreaInterface::getCalendar()
{
    return gsettingGet(CALENDAR_KEY);;
}

QString AreaInterface::getFirstDay()
{
    return gsettingGet(FIRST_DAY_KEY);;
}

QString AreaInterface::getDateFormat()
{
    return gsettingGet(DATE_FORMATE_KEY);;
}

QString AreaInterface::getTimeFormat()
{
    return gsettingGet(TIME_FORMAT_KEY);;
}

QString AreaInterface::getLunarDate()
{
    return gsettingCalendarGet(LUNAR_DATE_KEY);;
}

QString AreaInterface::getLanguage()
{
    return accountInterface->property("Language").toString();;
}

void AreaInterface::setFormatCountry(const QString &formatCountry)
{
    if (getFormatCountryList().contains(formatCountry)) {
        accountInterface->call("SetFormatsLocale", formatCountry);
        Q_EMIT changed("formatCountry");
    }
}

// Gsetting设置会触发changed(key)信号，因此无需Q_EMIT changed，避免重复触发信号。
void AreaInterface::setCalendar(const QString &calendar)
{
    if (getCalendarList().contains(calendar)) {
        gsettingSet(CALENDAR_KEY, calendar);
    }
}

void AreaInterface::setFirstDay(const QString &firstDay)
{
    if (getFirstDayList().contains(firstDay)) {
        gsettingSet(FIRST_DAY_KEY, firstDay);
    };
}

void AreaInterface::setDateFormat(const QString &dateFormat)
{
    if (getDateFormatList().contains(dateFormat)) {
        gsettingSet(DATE_FORMATE_KEY, dateFormat);
    }
}

void AreaInterface::setTimeFormat(const QString &timeFormat)
{
    if (getTimeFormatList().contains(timeFormat)) {
#ifdef Nile
        // 与区域语言页 UI 一致：面板/时钟读的是 kysdk 时间格式，不能只写 gsettings
        if (timeFormat == QStringLiteral("24")) {
            kdk_system_set_24_timeformat();
        } else {
            kdk_system_set_12_timeformat();
        }
#endif
        gsettingSet(TIME_FORMAT_KEY, timeFormat);
    }
}

void AreaInterface::setShowLanguageList(const QStringList &languageList)
{
    bool flag = true;
    for (QString s: languageList) {
        if (!getLanguageList().contains(s)) {
           flag = false;
           break;
        }
    }
    if (flag) {
        if (ukccPanelGsettings->keys().contains(SHOW_LANGUAGE_KEY)) {
            ukccPanelGsettings->set(SHOW_LANGUAGE_KEY, languageList);
        } else {
            qCritical() << "ukccPanelGsettings not contains the key: " << SHOW_LANGUAGE_KEY;
        }
    }
}

void AreaInterface::setLanguage(const QString &languageCode)
{
    if (getLanguageList().contains(languageCode)) {
        accountInterface->call("SetLanguage", languageCode);
        Q_EMIT changed("language");
    }
}

// 重新加载输入法配置
void AreaInterface::reloadInputMethodConfig()
{
    if (fcitxInterface && fcitxInterface->isValid()) {
        fcitxInterface->call("ReloadConfig");
    }
}

// 修改输入法启用状态
void AreaInterface::changeInputMethod(const QString &inputMethod, const bool &add)
{
    QString fileTextString;
    QFile configFile(FCITX_CONFIG_FILE);
    if (configFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream stream(&configFile);
        fileTextString = stream.readAll();

        if (!fileTextString.contains(inputMethod)) {
            configFile.close();
            return;
        }
        QString addString = inputMethod + ":True";
        QString removeString = inputMethod + ":False";
        if (add) {
            fileTextString.replace(removeString, addString);
        } else {
            fileTextString.replace(addString, removeString);
        }
        stream << fileTextString;
        configFile.close();
        reloadInputMethodConfig();
    }
}

// 判断输入法是否已启用
bool AreaInterface::isAddedInputMethod(const QString &inputMethod)
{
    QString fileTextString;
    QFile mFile(FCITX_CONFIG_FILE);
    if (mFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&mFile);
        fileTextString = stream.readAll();
        QString string = inputMethod + ":True";
        mFile.close();
        return fileTextString.contains(string);
    }
    qCritical() << "open " << FCITX_CONFIG_FILE << " error.";
    return false;
}

// 打开语言特性安装入口
void AreaInterface::showInstallLanguageFeatures()
{
    const QStringList args = {
        QStringLiteral("-m"),
        QStringLiteral("area"),
        QStringLiteral("installLanguageFeatures"),
    };
    const bool started = QProcess::startDetached(QStringLiteral("/usr/bin/ukui-control-center"), args);
    qDebug() << Q_FUNC_INFO << "start ukui-control-center" << args << started;
}
