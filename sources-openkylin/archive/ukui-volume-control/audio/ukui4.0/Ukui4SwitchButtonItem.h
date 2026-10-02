#ifndef UKUI4SWITCHBUTTONITEM_H
#define UKUI4SWITCHBUTTONITEM_H

#include <unordered_map>
#include <kswitchbutton.h>

#include "ISwitchButtonItem.h"

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

class Ukui4SwitchButton : public ISwitchButton
{
public:
    Ukui4SwitchButton(QPushButton*, const QString& = "");
    ~Ukui4SwitchButton() = default;

public:
    virtual bool isChecked() const override;

    virtual void setChecked(bool) override;

    virtual QWidget* getWidget() override;

private:
    QPushButton* m_pSwitchBtn = nullptr;
};

class Ukui4SwitchButtonItem : public QFrame, public ISwitchButtonItem
{
    Q_OBJECT
public:
    Ukui4SwitchButtonItem(std::shared_ptr<ISwitchButton>, QLabel*, SwitchButtonType, QWidget* = nullptr);
    ~Ukui4SwitchButtonItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

private Q_SLOTS:
    void stateChangedSlots(bool);
};

class Ukui4SwitchButtonWithDescItem : public QFrame, public ISwitchButtonItem
{
    Q_OBJECT
public:
    Ukui4SwitchButtonWithDescItem(std::shared_ptr<ISwitchButton>, QLabel*, QLabel*, SwitchButtonType, QWidget* = nullptr);

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

private:
    QLabel* m_pDescLabel;
};

#endif // UKUI4SWITCHBUTTONITEM_H
