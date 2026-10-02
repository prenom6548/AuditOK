#ifndef UKUI2SYSTEMVOLUMESLIDERITEM_H
#define UKUI2SYSTEMVOLUMESLIDERITEM_H

#include <QSlider>

#include "ITrayVolumeSliderItem.h"
#include "Ukui4SystemVolumeSliderItem.h"

class Ukui2SystemVolumeSlider : public ITrayVolumeSlider
{
public:
    Ukui2SystemVolumeSlider(QSlider*);
    ~Ukui2SystemVolumeSlider() = default;

public:
    virtual void setRang(int, int) override;

    virtual void setValue(int) override;

    virtual QWidget* getWidget() override;

private:
    QSlider* m_pSlider = nullptr;
};

class Ukui2SystemVolumeSliderItem : public QWidget, public ITrayVolumeSliderItem
{
    Q_OBJECT
public:
    Ukui2SystemVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider>, QLabel*, QPushButton*, const VolumeSliderType&, QWidget* = nullptr);
    ~Ukui2SystemVolumeSliderItem() = default;

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

class Ukui2DeviceVolumeSliderItem : public QWidget, public ITrayVolumeSliderItem
{
    Q_OBJECT
public:
    Ukui2DeviceVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider>, QLabel*, QPushButton*, QPushButton*, const VolumeSliderType&, QWidget* = nullptr);
    ~Ukui2DeviceVolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

    virtual void setIcon(const QIcon&) override;

    virtual void setValue(int) override;

    virtual QPushButton* getMuteButton() override;

    void setPercent(int);

    void setDescription(const QString&);

Q_SIGNALS:
    void valueChangedSignals(int);

private Q_SLOTS:
    void valueChangedSlots(int);

private:
    void updateMuteBtnIcon(int, bool = false);

private:
    VolumeSliderType m_type = VolumeSliderType::VOLUME_SLIDER_SYSTEM_OUTPUT;
    QLabel* m_pDescLabel = nullptr;
    QPushButton* m_pMuteBtn = nullptr;
};

class Ukui2AppVolumeSlider : public ITrayVolumeSlider
{
public:
    Ukui2AppVolumeSlider(QSlider*);
    ~Ukui2AppVolumeSlider() = default;

public:
    virtual void setRang(int, int) override;

    virtual void setValue(int) override;

    virtual QWidget* getWidget() override;

private:
    QSlider* m_pSlider = nullptr;
};


class Ukui2AppVolumeSliderItem : public QWidget, public ITrayVolumeSliderItem
{
    Q_OBJECT
public:
    Ukui2AppVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider>, QPushButton*, QPushButton*, QLabel*, QWidget* = nullptr);
    ~Ukui2AppVolumeSliderItem() = default;

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
    QLabel* m_pDisplayLabel = nullptr;
    QPushButton* m_pMuteBtn = nullptr;
};


#endif // UKUI2SYSTEMVOLUMESLIDERITEM_H
