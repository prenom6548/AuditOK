#include "test_config-progressbar-animation-helper.h"

TestConfigProgressBarAnimationHelper::TestConfigProgressBarAnimationHelper(QObject *parent)
    : QObject(parent)
    , m_testWidget(nullptr)
    , m_testAnimation(nullptr)
{
}

TestConfigProgressBarAnimationHelper::~TestConfigProgressBarAnimationHelper()
{
    if (m_testWidget) {
        delete m_testWidget;
        m_testWidget = nullptr;
    }
}

void TestConfigProgressBarAnimationHelper::initTestCase()
{
}

void TestConfigProgressBarAnimationHelper::cleanupTestCase()
{
}

void TestConfigProgressBarAnimationHelper::init()
{
    m_testWidget = new QWidget();
    m_testAnimation = new QVariantAnimation(m_testWidget);
}

void TestConfigProgressBarAnimationHelper::cleanup()
{
    if (m_testAnimation) {
        delete m_testAnimation;
        m_testAnimation = nullptr;
    }
    if (m_testWidget) {
        delete m_testWidget;
        m_testWidget = nullptr;
    }
}

void TestConfigProgressBarAnimationHelper::test_constructor()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    QVERIFY(helper != nullptr);
    QVERIFY(helper->parent() == nullptr);
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_destructor()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    delete helper;
    QVERIFY(true);
}

void TestConfigProgressBarAnimationHelper::test_initAnimationMap()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();

    QWidget *testWidget = new QWidget();
    QVariantAnimation *animation = new QVariantAnimation(testWidget);
    helper->initAnimationMap(testWidget, animation);

    QVariantAnimation *retrieved = helper->getAnimation(testWidget);
    QVERIFY(retrieved == animation);

    delete testWidget;
    delete helper;
    QTest::qWait(50);
}

void TestConfigProgressBarAnimationHelper::test_initAnimationMapWithNullWidget()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    QVariantAnimation *animation = new QVariantAnimation(m_testWidget);
    
    helper->initAnimationMap(nullptr, animation);
    
    QVariantAnimation *retrieved = helper->getAnimation(nullptr);
    QVERIFY(retrieved == nullptr);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_initAnimationMapWithNullAnimation()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    QWidget *widget = new QWidget();
    
    helper->initAnimationMap(widget, nullptr);
    
    QVariantAnimation *retrieved = helper->getAnimation(widget);
    QVERIFY(retrieved == nullptr);
    
    delete helper;
    delete widget;
}

void TestConfigProgressBarAnimationHelper::test_stopAnimation()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();

    QWidget *testWidget = new QWidget();
    QVariantAnimation *animation = new QVariantAnimation(testWidget);
    helper->initAnimationMap(testWidget, animation);

    helper->stopAnimation(testWidget);

    QVariantAnimation *retrieved = helper->getAnimation(testWidget);
    QVERIFY(retrieved == nullptr);

    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_stopAnimationNotInMap()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    QWidget *widget = new QWidget();
    
    helper->stopAnimation(widget);
    
    QVERIFY(true);
    
    delete helper;
    delete widget;
}

void TestConfigProgressBarAnimationHelper::test_getAnimation()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();

    QWidget *testWidget = new QWidget();
    QVariantAnimation *animation = new QVariantAnimation(testWidget);
    helper->initAnimationMap(testWidget, animation);

    QVariantAnimation *retrieved = helper->getAnimation(testWidget);
    QVERIFY(retrieved == animation);

    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_getAnimationNotInMap()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    QWidget *widget = new QWidget();
    
    QVariantAnimation *retrieved = helper->getAnimation(widget);
    QVERIFY(retrieved == nullptr);
    
    delete helper;
    delete widget;
}

void TestConfigProgressBarAnimationHelper::test_canEnableNormalAnimation()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    QVERIFY(helper->canEnableNormalAnimation(150) == true);
    QVERIFY(helper->canEnableNormalAnimation(200) == true);
    QVERIFY(helper->canEnableNormalAnimation(100) == false);
    QVERIFY(helper->canEnableNormalAnimation(0) == false);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_canEnableNormalAnimationBoundary()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    QVERIFY(helper->canEnableNormalAnimation(149) == false);
    QVERIFY(helper->canEnableNormalAnimation(150) == true);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_getSliderFixedSpan()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    int span = helper->getSliderFixedSpan();
    QVERIFY(span == 80);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_cacheTotalBarInfo()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    helper->cacheTotalBarInfo(300, 50);
    
    int width = 0;
    int height = 0;
    helper->getTotalBarInfo(width, height);
    
    QVERIFY(width == 300);
    QVERIFY(height == 50);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_getTotalBarInfo()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    helper->cacheTotalBarInfo(100, 20);
    
    int width = -1;
    int height = -1;
    helper->getTotalBarInfo(width, height);
    
    QVERIFY(width == 100);
    QVERIFY(height == 20);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_meetCondition()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    QVERIFY(helper->meetCondition(200) == true);
    QVERIFY(helper->meetCondition(300) == true);
    QVERIFY(helper->meetCondition(100) == false);
    QVERIFY(helper->meetCondition(0) == false);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_meetConditionBoundary()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    QVERIFY(helper->meetCondition(199) == false);
    QVERIFY(helper->meetCondition(200) == true);
    
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_setAnimationDuration()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    QWidget *testWidget = new QWidget();
    QVariantAnimation *animation = new QVariantAnimation(testWidget);

    helper->setAnimationDuration(animation, 100, 0.5);

    int expectedDuration = 2000 + 100 * 0.5;
    QVERIFY(animation->duration() == expectedDuration);

    delete testWidget;
    delete helper;
}

void TestConfigProgressBarAnimationHelper::test_setAnimationDurationWithNullAnimation()
{
    ConfigProgressBarAnimationHelper *helper = new ConfigProgressBarAnimationHelper();
    
    helper->setAnimationDuration(nullptr, 100, 0.5);
    
    QVERIFY(true);
    
    delete helper;
}
