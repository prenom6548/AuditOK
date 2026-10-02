#ifndef TEST_GSETTINGSWRAPPER_H
#define TEST_GSETTINGSWRAPPER_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../kybackup/gsettingswrapper.h"

class TestGSettingsWrapper : public QObject
{
    Q_OBJECT

public:
    explicit TestGSettingsWrapper(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试构造函数和析构函数
    void testConstructor();
    
    // 测试isDarkTheme方法
    void testIsDarkTheme();
    
    // 测试信号
    void testSignals();

private:
    GSettingsWrapper* m_gsettingsWrapper;
};

#endif // TEST_GSETTINGSWRAPPER_H