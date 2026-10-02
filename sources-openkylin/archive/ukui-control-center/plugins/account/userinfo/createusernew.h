#ifndef CREATEUSERNEW_H
#define CREATEUSERNEW_H

#include <QCheckBox>
#include <QComboBox>
#include <QScrollArea>

#include <QToolButton>
#include <kbubblewidget.h>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

#include "qtdbus/uniauthservice.h"
#include "passwdcheckutil.h"

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}
#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2
#endif

class QVBoxLayout;
class QHBoxLayout;
class QRadioButton;
class QFrame;
class QLineEdit;
class QButtonGroup;

#define LABELWIDTH 92
#define SECURITY_QUESTION_COUNT 3

class CreateUserNew : public KDialog
{
    Q_OBJECT

public:
    explicit CreateUserNew(QStringList allUsers, QWidget *parent = nullptr);
    ~CreateUserNew();

public:
    void initUI();
    void setConnect();
    void makeSurePwdNeedCheck();
    void nameLegalityCheck(QString username);
    void nameLegalityCheck2(QString nickname);
    void pwdLegalityCheck(QString pwd);
    void initQustionCombox(const QStringList &questionlist);

private:
    QVBoxLayout * m_pScrollVLayout = nullptr;

    QComboBox *m_pUserTypeComboBox = nullptr;
    QPushButton *m_pUserTypeDetailBtn = nullptr;
    KBubbleWidget *m_pBubbleTips = nullptr;
    KLabel *m_pUsernameTipLabel = nullptr;
    KLabel *m_pNicknameTipLabel = nullptr;
    KLabel *m_pHostnameTipLabel = nullptr;
    KLabel *m_pNewpwdTipLabel = nullptr;
    KLabel *m_pSurepwdTipLabel = nullptr;

    QLineEdit *m_pUsernameLineEdit = nullptr;
    QLineEdit *m_pNicknameLineEdit = nullptr;
    QLineEdit *m_pHostnameLineEdit = nullptr;
    QLineEdit *m_pNewPwdLineEdit = nullptr;
    QLineEdit *m_pSurePwdLineEdit = nullptr;

    KPushButton *m_pConfirmBtn = nullptr;
    KPushButton *m_pCancelBtn = nullptr;

    QCheckBox *m_pSecurityQusetionChecbox = nullptr;
    KLabel *m_pSecurityQusetionTitleLabel = nullptr;
    QVBoxLayout *m_pSecurityQusetionVLayout = nullptr;
    QWidget *m_pSecurityQuestionWidget = nullptr;

    QList<QComboBox *> m_questionComboxList;
    QList<QLineEdit *> m_answerLineeditList;

    QStringList m_questionlist;
    QScrollArea *m_pScrollArea = nullptr;
    QWidget *m_pScrollWidget = nullptr;

    QLineEdit *m_pPasswordPromptLineedit = nullptr;
    KLabel *m_pPasswordPromptTips = nullptr;

    QString userNameTip;
    QString nickNameTip;
    QString newPwdTip;
    QString surePwdTip;
    QString oldName;
    QString oldNickName;

    QStringList _allNames;

    bool enablePwdQuality;
    bool hasNickName = false;

#ifdef ENABLEPQ
    pwquality_settings_t *settings;
#endif

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *);

private:
    bool nameTraverse(QString username);
    bool isHomeUserExists(QString username);
    bool isGroupNameEixsts(QString username);
    bool checkCharLegitimacy(QString password);
    void addSecurityQuestionItem(const int count, bool mutex);
    void setSecurityQuestionVisiable(bool visible);

signals:
    void confirmCreated(QString name, QString fullname, int accounttype, QString pwd);
    void bindSecurityQuestion(QString name, QList<AnswerInfo> answerInfo);
    void bindPasswordTips(const QString &passwordTips);

public Q_SLOTS:
    void setQustionCombox();
    void refreshConfirmBtnStatus();
    void passwordPromptChangedSlot();
};

#endif // CREATEUSERNEW_H
