#include "test_config-progressbar-animation.h"

TestConfigProgressBarAnimation::TestConfigProgressBarAnimation(QObject *parent)
    : QObject(parent)
    , m_testWidget(nullptr)
{
}

TestConfigProgressBarAnimation::~TestConfigProgressBarAnimation()
{
    if (m_testWidget) {
        delete m_testWidget;
        m_testWidget = nullptr;
    }
}

void TestConfigProgressBarAnimation::initTestCase()
{
}

void TestConfigProgressBarAnimation::cleanupTestCase()
{
}

void TestConfigProgressBarAnimation::init()
{
    m_testWidget = new QWidget();
}

void TestConfigProgressBarAnimation::cleanup()
{
    if (m_testWidget) {
        delete m_testWidget;
        m_testWidget = nullptr;
    }
}

void TestConfigProgressBarAnimation::test_constructorWithNormalStatus()
{
    ConfigProgressBarAnimation *animation = new ConfigProgressBarAnimation(m_testWidget, ProgressBarStatus::NORMAL);
    QVERIFY(animation != nullptr);
    QVERIFY(animation->parent() == m_testWidget);
    QVERIFY(animation->duration() == 2500);
    QVERIFY(animation->startValue().toDouble() == 0.0);
    QVERIFY(animation->endValue().toDouble() == 1.0);
    QVERIFY(animation->direction() == QAbstractAnimation::Forward);
    delete animation;
}

void TestConfigProgressBarAnimation::test_constructorWithIndeterminateStatus()
{
    ConfigProgressBarAnimation *animation = new ConfigProgressBarAnimation(m_testWidget, ProgressBarStatus::INDETERMINATE);
    QVERIFY(animation != nullptr);
    QVERIFY(animation->parent() == m_testWidget);
    QVERIFY(animation->duration() == 2500);
    QVERIFY(animation->startValue().toDouble() == 0.0);
    QVERIFY(animation->endValue().toDouble() == 1.0);
    QVERIFY(animation->direction() == QAbstractAnimation::Forward);
    delete animation;
}

void TestConfigProgressBarAnimation::test_target()
{
    ConfigProgressBarAnimation *animation = new ConfigProgressBarAnimation(m_testWidget, ProgressBarStatus::NORMAL);
    QWidget *target = animation->target();
    QVERIFY(target == m_testWidget);
    delete animation;
}

void TestConfigProgressBarAnimation::test_destructor()
{
    ConfigProgressBarAnimation *animation = new ConfigProgressBarAnimation(m_testWidget, ProgressBarStatus::NORMAL);
    delete animation;
    QVERIFY(true);
}

void TestConfigProgressBarAnimation::test_signalValueChanged()
{
    ConfigProgressBarAnimation *animation = new ConfigProgressBarAnimation(m_testWidget, ProgressBarStatus::NORMAL);
    QSignalSpy spy(animation, &QVariantAnimation::valueChanged);
    QVERIFY(spy.isValid());
    animation->start();
    QTest::qWait(100);
    animation->stop();
    delete animation;
}

void TestConfigProgressBarAnimation::test_signalFinished()
{
    ConfigProgressBarAnimation *animation = new ConfigProgressBarAnimation(m_testWidget, ProgressBarStatus::NORMAL);
    QSignalSpy spy(animation, &QVariantAnimation::finished);
    QVERIFY(spy.isValid());
    animation->start();
    QTest::qWait(2600);
    animation->stop();
    delete animation;
}
