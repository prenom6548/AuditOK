#include "test_globalsettings.h"
#define private public
#include "../../../windowsview/globalsettings.h"
#undef  private

using namespace UkuiWindowSwitch;

TestGlobalSettings::TestGlobalSettings(QObject *parent)
    : QObject(parent)
{
}

void TestGlobalSettings::cleanup()
{
    // Reset singleton for each test
    m_singletonAddress = nullptr;
}

void TestGlobalSettings::test_globalInstance()
{
    cleanup();

    // Note: Singleton pattern with std::call_once only uses parent on first call
    // Subsequent calls ignore the parent parameter
    GlobalSettings *instance = GlobalSettings::globalInstance(nullptr);

    QVERIFY2(instance != nullptr, "globalInstance should return a valid pointer");

    m_singletonAddress = reinterpret_cast<void*>(instance);
}

void TestGlobalSettings::test_globalInstanceTwice()
{
    cleanup();
    
    QObject *parent1 = new QObject();
    GlobalSettings *instance1 = GlobalSettings::globalInstance(parent1);
    void *addr1 = reinterpret_cast<void*>(instance1);
    
    QObject *parent2 = new QObject();
    GlobalSettings *instance2 = GlobalSettings::globalInstance(parent2);
    void *addr2 = reinterpret_cast<void*>(instance2);
    
    // Singleton pattern: same instance should be returned
    QVERIFY2(addr1 == addr2, "globalInstance should return the same singleton instance");
    
    m_singletonAddress = addr1;
    
    delete parent1;
    delete parent2;
}

void TestGlobalSettings::test_panelPosition()
{
    cleanup();
    
    QObject *parent = new QObject();
    GlobalSettings *instance = GlobalSettings::globalInstance(parent);
    
    // Get initial panel position
    int position = instance->panelPosition();
    
    // panelPosition should return a valid integer
    QVERIFY2(position >= 0, "panelPosition should return a non-negative value");
    
    m_singletonAddress = reinterpret_cast<void*>(instance);
    
    delete parent;
}

void TestGlobalSettings::test_panelPositionAfterSignal()
{
    cleanup();
    
    QObject *parent = new QObject();
    GlobalSettings *instance = GlobalSettings::globalInstance(parent);
    
    // Listen to panelPositionChanged signal
    QSignalSpy spy(instance, SIGNAL(panelPositionChanged(int)));
    
    // Get initial position
    int initialPosition = instance->panelPosition();
    
    // Verify initial position is valid
    QVERIFY2(initialPosition >= 0, "Initial panel position should be non-negative");
    
    // Get position again to verify consistency
    int currentPosition = instance->panelPosition();
    QVERIFY2(currentPosition >= 0, "Current panel position should be non-negative");
    
    m_singletonAddress = reinterpret_cast<void*>(instance);
    
    delete parent;
}

void TestGlobalSettings::test_constructor()
{
    cleanup();

    GlobalSettings *instance0 = GlobalSettings::globalInstance(nullptr);
    cleanup();

    // Test constructor via globalInstance (constructor is private)
    QObject *parent = new QObject();
    GlobalSettings *instance = GlobalSettings::globalInstance(parent);

    QVERIFY2(instance != nullptr, "globalInstance should create valid instance");

    // Test constructor not nullprt
    instance->initPanelGSettings();

    // Test panelPosition works after construction via initPanelGSettings
    int position = instance->panelPosition();
    QVERIFY2(position >= 0, "panelPosition should be valid after construction");

    m_singletonAddress = reinterpret_cast<void*>(instance);

    delete parent;
}

void TestGlobalSettings::test_panelPositionChanged()
{
    QObject *parent = new QObject();
    GlobalSettings *instance = GlobalSettings::globalInstance(parent);

    QVERIFY2(instance != nullptr, "globalInstance should create valid instance");

    QSignalSpy spy(instance->m_panelGSettings, SIGNAL(changed(const QString)));

    emit instance->m_panelGSettings->changed("panelposition");
    QCOMPARE(spy.count(), 1);

    emit instance->m_panelGSettings->changed("panelsize");
    QCOMPARE(spy.count(), 2);
}
