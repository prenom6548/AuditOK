#include "test_config-radiobutton-animator.h"

TestConfigRadioButtonAnimator::TestConfigRadioButtonAnimator(QObject *parent)
    : QObject(parent)
    , m_animator(nullptr)
    , m_radioButton(nullptr)
{
}

void TestConfigRadioButtonAnimator::initTestCase()
{
    m_animator = new UKUIConfigStyleSpace::ConfigRadioButtonAnimator();
    m_radioButton = new QRadioButton();
}

void TestConfigRadioButtonAnimator::cleanupTestCase()
{
    delete m_animator;
    m_animator = nullptr;
    delete m_radioButton;
    m_radioButton = nullptr;
}

void TestConfigRadioButtonAnimator::init()
{
}

void TestConfigRadioButtonAnimator::cleanup()
{
}

void TestConfigRadioButtonAnimator::testConstructor()
{
    QVERIFY(m_animator != nullptr);
    QVERIFY(m_animator->animationCount() == 0);
}

void TestConfigRadioButtonAnimator::testDestructor()
{
    UKUIConfigStyleSpace::ConfigRadioButtonAnimator *animator = new UKUIConfigStyleSpace::ConfigRadioButtonAnimator();
    delete animator;
}

void TestConfigRadioButtonAnimator::testBindWidget()
{
    bool result = m_animator->bindWidget(m_radioButton);
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() == m_radioButton);
    QVERIFY(m_animator->animationCount() == 5);
}

void TestConfigRadioButtonAnimator::testBindWidgetWithDoNotAnimateProperty()
{
    UKUIConfigStyleSpace::ConfigRadioButtonAnimator *animator2 = new UKUIConfigStyleSpace::ConfigRadioButtonAnimator();
    QRadioButton *radioBtn = new QRadioButton();
    radioBtn->setProperty("doNotAnimate", true);
    bool result = animator2->bindWidget(radioBtn);
    QVERIFY(result == false);
    QVERIFY(animator2->animationCount() == 0);
    delete animator2;
    delete radioBtn;
}

void TestConfigRadioButtonAnimator::testBindWidgetWithNonRadioButton()
{
    UKUIConfigStyleSpace::ConfigRadioButtonAnimator *animator2 = new UKUIConfigStyleSpace::ConfigRadioButtonAnimator();
    QWidget *widget = new QWidget();
    bool result = animator2->bindWidget(widget);
    QVERIFY(result == false);
    QVERIFY(animator2->animationCount() == 0);
    delete animator2;
    delete widget;
}

void TestConfigRadioButtonAnimator::testUnboundWidget()
{
    m_animator->bindWidget(m_radioButton);
    bool result = m_animator->unboundWidget();
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() != nullptr); // boundedWidget still returns old widget after unbound
}

void TestConfigRadioButtonAnimator::testValue()
{
    m_animator->bindWidget(m_radioButton);

    QVariant value1 = m_animator->value("SunKenOn");
    QVERIFY(value1.isValid());

    QVariant value2 = m_animator->value("Off");
    QVERIFY(value2.isValid());

    QVariant value3 = m_animator->value("OffHover");
    QVERIFY(value3.isValid());

    QVariant value4 = m_animator->value("OnHover");
    QVERIFY(value4.isValid());

    QVariant value5 = m_animator->value("SunkenOnHover");
    QVERIFY(value5.isValid());

    QVariant value6 = m_animator->value("Unknown");
    QVERIFY(!value6.isValid());
}

void TestConfigRadioButtonAnimator::testSetAnimatorStartValue()
{
    m_animator->bindWidget(m_radioButton);

    bool result1 = m_animator->setAnimatorStartValue("SunKenOn", 0.5);
    QVERIFY(result1 == true);

    bool result2 = m_animator->setAnimatorStartValue("Off", 0.8);
    QVERIFY(result2 == true);

    bool result3 = m_animator->setAnimatorStartValue("OffHover", 0.3);
    QVERIFY(result3 == true);

    bool result4 = m_animator->setAnimatorStartValue("OnHover", 1.2);
    QVERIFY(result4 == true);

    bool result5 = m_animator->setAnimatorStartValue("SunkenOnHover", 0.9);
    QVERIFY(result5 == true);

    bool result6 = m_animator->setAnimatorStartValue("Unknown", 0.5);
    QVERIFY(result6 == false);
}

void TestConfigRadioButtonAnimator::testSetAnimatorEndValue()
{
    m_animator->bindWidget(m_radioButton);

    bool result1 = m_animator->setAnimatorEndValue("SunKenOn", 0.9);
    QVERIFY(result1 == true);

    bool result2 = m_animator->setAnimatorEndValue("Off", 0.2);
    QVERIFY(result2 == true);

    bool result3 = m_animator->setAnimatorEndValue("OffHover", 0.7);
    QVERIFY(result3 == true);

    bool result4 = m_animator->setAnimatorEndValue("OnHover", 1.8);
    QVERIFY(result4 == true);

    bool result5 = m_animator->setAnimatorEndValue("SunkenOnHover", 1.5);
    QVERIFY(result5 == true);

    bool result6 = m_animator->setAnimatorEndValue("Unknown", 0.5);
    QVERIFY(result6 == false);
}

void TestConfigRadioButtonAnimator::testSetAnimatorDuration()
{
    m_animator->bindWidget(m_radioButton);

    bool result1 = m_animator->setAnimatorDuration("SunKenOn", 300);
    QVERIFY(result1 == true);

    bool result2 = m_animator->setAnimatorDuration("Off", 150);
    QVERIFY(result2 == true);

    bool result3 = m_animator->setAnimatorDuration("OffHover", 250);
    QVERIFY(result3 == true);

    bool result4 = m_animator->setAnimatorDuration("OnHover", 350);
    QVERIFY(result4 == true);

    bool result5 = m_animator->setAnimatorDuration("SunkenOnHover", 400);
    QVERIFY(result5 == true);

    bool result6 = m_animator->setAnimatorDuration("Unknown", 200);
    QVERIFY(result6 == false);
}

void TestConfigRadioButtonAnimator::testSetAnimatorDirectionForward()
{
    m_animator->bindWidget(m_radioButton);

    m_animator->setAnimatorDirectionForward("SunKenOn", true);
    m_animator->setAnimatorDirectionForward("Off", false);
    m_animator->setAnimatorDirectionForward("OffHover", true);
    m_animator->setAnimatorDirectionForward("OnHover", false);
    m_animator->setAnimatorDirectionForward("SunkenOnHover", true);

    m_animator->setAnimatorDirectionForward("Unknown", true);
}

void TestConfigRadioButtonAnimator::testIsRunning()
{
    m_animator->bindWidget(m_radioButton);

    QVERIFY(m_animator->isRunning("SunKenOn") == false);
    QVERIFY(m_animator->isRunning("Off") == false);
    QVERIFY(m_animator->isRunning("OffHover") == false);
    QVERIFY(m_animator->isRunning("OnHover") == false);
    QVERIFY(m_animator->isRunning("SunkenOnHover") == false);

    QVERIFY(m_animator->isRunning("Unknown") == false);
}

void TestConfigRadioButtonAnimator::testStartAnimator()
{
    m_animator->bindWidget(m_radioButton);

    m_animator->startAnimator("SunKenOn");
    QTest::qWait(50);
    QVERIFY(m_animator->isRunning("SunKenOn") || m_animator->state() == QAbstractAnimation::Running);
    m_animator->stopAnimator("SunKenOn");

    m_animator->startAnimator("Off");
    QTest::qWait(50);
    m_animator->stopAnimator("Off");

    m_animator->startAnimator("OffHover");
    QTest::qWait(50);
    m_animator->stopAnimator("OffHover");

    m_animator->startAnimator("OnHover");
    QTest::qWait(50);
    m_animator->stopAnimator("OnHover");

    m_animator->startAnimator("SunkenOnHover");
    QTest::qWait(50);
    m_animator->stopAnimator("SunkenOnHover");

    m_animator->startAnimator("Unknown");
}

void TestConfigRadioButtonAnimator::testStopAnimator()
{
    m_animator->bindWidget(m_radioButton);

    m_animator->startAnimator("SunKenOn");
    m_animator->stopAnimator("SunKenOn");
    QVERIFY(m_animator->isStopped("SunKenOn") == true);

    m_animator->startAnimator("Off");
    m_animator->stopAnimator("Off");
    QVERIFY(m_animator->isStopped("Off") == true);

    m_animator->startAnimator("OffHover");
    m_animator->stopAnimator("OffHover");
    QVERIFY(m_animator->isStopped("OffHover") == true);

    m_animator->startAnimator("OnHover");
    m_animator->stopAnimator("OnHover");
    QVERIFY(m_animator->isStopped("OnHover") == true);

    m_animator->startAnimator("SunkenOnHover");
    m_animator->stopAnimator("SunkenOnHover");
    QVERIFY(m_animator->isStopped("SunkenOnHover") == true);

    m_animator->startAnimator();
    m_animator->stopAnimator("Unknown");
    QVERIFY(m_animator->isStopped("Unknown") == true);
}

void TestConfigRadioButtonAnimator::testCurrentAnimatorTime()
{
    m_animator->bindWidget(m_radioButton);

    int time1 = m_animator->currentAnimatorTime("SunKenOn");
    QVERIFY(time1 >= 0);

    int time2 = m_animator->currentAnimatorTime("Off");
    QVERIFY(time2 >= 0);

    int time3 = m_animator->currentAnimatorTime("OffHover");
    QVERIFY(time3 >= 0);

    int time4 = m_animator->currentAnimatorTime("OnHover");
    QVERIFY(time4 >= 0);

    int time5 = m_animator->currentAnimatorTime("SunkenOnHover");
    QVERIFY(time5 >= 0);

    int time6 = m_animator->currentAnimatorTime("Unknown");
    QVERIFY(time6 >= 0);
}

void TestConfigRadioButtonAnimator::testSetAnimatorCurrentTime()
{
    m_animator->bindWidget(m_radioButton);

    m_animator->setAnimatorCurrentTime("SunKenOn", 100);
    m_animator->setAnimatorCurrentTime("Off", 50);
    m_animator->setAnimatorCurrentTime("OffHover", 75);
    m_animator->setAnimatorCurrentTime("OnHover", 125);
    m_animator->setAnimatorCurrentTime("SunkenOnHover", 150);
    m_animator->setAnimatorCurrentTime("Unknown", 100);
}

void TestConfigRadioButtonAnimator::testTotalAnimationDuration()
{
    m_animator->bindWidget(m_radioButton);

    int duration1 = m_animator->totalAnimationDuration("SunKenOn");
    QVERIFY(duration1 == 200);

    int duration2 = m_animator->totalAnimationDuration("Off");
    QVERIFY(duration2 == 100);

    int duration3 = m_animator->totalAnimationDuration("OffHover");
    QVERIFY(duration3 == 200);

    int duration4 = m_animator->totalAnimationDuration("OnHover");
    QVERIFY(duration4 == 200);

    int duration5 = m_animator->totalAnimationDuration("SunkenOnHover");
    QVERIFY(duration5 == 200);

    int duration6 = m_animator->totalAnimationDuration("Unknown");
    QVERIFY(duration6 >= 0); // Unknown property returns this->duration()
}

void TestConfigRadioButtonAnimator::testIsStopped()
{
    m_animator->bindWidget(m_radioButton);

    QVERIFY(m_animator->isStopped("SunKenOn") == true);
    QVERIFY(m_animator->isStopped("Off") == true);
    QVERIFY(m_animator->isStopped("OffHover") == true);
    QVERIFY(m_animator->isStopped("OnHover") == true);
    QVERIFY(m_animator->isStopped("SunkenOnHover") == true);

    QVERIFY(m_animator->isStopped("Unknown") == true);
}

QTEST_MAIN(TestConfigRadioButtonAnimator)
#include "test_config-radiobutton-animator.moc"
