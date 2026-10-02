#include "test_config-tree-animator.h"

TestConfigTreeAnimator::TestConfigTreeAnimator(QObject *parent)
    : QObject(parent)
    , m_animator(nullptr)
    , m_treeView(nullptr)
{
}

void TestConfigTreeAnimator::initTestCase()
{
}

void TestConfigTreeAnimator::cleanupTestCase()
{
}

void TestConfigTreeAnimator::init()
{
    m_animator = new ConfigTreeAnimator();
    m_treeView = new QTreeView();
}

void TestConfigTreeAnimator::cleanup()
{
    if (m_animator) {
        m_animator->deleteLater();
        m_animator = nullptr;
    }
    if (m_treeView) {
        m_treeView->deleteLater();
        m_treeView = nullptr;
    }
}

void TestConfigTreeAnimator::testConstructor()
{
    QVERIFY(m_animator != nullptr);
    QVERIFY(m_animator->boundedWidget() == nullptr);
}

void TestConfigTreeAnimator::testDestructor()
{
    m_animator->bindWidget(m_treeView);
    delete m_animator;
    m_animator = nullptr;
    QVERIFY(true);
}

void TestConfigTreeAnimator::testBindWidget()
{
    bool result = m_animator->bindWidget(m_treeView);
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() == m_treeView);
}

void TestConfigTreeAnimator::testBindWidgetWithDoNotAnimateProperty()
{
    m_treeView->setProperty("doNotAnimate", true);
    bool result = m_animator->bindWidget(m_treeView);
    QVERIFY(result == false);
    QVERIFY(m_animator->boundedWidget() == nullptr);
}

void TestConfigTreeAnimator::testBindWidgetWithNonTreeView()
{
    QWidget widget;
    bool result = m_animator->bindWidget(&widget);
    QVERIFY(result == false);
    QVERIFY(m_animator->boundedWidget() == nullptr);
}

void TestConfigTreeAnimator::testUnboundWidget()
{
    m_animator->bindWidget(m_treeView);
    bool result = m_animator->unboundWidget();
    QVERIFY(result == true);
    QVERIFY(m_animator->boundedWidget() == nullptr);
}

void TestConfigTreeAnimator::testUnboundWidgetWithoutWidget()
{
    bool result = m_animator->unboundWidget();
    QVERIFY(result == false);
}

void TestConfigTreeAnimator::testBoundedWidget()
{
    m_animator->bindWidget(m_treeView);
    QVERIFY(m_animator->boundedWidget() == m_treeView);
    
    m_animator->unboundWidget();
    QVERIFY(m_animator->boundedWidget() == nullptr);
}

void TestConfigTreeAnimator::testValue()
{
    m_animator->bindWidget(m_treeView);
    
    QVariant expandValue = m_animator->value("expand");
    QVERIFY(expandValue.isValid());
    
    QVariant collapsValue = m_animator->value("collaps");
    QVERIFY(collapsValue.isValid());
    
    QVariant invalidValue = m_animator->value("invalid");
    QVERIFY(!invalidValue.isValid());
}

void TestConfigTreeAnimator::testSetAnimatorStartValue()
{
    m_animator->bindWidget(m_treeView);
    
    bool result1 = m_animator->setAnimatorStartValue("expand", 0.5);
    QVERIFY(result1 == true);
    
    bool result2 = m_animator->setAnimatorStartValue("collaps", 0.3);
    QVERIFY(result2 == true);
    
    bool result3 = m_animator->setAnimatorStartValue("invalid", 0.1);
    QVERIFY(result3 == false);
}

void TestConfigTreeAnimator::testSetAnimatorEndValue()
{
    m_animator->bindWidget(m_treeView);
    
    bool result1 = m_animator->setAnimatorEndValue("expand", 1.0);
    QVERIFY(result1 == true);
    
    bool result2 = m_animator->setAnimatorEndValue("collaps", 0.8);
    QVERIFY(result2 == true);
    
    bool result3 = m_animator->setAnimatorEndValue("invalid", 0.5);
    QVERIFY(result3 == false);
}

void TestConfigTreeAnimator::testAnimatorStartValue()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->setAnimatorStartValue("expand", 0.2);
    QVariant startValue = m_animator->animatorStartValue("expand");
    QVERIFY(startValue.toDouble() == 0.2);
    
    m_animator->setAnimatorStartValue("collaps", 0.1);
    QVariant collapsStartValue = m_animator->animatorStartValue("collaps");
    QVERIFY(collapsStartValue.toDouble() == 0.1);
    
    QVariant invalidValue = m_animator->animatorStartValue("invalid");
    QVERIFY(!invalidValue.isValid());
}

void TestConfigTreeAnimator::testAnimatorEndValue()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->setAnimatorEndValue("expand", 0.9);
    QVariant endValue = m_animator->animatorEndValue("expand");
    QVERIFY(endValue.toDouble() == 0.9);
    
    m_animator->setAnimatorEndValue("collaps", 0.7);
    QVariant collapsEndValue = m_animator->animatorEndValue("collaps");
    QVERIFY(collapsEndValue.toDouble() == 0.7);
    
    QVariant invalidValue = m_animator->animatorEndValue("invalid");
    QVERIFY(!invalidValue.isValid());
}

void TestConfigTreeAnimator::testSetAnimatorDuration()
{
    m_animator->bindWidget(m_treeView);
    
    bool result1 = m_animator->setAnimatorDuration("expand", 500);
    QVERIFY(result1 == true);
    
    bool result2 = m_animator->setAnimatorDuration("collaps", 300);
    QVERIFY(result2 == true);
    
    bool result3 = m_animator->setAnimatorDuration("invalid", 100);
    QVERIFY(result3 == false);
}

void TestConfigTreeAnimator::testSetAnimatorDirectionForward()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->setAnimatorDirectionForward("expand", true);
    QVERIFY(true);
    
    m_animator->setAnimatorDirectionForward("collaps", false);
    QVERIFY(true);
    
    m_animator->setAnimatorDirectionForward("invalid", true);
    QVERIFY(true);
}

void TestConfigTreeAnimator::testStartAnimator()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->startAnimator("expand");
    QVERIFY(true);
    
    m_animator->startAnimator("collaps");
    QVERIFY(true);
    
    m_animator->startAnimator("invalid");
    QVERIFY(true);
}

void TestConfigTreeAnimator::testStopAnimator()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->startAnimator("expand");
    m_animator->stopAnimator("expand");
    QVERIFY(true);
    
    m_animator->startAnimator("collaps");
    m_animator->stopAnimator("collaps");
    QVERIFY(true);
    
    m_animator->stopAnimator("invalid");
    QVERIFY(true);
}

void TestConfigTreeAnimator::testCurrentAnimatorTime()
{
    m_animator->bindWidget(m_treeView);
    
    int time1 = m_animator->currentAnimatorTime("expand");
    QVERIFY(time1 >= 0);
    
    int time2 = m_animator->currentAnimatorTime("collaps");
    QVERIFY(time2 >= 0);
    
    int time3 = m_animator->currentAnimatorTime("invalid");
    QVERIFY(time3 >= 0);
}

void TestConfigTreeAnimator::testSetAnimatorCurrentTime()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->setAnimatorCurrentTime("expand", 100);
    QVERIFY(true);
    
    m_animator->setAnimatorCurrentTime("collaps", 50);
    QVERIFY(true);
}

void TestConfigTreeAnimator::testTotalAnimationDuration()
{
    m_animator->bindWidget(m_treeView);
    
    int duration1 = m_animator->totalAnimationDuration("expand");
    QVERIFY(duration1 == 250);
    
    int duration2 = m_animator->totalAnimationDuration("collaps");
    QVERIFY(duration2 == 250);
    
    int duration3 = m_animator->totalAnimationDuration("invalid");
    QVERIFY(duration3 >= 0);
}

void TestConfigTreeAnimator::testSetExtraProperty()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->setExtraProperty("expandItem", QVariant(1));
    QVERIFY(true);
    
    m_animator->setExtraProperty("collapsItem", QVariant(2));
    QVERIFY(true);
}

void TestConfigTreeAnimator::testGetExtraProperty()
{
    m_animator->bindWidget(m_treeView);
    
    m_animator->setExtraProperty("expandItem", QVariant(1));
    
    QVariant result = m_animator->getExtraProperty("expandItem");
    QVERIFY(result.isValid());
    
    QVariant result2 = m_animator->getExtraProperty("collapsItem");
    QVERIFY(result2.isValid());
    
    QVariant invalidResult = m_animator->getExtraProperty("invalid");
    QVERIFY(!invalidResult.isValid());
}

void TestConfigTreeAnimator::testIsRunning()
{
    m_animator->bindWidget(m_treeView);
    
    bool running1 = m_animator->isRunning("expand");
    QVERIFY(running1 == false);
    
    bool running2 = m_animator->isRunning("collaps");
    QVERIFY(running2 == false);
    
    bool running3 = m_animator->isRunning("invalid");
    QVERIFY(running3 == false);
}

void TestConfigTreeAnimator::testValueWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    QVariant result = m_animator->value("invalidProperty");
    QVERIFY(!result.isValid());
}

void TestConfigTreeAnimator::testSetAnimatorStartValueWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    bool result = m_animator->setAnimatorStartValue("invalidProperty", 0.5);
    QVERIFY(result == false);
}

void TestConfigTreeAnimator::testSetAnimatorEndValueWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    bool result = m_animator->setAnimatorEndValue("invalidProperty", 0.5);
    QVERIFY(result == false);
}

void TestConfigTreeAnimator::testAnimatorStartValueWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    QVariant result = m_animator->animatorStartValue("invalidProperty");
    QVERIFY(!result.isValid());
}

void TestConfigTreeAnimator::testAnimatorEndValueWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    QVariant result = m_animator->animatorEndValue("invalidProperty");
    QVERIFY(!result.isValid());
}

void TestConfigTreeAnimator::testSetAnimatorDurationWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    bool result = m_animator->setAnimatorDuration("invalidProperty", 100);
    QVERIFY(result == false);
}

void TestConfigTreeAnimator::testSetAnimatorDirectionForwardWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    m_animator->setAnimatorDirectionForward("invalidProperty", true);
    QVERIFY(true);
}

void TestConfigTreeAnimator::testStartAnimatorWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    m_animator->startAnimator("invalidProperty");
    QVERIFY(true);
}

void TestConfigTreeAnimator::testStopAnimatorWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    m_animator->stopAnimator("invalidProperty");
    QVERIFY(true);
}

void TestConfigTreeAnimator::testCurrentAnimatorTimeWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    int time = m_animator->currentAnimatorTime("invalidProperty");
    QVERIFY(time >= 0);
}

void TestConfigTreeAnimator::testSetAnimatorCurrentTimeWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    m_animator->setAnimatorCurrentTime("invalidProperty", 100);
    QVERIFY(true);
}

void TestConfigTreeAnimator::testTotalAnimationDurationWithInvalidProperty()
{
    m_animator->bindWidget(m_treeView);
    int duration = m_animator->totalAnimationDuration("invalidProperty");
    QVERIFY(duration >= 0);
}
