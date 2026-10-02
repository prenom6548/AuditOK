#ifndef USERFRAME_H
#define USERFRAME_H

#include <QWidget>
#include <../PluginService/ui_unit/middleframemanager.h>
#include <QStackedWidget>
#include "conferquestionframe.h"
#include "userregisterframe.h"
#include "modeselectframe.h"
#include "securityquestions.h"

namespace KInstaller {

class UserFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit UserFrame(QWidget *parent = nullptr);
    ~UserFrame();
    void writeSettingIni(quint8 flag);
    QString SourcePath;
    void setBtnValue();
    void setButtonNext();
    void setButtonStart();
    void changeNavigations(bool isIgnor);
    void addStyleSheet();                       //添加窗口样式
    void initFocusPolicy();                     //初始化焦点序列
    void reSetFocusPolicy();                     //重新设置焦点序列
signals:
    void signalConfirmPasswd();
    void savRegisterDateSignal();
    void savconferDateSignal();
    void translateStrSignal();
    void currentwidgetchange(QWidget* p);

public slots:
    void clickNextButton();
    void clickPrevButton(QWidget *p) override;
    void getmessage(QWidget *p,QString str);
    void enableNextButtonSlot(bool value);
    void changeNextButtonSlot();
    void modeselslots(int index);
public:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);

    QStackedWidget* stack = nullptr;
    modeselect* modesel = nullptr; //1,“创建账户”子界面，是第一个显示的界面
    userregisterwidiget* usregister = nullptr; //2,“创建用户”子界面，当在“创建账户”子界面中点击“立刻创建”时(并点击下一步按钮后)，才会出现本界面
    securityquestions *m_securityquest = nullptr;//3,“密码安全问题”子界面
    quint8 curIndex=0;//当前显示的子界面
    conferquestion* confer;
//    bool m_registerLater = false;

    bool m_startChangeModule = false;
    bool m_lastregisterLaterstatus = false;
    bool m_lastregisterNowstatus = false;


};
}
#endif // USERFRAME_H
