#ifndef UKUI4SYSTEMVOLUMESLIDERITEM_H
#define UKUI4SYSTEMVOLUMESLIDERITEM_H

#include <memory>
#include <QWidget>
#include <QSlider>
#include <QPushButton>
#include "ITrayVolumeSliderItem.h"

static const char* iconNameOutputs[] = {
        "audio-volume-muted-symbolic",
        "audio-volume-low-symbolic",
        "audio-volume-medium-symbolic",
        "audio-volume-high-symbolic",
        NULL
};

/**
* @brief 包含VolumeSlider子项的类型
*/
enum class VolumeSliderType
{
    VOLUME_SLIDER_MINI_OUTPUT = 0, // 输出音量子项
    VOLUME_SLIDER_DEVICE_OUTPUT,
    VOLUME_SLIDER_DEVICE_INPUT,
    VOLUME_SLIDER_APP_OUTPUT,
    VOLUME_SLIDER_SYSTEM_OUTPUT
};

class Ukui4SystemVolumeSlider : public ITrayVolumeSlider
{
public:
    Ukui4SystemVolumeSlider(QSlider*);
    ~Ukui4SystemVolumeSlider() = default;

public:
    virtual void setRang(int, int) override;

    virtual void setValue(int) override;

    virtual QWidget* getWidget() override;

private:
    QSlider* m_pSlider = nullptr;
};

class Ukui4SystemVolumeSliderItem : public QWidget, public ITrayVolumeSliderItem
{
    Q_OBJECT
public:
    Ukui4SystemVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider>, QLabel*, QPushButton*, const VolumeSliderType&, QWidget* = nullptr);
    ~Ukui4SystemVolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

    virtual void setIcon(const QIcon&) override;

    virtual void setValue(int) override;

    virtual QPushButton* getMuteButton() override;

    void setPercent(int);

Q_SIGNALS:
    void valueChangedSignals(int);

private Q_SLOTS:
    void valueChangedSlots(int);

private:
    void updateMuteBtnIcon(int, bool = false);

private:
    VolumeSliderType m_type = VolumeSliderType::VOLUME_SLIDER_SYSTEM_OUTPUT;
    QLabel* m_pPercentLabel = nullptr;
};

class Ukui4AppVolumeSlider : public ITrayVolumeSlider
{
public:
    Ukui4AppVolumeSlider(QSlider*);
    ~Ukui4AppVolumeSlider() = default;

public:
    virtual void setRang(int, int) override;

    virtual void setValue(int) override;

    virtual QWidget* getWidget() override;

private:
    QSlider* m_pSlider = nullptr;
};


class Ukui4AppVolumeSliderItem : public QWidget, public ITrayVolumeSliderItem
{
    Q_OBJECT
public:
    Ukui4AppVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider>, QPushButton*, QLabel*, QWidget* = nullptr);
    ~Ukui4AppVolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual void setIcon(const QIcon&) override;

    virtual QWidget* getWidget() override;

    virtual void setValue(int) override;

    virtual QPushButton* getMuteButton() override;

private:
    void setPercent(int);

Q_SIGNALS:
    void valueChangedSignals(int);

private Q_SLOTS:
    void valueChangedSlots(int);

private:
    QLabel* m_pPercentLabel = nullptr;
};


#endif // UKUI4SYSTEMVOLUMESLIDERITEM_H
