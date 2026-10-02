/*
 * Unit test for ConfigBoxAnimationHelper
 */

#include <QTest>
#include <QWidget>
#include <QComboBox>
#include <QSignalSpy>
#include "config-box-animation-helper.h"

class TestConfigBoxAnimationHelper : public QObject
{
    Q_OBJECT

public:
    TestConfigBoxAnimationHelper();
    ~TestConfigBoxAnimationHelper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test cases for each public function
    void test_constructor();
    void test_constructor_withParent();
    void test_destructor();
    void test_registerWidget_success();
    void test_registerWidget_notCombobox();
    void test_registerWidget_multipleWidgets();
    void test_unregisterWidget_success();
    void test_unregisterWidget_notRegistered();
    void test_unregisterWidget_multipleAnimators();
    void test_animator_registered();
    void test_animator_notRegistered();
    void test_animator_nullWidget();

private:
    UKUIConfigStyleSpace::ConfigBoxAnimationHelper *m_helper;
    QComboBox *m_testWidget;
};

TestConfigBoxAnimationHelper::TestConfigBoxAnimationHelper()
    : m_helper(nullptr)
    , m_testWidget(nullptr)
{
}

TestConfigBoxAnimationHelper::~TestConfigBoxAnimationHelper()
{
}

void TestConfigBoxAnimationHelper::initTestCase()
{
}

void TestConfigBoxAnimationHelper::cleanupTestCase()
{
}

void TestConfigBoxAnimationHelper::init()
{
    m_helper = new UKUIConfigStyleSpace::ConfigBoxAnimationHelper();
    m_testWidget = new QComboBox();
    m_testWidget->setObjectName("testWidget");
}

void TestConfigBoxAnimationHelper::cleanup()
{
    if (m_helper) {
        // Make sure all widgets are unregistered before deleting
        if (m_testWidget && m_helper->animator(m_testWidget)) {
            m_helper->unregisterWidget(m_testWidget);
        }
        delete m_helper;
        m_helper = nullptr;
    }
    if (m_testWidget) {
        delete m_testWidget;
        m_testWidget = nullptr;
    }
}

void TestConfigBoxAnimationHelper::test_constructor()
{
    UKUIConfigStyleSpace::ConfigBoxAnimationHelper helper;
    QVERIFY2(true, "Constructor without parent created successfully");
}

void TestConfigBoxAnimationHelper::test_constructor_withParent()
{
    QObject *parent = new QObject();
    UKUIConfigStyleSpace::ConfigBoxAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigBoxAnimationHelper(parent);
    QVERIFY(helper != nullptr);
    QVERIFY(helper->parent() == parent);
    delete helper;
    delete parent;
}

void TestConfigBoxAnimationHelper::test_destructor()
{
    UKUIConfigStyleSpace::ConfigBoxAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigBoxAnimationHelper();
    delete helper;
    QVERIFY2(true, "Destructor executed without crash");
}

void TestConfigBoxAnimationHelper::test_registerWidget_success()
{
    // Test successful widget registration with QComboBox
    bool result = m_helper->registerWidget(m_testWidget);
    QVERIFY(result == true);

    // Verify animator is created
    AnimatorIface *animator = m_helper->animator(m_testWidget);
    QVERIFY(animator != nullptr);
}

void TestConfigBoxAnimationHelper::test_registerWidget_notCombobox()
{
    // Test with regular QWidget (not QComboBox) - should fail
    QWidget *regularWidget = new QWidget();
    bool result = m_helper->registerWidget(regularWidget);
    QVERIFY(result == false);
    delete regularWidget;
}

void TestConfigBoxAnimationHelper::test_registerWidget_multipleWidgets()
{
    // Register first widget
    QComboBox *widget1 = new QComboBox();
    bool result1 = m_helper->registerWidget(widget1);
    QVERIFY(result1 == true);

    // Register second widget
    QComboBox *widget2 = new QComboBox();
    bool result2 = m_helper->registerWidget(widget2);
    QVERIFY(result2 == true);

    // Both should have animators
    QVERIFY(m_helper->animator(widget1) != nullptr);
    QVERIFY(m_helper->animator(widget2) != nullptr);

    // Cleanup
    m_helper->unregisterWidget(widget1);
    m_helper->unregisterWidget(widget2);
    delete widget1;
    delete widget2;
}

void TestConfigBoxAnimationHelper::test_unregisterWidget_success()
{
    // First register a widget
    bool regResult = m_helper->registerWidget(m_testWidget);
    QVERIFY(regResult == true);

    // Then unregister it
    bool unregResult = m_helper->unregisterWidget(m_testWidget);
    QVERIFY(unregResult == true);

    // Verify animator is removed
    AnimatorIface *animator = m_helper->animator(m_testWidget);
    QVERIFY(animator == nullptr);
}

void TestConfigBoxAnimationHelper::test_unregisterWidget_notRegistered()
{
    // Try to unregister a widget that was never registered
    QComboBox *unregisteredWidget = new QComboBox();
    bool result = m_helper->unregisterWidget(unregisteredWidget);
    // Should return false since nothing was registered
    QVERIFY(result == false);
    delete unregisteredWidget;
}

void TestConfigBoxAnimationHelper::test_unregisterWidget_multipleAnimators()
{
    // Register same widget multiple times (insertMulti allows duplicates)
    QComboBox *widget = new QComboBox();
    m_helper->registerWidget(widget);
    m_helper->registerWidget(widget);
    m_helper->registerWidget(widget);

    // Unregister should handle multiple animators
    bool result = m_helper->unregisterWidget(widget);
    QVERIFY(result == true);

    // Verify all animators are removed
    QVERIFY(m_helper->animator(widget) == nullptr);
    delete widget;
}

void TestConfigBoxAnimationHelper::test_animator_registered()
{
    // Register a widget first
    m_helper->registerWidget(m_testWidget);

    // Get animator
    AnimatorIface *animator = m_helper->animator(m_testWidget);
    QVERIFY(animator != nullptr);
}

void TestConfigBoxAnimationHelper::test_animator_notRegistered()
{
    // Query animator for unregistered widget
    QComboBox *unregisteredWidget = new QComboBox();
    AnimatorIface *animator = m_helper->animator(unregisteredWidget);
    QVERIFY(animator == nullptr);
    delete unregisteredWidget;
}

void TestConfigBoxAnimationHelper::test_animator_nullWidget()
{
    // Query animator for null widget
    AnimatorIface *animator = m_helper->animator(nullptr);
    QVERIFY(animator == nullptr);
}

QTEST_MAIN(TestConfigBoxAnimationHelper)

#include "test_config-box-animation-helper.moc"
