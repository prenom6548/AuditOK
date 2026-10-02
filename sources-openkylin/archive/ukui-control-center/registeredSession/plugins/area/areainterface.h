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
#ifndef AREAINTERFACE_H
#define AREAINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>
#include "../../universalinterface.h"
#include <kysdk/kysdk-system/libkysysinfo.h>
#ifdef Nile
#include <kysdk/kysdk-system/libkydate.h>
#endif

class AreaInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Area")
    Q_PROPERTY(QStringList formatCountryList READ getFormatCountryList)
    Q_PROPERTY(QStringList calendarList READ getCalendarList)
    Q_PROPERTY(QStringList firstDayList READ getFirstDayList)
    Q_PROPERTY(QStringList dateFormatList READ getDateFormatList)
    Q_PROPERTY(QStringList timeFormatList READ getTimeFormatList)
    Q_PROPERTY(QStringList languageList READ getLanguageList)
    Q_PROPERTY(QStringList showLanguageList READ getShowLanguageList)
    Q_PROPERTY(QString formatCountry READ getFormatCountry)
    Q_PROPERTY(QString calendar READ getCalendar)
    Q_PROPERTY(QString firstDay READ getFirstDay)
    Q_PROPERTY(QString dateFormat READ getDateFormat)
    Q_PROPERTY(QString timeFormat READ getTimeFormat)
    Q_PROPERTY(QString language READ getLanguage)
    Q_PROPERTY(QString lunarDate READ getLunarDate)

public:
     AreaInterface();
    ~AreaInterface();
    void initConnect();

protected:
    // 对属性的访问函数
    QStringList getFormatCountryList();
    QStringList getCalendarList();
    QStringList getFirstDayList();
    QStringList getDateFormatList();
    QStringList getTimeFormatList();
    QStringList getLanguageList();
    QStringList getShowLanguageList();
    QString getFormatCountry();
    QString getCalendar();
    QString getFirstDay();
    QString getDateFormat();
    QString getTimeFormat();
    QString getLanguage();
    QString getLunarDate();
    bool isOpenkylin();
    bool isV11();

public Q_SLOTS:

    // 对属性的设置函数
    void setFormatCountry(const QString &formatCountry);
    void setCalendar(const QString &calendar);
    void setFirstDay(const QString &firstDay);
    void setDateFormat(const QString &dateFormat);
    void setTimeFormat(const QString &timeFormat);
    void setShowLanguageList(const QStringList &languageList);
    void setLanguage(const QString &languageCode);
    // 修改输入法启用状态
    void changeInputMethod(const QString &inputMethod, const bool &add);
    // 判断输入法是否已启用
    bool isAddedInputMethod(const QString &inputMethod);
    // 打开语言特性安装入口
    void showInstallLanguageFeatures();

Q_SIGNALS:
    void changed(QString keyName);

private:

    // 设置和获取gsetting的string类型的键值
    QString gsettingGet(const QString &key) const;
    void gsettingSet(const QString &key, const QString &value);
    QString gsettingCalendarGet(const QString &key) const;
    // 重新加载输入法配置
    void reloadInputMethodConfig();

private:
    const QString SHOW_LANGUAGE_KEY = QString("showlanguage");
    const QString FCITX_CONFIG_FILE = QDir::homePath() + "/.config/fcitx/profile";
    const QString CALENDAR_KEY      = QString("calendar");
    const QString FIRST_DAY_KEY     = QString("firstday");
    const QString DATE_FORMATE_KEY  = QString("date");
    const QString TIME_FORMAT_KEY   = QString("hoursystem");
    const QString LUNAR_DATE_KEY    = QString("lunarDate");

private:
    QGSettings     *ukccPanelGsettings = nullptr;
    QDBusInterface *accountInterface   = nullptr;
    QDBusInterface *fcitxInterface     = nullptr;
    QGSettings     *styleGsettings     = nullptr;
    QGSettings      *m_calendarGsettings = nullptr;
};

#endif // AREAINTERFACE_H
