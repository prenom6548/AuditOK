#include "test_config-checkbox-animator.h"

void TestConfigCheckBoxAnimator::initTestCase()
{
    m_animator = nullptr;
    m_checkBox = nullptr;
}

void TestConfigCheckBoxAnimator::cleanupTestCase()
{
}

void TestConfigCheckBoxAnimator::init()
{
    m_animator = new ConfigCheckBoxAnimator();
    m_checkBox = new QCheckBox();
}

void TestConfigCheckBoxAnimator::cleanup()
{
    if (m_animator) {
        delete m_animator;
        m_animator = nullptr;
    }
    if (m_checkBox) {
        delete m_checkBox;
        m_checkBox = nullptr;
    }
}

void TestConfigCheckBoxAnimator::testConstructor()
{
    ConfigCheckBoxAnimator *animator = new ConfigCheckBoxAnimator();
    QVERIFY(animator != nullptr);
    delete animator;
}

void TestConfigCheckBoxAnimator::testDestructor()
{
    ConfigCheckBoxAnimator *animator = new ConfigCheckBoxAnimator();
    animator->bindWidget(m_checkBox);
    delete animator;
    QTest::qWait(10);
}

void TestConfigCheckBoxAnimator::testBindWidget()
{
    bool result = m_animator->bindWidget(m_checkBox);
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() == m_checkBox);
}

void TestConfigCheckBoxAnimator::testBindWidgetWithDoNotAnimate()
{
    m_checkBox->setProperty("doNotAnimate", true);
    bool result = m_animator->bindWidget(m_checkBox);
    QVERIFY(result == false);
    m_checkBox->setProperty("doNotAnimate", false);
}

void TestConfigCheckBoxAnimator::testBindWidgetWithNonCheckbox()
{
    QWidget *widget = new QWidget();
    bool result = m_animator->bindWidget(widget);
    QVERIFY(result == false);
    delete widget;
}

void TestConfigCheckBoxAnimator::testUnboundWidget()
{
    m_animator->bindWidget(m_checkBox);
    bool result = m_animator->unboundWidget();
    QVERIFY(result == true);
}

void TestConfigCheckBoxAnimator::testUnboundWidgetWithoutBind()
{
    bool result = m_animator->unboundWidget();
    QVERIFY(result == false);
}

void TestConfigCheckBoxAnimator::testBoundedWidget()
{
    m_animator->bindWidget(m_checkBox);
    QWidget *bounded = m_animator->boundedWidget();
    QVERIFY(bounded == m_checkBox);
}

void TestConfigCheckBoxAnimator::testValue()
{
    m_animator->bindWidget(m_checkBox);
    QVariant valueOnScale = m_animator->value("OnScale");
    QVERIFY(valueOnScale.isValid());
    QVariant valueOnOpacity = m_animator->value("OnOpacity");
    QVERIFY(valueOnOpacity.isValid());
    QVariant valueOnBase = m_animator->value("OnBase");
    QVERIFY(valueOnBase.isValid());
    QVariant valueOff = m_animator->value("Off");
    QVERIFY(valueOff.isValid());
}

void TestConfigCheckBoxAnimator::testValueInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    QVariant valueInvalid = m_animator->value("InvalidProperty");
    QVERIFY(!valueInvalid.isValid());
}

void TestConfigCheckBoxAnimator::testIsRunning()
{
    m_animator->bindWidget(m_checkBox);
    bool runningOnScale = m_animator->isRunning("OnScale");
    QVERIFY(runningOnScale == false);
    bool runningOnOpacity = m_animator->isRunning("OnOpacity");
    QVERIFY(runningOnOpacity == false);
    bool runningOnBase = m_animator->isRunning("OnBase");
    QVERIFY(runningOnBase == false);
    bool runningOff = m_animator->isRunning("Off");
    QVERIFY(runningOff == false);
}

void TestConfigCheckBoxAnimator::testIsRunningInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    bool runningInvalid = m_animator->isRunning("InvalidProperty");
    QVERIFY(runningInvalid == false);
}

void TestConfigCheckBoxAnimator::testSetAnimatorStartValue()
{
    m_animator->bindWidget(m_checkBox);
    bool resultOnScale = m_animator->setAnimatorStartValue("OnScale", 0.5);
    QVERIFY(resultOnScale == true);
    bool resultOnOpacity = m_animator->setAnimatorStartValue("OnOpacity", 0.3);
    QVERIFY(resultOnOpacity == true);
    bool resultOnBase = m_animator->setAnimatorStartValue("OnBase", 0.2);
    QVERIFY(resultOnBase == true);
    bool resultOff = m_animator->setAnimatorStartValue("Off", 0.8);
    QVERIFY(resultOff == true);
}

void TestConfigCheckBoxAnimator::testSetAnimatorStartValueInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    bool resultInvalid = m_animator->setAnimatorStartValue("InvalidProperty", 0.5);
    QVERIFY(resultInvalid == false);
}

void TestConfigCheckBoxAnimator::testSetAnimatorEndValue()
{
    m_animator->bindWidget(m_checkBox);
    bool resultOnScale = m_animator->setAnimatorEndValue("OnScale", 0.8);
    QVERIFY(resultOnScale == true);
    bool resultOnOpacity = m_animator->setAnimatorEndValue("OnOpacity", 0.6);
    QVERIFY(resultOnOpacity == true);
    bool resultOnBase = m_animator->setAnimatorEndValue("OnBase", 0.9);
    QVERIFY(resultOnBase == true);
    bool resultOff = m_animator->setAnimatorEndValue("Off", 0.1);
    QVERIFY(resultOff == true);
}

void TestConfigCheckBoxAnimator::testSetAnimatorEndValueInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    bool resultInvalid = m_animator->setAnimatorEndValue("InvalidProperty", 0.5);
    QVERIFY(resultInvalid == false);
}

void TestConfigCheckBoxAnimator::testSetAnimatorDuration()
{
    m_animator->bindWidget(m_checkBox);
    bool resultOnScale = m_animator->setAnimatorDuration("OnScale", 200);
    QVERIFY(resultOnScale == true);
    bool resultOnOpacity = m_animator->setAnimatorDuration("OnOpacity", 150);
    QVERIFY(resultOnOpacity == true);
    bool resultOnBase = m_animator->setAnimatorDuration("OnBase", 180);
    QVERIFY(resultOnBase == true);
    bool resultOff = m_animator->setAnimatorDuration("Off", 100);
    QVERIFY(resultOff == true);
}

void TestConfigCheckBoxAnimator::testSetAnimatorDurationInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    bool resultInvalid = m_animator->setAnimatorDuration("InvalidProperty", 100);
    QVERIFY(resultInvalid == false);
}

void TestConfigCheckBoxAnimator::testSetAnimatorDirectionForward()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->setAnimatorDirectionForward("OnScale", true);
    m_animator->setAnimatorDirectionForward("OnOpacity", false);
    m_animator->setAnimatorDirectionForward("OnBase", true);
    m_animator->setAnimatorDirectionForward("Off", false);
    m_animator->setAnimatorDirectionForward("InvalidProperty", true);
    QTest::qWait(10);
}

void TestConfigCheckBoxAnimator::testSetAnimatorDirectionForwardInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->setAnimatorDirectionForward("InvalidProperty", true);
    QTest::qWait(10);
}

void TestConfigCheckBoxAnimator::testStartAnimator()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->startAnimator("OnScale");
    QTest::qWait(50);
    m_animator->startAnimator("OnOpacity");
    QTest::qWait(50);
    m_animator->startAnimator("OnBase");
    QTest::qWait(50);
    m_animator->startAnimator("Off");
    QTest::qWait(50);
    m_animator->startAnimator("InvalidProperty");
}

void TestConfigCheckBoxAnimator::testStartAnimatorInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->startAnimator("InvalidProperty");
    QTest::qWait(10);
}

void TestConfigCheckBoxAnimator::testStopAnimator()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->startAnimator("OnScale");
    QTest::qWait(10);
    m_animator->stopAnimator("OnScale");
    m_animator->startAnimator("OnOpacity");
    QTest::qWait(10);
    m_animator->stopAnimator("OnOpacity");
    m_animator->startAnimator("OnBase");
    QTest::qWait(10);
    m_animator->stopAnimator("OnBase");
    m_animator->startAnimator("Off");
    QTest::qWait(10);
    m_animator->stopAnimator("Off");
    m_animator->stopAnimator("InvalidProperty");
}

void TestConfigCheckBoxAnimator::testStopAnimatorInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->stopAnimator("InvalidProperty");
}

void TestConfigCheckBoxAnimator::testCurrentAnimatorTime()
{
    m_animator->bindWidget(m_checkBox);
    int timeOnScale = m_animator->currentAnimatorTime("OnScale");
    QVERIFY(timeOnScale >= 0);
    int timeOnOpacity = m_animator->currentAnimatorTime("OnOpacity");
    QVERIFY(timeOnOpacity >= 0);
    int timeOnBase = m_animator->currentAnimatorTime("OnBase");
    QVERIFY(timeOnBase >= 0);
    int timeOff = m_animator->currentAnimatorTime("Off");
    QVERIFY(timeOff >= 0);
}

void TestConfigCheckBoxAnimator::testCurrentAnimatorTimeInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    int timeInvalid = m_animator->currentAnimatorTime("InvalidProperty");
    QVERIFY(timeInvalid >= 0);
}

void TestConfigCheckBoxAnimator::testSetAnimatorCurrentTime()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->setAnimatorCurrentTime("OnScale", 50);
    m_animator->setAnimatorCurrentTime("OnOpacity", 30);
    m_animator->setAnimatorCurrentTime("OnBase", 20);
    m_animator->setAnimatorCurrentTime("Off", 10);
    QTest::qWait(10);
}

void TestConfigCheckBoxAnimator::testSetAnimatorCurrentTimeInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->setAnimatorCurrentTime("InvalidProperty", 50);
    QTest::qWait(10);
}

void TestConfigCheckBoxAnimator::testTotalAnimationDuration()
{
    m_animator->bindWidget(m_checkBox);
    int durationOnScale = m_animator->totalAnimationDuration("OnScale");
    QVERIFY(durationOnScale >= 0);
    int durationOnOpacity = m_animator->totalAnimationDuration("OnOpacity");
    QVERIFY(durationOnOpacity >= 0);
    int durationOnBase = m_animator->totalAnimationDuration("OnBase");
    QVERIFY(durationOnBase >= 0);
    int durationOff = m_animator->totalAnimationDuration("Off");
    QVERIFY(durationOff >= 0);
}

void TestConfigCheckBoxAnimator::testTotalAnimationDurationInvalidProperty()
{
    m_animator->bindWidget(m_checkBox);
    int durationInvalid = m_animator->totalAnimationDuration("InvalidProperty");
    QVERIFY(durationInvalid >= 0);
}

void TestConfigCheckBoxAnimator::testSetExtraProperty()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->setExtraProperty("nochange", true);
    m_animator->setExtraProperty("nochange", false);
}

void TestConfigCheckBoxAnimator::testGetExtraProperty()
{
    m_animator->bindWidget(m_checkBox);
    m_animator->setExtraProperty("nochange", true);
    QVariant value = m_animator->getExtraProperty("nochange");
    QVERIFY(value.toBool() == true);
    QVariant invalidValue = m_animator->getExtraProperty("InvalidProperty");
    QVERIFY(!invalidValue.isValid());
}

QTEST_MAIN(TestConfigCheckBoxAnimator)

#include "test_config-checkbox-animator.moc"
