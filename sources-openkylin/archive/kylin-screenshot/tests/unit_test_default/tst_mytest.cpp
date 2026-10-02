/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include <QtTest/QTest>
#include <QStringList>
#include <QSignalSpy>
#include <QThread>
#include <memory>
#include <QBuffer>
//#include <QPrintDialog>
#include <QWidget>
#include "src/core/controller.h"
#include "src/core/flameshotdbusadapter.h"
#include "src/utils/dbusutils.h"
#include "qstandardpaths.h"
#include "src/widgets/capture/buttonhandler.h"
#include "src/utils/confighandler.h"
#include "src/widgets/capture/capturewidget.h"
//#include "CommandLineOptions.h"
//#include "Logger.h"
#include "src/tools/text/textconfig.h"
#include "src/tools/text/textwidget.h"
#include "src/tools/text/texttool.h"
#include "src/tools/options/options.h"
#include "src/tools/scroll/scroll_tool.h"
#include "src/tools/ocr/ocrtool.h"
class MyTest : public QObject
{
    Q_OBJECT
public:
    MyTest();
    ~MyTest();

private Q_SLOTS:
    static void initTestCase()
    {
        int argc = 0;
        char *argv[] = {};
        m_app = std::make_unique<QApplication>(argc, argv);
//        if (m_dbus == nullptr) {
//            m_dbus = new Dbus;
//        }
    }
    static void cleanupTestCase()
    {
        m_app.reset();
//        if (m_coreInteraction) {
//            delete m_coreInteraction;
//            m_coreInteraction = nullptr;
//        }
    }

    void tst_getInstance();
    void tst_getDBusAdapter();
    void tst_openConfigWindow();
    //void tst_openShotCutWindow();
    //void tst_openInfoWindow();
    void tst_openLauncherWindow();
    void tst_enableTrayIcon();
    void tst_disableTrayIcon();

    void tst_fullScreen();
    void tst_screenGrab();
    void tst_visualCapture();
    void tst_buttonHandler();
    void tst_captureFailed();
    void tst_captureExit();
    void tst_captureCopy();

    void tst_commandLineParser();
    void tst_commandLineParserOption();

    void tst_textconfig();
    void tst_options();
    void tst_scrollTool();
    void tst_ocrTool();
private:
    static std::unique_ptr<QApplication> m_app;
    static Controller *m_controller;
    static FlameshotDBusAdapter *m_dbusAdapter;
    static CaptureWidget *m_captureWidget;
};

std::unique_ptr<QApplication> MyTest::m_app;
Controller *MyTest::m_controller = nullptr;
FlameshotDBusAdapter *MyTest::m_dbusAdapter = nullptr;
CaptureWidget *MyTest::m_captureWidget = nullptr;

MyTest::MyTest()
{
}

MyTest::~MyTest()
{
}

void MyTest::tst_getInstance()
{
    if (m_controller == nullptr) {
       m_controller = Controller::getInstance();
    }
    m_controller->emitSCreenSaveLock();
    m_controller->emitSCreenSaveUnLock();
    m_controller->getScreenSaveLockStatus();
    m_controller->setScreenSaveLockStatus(false);
    QVERIFY(m_controller != nullptr);
}

void MyTest::tst_getDBusAdapter()
{
    if (m_dbusAdapter == nullptr) {
       m_dbusAdapter = new FlameshotDBusAdapter(m_controller);
    }
    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_openConfigWindow()
{
    m_controller->openConfigWindow();
//    QVERIFY(m_controller->m_configWindow);
    QVERIFY(m_dbusAdapter != nullptr);
}

/*void MyTest::tst_openShotCutWindow()
{
    m_controller->openShotCutWindow();
//    QVERIFY(m_controller->m_shortCutWindow);
    QVERIFY(m_dbusAdapter != nullptr);
}*/
/*
void MyTest::tst_openInfoWindow()
{
//    m_controller->openInfoWindow();
     m_controller->openShotCutWindow();
//    QVERIFY(m_controller->m_aboutdialog);
    QVERIFY(m_dbusAdapter != nullptr);
}
*/
void MyTest::tst_openLauncherWindow()
{
    m_controller->openLauncherWindow();
    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_commandLineParser()
{
    CommandLineParser parser;
    // Add description
    parser.setDescription(
                QStringLiteral("Powerful yet simple to use screenshot software."));
    parser.setGeneralErrorMessage(QStringLiteral("See 'kylin-screenshot --help'."));
    // Arguments
    CommandArgument fullArgument(QStringLiteral("full"),
                                 QStringLiteral("Capture the entire desktop."));
    CommandArgument launcherArgument(QStringLiteral("launcher"),
                                     QStringLiteral("Open the capture launcher."));
    CommandArgument guiArgument(QStringLiteral("gui"),
                                QStringLiteral("Start a manual capture in GUI mode."));
    CommandArgument configArgument(QStringLiteral("config"),
                                   QStringLiteral("Configure Kylin-Screenshot."));
    CommandArgument screenArgument(QStringLiteral("screen"),
                                   QStringLiteral("Capture a single screen."));
    screenArgument.setName(QStringLiteral("screen"));
    screenArgument.setDescription(QStringLiteral("Capture a single screen."));
    qDebug() << screenArgument.description();
    QVERIFY(!screenArgument.isRoot());

    // Options
    CommandOption pathOption(
    {"p", "path"},
                QStringLiteral("Path where the capture will be saved"),
                QStringLiteral("path"));
    CommandOption clipboardOption(
    {"c", "clipboard"},
                QStringLiteral("Save the capture to the clipboard"));
    CommandOption delayOption(
    {"d", "delay"},
                QStringLiteral("Delay time in milliseconds"),
                QStringLiteral("milliseconds"));
    CommandOption filenameOption(
    {"f", "filename"},
                QStringLiteral("Set the filename pattern"),
                QStringLiteral("pattern"));
    CommandOption trayOption(
    {"t", "trayicon"},
                QStringLiteral("Enable or disable the trayicon"),
                QStringLiteral("bool"));
    CommandOption autostartOption(
    {"a", "autostart"},
                QStringLiteral("Enable or disable run at startup"),
                QStringLiteral("bool"));
    CommandOption showHelpOption(
    {"s", "showhelp"},
                QStringLiteral("Show the help message in the capture mode"),
                QStringLiteral("bool"));
    CommandOption mainColorOption(
    {"m", "maincolor"},
                QStringLiteral("Define the main UI color"),
                QStringLiteral("color-code"));
    CommandOption contrastColorOption(
    {"k", "contrastcolor"},
                QStringLiteral("Define the contrast UI color"),
                QStringLiteral("color-code"));
    CommandOption rawImageOption(
    {"r", "raw"},
                QStringLiteral("Print raw PNG capture"));
    CommandOption screenNumberOption(
    {"n", "number"},
                QStringLiteral("Define the screen to capture,\ndefault: screen containing the cursor"),
                QStringLiteral("Screen number"), QStringLiteral("-1"));

    CommandOption TypeOption(
    {"T", "Type"},
                QStringLiteral("Type where the capture will be saved"),
                QStringLiteral("type"));
    CommandOption NewnameOption(
    {"N", "Newname"},
                QStringLiteral("Name where the capture will be saved"),
                QStringLiteral("name"));
    NewnameOption.setNames({"N", "Newname"});
    qDebug() << NewnameOption.names();

    CommandOption testOption("", "", "", "");
    testOption.setName("test");
    testOption.setDescription("testdesc");
    testOption.setValue("testValue");
    testOption.setValueName("testvaluename");
    qDebug() << testOption.value();
    qDebug() << testOption.valueName();
    qDebug() << testOption.description();
    qDebug() << testOption.names();
    qDebug() << testOption.errorMsg();
    testOption.checkValue(testOption.value());

    CommandOption testOption1("", "", "", "");
    testOption1 = testOption;

    // Add checkers
    auto colorChecker = [](const QString &colorCode) -> bool {
        QColor parsedColor(colorCode);
        return parsedColor.isValid() && parsedColor.alphaF() == 1.0;
    };
    QString colorErr = "Invalid color, "
                       "this flag supports the following formats:\n"
                       "- #RGB (each of R, G, and B is a single hex digit)\n"
                       "- #RRGGBB\n- #RRRGGGBBB\n"
                       "- #RRRRGGGGBBBB\n"
                       "- Named colors like 'blue' or 'red'\n"
                       "You may need to escape the '#' sign as in '\\#FFF'";

    const QString delayErr = QStringLiteral("Invalid delay, it must be higher than 0");
    const QString numberErr = QStringLiteral("Invalid screen number, it must be non negative");
    auto numericChecker = [](const QString &delayValue) -> bool {
        int value = delayValue.toInt();
        return value >= 0;
    };

    const QString pathErr = QStringLiteral("Invalid path, it must be a real path in the system");
    auto pathChecker = [pathErr](const QString &pathValue) -> bool {
        bool res = QDir(pathValue).exists();
        if (!res) {
//            SystemNotification().sendMessage(QObject::tr(
//                                                 pathErr.toLatin1().data()));
        }
//        return res;
    };

    const QString booleanErr = QStringLiteral(
                "Invalid value, it must be defined as 'true' or 'false'");
    auto booleanChecker = [](const QString &value) -> bool {
        return value == QLatin1String("true") || value == QLatin1String(
                    "false");
    };

    contrastColorOption.addChecker(colorChecker, colorErr);
    mainColorOption.addChecker(colorChecker, colorErr);
    delayOption.addChecker(numericChecker, delayErr);
    pathOption.addChecker(pathChecker, pathErr);
    trayOption.addChecker(booleanChecker, booleanErr);
    autostartOption.addChecker(booleanChecker, booleanErr);
    showHelpOption.addChecker(booleanChecker, booleanErr);
    screenNumberOption.addChecker(numericChecker, numberErr);

    // Relationships
    parser.AddArgument(guiArgument);
    parser.AddArgument(screenArgument);
    parser.AddArgument(fullArgument);
    parser.AddArgument(launcherArgument);
    parser.AddArgument(configArgument);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.AddOptions({ pathOption, delayOption, rawImageOption }, guiArgument);
    parser.AddOptions({ TypeOption, NewnameOption, screenNumberOption, clipboardOption, pathOption,
                        delayOption, rawImageOption },
                      screenArgument);
    parser.AddOptions({TypeOption, NewnameOption, pathOption, clipboardOption, delayOption,
                       rawImageOption },
                      fullArgument);
    parser.AddOptions({ autostartOption, filenameOption, trayOption,
                        showHelpOption, mainColorOption, contrastColorOption },
                      configArgument);

    QStringList args;
    args << "/home/panjinrui/音乐/baihe/dbug/kylin_screenshot/kylin-screenshot";
    args << "full";
    QVERIFY(parser.parse(args));
    QVERIFY(parser.isSet(fullArgument));
    QVERIFY(!parser.isSet(pathOption));
    qDebug() << parser.value(pathOption);
    parser.printVersion();

//    CommandLineOptions command_line_options;
//    command_line_options.Parse();
//    Logger logger;
//    logger.SetLogFile("/home/1.log");
//    logger.LogError("123");
//    logger.LogInfo("123");
//    logger.LogWarning("123");
//    qDebug() << optionsToString
//    qDebug() << parser.printHelp();
//    qDebug() << DefaultLogFile();
//    qDebug() << DefaultSettingsFile();
//    PrintOptionHelp();
//    qDebug() << DefaultStatsFile();
//    CheckOptionHasValue("", "");
//    CheckOptionHasNoValue("", "1");
}

void MyTest::tst_commandLineParserOption()
{
    CommandLineParser parser;
    // Add description
    parser.setDescription(
                QStringLiteral("Powerful yet simple to use screenshot software."));
    parser.setGeneralErrorMessage(QStringLiteral("See 'kylin-screenshot --help'."));
    // Arguments
    CommandArgument fullArgument(QStringLiteral("full"),
                                 QStringLiteral("Capture the entire desktop."));
    CommandArgument launcherArgument(QStringLiteral("launcher"),
                                     QStringLiteral("Open the capture launcher."));
    CommandArgument guiArgument(QStringLiteral("gui"),
                                QStringLiteral("Start a manual capture in GUI mode."));
    CommandArgument configArgument(QStringLiteral("config"),
                                   QStringLiteral("Configure Kylin-Screenshot."));
    CommandArgument screenArgument(QStringLiteral("screen"),
                                   QStringLiteral("Capture a single screen."));

    // Options
    CommandOption pathOption(
    {"p", "path"},
                QStringLiteral("Path where the capture will be saved"),
                QStringLiteral("path"));
    CommandOption clipboardOption(
    {"c", "clipboard"},
                QStringLiteral("Save the capture to the clipboard"));
    CommandOption delayOption(
    {"d", "delay"},
                QStringLiteral("Delay time in milliseconds"),
                QStringLiteral("milliseconds"));
    CommandOption filenameOption(
    {"f", "filename"},
                QStringLiteral("Set the filename pattern"),
                QStringLiteral("pattern"));
    CommandOption trayOption(
    {"t", "trayicon"},
                QStringLiteral("Enable or disable the trayicon"),
                QStringLiteral("bool"));
    CommandOption autostartOption(
    {"a", "autostart"},
                QStringLiteral("Enable or disable run at startup"),
                QStringLiteral("bool"));
    CommandOption showHelpOption(
    {"s", "showhelp"},
                QStringLiteral("Show the help message in the capture mode"),
                QStringLiteral("bool"));
    CommandOption mainColorOption(
    {"m", "maincolor"},
                QStringLiteral("Define the main UI color"),
                QStringLiteral("color-code"));
    CommandOption contrastColorOption(
    {"k", "contrastcolor"},
                QStringLiteral("Define the contrast UI color"),
                QStringLiteral("color-code"));
    CommandOption rawImageOption(
    {"r", "raw"},
                QStringLiteral("Print raw PNG capture"));
    CommandOption screenNumberOption(
    {"n", "number"},
                QStringLiteral("Define the screen to capture,\ndefault: screen containing the cursor"),
                QStringLiteral("Screen number"), QStringLiteral("-1"));

    CommandOption TypeOption(
    {"T", "Type"},
                QStringLiteral("Type where the capture will be saved"),
                QStringLiteral("type"));
    CommandOption NewnameOption(
    {"N", "Newname"},
                QStringLiteral("Name where the capture will be saved"),
                QStringLiteral("name"));

    // Add checkers
    auto colorChecker = [](const QString &colorCode) -> bool {
        QColor parsedColor(colorCode);
        return parsedColor.isValid() && parsedColor.alphaF() == 1.0;
    };
    QString colorErr = "Invalid color, "
                       "this flag supports the following formats:\n"
                       "- #RGB (each of R, G, and B is a single hex digit)\n"
                       "- #RRGGBB\n- #RRRGGGBBB\n"
                       "- #RRRRGGGGBBBB\n"
                       "- Named colors like 'blue' or 'red'\n"
                       "You may need to escape the '#' sign as in '\\#FFF'";

    const QString delayErr = QStringLiteral("Invalid delay, it must be higher than 0");
    const QString numberErr = QStringLiteral("Invalid screen number, it must be non negative");
    auto numericChecker = [](const QString &delayValue) -> bool {
        int value = delayValue.toInt();
        return value >= 0;
    };

    const QString pathErr = QStringLiteral("Invalid path, it must be a real path in the system");
    auto pathChecker = [pathErr](const QString &pathValue) -> bool {
        bool res = QDir(pathValue).exists();
        if (!res) {
//            SystemNotification().sendMessage(QObject::tr(
//                                                 pathErr.toLatin1().data()));
        }
//        return res;
    };

    const QString booleanErr = QStringLiteral(
                "Invalid value, it must be defined as 'true' or 'false'");
    auto booleanChecker = [](const QString &value) -> bool {
        return value == QLatin1String("true") || value == QLatin1String(
                    "false");
    };

    contrastColorOption.addChecker(colorChecker, colorErr);
    mainColorOption.addChecker(colorChecker, colorErr);
    delayOption.addChecker(numericChecker, delayErr);
    pathOption.addChecker(pathChecker, pathErr);
    trayOption.addChecker(booleanChecker, booleanErr);
    autostartOption.addChecker(booleanChecker, booleanErr);
    showHelpOption.addChecker(booleanChecker, booleanErr);
    screenNumberOption.addChecker(numericChecker, numberErr);

    // Relationships
    parser.AddArgument(guiArgument);
    parser.AddArgument(screenArgument);
    parser.AddArgument(fullArgument);
    parser.AddArgument(launcherArgument);
    parser.AddArgument(configArgument);
//    auto helpOption = parser.addHelpOption();
//    auto versionOption = parser.addVersionOption();
    parser.AddOptions({ pathOption, delayOption, rawImageOption }, guiArgument);
    parser.AddOptions({ TypeOption, NewnameOption, screenNumberOption, clipboardOption, pathOption,
                        delayOption, rawImageOption },
                      screenArgument);
    parser.AddOptions({TypeOption, NewnameOption, pathOption, clipboardOption, delayOption,
                       rawImageOption },
                      fullArgument);
    parser.AddOptions({ autostartOption, filenameOption, trayOption,
                        showHelpOption, mainColorOption, contrastColorOption },
                      configArgument);

    QStringList args;
    args << "/home/panjinrui/音乐/baihe/dbug/kylin_screenshot/kylin-screenshot";
    args << "--help";
    QVERIFY(!parser.parse(args));

    QStringList args1;
    args1 << "/home/panjinrui/音乐/baihe/dbug/kylin_screenshot/kylin-screenshot";
    args1 << "--version";
    QVERIFY(!parser.parse(args));
}

void MyTest::tst_textconfig()
{
    TextConfig textConfig;
    textConfig.setUnderline(true);
    textConfig.setStrikeOut(true);
    textConfig.setWeight(10);
    textConfig.setItalic(true);
    textConfig.weightButtonPressed(true);
    textConfig.weightButtonPressed(false);

    TextWidget textWidget;
    textWidget.adjustSize();
    textWidget.setMaxSize(100, 100);
    QFont f = textWidget.font();
    textWidget.setFont(f);
    textWidget.updateFont(f);
    QColor cl = textWidget.textColor();
    textWidget.setTextColor(cl);
    textWidget.setFontPointSize(10);
    textWidget.showEvent(nullptr);
    textWidget.resizeEvent(nullptr);

    TextTool texttool;
    texttool.isValid();
    texttool.closeOnButtonPressed();
    texttool.isSelectable();
    texttool.showMousePreview();
    texttool.name();
    texttool.nameID();
    texttool.description();
    texttool.widget();
    texttool.configurationWidget();
    texttool.copy();

    texttool.drawEnd(QPoint(0, 0));
    texttool.drawMove(QPoint(0, 0));
    texttool.thicknessChanged(1);
    texttool.textthicknessChanged(1);

    texttool.updateText("");
    texttool.setFont(textWidget.font());
    texttool.updateFamily("");
    texttool.updateFontUnderline(true);
    texttool.updateFontStrikeOut(true);
    texttool.updateFontWeight(QFont::Medium);
    texttool.updateFontItalic(true);
    QSize size;
    texttool.splitString("aaaa", size, 1);

    CaptureContext context;
    texttool.drawStart(context);
    texttool.pressed(context);
    texttool.textChanged(context);
    QColor background;
    texttool.icon(background, true);
    texttool.icon(background, false);
    texttool.icon(background, true, context);
    texttool.icon(background, false, context);
    context.style_name = "ukui-dark";
    texttool.icon(background, true, context);
    texttool.icon(background, false, context);
    context.style_name = "ukui-black";
    texttool.icon(background, true, context);
    texttool.icon(background, false, context);

    QPixmap p;
    texttool.undo(p);
    QPainter painter;
    texttool.paintMousePreview(painter, context);
    texttool.process(painter, p, true);
    texttool.colorChanged(background);
}

void MyTest::tst_scrollTool()
{
   ScrollTool st;
   QColor c;
   st.icon(c, true);
   st.icon(c, false);

   CaptureContext context;
   st.icon(c, true, context);
   st.icon(c, false, context);
   context.style_name = "ukui-dark";
   st.icon(c, true, context);
   st.icon(c, false, context);
   context.style_name = "ukui-black";
   st.icon(c, true, context);
   st.icon(c, false, context);
   st.pressed(context);
   QVERIFY(st.copy(nullptr) != nullptr);
   QVERIFY(!st.closeOnButtonPressed());
   QVERIFY(st.nameID().isEmpty());
}

void MyTest::tst_ocrTool()
{
   OcrTool st;
   QColor c;
   st.icon(c, true);
   st.icon(c, false);

   CaptureContext context;
   st.icon(c, true, context);
   st.icon(c, false, context);
   context.style_name = "ukui-dark";
   st.icon(c, true, context);
   st.icon(c, false, context);
   context.style_name = "ukui-black";
   st.icon(c, true, context);
   st.icon(c, false, context);
   st.pressed(context);
   QVERIFY(st.copy(nullptr) != nullptr);
   QVERIFY(st.closeOnButtonPressed());
   QVERIFY(st.nameID().isEmpty());
}

void MyTest::tst_options()
{
    Options option;
    option.closeOnButtonPressed();
    option.name();
    option.nameID();
    option.description();
    option.copy();
//    option.icon();
//            QIcon icon(const QColor &background, bool inEditor) const override;
//            QIcon icon(const QColor &background, bool inEditor, const CaptureContext &context) const override;
}

void MyTest::tst_enableTrayIcon()
{
    m_controller->enableTrayIcon();
    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_disableTrayIcon()
{
//    m_controller->disableTrayIcon();
    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_fullScreen()
{
//    QSignalSpy spy(m_dbus, &Dbus::processingFullScreen);
//    m_dbus->fullScreen();
//    QCOMPARE(spy.count(), 1);
//    m_dbusAdapter->fullScreen(pathValue, type, toClipboard, delay, id);
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString pathValue = QStringLiteral("/home/panjinrui/图片/");
    QString type = ".png";
    int delay = 0;
    bool toClipboard = false;
    QString ty = ScreenshotGsettings->get("screenshot-type").toString();
    if (!type.isEmpty()) {
        ScreenshotGsettings->set("screenshot-type", type);
    } else {
        if (ty.isEmpty() || ty == "") {
            type = ".png";
        } else {
            type = ty;
        }
        ScreenshotGsettings->set("screenshot-type", type);
    }
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if (!pathValue.isEmpty()) {
        ScreenshotGsettings->set("screenshot-path", pathValue);
    } else {
        if (path.isEmpty() || path == "") {
            QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            pathValue = a.at(0);
        } else {
            pathValue = path;
        }
        ScreenshotGsettings->set("screenshot-path", pathValue);
    }

    CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, pathValue, type);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!pathValue.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
//    m_id = req.id();
    m_controller->startFullscreenCapture(req.id());
    req.setStaticID(req.id());
    req.captureMode();
    req.delay();
    req.path();
    req.data();
    //                QString path, QString type, bool toClipboard, int delay, uint id);
//    DBusUtils dbusUtils;
//    QDBusMessage m
//        = QDBusMessage::createMethodCall(QStringLiteral("org.dharkael.kylinscreenshot"),
//                                         QStringLiteral("/"), QLatin1String(
//                                             ""), QStringLiteral("fullScreen"));
//    m << pathValue << type << toClipboard << delay << id;
//    QDBusConnection sessionBus = QDBusConnection::sessionBus();
//    dbusUtils.checkDBusConnection(sessionBus);
//    sessionBus.call(m);

//    if (isRaw) {
//        dbusUtils.connectPrintCapture(sessionBus, id);
//        // timeout just in case
//        QTimer t;
//        t.setInterval(delay + 2000);
//        QObject::connect(&t, &QTimer::timeout, qApp,
//                         &QCoreApplication::quit);
//        t.start();
//        // wait
//        return app.exec();
//    }
    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_screenGrab()
{
    QString numberStr = "-1";
    int number = numberStr.startsWith(QLatin1String("-")) ? -1 : numberStr.toInt();
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString pathValue = QStringLiteral("/home/panjinrui/图片/");
    QString type = ".png";
    int delay = 0;
    bool toClipboard = false;
    QString ty = ScreenshotGsettings->get("screenshot-type").toString();
    if (!type.isEmpty()) {
        ScreenshotGsettings->set("screenshot-type", type);
    } else {
        if (ty.isEmpty() || ty == "") {
            type = ".png";
        } else {
            type = ty;
        }
        ScreenshotGsettings->set("screenshot-type", type);
    }
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if (!pathValue.isEmpty()) {
        ScreenshotGsettings->set("screenshot-path", pathValue);
    } else {
        if (path.isEmpty() || path == "") {
            QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            pathValue = a.at(0);
        } else {
            pathValue = path;
        }
        ScreenshotGsettings->set("screenshot-path", pathValue);
    }

    CaptureRequest req(CaptureRequest::SCREEN_MODE,
                       delay, pathValue, type, number);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!pathValue.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    uint id = req.id();
//    int &&number = req.data().toInt();
    m_controller->startScreenGrab(id, number);
    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_visualCapture()
{
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString pathValue = QStringLiteral("/home/panjinrui/图片/");
    int delay = 0;
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if (!pathValue.isEmpty()) {
        ScreenshotGsettings->set("screenshot-path", pathValue);
    } else {
        if (path.isEmpty() || path == "") {
            QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            pathValue = a.at(0);
        } else {
            pathValue = path;
        }
        ScreenshotGsettings->set("screenshot-path", pathValue);
    }

    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, delay, pathValue);
    uint id = req.id();
    m_controller->startVisualCapture(id, path);
    m_captureWidget = m_controller->m_captureWindow;//new CaptureWidget(id, path);
    m_captureWidget->m_selection->setGeometry(100, 100, 400, 400);
    m_captureWidget->m_selection->saveGeometry();
    m_captureWidget->m_selection->savedGeometry();
    m_captureWidget->m_selection->getMouseSide(QPoint(0,0));
    m_captureWidget->m_selection->getMouseSide(QPoint(50,50));
    m_captureWidget->m_selection->getMouseSide(QPoint(450,450));
    m_captureWidget->m_selection->getMouseSide(QPoint(0,0));
    m_captureWidget->m_selection->getMouseSide(QPoint(0,0));
    m_captureWidget->m_selection->handlerAreas();
    m_captureWidget->m_selection->updateColor(Qt::black);
    m_captureWidget->m_selection->setVisible(true);
    CaptureContext context;
    m_captureWidget->updateChildWindow();
    m_captureWidget->updateMagnifier(context);
    m_captureWidget->updateCrosspixmap(QPoint(0,0));
    m_captureWidget->setTextDrawThickness(1);
    m_captureWidget->leftResize();
    m_captureWidget->rightResize();
    m_captureWidget->upResize();
    m_captureWidget->downResize();
    m_captureWidget->updateSizeIndicator();
    m_captureWidget->togglePanel();
    m_captureWidget->extendedSelection();
    QRect r(100, 100, 400, 400);
    m_captureWidget->extendedRect(&r);
    m_captureWidget->ClickedSaveType(0);
    m_captureWidget->ClickedSaveType(1);
    m_captureWidget->ClickedSaveType(2);
    m_captureWidget->ClickedSaveType(3);
    m_captureWidget->ClickedSaveType2(0);
    m_captureWidget->ClickedSaveType2(1);
    m_captureWidget->ClickedSaveType2(2);
    m_captureWidget->ClickedSaveType2(3);
    m_captureWidget->font_options_defult();
    m_captureWidget->font_options2_defult();
    m_captureWidget->fontsize_color_chose_default();
    m_captureWidget->fontsize_color_chose2_default();
    m_captureWidget->savetype_chose_default();
    m_captureWidget->savetype_chose2_default();
    m_captureWidget->destroy();
    if (m_captureWidget->getTaskbar_label())
    {
        m_captureWidget->getTaskbar_label()->setFirstMove(true);
        QVERIFY(m_captureWidget->getTaskbar_label()->getFirstMove());
    }
//    m_captureWidget->hide_window();
    //    m_captureWidget->pushToolToStack();
//    m_captureWindow->size_label_option();
}

void MyTest::tst_buttonHandler()
{
    QLabel *taskbar_label = new QLabel();
    QVector<CaptureButton *> vectorButtons;
    ConfigHandler m_config;
    ButtonHandler *m_buttonHandler = new ButtonHandler();
    auto buttons = m_config.getButtons(
            #ifdef ENABLE_RECORD
        isRecord
            #endif
        );
    for (const CaptureButton::ButtonType &t: buttons) {
        CaptureButton *b = new CaptureButton(t, taskbar_label);
#ifndef SUPPORT_NEWUI
        if (t == CaptureButton::TYPE_OPTION) {
            m_sizeIndButton = b;
            m_sizeIndButton->setColor(Qt::black);
        } else
#endif
        b->setColor(Qt::black);
//        makeChild(b);
#ifdef ENABLE_RECORD
        if (b->tool()->isIsolated()) {
            m_isolatedButtons.insert(b->m_buttonType, b->tool());
        }

#endif
        QVERIFY(!b->styleSheet().isEmpty());
        vectorButtons << b;
    }
    m_buttonHandler->setButtons(vectorButtons);
    m_buttonHandler->hide();
    QVERIFY(!m_buttonHandler->isVisible());
    m_buttonHandler->show();
//    QVERIFY(m_buttonHandler->isVisible());
//    m_buttonHandler->isVisible();
    m_buttonHandler->updateBlockedSides();
    m_buttonHandler->buttonsAreInside();
//    updateButtons();
    QRect rect(100, 100, 400, 400);
    m_buttonHandler->updatePosition(rect);
    m_buttonHandler->intersectWithAreas(rect);
    m_buttonHandler->size();
    m_buttonHandler->horizontalPoints(QPoint(0, 0), 2, true);
    m_buttonHandler->verticalPoints(QPoint(0, 0), 2, true);
    m_buttonHandler->resetRegionTrack();
//    m_buttonHandler->positionButtonsInside(1);
    m_buttonHandler->expandSelection();
    m_buttonHandler->ensureSelectionMinimunSize();
    QPoint p(0, 0);
    m_buttonHandler->adjustHorizontalCenter(p);
    m_buttonHandler->move_Font_Options_Window(0, 0);
    m_buttonHandler->move_Save_Location_Window(0, 0);
    m_buttonHandler->move_FontSize_Color_Chose_Window(0);
    m_buttonHandler->clearButtons();
    m_buttonHandler->updateScreenRegions(rect);
//    m_buttonHandler->contains(p);

    QVERIFY(m_dbusAdapter != nullptr);
}

void MyTest::tst_captureFailed()
{
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString pathValue = QStringLiteral("/home/panjinrui/图片/");
    QString type = ".png";
    int delay = 0;
    bool toClipboard = false;
    QString ty = ScreenshotGsettings->get("screenshot-type").toString();
    if (!type.isEmpty()) {
        ScreenshotGsettings->set("screenshot-type", type);
    } else {
        if (ty.isEmpty() || ty == "") {
            type = ".png";
        } else {
            type = ty;
        }
        ScreenshotGsettings->set("screenshot-type", type);
    }
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if (!pathValue.isEmpty()) {
        ScreenshotGsettings->set("screenshot-path", pathValue);
    } else {
        if (path.isEmpty() || path == "") {
            QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            pathValue = a.at(0);
        } else {
            pathValue = path;
        }
        ScreenshotGsettings->set("screenshot-path", pathValue);
    }

    CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, pathValue, type);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!pathValue.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    QSignalSpy spy(m_dbusAdapter, &FlameshotDBusAdapter::captureFailed);
    m_dbusAdapter->captureFailed(req.id());
    QCOMPARE(spy.count(), 1);
}

void MyTest::tst_captureExit()
{
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString pathValue = QStringLiteral("/home/panjinrui/图片/");
    QString type = ".png";
    int delay = 0;
    bool toClipboard = false;
    QString ty = ScreenshotGsettings->get("screenshot-type").toString();
    if (!type.isEmpty()) {
        ScreenshotGsettings->set("screenshot-type", type);
    } else {
        if (ty.isEmpty() || ty == "") {
            type = ".png";
        } else {
            type = ty;
        }
        ScreenshotGsettings->set("screenshot-type", type);
    }
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if (!pathValue.isEmpty()) {
        ScreenshotGsettings->set("screenshot-path", pathValue);
    } else {
        if (path.isEmpty() || path == "") {
            QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            pathValue = a.at(0);
        } else {
            pathValue = path;
        }
        ScreenshotGsettings->set("screenshot-path", pathValue);
    }

    CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, pathValue, type);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!pathValue.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    QSignalSpy spy(m_dbusAdapter, &FlameshotDBusAdapter::captureExit);
    m_dbusAdapter->captureExit(req.id());
    QCOMPARE(spy.count(), 1);
}

void MyTest::tst_captureCopy()
{
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    QString pathValue = QStringLiteral("/home/panjinrui/图片/");
    QString type = ".png";
    int delay = 0;
    bool toClipboard = false;
    QString ty = ScreenshotGsettings->get("screenshot-type").toString();
    if (!type.isEmpty()) {
        ScreenshotGsettings->set("screenshot-type", type);
    } else {
        if (ty.isEmpty() || ty == "") {
            type = ".png";
        } else {
            type = ty;
        }
        ScreenshotGsettings->set("screenshot-type", type);
    }
    QString path = ScreenshotGsettings->get("screenshot-path").toString();
    if (!pathValue.isEmpty()) {
        ScreenshotGsettings->set("screenshot-path", pathValue);
    } else {
        if (path.isEmpty() || path == "") {
            QStringList a = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
            pathValue = a.at(0);
        } else {
            pathValue = path;
        }
        ScreenshotGsettings->set("screenshot-path", pathValue);
    }

    CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, pathValue, type);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!pathValue.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    QSignalSpy spy(m_dbusAdapter, &FlameshotDBusAdapter::captureCopy);
    m_dbusAdapter->captureCopy(req.id());
    QCOMPARE(spy.count(), 1);
}

QTEST_APPLESS_MAIN(MyTest)
#include "tst_mytest.moc"

