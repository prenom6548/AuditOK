#ifndef IVOLUMESLIDERITEM_H
#define IVOLUMESLIDERITEM_H

#include <memory>
#include <QLabel>
#include <QPushButton>

/**
* @brief 包含VolumeSlider子项的类型
*/
enum class VolumeSliderType
{
    VOLUME_SLIDER_INPUT = 0,  // 输入音量子项
    VOLUME_SLIDER_OUTPUT,     // 输出音量子项
    VOLUME_SLIDER_BALANCE,    // 平衡音量子项
    VOLUME_SLIDER_INPUTLEVEL, // 输入反馈子项
    VOLUME_SLIDER_SINKINPUT
};

static const char* iconNameOutputs[] = {
        "audio-volume-muted-symbolic",
        "audio-volume-low-symbolic",
        "audio-volume-medium-symbolic",
        "audio-volume-high-symbolic",
        NULL
};

static const char* iconNameInputs[] = {
        "microphone-sensitivity-muted-symbolic",
        "microphone-sensitivity-low-symbolic",
        "microphone-sensitivity-medium-symbolic",
        "microphone-sensitivity-high-symbolic",
        NULL
};

class IVolumeSlider {
public:
    virtual void setRange(int, int) = 0;

    virtual void setValue(int) = 0;

    virtual void setEnabled(bool) = 0;

    virtual bool isEnabled() const = 0;

    virtual uint32_t getValue() const = 0;

    virtual QWidget* getWidget() = 0;
};

class IVolumeSliderItem {
public:
    IVolumeSliderItem(std::shared_ptr<IVolumeSlider>, QLabel*);
    virtual ~IVolumeSliderItem() = default;

public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual QWidget* getWidget() = 0;

    virtual void setValue(int) = 0;

    virtual void setMute(bool) = 0;

    virtual void setEnabled(bool) = 0;

    virtual QPushButton* getMuteBtn() const = 0;

    std::shared_ptr<IVolumeSlider> getSlider() const;

    void setRange(int, int);

protected:
    std::shared_ptr<IVolumeSlider> m_pVolumeSlider = nullptr;
    QLabel* m_pDisplayLabel = nullptr;
};

#endif // IVOLUMESLIDERITEM_H
