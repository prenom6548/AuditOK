#include <QtTest>
#include <QTimer>
#include <QApplication>
#include "src/view/copydisc.h"
#include "src/data/datacdrom.h"
#include "src/model/logiccopydisc.h"

class CopyDiscTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testCopyDiscCreation();
    void testCopyDiscInitialization();
    void testCopyDiscMethods();
    void testLogicCopyDisc();
};

void CopyDiscTest::initTestCase()
{
    // 在所有测试开始前执行
}

void CopyDiscTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void CopyDiscTest::init()
{
    // 在每个测试函数执行前执行
}

void CopyDiscTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void CopyDiscTest::testCopyDiscCreation()
{
    CopyDisc *widget = new CopyDisc();
    QVERIFY(widget != nullptr);
    QVERIFY(widget->objectName() != "");
    delete widget;
}

void CopyDiscTest::testCopyDiscInitialization()
{
    CopyDisc *widget = new CopyDisc();
    QVERIFY(widget != nullptr);
    
    // 检查初始状态
    QVERIFY(widget->isEnabled() == true);
    
    delete widget;
}

void CopyDiscTest::testCopyDiscMethods()
{
    CopyDisc *widget = new CopyDisc();
    QVERIFY(widget != nullptr);

    QTimer* timer = new QTimer();
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [widget, timer]() {
        timer->stop();
        if (widget) {
            widget->close();

        }
    });
    timer->start(1000);

    widget->check();
    widget->updateLabelIsoTip("copy disc tipinfo");
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    DataCDROM* srcRom = new DataCDROM("/dev/sr1");
    widget->slotsAddDestination(cdrom);

    widget->slotsAddSource(srcRom);
   // widget->slotsButtonClick();


    widget->slotsDelSource(0);
    widget->slotsDelDestination(0);

    //widget->slotsButtonClick();

    QVERIFY(true);
}

void CopyDiscTest::testLogicCopyDisc()
{
    // 创建LogicCopyDisc对象
    LogicCopyDisc* logicCopyDisc = LogicCopyDisc::instance();
    QVERIFY(logicCopyDisc != nullptr);

    QTimer* timer = new QTimer();
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [timer]() {
        QWidget* activePopup = QApplication::activeModalWidget();
        if (activePopup) {
            QTest::keyClick(activePopup, Qt::Key_Return);
        }
        timer->stop();
        timer->deleteLater();
    });
    timer->start(1000);

    // 添加一些对象来测试其他分支
    DataCDROM* mockSrc = new DataCDROM("/dev/sr0");
    DataCDROM* mockDes = new DataCDROM("/dev/sr1");

    //true
    logicCopyDisc->slotsAddSource(static_cast<void*>(mockSrc));
    logicCopyDisc->slotsAddDestination(static_cast<void*>(mockDes));

    //false
    logicCopyDisc->slotsAddSource(static_cast<void*>(mockSrc));
    logicCopyDisc->slotsAddDestination(static_cast<void*>(mockDes));
    logicCopyDisc->isDestinationDiscDL();

    logicCopyDisc->capacity();
    logicCopyDisc->slotsDestinationChange(0);
    logicCopyDisc->slotsSourceChange(0);
    logicCopyDisc->capacity();

    logicCopyDisc->isDestinationDiscDL();

    logicCopyDisc->updateISOPath("/home/path");


    logicCopyDisc->progressWidgetDelayResponeTimer(200);

    logicCopyDisc->slotsSetting();


    logicCopyDisc->slotsGetISO();

    delete logicCopyDisc->m_oInstance;

    QVERIFY(true);
}

#ifndef RUN_VIEW_UNIFIED_TESTS
QTEST_MAIN(CopyDiscTest)
#endif

#include "copydisc_test.moc"
