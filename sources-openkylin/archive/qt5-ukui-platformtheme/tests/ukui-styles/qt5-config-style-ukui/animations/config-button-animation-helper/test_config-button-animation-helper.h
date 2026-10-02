#ifndef TEST_CONFIG_BUTTON_ANIMATION_HELPER_H
#define TEST_CONFIG_BUTTON_ANIMATION_HELPER_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QTest>

#include "config-button-animation-helper.h"

class TestConfigButtonAnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigButtonAnimationHelper(QObject *parent = nullptr);
    ~TestConfigButtonAnimationHelper();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testRegisterWidget();
    void testRegisterWidget_NullWidget();
    void testUnregisterWidget();
    void testUnregisterWidget_NotRegistered();
    void testAnimator();
    void testAnimator_NotFound();

private:
    UKUIConfigStyleSpace::ConfigButtonAnimationHelper *m_helper;
    QPushButton *m_testWidget;
};

#endif // TEST_CONFIG_BUTTON_ANIMATION_HELPER_H
