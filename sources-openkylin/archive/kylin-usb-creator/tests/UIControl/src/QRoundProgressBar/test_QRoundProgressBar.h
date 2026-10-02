#ifndef TEST_QROUNDPROGRESSBAR_H
#define TEST_QROUNDPROGRESSBAR_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>

class QRoundProgressBar;

class TestQRoundProgressBar : public QObject
{
    Q_OBJECT

public:
    TestQRoundProgressBar();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试用例
    void testConstructor();
    void testSetPersent_data();
    void testSetPersent();
    void testPaintEvent();
    void testSetPersentWithInvalidValues_data();
    void testSetPersentWithInvalidValues();
    void testSetPersentBoundaryConditions_data();
    void testSetPersentBoundaryConditions();
    void testPaintEventWithDifferentThemes();
    void testSetPersentEdgeCases();
    void testSetPersentWithSameValue();
    void testPaintEventWithZeroSize();
    void testPaintEventWithDifferentPercentValues();
    void testSetPersentNegativeValues();

private:
    QRoundProgressBar *m_progressBar;
};

#endif // TEST_QROUNDPROGRESSBAR_H