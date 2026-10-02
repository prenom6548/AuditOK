#ifndef APP_INFO_DBUS_ARGUMENT_TEST_H
#define APP_INFO_DBUS_ARGUMENT_TEST_H

#include <QObject>

class AppInfoDbusArgumentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPropertyEnumRoundTrip();
    void testApplicationPropertyMapRoundTrip();
    void testEmptyApplicationPropertyMapRoundTrip();
    void testApplicationInfoMapRoundTrip();
    void testEmptyApplicationInfoMapRoundTrip();
};

#endif // APP_INFO_DBUS_ARGUMENT_TEST_H
