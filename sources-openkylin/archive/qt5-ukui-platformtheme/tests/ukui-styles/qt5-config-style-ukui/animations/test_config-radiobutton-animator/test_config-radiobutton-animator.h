#ifndef TEST_CONFIG_RADIOBUTTON_ANIMATOR_H
#define TEST_CONFIG_RADIOBUTTON_ANIMATOR_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>
#include <QRadioButton>
#include "config-radiobutton-animator.h"

class TestConfigRadioButtonAnimator : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigRadioButtonAnimator(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testBindWidget();
    void testBindWidgetWithDoNotAnimateProperty();
    void testBindWidgetWithNonRadioButton();
    void testUnboundWidget();
    void testValue();
    void testSetAnimatorStartValue();
    void testSetAnimatorEndValue();
    void testSetAnimatorDuration();
    void testSetAnimatorDirectionForward();
    void testIsRunning();
    void testStartAnimator();
    void testStopAnimator();
    void testCurrentAnimatorTime();
    void testSetAnimatorCurrentTime();
    void testTotalAnimationDuration();
    void testIsStopped();

private:
    UKUIConfigStyleSpace::ConfigRadioButtonAnimator *m_animator;
    QRadioButton *m_radioButton;
};

#endif // TEST_CONFIG_RADIOBUTTON_ANIMATOR_H
