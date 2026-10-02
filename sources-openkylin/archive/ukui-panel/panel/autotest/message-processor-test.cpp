#include "message-processor-test.h"

#include <QFile>
#include <QElapsedTimer>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTest>

#define private public
#define protected public
#include "message-processor.h"
#include "general-config-define.h"
#include "rep_remote-config_replica.h"
#include "rep_remote-config_source.h"
#undef protected
#undef private

#include <unistd.h>

namespace MessageProcessorTestSupport {

struct ReplicaState
{
    GeneralConfigDefine::MergeIcons mergeIcons = GeneralConfigDefine::Always;
    GeneralConfigDefine::PanelLocation panelLocation = GeneralConfigDefine::Left;
    GeneralConfigDefine::PanelSizePolicy panelSizePolicy = GeneralConfigDefine::Large;
    bool allowCustomSize = true;
    bool panelAutoHide = false;
    bool panelLock = false;
    QStringList panelWidgets{QStringLiteral("launcher"), QStringLiteral("taskbar")};
    QStringList disabledWidgets{QStringLiteral("clock"), QStringLiteral("menu")};
    QStringList trayIconsInhibited{QStringLiteral("nm"), QStringLiteral("bluetooth")};
    QStringList trayIconsCanInhibited{QStringLiteral("nm"), QStringLiteral("bluetooth"), QStringLiteral("power")};
    GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn = GeneralConfigDefine::PanelWhereWindowIsOpen;
    GeneralConfigDefine::PanelType panelType = GeneralConfigDefine::Version4;
    bool showPanelOnAllScreens = true;
    bool showSystemTrayOnAllPanels = false;
    GeneralConfigDefine::PanelLocation panelLocationNext = GeneralConfigDefine::Top;
    GeneralConfigDefine::PanelSizePolicy panelSizePolicyNext = GeneralConfigDefine::Small;
    bool panelAutoHideNext = true;
    bool panelLockNext = true;
    QStringList panelWidgetsNext{QStringLiteral("menu"), QStringLiteral("status")};
    QStringList disabledWidgetsNext{QStringLiteral("menu"), QStringLiteral("status")};
};

} // namespace MessageProcessorTestSupport

Q_DECLARE_METATYPE(MessageProcessorTestSupport::ReplicaState)

using MessageProcessorTestSupport::ReplicaState;

namespace {

ReplicaState defaultReplicaState(GeneralConfigDefine::PanelType panelType)
{
    ReplicaState state;
    state.panelType = panelType;
    return state;
}

class TestRemoteConfigSource final : public RemoteConfigSimpleSource
{
public:
    using RemoteConfigSimpleSource::RemoteConfigSimpleSource;

    void setAllowCustomSizeForTest(bool allowCustomSize)
    {
        setAllowCustomSize(allowCustomSize);
    }

    void setDisabledWidgetsForTest(const QStringList &disabledWidgets)
    {
        setDisabledWidgets(disabledWidgets);
    }

    void setTrayIconsCanInhibitedForTest(const QStringList &trayIconsCanInhibited)
    {
        setTrayIconsCanInhibited(trayIconsCanInhibited);
    }

    void setDisabledWidgetsNextForTest(const QStringList &disabledWidgetsNext)
    {
        setDisabledWidgetsNext(disabledWidgetsNext);
    }

    void disableWidget(const QString &id, bool disable) override
    {
        auto widgets = disabledWidgets();
        if (disable) {
            if (!widgets.contains(id)) {
                widgets.append(id);
            }
        } else {
            widgets.removeAll(id);
        }

        setDisabledWidgets(widgets);
    }

    void disableWidgetNext(const QString &id, bool disable) override
    {
        auto widgets = disabledWidgetsNext();
        if (disable) {
            if (!widgets.contains(id)) {
                widgets.append(id);
            }
        } else {
            widgets.removeAll(id);
        }

        setDisabledWidgetsNext(widgets);
    }
};

QVariantList replicaProperties(const ReplicaState &state)
{
    QVariantList properties;
    properties.reserve(20);
    properties << QVariant::fromValue(state.mergeIcons)
               << QVariant::fromValue(state.panelLocation)
               << QVariant::fromValue(state.panelSizePolicy)
               << QVariant::fromValue(state.allowCustomSize)
               << QVariant::fromValue(state.panelAutoHide)
               << QVariant::fromValue(state.panelLock)
               << QVariant::fromValue(state.panelWidgets)
               << QVariant::fromValue(state.disabledWidgets)
               << QVariant::fromValue(state.trayIconsInhibited)
               << QVariant::fromValue(state.trayIconsCanInhibited)
               << QVariant::fromValue(state.taskBarIconsShowedOn)
               << QVariant::fromValue(state.panelType)
               << QVariant::fromValue(state.showPanelOnAllScreens)
               << QVariant::fromValue(state.showSystemTrayOnAllPanels)
               << QVariant::fromValue(state.panelLocationNext)
               << QVariant::fromValue(state.panelSizePolicyNext)
               << QVariant::fromValue(state.panelAutoHideNext)
               << QVariant::fromValue(state.panelLockNext)
               << QVariant::fromValue(state.panelWidgetsNext)
               << QVariant::fromValue(state.disabledWidgetsNext);
    return properties;
}

QString runProcessorAndCaptureOutput(const QStringList &arguments,
                                     const ReplicaState &state)
{
    qputenv("USER", "autotest-user");
    qputenv("XDG_SESSION_TYPE", "x11");
    qputenv("DISPLAY", "display77");
    qputenv("XDG_RUNTIME_DIR", "/tmp");

    QTemporaryFile outputFile;
    outputFile.setAutoRemove(true);
    if (!outputFile.open()) {
        return {};
    }

    fflush(stdout);
    const int stdoutFd = dup(STDOUT_FILENO);
    if (stdoutFd < 0) {
        return {};
    }
    dup2(outputFile.handle(), STDOUT_FILENO);

    QString output;
    {
        MessageProcessor processor;
        processor.preprocessMessage(arguments);
        QElapsedTimer timer;
        timer.start();
        while (processor.m_configReplica == nullptr && timer.elapsed() < 3000) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            QTest::qWait(10);
        }
        if (processor.m_configReplica == nullptr) {
            dup2(stdoutFd, STDOUT_FILENO);
            close(stdoutFd);
            return {};
        }

        processor.m_configReplica->setProperties(replicaProperties(state));
        QMetaObject::invokeMethod(processor.m_configReplica, "initialized", Qt::DirectConnection);

        QTest::qWait(50);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        fflush(stdout);
    }

    dup2(stdoutFd, STDOUT_FILENO);
    close(stdoutFd);

    outputFile.seek(0);
    output = QString::fromLocal8Bit(outputFile.readAll()).trimmed();
    return output;
}

QString runCliChildModeAndCaptureOutput(const QByteArray &mode)
{
    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("HOME"), QStringLiteral("/tmp"));
    environment.insert(QStringLiteral("XDG_RUNTIME_DIR"), QStringLiteral("/tmp"));
    environment.insert(QStringLiteral("USER"), QStringLiteral("autotest-user"));
    environment.insert(QStringLiteral("MESSAGE_PROCESSOR_CHILD_MODE"), QString::fromLocal8Bit(mode));
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

    return QString::fromLocal8Bit(process.readAll()).trimmed();
}

} // namespace

void MessageProcessorTest::initTestCase()
{
    qRegisterMetaType<GeneralConfigDefine::PanelType>("GeneralConfigDefine::PanelType");
    qRegisterMetaType<ReplicaState>("ReplicaState");

    const QByteArray childMode = qgetenv("MESSAGE_PROCESSOR_CHILD_MODE");
    if (!childMode.isEmpty()) {
        MessageProcessor processor;
        QStringList arguments{QCoreApplication::applicationFilePath()};

        if (childMode == "help-option") {
            arguments << QStringLiteral("--help");
        } else if (childMode == "unknown-option") {
            arguments << QStringLiteral("--not-a-real-option");
        } else if (childMode == "version-option") {
            QCoreApplication::setApplicationVersion(QStringLiteral("1.0-test"));
            arguments << QStringLiteral("--version");
        }

        processor.preprocessMessage(arguments);
        qApp->exit(0);
    }
}

void MessageProcessorTest::testNodeUrlUsesDisplay_data()
{
    QTest::addColumn<QByteArray>("sessionType");
    QTest::addColumn<QByteArray>("displayValue");
    QTest::addColumn<QString>("expected");
    const QString uid = QString::number(getuid());

    QTest::newRow("x11")
            << QByteArray("x11")
            << QByteArray(":88")
            << QStringLiteral("local:ukui-panel-config-autotest-user-") + uid + QStringLiteral("-:88");
    QTest::newRow("wayland")
            << QByteArray("wayland")
            << QByteArray("wayland-1")
            << QStringLiteral("local:ukui-panel-config-autotest-user-") + uid + QStringLiteral("-wayland-1");
}

void MessageProcessorTest::testNodeUrlUsesDisplay()
{
    QFETCH(QByteArray, sessionType);
    QFETCH(QByteArray, displayValue);
    QFETCH(QString, expected);

    qputenv("USER", "autotest-user");
    qputenv("XDG_SESSION_TYPE", sessionType);
    if (sessionType == "wayland") {
        qputenv("WAYLAND_DISPLAY", displayValue);
        qunsetenv("DISPLAY");
    } else {
        qputenv("DISPLAY", displayValue);
        qunsetenv("WAYLAND_DISPLAY");
    }

    QCOMPARE(MessageProcessor::nodeUrl().toString(), expected);
}

void MessageProcessorTest::testMessageProcessorMetaObject()
{
    MessageProcessor processor;

    QCOMPARE(QString::fromLatin1(processor.metaObject()->className()), QStringLiteral("MessageProcessor"));
    QVERIFY(processor.qt_metacast("MessageProcessor") != nullptr);
    QVERIFY(processor.qt_metacast("QObject") != nullptr);
    QVERIFY(processor.qt_metacast("NotARealType") == nullptr);
}

void MessageProcessorTest::testCliHelperHelpAndVersion_data()
{
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<QString>("expectedText");

    QTest::newRow("no-arguments")
            << QStringList()
            << QStringLiteral("--panelLocation");
    QTest::newRow("help-option")
            << QStringList{QStringLiteral("--help")}
            << QStringLiteral("--showPanelOnAllScreens");
    QTest::newRow("unknown-option")
            << QStringList{QStringLiteral("--not-a-real-option")}
            << QStringLiteral("--panelType");
    QTest::newRow("version-option")
            << QStringList{QStringLiteral("--version")}
            << QStringLiteral("1.0-test");
}

void MessageProcessorTest::testCliHelperHelpAndVersion()
{
    QFETCH(QStringList, arguments);
    QFETCH(QString, expectedText);

    QByteArray mode;
    if (arguments.isEmpty()) {
        mode = "no-arguments";
    } else if (arguments == QStringList{QStringLiteral("--help")}) {
        mode = "help-option";
    } else if (arguments == QStringList{QStringLiteral("--not-a-real-option")}) {
        mode = "unknown-option";
    } else if (arguments == QStringList{QStringLiteral("--version")}) {
        mode = "version-option";
    }

    QVERIFY(!mode.isEmpty());

    const QString output = runCliChildModeAndCaptureOutput(mode);
    QVERIFY2(!output.isEmpty(), "child process did not produce output");
    QVERIFY2(output.contains(expectedText), qPrintable(output));
}

void MessageProcessorTest::testRemoteConfigReplicaGeneratedCode()
{
    RemoteConfigReplica replica;
    const auto properties = replicaProperties(defaultReplicaState(GeneralConfigDefine::Version5));
    replica.setProperties(properties);

    QCOMPARE(QString::fromLatin1(replica.metaObject()->className()), QStringLiteral("RemoteConfigReplica"));
    QVERIFY(replica.qt_metacast("RemoteConfigReplica") != nullptr);
    QVERIFY(replica.qt_metacast("QRemoteObjectReplica") != nullptr);
    QVERIFY(replica.qt_metacast("NotARealReplica") == nullptr);

    QCOMPARE(replica.mergeIcons(), GeneralConfigDefine::Always);
    QCOMPARE(replica.panelLocation(), GeneralConfigDefine::Left);
    QCOMPARE(replica.panelSizePolicy(), GeneralConfigDefine::Large);
    QCOMPARE(replica.allowCustomSize(), true);
    QCOMPARE(replica.panelAutoHide(), false);
    QCOMPARE(replica.panelLock(), false);
    QCOMPARE(replica.panelWidgets(), QStringList({QStringLiteral("launcher"), QStringLiteral("taskbar")}));
    QCOMPARE(replica.disabledWidgets(), QStringList({QStringLiteral("clock"), QStringLiteral("menu")}));
    QCOMPARE(replica.trayIconsInhibited(), QStringList({QStringLiteral("nm"), QStringLiteral("bluetooth")}));
    QCOMPARE(replica.trayIconsCanInhibited(), QStringList({QStringLiteral("nm"), QStringLiteral("bluetooth"), QStringLiteral("power")}));
    QCOMPARE(replica.taskBarIconsShowedOn(), GeneralConfigDefine::PanelWhereWindowIsOpen);
    QCOMPARE(replica.panelType(), GeneralConfigDefine::Version5);
    QCOMPARE(replica.showPanelOnAllScreens(), true);
    QCOMPARE(replica.showSystemTrayOnAllPanels(), false);
    QCOMPARE(replica.panelLocationNext(), GeneralConfigDefine::Top);
    QCOMPARE(replica.panelSizePolicyNext(), GeneralConfigDefine::Small);
    QCOMPARE(replica.panelAutoHideNext(), true);
    QCOMPARE(replica.panelLockNext(), true);
    QCOMPARE(replica.panelWidgetsNext(), QStringList({QStringLiteral("menu"), QStringLiteral("status")}));
    QCOMPARE(replica.disabledWidgetsNext(), QStringList({QStringLiteral("menu"), QStringLiteral("status")}));

    replica.setMergeIcons(GeneralConfigDefine::Never);
    replica.setPanelLocation(GeneralConfigDefine::Right);
    replica.setPanelSizePolicy(GeneralConfigDefine::Medium);
    replica.setPanelAutoHide(true);
    replica.setPanelLock(true);
    replica.setPanelWidgets(QStringList{QStringLiteral("clock")});
    replica.setTrayIconsInhibited(QStringList{QStringLiteral("power")});
    replica.setTaskBarIconsShowedOn(GeneralConfigDefine::AllPanels);
    replica.setPanelType(GeneralConfigDefine::Version4);
    replica.setShowPanelOnAllScreens(false);
    replica.setShowSystemTrayOnAllPanels(true);
    replica.setPanelLocationNext(GeneralConfigDefine::Bottom);
    replica.setPanelSizePolicyNext(GeneralConfigDefine::Custom);
    replica.setPanelAutoHideNext(false);
    replica.setPanelLockNext(false);
    replica.setPanelWidgetsNext(QStringList{QStringLiteral("menu")});
    replica.disableWidget(QStringLiteral("clock"), true);
    replica.disableWidgetNext(QStringLiteral("menu"), false);

    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "mergeIconsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::MergeIcons, GeneralConfigDefine::Always)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelLocationChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelLocation, GeneralConfigDefine::Left)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelSizePolicyChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelSizePolicy, GeneralConfigDefine::Large)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "allowCustomSizeChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelAutoHideChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelLockChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelWidgetsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("launcher")})));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "disabledWidgetsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("clock")})));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "trayIconsInhibitedChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("bluetooth")})));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "trayIconsCanInhibitedChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("power")})));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "taskBarIconsShowedOnChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::TaskBarIconsShowedOn, GeneralConfigDefine::AllPanels)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelTypeChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelType, GeneralConfigDefine::Version5)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "showPanelOnAllScreensChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "showSystemTrayOnAllPanelsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelLocationNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelLocation, GeneralConfigDefine::Top)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelSizePolicyNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelSizePolicy, GeneralConfigDefine::Small)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelAutoHideNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelLockNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "panelWidgetsNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("status")})));
    QVERIFY(QMetaObject::invokeMethod(&replica,
                                      "disabledWidgetsNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("menu")})));
}

void MessageProcessorTest::testRemoteConfigSourceGeneratedCode()
{
    TestRemoteConfigSource source;
    QSignalSpy disabledWidgetsSpy(&source, SIGNAL(disabledWidgetsChanged(QStringList)));
    QSignalSpy disabledWidgetsNextSpy(&source, SIGNAL(disabledWidgetsNextChanged(QStringList)));

    QCOMPARE(QString::fromLatin1(source.metaObject()->className()), QStringLiteral("RemoteConfigSimpleSource"));
    QVERIFY(source.qt_metacast("RemoteConfigSource") != nullptr);
    QVERIFY(source.qt_metacast("QObject") != nullptr);
    QVERIFY(source.qt_metacast("NotARealSource") == nullptr);

    source.setMergeIcons(GeneralConfigDefine::Always);
    source.setPanelLocation(GeneralConfigDefine::Left);
    source.setPanelSizePolicy(GeneralConfigDefine::Large);
    source.setAllowCustomSizeForTest(true);
    source.setPanelAutoHide(true);
    source.setPanelLock(true);
    source.setPanelWidgets(QStringList{QStringLiteral("launcher"), QStringLiteral("taskbar")});
    source.setDisabledWidgetsForTest(QStringList{QStringLiteral("clock")});
    source.setTrayIconsInhibited(QStringList{QStringLiteral("nm")});
    source.setTrayIconsCanInhibitedForTest(QStringList{QStringLiteral("power")});
    source.setTaskBarIconsShowedOn(GeneralConfigDefine::AllPanels);
    source.setPanelType(GeneralConfigDefine::Version5);
    source.setShowPanelOnAllScreens(true);
    source.setShowSystemTrayOnAllPanels(true);
    source.setPanelLocationNext(GeneralConfigDefine::Top);
    source.setPanelSizePolicyNext(GeneralConfigDefine::Small);
    source.setPanelAutoHideNext(true);
    source.setPanelLockNext(true);
    source.setPanelWidgetsNext(QStringList{QStringLiteral("menu")});
    source.setDisabledWidgetsNextForTest(QStringList{QStringLiteral("status")});

    QCOMPARE(source.mergeIcons(), GeneralConfigDefine::Always);
    QCOMPARE(source.panelLocation(), GeneralConfigDefine::Left);
    QCOMPARE(source.panelSizePolicy(), GeneralConfigDefine::Large);
    QCOMPARE(source.allowCustomSize(), true);
    QCOMPARE(source.panelAutoHide(), true);
    QCOMPARE(source.panelLock(), true);
    QCOMPARE(source.panelWidgets(), QStringList({QStringLiteral("launcher"), QStringLiteral("taskbar")}));
    QCOMPARE(source.disabledWidgets(), QStringList({QStringLiteral("clock")}));
    QCOMPARE(source.trayIconsInhibited(), QStringList({QStringLiteral("nm")}));
    QCOMPARE(source.trayIconsCanInhibited(), QStringList({QStringLiteral("power")}));
    QCOMPARE(source.taskBarIconsShowedOn(), GeneralConfigDefine::AllPanels);
    QCOMPARE(source.panelType(), GeneralConfigDefine::Version5);
    QCOMPARE(source.showPanelOnAllScreens(), true);
    QCOMPARE(source.showSystemTrayOnAllPanels(), true);
    QCOMPARE(source.panelLocationNext(), GeneralConfigDefine::Top);
    QCOMPARE(source.panelSizePolicyNext(), GeneralConfigDefine::Small);
    QCOMPARE(source.panelAutoHideNext(), true);
    QCOMPARE(source.panelLockNext(), true);
    QCOMPARE(source.panelWidgetsNext(), QStringList({QStringLiteral("menu")}));
    QCOMPARE(source.disabledWidgetsNext(), QStringList({QStringLiteral("status")}));

    source.disableWidget(QStringLiteral("tray"), true);
    source.disableWidget(QStringLiteral("tray"), true);
    source.disableWidget(QStringLiteral("clock"), false);
    source.disableWidgetNext(QStringLiteral("menu"), true);
    source.disableWidgetNext(QStringLiteral("status"), false);

    QVERIFY(source.disabledWidgets().contains(QStringLiteral("tray")));
    QVERIFY(!source.disabledWidgets().contains(QStringLiteral("clock")));
    QVERIFY(source.disabledWidgetsNext().contains(QStringLiteral("menu")));
    QVERIFY(!source.disabledWidgetsNext().contains(QStringLiteral("status")));
    QVERIFY(disabledWidgetsSpy.count() >= 2);
    QVERIFY(disabledWidgetsNextSpy.count() >= 2);

    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "mergeIconsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::MergeIcons, GeneralConfigDefine::Never)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelLocationChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelLocation, GeneralConfigDefine::Right)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelSizePolicyChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelSizePolicy, GeneralConfigDefine::Medium)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "allowCustomSizeChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelAutoHideChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelLockChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelWidgetsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("launcher")})));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "disabledWidgetsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("clock")})));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "trayIconsInhibitedChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("nm")})));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "trayIconsCanInhibitedChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("power")})));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "taskBarIconsShowedOnChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::TaskBarIconsShowedOn, GeneralConfigDefine::PanelWhereWindowIsOpen)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelTypeChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelType, GeneralConfigDefine::Version4)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "showPanelOnAllScreensChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "showSystemTrayOnAllPanelsChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelLocationNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelLocation, GeneralConfigDefine::Bottom)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelSizePolicyNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(GeneralConfigDefine::PanelSizePolicy, GeneralConfigDefine::Custom)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelAutoHideNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelLockNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(bool, false)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "panelWidgetsNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("status")})));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "disabledWidgetsNextChanged",
                                      Qt::DirectConnection,
                                      Q_ARG(QStringList, QStringList{QStringLiteral("menu")})));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "disableWidget",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QStringLiteral("launcher")),
                                      Q_ARG(bool, true)));
    QVERIFY(QMetaObject::invokeMethod(&source,
                                      "disableWidgetNext",
                                      Qt::DirectConnection,
                                      Q_ARG(QString, QStringLiteral("taskbar")),
                                      Q_ARG(bool, true)));
}

void MessageProcessorTest::testPreprocessMessageOutputsExpectedValue_data()
{
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<ReplicaState>("state");
    QTest::addColumn<QString>("expected");

    const auto version4State = defaultReplicaState(GeneralConfigDefine::Version4);
    const auto version5State = defaultReplicaState(GeneralConfigDefine::Version5);

    auto version4ShowAllFalse = version4State;
    version4ShowAllFalse.showPanelOnAllScreens = false;

    auto version5ShowTrayTrue = version5State;
    version5ShowTrayTrue.showSystemTrayOnAllPanels = true;

    QTest::newRow("panelLocation-v4")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelLocation")}
            << version4State
            << QStringLiteral("Left");
    QTest::newRow("panelLocation-v5")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelLocation")}
            << version5State
            << QStringLiteral("Top");
    QTest::newRow("panelSizePolicy-v4")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelSizePolicy")}
            << version4State
            << QStringLiteral("Large");
    QTest::newRow("panelSizePolicy-v5")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelSizePolicy")}
            << version5State
            << QStringLiteral("Small");
    QTest::newRow("panelAutoHide-v4")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelAutoHide")}
            << version4State
            << QStringLiteral("false");
    QTest::newRow("panelAutoHide-v5")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelAutoHide")}
            << version5State
            << QStringLiteral("true");
    QTest::newRow("panelLock-v4")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelLock")}
            << version4State
            << QStringLiteral("false");
    QTest::newRow("panelLock-v5")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelLock")}
            << version5State
            << QStringLiteral("true");
    QTest::newRow("disabledWidgets-v4")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--disabledWidgets")}
            << version4State
            << QStringLiteral("clock,menu");
    QTest::newRow("disabledWidgets-v5")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--disabledWidgets")}
            << version5State
            << QStringLiteral("menu,status");
    QTest::newRow("trayIconsInhibited")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--trayIconsInhibited")}
            << version4State
            << QStringLiteral("nm,bluetooth");
    QTest::newRow("taskBarIconsShowedOn")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--taskBarIconsShowedOn")}
            << version4State
            << QStringLiteral("PanelWhereWindowIsOpen");
    QTest::newRow("panelType")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--panelType")}
            << version5State
            << QStringLiteral("Version5");
    QTest::newRow("showPanelOnAllScreens-true")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--showPanelOnAllScreens")}
            << version4State
            << QStringLiteral("true");
    QTest::newRow("showPanelOnAllScreens-false")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--showPanelOnAllScreens")}
            << version4ShowAllFalse
            << QStringLiteral("false");
    QTest::newRow("showSystemTrayOnAllPanels-false")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--showSystemTrayOnAllPanels")}
            << version4State
            << QStringLiteral("false");
    QTest::newRow("showSystemTrayOnAllPanels-true")
            << QStringList{QStringLiteral("ukui-panel"), QStringLiteral("--showSystemTrayOnAllPanels")}
            << version5ShowTrayTrue
            << QStringLiteral("true");
}

void MessageProcessorTest::testPreprocessMessageOutputsExpectedValue()
{
    QFETCH(QStringList, arguments);
    QFETCH(ReplicaState, state);
    QFETCH(QString, expected);

    const auto output = runProcessorAndCaptureOutput(arguments, state);

    QCOMPARE(output, expected);
}

QTEST_MAIN(MessageProcessorTest)
