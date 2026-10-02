#ifndef TEST_KYLINAGENT_H
#define TEST_KYLINAGENT_H

#include <QObject>
#include <QWidget>

class TestKylinAgent : public QObject
{
    Q_OBJECT
public:
    explicit TestKylinAgent(QObject *parent = nullptr);
    ~TestKylinAgent();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void test_constructor();
    void test_destructor();
    void test_startKylinAgent();
    void test_setParentWidget();

private:
    QWidget *m_testWidget;
};

#endif // TEST_KYLINAGENT_H
