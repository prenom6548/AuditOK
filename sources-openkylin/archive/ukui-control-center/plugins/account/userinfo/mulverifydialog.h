#ifndef MULVERIFYDIALOG_H
#define MULVERIFYDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPaintEvent>

#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

#define OTP_VERIFY_FACTORY 0
#define SECURITY_VERIFY_FACTORY 1

class MulVerifyDialog : public KDialog
{
    Q_OBJECT
public:
    explicit MulVerifyDialog(bool isPushBtnUI, QWidget *parent = nullptr);
    ~MulVerifyDialog();

    void refreshBindStatus(const QString &method, bool status);
private:
    void initUI();
    void initConnect();

private:
    QRadioButton *m_pSecurityBtn = nullptr;
    KLabel *m_pSecurityBindLabel = nullptr;
    QRadioButton *m_pOtpBtn = nullptr;
    KLabel *m_pOtpBindLabel = nullptr;
    QButtonGroup *m_pRadioBtnGroup = nullptr;

    KPushButton *m_pSecurityBindBtn = nullptr;
    KPushButton *m_pOtpBindBtn = nullptr;

    KPushButton *m_pConfirmBtn = nullptr;
    KPushButton *m_pCancelBtn = nullptr;

    bool m_isBoundOtp = false;
    bool m_isBoundUkey = false;

    bool m_isPushBtnUI = false;

Q_SIGNALS:
    void onSelectFactorySignal(int id);

public slots:
    void onConfirmBtnPressed();

protected:
    void keyPressEvent(QKeyEvent *);
};

#endif // MULVERIFYDIALOG_H
