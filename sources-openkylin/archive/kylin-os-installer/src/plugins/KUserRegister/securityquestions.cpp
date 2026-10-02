#include "securityquestions.h"
//#include "userregisterframe.h"
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
#include <QListView>
#include <QScrollBar>
#include <locale.h>
#include <libintl.h>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QSizePolicy>
#include <QScrollBar>

#include "../PluginService/ksystemenv.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include "../PluginService/sysInfo/kpasswordcheck.h"
#include <kylin-chkname.h>
#include <string>
#include <stdio.h>
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/usersecurityquestions.h"

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <time.h>
//#include <libintl.h>
//#include <locale.h>
////#include "/usr/include/kylin-passwdreset/kylin-passwdreset.h"
//#include "kylin-passwdreset/kylin-passwdreset.h"
////#include "../lib/pbkdf2_hmac.h"

//int randnumber();
//SecurityAnswerListNode *bindSecurityQuestionsAnswer(SecurityQuestionListNode *head, int count, int uid, char * enc_answer);
//SecurityAnswerListNode *bindSecurityQuestionsAnswerv2(SecurityQuestionListNode *head, int count, int uid, char * enc_answer);
//void testQK();

//QDBusArgument &operator<<(QDBusArgument &argument, const QuestionInfo &questionInfo)
//{
//    argument.beginStructure();
//    argument << questionInfo.nQuestionId << questionInfo.strQuestionText;
//    argument.endStructure();
//    return argument;
//}

//const QDBusArgument &operator>>(const QDBusArgument &argument, QuestionInfo &questionInfo)
//{
//    argument.beginStructure();
//    argument >> questionInfo.nQuestionId >> questionInfo.strQuestionText;
//    argument.endStructure();
//    return argument;
//}

//QDBusArgument &operator<<(QDBusArgument &argument, const AnswerInfo &answerInfo)
//{
//    argument.beginStructure();
//    argument << answerInfo.nQuestionId << answerInfo.strAnswerText;
//    argument.endStructure();
//    return argument;
//}

//const QDBusArgument &operator>>(const QDBusArgument &argument, AnswerInfo &answerInfo)
//{
//    argument.beginStructure();
//    argument >> answerInfo.nQuestionId >> answerInfo.strAnswerText;
//    argument.endStructure();
//    return argument;
//}

namespace KInstaller{

using namespace KServer;

securityquestions::securityquestions(QWidget *parent) : QWidget(parent)
//    m_autoLoginFlag(false),
//    blfinger(false)
{
    this->setObjectName("securityquestions");
    initUI();
    initAllConnect();
    initSecurityDBusInterface();
    translateStr();
    setBoardTabOrder();
//    KServer::KPasswordCheck::init();
//    setFocusPolicy(Qt::StrongFocus);
//    this->setFocus();
    if(m_count < 3){//安全问题太少，就默认勾选稍后设置
        m_Setuplater->setChecked(true);
        m_Setuplater->setDisabled(true);
    }
    setNextButton();
   this->setStyleSheet("QLineEdit{width: 420px;height: 40px;}QComboBox{width: 380px;height: 40px;}");
}

securityquestions::~securityquestions()
{
//    KServer::KPasswordCheck::getInstance()->releasePWquality();
}

void securityquestions::initUI()
{
    m_totalLayout = new  QVBoxLayout;
    this->setLayout(m_totalLayout);
    m_totalLayout->setContentsMargins(0, 0, 0, 0);
    m_totalLayout->setSpacing(0);

    QRect rect = KServer::GetScreenRect();
    if(rect.height() > 768)
        m_totalLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    else
        m_totalLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_title = new QLabel();
    m_title->setObjectName("m_title");
    m_title->setStyleSheet("font-size: 28px;\
                         font-family: Noto Sans CJK SC;\
                         font-weight: 500;\
                         line-height: 41px;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_totalLayout->addWidget(m_title, 0, Qt::AlignCenter | Qt::AlignTop);
    m_totalLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));

    m_titleCommentary = new QLabel();
    m_titleCommentary->setObjectName("m_titleCommentary");
    m_titleCommentary->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_totalLayout->addWidget(m_titleCommentary, 0, Qt::AlignCenter | Qt::AlignTop);
    m_totalLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Fixed));

    m_formsWidget = new QWidget(this);
    //m_formsWidget->setFixedSize(460,560);
    m_formsWidget->setFixedSize(560,560);
    m_forms = new QFormLayout();
    m_forms->setContentsMargins(0, 0, 0, 0);
    m_forms->setVerticalSpacing(8);
    m_forms->setHorizontalSpacing(8);

    m_questionTitle1 = new QLabel();
    m_questionTitle1->setObjectName("m_questionTitle1");
    m_questionTitle1->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_question1 = new QComboBox;
    m_question1->installEventFilter(this);
    m_question1->setView(new QListView);
    m_question1->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_question1->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    m_question1->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

    m_question1->setObjectName("m_question1");
//    m_question1->setFixedSize(200,20);
    m_question1->setFixedHeight(40);
    m_question1->setMaxVisibleItems(3);
    m_question1->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_question1->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px; border-radius:5px;}");
   // m_question1->setStyleSheet("QComboBox#m_question1:focus{border:2px solid #5BADFF;}");
    m_forms->addRow(m_questionTitle1, m_question1);

    m_answerTitle1 = new QLabel();
    m_answerTitle1->setObjectName("m_answerTitle1");
    m_answerTitle1->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_answer1= new QLineEdit();
    m_answer1->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
//    m_answer1->setFixedSize(200,20);
    m_answer1->setFixedHeight(40);
    m_answer1->setStyleSheet("QLineEdit{border-radius:5px;background:rgba(0, 0, 0, 0.35);color:#FFFFFF;}\
                                QLineEdit:focus{border:2px solid #5BADFF;background: rgba(0, 0, 0, 0.15);}");
    m_forms->addRow(m_answerTitle1, m_answer1);

    m_forms->addItem(new QSpacerItem(10, 16, QSizePolicy::Expanding, QSizePolicy::Fixed));

    m_questionTitle2 = new QLabel();
    m_questionTitle2->setObjectName("m_questionTitle2");
    m_questionTitle2->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         border-radius:5px;\
                         opacity: 1;");
    m_question2 = new QComboBox;
    m_question2->setView(new QListView);
    m_question2->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_question2->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    m_question2->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    m_question2->setObjectName("m_question2");
//    m_question2->setFixedSize(200,20);
    m_question2->setFixedHeight(40);
    m_question2->setMaxVisibleItems(3);
    m_question2->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_question2->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px; border-radius:5px;}");
    m_question2->setStyleSheet("QComboBox#m_question2:focus{border:2px solid #5BADFF;}");
    m_forms->addRow(m_questionTitle2, m_question2);

    m_answerTitle2 = new QLabel();
    m_answerTitle2->setObjectName("m_answerTitle2");
    m_answerTitle2->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_answer2= new QLineEdit();
    m_answer2->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
//    m_answer2->setFixedSize(200,20);
    m_answer2->setFixedHeight(40);
    m_answer2->setStyleSheet("QLineEdit{border-radius:5px;background:rgba(0, 0, 0, 0.35);color:#FFFFFF;}\
                                QLineEdit:focus{border:2px solid #5BADFF;background: rgba(0, 0, 0, 0.15);}");
    m_forms->addRow(m_answerTitle2, m_answer2);

    m_forms->addItem(new QSpacerItem(10, 16, QSizePolicy::Expanding, QSizePolicy::Fixed));

    m_questionTitle3 = new QLabel();
    m_questionTitle3->setObjectName("m_questionTitle3");
    m_questionTitle3->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_question3 = new QComboBox;
    m_question3->setView(new QListView);
    m_question3->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_question3->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    m_question3->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    m_question3->setObjectName("m_question3");
//    m_question3->setFixedSize(200,20);
    m_question3->setFixedHeight(40);
    m_question3->setMaxVisibleItems(3);
    m_question3->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_question3->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px; border-radius:5px;}");\
    m_question3->setStyleSheet("QComboBox#m_question3:focus{border:2px solid #5BADFF;}");
    m_forms->addRow(m_questionTitle3, m_question3);

    m_answerTitle3 = new QLabel();
    m_answerTitle3->setObjectName("m_answerTitle3");
    m_answerTitle3->setStyleSheet("font-size: 14px;\
                         font-family: Noto Sans CJK SC;\
                         color: #FFFFFF;\
                         opacity: 1;");
    m_answer3= new QLineEdit();
    m_answer3->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
//    m_answer3->setFixedSize(200,20);
    m_answer3->setFixedHeight(40);
    m_answer3->setStyleSheet("QLineEdit{border-radius:5px;background:rgba(0, 0, 0, 0.35);color:#FFFFFF;}\
                                QLineEdit:focus{border:2px solid #5BADFF;background: rgba(0, 0, 0, 0.15);}");
    m_forms->addRow(m_answerTitle3, m_answer3);

    m_formsWidget->setLayout(m_forms);
//    m_totalLayout->addLayout(m_forms);
    m_totalLayout->addWidget(m_formsWidget,0,Qt::AlignCenter);
    m_totalLayout->addItem(new QSpacerItem(10, 60, QSizePolicy::Expanding, QSizePolicy::Expanding));

    m_Setuplater = new QCheckBox;
    m_Setuplater->setDisabled(false);
    m_Setuplater->setVisible(true);
    m_Setuplater->setObjectName("m_Setuplater");
//    connect(m_savehistory,&QCheckBox::clicked,this, &FullPartitionFrame::setLvmToDisk);
    m_Setuplater->setStyleSheet("QCheckBox {\
                                font-size: 14px;\
                                font-family: Noto Sans CJK SC;\
                                font-weight: 400;\
                                line-height: 64px;\
                                background:transparent;\
                                color: #FFFFFF;\
                                opacity: 1;}\
                                QCheckBox::indicator{background:rgba(0, 0, 0, 0.35);border-radius:3px;}\
                                QCheckBox::indicator:enabled:unchecked:focus {    image: url(:/res/svg/checkbox-unchecked-focus.svg);    width: 16px;height: 16px;}\
                                QCheckBox::indicator:disabled:unchecked {  image: url(:/res/svg/discheckboxcheck.svg);}\
                                QCheckBox::indicator:enabled:unchecked {   background:rgba(0, 0, 0, 0.35);border-radius:3px;}\
                                QCheckBox::indicator:disabled:checked {    image: url(:/res/svg/discheckboxchecked.svg);}\
                                QCheckBox::indicator:enabled:checked {    image: url(:/res/svg/checkboxchecked.svg);}\
                                QCheckBox::indicator:enabled:checked:focus {    image: url(:/res/svg/checkbox-checked-focus.svg);    width: 16px;height: 16px;} "
                );
    m_totalLayout->addWidget(m_Setuplater,0,Qt::AlignCenter);
    m_totalLayout->addItem(new QSpacerItem(10, 1000, QSizePolicy::Expanding, QSizePolicy::Expanding));
//    translateStr();
}

void securityquestions::setBoardTabOrder()
{
    QWidget::setTabOrder(m_question1, m_answer1);
    QWidget::setTabOrder(m_answer1, m_question2);
    QWidget::setTabOrder(m_question2, m_answer2);
    QWidget::setTabOrder(m_answer2, m_question3);
    QWidget::setTabOrder(m_question3, m_answer3);
    QWidget::setTabOrder(m_answer3, m_Setuplater);
}

void securityquestions::initAllConnect()
{
    connect(m_question1, SIGNAL(currentIndexChanged(int)), this, SLOT(changeQuestion1(int)));
    connect(m_answer1, &QLineEdit::textEdited, this, &securityquestions::checkAnswer1);
    connect(m_question2, SIGNAL(currentIndexChanged(int)), this, SLOT(changeQuestion2(int)));
    connect(m_answer2, &QLineEdit::textEdited, this, &securityquestions::checkAnswer2);
    connect(m_question3, SIGNAL(currentIndexChanged(int)), this, SLOT(changeQuestion3(int)));
    connect(m_answer3, &QLineEdit::textEdited, this, &securityquestions::checkAnswer3);

    QObject::connect(m_Setuplater, &QCheckBox::clicked, this, &securityquestions::setSetuplater);
}

void securityquestions::changeQuestion1(int index)
{
    m_uniqueNumbers[0] = m_question1->itemData(index).toUInt();
    setQuestions();
}

void securityquestions::changeQuestion2(int index)
{
    m_uniqueNumbers[1] = m_question2->itemData(index).toUInt();
    setQuestions();
}

void securityquestions::changeQuestion3(int index)
{
    m_uniqueNumbers[2] = m_question3->itemData(index).toUInt();
    setQuestions();
}

void securityquestions::checkAnswer1(QString strAnswer)
{
    QString strT = strAnswer;
//    char answer[ANSWERMAXLENGTH+1] = {0};
//    const char * pUsername = strAnswer.toStdString().c_str();
//    if(strlen(pUsername) > ANSWERMAXLENGTH){
//        strncpy(answer,pUsername,ANSWERMAXLENGTH);
//        answer[ANSWERMAXLENGTH] = '\0';
//        strT = answer;
//    } //去掉对答案长度的限制
    m_answer1->setText(strT);
    m_passwordList[0] = strT;
    setNextButton();
}

void securityquestions::checkAnswer2(QString strAnswer)
{
    QString strT = strAnswer;
//    char answer[ANSWERMAXLENGTH+1] = {0};
//    const char * pUsername = strAnswer.toStdString().c_str();
//    if(strlen(pUsername) > ANSWERMAXLENGTH){
//        strncpy(answer,pUsername,ANSWERMAXLENGTH);
//        answer[ANSWERMAXLENGTH] = '\0';
//        strT = answer;
//    } //去掉对答案长度的限制
    m_answer2->setText(strT);
    m_passwordList[1] = strT;
    setNextButton();

//    m_passwordList[1] = strAnswer;
//    setNextButton();
}

void securityquestions::checkAnswer3(QString strAnswer)
{
    QString strT = strAnswer;
//    char answer[ANSWERMAXLENGTH+1] = {0};
//    const char * pUsername = strAnswer.toStdString().c_str();
//    if(strlen(pUsername) > ANSWERMAXLENGTH){
//        strncpy(answer,pUsername,ANSWERMAXLENGTH);
//        answer[ANSWERMAXLENGTH] = '\0';
//        strT = answer;
//    } //去掉对答案长度的限制
    m_answer3->setText(strT);
    m_passwordList[2] = strT;
    setNextButton();

//    m_passwordList[2] = strAnswer;
//    setNextButton();
}

void securityquestions::setSetuplater(bool checked)
{
//    emit lsSetsetUserQestA(checked);

//    if(checked)
//        WriteSettingToCustIni("/tmp/setsecurityques.ini","config","security","false");
//    else
//        WriteSettingToCustIni("/tmp/setsecurityques.ini","config","security","true");

    setNextButton();
}

void securityquestions::setNextButton()
{
    if(m_Setuplater->isChecked())
    {
        m_answer1->setEnabled(false);
        m_answer2->setEnabled(false);
        m_answer3->setEnabled(false);
        emit enableNextButton(true);
    }
    else
    {
        m_answer1->setEnabled(true);
        m_answer2->setEnabled(true);
        m_answer3->setEnabled(true);

        if(!m_answer1->text().trimmed().isEmpty() &&
           !m_answer2->text().trimmed().isEmpty() &&
           !m_answer3->text().trimmed().isEmpty()
        )
            emit enableNextButton(true);
        else
            emit enableNextButton(false);
    }
}

void securityquestions::translateStr()
{
    m_strCurrentLang=ReadSettingIni("config", "language");

    m_title->setText(tr("Security Questions"));//密码安全问题
    m_titleCommentary->setText(tr("You can reset your password by answering security questions when you forget it, please make sure to remember the answer"));//你可以在忘记密码时通过回答安全问题重置密码，请确保一定会记住答案。
    m_questionTitle1->setText(tr("Question 1"));//问题1
    m_answerTitle1->setText(tr("Answer:"));//答案：
    m_questionTitle2->setText(tr("Question 2"));//问题2
    m_answerTitle2->setText(tr("Answer:"));//答案：
    m_questionTitle3->setText(tr("Question 3"));//问题3
    m_answerTitle3->setText(tr("Answer:"));//答案：
    m_Setuplater->setText(tr("Set up later"));//稍后设置
    initSecurityQuestions();
}

void securityquestions::checkactive(QWidget *p)
{
    if(p==this)
    {
        setNextButton();
    }
}

void securityquestions::showEvent(QShowEvent *event)
{
    m_firstfocusget=true;
    m_question1->setFocus();
    emit reSetFocusPolicySignal();
}

bool securityquestions::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==m_question1&&event->type()==QEvent::FocusIn)
    {
        if(m_firstfocusget==true)
        {
            m_question1->setStyleSheet(QString(
                "QComboBox#m_question1:focus {"
                "border: 2px solid transparent; "
                "outline: none;"
                "}"
                ));
            m_firstfocusget=false;
        }
        else
        {
            m_question1->setStyleSheet(
                "QComboBox#m_question1:focus {"
                "border: 2px solid #58ADFF; "
                "outline: none;"
                "}"
                );
        }
    }

    return QObject::eventFilter(watched,event);
}

//void securityquestions::initURFocus()//初始化焦点
//{
//    //当全部文本框都没有内容时由第一个文本框自动获取焦点
//    if( m_username->m_lineEdit->text().isEmpty() &&
//        m_hostname->m_lineEdit->text().isEmpty() &&
//        m_passwd->m_lineEdit->text().isEmpty() &&
//        m_confirm_passwd->m_lineEdit->text().isEmpty()
//      )
//    {
//        m_username->m_lineEdit->setFocus();
//    }
//}




////---------------------------------------
////int uniqueNumbers[3] = {0};

////重新排列队列uniqueNumbers中各元素的值。值范围（0-9
//int securityquestions::randnumber() {
//    int numbers[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//    int count = 0;

//    srand((unsigned int)time(NULL));

//    for (int i = 9; i > 0; i--) {
//        int j = rand() % (i + 1);
//        int temp = numbers[i];
//        numbers[i] = numbers[j];
//        numbers[j] = temp;
//    }

//    while (count < 3) {
//        int randomIndex = rand() % 10;
//        int randomNumber = numbers[randomIndex];

//        int isUnique = 1;
//        for (int k = 0; k < count; ++k) {
//            if (uniqueNumbers[k] == randomNumber) {
//                isUnique = 0;
//                break;
//            }
//        }

//        if (isUnique) {
//            uniqueNumbers[count++] = randomNumber;
//        }
//    }

//    printf("Three unique random numbers (0-9): ");
//    for (int i = 0; i < 3; ++i) {
//        printf("%d ", uniqueNumbers[i]);
//    }
//    printf("\n");

//    return 0;
//}

//SecurityAnswerListNode *securityquestions::bindSecurityQuestionsAnswer(SecurityQuestionListNode *head, int count, int uid, char * enc_answer){

//    SecurityQuestionListNode *current = head;
//    SecurityAnswerListNode *ret = NULL;

//    unsigned char* key = (unsigned char*)"xyza";

//    randnumber();
//    sleep(1);

//    char hmac[33] = { 0 };
//    KPR_PKCS5_PBKDF2_HMAC2((unsigned char*)enc_answer, strlen(enc_answer), key, 4, 2, 32, (unsigned char*)hmac);
//    char *hexBlk = Hex(hmac, 32, 0);

//    for (int i = 0; i < count; i++){
//        SecurityAnswer answer;
//        memset(&answer, 0, sizeof(SecurityAnswer));
//        answer.userId = uid;
//        answer.questionId = uniqueNumbers[i];
//        strcpy(answer.encryptedAnswer, hexBlk);

//        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

//    }

//    return ret;
//}

////head：没用到
////count：此用户一共设置了几个问题
////uid：此用户的uid
////enc_answer：此用户这些问题的答案。这里统一使用一个答案了
////返回（加密后的)密钥列表
//SecurityAnswerListNode *securityquestions::bindSecurityQuestionsAnswerv2(SecurityQuestionListNode *head, int count, int uid, char * enc_answer){

//    SecurityQuestionListNode *current = head;
//    SecurityAnswerListNode *ret = NULL;
//    char *hexBlk = NULL;

//    //unsigned char* key = (unsigned char*)"xyza";

//    randnumber();
//    sleep(1);

//    //char hmac[33] = { 0 };
//    KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)enc_answer, strlen(enc_answer), &hexBlk);
//    //char *hexBlk = Hex(hmac, 32, 0);

//    for (int i = 0; i < count; i++){
//        SecurityAnswer answer;
//        memset(&answer, 0, sizeof(SecurityAnswer));
//        answer.userId = uid;
//        answer.questionId = uniqueNumbers[i];//
//        strcpy(answer.encryptedAnswer, hexBlk);

//        addAnswerNode(&ret, (void *)&answer, sizeof(SecurityAnswer));

//    }

//    return ret;
//}

//void userregisterwidiget::testQK()
//{
//    int ret=0;
//    int count = 0;
//    char *token = NULL;
//    SecurityQuestionListNode *preset_list = NULL;//保存系统预设的安全问题列表
//    SecurityAnswerListNode *zky_ans_list = NULL;
//    SecurityAnswerListNode *user1_list = NULL; /*uid == 1001*/
//    SecurityAnswerListNode *user2_list = NULL; /*uid == 1002*/
//    SecurityQuestionListNode *zky_qst_list = NULL;
//    bindtextdomain("kylin-passwdreset", "/usr/share/locale");
//    textdomain("kylin-passwdreset");
//    setlocale(LC_ALL, "zh_CN.UTF-8");
//    if(geteuid() != 0)//在我本机测试返回0
//    {
//        printf("---------------kylin-passwdreset-test: Permission denied, current euid is %d. \n",geteuid());
//        return ;
//    }
//    //init_question_to_jsonfile();

////第一步，获取系统提供的问题列表。默认有10个问题，即preset_list中每个元素表示一个问题
//    /* 接口1--获取预设的安全问题列表，问题个数10个 */
//    preset_list = getPresetSecurityQuestions(&count);//获取系统预设的10个问题
//    printf("count = %d\n", count);//count = 10
//    printQuestionList(preset_list);//这里不会在终端打印出任何信息

//    SecurityQuestionListNode *current = preset_list;
//    while (current != NULL) {
//        printf("questionId: %d, questionText: %s\n", current->question.questionId, (current->question.questionText));//questionId值从0开始；questionText值这里如【Where is the city where you were born?】
//        current = current->next;
//    }
//    printf("-------------接口1测试完毕-------------\n");
////第二步，绑定某个用户的3个随机问题和这些问题的答案，同时生成一个（密码被加密后的）列表如 user1_list(即此答案列表中有三个元素)，并将这些信息记录到系统的相关后台中。
//    user1_list = bindSecurityQuestionsAnswerv2(preset_list, 3, 1001, "usr1xxx");//打印出如：Three unique random numbers (0-9): 1 5 6
//    printAnswerList(user1_list);//打印出如：
//    //uid: = 1001, queston: 1, answer: 8e0cbece3d6098f31eff67d9f54520fafa69e192950b4304e0fdae7034526960
//    //uid: = 1001, queston: 5, answer: 8e0cbece3d6098f31eff67d9f54520fafa69e192950b4304e0fdae7034526960
//    //uid: = 1001, queston: 6, answer: 8e0cbece3d6098f31eff67d9f54520fafa69e192950b4304e0fdae7034526960
//    /*接口2*/
//    setUserSecurityAnswers(user1_list);//将信息录入相关后台。应该是无打印

//    zky_ans_list = bindSecurityQuestionsAnswerv2(preset_list, 3, 1000, "zky123456");//打印出如：Three unique random numbers (0-9): 7 9 6
//    printAnswerList(zky_ans_list);//打印出如：
////uid: = 1000, queston: 7, answer: 32a119bdf4b7b633bb8a9de67b3337f3197bcb20503e2c35b2474df24e351a87
////uid: = 1000, queston: 9, answer: 32a119bdf4b7b633bb8a9de67b3337f3197bcb20503e2c35b2474df24e351a87
////uid: = 1000, queston: 6, answer: 32a119bdf4b7b633bb8a9de67b3337f3197bcb20503e2c35b2474df24e351a87

//    /*接口2*/
//    setUserSecurityAnswers(zky_ans_list);

//    user2_list = bindSecurityQuestionsAnswerv2(preset_list, 3, 1002, "usr2xxxxx");//打印出如：Three unique random numbers (0-9): 3 7 4
//    printAnswerList(user2_list);//打印出如：
////uid: = 1002, queston: 3, answer: 56fd5be1742b031bb8fd65a4effa57aba89e85116abe9b5a409a080771532f67
////uid: = 1002, queston: 7, answer: 56fd5be1742b031bb8fd65a4effa57aba89e85116abe9b5a409a080771532f67
////uid: = 1002, queston: 4, answer: 56fd5be1742b031bb8fd65a4effa57aba89e85116abe9b5a409a080771532f67

//    /*接口2*/
//    setUserSecurityAnswers(user2_list);
//    printf("-------------接口2测试完毕-------------\n");

////第三步，验证用户是否已成功绑定安全问题
//    /*接口3*/
//    ret = IsUserSecurityAnswers(1000);//无打印
//    if (ret == 1){
//        printf("user zky has security answer\n");//user zky has security answer
//    }

//    ret = IsUserSecurityAnswers(1001);
//    if (ret == 1){
//        printf("user usr1 has security answer\n");//user usr1 has security answer
//    }

//    ret = IsUserSecurityAnswers(1002);
//    if (ret == 1){
//        printf("user use2 has security answer\n");//user use2 has security answer
//    }
//    printf("-------------接口3测试完毕-------------\n");

////第四步，获取用户绑定好的安全问题列表。即这一步成功的前提是上面第二步绑定成功。
//    /*接口4*/
//    if (ret == 1){
//        zky_qst_list = getUserSecurityQuestions(1000, &count);//应该无打印
//        printQuestionList(zky_qst_list);//打印如下：
////questionId: 9, questionText: What is the name of your alma mater?
////questionId: 7, questionText: What was your childhood nickname?
////questionId: 6, questionText: Which team do you like the most?
//    }
//    printf("-------------接口4测试完毕-------------\n");

////第五步，校验用户的安全问题与对应答案是否匹配，如果匹配就返回一个token用来支持下一步的修改密码功能。即在第二步绑定成功的前提下，验证用户给的问题的密码是否正确。
//    /*接口5*/
//    if (ret == 1){
//        int is_ok = verifyUserSecurityAnswers(zky_ans_list, &token);//出参token是用来作为下一步修改密码的入参的，其中包含（is_ok==1即）修改密码需要的信息。打印出：
////questions: [679] [679]
////answer:    [12055daa14f4466cd5acbd6259dfc03853d90bd705fca6e3b550a26e97bc2a5b] [12055daa14f4466cd5acbd6259dfc03853d90bd705fca6e3b550a26e97bc2a5b]
//        printf("verifyUserSecurityAnswers: %s\n", is_ok == 1 ? "success" : "failed");//输出：verifyUserSecurityAnswers: success
//    }
//    printf("token: %s\n", token);//输出：token: 9d035cc2b8fcd59d9e0a5dc891f4d5bf9d025cc892567c3d47080522aa8a87d0d0c454e56cee548872c85eef6e465e8d4f03042ca222267878c45fe56f4ff78acf16c0e2c9af561e939a070dcc66bc17
//    printf("-------------接口5测试完毕-------------\n");

//    #if 0
//    /*测试加密函数*/
//    //TEST1();
//    //TEST2();
//    //TEST3();
//    //TEST4();
//    //TEST5();
//    //TEST6();

//    /*答案链表排序*/
//    SecurityAnswer answer[3];
//    answer[0].userId = 1000;
//    answer[1].userId = 1000;
//    answer[2].userId = 1000;
//    answer[0].questionId = 9;
//    memcpy(answer[0].encryptedAnswer, (unsigned char*)"0234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", 64);
//    answer[1].questionId = 5;
//    memcpy(answer[1].encryptedAnswer, (unsigned char*)"1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", 64);
//    answer[2].questionId = 0;
//    memcpy(answer[2].encryptedAnswer, (unsigned char*)"2234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", 64);
//    SecurityAnswerListNode *answer_list = NULL;
//    addAnswerNode(&answer_list, (void *)&answer[0], sizeof(answer[0]));
//    addAnswerNode(&answer_list, (void *)&answer[1], sizeof(answer[1]));
//    addAnswerNode(&answer_list, (void *)&answer[2], sizeof(answer[2]));

//    sortAnswerList(&answer_list);
//    printAnswerList(answer_list);
//    #endif

////第六步，修改此用户密码。需要使用第五步成功后返回的token作为实参。
//    /*接口6*/
//    sleep(20);
//    ret = changePassword("test", "Kylin@123", 30, token);//将用户名为test的密码修改为Kylin@123，30秒内没有修改就返回false
//    printf("changePassword: %s\n", ret == 0 ? "success" : "failed");//[ERROR] (null) is not exist.changePassword: failed
//    sleep(20);
//    ret = changePassword("test", "Kylin@123", 30, token);
//    printf("changePassword: %s\n", ret == 1 ? "success" : "failed");//[ERROR] (null) is not exist.changePassword: success
//    printf("-------------接口6测试完毕-------------\n");
//}

bool securityquestions::bindSecurityQuestionsAnswerV3(void)
{
    m_encryptedAnswerlist.clear();
    for (int n = 0; n < 3; n++) {
        AnswerInfo answerInfo;
        answerInfo.nQuestionId = m_uniqueNumbers[n];

//        QDBusMessage result = m_pServiceInterface->call("EncodeSecurityInfo",m_passwordList[n]);
//        if (result.type() == QDBusMessage::ErrorMessage) {
//            qDebug() << "EncodeSecurityInfo error:" << result.errorMessage();
////            return QString("");
//        }
//        QList<QVariant> varResult = result.arguments();
//        if (varResult.size() > 0) {
//            answerInfo.strAnswerText = varResult.takeFirst().toString();
//        }

        answerInfo.strAnswerText = m_uniauthService->EncodeSecurityInfo(m_passwordList[n]);
        m_encryptedAnswerlist.append(answerInfo);
    }

    for (auto ncryptedAnswer : m_encryptedAnswerlist) {
        qDebug() << ncryptedAnswer.nQuestionId <<"  "<< ncryptedAnswer.strAnswerText;
    }

//    m_encryptedAnswerlist = nullptr;
//    for (int i = 0; i < 3; i++){
//        const char * password = m_passwordList[i].toStdString().c_str();
//        char *hexBlk = NULL;
//        KPR_PKCS5_PBKDF2_HMAC2_v2((unsigned char*)password, strlen(password), &hexBlk);

//        SecurityAnswer answer;
//        memset(&answer, 0, sizeof(SecurityAnswer));
//        answer.userId = m_euid;
//        answer.questionId = m_uniqueNumbers[i];//
//        strcpy(answer.encryptedAnswer, hexBlk);

//        addAnswerNode(&m_encryptedAnswerlist, (void *)&answer, sizeof(SecurityAnswer));
//    }

    return true;
}

bool securityquestions::initSecurityQuestions(void){
//    bindtextdomain("kba", "/usr/share/locale");
//    textdomain("kba");
//    setlocale(LC_ALL, "zh_CN.UTF-8");
//    updateLanguage(m_strCurrentLang);

    qDebug() << "initSecurityQuestions";
    m_count = 0;
    m_preset_list = m_uniauthService->GetPresetSecurityQuestions(m_strCurrentLang);
    m_count = m_preset_list.count();

//    QDBusReply<QList<QuestionInfo>> reply = m_pServiceInterface->call("GetPresetSecurityQuestions",m_strCurrentLang);
//qDebug() << "--------------------securityquestions::initSecurityQuestions-2";
//    if (!reply.isValid()) {
//        qDebug() << "initSecurityQuestions error: " << reply.error().message();
//    }
//qDebug() << "--------------------securityquestions::initSecurityQuestions-3";
//    m_preset_list = reply.value();

//    qDebug() << "ecurityQuestions count:" << m_count;
//    for (auto questionInfo : m_preset_list) {
//        qDebug() << questionInfo.nQuestionId << questionInfo.strQuestionText;
//    }

    setQuestions();
    return true;
}

bool securityquestions::setQuestions(void){
    if(m_preset_list.count() == 0)
        return false;
//    SecurityQuestionListNode *current = m_preset_list;
    qDebug()<<"Security Questions count:"<<m_count;
    m_question1->blockSignals(true);
    m_question2->blockSignals(true);
    m_question3->blockSignals(true);
    m_question1->clear();
    m_question2->clear();
    m_question3->clear();

    for (auto questionInfo : m_preset_list) {
        QuestionInfo qsin = questionInfo;
        qDebug() << qsin.nQuestionId <<"  "<< qsin.strQuestionText;
        if(m_uniqueNumbers[0] == qsin.nQuestionId){
            m_question1->insertItem(0,qsin.strQuestionText,qsin.nQuestionId);
        }else if (m_uniqueNumbers[1] == qsin.nQuestionId) {
            m_question2->insertItem(0,qsin.strQuestionText,qsin.nQuestionId);
        }else if (m_uniqueNumbers[2] == qsin.nQuestionId) {
            m_question3->insertItem(0,qsin.strQuestionText,qsin.nQuestionId);
        }else{
            m_question1->addItem(qsin.strQuestionText, qsin.nQuestionId);//显示的是国际化语言
            m_question2->addItem(qsin.strQuestionText, qsin.nQuestionId);//显示的是国际化语言
            m_question3->addItem(qsin.strQuestionText, qsin.nQuestionId);//显示的是国际化语言
        }
    }

//    while (current != NULL) {
//        QString str = gettext(current->question.questionText);//翻译
//        qDebug()<<"Security Question Id:"<<current->question.questionId<<"  text:"<<str;
//        if(m_uniqueNumbers[0] == current->question.questionId){
//            m_question1->insertItem(0,str,current->question.questionId);
//        }else if (m_uniqueNumbers[1] == current->question.questionId) {
//            m_question2->insertItem(0,str,current->question.questionId);
//        }else if (m_uniqueNumbers[2] == current->question.questionId) {
//            m_question3->insertItem(0,str,current->question.questionId);
//        }else{
//            m_question1->addItem(str, current->question.questionId);//显示的是国际化语言
////            m_question1->setItemData(i, current->question.questionId);//附加信息
//            m_question2->addItem(str, current->question.questionId);//显示的是国际化语言
////            m_question2->setItemData(i, current->question.questionId);//附加信息
//            m_question3->addItem(str, current->question.questionId);//显示的是国际化语言
////            m_question3->setItemData(i, current->question.questionId);//附加信息
//        }
//        current = current->next;
//    }

    m_question1->setCurrentIndex(0);
    m_question2->setCurrentIndex(0);
    m_question3->setCurrentIndex(0);
    m_question1->blockSignals(false);
    m_question2->blockSignals(false);
    m_question3->blockSignals(false);
    return true;
}

//bool securityquestions::finishSecurityQuestionBind(void){
//    qDebug()<<"in finishSecurityQuestionBind";
//    bindSecurityQuestionsAnswerV3();
//    if(0 == setUserSecurityAnswers(m_encryptedAnswerlist)){//将信息录入相关后台
//        qDebug()<<"setUserSecurityAnswers success!";
////        return true;
//    } else{
//        qDebug()<<"setUserSecurityAnswers failed!";
////        return false;
//    }

//    if(1 == IsUserSecurityAnswers(m_euid)){//检查绑定是否成功
//        qDebug()<<"Userid "<<m_euid <<"Set security answers success!";
//        WriteSettingToIni("config","security","true");
//        return true;
//    } else{
//        qDebug()<<"Userid "<<m_euid <<"Set security answers failed!";
////        emit setSecurityAnswersErro();
//        return false;
//    }
//}

void securityquestions::updateLanguage(QString languageCode)
{
    qDebug()<<"Language for security question:"<<languageCode;
    if (languageCode.startsWith("zh_CN")) {
        setenv("LANGUAGE", "zh_CN", 1);
        setenv("LANG", "zh_CN", 1);
        setlocale(LC_ALL, "zh_CN.utf8");
        QLocale lang("zh_CN");
        /*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
        QLocale::setDefault(lang);
    } else {
        setenv("LANGUAGE", languageCode.toLatin1().data(), 1);
        setenv("LANG", languageCode.toLatin1().data(), 1);
        setlocale(LC_ALL, "");
        /*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
        QLocale::setDefault(languageCode);
    }

    bindtextdomain("kba", "/usr/share/locale");
    textdomain("kba");
}

void securityquestions::initSecurityDBusInterface()
{
//    m_pServiceInterface = new QDBusInterface(KPR_DBUS_SERVICE, \
//                                             KPR_DBUS_PATH, \
//                                             KPR_DBUS_INTERFACE, \
//                                             QDBusConnection::systemBus());

    m_uniauthService = new UniAuthService();
}

}
