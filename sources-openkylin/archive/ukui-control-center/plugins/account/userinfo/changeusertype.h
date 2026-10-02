#ifndef CHANGEUSERTYPE_H
#define CHANGEUSERTYPE_H

#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
#include <kysdk/applications/kbackgroundgroup.h>
#include <kysdk/applications/kdialog.h>
using namespace kdk;

class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class QFrame;
class SettingFrame;

class ChangeUserType : public KDialog
{
    Q_OBJECT

public:
    explicit ChangeUserType(QWidget *parent = nullptr);
    ~ChangeUserType();

public:
    void requireUserInfo(QString logo, QString nname, int id, QString utype);
    void initUI();
    void setConnect();

public:
    QRadioButton *m_pAdminRadioBtn = nullptr;
    QRadioButton *m_pStandardRadioBtn = nullptr;
    QButtonGroup *m_pTypesBtnGroup = nullptr;

    KLabel *m_pNickNameLabel = nullptr;
    KLabel *m_pUserTypeLabel = nullptr;
    KLabel *m_pUserLogoBtn = nullptr;

    KPushButton *m_pConfirmBtn = nullptr;
    KPushButton *m_pCancelBtn = nullptr;

    SettingFrame *m_pAdminFrame = nullptr;
    SettingFrame *m_pStandardFrame = nullptr;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *);
    QSize sizeHint() const;

private:
    int oldid;

signals:
    void confirmChanged(int id);
};

#endif // CHANGEUSERTYPE_H
