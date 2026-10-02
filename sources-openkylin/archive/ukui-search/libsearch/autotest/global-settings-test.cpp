#include "global-settings-test.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "global-settings-private.h"
#undef private

using namespace UkuiSearch;

namespace {

class ScopedEnvVar
{
public:
    ScopedEnvVar(const QByteArray &name, const QByteArray &value)
        : m_name(name),
          m_hadValue(qEnvironmentVariableIsSet(name.constData())),
          m_oldValue(qgetenv(name.constData()))
    {
        qputenv(m_name, value);
    }

    ~ScopedEnvVar()
    {
        if (m_hadValue) {
            qputenv(m_name, m_oldValue);
        } else {
            qunsetenv(m_name);
        }
    }

private:
    QByteArray m_name;
    bool m_hadValue = false;
    QByteArray m_oldValue;
};

bool writeSchemaFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(content) == content.size();
}

}

void GlobalSettingsTest::testDefaultValuesAndCacheAccess()
{
    GlobalSettingsPrivate settings;

    const double transparency = settings.getValue(TRANSPARENCY_KEY).toDouble();
    QVERIFY(transparency >= 0.0);
    QVERIFY(transparency <= 1.0);
    QVERIFY(!settings.getValue(STYLE_NAME_KEY).toString().isEmpty());
    QVERIFY(settings.getValue(FONT_SIZE_KEY).toInt() > 0);
    QVERIFY(settings.getValue(WINDOW_RADIUS_KEY).toInt() >= 0);
    QVERIFY(!settings.getValue(WIDGET_THEME_NAME_KEY).toString().isEmpty());
    QVERIFY(!settings.getValue(FONT_KEY).toString().isEmpty());

    settings.setValue(QStringLiteral("unit-test-key"), QStringLiteral("unit-test-value"));
    QCOMPARE(settings.getValue(QStringLiteral("unit-test-key")).toString(), QStringLiteral("unit-test-value"));
}

void GlobalSettingsTest::testHandleRadiusSetBranches()
{
    GlobalSettingsPrivate settings;

    settings.handleRadiusSet(QStringLiteral("bigRadius"));
    QCOMPARE(settings.getValue(WINDOW_RADIUS_KEY).toInt(), 12);

    settings.handleRadiusSet(QStringLiteral("mediumRadius"));
    QCOMPARE(settings.getValue(WINDOW_RADIUS_KEY).toInt(), 6);

    settings.handleRadiusSet(QStringLiteral("noRadius"));
    QCOMPARE(settings.getValue(WINDOW_RADIUS_KEY).toInt(), 0);

    settings.handleRadiusSet(QStringLiteral("9, 4"));
    QCOMPARE(settings.getValue(WINDOW_RADIUS_KEY).toInt(), 9);

    settings.handleRadiusSet(QStringLiteral("unchanged"));
    QCOMPARE(settings.getValue(WINDOW_RADIUS_KEY).toInt(), 9);
}

void GlobalSettingsTest::testSingletonReturnsConfiguredDefaults()
{
    GlobalSettings &settings = GlobalSettings::getInstance();

    QVERIFY(settings.getValue(TRANSPARENCY_KEY).isValid());
    QVERIFY(settings.getValue(WINDOW_RADIUS_KEY).isValid());
}

void GlobalSettingsTest::testSchemaDrivenUpdatesEmitSignalsAndRefreshCache()
{
    QTemporaryDir schemaDir;
    QVERIFY(schemaDir.isValid());

    const QByteArray personaliseSchema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<schemalist>"
        "  <schema id=\"org.ukui.control-center.personalise\" path=\"/org/ukui/control-center/personalise/\">"
        "    <key name=\"transparency\" type=\"d\">"
        "      <default>0.7</default>"
        "    </key>"
        "  </schema>"
        "</schemalist>";
    const QByteArray styleSchema =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<schemalist>"
        "  <schema id=\"org.ukui.style\" path=\"/org/ukui/style/\">"
        "    <key name=\"styleName\" type=\"s\">"
        "      <default>'ukui-light'</default>"
        "    </key>"
        "    <key name=\"systemFontSize\" type=\"d\">"
        "      <default>11.0</default>"
        "    </key>"
        "    <key name=\"systemFont\" type=\"s\">"
        "      <default>'Noto Sans CJK SC'</default>"
        "    </key>"
        "    <key name=\"iconThemeName\" type=\"s\">"
        "      <default>'default'</default>"
        "    </key>"
        "    <key name=\"windowRadius\" type=\"s\">"
        "      <default>'12, 12'</default>"
        "    </key>"
        "    <key name=\"widgetThemeName\" type=\"s\">"
        "      <default>'default'</default>"
        "    </key>"
        "  </schema>"
        "</schemalist>";

    QVERIFY(writeSchemaFile(schemaDir.filePath(QStringLiteral("org.ukui.control-center.personalise.gschema.xml")),
                            personaliseSchema));
    QVERIFY(writeSchemaFile(schemaDir.filePath(QStringLiteral("org.ukui.style.gschema.xml")),
                            styleSchema));
    QCOMPARE(QProcess::execute(QStringLiteral("glib-compile-schemas"),
                               QStringList{schemaDir.path()}),
             0);

    ScopedEnvVar schemaEnv("GSETTINGS_SCHEMA_DIR", schemaDir.path().toUtf8());
    ScopedEnvVar backendEnv("GSETTINGS_BACKEND", QByteArrayLiteral("memory"));

    GlobalSettingsPrivate settings;
    QVERIFY(settings.m_transGsettings != nullptr);
    QVERIFY(settings.m_themeGsettings != nullptr);

    QSignalSpy spy(&settings, &GlobalSettingsPrivate::valueChanged);

    settings.m_transGsettings->set(TRANSPARENCY_KEY, 0.42);
    QTRY_COMPARE(settings.getValue(TRANSPARENCY_KEY).toDouble(), 0.42);

    settings.m_themeGsettings->set(STYLE_NAME_KEY, QStringLiteral("ukui-dark"));
    QTRY_COMPARE(settings.getValue(STYLE_NAME_KEY).toString(), QStringLiteral("ukui-dark"));

    settings.m_themeGsettings->set(FONT_SIZE_KEY, 15.0);
    QTRY_COMPARE(settings.getValue(FONT_SIZE_KEY).toDouble(), 15.0);

    settings.m_themeGsettings->set(FONT_KEY, QStringLiteral("Unit Test Sans"));
    QTRY_COMPARE(settings.getValue(FONT_KEY).toString(), QStringLiteral("Unit Test Sans"));

    settings.m_themeGsettings->set(ICON_THEME_KEY, QStringLiteral("unit-icons"));
    QTRY_COMPARE(spy.count() >= 5, true);

    settings.m_themeGsettings->set(WINDOW_RADIUS_KEY, QStringLiteral("mediumRadius"));
    QTRY_COMPARE(settings.getValue(WINDOW_RADIUS_KEY).toInt(), 6);

    settings.m_themeGsettings->set(WIDGET_THEME_NAME_KEY, QStringLiteral("unit-theme"));
    QTRY_COMPARE(settings.getValue(WINDOW_RADIUS_KEY).toString(), QStringLiteral("mediumRadius"));

    bool sawTransparency = false;
    bool sawStyle = false;
    bool sawFontSize = false;
    bool sawFont = false;
    bool sawIcon = false;
    bool sawRadius = false;
    bool sawWidgetTheme = false;
    for (const auto &call : spy) {
        const QString key = call.at(0).toString();
        const QVariant value = call.at(1);
        if (key == TRANSPARENCY_KEY && qFuzzyCompare(value.toDouble(), 0.42)) {
            sawTransparency = true;
        } else if (key == STYLE_NAME_KEY && value.toString() == QStringLiteral("ukui-dark")) {
            sawStyle = true;
        } else if (key == FONT_SIZE_KEY && qFuzzyCompare(value.toDouble(), 15.0)) {
            sawFontSize = true;
        } else if (key == FONT_KEY && value.toString() == QStringLiteral("Unit Test Sans")) {
            sawFont = true;
        } else if (key == ICON_THEME_KEY && value.toString() == QStringLiteral("unit-icons")) {
            sawIcon = true;
        } else if (key == WINDOW_RADIUS_KEY && value.toInt() == 6) {
            sawRadius = true;
        } else if (key == WIDGET_THEME_NAME_KEY && value.toString() == QStringLiteral("mediumRadius")) {
            sawWidgetTheme = true;
        }
    }

    QVERIFY(sawTransparency);
    QVERIFY(sawStyle);
    QVERIFY(sawFontSize);
    QVERIFY(sawFont);
    QVERIFY(sawIcon);
    QVERIFY(sawRadius);
    QVERIFY(sawWidgetTheme);
}

QTEST_MAIN(GlobalSettingsTest)
