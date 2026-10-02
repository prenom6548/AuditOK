#include "areatest.h"

AreaTest::AreaTest()
{

}

AreaTest::~AreaTest()
{

}

void AreaTest::initTestCase()
{
    area = new AreaInterface();
    formatCountry    = area->property("formatCountry").toString();
    calendar         = area->property("calendar").toString();
    firstDay         = area->property("firstDay").toString();
    dateFormat       = area->property("dateFormat").toString();
    timeFormat       = area->property("timeFormat").toString();
    showLanguageList = area->property("showLanguageList").toStringList();
    language         = area->property("language").toString();
}

void AreaTest::cleanupTestCase()
{
    // 避免单元测试修改了系统配置，因此重置
    area->setFormatCountry(formatCountry);
    area->setCalendar(calendar);
    area->setFirstDay(firstDay);
    area->setDateFormat(dateFormat);
    area->setTimeFormat(timeFormat);
    area->setShowLanguageList(showLanguageList);
    area->setCalendar(language);

    delete area;
    area = nullptr;
}

void AreaTest::testFormatCountry() const
{
    area->setFormatCountry("aaa");
    QCOMPARE(formatCountry, area->property("formatCountry").toString());

    area->setFormatCountry("en_US.UTF-8");
    QCOMPARE("en_US.UTF-8", area->property("formatCountry").toString());

    area->setFormatCountry("zh_CN.UTF-8");
    QCOMPARE("zh_CN.UTF-8", area->property("formatCountry").toString());

    area->setFormatCountry("bo_CN.UTF-8");
    QCOMPARE("bo_CN.UTF-8", area->property("formatCountry").toString());
}

void AreaTest::testCalendar() const
{
    area->setCalendar("aaa");
    QCOMPARE(calendar, area->property("calendar").toString());

    area->setCalendar("lunar");
    QCOMPARE("lunar", area->property("calendar").toString());

    area->setCalendar("solarlunar");
    QCOMPARE("solarlunar", area->property("calendar").toString());
}

void AreaTest::testFirstDay() const
{
    area->setFirstDay("aaa");
    QCOMPARE(firstDay, area->property("firstDay").toString());

    area->setFirstDay("sunday");
    QCOMPARE("sunday", area->property("firstDay").toString());

    area->setFirstDay("monday");
    QCOMPARE("monday", area->property("firstDay").toString());
}

void AreaTest::testDateFormat() const
{
    area->setDateFormat("aaa");
    QCOMPARE(dateFormat, area->property("dateFormat").toString());

    area->setDateFormat("cn");
    QCOMPARE("cn", area->property("dateFormat").toString());

    area->setDateFormat("en");
    QCOMPARE("en", area->property("dateFormat").toString());
}

void AreaTest::testTimeFormat() const
{
    area->setTimeFormat("aaa");
    QCOMPARE(timeFormat, area->property("timeFormat").toString());

    area->setTimeFormat("12");
    QCOMPARE("12", area->property("timeFormat").toString());

    area->setTimeFormat("24");
    QCOMPARE("24", area->property("timeFormat").toString());
}

void AreaTest::testShowLanguageList() const
{
    QStringList list = {};
    list.append("aaa");
    list.append("bbb");

    area->setShowLanguageList(list);
    QCOMPARE(showLanguageList, area->property("showLanguageList").toStringList());

    list.clear();
    list.append("zh_CN");
    list.append("bo_CN");
    area->setShowLanguageList(list);
    QCOMPARE(list, area->property("showLanguageList").toStringList());
}

void AreaTest::testLanguage() const
{
    area->setLanguage("aaa");
    QCOMPARE(language, area->property("language").toString());

    area->setLanguage("zh_CN");
    QCOMPARE("zh_CN", area->property("language").toString());

    area->setLanguage("en");
    QCOMPARE("en", area->property("language").toString());

    area->setLanguage("bo_CN");
    QCOMPARE("bo_CN", area->property("language").toString());
}

QTEST_MAIN(AreaTest)
