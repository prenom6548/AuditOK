#ifndef TEST_CONFIG_BUTTON_ANIMATOR_H
#define TEST_CONFIG_BUTTON_ANIMATOR_H

#include <QObject>
#include <QApplication>
#include <QTest>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QWidget>

#include "config-button-animator.h"

using namespace UKUIConfigStyleSpace;

class TestConfigButtonAnimator : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_bindWidget();
    void test_bindWidgetWithDoNotAnimate();
    void test_bindWidgetWithUnsupportedWidget();
    void test_unboundWidget();
    void test_unboundWidgetWithoutBind();
    void test_boundedWidget();
    void test_value();
    void test_valueInvalidProperty();
    void test_setAnimatorStartValue();
    void test_setAnimatorStartValueInvalid();
    void test_setAnimatorEndValue();
    void test_setAnimatorEndValueInvalid();
    void test_setAnimatorDuration();
    void test_setAnimatorDurationInvalid();
    void test_setAnimatorDirectionForward();
    void test_setAnimatorDirectionForwardDefault();
    void test_setAnimatorDirectionForwardInvalid();
    void test_isRunning();
    void test_isRunningDefault();
    void test_isRunningInvalid();
    void test_startAnimator();
    void test_startAnimatorDefault();
    void test_startAnimatorInvalid();
    void test_stopAnimator();
    void test_stopAnimatorDefault();
    void test_stopAnimatorInvalid();
    void test_currentAnimatorTime();
    void test_currentAnimatorTimeDefault();
    void test_currentAnimatorTimeInvalid();
    void test_setAnimatorCurrentTime();
    void test_setAnimatorCurrentTimeDefault();
    void test_setAnimatorCurrentTimeInvalid();
    void test_totalAnimationDuration();
    void test_totalAnimationDurationDefault();
    void test_totalAnimationDurationInvalid();
    void test_endValue();
    void test_endValueInvalid();

private:
    ConfigButtonAnimator *m_animator;
    QToolButton *m_toolButton;
    QPushButton *m_pushButton;
    QComboBox *m_comboBox;
    QSpinBox *m_spinBox;
    QDoubleSpinBox *m_doubleSpinBox;
    QWidget *m_plainWidget;
};

#endif // TEST_CONFIG_BUTTON_ANIMATOR_H
