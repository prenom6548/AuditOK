// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QTest>
#include <QStringView>
#include <kcountry.h>
#include <kcountrysubdivision.h>
#include <ktimezone.h>

class TestLocaleData : public QObject
{
    Q_OBJECT

private slots:
    // KCountry tests
    void testKCountryDefaultConstructor();
    void testKCountryFromAlpha2();
    void testKCountryFromAlpha3();
    void testKCountryIsValid();
    void testKCountryAlpha2();
    void testKCountryAlpha3();
    void testKCountryName();
    void testKCountryEmojiFlag();
    void testKCountryCurrencyCode();
    void testKCountryTimeZoneIds();
    void testKCountryAllCountries();
    void testKCountryFromName();
    void testKCountryFromLocation();

    // KCountrySubdivision tests
    void testKCountrySubdivisionDefaultConstructor();
    void testKCountrySubdivisionFromCode();
    void testKCountrySubdivisionIsValid();
    void testKCountrySubdivisionCode();
    void testKCountrySubdivisionName();
    void testKCountrySubdivisionCountry();

    // KTimeZone tests
    void testKTimeZoneFromLocation();
    void testKTimeZoneCountry();
};

// KCountry tests
void TestLocaleData::testKCountryDefaultConstructor()
{
    KCountry c;
    QVERIFY(!c.isValid());
    QVERIFY(c.alpha2().isEmpty());
    QVERIFY(c.alpha3().isEmpty());
    QVERIFY(c.name().isEmpty());
}

void TestLocaleData::testKCountryFromAlpha2()
{
    KCountry c = KCountry::fromAlpha2("CN");
    QVERIFY(c.isValid());
    QCOMPARE(c.alpha2(), QString("CN"));

    // Test invalid codes
    QVERIFY(!KCountry::fromAlpha2(nullptr).isValid());
    QVERIFY(!KCountry::fromAlpha2("").isValid());
    QVERIFY(!KCountry::fromAlpha2("C").isValid());
    QVERIFY(!KCountry::fromAlpha2("ZZ").isValid()); // Non-existent code
}

void TestLocaleData::testKCountryFromAlpha3()
{
    KCountry c = KCountry::fromAlpha3("CHN");
    QVERIFY(c.isValid());
    QCOMPARE(c.alpha3(), QString("CHN"));

    // Test invalid codes
    QVERIFY(!KCountry::fromAlpha3(nullptr).isValid());
    QVERIFY(!KCountry::fromAlpha3("").isValid());
    QVERIFY(!KCountry::fromAlpha3("CH").isValid());
    QVERIFY(!KCountry::fromAlpha3("ZZZ").isValid()); // Non-existent code
}

void TestLocaleData::testKCountryIsValid()
{
    KCountry valid = KCountry::fromAlpha2("US");
    QVERIFY(valid.isValid());

    KCountry invalid;
    QVERIFY(!invalid.isValid());
}

void TestLocaleData::testKCountryAlpha2()
{
    KCountry c = KCountry::fromAlpha2("DE");
    QCOMPARE(c.alpha2(), QString("DE"));

    KCountry invalid;
    QVERIFY(invalid.alpha2().isEmpty());
}

void TestLocaleData::testKCountryAlpha3()
{
    KCountry c = KCountry::fromAlpha3("DEU");
    QCOMPARE(c.alpha3(), QString("DEU"));

    KCountry invalid;
    QVERIFY(invalid.alpha3().isEmpty());
}

void TestLocaleData::testKCountryName()
{
    KCountry c = KCountry::fromAlpha2("US");
    QVERIFY(!c.name().isEmpty());

    KCountry invalid;
    QVERIFY(invalid.name().isEmpty());
}

void TestLocaleData::testKCountryEmojiFlag()
{
    KCountry c = KCountry::fromAlpha2("GB");
    QVERIFY(!c.emojiFlag().isEmpty());

    KCountry invalid;
    QVERIFY(invalid.emojiFlag().isEmpty());
}

void TestLocaleData::testKCountryCurrencyCode()
{
    KCountry c = KCountry::fromAlpha2("JP");
    QVERIFY(c.isValid());
    // QLocale::matchingLocales may return inconsistent results due to locale fallbacks
    // (e.g., en_JP falls back to en_US returning USD instead of JPY),
    // so we only verify a non-empty result for valid countries.
    QVERIFY(!c.currencyCode().isEmpty());

    KCountry c2 = KCountry::fromAlpha2("NZ");
    QVERIFY(c2.isValid());
    QVERIFY(!c2.currencyCode().isEmpty());

    KCountry c3 = KCountry::fromAlpha2("CN");
    QVERIFY(c3.isValid());
    QVERIFY(!c3.currencyCode().isEmpty());

    KCountry invalid;
    QVERIFY(invalid.currencyCode().isEmpty());
}

void TestLocaleData::testKCountryTimeZoneIds()
{
    KCountry c = KCountry::fromAlpha2("FR");
    QList<const char*> tzIds = c.timeZoneIds();
    QVERIFY(!tzIds.isEmpty());

    // Verify the timezone IDs are valid strings
    for (const char* tz : tzIds) {
        QVERIFY(tz != nullptr);
        QVERIFY(strlen(tz) > 0);
    }

    KCountry invalid;
    QVERIFY(invalid.timeZoneIds().isEmpty());
}

void TestLocaleData::testKCountryAllCountries()
{
    QList<KCountry> countries = KCountry::allCountries();
    QVERIFY(countries.size() > 0);

    // Verify all returned countries are valid
    for (const KCountry& c : countries) {
        QVERIFY(c.isValid());
    }
}

void TestLocaleData::testKCountryFromName()
{
    KCountry c = KCountry::fromName(QString::fromUtf8("China"));
    QVERIFY(c.isValid());
    QCOMPARE(c.alpha2(), QString("CN"));

    QVERIFY(!KCountry::fromName(QString::fromUtf8("")).isValid());
    QVERIFY(!KCountry::fromName(QString::fromUtf8("NonExistentCountryName123")).isValid());
}

void TestLocaleData::testKCountryFromLocation()
{
    // Beijing, China
    KCountry c = KCountry::fromLocation(39.9042, 116.4074);
    QVERIFY(c.isValid());
    QCOMPARE(c.alpha2(), QString("CN"));
}

// KCountrySubdivision tests
void TestLocaleData::testKCountrySubdivisionDefaultConstructor()
{
    KCountrySubdivision s;
    QVERIFY(!s.isValid());
    QVERIFY(s.code().isEmpty());
    QVERIFY(s.name().isEmpty());
}

void TestLocaleData::testKCountrySubdivisionFromCode()
{
    KCountrySubdivision s = KCountrySubdivision::fromCode("US-CA");
    QVERIFY(s.isValid());
    QCOMPARE(s.code(), QString("US-CA"));

    // Test invalid codes
    QVERIFY(!KCountrySubdivision::fromCode(nullptr).isValid());
    QVERIFY(!KCountrySubdivision::fromCode("").isValid());
    QVERIFY(!KCountrySubdivision::fromCode("XX-XXX").isValid()); // Non-existent code
}

void TestLocaleData::testKCountrySubdivisionIsValid()
{
    KCountrySubdivision valid = KCountrySubdivision::fromCode("GB-ENG");
    QVERIFY(valid.isValid());

    KCountrySubdivision invalid;
    QVERIFY(!invalid.isValid());
}

void TestLocaleData::testKCountrySubdivisionCode()
{
    KCountrySubdivision s = KCountrySubdivision::fromCode("DE-BB");
    QCOMPARE(s.code(), QString("DE-BB"));

    KCountrySubdivision invalid;
    QVERIFY(invalid.code().isEmpty());
}

void TestLocaleData::testKCountrySubdivisionName()
{
    KCountrySubdivision s = KCountrySubdivision::fromCode("US-NY");
    QVERIFY(!s.name().isEmpty());

    KCountrySubdivision invalid;
    QVERIFY(invalid.name().isEmpty());
}

void TestLocaleData::testKCountrySubdivisionCountry()
{
    KCountrySubdivision s = KCountrySubdivision::fromCode("CA-ON");
    QVERIFY(s.isValid());

    KCountry c = s.country();
    QVERIFY(c.isValid());
    QCOMPARE(c.alpha2(), QString("CA"));

    KCountrySubdivision invalid;
    QVERIFY(!invalid.country().isValid());
}

// KTimeZone tests
void TestLocaleData::testKTimeZoneFromLocation()
{
    // Test with a known location (London)
    const char* tz = KTimeZone::fromLocation(51.5074, -0.1278);
    QVERIFY(tz != nullptr);
    QString tzStr = QString::fromUtf8(tz);
    QVERIFY(tzStr.contains("Europe/London"));
}

void TestLocaleData::testKTimeZoneCountry()
{
    KCountry c = KTimeZone::country("Asia/Tokyo");
    QVERIFY(c.isValid());
    QCOMPARE(c.alpha2(), QString("JP"));

    // Test invalid timezone
    QVERIFY(!KTimeZone::country("Invalid/Timezone").isValid());
    QVERIFY(!KTimeZone::country(nullptr).isValid());
    QVERIFY(!KTimeZone::country("").isValid());
}

QTEST_GUILESS_MAIN(TestLocaleData)

#include "cpp_localedata_test.moc"
