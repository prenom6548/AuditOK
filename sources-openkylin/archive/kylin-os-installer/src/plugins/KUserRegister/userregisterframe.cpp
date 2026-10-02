#include "userregisterframe.h"
#include <pwquality.h>
#include <QApplication>
#include <clocale>
#include <QDebug>
#include <QKeyEvent>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QtDBus>
#include <QRegExp>
#include <QDBusInterface>

#include <locale.h>
#include <libintl.h>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QSizePolicy>
#include "../PluginService/ksystemenv.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include "../PluginService/sysInfo/kpasswordcheck.h"
#include <kylin-chkname.h>
#include <string>
#include <stdio.h>
#include "../PluginService/ui_unit/xrandrobject.h"

namespace KInstaller{

using namespace KServer;

userregisterwidiget::userregisterwidiget(QWidget *parent) : QWidget(parent),
    m_autoLoginFlag(false),
    blfinger(false)
{
    this->setObjectName("userregisterwidiget");
    blfinger = findFingerPrintDevice();
    m_settings = pwquality_default_settings();
    pwquality_read_config(m_settings,PWQUALITYPATH, &auxerror);

    initUI();
    initAllConnect();
    addStyleSheet();
    translateStr();
    setBoardTabOrder();
    KServer::KPasswordCheck::init();
    setFocusPolicy(Qt::StrongFocus);
    //this->setFocus();
}

userregisterwidiget::~userregisterwidiget()
{
    KServer::KPasswordCheck::getInstance()->releasePWquality();
}

void userregisterwidiget::initUI()
{
    QVBoxLayout *usersetupLayout = new  QVBoxLayout();
    this->setLayout(usersetupLayout);
    usersetupLayout->setContentsMargins(0, 0, 0, 0);
    usersetupLayout->setSpacing(0);

    QRect rect = KServer::GetScreenRect();

    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("register_mainTitle");

    if(rect.height() > 768)
        usersetupLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    usersetupLayout->addWidget(m_mainTitle, 0, Qt::AlignCenter | Qt::AlignTop);
    if(rect.height() > 768)
        usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));

    //用户名框设置
    QHBoxLayout* hlayout1 = new QHBoxLayout;
    hlayout1->setMargin(0);
    hlayout1->setContentsMargins(0,10,0,0);
    m_username = new CustomLineEdit("", QIcon(":/res/svg/user.svg"), this);

    m_username->setObjectName("username");
    m_username->setFixedSize(320,48);
    hlayout1->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    //hlayout1->addLayout(m_username->layout);
    hlayout1->addWidget(m_username);
    hlayout1->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout1);

    //用户名出错提示
    m_userErrorTip=new QLabel(this);
    QHBoxLayout* hlayout1e = new QHBoxLayout;
    hlayout1e->setMargin(0);
    hlayout1e->setContentsMargins(0,10,0,0);
    hlayout1e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout1e->addWidget(m_userErrorTip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout1e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout1e);
    m_userErrorTip->setObjectName("m_userErrorTip");
    m_userErrorTip->setAlignment(Qt::AlignTop);
    m_userErrorTip->setStyleSheet("font-size: 12px;\
                                     font-family: Noto Sans CJK SC;\
                                    color: #FB9140;\
                                    opacity: 1;");

     //    letter-spacing: 50px;\

    //主机名框设置
    QHBoxLayout* hlayout2 = new QHBoxLayout;
    hlayout2->setMargin(0);
    hlayout2->setContentsMargins(0,10,0,0);
    m_hostname = new CustomLineEdit("", QIcon(":/res/svg/computer.svg"), this);

    m_hostname->setObjectName("hostname");
    m_hostname->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    m_hostname->setFixedSize(320,48);
    hlayout2->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
   //hlayout2->addLayout(m_hostname->layout);
    hlayout2->addWidget(m_hostname);
    hlayout2->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout2);

    //主机名出错提示
    m_hostErrorTip=new QLabel(this);
    m_hostErrorTip->setObjectName("m_hostErrorTip");
    QHBoxLayout* hlayout2e = new QHBoxLayout;
    hlayout2e->setMargin(0);
    hlayout2e->setContentsMargins(0,10,0,0);
    hlayout2e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout2e->addWidget(m_hostErrorTip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout2e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout2e);
    m_hostErrorTip->setStyleSheet("font-size: 12px;\
                                  font-family: Noto Sans CJK SC;\
                                 color: #FB9140;\
                                 opacity: 1;");

    //密码框设置
    QHBoxLayout* hlayout3 = new QHBoxLayout;
    hlayout3->setMargin(0);
    hlayout3->setContentsMargins(0,10,0,0);
    m_passwd = new CustomLineEdit("", QIcon(":/res/svg/passwd.svg"), this);

    m_passwd->setObjectName("passwd");
    m_passwd->setTrailingPng(QIcon(":/res/svg/code.svg"));
//    m_passwd->m_lineEdit->setEchoMode(QLineEdit::Password);
//    m_passwd->m_lineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_passwd->setEchoMode(QLineEdit::Password);
    m_passwd->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_passwd->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    m_passwd->setBLToolTip(true);
    m_passwd->setFixedSize(320,48);
    hlayout3->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    //hlayout3->addLayout(m_passwd->layout);
    hlayout3->addWidget(m_passwd);
    hlayout3->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout3);

    //密码强度提示
//    m_pPasswordLevel = new kdk::KSecurityLevelBar(this);
//    m_pPasswordLevel->setFixedSize(200,30);
    m_pInvidTipWidget  = new QWidget(this);
    m_pInvidIconLayout = new QHBoxLayout;
    m_pInvidIconLayout->setSpacing(4);
    m_pInvidTitl1 = new QLabel(this);
//    m_pInvidTitl1->setText(tr("Password strength:"));//"密码强度："
    m_pInvidTitl1->setStyleSheet("font-size: 12px;\
                                  font-family: Noto Sans CJK SC;\
                                 color: #ffffff;\
                                 opacity: 1;");
    m_pInvidIcon1 = new QLabel(this);
    m_pInvidIcon1->setFixedSize(30,6);
//    m_pInvidIcon1->setMargin(6);
//    m_pInvidIcon1->setBackgroundRole();
//    m_pInvidIcon1->setFrameShape(QFrame::Box);//设置边框样式
//    m_pInvidIcon1->setFrameShadow(QFrame::Plain);//设置填充样式
    m_pInvidIcon2 = new QLabel(this);
    m_pInvidIcon2->setFixedSize(30,6);
    m_pInvidIcon3 = new QLabel(this);
    m_pInvidIcon3->setFixedSize(30,6);
    m_pInvidTitl2 = new QLabel(this);
    m_pInvidTitl2->setStyleSheet("font-size: 12px;\
                                  font-family: Noto Sans CJK SC;\
                                 color: #ffffff;\
                                 opacity: 1;");
    m_pInvidIconLayout->addWidget(m_pInvidTitl1);
    m_pInvidIconLayout->addItem(new QSpacerItem(2, 34, QSizePolicy::Fixed, QSizePolicy::Preferred));
    m_pInvidIconLayout->addWidget(m_pInvidIcon1,0,Qt::AlignCenter);
    m_pInvidIconLayout->addWidget(m_pInvidIcon2,0,Qt::AlignCenter);
    m_pInvidIconLayout->addWidget(m_pInvidIcon3,0,Qt::AlignCenter);
    m_pInvidIconLayout->addItem(new QSpacerItem(10, 34, QSizePolicy::Fixed, QSizePolicy::Preferred));
    m_pInvidIconLayout->addWidget(m_pInvidTitl2);
    m_pInvidIconLayout->addStretch();
    m_pInvidIconLayout->setContentsMargins(0,0,0,0);
    m_pInvidTipWidget->setLayout(m_pInvidIconLayout);
//    m_pInvidTipWidget->setAutoFillBackground(true);

    //密码错误提示
    m_pwdErrorTip=new QLabel(this);
    m_pwdErrorTip->setObjectName("m_pwdErrorTip");
    QHBoxLayout* hlayout3e = new QHBoxLayout;
    hlayout3e->setMargin(0);
    hlayout3e->setContentsMargins(0,10,0,0);
    hlayout3e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
//    hlayout3e->addWidget(m_pwdErrorTip,1,Qt::AlignTop|Qt::AlignHCenter);
//    hlayout3e->addWidget(m_pPasswordLevel,1,Qt::AlignTop|Qt::AlignHCenter);
    m_pStackInvidTip   = new QStackedWidget(this);
    m_pStackInvidTip->setAutoFillBackground(false);
    m_pwdErrorTipWidget  = new QWidget(this);
    QHBoxLayout* hlpwdErrorTip = new QHBoxLayout;
    hlpwdErrorTip->setMargin(0);
    hlpwdErrorTip->setContentsMargins(0,0,0,0);
    hlpwdErrorTip->addStretch();
    hlpwdErrorTip->addWidget(m_pwdErrorTip,1,Qt::AlignHCenter);
    hlpwdErrorTip->addStretch();
    m_pwdErrorTipWidget->setLayout(hlpwdErrorTip);
//    m_pStackInvidTip->addWidget(m_pwdErrorTip);
    m_pStackInvidTip->addWidget(m_pwdErrorTipWidget);
    m_pStackInvidTip->addWidget(m_pInvidTipWidget);
    m_pStackInvidTip->setCurrentWidget(m_pwdErrorTipWidget);
    hlayout3e->addWidget(m_pStackInvidTip,1,Qt::AlignCenter);
    hlayout3e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout3e);
    m_pwdErrorTip->setStyleSheet("font-size: 12px;\
                                 font-family: Noto Sans CJK SC;\
                                color: #FB9140;\
                                opacity: 1;");

    //确认密码框设置
    QHBoxLayout* hlayout4 = new QHBoxLayout;
    hlayout4->setMargin(0);
    hlayout4->setContentsMargins(0,10,0,0);
    m_confirm_passwd = new CustomLineEdit("", QIcon(":/res/svg/confirm_passwd.svg"), this);

//    m_confirm_passwd->m_lineEdit->setEchoMode(QLineEdit::Password);
//    m_confirm_passwd->m_lineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_confirm_passwd->setEchoMode(QLineEdit::Password);
    m_confirm_passwd->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_confirm_passwd->setObjectName("confirm_passwd");
    m_confirm_passwd->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    m_confirm_passwd->setTrailingPng(QIcon(":/res/svg/code.svg"));
    m_confirm_passwd->setFixedSize(320,48);
    hlayout4->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    //hlayout4->addLayout(m_confirm_passwd->layout);
    hlayout4->addWidget(m_confirm_passwd);
    hlayout4->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout4);

    //确认密码框出错提示
    m_cpwdErrortip=new QLabel(this);
    m_cpwdErrortip->setObjectName("m_cpwdErrortip");
    QHBoxLayout* hlayout4e = new QHBoxLayout;
    hlayout4e->setMargin(0);
    hlayout4e->setContentsMargins(0,10,0,0);
    hlayout4e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout4e->addWidget(m_cpwdErrortip,1,Qt::AlignTop|Qt::AlignHCenter);
    hlayout4e->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Minimum));
    usersetupLayout->addLayout(hlayout4e);
    m_cpwdErrortip->setStyleSheet("font-size: 12px;\
                                  font-family: Noto Sans CJK SC;\
                                 color: #FB9140;\
                                 opacity: 1;");

    //开机自动登录勾选框
    QHBoxLayout* hlayout5 = new QHBoxLayout;
    int len=(this->width()-m_username->width())/2;
    hlayout5->setContentsMargins(len,0,len,0);
    pwLoginCKBox = new QCheckBox();
    pwLoginCKBox->setChecked(false);
    hlayout5->addSpacerItem(new QSpacerItem(len, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout5->addWidget(pwLoginCKBox);
    hlayout5->addSpacerItem(new QSpacerItem(len+m_username->width(), 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout5);
    usersetupLayout->addItem(new QSpacerItem(1000, 15, QSizePolicy::Expanding, QSizePolicy::Minimum));

//    QString strModle = KServer::ReadSettingIni("config","first_mode");
//    if(strModle.compare("oem2")!=0 || blfinger)
//    {
//        QVBoxLayout* hlay = new QVBoxLayout;
//        hlay->setSpacing(20);
//        hlay->setContentsMargins(0, 0, 0, 0);
//        QLabel *contex=new QLabel;
//        contex->setLayout(hlay);
//        contex->setMinimumSize(500,120);
//        contex->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);

//        //提示稍后设置密保
//        if(strModle.compare("oem2")!=0){
//            securityanswerlatertip = new QLabel;
//            securityanswerlatertip->setObjectName("securityanswerlatertip");
//            securityanswerlatertip->setWordWrap(true);
//            hlay->addWidget(securityanswerlatertip, Qt::AlignCenter);
//        }

//        //生物特征设备提示
//        if(blfinger){
//            fingerprinttip = new QLabel;
//            fingerprinttip->setObjectName("fingertip");
//            fingerprinttip->setWordWrap(true);//bug174241
//            hlay->addWidget(fingerprinttip, Qt::AlignCenter);
//        }
//        usersetupLayout->addWidget(contex,13,Qt::AlignCenter);
//    }

    //生物特征设备提示
    fingerprintpng = new QLabel;
    fingerprinttip = new QLabel;
    QString strModle = KServer::ReadSettingIni("config","first_mode");
    if(strModle.compare("oem2")!=0 || blfinger){
//    if(blfinger) {
        fingerprintpng->setObjectName("fingerpng");
        fingerprintpng->setPixmap(QIcon(":/res/svg/detail.svg").pixmap(16, 16));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHeightForWidth(fingerprintpng->sizePolicy().hasHeightForWidth());
        sizePolicy.setWidthForHeight(fingerprintpng->sizePolicy().hasWidthForHeight());
        sizePolicy.setVerticalStretch(0);
        fingerprintpng->setSizePolicy(sizePolicy);

        fingerprinttip->setObjectName("fingertip");
        fingerprinttip->setWordWrap(true);//bug174241
        fingerprintpng->setContentsMargins(3, 3, 3, 66);
        QHBoxLayout* hlay = new QHBoxLayout;
        hlay->setSpacing(8);
        hlay->setContentsMargins(0, 0, 0, 0);
        QLabel *contex=new QLabel;
        contex->setLayout(hlay);
        contex->setMinimumSize(670,90);//bug233867
        contex->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
        hlay->addItem(new QSpacerItem(10 ,20, QSizePolicy::Expanding, QSizePolicy::Expanding));
        hlay->addWidget(fingerprintpng, Qt::AlignTop);
        //fingerprintpng->setAlignment(Qt::AlignTop);
        hlay->addWidget(fingerprinttip);
        fingerprinttip->setAlignment(Qt::AlignTop);

        hlay->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));
        usersetupLayout->addWidget(contex,1,Qt::AlignCenter);
    }/*else{
        usersetupLayout->addItem(new QSpacerItem(10, 1000, QSizePolicy::Expanding, QSizePolicy::Expanding));
    }*/

//    usersetupLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
    usersetupLayout->addItem(new QSpacerItem(10, 500, QSizePolicy::Expanding, QSizePolicy::Expanding));
    translateStr();
}

bool userregisterwidiget::findFingerPrintDevice()
{

    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.Biometric",
                         "/org/ukui/Biometric",
                         "org.ukui.Biometric",
                         "GetDevList");

    int count = 0;
    QDBusMessage res = QDBusConnection::systemBus().call(m);
    if(res.type() == QDBusMessage::ReplyMessage) {
        count = res.arguments().takeFirst().toInt();
    } else {
        return false;
    }
    if(count > 0) {
        return true;
    } else
        return false;

}

void userregisterwidiget::setBoardTabOrder()
{
    QWidget::setTabOrder(m_username, m_hostname);
    QWidget::setTabOrder(m_hostname, m_passwd);
    QWidget::setTabOrder(m_passwd, m_confirm_passwd);
}

void userregisterwidiget::initAllConnect()
{
    //用户名有改变，需要同时核对主机名，密码
//    QObject::connect(m_username->m_lineEdit, &QLineEdit::textChanged, this, &userregisterwidiget::checkUserName);
//    QObject::connect(m_hostname->m_lineEdit, &QLineEdit::textChanged, this, &userregisterwidiget::checkHostName);
//    QObject::connect(m_passwd->m_lineEdit, &QLineEdit::textChanged, this, &userregisterwidiget::checkPasswd);
//    QObject::connect(m_confirm_passwd->m_lineEdit, &QLineEdit::textChanged, this, &userregisterwidiget::checkCPasswd);
    QObject::connect(m_username, &QLineEdit::textChanged, this, &userregisterwidiget::checkUserName);
    QObject::connect(m_hostname, &QLineEdit::textChanged, this, &userregisterwidiget::checkHostName);
    QObject::connect(m_passwd, &QLineEdit::textChanged, this, &userregisterwidiget::checkPasswd);
    QObject::connect(m_confirm_passwd, &QLineEdit::textChanged, this, &userregisterwidiget::checkCPasswd);
    QObject::connect(pwLoginCKBox, &QCheckBox::clicked, this, &userregisterwidiget::getAutoLogin);

}

void userregisterwidiget::checkUserName(QString username)
{
    if(username.isEmpty()) m_userstate=EMPY;
    else
    {
        m_userstate=kylin_username_check(username.toUtf8(), 1);
        m_usererr=kylin_username_strerror(m_userstate);
        qDebug()<<"User name check statue:"<<  m_userstate<< m_usererr;
    }
    markLineEditCorrect();
    setComputerText();//设置主机名
    checkPasswd(m_passwd->getLineEditText());
}

void userregisterwidiget::checkHostName(QString hostname)
{
    if(m_username->getLineEditText().isEmpty()){
        m_hostname->setLineEditText("");
        m_hoststate=EMPY;
        return;
    }

     const QRegExp reg("[a-zA-Z0-9\\.-]*");
    if(hostname.isEmpty()) m_hoststate=EMPY;
    else if(hostname.length() < HOSTNAME_MIN) {m_hoststate=HOSTNAME_TOOSHORT; }
    else if(hostname.length() > HOSTNAME_MAX)m_hoststate=HOSTNAME_TOOLONG;
    else if(!reg.exactMatch(hostname))
    {
        m_hoststate=HOSTNAME_INVALCHAR;
    }//主机名中只能含有字母，数字，连字符和点
    else if(hostname.contains(QRegExp("^[-.]")))
    {
        m_hoststate=HOSTNAME_BEGINERROR;//首部不可由-或.结尾
    }
    else if(hostname.contains(QRegExp("[-.]$")))
    {
        m_hoststate=HOSTNAME_ENDERROR;//尾部不可由-或.结尾
    }
    else if((hostname.contains(QRegExp("[-][.]"))) || (hostname.contains(QRegExp("[.][-]"))))
    {
        m_hoststate=HOSTNAME_SCERROR;//-不可与.相连
    }
    else if(hostname.contains(QRegExp("[.][.]")))
    {
        m_hoststate=HOSTNAME_SSERROR;//不能有连续的.
    }
    else
    {
        m_hoststate=HOSTNAME_OK;
    }

    markLineEditCorrect();
}


void userregisterwidiget::checkPasswd(QString pwd)
{
    if(pwd.isEmpty()) m_pwdstate=EMPY;
    else
    {
        m_pwdstate = KServer::KPasswordCheck::getInstance()->passwdCheck(pwd, m_pwderr, m_username->getLineEditText());
        if(m_pwdstate<0)
            m_pwdstate=200;
        else
            m_pwdstate=0;
    }
    markLineEditCorrect();
    checkCPasswd(m_confirm_passwd->getLineEditText());

}

void userregisterwidiget::checkCPasswd(QString cpwd)
{
    if(cpwd.isEmpty()) m_cpwdstate=EMPY;
    else if(cpwd.compare(m_passwd->getLineEditText())==0)
        m_cpwdstate=CPPASSWD_OK;
    else
    {
        m_cpwdstate=CPPASSWD_DIFF;
        m_cpwderr=QString(tr("Two password entries are inconsistent!"));
    }
   markLineEditCorrect();
}

void userregisterwidiget::getAutoLogin()
{
    if(pwLoginCKBox->isChecked()) {
        m_autoLoginFlag = true;
    } else {
        m_autoLoginFlag = false;
    }
}

//标记错误信息
void userregisterwidiget::markLineEditCorrect()
{
    //如果没有错，使能下一步按钮
    if((m_userstate | m_hoststate | m_pwdstate | m_cpwdstate)==0)
    {
        qDebug()<<"active:"<<true;
        emit enableNextButton(true);
    }
    else
    {
        qDebug()<<"active:"<<false;
        emit enableNextButton(false);
    }

    //1.用户名
     if(m_userstate==0)
     {
         m_userErrorTip->setText("");
     }
     else if(m_userstate==EMPY){
         m_userErrorTip->setText("");
     }
     else{
        m_userErrorTip->setText(m_usererr);
    }

    //2.主机名
     if(m_hoststate==0){
         m_hosterr="";
     m_hostErrorTip->setText("");
    }
    else if(m_hoststate==EMPY){
          m_hosterr="";
         m_hostErrorTip->setText("");
     }
     else{
         if(m_hoststate==HOSTNAME_TOOSHORT)
             m_hosterr=tr("Hostname should be more than 0 bits in length.");
         else if(m_hoststate==HOSTNAME_TOOLONG)
             m_hosterr=tr("Hostname should be no more than 64 bits in length.");
         else if(m_hoststate==HOSTNAME_INVALCHAR)
             m_hosterr=tr("Hostname only letters,numbers,hyphen and dot notation are allowed");
         else if(m_hoststate==HOSTNAME_BEGINERROR)
             m_hosterr=tr("Hostname must start with a number or a letter");
         else if(m_hoststate==HOSTNAME_ENDERROR)
             m_hosterr=tr("Hostname must end with a number or a letter");
         else if(m_hoststate==HOSTNAME_SCERROR)
             m_hosterr=tr("Hostname cannot have consecutive ' - ' and ' . '");
         else if(m_hoststate==HOSTNAME_SSERROR)
             m_hosterr=tr("Hostname cannot have consecutive ' . '");

//        m_hosterr=tr("Your hostname only letters,numbers,underscore and hyphen are allowed, no more than 64 bits in length.");
        m_hostErrorTip->setText(m_hosterr);
    }

    //3.密码
     if(m_pwdstate==0) {
     m_pwdErrorTip->setText("");
    }
    else if(m_pwdstate==EMPY){
         m_pwdErrorTip->setText("");
     }
     else{
        m_pwdErrorTip->setText(m_pwderr);
        m_pStackInvidTip->setCurrentWidget(m_pwdErrorTipWidget);
    }
     //密码强度
     if(  0 == m_pwdstate
          && !m_passwd->getLineEditText().isEmpty()
          && m_pwdErrorTip->text().isEmpty() ){
         QRegExp regNum(NUM_CHECK);
         QRegExp regSTR(STR_CHECK);
         QRegExp regCode(CODE_CHECK);
         QString passwdStr = m_passwd->getLineEditText();
         int result = (regNum.indexIn(passwdStr) != -1) + (regSTR.indexIn(passwdStr) != -1) + (regCode.indexIn(passwdStr) != -1);
         qDebug()<<"result" <<result;

//         if(m_passwd->getLineEditText().length() >= 8 && result == 1){//低强度密码
         if(1==result){//低强度密码（只含有一种字符（数字/字母/特殊符号），不论密码长度多少都为低强度密码）
//             m_pPasswordLevel->setSecurityLevel(kdk::SecurityLevel::Low);
             setInvidIcontoLow();
         }
         else if(2==result ||
                 (3==result && passwdStr.length() < 8 )){//中强度密码(包含2种字符（不论长度多少），或者包含3种字符但是总长度小于8个)
//             m_pPasswordLevel->setSecurityLevel(kdk::SecurityLevel::Medium);
             setInvidIcontoMedium();
         }
         else if(3==result && passwdStr.length() >= 8 ){//高强度密码(包含3种字符且总长度不小于8个)
//             m_pPasswordLevel->setSecurityLevel(kdk::SecurityLevel::High);
             setInvidIcontoHigh();
         }
         m_pStackInvidTip->setCurrentWidget(m_pInvidTipWidget);
     }

     //清空密码错误提示和强度提示  bug216534
     if(m_pwdstate==EMPY && m_pwdErrorTip->text().isEmpty()){
         m_pStackInvidTip->setCurrentWidget(m_pwdErrorTipWidget);
     }

    //4确认密码
     if(m_cpwdstate==0){
     m_cpwdErrortip->setText("");
    }
    else if(m_cpwdstate==EMPY){
         m_cpwdErrortip->setText("");
     }
     else if( m_cpwdstate==CPPASSWD_DIFF){
        m_cpwdErrortip->setText(m_cpwderr);
    }
}

void userregisterwidiget::addStyleSheet()
{
    QFile file(":/res/qss/userregisterframe.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
   QString stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void userregisterwidiget::setComputerText()
{
   if(m_username->getLineEditText().isEmpty()) {
    m_hostname->setLineEditText("");
     m_hoststate=EMPY;
     return;
   }
   m_hostname->setLineEditText(m_username->getLineEditText() + "-pc");

   /* 目前主机名定为:用户名-pc，以下命名方法暂时不用
    QString path = QString("");
    QString error = QString("");
    QString systemManu = QString("");
    QString systemProName = QString("");
    QString systemVer = QString("");
    int excid = 0;
    QString txt = ("[\\ ,,\\*,\\&,\\\\,\\/,\\?,\\|,\\:,\\<,\\>,\"]");
    QRegExp reg(txt);
    KServer::KCommand::getInstance()->RunScripCommand("dmidecode", {"-s", "system-manufacturer"}, path, systemManu, error, excid);

    if (systemManu != "" && !systemManu.contains("N/A") && !systemManu.contains("Not Provided") && !systemManu.contains("Loongson", Qt::CaseInsensitive))
    {
        systemManu = systemManu.simplified().toLower();
        if(systemManu.contains("lenovo") || systemManu.contains("ibm") || systemManu.contains("huawei")) {
            KServer::KCommand::getInstance()->RunScripCommand("dmidecode", {"-s", "system-version"}, path, systemVer, error, excid);
            systemVer = systemVer.simplified().toLower();
            if (systemVer != "" && ! systemVer.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" +systemVer.remove(reg));
            } else {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" + systemVer.remove(reg));
            }

        } else {
            KServer::KCommand::getInstance()->RunScripCommand("dmidecode", {"-s", "system-product-name"}, path, systemProName, error, excid);
            systemProName = systemProName.simplified().toLower();
             systemProName.remove("-");
            if (systemProName != "" && ! systemProName.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" + systemProName.remove(reg));
            } else {
                m_hostname->setLineEditText(m_username->getLineEditText() + "-" + systemManu.remove(reg));
            }

        }
    } else {
        m_hostname->setLineEditText(m_username->getLineEditText() + "-PC");
        QDBusInterface *iface = new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus());

        bool isLapTop = false;
        QVariant msg = iface->property("LidIsPresent");
        isLapTop =  msg.toBool();
        qDebug() << isLapTop;
        if (isLapTop) {
            m_hostname->setLineEditText(m_username->getLineEditText() + "-laptop");
        } else {
            m_hostname->setLineEditText(m_username->getLineEditText() + "-pc");
        }
    }*/
}

void userregisterwidiget::translateStr()
{
    unsetenv("LANG");
    unsetenv("LANGUAGE");
    if(ReadSettingIni("config", "language").compare("zh_CN")==0)
    {
        setenv("LANG", "zh_CN.UTF-8",1);
        setenv("LANGUAGE", "zh_CN:en",1);
    }
    else if(ReadSettingIni("config", "language").compare("bo_CN")==0)
    {
        setenv("LANG", "bo_CN.UTF-8",1);
        setenv("LANGUAGE", "bo_CN:en",1);
    }
    else if(ReadSettingIni("config", "language").compare("en_US")==0)
    {
        setenv("LANG", "en_US.UTF-8",1);
        setenv("LANGUAGE", "en",1);
    }
    else if(ReadSettingIni("config", "language").compare("mn_MN")==0)
    {
        setenv("LANG", "mn_MN.UTF-8",1);
        setenv("LANGUAGE", "mn_MN.UTF-8",1);
    }
    else if(ReadSettingIni("config", "language").compare("zh_HK")==0)
    {
        setenv("LANG", "zh_HK.UTF-8",1);
        setenv("LANGUAGE", "zh_HK.UTF-8",1);
    }

    qDebug()<<"userregisterwidiget::translateStr()";
//    emit setNextText();
    m_mainTitle->setText(tr("Create User"));
    m_username->setPlaceholderText2(tr("username"));
    m_hostname->setPlaceholderText2(tr("hostname"));
    m_passwd->setPlaceholderText2(tr("new password"));
    m_pInvidTitl1->setText(tr("Password strength:"));//密码强度：
    m_confirm_passwd->setPlaceholderText2(tr("enter the password again"));
    pwLoginCKBox->setText(tr("Automatic login on boot"));

    QString strModle = KServer::ReadSettingIni("config","first_mode");
    if(strModle.compare("oem2")!=0){
        if(blfinger)
            fingerprinttip->setText(tr("After logging into the system, it is recommended that you set biometric passwords and password security questions in [Settings - Login Options] to provide a better experience in the system."));//登录系统后，建议你在“设置-登录选项”中设置生物特征密码和密码安全问题，以在系统中获得更好体验。
        else
            fingerprinttip->setText(tr("After logging into the system, it is recommended that you set password security questions in [Settings - Login Options] to provide a better experience in the system."));//登录系统后，建议你在“设置-登录选项”中设置密码安全问题，以在系统中获得更好体验。
    }else{
        if(blfinger)
            fingerprinttip->setText(tr("After logging into the system, it is recommended that you set the biometric password in [Settings - Login Options] to have a better experience in the system."));//登录系统后，建议你在“设置-登录选项”中设置生物特征密码，以在系统中获得更好体验。
    }

    QFontMetrics fm(fingerprinttip->font());
    //根据内容设置label宽度，目的是为了当label内容过短时也能在界面中居中显示,会被两边弹簧压缩
    int textwidth=fm.width(fingerprinttip->text());
    if(textwidth>610)
    {
       fingerprinttip->setMinimumSize(600,20);
    }
    else
    {
        fingerprinttip->setMinimumSize(textwidth+5,20);
    }
    checkUserName(m_username->getLineEditText());
    checkHostName(m_hostname->getLineEditText());
    checkPasswd(m_passwd->getLineEditText());
    checkCPasswd(m_confirm_passwd->getLineEditText());

}

void userregisterwidiget::checkactive(QWidget *p)
{
    if(p==this)
    {
        markLineEditCorrect();
    }
}

bool userregisterwidiget::event(QEvent *event)
{
    if(event->type() == QEvent::Enter)
    {
//        QApplication::setActiveWindow(this);
        activateWindow();//bug173007
        raise();
        //initURFocus();
    }
    else if(event->type() == QEvent::Show)
    {
        //m_username->m_lineEdit->setFocus();
        m_username->setFocus();
        emit reSetFocusPolicySignal();
    }
    return QObject::event(event);
}

void userregisterwidiget::setInvidIcontoLow()//修改密码强度提示
{
    m_pInvidIcon1->setStyleSheet("color:#FB9140;border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background-color:rgb(255,11,47)");

    m_pInvidIcon2->setStyleSheet("color:#FB9140;border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background: rgba(0, 0, 0, 0.35);");

    m_pInvidIcon3->setStyleSheet("color:#FB9140;border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background: rgba(0, 0, 0, 0.35);");

    m_pInvidTitl2->setText(tr("Low"));//弱
}

void userregisterwidiget::setInvidIcontoMedium()//修改密码强度提示
{
//    m_pInvidIcon1->setStyleSheet("border-width:3px;border-style:solid;border-color:rgb(255,170,0);background-color:rgb(0,255,127)");//rgb(255,11,47)  rgb(255,196,16)

    m_pInvidIcon1->setStyleSheet("border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background-color:rgb(255,196,16)");

    m_pInvidIcon2->setStyleSheet("border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background-color:rgb(255,196,16)");

    m_pInvidIcon3->setStyleSheet("border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background: rgba(0, 0, 0, 0.35);");

    m_pInvidTitl2->setText(tr("Medium"));//中等
}

void userregisterwidiget::setInvidIcontoHigh()//修改密码强度提示
{
    m_pInvidIcon1->setStyleSheet("border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background-color:rgb(0,255,127)");

    m_pInvidIcon2->setStyleSheet("border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background-color:rgb(0,255,127)");

    m_pInvidIcon3->setStyleSheet("border-width:0px;border-style:solid;border-color:transparent;border-radius:3px;background-color:rgb(0,255,127)");

    m_pInvidTitl2->setText(tr("High"));//强
}

void userregisterwidiget::initURFocus()//初始化焦点
{
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    //当全部文本框都为空时由第一个文本框自动获取焦点
    if( /*m_username->m_lineEdit->text().isEmpty() &&
        m_hostname->m_lineEdit->text().isEmpty() &&
        m_passwd->m_lineEdit->text().isEmpty() &&
        m_confirm_passwd->m_lineEdit->text().isEmpty()*/
            m_username->text().isEmpty() &&
                    m_hostname->text().isEmpty() &&
                    m_passwd->text().isEmpty() &&
                    m_confirm_passwd->text().isEmpty()
      )
    {
        //m_username->m_lineEdit->setFocus();
        m_username->setFocus();
    }
}

}
