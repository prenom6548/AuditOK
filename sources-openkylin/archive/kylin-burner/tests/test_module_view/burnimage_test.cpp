#include <QtTest>
#include "src/view/burnimage.h"
#include "src/data/datacdrom.h"

class BurnImageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testBurnImageCreation();
    void testBurnImageInitialization();
    void testBurnImageMethods();
};

void BurnImageTest::initTestCase()
{
    // 在所有测试开始前执行
}

void BurnImageTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void BurnImageTest::init()
{
    // 在每个测试函数执行前执行
}

void BurnImageTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void BurnImageTest::testBurnImageCreation()
{
    BurnImage *widget = new BurnImage();
    QVERIFY(widget != nullptr);
    delete widget;
}

void BurnImageTest::testBurnImageInitialization()
{
    BurnImage *widget = new BurnImage();
    QVERIFY(widget != nullptr);
    
    delete widget;
}

void BurnImageTest::testBurnImageMethods()
{
    BurnImage *widget = new BurnImage();
    QVERIFY(widget != nullptr);
    widget->resize(200,100);

    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout,this,[&]{
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            widget->close();
        }
    });
    timer->start(1000);

    //widget->slotsBrowseISO();
    widget->slotsISOError(QString("info is error"));
    widget->slotsReady(QString("the medium is ready"));
    widget->slotsNewBurner(QIcon(),"new burner", "tooltip", "uuid");
    widget->slotsDeleteBurner("uuid");
    widget->slotsBurnerChange(0);

    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    widget->slotsUpdateToolTipText(cdrom, "new tooltip");
    widget->updateComboboxTooltip();
    widget->on_comboBoxBurner_activated(0);
    QVERIFY(true);

}

#ifndef RUN_VIEW_UNIFIED_TESTS
QTEST_MAIN(BurnImageTest)
#endif

#include "burnimage_test.moc"
