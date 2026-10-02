#ifndef TEST_GLOBALSETTINGS_H
#define TEST_GLOBALSETTINGS_H

#include <QObject>
#include <QTest>
#include <QSignalSpy>

class TestGlobalSettings : public QObject
{
    Q_OBJECT

public:
    explicit TestGlobalSettings(QObject *parent = nullptr);
    ~TestGlobalSettings() = default;

private slots:
    void test_globalInstance();
    void test_globalInstanceTwice();
    void test_panelPosition();
    void test_panelPositionAfterSignal();
    void test_constructor();
    void test_panelPositionChanged();

private:
    void cleanup();

    void *m_singletonAddress = nullptr;
};

#endif // TEST_GLOBALSETTINGS_H
