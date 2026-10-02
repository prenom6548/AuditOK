#ifndef UKUI2VOLUMESLIDERITEM_H
#define UKUI2VOLUMESLIDERITEM_H

#include <memory>
#include <QFrame>
#include <QSlider>
#include <QPushButton>
#include "IVolumeSliderItem.h"

///**
//* @brief 包含VolumeSlider子项的类型
//*/
//enum class VolumeSliderType
//{
//    VOLUME_SLIDER_INPUT = 0,  // 输入音量子项
//    VOLUME_SLIDER_OUTPUT,     // 输出音量子项
//    VOLUME_SLIDER_BALANCE,    // 平衡音量子项
//    VOLUME_SLIDER_INPUTLEVEL  // 输入反馈子项
//};

class Ukui2VolumeSlider : public IVolumeSlider
{
public:
    Ukui2VolumeSlider(QSlider*);
    ~Ukui2VolumeSlider() = default;

public:
    virtual void setRange(int, int) override;

    virtual void setValue(int) override;

    virtual void setEnabled(bool) override;

    virtual bool isEnabled() const override;

    virtual uint32_t getValue() const override;

    virtual QWidget* getWidget() override;

private:
    QSlider* m_pSlider = nullptr;
};


class Ukui2VolumeSliderItem : public QFrame, public IVolumeSliderItem
{
public:
    Ukui2VolumeSliderItem(std::shared_ptr<IVolumeSlider>, QLabel*, QLabel*, QPushButton*, QWidget* = nullptr);
    ~Ukui2VolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

    virtual void setValue(int) override;

    virtual void setMute(bool) override;

    virtual void setEnabled(bool) override;

    virtual QPushButton* getMuteBtn() const override;

    void setIcon(const QIcon&);

    void setPercent(int);

private:
    QLabel* m_pPercentLabel = nullptr;
    QPushButton* m_pMuteBtn = nullptr;
};


#endif // UKUI2VOLUMESLIDERITEM_H
