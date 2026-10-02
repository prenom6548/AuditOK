#ifndef TEST_CONFIG_PROGRESSBAR_ANIMATION_H
#define TEST_CONFIG_PROGRESSBAR_ANIMATION_H

#include <QObject>
#include <QWidget>
#include <QTest>
#include <QSignalSpy>
#include "config-progressbar-animation.h"

using namespace UKUIConfigStyleSpace;

class TestConfigProgressBarAnimation : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigProgressBarAnimation(QObject *parent = nullptr);
    ~TestConfigProgressBarAnimation();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructorWithNormalStatus();
    void test_constructorWithIndeterminateStatus();
    void test_target();
    void test_destructor();
    void test_signalValueChanged();
    void test_signalFinished();

private:
    QWidget *m_testWidget;
};

#endif // TEST_CONFIG_PROGRESSBAR_ANIMATION_H
