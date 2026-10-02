#ifndef INSTALLINGOEMCONFIGFRAME_H
#define INSTALLINGOEMCONFIGFRAME_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include "../PluginService/backendThread/oeminstallerprocess.h"
#include "../plugins/KUserRegister/userframe.h"

namespace KInstaller {

class InstallingOEMConfigFrame : public QWidget
{
    Q_OBJECT
public:
    explicit InstallingOEMConfigFrame(UserFrame* pUF = nullptr, QWidget *parent = nullptr);
    ~InstallingOEMConfigFrame();
    void initUI();
    void translateStr();
    void addStyleSheet();
    void showOEMErrorFrame();
private:
    void changeEvent(QEvent *event);
signals:
    void signalExecError(QString str);   //配置失败
    void setSecurityAnswers();           //开始设置密保
public slots:
    void finishLoadDisk();
//    void setsetUserQestAnsw(bool lsSet);
    void sendUserSecurityAnswersErro();
    void clickRestartBtn();
public:
    QThread* m_backOEMThread;            //oem线程
    OemInstallerProcess* m_oemProcess;   //oem代码段
    QGridLayout* layout;
    QLabel *label, *label1, *m_loadpic;
    int var;
    QTimer *timer;
    bool m_setUserQestAnsw = true;       //默认需要设置密保
    UserFrame *m_UFplugin = nullptr;
    bool m_lsQADone = false;
};

}
#endif // INSTALLINGOEMCONFIGFRAME_H
