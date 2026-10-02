#ifndef OTPBINDDIALOG_H
#define OTPBINDDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

#include "qtdbus/pwdlessservice.h"

class OtpBindDialog :public KDialog
{
    Q_OBJECT
public:
    explicit OtpBindDialog(QString username, PwdlessService *service, QWidget *parent = nullptr);
    ~OtpBindDialog();

    void initUI();
    void initConnect();

    void setQrCodePixmap(QString code);
    QPushButton *GetConfirmBtn();
    QString GetToken();
    void setErrorMsg(QString string);

    void showFinishedPrompt(bool ret, QString errorMsg);

protected:
    void keyPressEvent(QKeyEvent *);
    QSize sizeHint() const;

private:
    PwdlessService *m_serviceInterface;
    QString m_username;

    KLabel *m_pTitleLabel = nullptr;
    KLabel *m_pQrcodePixmap = nullptr;
    QLineEdit *m_pOtpLineEdit = nullptr;
    KLabel *m_tipMsgLabel= nullptr;

    KPushButton *m_pConfirmBtn = nullptr;
    KPushButton *m_pCancelBtn = nullptr;

    QWidget *m_pFinishWidget = nullptr;
    QLabel *m_pFinishLabel = nullptr;
    KLabel *m_pFinishMsg = nullptr;
    QLabel *m_pFinishTip = nullptr;

    bool isFinished = false;
Q_SIGNALS:
    void bindOtpSignals(QString token);
public Q_SLOTS:
    void onBntnOkClicked();
};

#endif // OTPBINDDIALOG_H
