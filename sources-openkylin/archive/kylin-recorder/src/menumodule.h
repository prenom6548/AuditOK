#ifndef MENUMODULE_H
#define MENUMODULE_H
#define DEBUG_MENUMODULE 0 // DEBUG模式开关，代码上线之前务必关掉

#define UKUI_THEME_GSETTING_PATH "org.ukui.style"

#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRect>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>
#include <QToolButton>
#include <QFont>
#include <QTextBrowser>
#include "daemondbus.h"
#include "kaboutdialog.h"

class menuModule : public QWidget
{
    Q_OBJECT

    friend class TestMainWindow;
public:
    explicit menuModule(QWidget *);
    void themeUpdate();

signals:
    void setSignal();
    void menuModuleClose();
    void menuModuleSetThemeStyle(QString);

public:
    QToolButton *m_menuButton = nullptr;
    QToolButton *m_titleButtonClose = nullptr;

public:
    //    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString m_appName = "kylin-recorder";      //格式kylin-recorder
    QString m_appShowingName = tr("Recorder"); //格式kylin-recorder ,用来在前端展示
    QString m_appDescription = "kylin recorder";
    QString m_iconPath = ":/svg/svg/recording_128.svg";
    QString m_configPath = "org.kylin-recorder-data.settings";
    QFont m_font;
    QGSettings *m_pGsettingThemeStatus = nullptr;
    kdk::KAboutDialog *m_aboutWindow = nullptr;

private:
    QPalette m_pal;
    QMenu *m_menu = nullptr;
    QSize m_iconSize;
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QVBoxLayout *m_bodyLayout = nullptr;



    QGSettings *m_pGsettingThemeData = nullptr;

    enum typeThemeStatus { themeAuto = 0, themeBlackOnly = 1, themeLightOnly = 2 } themeStatus;
public slots:
    void dealSystemGsettingChange(const QString);

private:
    void initGsetting();
    void initAction();
    void triggerMenu(QAction *act);      //主菜单动作4
    void triggerThemeMenu(QAction *act); //主题动作
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    void setThemeFromLocalThemeSetting(QList<QAction *>); //获取本地主题配置
    void setStyleByThemeGsetting();                       //通过外部主题配置设置主题
    void setThemeStyle();
    void setThemeLight();
    void setThemeDark();
    DaemonDbus *m_ipcDbus = nullptr;
    //    void updateTheme(); //点击菜单中的主题设置后更新一次主题

    void refreshThemeBySystemConf(); //通过系统配置更改主题
};

#endif // MENUMODULE_H
