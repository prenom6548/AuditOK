#include "app-data-utils-test.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QTest>

#include "app-data-utils.h"

using namespace UkuiSearch;

namespace {
class ScopedEnv
{
public:
    ScopedEnv(const QByteArray &key, const QByteArray &value)
        : m_key(key),
          m_hadOriginal(qEnvironmentVariableIsSet(key.constData())),
          m_original(qgetenv(key.constData()))
    {
        qputenv(key.constData(), value);
    }

    ~ScopedEnv()
    {
        if (m_hadOriginal) {
            qputenv(m_key.constData(), m_original);
        } else {
            qunsetenv(m_key.constData());
        }
    }

private:
    QByteArray m_key;
    bool m_hadOriginal;
    QByteArray m_original;
};
}

void AppDataUtilsTest::testDesktopPathsFromXdgDataDirsAndHomePath()
{
    ScopedEnv env("XDG_DATA_DIRS", "/usr/share:/opt/share");

    const QStringList paths = AppDataUtils::getDesktopPaths();

    QVERIFY(paths.contains(QStringLiteral("/usr/share/applications")));
    QVERIFY(paths.contains(QStringLiteral("/opt/share/applications")));
    QVERIFY(paths.contains(QDir::cleanPath(QDir::homePath() + QStringLiteral("/.local/share/applications"))));
}

void AppDataUtilsTest::testDuplicateXdgDataDirsAreRemoved()
{
    ScopedEnv env("XDG_DATA_DIRS", "/usr/share:/usr/share");

    const QStringList paths = AppDataUtils::getDesktopPaths();

    QCOMPARE(paths.count(QStringLiteral("/usr/share/applications")), 1);
}

void AppDataUtilsTest::testDesktopPathsWithoutXdgDataDirs()
{
    // When XDG_DATA_DIRS is not set, getenv returns nullptr → loop body skipped
    QByteArray old = qgetenv("XDG_DATA_DIRS");
    qunsetenv("XDG_DATA_DIRS");

    const QStringList paths = AppDataUtils::getDesktopPaths();
    // Should still contain the Android desktop path and config file entries
    QVERIFY(paths.contains(QDir::cleanPath(QDir::homePath() + QStringLiteral("/.local/share/applications"))));

    if (!old.isEmpty()) qputenv("XDG_DATA_DIRS", old);
}

QTEST_MAIN(AppDataUtilsTest)
