#ifndef TEST_CONFIG_PROGRESSBAR_ANIMATION_HELPER_H
#define TEST_CONFIG_PROGRESSBAR_ANIMATION_HELPER_H

#include <QObject>
#include <QWidget>
#include <QTest>
#include <QSignalSpy>
#include <QVariantAnimation>
#include "config-progressbar-animation-helper.h"

using namespace UKUIConfigStyleSpace;

class TestConfigProgressBarAnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigProgressBarAnimationHelper(QObject *parent = nullptr);
    ~TestConfigProgressBarAnimationHelper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_initAnimationMap();
    void test_initAnimationMapWithNullWidget();
    void test_initAnimationMapWithNullAnimation();
    void test_stopAnimation();
    void test_stopAnimationNotInMap();
    void test_getAnimation();
    void test_getAnimationNotInMap();
    void test_canEnableNormalAnimation();
    void test_canEnableNormalAnimationBoundary();
    void test_getSliderFixedSpan();
    void test_cacheTotalBarInfo();
    void test_getTotalBarInfo();
    void test_meetCondition();
    void test_meetConditionBoundary();
    void test_setAnimationDuration();
    void test_setAnimationDurationWithNullAnimation();

private:
    QWidget *m_testWidget;
    QVariantAnimation *m_testAnimation;
};

#endif
