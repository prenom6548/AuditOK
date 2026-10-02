/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <QtTest>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QSignalSpy>

#define private public
#include "../../../src/utils/weather-helper.h"
#undef private

#include "app-manager.h"

using namespace Sidebar;

namespace {
constexpr auto WeatherSchema = "org.kylin-weather.settings";
constexpr auto WeatherKey = "weather";

QString weatherPayload(const QString &city = QStringLiteral("Beijing"),
                       const QString &condition = QStringLiteral("Sunny"),
                       const QString &temperature = QStringLiteral("26C"),
                       const QString &icon = QStringLiteral("weather-clear"))
{
    return QStringList {
        QStringLiteral("province"),
        QStringLiteral("district"),
        city,
        condition,
        QStringLiteral("wind"),
        temperature,
        QStringLiteral("humidity"),
        QStringLiteral("quality"),
        icon
    }.join(QStringLiteral(","));
}
} // namespace

class weatherHelper_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void cleanupTestCase();

    void test_constructor_withoutSchemaKeepsEmpty();
    void test_constructor_withoutWeatherKeyKeepsEmpty();
    void test_constructor_readsWeatherFromSettings();
    void test_extractWeatherInfo_rejectsShortData();
    void test_extractWeatherInfo_acceptsValidDataAndOverwritesState();
    void test_settingsChanged_ignoresOtherKeys();
    void test_settingsChanged_emitsForValidWeather();
    void test_settingsChanged_doesNotEmitForInvalidWeather();
    void test_openWeather_launchesWeatherApplication();

private:
    void installWeatherSettings(const QString &value);
    int removeGeneratedGcovFiles() const;
};

void weatherHelper_test::init()
{
    QGSettings::resetTestData();
    AppManager::resetTestData();
    removeGeneratedGcovFiles();
}

void weatherHelper_test::cleanup()
{
    QGSettings::resetTestData();
    AppManager::resetTestData();
    removeGeneratedGcovFiles();
}

void weatherHelper_test::cleanupTestCase()
{
    removeGeneratedGcovFiles();
}

void weatherHelper_test::installWeatherSettings(const QString &value)
{
    QGSettings::installSchema(WeatherSchema, {QString::fromLatin1(WeatherKey)});
    QGSettings::setValue(WeatherSchema, QString::fromLatin1(WeatherKey), value);
}

int weatherHelper_test::removeGeneratedGcovFiles() const
{
    int removed = 0;
    QDirIterator it(QDir::currentPath(),
                    {QStringLiteral("*.gcov")},
                    QDir::Files,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        if (QFile::remove(it.next())) {
            ++removed;
        }
    }
    return removed;
}

void weatherHelper_test::test_constructor_withoutSchemaKeepsEmpty()
{
    WeatherHelper helper;

    QVERIFY(helper.m_gsettings == nullptr);
    QCOMPARE(helper.getWeather(), QString());
    QCOMPARE(helper.getIcon(), QString());
}

void weatherHelper_test::test_constructor_withoutWeatherKeyKeepsEmpty()
{
    QGSettings::installSchema(WeatherSchema, {QStringLiteral("other")});
    QGSettings::setValue(WeatherSchema, QStringLiteral("other"), QStringLiteral("unused"));

    WeatherHelper helper;

    QVERIFY(helper.m_gsettings != nullptr);
    QCOMPARE(helper.getWeather(), QString());
    QCOMPARE(helper.getIcon(), QString());
}

void weatherHelper_test::test_constructor_readsWeatherFromSettings()
{
    installWeatherSettings(weatherPayload());

    WeatherHelper helper;

    QCOMPARE(helper.getWeather(), QStringLiteral("BeijingSunny26C"));
    QCOMPARE(helper.getIcon(), QStringLiteral("weather-clear"));
}

void weatherHelper_test::test_extractWeatherInfo_rejectsShortData()
{
    WeatherHelper helper;
    helper.m_weather = QStringLiteral("old weather");
    helper.m_icon = QStringLiteral("old icon");

    QVERIFY(!helper.extractWeatherInfo(QStringLiteral("a,b,c,d,e,f")));
    QCOMPARE(helper.getWeather(), QStringLiteral("old weather"));
    QCOMPARE(helper.getIcon(), QStringLiteral("old icon"));
}

void weatherHelper_test::test_extractWeatherInfo_acceptsValidDataAndOverwritesState()
{
    WeatherHelper helper;

    QVERIFY(helper.extractWeatherInfo(weatherPayload(QStringLiteral("Xi'an"),
                                                     QStringLiteral("Cloudy"),
                                                     QStringLiteral("19C"),
                                                     QStringLiteral("weather-cloudy"))));
    QCOMPARE(helper.getWeather(), QStringLiteral("Xi'anCloudy19C"));
    QCOMPARE(helper.getIcon(), QStringLiteral("weather-cloudy"));
}

void weatherHelper_test::test_settingsChanged_ignoresOtherKeys()
{
    installWeatherSettings(weatherPayload());
    WeatherHelper helper;
    QSignalSpy spy(&helper, &WeatherHelper::weatherInfoChanged);

    QGSettings::setValue(WeatherSchema, QString::fromLatin1(WeatherKey),
                         weatherPayload(QStringLiteral("Shanghai"),
                                        QStringLiteral("Rain"),
                                        QStringLiteral("18C"),
                                        QStringLiteral("weather-showers")));
    helper.m_gsettings->emitChanged(QStringLiteral("other"));

    QCOMPARE(spy.count(), 0);
    QCOMPARE(helper.getWeather(), QStringLiteral("BeijingSunny26C"));
    QCOMPARE(helper.getIcon(), QStringLiteral("weather-clear"));
}

void weatherHelper_test::test_settingsChanged_emitsForValidWeather()
{
    installWeatherSettings(weatherPayload());
    WeatherHelper helper;
    QSignalSpy spy(&helper, &WeatherHelper::weatherInfoChanged);

    QGSettings::setValue(WeatherSchema, QString::fromLatin1(WeatherKey),
                         weatherPayload(QStringLiteral("Shanghai"),
                                        QStringLiteral("Rain"),
                                        QStringLiteral("18C"),
                                        QStringLiteral("weather-showers")));
    helper.m_gsettings->emitChanged(QString::fromLatin1(WeatherKey));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(helper.getWeather(), QStringLiteral("ShanghaiRain18C"));
    QCOMPARE(helper.getIcon(), QStringLiteral("weather-showers"));
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("ShanghaiRain18C"));
    QCOMPARE(spy.at(0).at(1).toString(), QStringLiteral("weather-showers"));
}

void weatherHelper_test::test_settingsChanged_doesNotEmitForInvalidWeather()
{
    installWeatherSettings(weatherPayload());
    WeatherHelper helper;
    QSignalSpy spy(&helper, &WeatherHelper::weatherInfoChanged);

    QGSettings::setValue(WeatherSchema, QString::fromLatin1(WeatherKey), QStringLiteral("bad,data"));
    helper.m_gsettings->emitChanged(QString::fromLatin1(WeatherKey));

    QCOMPARE(spy.count(), 0);
    QCOMPARE(helper.getWeather(), QStringLiteral("BeijingSunny26C"));
    QCOMPARE(helper.getIcon(), QStringLiteral("weather-clear"));
}

void weatherHelper_test::test_openWeather_launchesWeatherApplication()
{
    WeatherHelper helper;

    helper.openWeather();

    QCOMPARE(AppManager::launchCount(), 1);
    QCOMPARE(AppManager::lastDesktopFile(), QStringLiteral("/usr/share/applications/kylin-weather.desktop"));
    QCOMPARE(AppManager::lastApplicationName(), QStringLiteral("kylin-weather"));
}

QTEST_MAIN(weatherHelper_test)

#include "weather-helper-test.moc"
