#ifndef UKUI2SWITCHBUTTONITEM_H
#define UKUI2SWITCHBUTTONITEM_H

#include <unordered_map>
#include <QPushButton>
#include "ISwitchButtonItem.h"
#include "include/SwitchButton.h"

///**
//* @brief 包含SwitchButton子项的类型
//*/
//enum class SwitchButtonType
//{
//    SWITCH_BUTTON_VOLUMEBOOST = 0, // 音量增强子项
//    SWITCH_BUTTON_MONO,            // 单声道子项
//    SWITCH_BUTTON_ECHOCANCEL,      // 智能降噪子项
//    SWITCH_BUTTON_LOOPBACK,        // 侦听子项
//    SWITCH_BUTTON_STARTUP,         // 开机音乐子项
//    SWITCH_BUTTON_POWEROFF,        // 关机音乐子项
//    SWITCH_BUTTON_LOGOUT,          // 注销音乐子项
//    SWITCH_BUTTON_WAKEUP,          // 唤醒音乐子项
//    SWITCH_BUTTON_ALERT            // 提示音子项
//};

class Ukui2SwitchButton : public ISwitchButton
{
public:
    Ukui2SwitchButton(SwitchButton*);
    ~Ukui2SwitchButton() = default;

public:
    virtual bool isChecked() const override;

    virtual void setChecked(bool) override;

    virtual QWidget* getWidget() override;

private:
    SwitchButton* m_pSwitchBtn = nullptr;
};

class Ukui2SwitchButtonItem : public QFrame, public ISwitchButtonItem
{
    Q_OBJECT
public:
    Ukui2SwitchButtonItem(std::shared_ptr<ISwitchButton>, QLabel*, SwitchButtonType, QWidget* = nullptr);
    ~Ukui2SwitchButtonItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;
};

class Ukui2SwitchButtonWithDescItem : public QFrame, public ISwitchButtonItem
{
    Q_OBJECT
public:
    Ukui2SwitchButtonWithDescItem(std::shared_ptr<ISwitchButton>, QLabel*, QLabel*, SwitchButtonType, QWidget* = nullptr);
    ~Ukui2SwitchButtonWithDescItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

private:
    QLabel* m_pDescLabel;
};


//class Ukui2SwitchButton : public ISwitchButton
//{
//public:
//    virtual void setChecked(bool checked) override;
//    virtual QWidget* getWidget() override;

//private:
//    SwitchButton* m_pSwitchBtn = nullptr;
//};

//class Ukui2SwitchButtonItem : public QFrame, public ISwitchButtonItem
//{
//    Q_OBJECT
//public:
//    Ukui2SwitchButtonItem(std::shared_ptr<ISwitchButton> btn, QLabel* label, QWidget* parent = nullptr);
//    ~Ukui2SwitchButtonItem() = default;

//public:
//    virtual void initUi() override;
//    virtual QWidget* getWidget() override;
//};

#endif // UKUI2SWITCHBUTTONITEM_H
