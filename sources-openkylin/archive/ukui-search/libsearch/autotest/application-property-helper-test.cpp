#include "application-property-helper-test.h"

#include <QDateTime>
#include <QTest>

#include "application-property-helper.h"

using namespace UkuiSearch;

namespace {
struct MappingExpectation
{
    ApplicationProperty::Property property;
    const char *field;
    QMetaType::Type type;
};
}

Q_DECLARE_METATYPE(MappingExpectation)

void ApplicationPropertyHelperTest::testDefaultConstructorAndUnknownConversion()
{
    ApplicationPropertyHelper helper;
    QVariant value = 42;

    QCOMPARE(helper.info(), ApplicationProperty::Invalid);
    QVERIFY(helper.dataBaseField().isEmpty());
    QCOMPARE(helper.valueType(), QMetaType::UnknownType);
    QVERIFY(helper.toDataBaseString(value).isEmpty());
}

void ApplicationPropertyHelperTest::testPropertyMappings_data()
{
    QTest::addColumn<MappingExpectation>("mapping");

    const MappingExpectation mappings[] = {
        {ApplicationProperty::DesktopFilePath, "DESKTOP_FILE_PATH", QMetaType::QString},
        {ApplicationProperty::ModifiedTime, "MODIFYED_TIME", QMetaType::QDateTime},
        {ApplicationProperty::InsertTime, "INSERT_TIME", QMetaType::QDateTime},
        {ApplicationProperty::LocalName, "LOCAL_NAME", QMetaType::QString},
        {ApplicationProperty::NameEn, "NAME_EN", QMetaType::QString},
        {ApplicationProperty::NameZh, "NAME_ZH", QMetaType::QString},
        {ApplicationProperty::PinyinName, "PINYIN_NAME", QMetaType::QString},
        {ApplicationProperty::FirstLetterOfPinyin, "FIRST_LETTER_OF_PINYIN", QMetaType::QString},
        {ApplicationProperty::FirstLetterAll, "FIRST_LETTER_ALL", QMetaType::QString},
        {ApplicationProperty::Icon, "ICON", QMetaType::QString},
        {ApplicationProperty::Type, "TYPE", QMetaType::QString},
        {ApplicationProperty::Category, "CATEGORY", QMetaType::QString},
        {ApplicationProperty::Exec, "EXEC", QMetaType::QString},
        {ApplicationProperty::Comment, "CATEGORY", QMetaType::QString},
        {ApplicationProperty::Md5, "MD5", QMetaType::QString},
        {ApplicationProperty::LaunchTimes, "LAUNCH_TIMES", QMetaType::Int},
        {ApplicationProperty::Favorites, "FAVORITES", QMetaType::Int},
        {ApplicationProperty::Launched, "LAUNCHED", QMetaType::Int},
        {ApplicationProperty::Top, "TOP", QMetaType::Int},
        {ApplicationProperty::Lock, "LOCK", QMetaType::Int},
        {ApplicationProperty::DontDisplay, "DONT_DISPLAY", QMetaType::Int},
        {ApplicationProperty::AutoStart, "AUTO_START", QMetaType::Int},
        {ApplicationProperty::StartUpWMClass, "START_UP_WMCLASS", QMetaType::QString},
        {ApplicationProperty::Keywords, "KEYWORDS", QMetaType::QString},
        {ApplicationProperty::LocalKeywords, "LOCAL_KEYWORDS", QMetaType::QString},
    };

    for (const auto &mapping : mappings) {
        QTest::addRow("%d", static_cast<int>(mapping.property)) << mapping;
    }
}

void ApplicationPropertyHelperTest::testPropertyMappings()
{
    QFETCH(MappingExpectation, mapping);

    ApplicationPropertyHelper helper(mapping.property);
    QCOMPARE(helper.info(), mapping.property);
    QCOMPARE(helper.dataBaseField(), QString::fromLatin1(mapping.field));
    QCOMPARE(helper.valueType(), mapping.type);

    if (mapping.type == QMetaType::QString) {
        QVariant text = QStringLiteral("value");
        QCOMPARE(helper.toDataBaseString(text), QStringLiteral("value"));
    } else if (mapping.type == QMetaType::QDateTime) {
        QVariant date = QDateTime(QDate(2024, 5, 6), QTime(7, 8, 9));
        QCOMPARE(helper.toDataBaseString(date), QStringLiteral("2024-05-06 07:08:09"));
    } else {
        QVariant number = 7;
        QVERIFY(helper.toDataBaseString(number).isEmpty());
    }
}

void ApplicationPropertyHelperTest::testCopyAndAssignment()
{
    ApplicationPropertyHelper original(ApplicationProperty::Keywords);
    ApplicationPropertyHelper copied(original);
    ApplicationPropertyHelper assigned;
    assigned = original;

    QVERIFY(copied == original);
    QVERIFY(assigned == original);
    QCOMPARE(copied.info(), ApplicationProperty::Keywords);
    QCOMPARE(assigned.info(), ApplicationProperty::Keywords);
    QCOMPARE(assigned.dataBaseField(), QStringLiteral("KEYWORDS"));
}

QTEST_MAIN(ApplicationPropertyHelperTest)
