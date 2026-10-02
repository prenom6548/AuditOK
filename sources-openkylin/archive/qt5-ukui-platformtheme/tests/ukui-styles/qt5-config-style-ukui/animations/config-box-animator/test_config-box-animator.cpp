#include "test_config-box-animator.h"

void TestConfigBoxAnimator::initTestCase()
{
    m_animator = nullptr;
    m_comboBox = nullptr;
}

void TestConfigBoxAnimator::cleanupTestCase()
{
}

void TestConfigBoxAnimator::init()
{
    m_animator = new ConfigBoxAnimator();
    m_comboBox = new QComboBox();
}

void TestConfigBoxAnimator::cleanup()
{
    if (m_animator) {
        delete m_animator;
        m_animator = nullptr;
    }
    if (m_comboBox) {
        delete m_comboBox;
        m_comboBox = nullptr;
    }
}

void TestConfigBoxAnimator::testConstructor()
{
    ConfigBoxAnimator *animator = new ConfigBoxAnimator();
    QVERIFY(animator != nullptr);
    QVERIFY(animator->property("doNotAnimate").toBool() == false);
    delete animator;
}

void TestConfigBoxAnimator::testDestructor()
{
    ConfigBoxAnimator *animator = new ConfigBoxAnimator();
    animator->bindWidget(m_comboBox);
    delete animator;
    QTest::qWait(10);
}

void TestConfigBoxAnimator::testBindWidget()
{
    bool result = m_animator->bindWidget(m_comboBox);
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() == m_comboBox);
}

void TestConfigBoxAnimator::testBindWidgetWithDoNotAnimate()
{
    m_comboBox->setProperty("doNotAnimate", true);
    bool result = m_animator->bindWidget(m_comboBox);
    QVERIFY(result == false);
    m_comboBox->setProperty("doNotAnimate", false);
}

void TestConfigBoxAnimator::testBindWidgetNonComboBox()
{
    QWidget *widget = new QWidget();
    bool result = m_animator->bindWidget(widget);
    QVERIFY(result == false);
    delete widget;
}

void TestConfigBoxAnimator::testUnboundWidget()
{
    m_animator->bindWidget(m_comboBox);
    bool result = m_animator->unboundWidget();
    QVERIFY(result == true);
}

void TestConfigBoxAnimator::testValue()
{
    m_animator->bindWidget(m_comboBox);
    QVariant valueMouseOver = m_animator->value("MouseOver");
    QVERIFY(valueMouseOver.isValid());
    QVariant valueSunken = m_animator->value("SunKen");
    QVERIFY(valueSunken.isValid());
    QVariant valueInvalid = m_animator->value("InvalidProperty");
    QVERIFY(!valueInvalid.isValid());
}

void TestConfigBoxAnimator::testIsRunning()
{
    m_animator->bindWidget(m_comboBox);
    bool runningMouseOver = m_animator->isRunning("MouseOver");
    QVERIFY(runningMouseOver == false);
    bool runningSunken = m_animator->isRunning("SunKen");
    QVERIFY(runningSunken == false);
    bool runningInvalid = m_animator->isRunning("InvalidProperty");
    QVERIFY(runningInvalid == false);
}

void TestConfigBoxAnimator::testSetAnimatorStartValue()
{
    m_animator->bindWidget(m_comboBox);
    bool resultMouseOver = m_animator->setAnimatorStartValue("MouseOver", 0.5);
    QVERIFY(resultMouseOver == true);
    bool resultSunken = m_animator->setAnimatorStartValue("SunKen", 0.3);
    QVERIFY(resultSunken == true);
    bool resultInvalid = m_animator->setAnimatorStartValue("InvalidProperty", 0.5);
    QVERIFY(resultInvalid == false);
}

void TestConfigBoxAnimator::testSetAnimatorEndValue()
{
    m_animator->bindWidget(m_comboBox);
    bool resultMouseOver = m_animator->setAnimatorEndValue("MouseOver", 0.8);
    QVERIFY(resultMouseOver == true);
    bool resultSunken = m_animator->setAnimatorEndValue("SunKen", 0.6);
    QVERIFY(resultSunken == true);
    bool resultInvalid = m_animator->setAnimatorEndValue("InvalidProperty", 0.5);
    QVERIFY(resultInvalid == false);
}

void TestConfigBoxAnimator::testSetAnimatorDuration()
{
    m_animator->bindWidget(m_comboBox);
    bool resultMouseOver = m_animator->setAnimatorDuration("MouseOver", 200);
    QVERIFY(resultMouseOver == true);
    bool resultSunken = m_animator->setAnimatorDuration("SunKen", 150);
    QVERIFY(resultSunken == true);
    bool resultInvalid = m_animator->setAnimatorDuration("InvalidProperty", 100);
    QVERIFY(resultInvalid == false);
}

void TestConfigBoxAnimator::testSetAnimatorDirectionForward()
{
    m_animator->bindWidget(m_comboBox);
    m_animator->setAnimatorDirectionForward("MouseOver", true);
    m_animator->setAnimatorDirectionForward("SunKen", false);
    m_animator->setAnimatorDirectionForward("InvalidProperty", true);
    QTest::qWait(10);
}

void TestConfigBoxAnimator::testStartAnimator()
{
    m_animator->bindWidget(m_comboBox);
    m_animator->startAnimator("MouseOver");
    QTest::qWait(50);
    m_animator->startAnimator("SunKen");
    QTest::qWait(50);
    m_animator->startAnimator("InvalidProperty");
}

void TestConfigBoxAnimator::testStopAnimator()
{
    m_animator->bindWidget(m_comboBox);
    m_animator->startAnimator("MouseOver");
    QTest::qWait(10);
    m_animator->stopAnimator("MouseOver");
    m_animator->startAnimator("SunKen");
    QTest::qWait(10);
    m_animator->stopAnimator("SunKen");
    m_animator->stopAnimator("InvalidProperty");
}

void TestConfigBoxAnimator::testCurrentAnimatorTime()
{
    m_animator->bindWidget(m_comboBox);
    int timeMouseOver = m_animator->currentAnimatorTime("MouseOver");
    QVERIFY(timeMouseOver >= 0);
    int timeSunken = m_animator->currentAnimatorTime("SunKen");
    QVERIFY(timeSunken >= 0);
    int timeInvalid = m_animator->currentAnimatorTime("InvalidProperty");
    QVERIFY(timeInvalid >= 0);
}

void TestConfigBoxAnimator::testTotalAnimationDuration()
{
    m_animator->bindWidget(m_comboBox);
    int durationMouseOver = m_animator->totalAnimationDuration("MouseOver");
    QVERIFY(durationMouseOver >= 0);
    int durationSunken = m_animator->totalAnimationDuration("SunKen");
    QVERIFY(durationSunken >= 0);
    int durationInvalid = m_animator->totalAnimationDuration("InvalidProperty");
    QVERIFY(durationInvalid >= 0);
}

QTEST_MAIN(TestConfigBoxAnimator)

#include "test_config-box-animator.moc"
