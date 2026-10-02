#ifndef UKUI4VOLUMESLIDERITEM_H
#define UKUI4VOLUMESLIDERITEM_H

#include <memory>
#include <QFrame>
#include <QSlider>
#include <QPushButton>
#include <QProgressBar>
#include <QApplication>
#include <QFontMetrics>
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

class Ukui4VolumeSlider : public IVolumeSlider
{
public:
    Ukui4VolumeSlider(QSlider*, const QString& = "");
    ~Ukui4VolumeSlider() = default;

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

class Ukui4VolumeSliderItem : public QFrame, public IVolumeSliderItem
{
    Q_OBJECT
public:
    Ukui4VolumeSliderItem(std::shared_ptr<IVolumeSlider>, QLabel*, QLabel*, QPushButton*, VolumeSliderType, QWidget* = nullptr);
    ~Ukui4VolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

    virtual void setValue(int) override;

    virtual void setEnabled(bool) override;

    virtual QPushButton* getMuteBtn() const override;

    virtual void setMute(bool) override;

private Q_SLOTS:
    void valueChangedSlots(int);

private:
    void setPercent(int);

    void setIcon(const QIcon&);

    void updateIcon(int);

    void updateLabelWidth();

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    VolumeSliderType m_type = VolumeSliderType::VOLUME_SLIDER_INPUT;
    QLabel* m_pPercentLabel = nullptr;
    QPushButton* m_pMuteBtn = nullptr;
    bool m_muted = false;
};

class Ukui4BalanceVolumeSlider : public IVolumeSlider
{
public:
    Ukui4BalanceVolumeSlider(QSlider*, const QString& = "");
    ~Ukui4BalanceVolumeSlider() = default;

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

class Ukui4BalanceVolumeSliderItem : public QFrame, public IVolumeSliderItem
{
public:
    Ukui4BalanceVolumeSliderItem(std::shared_ptr<IVolumeSlider>, QLabel*, QLabel*, QLabel*, QWidget* = nullptr);
    ~Ukui4BalanceVolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

    virtual void setValue(int) override;

    virtual void setMute(bool) override;

    virtual void setEnabled(bool) override;

    virtual QPushButton* getMuteBtn() const override;

private:
    QLabel* m_pLeftLabel = nullptr;
    QLabel* m_pRightLabel = nullptr;
};

class Ukui4InputLevelVolumeSlider : public IVolumeSlider
{
public:
    Ukui4InputLevelVolumeSlider(QProgressBar*);
    ~Ukui4InputLevelVolumeSlider() = default;

public:
    virtual void setRange(int, int) override;

    virtual void setValue(int) override;

    virtual void setEnabled(bool) override;

    virtual bool isEnabled() const override;

    virtual uint32_t getValue() const override;

    virtual QWidget* getWidget() override;

private:
    QProgressBar* m_pProgressBar = nullptr;
};

class Ukui4InputLevelVolumeSliderItem : public QFrame, public IVolumeSliderItem
{
public:
    Ukui4InputLevelVolumeSliderItem(std::shared_ptr<IVolumeSlider>, QLabel*, QWidget* = nullptr);
    ~Ukui4InputLevelVolumeSliderItem() = default;

public:
    virtual void initUi() override;

    virtual void initSlots() override;

    virtual QWidget* getWidget() override;

    virtual void setValue(int) override;

    virtual void setMute(bool) override;

    virtual void setEnabled(bool) override;

    virtual QPushButton* getMuteBtn() const override;
};


#endif // UKUI4VOLUMESLIDERITEM_H
