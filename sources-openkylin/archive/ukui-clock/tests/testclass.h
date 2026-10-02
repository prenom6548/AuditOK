#ifndef TESTCLASS_H
#define TESTCLASS_H

#include <QObject>
#include <QtTest/QtTest>
#include <QtWidgets/QWidget>
#include "clock.h"



class TestClass : public QObject
{
    Q_OBJECT
public:
    explicit TestClass(QObject *parent = nullptr);
    ~TestClass();
private Q_SLOTS:
    void testClockConstructor();
    void testclock ();
    void testMuteBell();
    void testadaptscreeninfo();
    void testbaseverticalscroll();
    void testcJSON ();
    void testclockdbusadaptor ();
    void testclockentitydao ();
    void testClockInterface ();
    void testcommontooltip ();
    void testCountdownAnimation();
    void testdbusdeleteclockbyidadaptor ();
    void testdbusselectclockbyidadaptor ();
    void testdbusupdateclockadaptor ();
    void testdeleteMsg ();
    void testglobalinstance ();
    void testitemNew ();
    void testkylinqfiledialog ();
    void testprimarymanager ();
    void testqroundProgressBar ();
    void testselectbtn ();
    void testselectbtnutil ();
    void teststopwatchItem ();
    void testtheme ();
    void testtinycountdown ();
    void testverticalScroll24 ();
    void testverticalScroll60 ();
    void testverticalScroll99 ();
    void testTrayIconActivated();
    void testInteger();
    void testSetAlarmRepeatDialog();
    void testUtils();
    void testDbusNoticeShowAgainAdaptor();
    void testDeleteMsg();
    void testCustomListWidget();
    void testCommontooltip();
    void testQRoundProgressBar();
    void testBaseVerticalScroll();  // 测试baseverticalscroll.h中的BaseVerticalScroll类
    void testcomboBox();
    void testnoticealarmpoolutil();
    void testSingleApplication();
    void testStopwatchItem();
    void testTinyCountdown();
    void testDbusUpdateClockAdaptor();
    void testDbusSelectClockByIdAdaptor();
    void testMpvLibLoad();
    void testClockAdditionalFeatures();
    void testOtherFunctions();
    void testCloseEvent();
private:
        Clock * pClock {nullptr};

        CustomComBox* comboBox {nullptr};
};

#endif // TESTCLASS_H
