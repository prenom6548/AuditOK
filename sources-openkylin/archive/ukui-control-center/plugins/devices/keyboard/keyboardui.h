#ifndef KEYBOARDUI_H
#define KEYBOARDUI_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <kysdk/applications/klabel.h>

#include "settinggroup.h"
#include "titlelabel.h"
#include "switchwidget.h"
#include "sliderwidget.h"
#include "lineeditwidget.h"
#include "comboxwidget.h"
#include "pushbuttonwidget.h"

#define SMALLTEXTSIZE 24
#define MEDIUMTEXTSIZE 36
#define LARGETEXTSIZE 48

class KeyboardUi : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardUi(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~KeyboardUi();

public:
    inline SwitchWidget *keyRepeatFrame()
    {
        return mKeyRepeatFrame;
    }

    inline SliderWidget *delayFrame()
    {
        return mDelayFrame;
    }

    inline SliderWidget *speedFrame()
    {
        return mSpeedFrame;
    }

    inline LineEditWidget *inputTestFrame()
    {
        return mInputTestFrame;
    }

    inline SwitchWidget *keyTipsFrame()
    {
        return mKeyTipsFrame;
    }

    inline UkccFrame *inputMethodFrame()
    {
         return mInputMethodEditFrame;
    }

    inline QPushButton *inputMethodEditButton()
    {
        return mInputMethodEditBtn;
    }

    inline SwitchWidget *showIconOnTrayFrame()
    {
        return mShowIconOnTrayFrame;
    }

    inline SwitchWidget *showIconOnDesktopFrame()
    {
        return mShowIconOnDesktopFrame;
    }

    inline SliderWidget *virtualKeyboardSizeFrame()
    {
        return mVirtualKeyboardSizeFrame;
    }

    inline ComboxWidget *textSizeFrame()
    {
        return mTextSizeFrame;
    }

    inline SwitchWidget *animationFrame()
    {
        return mAnimationFrame;
    }

    inline SwitchWidget *soundEfectsFrame()
    {
        return mSoundEfectsFrame;
    }

    void setFcitxServiceWarningVisible(bool isVisible); // 设置显示警告信息是否可见

    void showFcitxServiceWarningDialog(); //提示fcitx服务不存在弹窗

    void setCurrentInputMethodInfo(const QString &name); // 设置当前输入法名称

    void setCurrentInputMethodVisible(bool isVisible); // 切换显示当前输入法或警告信息

private:
    QVBoxLayout *mVlayout = nullptr;

    SettingGroup *mKeyboardFrame = nullptr;
    SettingGroup *mVirtualKeyboardFrame = nullptr;
    TitleLabel *mKeyboardSetTitleLabel = nullptr;
    TitleLabel *mVirtualKeyboardSetTitleLabel = nullptr;
    TitleLabel *mInputTitleLabel = nullptr;

    SwitchWidget *mKeyRepeatFrame = nullptr;            //按键重复
    SliderWidget *mDelayFrame = nullptr;                //延迟
    SliderWidget *mSpeedFrame = nullptr;                //速度
    LineEditWidget *mInputTestFrame = nullptr;          //输入测试
    SwitchWidget *mKeyTipsFrame = nullptr;              //按键提示
    SwitchWidget *mShowIconOnTrayFrame = nullptr;       // 在托盘显示虚拟键盘图标
    SwitchWidget *mShowIconOnDesktopFrame = nullptr;    // 在桌面显示虚拟键盘图标
    SliderWidget *mVirtualKeyboardSizeFrame = nullptr;  // 键盘大小
    ComboxWidget *mTextSizeFrame = nullptr;        // 按键文本大小
    SwitchWidget *mAnimationFrame = nullptr;            // 动画效果
    SwitchWidget *mSoundEfectsFrame = nullptr;          // 按键输入音效

    // 输入法框架不存在时，提示的布局
    UkccFrame *mInputMethodEditFrame = nullptr;
    QHBoxLayout *mInputMethodHLayout = nullptr;
    QWidget *mInputMethodWidget = nullptr;
    KLabel *mInputMethodLabel = nullptr;
    QHBoxLayout *mInputMethodWarningLayout = nullptr;
    LightLabel *mInputMethodServiceWarning = nullptr;
    KLabel *mInputMethodServiceWarningIcon = nullptr;
    QPushButton *mInputMethodEditBtn = nullptr;

    // 显示当前输入法的布局（与警告布局互斥）
    QWidget *mCurrentInputMethodWidget = nullptr;
    QHBoxLayout *mCurrentInputMethodLayout = nullptr;
    LightLabel *mFirstInputMethodInfo = nullptr;

private:
    void initUI();
    void setKeyRepeatFrame();
    void setDelayFrame();
    void setSpeedFrame();
    void setInputTestFrame();
    void setKeyTipsFrame();
    void setInputMethodFrame();

    void setShowIconOnTrayFrame();
    void setShowIconOnDesktopFrame();
    void setVirtualKeyboardSizeFrame();
    void setTextSizeFrame();
    void setAnimationFrame();
    void setSoundEfectsFrame();
};

#endif // KEYBOARDUI_H
