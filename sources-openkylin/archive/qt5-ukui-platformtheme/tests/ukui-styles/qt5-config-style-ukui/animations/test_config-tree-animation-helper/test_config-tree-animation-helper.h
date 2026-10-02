#ifndef TEST_CONFIG_TREE_ANIMATION_HELPER_H
#define TEST_CONFIG_TREE_ANIMATION_HELPER_H

#include <QObject>
#include <QTest>
#include <QWidget>
#include <QTreeView>
#include <QHash>

#include "config-tree-animation-helper.h"
#include "config-tree-animator.h"

using namespace UKUIConfigStyleSpace;

class TestConfigTreeAnimationHelper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testConstructorWithParent();
    void testDestructor();
    void testRegisterWidget();
    void testRegisterWidgetFailed();
    void testUnregisterWidget();
    void testUnregisterWidgetNotRegistered();
    void testAnimator();
    void testAnimatorNotRegistered();
    void testRegisterAndUnregisterMultipleWidgets();
    void testDestroyedSignalConnection();

private:
    ConfigTreeAnimationHelper *m_helper;
    QTreeView *m_widget;
};

#endif // TEST_CONFIG_TREE_ANIMATION_HELPER_H
