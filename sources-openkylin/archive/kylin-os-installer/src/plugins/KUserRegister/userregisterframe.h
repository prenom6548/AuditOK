#ifndef USERREGISTERWIDIGET_H
#define USERREGISTERWIDIGET_H

#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QEvent>
#include <QCheckBox>
#include "checkedlineedit.h"
#include "clineeditlabel.h"
#include <QHash>
#include <QTextEdit>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include <QStackedWidget>

#include <pwquality.h>
//#include "kysdk/applications/ksecuritylevelbar.h"

//空字符
#define EMPY    100
//hostnane长度范围
#define HOSTNAME_MIN    1
#define HOSTNAME_MAX    64

//hostname 错误代码
#define HOSTNAME_OK     0
#define HOSTNAME_TOOSHORT   2
#define HOSTNAME_TOOLONG    3
#define HOSTNAME_INVALCHAR  4
#define HOSTNAME_BEGINERROR  5
#define HOSTNAME_ENDERROR  6
#define HOSTNAME_SCERROR  7
#define HOSTNAME_SSERROR  8

//密码安全检测配置文件路径
#define PWQUALITYPATH   "/etc/security/pwquality.conf"
//cpwd
#define CPPASSWD_OK 0
#define CPPASSWD_DIFF   1

//密码强度检查
#define NUM_CHECK              "[0-9]"
#define STR_CHECK              "[a-zA-Z]"
#define CODE_CHECK             "[~!@#$%^&*?_\\-]"

namespace KInstaller {

class userregisterwidiget : public QWidget
{
    Q_OBJECT
public:
    explicit userregisterwidiget(QWidget *parent = nullptr);
    ~userregisterwidiget();
    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void setNextBtnStyle(); //检测下一步按钮的状态显示
    void setBoardTabOrder();
    void setInvidIcontoLow();//修改密码强度提示
    void setInvidIcontoMedium();
    void setInvidIcontoHigh();
    void initURFocus();

    CustomLineEdit *m_username = nullptr;         //用户名输入框
    CustomLineEdit *m_hostname = nullptr;         //主机名输入框
    CustomLineEdit *m_passwd = nullptr;           //密码输入框
    CustomLineEdit *m_confirm_passwd = nullptr;   //确认密码输入框

    //四个编辑框内容的的错误类型以及错误提示
    int m_userstate=EMPY;
    int m_hoststate=EMPY;
    int m_pwdstate=EMPY;
    int m_cpwdstate=EMPY;
    QString m_usererr;
    QString m_hosterr;
    QString m_pwderr;
    QString m_cpwderr;

    //密码检测设置
    pwquality_settings_t *m_settings;

//    //密码强度校验
//    kdk::KSecurityLevelBar *m_pPasswordLevel;
    QStackedWidget  *m_pStackInvidTip;//包含了2个子界面： m_pwdErrorTip, m_pInvidTipWidget
    QWidget         *m_pInvidTipWidget;//密码强度校验界面
    QHBoxLayout     *m_pInvidIconLayout;
    QLabel *m_pInvidTitl1 = nullptr;
    QLabel *m_pInvidIcon1 = nullptr;
    QLabel *m_pInvidIcon2 = nullptr;
    QLabel *m_pInvidIcon3 = nullptr;
    QLabel *m_pInvidTitl2 = nullptr;

    void *auxerror;
    //错误图标
    QLabel *m_userCheck = nullptr;
    QLabel *m_hostCheck = nullptr;
    QLabel *m_pwdCheck = nullptr;
    QLabel *m_cpwdCheck = nullptr;
   //错误提示语句
    QLabel *m_userErrorTip=nullptr;
    QLabel *m_hostErrorTip=nullptr;
    QWidget* m_pwdErrorTipWidget=nullptr;
    QLabel *m_pwdErrorTip=nullptr;
    QLabel *m_cpwdErrortip=nullptr;

    //生物特征支持提示
    QLabel* fingerprintpng = nullptr;
    QLabel* fingerprinttip = nullptr;

    QLabel* m_mainTitle = nullptr;

    QCheckBox* autoLoginCKBox = nullptr;
    QCheckBox* pwLoginCKBox = nullptr;
    bool m_autoLoginFlag = false;
    bool blfinger = false;

private:
    bool event(QEvent *event);

signals:
    void enableNextButton(bool value);
    void setNextText();
    void reSetFocusPolicySignal();
public slots:

    void setComputerText();
    void checkUserName(QString m_username);
    void checkHostName(QString m_hostname);
    void checkPasswd(QString pwd);
    void checkCPasswd(QString cpwd);
    void markLineEditCorrect();
    bool findFingerPrintDevice();
    void getAutoLogin();
    void translateStr();
    void checkactive(QWidget *p);


};
}

#endif // USERREGISTERWIDIGET_H
