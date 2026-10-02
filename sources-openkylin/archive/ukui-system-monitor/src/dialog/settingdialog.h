#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QObject>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <kysdk/applications/kpushbutton.h>
#include <kdialog.h>
using namespace kdk;

class SettingDialog : public KDialog
{
    Q_OBJECT
public:
    SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();

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

    QGSettings *m_pSystemMonitorGSetting = nullptr;

protected:
    void keyPressEvent(QKeyEvent *);
};

#endif // SETTINGDIALOG_H
