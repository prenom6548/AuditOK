#ifndef ISWITCHBUTTONITEM_H
#define ISWITCHBUTTONITEM_H

#include <memory>
#include <QLabel>
#include <QObject>

/**
* @brief 包含SwitchButton子项的类型
*/
enum class SwitchButtonType
{
    SWITCH_BUTTON_VOLUMEBOOST = 0, // 音量增强子项
    SWITCH_BUTTON_MONO,            // 单声道子项
    SWITCH_BUTTON_ECHOCANCEL,      // 智能降噪子项
    SWITCH_BUTTON_LOOPBACK,        // 侦听子项
    SWITCH_BUTTON_STARTUP,         // 开机音乐子项
    SWITCH_BUTTON_POWEROFF,        // 关机音乐子项
    SWITCH_BUTTON_LOGOUT,          // 注销音乐子项
    SWITCH_BUTTON_WAKEUP,          // 唤醒音乐子项
    SWITCH_BUTTON_ALERT,           // 提示音子项
    SWITCH_BUTTON_AUTO_PAUSE       // 断开音频自动暂停子项
};

class ISwitchButton {
public:
    virtual bool isChecked() const = 0;

    virtual void setChecked(bool) = 0;

    virtual QWidget* getWidget() = 0;
};

class ISwitchButtonItem {
public:
    ISwitchButtonItem(std::shared_ptr<ISwitchButton>, QLabel*, SwitchButtonType);
    virtual ~ISwitchButtonItem() = default;

public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual QWidget* getWidget() = 0;

    void setChecked(bool);

    std::shared_ptr<ISwitchButton> getSwitchButton() const;

protected:
    SwitchButtonType m_type = SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST;
    std::shared_ptr<ISwitchButton> m_pSwitchBtn = nullptr;
    QLabel* m_pDisplayLabel = nullptr;
};

#endif // ISWITCHBUTTONITEM_H
