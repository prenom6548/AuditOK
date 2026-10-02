#ifndef TEST_CONFIG_SLIDER_ANIMATOR_H
#define TEST_CONFIG_SLIDER_ANIMATOR_H

#include <QObject>
#include <QTest>
#include <QSlider>
#include <QVariantAnimation>

#include "config-slider-animator.h"

using namespace UKUIConfigStyleSpace;

class TestConfigSliderAnimator : public QObject
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
    void testBindWidgetNonSlider();
    void testUnboundWidget();
    void testBoundedWidget();
    void testValue();
    void testIsRunning();
    void testSetAnimatorStartValue();
    void testSetAnimatorEndValue();
    void testAnimatorStartValue();
    void testAnimatorEndValue();
    void testSetAnimatorDuration();
    void testSetAnimatorDirectionForward();
    void testStartAnimator();
    void testStopAnimator();
    void testCurrentAnimatorTime();
    void testSetAnimatorCurrentTime();
    void testTotalAnimationDuration();
    void testSetExtraProperty();
    void testGetExtraProperty();

private:
    ConfigSliderAnimator *m_animator;
    QSlider *m_slider;
};

#endif // TEST_CONFIG_SLIDER_ANIMATOR_H
