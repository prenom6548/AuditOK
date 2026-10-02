#include "setting.h"
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>
#include <QErrorMessage>
#include <QApplication>
#include <QGraphicsEffect>
#include "unistd.h"
#include "customstyle.h"
#include "util/xatom-helper.h"
#include <kylin_system/window_management.hpp>
#include <kylin_system/user_manual.hpp>
#include "kmenubutton.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include "windowmanager/windowmanager.h"
#include "ukuiwindowhelper.h"
#include "globalinstance.h"

#define MAINWINDOWTITLEBARHEIGHT 42
/***********#11358 【cpm】【KelvinV-W5821-LV--比亚迪精密】使用相机拍照片后，
 * *******发现系统将照片（jpg格式）自动归类到视频文件夹分类，图片文件夹找不到照片。******/
//#define SETTINGPAGEWIDTH 550
//#define SETTINGPAGEHEIGHT 400
/***********#11358 【cpm】【KelvinV-W5821-LV--比亚迪精密】使用相机拍照片后，
 * *******发现系统将照片（jpg格式）自动归类到视频文件夹分类，图片文件夹找不到照片。******/
#define SETTINGPAGEWIDTH 550
#define SETTINGPAGEHEIGHT 350
#define SETTINGPAGEWIDTHMAX 640
#define SETTINGPAGEHEIGHTMAX 400
#define ABOUTWINDOWWIDETH  420                 //窗口宽度
#define ABOUTWINDOWHEIGHT  316                 //窗口高度
#define ABOUTTITLEHEIGHT 42                    //标题栏高度

Setting::Setting(QWidget *main_widget, QWidget *parent)
  : QFrame(parent)
{
    //菜单及菜单项的设置
    this->main_widget = main_widget;
    menu = new QMenu;
    menu->setAccessibleName("kylin-camera_QMenu_menu");
    set = new QAction(tr("Set"),menu);
    theme = new QAction(tr("Theme"),menu);
    help = new QAction(tr("Help"),menu);
    about = new QAction(tr("About"),menu);
    quit = new QAction(tr("Quit"),menu);
    menu->setCursor(Qt::PointingHandCursor);

    kdk::KMenuButton* act_advice_feedback = new kdk::KMenuButton();
    act_advice_feedback->setAccessibleName("kylin-camera_setting_KMenuButton_act_advice_feedback");

    menu->addAction(set);
    menu->addAction(help);
    menu->addAction(about);
    menu->addAction(act_advice_feedback->feedbackAction());
    menu->addAction(quit);
    //“设置”菜单项对应的页面

/*****************暂时未启用的功能start***********************/
    delay_action = new QWidgetAction(this);
    delaylab = new QLabel;
    delaylab->setAccessibleName("kylin-camera_QLabel_delaylab");
    delay = new QWidget;
    delay->setAccessibleName("kylin-camera_setting_QWidget_delay");
    delaylab->setText(tr("Delayed"));
    delaybtn = new SwitchButton(this);
    delaybtn->setAccessibleName("kylin-camera_setting_SwitchButton_delaybtn");
    themeMenu = new QMenu;
    themeMenu->setAccessibleName("kylin-camera_QMenu_themeMenu");
    delayLayout = new QHBoxLayout;
    delay->setFixedHeight(30);
    //设置按钮布局
    delayLayout->addSpacing(12);
    delayLayout->addWidget(delaylab);
    delayLayout->addStretch(5);
    delayLayout->addWidget(delaybtn);
    delayLayout->addSpacing(10);
    delayLayout->setMargin(0);
    delay->setLayout(delayLayout);
    delay_action->setDefaultWidget(delay);

    QActionGroup *themeMenuGroup = new QActionGroup(this);
    QAction *autoTheme = new QAction(tr("Auto"),this);
    themeMenuGroup->addAction(autoTheme);
    themeMenu->addAction(autoTheme);
    autoTheme->setCheckable(true);
    QAction *lightTheme = new QAction(tr("Light"),this);
    themeMenuGroup->addAction(lightTheme);
    themeMenu->addAction(lightTheme);
    lightTheme->setCheckable(true);
    QAction *darkTheme = new QAction(tr("Dark"),this);
    themeMenuGroup->addAction(darkTheme);
    themeMenu->addAction(darkTheme);
    darkTheme->setCheckable(true);
    QList<QAction* > themeActions;
    themeActions<<autoTheme<<lightTheme<<darkTheme;
/*****************暂时未启用的功能end***********************/
    kdk::kabase::WindowManagement::setWindowMotifHint(this->winId());

    connect(menu,&QMenu::triggered,this,&Setting::triggerMenu);
    this->setForegroundRole(QPalette::Highlight);
    this->setAutoFillBackground(true);
    this->setStyleSheet("QWidget#widgetMain{border: 1px solid #FF00FF; border-radius: 5px;}");
    QGraphicsEffect *effect = nullptr;
    this->setFrameStyle(QFrame::Panel);
    this->setGraphicsEffect(effect);
}
Setting::~Setting(){
    delete menu;
    delete setPage;
}
void Setting::userManualTest(void)
{
    kdk::kabase::UserManualManagement userManualTest;
    if (!userManualTest.callUserManual(kdk::kabase::AppName::KylinCamera)) {
        qCritical() << "user manual call fail!";
    }

    return;
}

static QString getAppVersion()
{
    QProcess process;
    QStringList list;
    QString str("dpkg -l | grep "+QString("kylin-camera") +" | awk '{print $3}'");
    list<<"-c"<<str;
    process.start("/bin/bash",list);
    process.waitForFinished();
    QString result;
    QString string;
    string=process.readAllStandardOutput().data();
    if(string =="" || string.isNull())
    {
        qDebug() << "cannot use dpkg get version!";
        result += APP_VERSION;
    }
    else
    {
        result += string;
        result.remove("\n"); //去掉 \n 字符
    }
    return result;

}

//点击menu按钮 下拉菜单
void Setting::triggerMenu(QAction *action){
    Q_EMIT settingTestSig();
    QString str = action->text();
     if(str == tr("Help")){
         userManualTest();
     }
    if(str == tr("About")){
        QString aboutTitleName = tr("Camera");
#ifdef NOTKAIMING
        QString appVersion = getAppVersion();
#else
        QString appVersion = QString(qgetenv("APP_VERSION"));
#endif
        if (aboutWinWidget == nullptr)
        {
            if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
                aboutWinWidget = new kdk::KAboutDialog(main_widget, QIcon::fromTheme("kylin-camera"),tr(aboutTitleName.toLocal8Bit()),tr("Version: ") + appVersion);
                aboutWinWidget->setAccessibleName("kylin-camera_setting_KAboutDialog_aboutWinWidget");
            }else{
                aboutWinWidget = new kdk::KAboutDialog(this->main_widget, QIcon::fromTheme("kylin-camera"),tr(aboutTitleName.toLocal8Bit()),tr("Version: ") + appVersion);
                aboutWinWidget->setAccessibleName("kylin-camera_setting_KAboutDialog_aboutWinWidget");
            }
            aboutWinWidget->setBodyText(tr("Camera is a very strong camera software. "
                                           "It has the characteristics of easy to use. "
                                           "You can realize the function of taking a photo recording and displaying the album"));
        }
        aboutWinWidget->setBodyTextVisiable(false);
        aboutWinWidget->setWindowModality(Qt::WindowModal);
        aboutWinWidget->setWindowModality(Qt::ApplicationModal);
        kdk::kabase::WindowManagement::setWindowMotifHint(aboutWinWidget->winId());

        if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(aboutWinWidget);
            windowHelper->removeTitleBar();
            windowHelper->setSkipSwitcher(true);
            windowHelper->setSkipTaskBar(true);
        } else {
            kdk::UkuiStyleHelper::self()->removeHeader(aboutWinWidget);
            kdk::WindowManager::setSkipTaskBar(aboutWinWidget->windowHandle(),true);
            kdk::WindowManager::setSkipSwitcher(aboutWinWidget->windowHandle(),true);
        }


        aboutWinWidget->show();
        aboutWinWidget->update();

        //设置about弹窗居中显示
        aboutWinWidget->move(main_widget->geometry().x() + (main_widget->width() - aboutWinWidget->width())/2,
                             main_widget->geometry().y() + (main_widget->height() - aboutWinWidget->height())/2);
        aboutWinWidget->exec();
    }
    if(str == tr("Set")){
        if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
            setPage = new SettingPage(main_widget);
            setPage->setAccessibleName("kylin-camera_setting_SettingPage_setPage");
        }else{
            setPage = new SettingPage(this);
            setPage->setAccessibleName("kylin-camera_setting_SettingPage_setPage");
        }
        QString locale = QLocale::system().name();
        if(locale == "bo_CN"){
            QPoint pos = QWidget::mapToGlobal(QPoint((CurrentDeviceInfo::winWidth - SETTINGPAGEWIDTHMAX)/2,
                                                     (CurrentDeviceInfo::winHeight + MAINWINDOWTITLEBARHEIGHT - SETTINGPAGEHEIGHTMAX)/2));
            setPage->Geometry(&pos);
            setPage->setMinimumSize(SETTINGPAGEWIDTHMAX,SETTINGPAGEHEIGHTMAX);
            setPage->setMaximumSize(SETTINGPAGEWIDTHMAX,SETTINGPAGEHEIGHTMAX);
        }else{
            QPoint pos = QWidget::mapToGlobal(QPoint((CurrentDeviceInfo::winWidth - SETTINGPAGEWIDTH)/2,
                                                     (CurrentDeviceInfo::winHeight + MAINWINDOWTITLEBARHEIGHT - SETTINGPAGEHEIGHT)/2));
            setPage->Geometry(&pos);
            setPage->setMinimumSize(SETTINGPAGEWIDTH,SETTINGPAGEHEIGHT);
            setPage->setMaximumSize(SETTINGPAGEWIDTH,SETTINGPAGEHEIGHT);
        }

        setPage->setWindowModality(Qt::ApplicationModal);
        setPage->setWindowFlag(Qt::Tool);  //如果on为true，则在此小部件上设置窗口标志；否则清除标志
        kdk::kabase::WindowManagement::setWindowMotifHint(setPage->winId());

        if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(setPage);
            windowHelper->removeTitleBar();
            windowHelper->setSkipSwitcher(true);
            windowHelper->setSkipTaskBar(true);
        } else {
            kdk::UkuiStyleHelper::self()->removeHeader(setPage);
            kdk::WindowManager::setSkipTaskBar(setPage->windowHandle(),true);
            kdk::WindowManager::setSkipSwitcher(setPage->windowHandle(),true);
        }


        setPage->show();
        setPage->move(main_widget->geometry().x() + (main_widget->width() - setPage->width())/2,
                      main_widget->geometry().y() + (main_widget->height() - setPage->height())/2);
    }
    if(str == tr("Quit")){
        main_widget->close();
    }
}


