#include "menumodule.h"
#include "mainwindow.h"
// #include "xatom-helper.h"
#include <gsettingmonitor.h>
#include "kmenubutton.h"

menuModule::menuModule(QWidget *parent = nullptr) : QWidget(parent)
{
    initAction();
}

void menuModule::initAction()
{
    m_iconSize = QSize(30, 30);
    m_menuButton = new QToolButton(MainWindow::mutual->m_titleRightWidget);
    m_menuButton->setAccessibleName("kylin-recorder_menumodule_QToolButton_m_menuButton");
    m_font = QFont();
    m_font.setPixelSize(14);
    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-recorder_QMenu_m_menu");
    QList<QAction *> actions;
    QAction *actionSetting = new QAction(m_menu);
    actionSetting->setText(tr("Settings"));
    //    actionSetting->setFont(ft);
    QAction *actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));
    actionTheme->setFont(m_font);
    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    //    actionHelp->setFont(ft);
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    //    actionAbout->setFont(ft);
    kdk::KMenuButton* act_advice_feedback = new kdk::KMenuButton();
    act_advice_feedback->setAccessibleName("kylin-recorder_menumodule_KMenuButton_act_advice_feedback");
    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));
    //    actionQuit->setFont(ft);
    actions << actionSetting /*<<actionTheme*/ << actionHelp << actionAbout << act_advice_feedback->feedbackAction() << actionQuit; //暂时禁掉主题切换按钮
    m_menu->addActions(actions);
    //    互斥按钮组
    QMenu *themeMenu = new QMenu;
    themeMenu->setAccessibleName("kylin-recorder_QMenu_themeMenu");
    QActionGroup *themeMenuGroup = new QActionGroup(this);
    QAction *autoTheme = new QAction("Auto", this);
    themeMenuGroup->addAction(autoTheme);
    themeMenu->addAction(autoTheme);
    autoTheme->setCheckable(true);
    QAction *lightTheme = new QAction("Light", this);
    themeMenuGroup->addAction(lightTheme);
    themeMenu->addAction(lightTheme);
    lightTheme->setCheckable(true);
    QAction *darkTheme = new QAction("Dark", this);
    themeMenuGroup->addAction(darkTheme);
    themeMenu->addAction(darkTheme);
    darkTheme->setCheckable(true);
    QList<QAction *> themeActions;
    themeActions << autoTheme << lightTheme << darkTheme;
    //    autoTheme->setChecked(true);
    actionTheme->setMenu(themeMenu);
    m_menuButton->setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &menuModule::triggerMenu);
    initGsetting();
    setThemeFromLocalThemeSetting(themeActions);
    themeUpdate();
    connect(themeMenu, &QMenu::triggered, this, &menuModule::triggerThemeMenu);
    connect(this, &menuModule::setSignal, MainWindow::mutual, &MainWindow::goset);
    connect(this, &menuModule::menuModuleClose, MainWindow::mutual, &MainWindow::close);
    connect(this, &menuModule::menuModuleSetThemeStyle, MainWindow::mutual, [=](QString str) {
        MainWindow::mutual->m_limitThemeColor = str;
        qDebug() << "限制主题颜色为" << MainWindow::mutual->m_limitThemeColor << str;
    });
}

void menuModule::setThemeFromLocalThemeSetting(QList<QAction *> themeActions)
{
#if DEBUG_MENUMODULE
    confPath = "org.kylin-recorder-data.settings";
#endif
    m_pGsettingThemeStatus = new QGSettings(m_configPath.toLocal8Bit());
    QString appConf = m_pGsettingThemeStatus->get("thememode").toString();
    if ("lightonly" == appConf) {
        themeStatus = themeLightOnly;
        themeActions[1]->setChecked(true); //程序gsetting中为浅色only的时候就给浅色按钮设置checked
    } else if ("darkonly" == appConf) {
        themeStatus = themeBlackOnly;
        themeActions[2]->setChecked(true);
    } else {
        themeStatus = themeAuto;
        themeActions[0]->setChecked(true);
    }
}

void menuModule::themeUpdate()
{
    if (themeStatus == themeLightOnly) {
        setThemeLight();
    } else if (themeStatus == themeBlackOnly) {
        setThemeDark();
    } else {
        setStyleByThemeGsetting();
    }
}

void menuModule::setStyleByThemeGsetting()
{
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    if ("ukui-dark" == nowThemeStyle || "ukui-black" == nowThemeStyle) {
        setThemeDark();
    } else {
        setThemeLight();
    }
}

void menuModule::triggerMenu(QAction *act)
{


    QString str = act->text();
    if (tr("Quit") == str) {
        emit menuModuleClose(); //退出
    } else if (tr("About") == str) {
        aboutAction(); //关于
    } else if (tr("Help") == str) {
        helpAction(); //帮助
    } else if (tr("Settings") == str) {
        emit setSignal(); //设置
    }
}

void menuModule::triggerThemeMenu(QAction *act)
{
    if (!m_pGsettingThemeStatus) {
        m_pGsettingThemeStatus =
            new QGSettings(m_configPath.toLocal8Bit()); // m_pGsettingThemeStatus指针重复使用避免占用栈空间
    }
    QString str = act->text();
    if ("Light" == str) {
        themeStatus = themeLightOnly;
        disconnect(m_pGsettingThemeData, &QGSettings::changed, this, &menuModule::dealSystemGsettingChange);
        m_pGsettingThemeStatus->set("thememode", "lightonly");
        qDebug() << m_pGsettingThemeStatus->get("thememode").toString();
        setThemeLight();
    } else if ("Dark" == str) {
        themeStatus = themeBlackOnly;
        disconnect(m_pGsettingThemeData, &QGSettings::changed, this, &menuModule::dealSystemGsettingChange);
        m_pGsettingThemeStatus->set("thememode", "darkonly");
        qDebug() << m_pGsettingThemeStatus->get("thememode").toString();
        setThemeDark();
    } else {
        themeStatus = themeAuto;
        m_pGsettingThemeStatus->set("thememode", "auto");
        initGsetting();
        //        updateTheme();
        themeUpdate();
    }
}

void menuModule::aboutAction()
{
    //    关于点击事件处理
    initAbout();
}

void menuModule::helpAction()
{
//    帮助点击事件处理
#if DEBUG_MENUMODULE
    appName = "kylin-recorder";
#endif
    if (!m_ipcDbus) {
        m_ipcDbus = new DaemonDbus();
    }

    if (!m_ipcDbus->daemonIsNotRunning()) {
        m_ipcDbus->showGuide(m_appName);
    }
}

void menuModule::initAbout()
{
#ifdef NOTKAIMING
    QString m_appVersion = qApp->applicationVersion();
#else
    QString m_appVersion = QString(qgetenv("APP_VERSION"));
#endif
    m_aboutWindow = new kdk::KAboutDialog(this,QIcon::fromTheme("kylin-recorder"),tr(m_appShowingName.toLocal8Bit()),tr("Version:") + m_appVersion);
    m_aboutWindow->setAccessibleName("kylin-recorder_menumodule_KAboutDialog_m_aboutWindow");
    m_aboutWindow->setBodyText(tr("The UI is friendly and easy to operate."
                                  " It supports by microphone"
                                  " ,playing and deleting in file list,"
                                  " and switching between Mini mode and Theme mode"));

    m_aboutWindow->setBodyTextVisiable(false);
    m_aboutWindow->setWindowModality(Qt::WindowModal);
    m_aboutWindow->setWindowModality(Qt::ApplicationModal);

    m_aboutWindow->show();
    m_aboutWindow->move(MainWindow::mutual->geometry().center() - m_aboutWindow->rect().center());
}

void menuModule::initGsetting()
{
    if (QGSettings::isSchemaInstalled(UKUI_THEME_GSETTING_PATH)) {
        m_pGsettingThemeData = new QGSettings(UKUI_THEME_GSETTING_PATH);
        connect(m_pGsettingThemeData, &QGSettings::changed, this, &menuModule::dealSystemGsettingChange);
    }
}

void menuModule::dealSystemGsettingChange(const QString key)
{
    if (key == "styleName") {
        refreshThemeBySystemConf();
    }
}

void menuModule::refreshThemeBySystemConf()
{
    QString themeNow = m_pGsettingThemeData->get("styleName").toString();
    if ("ukui-dark" == themeNow || "ukui-black" == themeNow) {
        setThemeDark();
    } else {
        setThemeLight();
    }
}

void menuModule::setThemeDark()
{
    qDebug() << "set theme dark";
    emit menuModuleSetThemeStyle("dark-theme");
}

void menuModule::setThemeLight()
{
    qDebug() << "set theme light";
    emit menuModuleSetThemeStyle("light-theme");
}
