#include "test_config-slider-animator.h"

void TestConfigSliderAnimator::initTestCase()
{
    m_animator = nullptr;
    m_slider = nullptr;
}

void TestConfigSliderAnimator::cleanupTestCase()
{
}

void TestConfigSliderAnimator::init()
{
    m_animator = new ConfigSliderAnimator();
    m_slider = new QSlider();
}

void TestConfigSliderAnimator::cleanup()
{
    if (m_animator) {
        delete m_animator;
        m_animator = nullptr;
    }
    if (m_slider) {
        delete m_slider;
        m_slider = nullptr;
    }
}

void TestConfigSliderAnimator::testConstructor()
{
    ConfigSliderAnimator *animator = new ConfigSliderAnimator();
    QVERIFY(animator != nullptr);
    delete animator;
}

void TestConfigSliderAnimator::testDestructor()
{
    ConfigSliderAnimator *animator = new ConfigSliderAnimator();
    animator->bindWidget(m_slider);
    delete animator;
    QTest::qWait(10);
}

void TestConfigSliderAnimator::testBindWidget()
{
    bool result = m_animator->bindWidget(m_slider);
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() == m_slider);
}

void TestConfigSliderAnimator::testBindWidgetWithDoNotAnimate()
{
    m_slider->setProperty("doNotAnimate", true);
    bool result = m_animator->bindWidget(m_slider);
    QVERIFY(result == false);
    m_slider->setProperty("doNotAnimate", false);
}

void TestConfigSliderAnimator::testBindWidgetNonSlider()
{
    QWidget *widget = new QWidget();
    bool result = m_animator->bindWidget(widget);
    QVERIFY(result == false);
    delete widget;
}

void TestConfigSliderAnimator::testUnboundWidget()
{
    m_animator->bindWidget(m_slider);
    bool result = m_animator->unboundWidget();
    QVERIFY(result == true);
}

void TestConfigSliderAnimator::testBoundedWidget()
{
    m_animator->bindWidget(m_slider);
    QWidget *bounded = m_animator->boundedWidget();
    QVERIFY(bounded == m_slider);
}

void TestConfigSliderAnimator::testValue()
{
    m_animator->bindWidget(m_slider);
    QVariant valueSliderOpacity = m_animator->value("slider_opacity");
    QVERIFY(valueSliderOpacity.isValid());
    QVariant valueAdditionalOpacity = m_animator->value("additional_opacity");
    QVERIFY(valueAdditionalOpacity.isValid());
    QVariant valueMovePosition = m_animator->value("move_position");
    QVERIFY(valueMovePosition.isValid());
    QVariant valueBgWidth = m_animator->value("bg_width");
    QVERIFY(valueBgWidth.isValid());
    QVariant valueHoverZoomin = m_animator->value("hover_zoomin");
    QVERIFY(valueHoverZoomin.isValid());
    QVariant valueClickZoomin = m_animator->value("click_zoomin");
    QVERIFY(valueClickZoomin.isValid());
    QVariant valueInvalid = m_animator->value("InvalidProperty");
    QVERIFY(!valueInvalid.isValid());
}

void TestConfigSliderAnimator::testIsRunning()
{
    m_animator->bindWidget(m_slider);
    bool runningSliderOpacity = m_animator->isRunning("slider_opacity");
    QVERIFY(runningSliderOpacity == false);
    bool runningAdditionalOpacity = m_animator->isRunning("additional_opacity");
    QVERIFY(runningAdditionalOpacity == false);
    bool runningMovePosition = m_animator->isRunning("move_position");
    QVERIFY(runningMovePosition == false);
    bool runningInvalid = m_animator->isRunning("InvalidProperty");
    QVERIFY(runningInvalid == false);
}

void TestConfigSliderAnimator::testSetAnimatorStartValue()
{
    m_animator->bindWidget(m_slider);
    bool resultSliderOpacity = m_animator->setAnimatorStartValue("slider_opacity", 0.3);
    QVERIFY(resultSliderOpacity == true);
    bool resultAdditionalOpacity = m_animator->setAnimatorStartValue("additional_opacity", 0.1);
    QVERIFY(resultAdditionalOpacity == true);
    bool resultMovePosition = m_animator->setAnimatorStartValue("move_position", 0.2);
    QVERIFY(resultMovePosition == true);
    bool resultBgWidth = m_animator->setAnimatorStartValue("bg_width", 0.4);
    QVERIFY(resultBgWidth == true);
    bool resultHoverZoomin = m_animator->setAnimatorStartValue("hover_zoomin", 1.0);
    QVERIFY(resultHoverZoomin == true);
    bool resultClickZoomin = m_animator->setAnimatorStartValue("click_zoomin", 0.0);
    QVERIFY(resultClickZoomin == true);
    bool resultInvalid = m_animator->setAnimatorStartValue("InvalidProperty", 0.5);
    QVERIFY(resultInvalid == false);
}

void TestConfigSliderAnimator::testSetAnimatorEndValue()
{
    m_animator->bindWidget(m_slider);
    bool resultSliderOpacity = m_animator->setAnimatorEndValue("slider_opacity", 0.8);
    QVERIFY(resultSliderOpacity == true);
    bool resultAdditionalOpacity = m_animator->setAnimatorEndValue("additional_opacity", 0.15);
    QVERIFY(resultAdditionalOpacity == true);
    bool resultMovePosition = m_animator->setAnimatorEndValue("move_position", 0.9);
    QVERIFY(resultMovePosition == true);
    bool resultBgWidth = m_animator->setAnimatorEndValue("bg_width", 1.0);
    QVERIFY(resultBgWidth == true);
    bool resultHoverZoomin = m_animator->setAnimatorEndValue("hover_zoomin", 1.2);
    QVERIFY(resultHoverZoomin == true);
    bool resultClickZoomin = m_animator->setAnimatorEndValue("click_zoomin", 1.0);
    QVERIFY(resultClickZoomin == true);
    bool resultInvalid = m_animator->setAnimatorEndValue("InvalidProperty", 0.5);
    QVERIFY(resultInvalid == false);
}

void TestConfigSliderAnimator::testAnimatorStartValue()
{
    m_animator->bindWidget(m_slider);
    m_animator->setAnimatorStartValue("slider_opacity", 0.2);
    QVariant startValue = m_animator->animatorStartValue("slider_opacity");
    QVERIFY(startValue.isValid());
    QVERIFY(startValue.toDouble() == 0.2);
    
    QVariant startValueInvalid = m_animator->animatorStartValue("InvalidProperty");
    QVERIFY(!startValueInvalid.isValid());
}

void TestConfigSliderAnimator::testAnimatorEndValue()
{
    m_animator->bindWidget(m_slider);
    m_animator->setAnimatorEndValue("slider_opacity", 0.7);
    QVariant endValue = m_animator->animatorEndValue("slider_opacity");
    QVERIFY(endValue.isValid());
    QVERIFY(endValue.toDouble() == 0.7);
    
    QVariant endValueInvalid = m_animator->animatorEndValue("InvalidProperty");
    QVERIFY(!endValueInvalid.isValid());
}

void TestConfigSliderAnimator::testSetAnimatorDuration()
{
    m_animator->bindWidget(m_slider);
    bool resultSliderOpacity = m_animator->setAnimatorDuration("slider_opacity", 300);
    QVERIFY(resultSliderOpacity == true);
    bool resultAdditionalOpacity = m_animator->setAnimatorDuration("additional_opacity", 200);
    QVERIFY(resultAdditionalOpacity == true);
    bool resultMovePosition = m_animator->setAnimatorDuration("move_position", 250);
    QVERIFY(resultMovePosition == true);
    bool resultBgWidth = m_animator->setAnimatorDuration("bg_width", 280);
    QVERIFY(resultBgWidth == true);
    bool resultHoverZoomin = m_animator->setAnimatorDuration("hover_zoomin", 150);
    QVERIFY(resultHoverZoomin == true);
    bool resultClickZoomin = m_animator->setAnimatorDuration("click_zoomin", 120);
    QVERIFY(resultClickZoomin == true);
    bool resultInvalid = m_animator->setAnimatorDuration("InvalidProperty", 100);
    QVERIFY(resultInvalid == false);
}

void TestConfigSliderAnimator::testSetAnimatorDirectionForward()
{
    m_animator->bindWidget(m_slider);
    m_animator->setAnimatorDirectionForward("slider_opacity", true);
    m_animator->setAnimatorDirectionForward("additional_opacity", false);
    m_animator->setAnimatorDirectionForward("move_position", true);
    m_animator->setAnimatorDirectionForward("bg_width", false);
    m_animator->setAnimatorDirectionForward("hover_zoomin", true);
    m_animator->setAnimatorDirectionForward("click_zoomin", false);
    m_animator->setAnimatorDirectionForward("InvalidProperty", true);
    QTest::qWait(10);
}

void TestConfigSliderAnimator::testStartAnimator()
{
    m_animator->bindWidget(m_slider);
    m_animator->startAnimator("slider_opacity");
    QTest::qWait(50);
    m_animator->startAnimator("additional_opacity");
    QTest::qWait(50);
    m_animator->startAnimator("move_position");
    QTest::qWait(50);
    m_animator->startAnimator("bg_width");
    QTest::qWait(50);
    m_animator->startAnimator("hover_zoomin");
    QTest::qWait(50);
    m_animator->startAnimator("click_zoomin");
    QTest::qWait(50);
    m_animator->startAnimator("InvalidProperty");
}

void TestConfigSliderAnimator::testStopAnimator()
{
    m_animator->bindWidget(m_slider);
    m_animator->startAnimator("slider_opacity");
    QTest::qWait(10);
    m_animator->stopAnimator("slider_opacity");
    m_animator->startAnimator("additional_opacity");
    QTest::qWait(10);
    m_animator->stopAnimator("additional_opacity");
    m_animator->startAnimator("move_position");
    QTest::qWait(10);
    m_animator->stopAnimator("move_position");
    m_animator->startAnimator("bg_width");
    QTest::qWait(10);
    m_animator->stopAnimator("bg_width");
    m_animator->startAnimator("hover_zoomin");
    QTest::qWait(10);
    m_animator->stopAnimator("hover_zoomin");
    m_animator->startAnimator("click_zoomin");
    QTest::qWait(10);
    m_animator->stopAnimator("click_zoomin");
    m_animator->stopAnimator("InvalidProperty");
}

void TestConfigSliderAnimator::testCurrentAnimatorTime()
{
    m_animator->bindWidget(m_slider);
    int timeSliderOpacity = m_animator->currentAnimatorTime("slider_opacity");
    QVERIFY(timeSliderOpacity >= 0);
    int timeAdditionalOpacity = m_animator->currentAnimatorTime("additional_opacity");
    QVERIFY(timeAdditionalOpacity >= 0);
    int timeMovePosition = m_animator->currentAnimatorTime("move_position");
    QVERIFY(timeMovePosition >= 0);
    int timeBgWidth = m_animator->currentAnimatorTime("bg_width");
    QVERIFY(timeBgWidth >= 0);
    int timeHoverZoomin = m_animator->currentAnimatorTime("hover_zoomin");
    QVERIFY(timeHoverZoomin >= 0);
    int timeClickZoomin = m_animator->currentAnimatorTime("click_zoomin");
    QVERIFY(timeClickZoomin >= 0);
    int timeInvalid = m_animator->currentAnimatorTime("InvalidProperty");
    QVERIFY(timeInvalid >= 0);
}

void TestConfigSliderAnimator::testSetAnimatorCurrentTime()
{
    m_animator->bindWidget(m_slider);
    m_animator->setAnimatorCurrentTime("move_position", 100);
    m_animator->setAnimatorCurrentTime("hover_zoomin", 50);
    m_animator->setAnimatorCurrentTime("click_zoomin", 80);
    int time = m_animator->currentAnimatorTime("move_position");
    QVERIFY(time >= 0);
}

void TestConfigSliderAnimator::testTotalAnimationDuration()
{
    m_animator->bindWidget(m_slider);
    int durationSliderOpacity = m_animator->totalAnimationDuration("slider_opacity");
    QVERIFY(durationSliderOpacity >= 0);
    int durationAdditionalOpacity = m_animator->totalAnimationDuration("additional_opacity");
    QVERIFY(durationAdditionalOpacity >= 0);
    int durationMovePosition = m_animator->totalAnimationDuration("move_position");
    QVERIFY(durationMovePosition >= 0);
    int durationBgWidth = m_animator->totalAnimationDuration("bg_width");
    QVERIFY(durationBgWidth >= 0);
    int durationHoverZoomin = m_animator->totalAnimationDuration("hover_zoomin");
    QVERIFY(durationHoverZoomin >= 0);
    int durationClickZoomin = m_animator->totalAnimationDuration("click_zoomin");
    QVERIFY(durationClickZoomin >= 0);
    int durationInvalid = m_animator->totalAnimationDuration("InvalidProperty");
    QVERIFY(durationInvalid >= 0);
}

void TestConfigSliderAnimator::testSetExtraProperty()
{
    m_animator->bindWidget(m_slider);
    m_animator->setExtraProperty("end_position", 100);
    m_animator->setExtraProperty("start_position", 0);
    m_animator->setExtraProperty("addValue", true);
    QTest::qWait(10);
}

void TestConfigSliderAnimator::testGetExtraProperty()
{
    m_animator->bindWidget(m_slider);
    m_animator->setExtraProperty("end_position", 100);
    m_animator->setExtraProperty("start_position", 0);
    m_animator->setExtraProperty("addValue", true);
    
    QVariant endPosition = m_animator->getExtraProperty("end_position");
    QVERIFY(endPosition.isValid());
    QVERIFY(endPosition.toInt() == 100);
    
    QVariant startPosition = m_animator->getExtraProperty("start_position");
    QVERIFY(startPosition.isValid());
    QVERIFY(startPosition.toInt() == 0);
    
    QVariant addValue = m_animator->getExtraProperty("addValue");
    QVERIFY(addValue.isValid());
    QVERIFY(addValue.toBool() == true);
    
    QVariant invalidProperty = m_animator->getExtraProperty("InvalidProperty");
    QVERIFY(!invalidProperty.isValid());
}

QTEST_MAIN(TestConfigSliderAnimator)

#include "test_config-slider-animator.moc"
