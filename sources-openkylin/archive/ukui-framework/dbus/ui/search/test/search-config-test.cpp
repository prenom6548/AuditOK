#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QProcess>

#include "search-config.h"

class SearchConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void readsDefaultsFromSearchSettings();
    void updatesDefaultSearchEngine();
    void rejectsUnsupportedDefaultSearchEngine();
    void updatesFileNameSearchEnabled();
};

void SearchConfigTest::readsDefaultsFromSearchSettings()
{
    SearchConfig searchConfig;

    QCOMPARE(searchConfig.GetDefaultSearchEngine(), QStringLiteral("baidu"));
    QCOMPARE(searchConfig.IsFileNameSearchEnabled(), false);
}

void SearchConfigTest::updatesDefaultSearchEngine()
{
    SearchConfig searchConfig;
    QSignalSpy spy(&searchConfig, &SearchConfig::DefaultSearchEngineChanged);

    QVERIFY(searchConfig.SetDefaultSearchEngine(QStringLiteral("sougou")));

    QCOMPARE(searchConfig.GetDefaultSearchEngine(), QStringLiteral("sougou"));
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral("sougou"));
}

void SearchConfigTest::rejectsUnsupportedDefaultSearchEngine()
{
    SearchConfig searchConfig;

    QVERIFY(searchConfig.SetDefaultSearchEngine(QStringLiteral("360")));
    QVERIFY(!searchConfig.SetDefaultSearchEngine(QStringLiteral("invalid-engine")));

    QCOMPARE(searchConfig.GetDefaultSearchEngine(), QStringLiteral("360"));
}

void SearchConfigTest::updatesFileNameSearchEnabled()
{
    SearchConfig searchConfig;
    QSignalSpy spy(&searchConfig, &SearchConfig::FileNameSearchEnabledChanged);

    QVERIFY(searchConfig.SetFileNameSearchEnabled(true));

    QCOMPARE(searchConfig.IsFileNameSearchEnabled(), true);
    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);
}

static bool prepareSchemaDirectory(QString *error)
{
    const QString basePath = QDir::tempPath() + QStringLiteral("/ukui-framework-search-test-%1").arg(QCoreApplication::applicationPid());
    QDir schemaDir(basePath);
    if (schemaDir.exists()) {
        schemaDir.removeRecursively();
    }
    if (!QDir().mkpath(basePath)) {
        *error = QStringLiteral("Failed to create schema dir: %1").arg(basePath);
        return false;
    }

    QFile schemaFile(schemaDir.filePath(QStringLiteral("org.ukui.search.data.gschema.xml")));
    if (!schemaFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        *error = QStringLiteral("Failed to write schema file: %1").arg(schemaFile.errorString());
        return false;
    }
    schemaFile.write(R"xml(<schemalist>
  <schema id="org.ukui.search.settings" path="/org/ukui/ukui-search/settings/">
    <key name="file-index-enable" type="b">
      <default>false</default>
    </key>
    <key name="web-engine" type="s">
      <default>"baidu"</default>
    </key>
  </schema>
</schemalist>
)xml");
    schemaFile.close();

    QProcess compileSchemas;
    compileSchemas.start(QStringLiteral("glib-compile-schemas"), QStringList{basePath});
    if (!compileSchemas.waitForFinished(5000) || compileSchemas.exitCode() != 0) {
        *error = QStringLiteral("glib-compile-schemas failed: %1").arg(QString::fromLocal8Bit(compileSchemas.readAllStandardError()));
        return false;
    }

    qputenv("GSETTINGS_SCHEMA_DIR", basePath.toLocal8Bit());
    qputenv("GSETTINGS_BACKEND", "memory");
    return true;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QString error;
    if (!prepareSchemaDirectory(&error)) {
        qWarning().noquote() << error;
        return 1;
    }

    SearchConfigTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "search-config-test.moc"
