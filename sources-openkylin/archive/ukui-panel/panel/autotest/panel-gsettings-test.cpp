#include <gio/gio.h>

#include "panel-gsettings-test.h"

#include <QDir>
#include <QFile>
#include <QGSettings>
#include <QProcess>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <cstdlib>

#define private public
#include "panel-gsettings.h"
#undef private

namespace {

constexpr auto kPanelSchemaId = "org.ukui.panel.settings";
constexpr auto kPanelSettingsPath = "/org/ukui/panel/settings/";
constexpr auto kPanelSchemaFileName = "org.ukui.panel.settings.gschema.xml";

void resetPanelSettings()
{
    GSettings *settings = g_settings_new_with_path(kPanelSchemaId, kPanelSettingsPath);
    QVERIFY(settings != nullptr);

    g_settings_reset(settings, "panellength");
    g_settings_reset(settings, "paneltype");
    g_settings_reset(settings, "dataislandposition");
    g_settings_reset(settings, "settingsislandposition");
    g_settings_reset(settings, "topbarsize");
    g_settings_sync();

    g_object_unref(settings);
}

QGSettings *createReader(QObject *parent = nullptr)
{
    auto *reader = new QGSettings(kPanelSchemaId, kPanelSettingsPath, parent);
    return reader;
}

QString compileSchemasInDir(const QString &schemaDir)
{
    QProcess process;
    process.setProgram(QStringLiteral("glib-compile-schemas"));
    process.setArguments({schemaDir});
    process.start();
    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished();
        return {};
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }

    return schemaDir;
}

QString createSchemaDirWithoutPanelLength()
{
    QTemporaryDir temporaryDir;
    temporaryDir.setAutoRemove(false);
    if (!temporaryDir.isValid()) {
        return {};
    }

    const QString sourceSchemaPath = QStringLiteral(PANEL_GSETTINGS_TEST_SCHEMA_DIR) + QLatin1Char('/') + QString::fromLatin1(kPanelSchemaFileName);
    QFile sourceFile(sourceSchemaPath);
    if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QString schemaContent = QString::fromUtf8(sourceFile.readAll());
    sourceFile.close();

    const QString keyStart = QStringLiteral("<key name=\"panellength\"");
    const int startIndex = schemaContent.indexOf(keyStart);
    if (startIndex < 0) {
        return {};
    }

    const int endIndex = schemaContent.indexOf(QStringLiteral("</key>"), startIndex);
    if (endIndex < 0) {
        return {};
    }

    schemaContent.remove(startIndex, endIndex - startIndex + QStringLiteral("</key>").size());

    QFile targetFile(temporaryDir.path() + QLatin1Char('/') + QString::fromLatin1(kPanelSchemaFileName));
    if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return {};
    }
    targetFile.write(schemaContent.toUtf8());
    targetFile.close();

    return compileSchemasInDir(temporaryDir.path());
}

bool runPanelGSettingsChildMode()
{
    const QByteArray childMode = qgetenv("PANEL_GSETTINGS_CHILD_MODE");
    if (childMode.isEmpty()) {
        return false;
    }

    const QByteArray schemaDir = qgetenv("PANEL_GSETTINGS_CHILD_SCHEMA_DIR");
    if (!schemaDir.isEmpty()) {
        qputenv("GSETTINGS_SCHEMA_DIR", schemaDir);
    }
    qputenv("GSETTINGS_BACKEND", "memory");

    QScopedPointer<UkuiPanel::PanelGSettings> settings(new UkuiPanel::PanelGSettings());
    bool success = false;

    if (childMode == "schema_without_key") {
        success = settings
                  && settings->m_qgSettings != nullptr
                  && !settings->isKeysContain("panellength")
                  && settings->getPanelLength(QStringLiteral("HDMI-9")) == -1;
        if (success) {
            settings->setPanelLength(QStringLiteral("HDMI-9"), 900);
            success = settings->getPanelLength(QStringLiteral("HDMI-9")) == -1;
        }
    }

    fprintf(stdout, "%s\n", success ? "ok" : "fail");
    fflush(stdout);
    ::exit(success ? 0 : 1);
    return true;
}

QString runPanelGSettingsChildProcess(const QByteArray &mode, const QString &schemaDir)
{
    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("HOME"), QStringLiteral("/tmp"));
    environment.insert(QStringLiteral("XDG_RUNTIME_DIR"), QStringLiteral("/tmp"));
    environment.insert(QStringLiteral("GSETTINGS_BACKEND"), QStringLiteral("memory"));
    environment.insert(QStringLiteral("PANEL_GSETTINGS_CHILD_MODE"), QString::fromLocal8Bit(mode));
    environment.insert(QStringLiteral("PANEL_GSETTINGS_CHILD_SCHEMA_DIR"), schemaDir);
    process.setProcessEnvironment(environment);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setProgram(QCoreApplication::applicationFilePath());

    process.start();
    if (!process.waitForStarted()) {
        return {};
    }
    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished();
        return {};
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }

    return QString::fromLocal8Bit(process.readAll()).trimmed();
}

} // namespace

void PanelGSettingsTest::initTestCase()
{
    if (runPanelGSettingsChildMode()) {
        return;
    }

    qputenv("GSETTINGS_SCHEMA_DIR", QByteArray(PANEL_GSETTINGS_TEST_SCHEMA_DIR));
    qputenv("GSETTINGS_BACKEND", "memory");

    QVERIFY(QGSettings::isSchemaInstalled(kPanelSchemaId));
    resetPanelSettings();
}

void PanelGSettingsTest::cleanup()
{
    resetPanelSettings();
}

void PanelGSettingsTest::testInstanceAndMetaObject()
{
    auto *instance = UkuiPanel::PanelGSettings::instance();

    QVERIFY(instance != nullptr);
    QCOMPARE(UkuiPanel::PanelGSettings::instance(), instance);
    QCOMPARE(QString::fromLatin1(instance->metaObject()->className()), QStringLiteral("UkuiPanel::PanelGSettings"));
    QVERIFY(instance->qt_metacast("UkuiPanel::PanelGSettings") != nullptr);
    QVERIFY(instance->qt_metacast("QObject") != nullptr);
    QVERIFY(instance->qt_metacast("NotARealType") == nullptr);
}

void PanelGSettingsTest::testConstructsQGSettingsWrapper()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();

    QVERIFY(settings != nullptr);
    QVERIFY(settings->m_qgSettings != nullptr);
    QVERIFY(settings->m_qgSettings->keys().contains(QStringLiteral("paneltype")));
    QVERIFY(settings->m_qgSettings->keys().contains(QStringLiteral("topbarsize")));
}

void PanelGSettingsTest::testPanelLengthRoundTrip()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();
    QVERIFY(settings != nullptr);

    QCOMPARE(settings->getPanelLength(QStringLiteral("HDMI-1")), -1);

    settings->setPanelLength(QStringLiteral("HDMI-1"), 640);
    QCOMPARE(settings->getPanelLength(QStringLiteral("HDMI-1")), 640);
    QCOMPARE(settings->getPanelLength(QStringLiteral("DP-1")), -1);

    settings->setPanelLength(QStringLiteral("DP-1"), 512);
    QCOMPARE(settings->getPanelLength(QStringLiteral("HDMI-1")), 640);
    QCOMPARE(settings->getPanelLength(QStringLiteral("DP-1")), 512);

    settings->setPanelLength(QStringLiteral("HDMI-1"), 720);
    QCOMPARE(settings->getPanelLength(QStringLiteral("HDMI-1")), 720);
}

void PanelGSettingsTest::testSetPanelTypeWritesValue()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();
    QScopedPointer<QGSettings> reader(createReader());
    QVERIFY(settings != nullptr);

    settings->setPanelType(1);
    QCOMPARE(reader->get(QStringLiteral("paneltype")).toInt(), 1);

    // 测试无效GSettings场景（m_qgSettings为空）
    auto *originalQGSettings = settings->m_qgSettings;
    settings->m_qgSettings = nullptr;
    settings->setPanelType(2); // 无崩溃，且不执行设置
    settings->m_qgSettings = originalQGSettings;
}

void PanelGSettingsTest::testSetIslandPositionRoutesKeys()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();
    QScopedPointer<QGSettings> reader(createReader());
    QVERIFY(settings != nullptr);

    settings->setIslandPosition(QStringLiteral("org.ukui.dataIsland"), 1);
    QCOMPARE(reader->get(QStringLiteral("dataislandposition")).toInt(), 1);
    QCOMPARE(reader->get(QStringLiteral("settingsislandposition")).toInt(), 0);

    settings->setIslandPosition(QStringLiteral("org.ukui.settingsIsland"), 1);
    QCOMPARE(reader->get(QStringLiteral("settingsislandposition")).toInt(), 1);

    settings->setIslandPosition(QStringLiteral("org.ukui.unknownIsland"), 0);
    QCOMPARE(reader->get(QStringLiteral("dataislandposition")).toInt(), 1);
    QCOMPARE(reader->get(QStringLiteral("settingsislandposition")).toInt(), 1);

    // 测试m_qgSettings为空的场景
    auto *originalQGSettings = settings->m_qgSettings;
    settings->m_qgSettings = nullptr;
    settings->setIslandPosition(QStringLiteral("org.ukui.dataIsland"), 2); // 无崩溃
    settings->m_qgSettings = originalQGSettings;
}

void PanelGSettingsTest::testSetTopBarSizeWritesValue()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();
    QScopedPointer<QGSettings> reader(createReader());
    QVERIFY(settings != nullptr);

    settings->setTopBarSize(48);
    QCOMPARE(reader->get(QStringLiteral("topbarsize")).toInt(), 48);

    // 测试无效GSettings场景
    auto *originalQGSettings = settings->m_qgSettings;
    settings->m_qgSettings = nullptr;
    settings->setTopBarSize(99); // 无崩溃
    settings->m_qgSettings = originalQGSettings;
}

// 新增：测试isKeysContain边界场景
void PanelGSettingsTest::testIsKeysContain()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();
    QVERIFY(settings != nullptr);

    // 正常场景
    QVERIFY(settings->isKeysContain("panellength"));
    QVERIFY(settings->isKeysContain("paneltype"));
    QVERIFY(!settings->isKeysContain("invalid-key"));
}

// 新增：测试getPanelLengthMap空数据/非uint32类型
void PanelGSettingsTest::testGetPanelLengthMap()
{
    auto *settings = UkuiPanel::PanelGSettings::instance();
    QVERIFY(settings != nullptr);
    QVERIFY(settings->m_qgSettings != nullptr);

    // 清空panellength数据
    GSettings *settingsHandle = g_settings_new_with_path(kPanelSchemaId, kPanelSettingsPath);
    QVERIFY(settingsHandle != nullptr);
    GVariant *empty = g_variant_new_array(G_VARIANT_TYPE("{sv}"), NULL, 0);
    g_settings_set_value(settingsHandle, "panellength", empty);

    QMap<QString, QVariant> emptyMap = settings->getPanelLengthMap();
    QVERIFY(emptyMap.isEmpty());

    // 插入非uint32类型值（测试类型过滤）
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "DP-2", g_variant_new_int32(100)); // int32（非uint32）
    GVariant* nonUint32 = g_variant_builder_end(&builder);
    g_settings_set_value(settingsHandle, "panellength", nonUint32);

    QMap<QString, QVariant> nonUintMap = settings->getPanelLengthMap();
    QVERIFY(nonUintMap.isEmpty()); // 非uint32值被过滤

    // 恢复正常数据
    settings->setPanelLength("HDMI-2", 800);
    QMap<QString, QVariant> normalMap = settings->getPanelLengthMap();
    QCOMPARE(normalMap.value("HDMI-2").toInt(), 800);

    g_object_unref(settingsHandle);
}

// 新增：测试单例线程安全（简化版）
void PanelGSettingsTest::testInstanceThreadSafety()
{
    QThread thread;
    QScopedPointer<UkuiPanel::PanelGSettings*> threadInstance;

    QObject::connect(&thread, &QThread::started, [&]() {
        threadInstance.reset(new UkuiPanel::PanelGSettings*(UkuiPanel::PanelGSettings::instance()));
        thread.quit();
    });

    thread.start();
    thread.wait();

    QVERIFY(*threadInstance.data() == UkuiPanel::PanelGSettings::instance());
}

void PanelGSettingsTest::testSchemaWithoutPanelLengthKey()
{
    const QString schemaDir = createSchemaDirWithoutPanelLength();
    QVERIFY(!schemaDir.isEmpty());
    const QString output = runPanelGSettingsChildProcess("schema_without_key", schemaDir);
    QVERIFY2(output.contains(QStringLiteral("ok")), qPrintable(output));
}

// 新增：测试析构函数
void PanelGSettingsTest::testDestructor()
{
    // 该用例必须放在最后，避免析构共享静态 GSettings 后影响后续用例。
    UkuiPanel::PanelGSettings* manualInstance = new UkuiPanel::PanelGSettings();
    QVERIFY(manualInstance != nullptr);
    delete manualInstance;
    QVERIFY(true);
}

QTEST_MAIN(PanelGSettingsTest)
