#ifndef ONCLOSEDIALOG_H
#define ONCLOSEDIALOG_H

#include <QObject>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

class OnCloseDialog : public KDialog
{
    Q_OBJECT
public:
    OnCloseDialog(QWidget *parent = nullptr);
    ~OnCloseDialog();

private:
    void initUI();
    void initStatus();
    void setConnect();

private:
    QVBoxLayout *m_pMainVerLayout = nullptr;

    KPushButton *m_pCancelBtn = nullptr;
    KPushButton *m_pConfirmBtn = nullptr;
    QRadioButton *m_pExitBtn = nullptr;
    QRadioButton *m_pMinimizeBtn = nullptr;
    QButtonGroup *m_pRadioBtnGroup = nullptr;
    QCheckBox *m_pRememberCheckBox = nullptr;

    QGSettings *m_pSystemMonitorGSetting = nullptr;

protected:
    void keyPressEvent(QKeyEvent *);

signals:
    void closeMainWindow();
    void needSaveSetting();
};

#endif // ONCLOSEDIALOG_H
