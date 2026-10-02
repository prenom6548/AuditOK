#include "test_config-tree-animation-helper.h"

void TestConfigTreeAnimationHelper::initTestCase()
{
    m_helper = nullptr;
    m_widget = nullptr;
}

void TestConfigTreeAnimationHelper::cleanupTestCase()
{
}

void TestConfigTreeAnimationHelper::init()
{
    m_helper = new ConfigTreeAnimationHelper();
    m_widget = new QTreeView();
}

void TestConfigTreeAnimationHelper::cleanup()
{
    if (m_helper) {
        delete m_helper;
        m_helper = nullptr;
    }
    if (m_widget) {
        delete m_widget;
        m_widget = nullptr;
    }
}

void TestConfigTreeAnimationHelper::testConstructor()
{
    ConfigTreeAnimationHelper *helper = new ConfigTreeAnimationHelper();
    QVERIFY(helper != nullptr);
    delete helper;
}

void TestConfigTreeAnimationHelper::testConstructorWithParent()
{
    QObject *parent = new QObject();
    ConfigTreeAnimationHelper *helper = new ConfigTreeAnimationHelper(parent);
    QVERIFY(helper != nullptr);
    QVERIFY(helper->parent() == parent);
    delete parent;
}

void TestConfigTreeAnimationHelper::testDestructor()
{
    QTreeView *widget = new QTreeView();
    ConfigTreeAnimationHelper *helper = new ConfigTreeAnimationHelper();
    bool result = helper->registerWidget(widget);
    if (result) {
        delete helper;
        QTest::qWait(10);
    }
    delete widget;
}

void TestConfigTreeAnimationHelper::testRegisterWidget()
{
    bool result = m_helper->registerWidget(m_widget);
    QVERIFY(result == true);
    AnimatorIface *animator = m_helper->animator(m_widget);
    QVERIFY(animator != nullptr);
}

void TestConfigTreeAnimationHelper::testRegisterWidgetFailed()
{
    QWidget *widget = new QWidget();
    bool result = m_helper->registerWidget(widget);
    QVERIFY(result == false);
    AnimatorIface *animator = m_helper->animator(widget);
    QVERIFY(animator == nullptr);
    delete widget;
}

void TestConfigTreeAnimationHelper::testUnregisterWidget()
{
    m_helper->registerWidget(m_widget);
    bool result = m_helper->unregisterWidget(m_widget);
    QVERIFY(result == true);
    AnimatorIface *animator = m_helper->animator(m_widget);
    QVERIFY(animator == nullptr);
}

void TestConfigTreeAnimationHelper::testUnregisterWidgetNotRegistered()
{
    QTreeView *newWidget = new QTreeView();
    bool result = m_helper->unregisterWidget(newWidget);
    QVERIFY(result == false);
    delete newWidget;
}

void TestConfigTreeAnimationHelper::testAnimator()
{
    m_helper->registerWidget(m_widget);
    AnimatorIface *animator = m_helper->animator(m_widget);
    QVERIFY(animator != nullptr);
}

void TestConfigTreeAnimationHelper::testAnimatorNotRegistered()
{
    QTreeView *newWidget = new QTreeView();
    AnimatorIface *animator = m_helper->animator(newWidget);
    QVERIFY(animator == nullptr);
    delete newWidget;
}

void TestConfigTreeAnimationHelper::testRegisterAndUnregisterMultipleWidgets()
{
    QTreeView *widget1 = new QTreeView();
    QTreeView *widget2 = new QTreeView();
    QTreeView *widget3 = new QTreeView();

    bool result1 = m_helper->registerWidget(widget1);
    QVERIFY(result1 == true);

    bool result2 = m_helper->registerWidget(widget2);
    QVERIFY(result2 == true);

    bool result3 = m_helper->registerWidget(widget3);
    QVERIFY(result3 == true);

    AnimatorIface *anim1 = m_helper->animator(widget1);
    AnimatorIface *anim2 = m_helper->animator(widget2);
    AnimatorIface *anim3 = m_helper->animator(widget3);
    QVERIFY(anim1 != nullptr);
    QVERIFY(anim2 != nullptr);
    QVERIFY(anim3 != nullptr);

    bool unreg1 = m_helper->unregisterWidget(widget1);
    QVERIFY(unreg1 == true);

    bool unreg2 = m_helper->unregisterWidget(widget2);
    QVERIFY(unreg2 == true);

    bool unreg3 = m_helper->unregisterWidget(widget3);
    QVERIFY(unreg3 == true);

    delete widget1;
    delete widget2;
    delete widget3;
}

void TestConfigTreeAnimationHelper::testDestroyedSignalConnection()
{
    QTreeView *tempWidget = new QTreeView();
    bool result = m_helper->registerWidget(tempWidget);
    QVERIFY(result == true);

    delete tempWidget;
    QTest::qWait(10);

    AnimatorIface *animator = m_helper->animator(tempWidget);
    QVERIFY(animator == nullptr);
}

QTEST_MAIN(TestConfigTreeAnimationHelper)

#include "test_config-tree-animation-helper.moc"
