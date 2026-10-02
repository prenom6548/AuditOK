#ifndef TEST_CONFIG_SCROLLBAR_ANIMATION_HELPER_H
#define TEST_CONFIG_SCROLLBAR_ANIMATION_HELPER_H

#include <QObject>
#include <QWidget>
#include <QTest>

class TestConfigScrollBarAnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigScrollBarAnimationHelper(QObject *parent = nullptr);
    ~TestConfigScrollBarAnimationHelper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testRegisterWidget();
    void testRegisterWidgetNull();
    void testUnregisterWidget();
    void testUnregisterWidgetNotRegistered();
    void testAnimator();
    void testAnimatorNotRegistered();

private:
    QWidget *m_testWidget;
};

#endif
