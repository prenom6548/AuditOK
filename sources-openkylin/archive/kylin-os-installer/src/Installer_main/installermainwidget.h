#ifndef INSTALLERMAINWIDGET_H
#define INSTALLERMAINWIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QStackedWidget>
#include "../PluginService/ui_unit/arrowtooltip.h"
// #include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginDll/plugindll.h"
#include "installingframe.h"
#include "finishedInstall.h"
#include "installingoemconfigframe.h"
#include "../plugins/VirtualKeyboard/src/virtualkeyboard.h"
#include "../plugins/VirtualKeyboard/src/keyboardwidget.h"
#include "../plugins/KChoiceLanguage/languageframe.h"

#include "QTimer"

namespace KInstaller{

class InstallerMainWidget : public QWidget
{
    Q_OBJECT

public:
    InstallerMainWidget(QWidget *parent = 0);
    ~InstallerMainWidget();
    void setInstallMode(QString flagstr);

    QString m_InstallMode;
    int AllStep;
    QTimer *timer;

private:
    void initUI();                              //加载界面ui
    void initBackground();                      //背景初始化
    void initAllConnect();                      //信号和槽链接
    void initFocusPolicy();                     //初始化走焦策略
    void addStyleSheet();                       //设置样式

    void addStackWidget();
    void setLabelDotVisible(bool flag);
    void changeLabelDotPix(int step);
    void updateButtonStatus();
    bool eventFilter(QObject *watched, QEvent *event);

    bool showOEMInstallingFrame();

    void initToolTip(QPushButton* button);
private:
    QPushButton* m_backbtn = nullptr;           //麒麟图标
    QPushButton* m_clostBtn = nullptr;          //关闭按钮
    QPushButton* m_keyboardBtn = nullptr;       //键盘按钮
    QPushButton* m_powerOffBtn = nullptr;       //电源按钮
 //    MiddleFrameManager* m_middleFrame = nullptr;//中间窗显示
    QStackedWidget* m_stackWidget = nullptr;    //存放全部插件界面，每个插件对应一个子界面
    int m_curIndex = 0;                         //当前显示界面在列表m_stackWidget中的下标
    int ImgIndex=0;                             //记录选择安装途径界面在列表m_stackWidget中的下标
    int ImgNext=0;                              //记录选择安装方式界面在列表m_stackWidget中的下标
    int m_stackCount = 0;

    QList<QPair<std::string,IPlugin*>> m_stackPageMap;
    QList<QPair<std::string,IPlugin*>>::iterator it;
    InstallingFrame* m_installingFrame = nullptr;
    FinishedFrame* m_finishedFrame = nullptr;
    InstallingOEMConfigFrame* m_oemFrame = nullptr;
    UserFrame *m_UFplugin = nullptr;

    QList<QLabel*> m_steps;                    //导航栏内图标列表，一个图标对应一个插件
    QHBoxLayout* m_dotlayout;                  //导航栏布局
    //ArrowWidget *w;
    QDialog *w;
    VirtualKeyboard* keyboard;
    int SCreenWidth;
    int SCreenHeight;
    bool m_show=true;
    bool m_flagofGhost;                        //是否点击过ghost安装
    int  m_ignoreUIsofGhost;                   //ghost安装时被略过的插件数目
    bool m_isIgnoreChoiceEdition = false;	

public slots:

    void clickCloseButton();                    //关闭按钮槽处理
    void clickkeyButton();                      //键盘按钮槽处理
    void clickPowerButton();                    //电源按钮槽处理
    void clickBackButton();

    void changeStackPage(int step);
    void addStackWidgetPage(QWidget* widget);

    void showInstallingFrame();
    void showInstallingErrorFrame(QString str);
    void showFinishedFrame();
     void ScreenAdapt();

signals:
    void signalScriptFinished();
    void signalInstalling();
    void signalInstallingClose();
    void signalcloseArrowWidget();
    void backwidgetcheck(QWidget* widget);
    void sendmessage(QWidget* widget,QString str);
    void signalMainFrameSize(QWidget *pFrame, int SCreenWidth, int SCreenHeight);

    // QWidget interface
protected:
   // bool focusNextPrevChild(bool next);

};
}
#endif // INSTALLERMAINWIDGET_H
