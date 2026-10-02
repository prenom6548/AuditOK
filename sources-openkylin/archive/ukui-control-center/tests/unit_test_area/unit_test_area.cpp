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
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QStringList>
#include <QMetaObject>
#include "../../registeredSession/plugins/area/areainterface.h"

class AreaInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAreaInterface = new AreaInterface();
    }

    static void TearDownTestSuite()
    {
        if (m_pAreaInterface) {
            delete m_pAreaInterface;
            m_pAreaInterface = nullptr;
        }
    }

    static AreaInterface* m_pAreaInterface;
};

AreaInterface *AreaInterfaceTest::m_pAreaInterface = nullptr;

TEST_F(AreaInterfaceTest, formatCountry)
{
    std::string oldValue = m_pAreaInterface->getFormatCountry().toStdString();
    m_pAreaInterface->setFormatCountry("zh_CN.UTF-8");
    std::string newValue = m_pAreaInterface->getFormatCountry().toStdString();
    ASSERT_STREQ(newValue.c_str(), "zh_CN.UTF-8");
    m_pAreaInterface->setFormatCountry(oldValue.c_str());
}

TEST_F(AreaInterfaceTest, formatCountryList)
{
    std::string value = m_pAreaInterface->getFormatCountryList().at(0).toStdString();
    ASSERT_STREQ(value.c_str(), "en_US.UTF-8");
}

TEST_F(AreaInterfaceTest, calendar)
{
    std::string oldValue = m_pAreaInterface->getCalendar().toStdString();
    m_pAreaInterface->setCalendar("lunar");
    std::string newValue = m_pAreaInterface->getCalendar().toStdString();
    ASSERT_STREQ(newValue.c_str(), "lunar");
    m_pAreaInterface->setCalendar(oldValue.c_str());
}

TEST_F(AreaInterfaceTest, calendarList)
{
    std::string value = m_pAreaInterface->getCalendarList().at(0).toStdString();
    ASSERT_STREQ(value.c_str(), "solarlunar");
}

TEST_F(AreaInterfaceTest, firstDayList)
{
    std::string value = m_pAreaInterface->getFirstDayList().at(0).toStdString();
    ASSERT_STREQ(value.c_str(), "monday");
}

TEST_F(AreaInterfaceTest, firstDay)
{
    std::string oldValue = m_pAreaInterface->getFirstDay().toStdString();
    m_pAreaInterface->setFirstDay("monday");
    std::string newValue = m_pAreaInterface->getFirstDay().toStdString();
    ASSERT_STREQ(newValue.c_str(), "monday");
    m_pAreaInterface->setFirstDay(oldValue.c_str());
}

TEST_F(AreaInterfaceTest, dateFormat)
{
    std::string oldValue = m_pAreaInterface->getDateFormat().toStdString();
    m_pAreaInterface->setDateFormat("cn");
    std::string newValue = m_pAreaInterface->getDateFormat().toStdString();
    ASSERT_STREQ(newValue.c_str(), "cn");
    m_pAreaInterface->setDateFormat(oldValue.c_str());
}

TEST_F(AreaInterfaceTest, dateFormatList)
{
    std::string value = m_pAreaInterface->getDateFormatList().at(0).toStdString();
    ASSERT_STREQ(value.c_str(), "cn");
}

TEST_F(AreaInterfaceTest, timeFormat)
{
    std::string oldValue = m_pAreaInterface->getTimeFormat().toStdString();
    m_pAreaInterface->setTimeFormat("24");
    std::string newValue = m_pAreaInterface->getTimeFormat().toStdString();
    ASSERT_STREQ(newValue.c_str(), "24");
    m_pAreaInterface->setTimeFormat(oldValue.c_str());
}

TEST_F(AreaInterfaceTest, timeFormatList)
{
    std::string value = m_pAreaInterface->getTimeFormatList().at(0).toStdString();
    ASSERT_STREQ(value.c_str(), "12");
}

TEST_F(AreaInterfaceTest, language)
{
    std::string oldValue = m_pAreaInterface->getLanguage().toStdString();
    m_pAreaInterface->setLanguage("zh_CN");
    std::string newValue = m_pAreaInterface->getLanguage().toStdString();
    ASSERT_STREQ(newValue.c_str(), "zh_CN");
    m_pAreaInterface->setLanguage(oldValue.c_str());
}

TEST_F(AreaInterfaceTest, showLanguageList)
{
    QStringList oldValue = m_pAreaInterface->getShowLanguageList();
    QStringList languageList = {"zh_CN", "en"};
    m_pAreaInterface->setShowLanguageList(languageList);
    std::string newValue = m_pAreaInterface->getShowLanguageList().at(0).toStdString();
    ASSERT_STREQ(newValue.c_str(), "zh_CN");
    m_pAreaInterface->setShowLanguageList(oldValue);
}

TEST_F(AreaInterfaceTest, languageList)
{
    std::string value = m_pAreaInterface->getLanguageList().at(0).toStdString();
    ASSERT_STREQ(value.c_str(), "zh_CN");
}

TEST_F(AreaInterfaceTest, lunarDate)
{
    std::string value = m_pAreaInterface->getLunarDate().toStdString();
    ASSERT_GE(value.size(), 0);
}

TEST_F(AreaInterfaceTest, setShowLanguageListInvalid)
{
    QStringList oldValue = m_pAreaInterface->getShowLanguageList();
    QStringList invalidList = {"zh_CN", "ukcc_not_exist_language"};
    m_pAreaInterface->setShowLanguageList(invalidList);
    QStringList newValue = m_pAreaInterface->getShowLanguageList();
    ASSERT_EQ(newValue, oldValue);
}

TEST_F(AreaInterfaceTest, gsettingInvalidKeyPaths)
{
    QString ret = m_pAreaInterface->gsettingGet("ukcc-invalid-key");
    ASSERT_TRUE(ret.isEmpty());
    m_pAreaInterface->gsettingSet("ukcc-invalid-key", "value");
    SUCCEED();
}

TEST_F(AreaInterfaceTest, calendarGsettingNullPath)
{
    QGSettings *oldCalendar = m_pAreaInterface->m_calendarGsettings;
    m_pAreaInterface->m_calendarGsettings = nullptr;
    QString ret = m_pAreaInterface->gsettingCalendarGet("lunarDate");
    ASSERT_TRUE(ret.isEmpty());
    m_pAreaInterface->m_calendarGsettings = oldCalendar;
}

TEST_F(AreaInterfaceTest, systemFlagsAndLanguageInvalidPath)
{
    bool openkylin = m_pAreaInterface->isOpenkylin();
    bool v11 = m_pAreaInterface->isV11();
    ASSERT_TRUE(openkylin || !openkylin);
    ASSERT_TRUE(v11 || !v11);

    QString oldLang = m_pAreaInterface->getLanguage();
    m_pAreaInterface->setLanguage("ukcc_not_exist_language");
    ASSERT_EQ(m_pAreaInterface->getLanguage(), oldLang);
}

TEST_F(AreaInterfaceTest, gsettingsChangedSignalBranches)
{
    int changedCount = 0;
    QObject::connect(m_pAreaInterface, &AreaInterface::changed, [&](const QString &) { changedCount++; });

    QMetaObject::invokeMethod(m_pAreaInterface->ukccPanelGsettings, "changed", Q_ARG(QString, "calendar"));
    QMetaObject::invokeMethod(m_pAreaInterface->ukccPanelGsettings, "changed", Q_ARG(QString, "firstday"));
    QMetaObject::invokeMethod(m_pAreaInterface->ukccPanelGsettings, "changed", Q_ARG(QString, "date"));
    QMetaObject::invokeMethod(m_pAreaInterface->ukccPanelGsettings, "changed", Q_ARG(QString, "hoursystem"));
    QMetaObject::invokeMethod(m_pAreaInterface->ukccPanelGsettings, "changed", Q_ARG(QString, "showlanguage"));
    QMetaObject::invokeMethod(m_pAreaInterface->styleGsettings, "changed", Q_ARG(QString, "iconThemeName"));
    if (m_pAreaInterface->m_calendarGsettings) {
        QMetaObject::invokeMethod(m_pAreaInterface->m_calendarGsettings, "changed", Q_ARG(QString, "lunarDate"));
    }
    ASSERT_GE(changedCount, 6);
}

TEST_F(AreaInterfaceTest, missingShowLanguageKeyPaths)
{
    QGSettings *oldPanel = m_pAreaInterface->ukccPanelGsettings;
    QGSettings fakePanel("org.mate.background");
    m_pAreaInterface->ukccPanelGsettings = &fakePanel;

    QStringList showList = m_pAreaInterface->getShowLanguageList();
    ASSERT_TRUE(showList.isEmpty());

    m_pAreaInterface->setShowLanguageList(QStringList({"zh_CN"}));
    ASSERT_TRUE(m_pAreaInterface->getShowLanguageList().isEmpty());
    m_pAreaInterface->ukccPanelGsettings = oldPanel;
}

TEST_F(AreaInterfaceTest, calendarMissingKeyPath)
{
    QGSettings *oldCalendar = m_pAreaInterface->m_calendarGsettings;
    QGSettings fakeCalendar("org.mate.background");
    m_pAreaInterface->m_calendarGsettings = &fakeCalendar;
    QString ret = m_pAreaInterface->gsettingCalendarGet("lunarDate");
    ASSERT_TRUE(ret.isEmpty());
    m_pAreaInterface->m_calendarGsettings = oldCalendar;
}
