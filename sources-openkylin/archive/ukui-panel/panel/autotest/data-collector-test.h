#ifndef DATA_COLLECTOR_TEST_H
#define DATA_COLLECTOR_TEST_H

#include <QObject>

class DataCollectorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testPositionToStr_data();
    void testPositionToStr();
    void testSingletonAndThreadSetup();
    void testStartRunningWhileAlreadyRunning();
    void testMetaObjectCoverage();
    void testSetPanelPositionEvent();
    void testSetPanelSizeEvent();
    void testDirectWrapperMethodsEmitSignals();
    void testStandaloneDestructorHandlesNullWrapper();
    void testDeleteThroughBasePointer();
    void testZZDestroySingletonClearsInstance();
};

#endif // DATA_COLLECTOR_TEST_H
