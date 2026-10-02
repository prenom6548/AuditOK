#ifndef TEST_KYPASSWORDDIALOG_H
#define TEST_KYPASSWORDDIALOG_H

#include <QObject>
#include <QTest>
#include "kylinpassworddialog.h"


class KyPasswordDialog;

class TestKyPasswordDialog : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void test_KyPasswordDialog_Constructor_Default();
    void test_KyPasswordDialog_Constructor_WithType();
    void test_KyPasswordDialog_Destructor();
    void test_KyPasswordDialog_InitConnect();
    void test_KyPasswordDialog_initUI_EmptyType();
    void test_KyPasswordDialog_initUI_WithType();
    void test_KyPasswordDialog_getPassword();
    void test_KyPasswordDialog_setPassword();
    void test_KyPasswordDialog_getUserName();
    void test_KyPasswordDialog_setUserName();
    void test_KyPasswordDialog_login_PasswordEmpty();
    void test_KyPasswordDialog_login_PasswordNotEmpty();
    void test_KyPasswordDialog_login_UserNameEmpty();
    void test_KyPasswordDialog_login_UserNameNotEmpty();
    void test_KyPasswordDialog_onTextChanged_EmptyText();
    void test_KyPasswordDialog_onTextChanged_PasswordEmpty();
    void test_KyPasswordDialog_onTextChanged_PasswordTooShort();
    void test_KyPasswordDialog_onTextChanged_PasswordValid();
    void test_KyPasswordDialog_onTextChanged_WithUserName();
    void test_KyPasswordDialog_onTextChanged_UserNameEmpty();
    void test_KyPasswordDialog_onTextChanged_BothValid();
    void test_KyPasswordDialog_RowWidget();
    void cleanupTestCase();

private:
    KyPasswordDialog *m_dialogDefault;
    KyPasswordDialog *m_dialogWithType;
    QWidget* pWidget = nullptr;
};

#endif // TEST_KYPASSWORDDIALOG_H