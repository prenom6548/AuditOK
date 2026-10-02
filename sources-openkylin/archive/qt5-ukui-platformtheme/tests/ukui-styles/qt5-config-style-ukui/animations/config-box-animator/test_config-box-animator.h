#ifndef TEST_CONFIG_BOX_ANIMATOR_H
#define TEST_CONFIG_BOX_ANIMATOR_H

#include <QObject>
#include <QApplication>
#include <QTest>
#include <QComboBox>
#include <QVariantAnimation>

#include "config-box-animator.h"

using namespace UKUIConfigStyleSpace;

class TestConfigBoxAnimator : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testBindWidget();
    void testBindWidgetWithDoNotAnimate();
    void testBindWidgetNonComboBox();
    void testUnboundWidget();
    void testValue();
    void testIsRunning();
    void testSetAnimatorStartValue();
    void testSetAnimatorEndValue();
    void testSetAnimatorDuration();
    void testSetAnimatorDirectionForward();
    void testStartAnimator();
    void testStopAnimator();
    void testCurrentAnimatorTime();
    void testTotalAnimationDuration();

private:
    ConfigBoxAnimator *m_animator;
    QComboBox *m_comboBox;
};

#endif // TEST_CONFIG_BOX_ANIMATOR_H
