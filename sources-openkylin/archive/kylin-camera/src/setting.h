#ifndef SETTING_H
#define SETTING_H


#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QHBoxLayout>
#include <QString>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QAction>
#include <QFileDialog>
#include <QSettings>
#include <QDialog>
#include <QComboBox>
#include <QString>
#include <QMap>
#include <QSettings>
#include <QMenu>
#include <QProcess>
#include <QFrame>
#include <QWidget>
#include <QWidgetAction>
#include "switchbutton.h"
#include "settingpage.h"
#include "currentdeviceinfo.h"
#include <kaboutdialog.h>

#include "util/daemonipcdbus.h"
class Setting: public QFrame
{
    Q_OBJECT
public:
    Setting(QWidget *main_widget, QWidget *parent = 0);
    ~Setting();
    //  ~Setting();
    QMenu *menu = nullptr;
    QWidget *main_widget = nullptr;
    QWidgetAction *delay_action = nullptr;
    QWidgetAction *mirror_action = nullptr;
    QAction *set = nullptr;
    QAction *theme = nullptr;
    QAction *help = nullptr;
    QAction *about = nullptr;
    QAction *quit = nullptr;
    QWidget *delay = nullptr;
    QLabel *mirrorlab = nullptr;
    QLabel *delaylab = nullptr;

    SwitchButton *delaybtn = nullptr;
    SwitchButton *mirrorbtn = nullptr;
    QMenu *themeMenu = nullptr; //设置主题菜单

    QHBoxLayout *delayLayout = nullptr;
    QHBoxLayout *mirrorLayout = nullptr;
    DaemonIpcDbus *m_pDaemonIpcDbus = nullptr;
    SettingPage *setPage = nullptr;

public Q_SLOTS:

private Q_SLOTS:
    void triggerMenu(QAction *action);
private:
    kdk::KAboutDialog *aboutWinWidget = nullptr;
    void initAboutText();
    void userManualTest(void);
Q_SIGNALS:
    void quitSig();
    void settingTestSig();
};

#endif // SETTING_H
