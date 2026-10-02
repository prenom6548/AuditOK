#ifndef CHANGEUSERPWD_H
#define CHANGEUSERPWD_H

#include <QTimer>

#include "lineeditwidget.h"

#include <kpasswordedit.h>
#include <ksecurityquestiondialog.h>
#include <kborderlessbutton.h>
#include <kdialog.h>
#include <kysdk/applications/klabel.h>
using namespace kdk;

#include "pwdcheckthread.h"
#include "passwdcheckutil.h"

#include "qtdbus/uniauthservice.h"
#include "messagebar.h"

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}

#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2

#endif

class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QMouseEvent;
class QKeyEvent;

class ChangeUserPwd : public KDialog
{
    Q_OBJECT

public:
    explicit ChangeUserPwd(QString n, QWidget *parent = nullptr);
    ~ChangeUserPwd();

public:
    void initUI();
    void setupConnect();
    void setupStatus(QString n);

    void makeSurePwqualityEnabled();

    void refreshConfirmBtnStatus();
    void refreshCloseBtnStatus();
    bool changePassword(const QString &programPath, const QString &oldPassword, const QString &newPassword, int *resultCode, QString *resultMsg);


public:
    QVBoxLayout * mainVerLayout;
    QVBoxLayout * contentVerLayout;
    QHBoxLayout * currentPwdHorLayout;
    QHBoxLayout * newPwdHorLayout;
    QHBoxLayout * tipHorLayout;
    QHBoxLayout * surePwdHorLayout;
    QHBoxLayout * bottomBtnsHorLayout;
    QHBoxLayout *curTipHorLayout;
    QHBoxLayout *newTipHorLayout;
    QHBoxLayout *sureTipHorLayout;

    KPushButton * cancelBtn;
    KPushButton * confirmBtn;

    KLabel *currentPwdLabel;
    KLabel *curTipLabel;
    KLabel *newPwdLabel;
    KLabel *newTipLabel;
    KLabel *surePwdLabel;
    KLabel *tipLabel;

    KPasswordEdit *currentPwdLineEdit;
    KPasswordEdit * newPwdLineEdit;
    KPasswordEdit * surePwdLineEdit;

    KLabel *m_pPasswordPromptLabel = nullptr;
    QLineEdit *m_pPasswordPromptLineedit = nullptr;
    KLabel *m_pPasswordPromptTips = nullptr;

    UniAuthService *m_pUniauthBackendDbus = nullptr;
    MessageBar *m_pTipMessage = nullptr;
    QDBusInterface *userSettingDbus = nullptr;
    bool m_isPasswordMinAgeReached = true;

protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *target, QEvent *event);
private:
    QString name;
    QString curPwdTip;
    QString newPwdTip;
    QString surePwdTip;
    QPixmap m_waitingPixmap;
    QTimer *m_timer = nullptr;

    bool isCurrentUser;
    bool isChecking = false;
    bool enablePwdQuality;
    bool m_EnablePwdHistory = false;

    PwdCheckThread * thread1ForCheckPwd;

#ifdef ENABLEPQ
    pwquality_settings_t *settings;
#endif

private:
    void checkPwdLegality();
    bool isContainLegitimacyChar(QString word);

private slots:
    void updatePixmap();
    void checkPwdFinishedSlot(QString re);
    void passwordPromptChangedSlot();

signals:
    void changeOtherUserPwd(QString pwd);
    void changeOtherUserPrompt(const QString &prompt);
};

#endif // CHANGEUSERPWD_H
