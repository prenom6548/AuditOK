#include "test_config-checkbox-animation-helper.h"
#include <QCheckBox>
#include <QTest>

using namespace UKUIConfigStyleSpace;

void TestConfigCheckBoxAnimationHelper::initTestCase()
{}

void TestConfigCheckBoxAnimationHelper::cleanupTestCase()
{}

void TestConfigCheckBoxAnimationHelper::test_constructor()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QVERIFY(helper != nullptr);
    delete helper;
}

void TestConfigCheckBoxAnimationHelper::test_destructor()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QVERIFY(helper != nullptr);
    delete helper;
}

void TestConfigCheckBoxAnimationHelper::test_registerWidget()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QCheckBox *widget = new QCheckBox();
    widget->setObjectName("testCheckboxWidget");

    bool result = helper->registerWidget(widget);
    QVERIFY(result == true);

    AnimatorIface *anim = helper->animator(widget);
    QVERIFY(anim != nullptr);

    helper->unregisterWidget(widget);
    delete widget;
    delete helper;
}

void TestConfigCheckBoxAnimationHelper::test_registerWidget_fail()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QWidget *widget = nullptr;

    bool result = helper->registerWidget(widget);
    QVERIFY(result == false);

    delete helper;
}

void TestConfigCheckBoxAnimationHelper::test_unregisterWidget()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QCheckBox *widget = new QCheckBox();
    widget->setObjectName("testCheckboxWidget2");

    bool regResult = helper->registerWidget(widget);
    QVERIFY(regResult == true);

    bool unregResult = helper->unregisterWidget(widget);
    QVERIFY(unregResult == true);

    delete widget;
    delete helper;
}

void TestConfigCheckBoxAnimationHelper::test_animator()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QCheckBox *widget = new QCheckBox();
    widget->setObjectName("testCheckboxWidget3");

    bool regResult = helper->registerWidget(widget);
    QVERIFY(regResult == true);

    AnimatorIface *anim = helper->animator(widget);
    QVERIFY(anim != nullptr);

    helper->unregisterWidget(widget);
    delete widget;
    delete helper;
}

void TestConfigCheckBoxAnimationHelper::test_animator_null()
{
    ConfigCheckBoxAnimationHelper *helper = new ConfigCheckBoxAnimationHelper();
    QWidget *widget = new QWidget();
    widget->setObjectName("testCheckboxWidget4");

    AnimatorIface *anim = helper->animator(widget);
    QVERIFY(anim == nullptr);

    delete widget;
    delete helper;
}
