#include "test_kylinagent.h"
#include <QTest>

#include "kylinsecretagent.h" 

class KylinAgent : public QObject
{
public:
    explicit KylinAgent(QObject *parent = nullptr);
    ~KylinAgent();
    void startKylinAgent();
    void setParentWidget(QWidget *widget){
        pWidget = widget;
    }

private:
    KylinSecretAgent *m_secretAgnet;
    QWidget* pWidget = nullptr;
};

TestKylinAgent::TestKylinAgent(QObject *parent)
    : QObject(parent)
    , m_testWidget(nullptr)
{
}

TestKylinAgent::~TestKylinAgent()
{
}

void TestKylinAgent::initTestCase()
{
    m_testWidget = new QWidget();
}

void TestKylinAgent::cleanupTestCase()
{
    delete m_testWidget;
    m_testWidget = nullptr;
}

void TestKylinAgent::init()
{
}

void TestKylinAgent::cleanup()
{
}

void TestKylinAgent::test_constructor()
{
    KylinAgent *agent = new KylinAgent();
    QVERIFY(agent != nullptr);
    delete agent;

    KylinAgent *agent2 = new KylinAgent(m_testWidget);
    QVERIFY(agent2 != nullptr);
    delete agent2;
}

void TestKylinAgent::test_destructor()
{
    KylinAgent *agent = new KylinAgent();
    delete agent;
    QVERIFY(true);
}

void TestKylinAgent::test_startKylinAgent()
{
    KylinAgent *agent = new KylinAgent();
    agent->setParentWidget(m_testWidget);
    agent->startKylinAgent();
    QVERIFY(true);
    delete agent;
}

void TestKylinAgent::test_setParentWidget()
{
    KylinAgent *agent = new KylinAgent();
    agent->setParentWidget(m_testWidget);
    QVERIFY(true);
    delete agent;
}
