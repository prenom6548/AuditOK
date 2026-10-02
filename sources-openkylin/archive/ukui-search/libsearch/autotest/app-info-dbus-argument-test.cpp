#include "app-info-dbus-argument-test.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDateTime>
#include <QTest>

#include "app-info-dbus-argument.h"

using namespace UkuiSearch;

namespace {
void registerTypes()
{
    qDBusRegisterMetaType<ApplicationProperty::Property>();
    qDBusRegisterMetaType<ApplicationPropertyMap>();
    qDBusRegisterMetaType<ApplicationInfoMap>();
}

template <typename T>
T roundTrip(const T &value)
{
    QDBusMessage message = QDBusMessage::createSignal(
        QStringLiteral("/org/ukui/search/test"),
        QStringLiteral("org.ukui.search.test"),
        QStringLiteral("RoundTrip"));
    message << QVariant::fromValue(value);
    return qdbus_cast<T>(message.arguments().constFirst());
}
}

void AppInfoDbusArgumentTest::testPropertyEnumRoundTrip()
{
    registerTypes();

    const ApplicationProperty::Property source = ApplicationProperty::Keywords;
    const ApplicationProperty::Property decoded = roundTrip(source);

    QCOMPARE(decoded, source);
}

void AppInfoDbusArgumentTest::testApplicationPropertyMapRoundTrip()
{
    registerTypes();

    ApplicationPropertyMap source;
    source.insert(ApplicationProperty::DesktopFilePath, QStringLiteral("/usr/share/applications/demo.desktop"));
    source.insert(ApplicationProperty::ModifiedTime, QDateTime(QDate(2024, 1, 2), QTime(3, 4, 5)));
    source.insert(ApplicationProperty::Favorites, 9);

    const ApplicationPropertyMap decoded = roundTrip(source);

    QCOMPARE(decoded.size(), source.size());
    QCOMPARE(decoded.value(ApplicationProperty::DesktopFilePath).toString(),
             QStringLiteral("/usr/share/applications/demo.desktop"));
    QCOMPARE(decoded.value(ApplicationProperty::ModifiedTime).toDateTime(),
             QDateTime(QDate(2024, 1, 2), QTime(3, 4, 5)));
    QCOMPARE(decoded.value(ApplicationProperty::Favorites).toInt(), 9);
}

void AppInfoDbusArgumentTest::testEmptyApplicationPropertyMapRoundTrip()
{
    registerTypes();

    const ApplicationPropertyMap decoded = roundTrip(ApplicationPropertyMap{});

    QVERIFY(decoded.isEmpty());
}

void AppInfoDbusArgumentTest::testApplicationInfoMapRoundTrip()
{
    registerTypes();

    ApplicationInfoMap source;
    ApplicationPropertyMap first;
    first.insert(ApplicationProperty::LocalName, QStringLiteral("Demo"));
    first.insert(ApplicationProperty::Keywords, QStringLiteral("demo;search"));
    ApplicationPropertyMap second;
    second.insert(ApplicationProperty::DesktopFilePath, QStringLiteral("/tmp/second.desktop"));
    second.insert(ApplicationProperty::Launched, 1);

    source.insert(QStringLiteral("demo.desktop"), first);
    source.insert(QStringLiteral("second.desktop"), second);

    const ApplicationInfoMap decoded = roundTrip(source);

    QCOMPARE(decoded.size(), 2);
    QCOMPARE(decoded.value(QStringLiteral("demo.desktop")).value(ApplicationProperty::LocalName).toString(),
             QStringLiteral("Demo"));
    QCOMPARE(decoded.value(QStringLiteral("demo.desktop")).value(ApplicationProperty::Keywords).toString(),
             QStringLiteral("demo;search"));
    QCOMPARE(decoded.value(QStringLiteral("second.desktop")).value(ApplicationProperty::DesktopFilePath).toString(),
             QStringLiteral("/tmp/second.desktop"));
    QCOMPARE(decoded.value(QStringLiteral("second.desktop")).value(ApplicationProperty::Launched).toInt(), 1);
}

void AppInfoDbusArgumentTest::testEmptyApplicationInfoMapRoundTrip()
{
    registerTypes();

    const ApplicationInfoMap decoded = roundTrip(ApplicationInfoMap{});

    QVERIFY(decoded.isEmpty());
}

QTEST_MAIN(AppInfoDbusArgumentTest)
