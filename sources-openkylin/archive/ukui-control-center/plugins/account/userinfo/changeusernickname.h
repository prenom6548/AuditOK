#ifndef CHANGEUSERNICKNAME_H
#define CHANGEUSERNICKNAME_H

#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class QMouseEvent;
class QKeyEvent;

class QDBusInterface;

class ChangeUserNickname : public KDialog
{
    Q_OBJECT

public:
    explicit ChangeUserNickname(QString nn, QStringList ns, QString op, QWidget *parent = nullptr);
    ~ChangeUserNickname();

public:
    void initUI();
    void setConnect();

public:
    KPushButton *m_pConfirmBtn = nullptr;
    KPushButton *m_pCancelBtn = nullptr;
    KLabel *m_pUserNameLabel = nullptr;
    KLabel *m_pNickNameLabel = nullptr;
    KLabel *m_pTipLabel = nullptr;
    QLineEdit *m_pNickNameLineEdit = nullptr;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *);

private:
    void nameLegalityCheck(QString nickname);
    void refreshConfirmBtnStatus();

private:
    QString realname;
    QStringList namesIsExists;
    QString tipString;
    QString oldNickName;

    QDBusInterface *m_pAccountServiceIface = nullptr;
};

#endif // CHANGEUSERNICKNAME_H
