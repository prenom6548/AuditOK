#ifndef TEST_DBUSADAPTOR_H
#define TEST_DBUSADAPTOR_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QWidget>
#include <QDBusAbstractAdaptor>

// 重新定义DbusAdaptor类，去除复杂依赖
class MockMainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MockMainWindow(QWidget *parent = nullptr) : QWidget(parent), m_handleIconClickedCalled(false) {}
    
    bool handleIconClickedCalled() const { return m_handleIconClickedCalled; }
    
public slots:
    void handleIconClickedSub(const QString& msg = "") {
        // 记录函数被调用
        m_handleIconClickedCalled = true;
        Q_UNUSED(msg);
    }
    
private:
    bool m_handleIconClickedCalled;
};

// 简化版DbusAdaptor实现
class DbusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.usb.creator")

public:
    DbusAdaptor(QObject *parent)
        : QDBusAbstractAdaptor(parent)
    {
        setAutoRelaySignals(true);
    }

    ~DbusAdaptor()
    {
        // destructor
    }

public slots:
    void showMainWindow()
    {
        // 调用父窗口的handleIconClickedSub方法
        QMetaObject::invokeMethod(parent(), "handleIconClickedSub", Qt::QueuedConnection);
    }
};

class TestDbusAdaptor : public QObject
{
    Q_OBJECT

public:
    TestDbusAdaptor();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testDestructor();
    void testShowMainWindow();

private:
    DbusAdaptor* m_dbusAdaptor;
    MockMainWindow* m_mockMainWindow;
};

#endif // TEST_DBUSADAPTOR_H