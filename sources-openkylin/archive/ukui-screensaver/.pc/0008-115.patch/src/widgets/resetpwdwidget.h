#ifndef RESETPWDWIDGET_H
#define RESETPWDWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include "securityquestionanswer.h"

#include "iconedit.h"

#include "passwdcheckutil.h"

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}

#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2

#endif

class TalksWidget;
class QPushButton;
class QLabel;
class KLabel;
class QLineEdit;
class IconButton;

extern float scale;
class ResetPwdWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResetPwdWidget(LockDialogModel *model, QString userName, QPixmap facePix, int uid, SecurityQuestionAnswer *securityQuestionAnswer,  QWidget *parent = nullptr);
    void updateResetPwdSize();
    void setFocusToEdit();
    void switchToConfidentialDig();
    void updateFontSize(double lfFontSize);

protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void initUI();
    void switchToResetPwdWdg();
    void verifyFailed();
    void refreshConfirmBtnStatus();
    void makeSurePwqualityEnabled();
    void checkPwdLegality();
    bool isContainLegitimacyChar(QString word);
    void clearLineEditText();

Q_SIGNALS:
    void backToAuthDialog(bool success);
    void confirmBtnclicked();
    void resetEditClicked();

private Q_SLOTS:
    void onConfirmBtnclicked();

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    QLabel              *m_pFaceLabel = nullptr;
    KLabel              *m_pNameLabel = nullptr;
    QList<TalksWidget*>  m_listTalkWidget;
    QWidget             *m_pQustionMainWidget = nullptr;
    TalksWidget         *m_pResetPwdWidget = nullptr;
    IconButton          *m_pConfirmBtn = nullptr;
    QPushButton         *m_pCancelBtn = nullptr;
    QString             m_nameText;
    QPixmap             m_facePixmap;
    int                 m_uid;
    KLabel              *m_pMessageLabel = nullptr;
    QWidget             *m_pSureWidget = nullptr;
    TalksWidget         *m_pNewPwdWidget = nullptr;
    KLabel              *m_pNewPwdTipLabel = nullptr;
    IconEdit          *m_pSureEdit = nullptr;
    KLabel              *m_pSureTipLabel = nullptr;
    SecurityQuestionAnswer      *m_pSecurityQuestionAnswer = nullptr;
    QList<QuestionInfo> m_listQuestionInfo;
    QString             m_newPwdTip;
    QString             m_surePwdTip;
    QString             m_verifyToken;
    bool                m_enablePwdQuality;
    QScrollArea         *m_pConftScrollArea;
    QScrollArea         *m_pResetPwdScrollArea;

#ifdef ENABLEPQ
    pwquality_settings_t *m_pSettings;
#endif
};

#endif // RESETPWDWIDGET_H
