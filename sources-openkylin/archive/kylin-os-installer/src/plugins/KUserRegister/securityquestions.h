#ifndef SECURITYQUESTIONS_H
#define SECURITYQUESTIONS_H

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
#include <QFormLayout>
#include <QComboBox>
#include <QDBusInterface>

#include <pwquality.h>
//#include "kysdk/applications/ksecuritylevelbar.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <libintl.h>
#include <locale.h>
#include <QDBusArgument>

//#include "/usr/include/kylin-passwdreset/kylin-passwdreset.h"
#include <kba/kba.h>
//#include <kba.h>
//#include "../lib/pbkdf2_hmac.h"

#include "../PluginService/dbus/uniauthservice.h"

//密保最大长度
#define   ANSWERMAXLENGTH    64

//#define KPR_DBUS_SERVICE "org.kylin.KprBackend"
//#define KPR_DBUS_PATH "/SecurityQuestionAnswer"
//#define KPR_DBUS_INTERFACE "org.kylin.KprBackend"

//struct QuestionInfo
//{
//    int nQuestionId;
//    QString strQuestionText;
//};

//struct AnswerInfo
//{
//    int nQuestionId;
//    QString strAnswerText;
//};

//Q_DECLARE_METATYPE(QuestionInfo)
//Q_DECLARE_METATYPE(QList<QuestionInfo>)
//Q_DECLARE_METATYPE(AnswerInfo)
//Q_DECLARE_METATYPE(QList<AnswerInfo>)

//QDBusArgument &operator<<(QDBusArgument &argument, const QuestionInfo &questionInfo);
//const QDBusArgument &operator>>(const QDBusArgument &argument, QuestionInfo &questionInfo);

//QDBusArgument &operator<<(QDBusArgument &argument, const AnswerInfo &answerInfo);
//const QDBusArgument &operator>>(const QDBusArgument &argument, AnswerInfo &answerInfo);

namespace KInstaller {

class securityquestions : public QWidget
{
    Q_OBJECT

public:
    explicit securityquestions(QWidget *parent = nullptr);
    ~securityquestions();
    void initUI();
    void initAllConnect();
    bool bindSecurityQuestionsAnswerV3(void);//将安全问题答案列表加密
    bool initSecurityQuestions(void);//初始化密保问题列表
    bool setQuestions(void);//显示密保问题
//    bool finishSecurityQuestionBind(void);
    void setBoardTabOrder();
    void setNextButton();
    void updateLanguage(QString languageCode);
    void initSecurityDBusInterface();

public:
    QVBoxLayout* m_totalLayout;
    QLabel * m_title;//密码安全问题
    QLabel * m_titleCommentary;//你可以在忘记密码时通过回答安全问题重置密码，请确保一定会记住答案。
//    QDBusInterface*             m_pServiceInterface = nullptr;
    UniAuthService *m_uniauthService = nullptr;

    QWidget *m_formsWidget;
    QFormLayout* m_forms;
    QLabel * m_questionTitle1;//问题1
    QComboBox *m_question1;
    QLabel * m_answerTitle1;//答案：
    QLineEdit *m_answer1;
    QLabel * m_questionTitle2;//问题2
    QComboBox *m_question2;
    QLabel * m_answerTitle2;//答案：
    QLineEdit *m_answer2;
    QLabel * m_questionTitle3;//问题3
    QComboBox *m_question3;
    QLabel * m_answerTitle3;//答案：
    QLineEdit *m_answer3;

    //稍后设置
    QCheckBox* m_Setuplater;

//    CLineEditLabel *m_passwdQuestion1 = nullptr;   //密码安全问题1输入框
//    int randnumber();//初始化m_uniqueNumbers列表，各元素值唯一，值范围[0,9]
//    SecurityAnswerListNode *bindSecurityQuestionsAnswer(SecurityQuestionListNode *head, int count, int uid, char * enc_answer);
//    void testQK();
    int m_count = 0;//系统预设的安全问题总数量
    QList<QuestionInfo> m_preset_list;//保存系统预设的安全问题列表。默认共有10个安全问题
    int m_uniqueNumbers[3] = {0,1,2};//当前显示的三个安全问题在列表中的下标。存放3个不同的值。值范围[0,m_count-1]
    QString m_passwordList[3];//安全问题的全部明文密码
    QList<AnswerInfo> m_encryptedAnswerlist ; //安全问题的全部密文密码，每个密文密码长度不超过64字节
    int m_euid = 1000;//当前用户的有效uid
    QString m_strCurrentLang="zh_CN.UTF-8";
    bool m_firstfocusget=false;//首次焦点获取标志(首次获取时隐藏边框)
signals:
    void enableNextButton(bool value);
    void lsSetsetUserQestA(bool);
    void setSecurityAnswersErro();
    void reSetFocusPolicySignal();
//signals:
//    void enableNextButton(bool value);
//    void setNextText();

public slots:
    void translateStr();
    void changeQuestion1(int index);
    void changeQuestion2(int index);
    void changeQuestion3(int index);
    void checkAnswer1(QString strAnswer);
    void checkAnswer2(QString strAnswer);
    void checkAnswer3(QString strAnswer);
    void setSetuplater(bool checked);
    void checkactive(QWidget *p);

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event);
};

//void finishSecurityQuestionBindG();
//void finishSecurityQuestionBindG(securityquestions* psecur);

}

#endif // SECURITYQUESTIONS_H
