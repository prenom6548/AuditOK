#ifndef DELETEUSEREXISTS_H
#define DELETEUSEREXISTS_H

#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

class QHBoxLayout;
class QVBoxLayout;
class QFrame;
class QRadioButton;
class QButtonGroup;

class DeleteUserExists : public KDialog
{
    Q_OBJECT

public:
    explicit DeleteUserExists(QString name, QString nick, QWidget *parent = nullptr);
    ~DeleteUserExists();
public:
    void initUI();
    void setConnect();

public:
    QFrame *m_pRemoveButKeepFilesFrame = nullptr;
    QFrame *m_pRemoveWholeFrame = nullptr;
    QButtonGroup *m_pRemoveBtnGroup = nullptr;
    QRadioButton *m_pRemoveButKeepFilesRadioBtn = nullptr;
    QRadioButton *m_pRemoveWholeRadioBtn = nullptr;

    KPushButton *m_pCancelBtn = nullptr;
    KPushButton *m_pConfirmBtn = nullptr;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent * event);
    QSize sizeHint() const;

private:
    QString _name;
    QString _nick;

signals:
    void confirmChanged(bool removeFiles);
};

#endif // DELETEUSEREXISTS_H
