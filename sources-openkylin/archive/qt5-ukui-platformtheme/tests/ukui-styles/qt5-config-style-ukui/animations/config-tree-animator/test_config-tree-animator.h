#ifndef TEST_CONFIG_TREE_ANIMATOR_H
#define TEST_CONFIG_TREE_ANIMATOR_H

#include <QObject>
#include <QTest>
#include <QTreeView>
#include "config-tree-animator.h"
using namespace UKUIConfigStyleSpace;

class TestConfigTreeAnimator : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigTreeAnimator(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testBindWidget();
    void testBindWidgetWithDoNotAnimateProperty();
    void testBindWidgetWithNonTreeView();
    void testUnboundWidget();
    void testBoundedWidget();
    void testValue();
    void testSetAnimatorStartValue();
    void testSetAnimatorEndValue();
    void testAnimatorStartValue();
    void testAnimatorEndValue();
    void testSetAnimatorDuration();
    void testSetAnimatorDirectionForward();
    void testStartAnimator();
    void testStopAnimator();
    void testCurrentAnimatorTime();
    void testSetAnimatorCurrentTime();
    void testTotalAnimationDuration();
    void testSetExtraProperty();
    void testGetExtraProperty();
    void testIsRunning();
    void testValueWithInvalidProperty();
    void testSetAnimatorStartValueWithInvalidProperty();
    void testSetAnimatorEndValueWithInvalidProperty();
    void testAnimatorStartValueWithInvalidProperty();
    void testAnimatorEndValueWithInvalidProperty();
    void testSetAnimatorDurationWithInvalidProperty();
    void testSetAnimatorDirectionForwardWithInvalidProperty();
    void testStartAnimatorWithInvalidProperty();
    void testStopAnimatorWithInvalidProperty();
    void testCurrentAnimatorTimeWithInvalidProperty();
    void testSetAnimatorCurrentTimeWithInvalidProperty();
    void testTotalAnimationDurationWithInvalidProperty();
    void testUnboundWidgetWithoutWidget();

private:
    ConfigTreeAnimator *m_animator;
    QTreeView *m_treeView;
};

#endif // TEST_CONFIG_TREE_ANIMATOR_H
