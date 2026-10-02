#include <QtTest>
#include <QDBusConnection>

#define private public
#define protected public
#include "plugin/tray-item.h"
#undef protected
#undef private

QIcon extractIcon(const QIcon &icon, const QIcon &defaultValue);

class trayItem_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void test_extractIconHelper();
    void test_basicState();
};

void trayItem_test::initTestCase()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session DBus is not available.");
    }
}

void trayItem_test::test_extractIconHelper()
{
    const QIcon fallback = QIcon::fromTheme(QStringLiteral("application-x-executable"));
    const QIcon fromNull = extractIcon(QIcon(), fallback);
    QCOMPARE(fromNull.cacheKey(), fallback.cacheKey());

    const QIcon icon = QIcon::fromTheme(QStringLiteral("folder"));
    const QIcon fromIcon = extractIcon(icon, fallback);
    QCOMPARE(fromIcon.cacheKey(), icon.cacheKey());
}

void trayItem_test::test_basicState()
{
    TrayItem item(QStringLiteral("org.imitate.Service/StatusNotifierItem"));

    QCOMPARE(item.source(), QStringLiteral("org.imitate.Service/StatusNotifierItem"));

    item.setDesktop(QStringLiteral("imitate.desktop"));
    QCOMPARE(item.desktop(), QStringLiteral("imitate.desktop"));

    item.setFixed(true);
    QCOMPARE(item.fixed(), true);
    item.setRecordOrder(true);
    QCOMPARE(item.recordOrder(), true);

    item.setFixed(false);
    item.setRecordOrder(false);
    QCOMPARE(item.fixed(), false);
    QCOMPARE(item.recordOrder(), false);
}

QTEST_MAIN(trayItem_test)

#include "trayItemTest.moc"
