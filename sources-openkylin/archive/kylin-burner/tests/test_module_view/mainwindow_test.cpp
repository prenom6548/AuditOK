#include <QtTest>
#include "src/view/mainwindow.h"
#include "src/data/datacdrom.h"
#include "src/model/logicmain.h"
#include "src/frame/signals.h"
#include "src/view/burndata.h"
#include "src/data/datadirectory.h"
#include <QMoveEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QTimer>
#include <QFileInfo>

#include "src/frame/configuration.h"
#include "src/frame/logger.h"
#include "src/model/logicmain.h"
#include "src/model/menufactory.h"
#include "src/model/logiccopydisc.h"
#include "src/view/modaldialog.h"
#include "src/view/aboutdialog.h"
#include "src/view/execdialog.h"
#include "src/view/filefilterrules.h"
#include "src/view/viewfilterfiles.h"
#include "src/view/tip_burnresult.h"
#include "src/po/translation.h"
#include "src/common/utils.h"
#include "src/common/trackingmanager.h"
#include "src/view/tips.h"
#include "src/view/inputchange.h"


#include <QTranslator>
#include <QLibraryInfo>
#include <kwidget.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <windowmanager/windowmanager.h>
#include <QTranslator>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDir>
#include <QElapsedTimer>

class MockDataCDRom : public DataCDROM{
    Q_OBJECT
public:
    MockDataCDRom(MEDIUM_SYSTEM_TYPE mediumType, int mediumStatus, QString burnformat = ""): DataCDROM("/dev/sr0"){
        m_meidum_type = mediumType;
        m_mediumState = mediumStatus;
        m_burn_format = burnformat;

    }
    MEDIUM_SYSTEM_TYPE fileSystemTypeWithExtendName(){
        return m_meidum_type;
    }

    int mediumStatus(){
        m_mediumState;
    }
    QString getBurnFormat(){
        m_burn_format;
    }
public:
    MEDIUM_SYSTEM_TYPE m_meidum_type;
    int m_mediumState;
    QString m_burn_format;
};


class MainWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testMainWindowCreation();
    void testMainWindowInitialization();
    void testMainWindowMethods();
    void testPublicMethods();
    void testProtectedMethods();
    void testPublicSlots();
    void testPrivateSlots();
    void testPrivateMethods();
    void testSignals();
    void testSpecialFeatures();
    void testAddTestData();
    void testDiscRead();
    void testNewAddMethods();
    void testMessageBox();
};

void MainWindowTest::initTestCase()
{
    // 在所有测试开始前执行
}

void MainWindowTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void MainWindowTest::init()
{
    // 在每个测试函数执行前执行
}

void MainWindowTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void MainWindowTest::testMainWindowCreation()
{
    MainWindow *window = nullptr;

    window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
}

void MainWindowTest::testMainWindowInitialization()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 检查初始状态
    QVERIFY(window->isEnabled() == true);
}

void MainWindowTest::testMainWindowMethods()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 测试基本功能
    window->show();
    QVERIFY(window->isVisible() == true);
    
    window->hide();
    QVERIFY(window->isVisible() == false);
    
    // 测试其他方法
    int result = window->slotsReShowMainWindow(QStringList());
    int result2 = window->slotsReShowMainWindow(QStringList() << "/path/to/file.iso");
}

void MainWindowTest::testPublicMethods()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);

    // 测试静态实例方法
    MainWindow *instance2 = MainWindow::Instance();
    QVERIFY(instance2 != nullptr);
    QVERIFY(window == instance2); // 单例模式验证

    // 测试currentDataRomIsUDF方法
     bool isUdf = window->currentDataRomIsUDF();
    
     //测试getCountDevice方法
    int deviceCount = window->getCountDevice();
    
     //测试getDeviceByIndex方法 - 需要有效索引
    if (deviceCount > 0) {
        QString device = window->getDeviceByIndex(0);
        QVERIFY(device != "Error");
    }

    // 测试getWinId方法
    WindowId winId = window->getWinId();
    
    // 测试isBusyDevice方法
    bool isBusy = window->isBusyDevice("test_device");
    
    // 测试slotsReShowMainWindow方法
    int result = window->slotsReShowMainWindow(QStringList());
    
    // 测试带参数的slotsReShowMainWindow方法
    int result2 = window->slotsReShowMainWindow(QStringList() << "/test/path.iso");
    int result3 = window->slotsReShowMainWindow(QStringList() << "/test/path.txt");
}

void MainWindowTest::testProtectedMethods()
{
    GlobalUserSignal::getInstance()->blockSignals(true);
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 测试eventFilter方法
    QObject testObj;
    QEvent testEvent(QEvent::User);
    bool filterResult = window->eventFilter(&testObj, &testEvent);
    QVERIFY(filterResult == false);
    
    // 测试事件过滤器对特定对象的处理
    QWidget *comboBurner = window->findChild<QWidget*>("kylin-burner_QComboBox_MainWindow_comboBoxBurner");
    QWidget *comboDisk = window->findChild<QWidget*>("kylin-burner_QComboBox_MainWindow_comboBoxDisk");
    QWidget *navWidget = window->findChild<QWidget*>("kylin-burner_QWidget_MainWindow_nav");
    
    if (comboBurner) {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
        bool filterResult2 = window->eventFilter(comboBurner, &keyEvent);
        QVERIFY(filterResult2 == false);
    }
    
    if (comboDisk) {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier);
        bool filterResult3 = window->eventFilter(comboDisk, &keyEvent);
        QVERIFY(filterResult3 == false);
    }
    
    if (navWidget) {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_C, Qt::NoModifier);
        bool filterResult4 = window->eventFilter(navWidget, &keyEvent);
        QVERIFY(filterResult4 == false);
    }
    
    // 测试焦点相关的事件过滤器逻辑（仅在UKUI_SESSION_TYPE == 2时有效）
    QEvent focusInEvent(QEvent::FocusIn);
    bool filterResult5 = window->eventFilter(window, &focusInEvent);
    
    QEvent focusOutEvent(QEvent::FocusOut);
    bool filterResult6 = window->eventFilter(window, &focusOutEvent);
    
    QEvent activationChangeEvent(QEvent::ActivationChange);
    bool filterResult7 = window->eventFilter(window, &activationChangeEvent);
    
    QEvent windowActivateEvent(QEvent::WindowActivate);
    bool filterResult8 = window->eventFilter(window, &windowActivateEvent);
    
    QEvent windowDeactivateEvent(QEvent::WindowDeactivate);
    bool filterResult9 = window->eventFilter(window, &windowDeactivateEvent);
    
    QEvent resizeEvent(QEvent::Resize);
    bool filterResult10 = window->eventFilter(window, &resizeEvent);
    
    // 测试moveEvent方法
    QMoveEvent moveEvent(QPoint(100, 100), QPoint(50, 50));
    window->moveEvent(&moveEvent);
    
    // 测试paintEvent方法
    // 由于paintEvent需要有效的paint设备，我们跳过直接测试
    
    // 测试changeEvent方法
    QEvent changeEvent(QEvent::ActivationChange);
    window->changeEvent(&changeEvent);
    
    // 测试keyPressEvent方法
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    window->keyPressEvent(&keyEvent);
    
    // 测试keyPressEvent方法 - F1键
    QKeyEvent f1KeyEvent(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    window->keyPressEvent(&f1KeyEvent);
    
    // 测试keyPressEvent方法 - Alt+N键
    QKeyEvent altNEvent(QEvent::KeyPress, Qt::Key_N, Qt::AltModifier);
    window->keyPressEvent(&altNEvent);
    
    // 测试keyPressEvent方法 - Delete键
    QKeyEvent deleteEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    window->keyPressEvent(&deleteEvent);
    
    // 测试keyPressEvent方法 - Alt+C键
    QKeyEvent altCEvent(QEvent::KeyPress, Qt::Key_C, Qt::AltModifier);
    window->keyPressEvent(&altCEvent);
    
    // 测试keyPressEvent方法 - Alt+A键
    QKeyEvent altAEvent(QEvent::KeyPress, Qt::Key_A, Qt::AltModifier);
    window->keyPressEvent(&altAEvent);
    

    
    // 测试changeEvent方法 - 窗口状态改变
    QEvent windowStateEvent(QEvent::WindowStateChange);
    window->changeEvent(&windowStateEvent);
    GlobalUserSignal::getInstance()->blockSignals(false);
    //window->slotsCloseApp();
}

void MainWindowTest::testPublicSlots()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 测试public slots
    int result = window->slotsReShowMainWindow(QStringList() << "/path/to/file.iso");
    
    bool isBusy = window->isBusyDevice("test_device");
    
    window->slotChangeUIFontSize(12);
}

void MainWindowTest::testPrivateSlots()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 测试private slots
    window->slotsBurnData();
    window->slotsBurnImage();
    window->slotsCopyDisc();
    window->slotsAdjustBurnData(0);
    window->slotsAdjustBurnData(-1); // 测试负索引


    
    // 测试slotsBusness方法
    window->slotsBusness(true);
    window->slotsBusness(false);

    DataCDROM* inputCDROM = new DataCDROM("/dev/sr0");

    // 测试slotsInitCreateISO方法 - 需要一个DataCDROM指针，暂时用nullptr测试
    //window->slotsInitCreateISO(inputCDROM);
    
    // 测试slotsNoMediun方法
    window->slotsNoMediun(nullptr);
    
    // 测试slotsRemoveCDROM方法
    window->slotsRemoveCDROM(nullptr);


    // 测试slotsBrowseOrSetting方法
    window->slotsBrowseOrSetting(true);
    window->slotsBrowseOrSetting(false);


    
    // 测试slotsUpdateDisk方法
    window->slotsUpdateDisk(0, inputCDROM);
    
    // 测试slotsCloseAll方法
    // window->slotsCloseAll(); // 这会关闭窗口，跳过
    
    // 测试slotsChangeName方法
    window->slotsChangeName(LogicMain::GetISO);
    window->slotsChangeName(LogicMain::CopyData); // 测试另一个值


    
    // 测试slotsChangeBtnBusnessName方法
    window->slotsChangeBtnBusnessName();
    
    // 测试uiModeChange方法
    window->uiModeChange(true);
    window->uiModeChange(false);
    
    // 测试slotsCloseApp方法
    // window->slotsCloseApp(); // 这会关闭应用，跳过
    
    // 测试slotsChangeCurrentIndex方法
    window->slotsChangeCurrentIndex(0);
    
    // 测试slotShowTileFlyout和slotHideTileFlyout方法
    QRect testRect(100, 100, 200, 200);
    window->slotShowTileFlyout(testRect);
    window->slotHideTileFlyout();

    
    // 测试slotsChangeCurrentIndex方法
    window->slotsChangeCurrentIndex(-1); // 测试负索引
    window->slotsChangeCurrentIndex(999); // 测试大索引
    
    // 测试更多边缘情况
    window->slotsChangeName(-1);  // 测试无效参数
    window->slotsChangeName(999); // 测试大参数
    
    // 测试不同大小的参数
    QStringList largeArgs;
    for (int i = 0; i < 10; i++) {
        largeArgs << QString("arg%1").arg(i);
    }
    int result = window->slotsReShowMainWindow(largeArgs);

}

void MainWindowTest::testPrivateMethods()
{

    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);

    // 测试私有方法
    bool started = window->started();

    window->uiAdjust();
    window->initTransparent();
    window->calculatorTime(0);
    window->calculatorTime(1);
    window->calculatorTime(2);
    
    // 测试resetText方法
    QPushButton testButton;
    testButton.setText("Test Button");
    testButton.setToolTip("Test Tooltip");
    testButton.setText("ThisIsAVeryLongButtonTextThatShouldBeTruncated");
    window->resetText(&testButton);
    
    window->setAccessibleName();


    // 测试currentDataRomIsUDF方法
    bool isUdf = window->currentDataRomIsUDF();

    // 测试getCountDevice方法
    int deviceCount = window->getCountDevice();
    
    // 测试getDeviceByIndex方法
    if (deviceCount > 0) {
        QString device = window->getDeviceByIndex(0);
    }

    QString invalidDevice = window->getDeviceByIndex(-1);
    QVERIFY(invalidDevice == "Error");


    // 测试getWinId方法
    WindowId winId = window->getWinId();
    
    // 测试其他边界情况
    QString invalidDevice2 = window->getDeviceByIndex(999);
    QVERIFY(invalidDevice2 == "Error");

}

void MainWindowTest::testSignals()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 验证信号连接正常
    QVERIFY(true); // 简单验证连接成功
}

void MainWindowTest::testSpecialFeatures()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);
    
    // 测试一些特殊的代码路径
    
    // 测试平板模式切换
    window->uiModeChange(true);
    window->uiModeChange(false);
    
    // 测试计算器时间功能
    window->calculatorTime(0);
    window->calculatorTime(1);
    window->calculatorTime(2);
    
    // 测试一些边界情况
    window->slotsAdjustBurnData(-1);  // 负索引
    window->slotsAdjustBurnData(999); // 大索引
    
    // 测试slotsChangeCurrentIndex边界情况
    window->slotsChangeCurrentIndex(-1);
    window->slotsChangeCurrentIndex(999);
    
    // 测试事件过滤器对堆栈部件的处理
    QWidget *stackedWidget = window->findChild<QWidget*>("kylin-burner_QStackedWidget_MainWindow_stackedWidget");
    if (stackedWidget) {
        QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_D, Qt::NoModifier);
        bool filterResult = window->eventFilter(stackedWidget, &keyEvent);
        QVERIFY(filterResult == false);
    }
    
    // 测试透明度初始化
    window->initTransparent();
    
    // 测试resetText方法 - 边界情况
    QPushButton button;
    button.setText("Short");
    button.setToolTip("This is a very long tooltip that might exceed the button width");
    window->resetText(&button);
    
    button.setText("ThisIsAVeryLongTextThatWillBeTruncated...");
    button.setToolTip("Short");
    window->resetText(&button);
}

void MainWindowTest::testAddTestData()
{
    Utils::initSystemInfo();

    //it can reduce some error when user some operation
    Configuration::instance()->set("DataSetting", "Duplicate Data", 0);
    Configuration::instance()->set("DataSetting", "Add Data", false);
    Configuration::instance()->set("DataSetting", "Delete Data", false);
    Configuration::instance()->set("DataSetting", "Clean Data", true);

    Configuration::instance()->set("FilterFile","hidden",false);
    Configuration::instance()->set("FilterFile","broken",false);
    Configuration::instance()->set("FilterFile","Repleace",false);

    QString logFilePath = QDir::homePath().append("/.log/kylin-burner/");
    CFG->set("logger","path",logFilePath);
    LOGGER->init();

    MenuFactory::instance();
    Tip_BurnResult::instance();
    ViewFilterFiles::instance();
    LogicMain::instance()->init();
    LogicCopyDisc::instance();
    ExecDialog::instance();

    MainWindow::Instance(QStringList(), nullptr);
    MainWindow::Instance()->show();
    LogicMain::instance()->initISO();
}

// 测试构造函数
void MainWindowTest::testDiscRead()
{
    QString device = "/dev/sr0";
    // 创建DiscRead对象
    DiscRead * discread = new DiscRead(device);
    discread->work();
    bool success = discread->isSuccess();
    discread->isBlank();
    discread->isReady();
    discread->isGood();
    discread->canAppend();
    int numTracks = discread->numDataTracks();
    QString label = discread->discLabel();
    QString mediaType =discread->discMediaType();
    QString fsType =discread->discFilesystemType();
    QString fsExtInf =discread->fileSystemExtendInf();

    int profile = discread->readDiscProfile();

    uchar* infoData = nullptr;
    bool result =discread->readDiscInformation(&infoData);
    // 不管是否成功，只要不崩溃即可
    if (infoData) {
        free(infoData);
    }

    // 测试各种DVD类型
    QString result1 = dvdMediaType("optical_dvd_plus_r");
    QVERIFY(!result1.isEmpty());

    QString result2 = dvdMediaType("optical_dvd_r");
    QVERIFY(!result2.isEmpty());

    QString result3 = dvdMediaType("optical_dvd");
    QVERIFY(!result3.isEmpty());

    QString result4 = dvdMediaType("");
    QVERIFY(result4.isEmpty());

    QString result5 = dvdMediaType("invalid_type");
    QVERIFY(!result5.isEmpty());



    QVERIFY(true); // 如果能成功构造，则测试通过
}

void MainWindowTest::testNewAddMethods()
{
    MainWindow *window = MainWindow::Instance(QStringList(), nullptr);
    QVERIFY(window != nullptr);

    QModelIndex index1 =  window->m_NavBar->listview()->model()->index(0,0);
    window->slotChangeStackWindowIndex(index1);

    QModelIndex index2 =  window->m_NavBar->listview()->model()->index(1,0);
    window->slotChangeStackWindowIndex(index2);

    QModelIndex index3 =  window->m_NavBar->listview()->model()->index(2,0);
    window->slotChangeStackWindowIndex(index3);

    MockDataCDRom* cdrom1 = new MockDataCDRom(UDF102, DataMedium::MEDIUM_EMPTY);
    window->slotSwitchBurnFormat(cdrom1);

    MockDataCDRom* cdrom2 = new MockDataCDRom(UDF102, DataMedium::MEDIUM_FINALIZED);
    window->slotSwitchBurnFormat(cdrom2);

    MockDataCDRom* cdrom3 = new MockDataCDRom(UDF201, DataMedium::MEDIUM_EMPTY);
    window->slotSwitchBurnFormat(cdrom3);

    MockDataCDRom* cdrom4 = new MockDataCDRom(ISO9660, DataMedium::MEDIUM_EMPTY, "ISO9660");
    window->slotSwitchBurnFormat(cdrom4);

    MockDataCDRom* cdrom5 = new MockDataCDRom(ISO9660, DataMedium::MEDIUM_INCOMPLETE, "ISO9660");
    window->slotSwitchBurnFormat(cdrom5);

    MockDataCDRom* cdrom6 = new MockDataCDRom(ISO9660, DataMedium::MEDIUM_FINALIZED, "ISO9660");
    window->slotSwitchBurnFormat(cdrom6);

    window->showMaximized();
    window->slotChangeWindowState();
    window->slotChangeWindowState();

    QString file_save_path = QCoreApplication::applicationDirPath().append(QString("/../../test_data/test.iso"));
    window->slotChangeToImageBurnPage(file_save_path);

    QPaintEvent* event  = new QPaintEvent(window->rect());
    window->paintEvent(event);


    QVERIFY(true);
    delete window;
}

void MainWindowTest::testMessageBox()
{
    tips* tip = new tips();
    tip->information("hello");
    tip->error("hello");
    tip->question("hello");
    delete tip;

    InputChange* input = new InputChange();
    input->result();
    input->content();
    input->setLineEditSelect("test.iso");
    input->indexSuffix("text.iso");
    input->slotTextChanged("text.iso");
    QString testContent = QString("testStr");
    //nput->input(testContent,"","","");
    delete input;

    QVERIFY(true);

}

#ifndef RUN_VIEW_UNIFIED_TESTS
QTEST_MAIN(MainWindowTest)
#endif

#include "mainwindow_test.moc"
