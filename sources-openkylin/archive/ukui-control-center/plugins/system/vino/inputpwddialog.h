#ifndef INPUTPWDDIALOG_H
#define INPUTPWDDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QGSettings>
#include <QLabel>
#include <QLineEdit>
#include <kpasswordedit.h>
using namespace kdk;

#include "krd.h"

namespace Ui {
class InputPwdDialog;
}
class InputPwdDialog : public QDialog
{
    Q_OBJECT

public:
    InputPwdDialog(QGSettings *Keygsettiings,QWidget *parent = nullptr);
    InputPwdDialog(ComKylinRemoteDesktopInterface *waylandDbus, QWidget *parent = nullptr);
    ~InputPwdDialog();

private:
    QGSettings *m_pGsettings;
    ComKylinRemoteDesktopInterface *m_pWaylandDbus;

    QPushButton *m_pCancelBtn;
    QPushButton *m_pConfirmBtn;

    KPasswordEdit *m_pPasswordEdit;
    QLabel *m_pHintLabel;

    QByteArray m_password;
    bool m_isEdited;

private:
    void setupInit();

    void initConnect();

protected:
    void keyPressEvent(QKeyEvent * event);

private slots:
    void mpwdInputSlot(const QString &pwd);
    void setPasswordSlot();
};


#endif // INPUTPWDDIALOG_H
