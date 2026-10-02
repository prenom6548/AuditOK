#ifndef TEST_QROUNDPROGRESSBAR_H
#define TEST_QROUNDPROGRESSBAR_H

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QWidget>

// 定义模拟的QGSettings类
#define QGSettings MockQGSettings
class MockQGSettings : public QObject
{
    Q_OBJECT
public:
    explicit MockQGSettings(const QString &schemaId, const QString &path = QString(), QObject *parent = nullptr)
        : QObject(parent) {}
    
    QVariant get(const QString &key) const {
        Q_UNUSED(key);
        return QVariant();
    }
    
    void set(const QString &key, const QVariant &value) {
        Q_UNUSED(key);
        Q_UNUSED(value);
    }
};

// 创建一个简化的menuModule模拟类
class menuModule : public QWidget
{
    Q_OBJECT
public:
    explicit menuModule(QWidget *parent = nullptr) : QWidget(parent), m_isDarktheme(false) {}
    bool m_isDarktheme;
    bool isDarkTheme() { return m_isDarktheme; }
};

// 创建一个简化的QRoundProgressBar模拟类
class QRoundProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit QRoundProgressBar(QWidget *parent = 0) : QWidget(parent), menu(new menuModule(this)), m_persent(0) {}
    
    menuModule *menu;

public slots:
    void setPersent(int persent) {
        m_persent = persent;
        repaint();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        // 简化的绘图事件，仅用于测试
    }

private:
    int m_persent;
};

class TestQRoundProgressBar : public QObject
{
    Q_OBJECT

public:
    TestQRoundProgressBar();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testSetPersent_data();
    void testSetPersent();

private:
    QRoundProgressBar* m_progressBar;
};

#endif // TEST_QROUNDPROGRESSBAR_H