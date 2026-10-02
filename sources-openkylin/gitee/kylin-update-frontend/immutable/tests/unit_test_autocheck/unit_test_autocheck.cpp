#include <QtTest>
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QCoreApplication>
#include <QDebug>
#include "../../plugin/src/updatedbus.h"


class TabBtnTest:public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pUpdateDbus = new UpdateDbus();
    }

    static void TearDownTestSuite()
    {
        delete m_pUpdateDbus;
        m_pUpdateDbus = nullptr;
    }

    static UpdateDbus *m_pUpdateDbus;
};
UpdateDbus *TabBtnTest::m_pUpdateDbus =  nullptr;

TEST_F(TabBtnTest, AutoCheckedLabTest)
{
    // 获取通知可更新应用按钮的当前状态
    QSqlQuery query(QSqlDatabase::database("A"));
    QString checkedstatus;//从数据库中获取内容
    while(query.next())
    {
        checkedstatus = query.value("auto_check").toString();
    }
    bool initStatus;    //initStatus为初始通知可更新应用按钮的后端状态(bool)

    QString oppositeStatus; //oppositeStatus为反着的通知可更新应用按钮的后端状态(QString)
    if(checkedstatus == "false")
    {
        oppositeStatus = "true";
        initStatus = false;
    }
    else
    {
        oppositeStatus = "false";
        initStatus = true;
    }

    // 反着设置通知可更新应用的状态(初始状态的反选)
    m_pUpdateDbus->insertInstallStates("auto_check",oppositeStatus);

    // test the current value and the value you have set
    QString checkedstatus2;//checkedstatus2为第二次获取到的状态(QString)
    bool getStatus;//设置反选的状态(bool)
    while(query.next())
    {
        checkedstatus2 = query.value("auto_check").toString();
    }
    if(checkedstatus2 == "false")
    {
        getStatus = false;
    }
    else
    {
        getStatus = true;
    }
    m_pUpdateDbus->callDBusUpdateSource();
    m_pUpdateDbus->GetDownloadLimit();
    ASSERT_EQ(!initStatus, getStatus);
    // restore the value before test
    m_pUpdateDbus->insertInstallStates("auto_check",checkedstatus);
}
