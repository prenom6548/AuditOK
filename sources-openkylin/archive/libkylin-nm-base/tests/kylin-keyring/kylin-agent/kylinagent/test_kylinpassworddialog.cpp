#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QMetaObject>
#include <QGraphicsView>

#include "test_kylinpassworddialog.h"



void TestKyPasswordDialog::initTestCase()
{
    m_dialogDefault = nullptr;
    m_dialogWithType = nullptr;
    pWidget=new QWidget();
}

void TestKyPasswordDialog::test_KyPasswordDialog_Constructor_Default()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    QVERIFY(m_dialogDefault != nullptr);
    //QVERIFY(m_dialogDefault->windowFlags().testFlag(Qt::Popup));
}

void TestKyPasswordDialog::test_KyPasswordDialog_Constructor_WithType()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection2",pWidget);
    QVERIFY(m_dialogWithType != nullptr);
}

void TestKyPasswordDialog::test_KyPasswordDialog_Destructor()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection2",pWidget);
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_InitConnect()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    QVERIFY(m_dialogDefault->isVisible() == false);
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_initUI_EmptyType()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    QVERIFY(m_dialogDefault != nullptr);
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_initUI_WithType()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    QVERIFY(m_dialogWithType != nullptr);
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_getPassword()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    QString password = m_dialogDefault->getPassword();
    QVERIFY(password.isEmpty());
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_setPassword()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    QString testPassword = "test123456";
    m_dialogDefault->setPassword(testPassword);
    QString password = m_dialogDefault->getPassword();
    QCOMPARE(password, testPassword);
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_getUserName()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    QString username = m_dialogWithType->getUserName();
    QVERIFY(username.isEmpty());
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_setUserName()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    QString testUsername = "testuser";
    m_dialogWithType->setUserName(testUsername);
    QString username = m_dialogWithType->getUserName();
    QCOMPARE(username, testUsername);
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_login_PasswordEmpty()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    m_dialogDefault->setPassword("");
    QMetaObject::invokeMethod(m_dialogDefault, "login", Qt::DirectConnection);
    QVERIFY(m_dialogDefault->result() == QDialog::Rejected);
    //QVERIFY(result == QDialog::Rejected);
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_login_PasswordNotEmpty()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    m_dialogDefault->setPassword("testpassword");
    QMetaObject::invokeMethod(m_dialogDefault, "login", Qt::DirectConnection);
    QVERIFY(m_dialogDefault->result() == QDialog::Accepted);
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_login_UserNameEmpty()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    m_dialogWithType->InitConnect();
    m_dialogWithType->setPassword("testpassword");
    QMetaObject::invokeMethod(m_dialogWithType, "login", Qt::DirectConnection);
    QVERIFY(m_dialogWithType->result() == QDialog::Rejected);
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_login_UserNameNotEmpty()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    m_dialogWithType->InitConnect();
    m_dialogWithType->setPassword("testpassword");
    m_dialogWithType->setUserName("testuser");
    QMetaObject::invokeMethod(m_dialogWithType, "login", Qt::DirectConnection);
    QVERIFY(m_dialogWithType->result() == QDialog::Accepted);
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_EmptyText()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    QMetaObject::invokeMethod(m_dialogDefault, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, ""));
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_PasswordEmpty()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    m_dialogDefault->setPassword("");
    QMetaObject::invokeMethod(m_dialogDefault, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, ""));
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_PasswordTooShort()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    m_dialogDefault->setPassword("1234567");
    QMetaObject::invokeMethod(m_dialogDefault, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, "1234567"));
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_PasswordValid()
{
    m_dialogDefault = new KyPasswordDialog("", "TestConnection",pWidget);
    m_dialogDefault->InitConnect();
    m_dialogDefault->setPassword("12345678");
    QMetaObject::invokeMethod(m_dialogDefault, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, "12345678"));
    delete m_dialogDefault;
    m_dialogDefault = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_WithUserName()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    m_dialogWithType->InitConnect();
    m_dialogWithType->setUserName("testuser");
    QMetaObject::invokeMethod(m_dialogWithType, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, "password"));
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_UserNameEmpty()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    m_dialogWithType->InitConnect();
    m_dialogWithType->setUserName("");
    QMetaObject::invokeMethod(m_dialogWithType, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, "password"));
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_onTextChanged_BothValid()
{
    m_dialogWithType = new KyPasswordDialog("WPA2", "TestConnection",pWidget);
    m_dialogWithType->InitConnect();
    m_dialogWithType->setUserName("testuser");
    m_dialogWithType->setPassword("testpassword");
    QMetaObject::invokeMethod(m_dialogWithType, "onTextChanged", Qt::DirectConnection, Q_ARG(QString, "testpassword"));
    delete m_dialogWithType;
    m_dialogWithType = nullptr;
}

void TestKyPasswordDialog::test_KyPasswordDialog_RowWidget()
{
    QWidget *tempwidget=new QWidget();
    RowWidget *pRowWidget = new RowWidget(tempwidget);
    pRowWidget->setKey("TEST_VALUE");

    delete pRowWidget;
    pRowWidget = nullptr;
}


void TestKyPasswordDialog::cleanupTestCase()
{
    if (m_dialogDefault) {
        delete m_dialogDefault;
        m_dialogDefault = nullptr;
    }
    if (m_dialogWithType) {
        delete m_dialogWithType;
        m_dialogWithType = nullptr;
    }
    if(pWidget){
        delete pWidget;
        pWidget = nullptr;
    }
}

