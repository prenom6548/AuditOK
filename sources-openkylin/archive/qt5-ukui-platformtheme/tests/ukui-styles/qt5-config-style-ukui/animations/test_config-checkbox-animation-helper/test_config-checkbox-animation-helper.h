#ifndef TEST_CONFIG_CHECKBOX_ANIMATION_HELPER_H
#define TEST_CONFIG_CHECKBOX_ANIMATION_HELPER_H

#include <QObject>
#include <QWidget>
#include "config-checkbox-animation-helper.h"

class TestConfigCheckBoxAnimationHelper : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_constructor();
    void test_destructor();
    void test_registerWidget();
    void test_registerWidget_fail();
    void test_unregisterWidget();
    void test_animator();
    void test_animator_null();
};

#endif // TEST_CONFIG_CHECKBOX_ANIMATION_HELPER_H
