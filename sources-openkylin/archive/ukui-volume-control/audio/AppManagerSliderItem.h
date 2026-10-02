#ifndef APPMANAGERSLIDERITEM_H
#define APPMANAGERSLIDERITEM_H

#include <QFrame>

#include "IVolumeSliderItem.h"

class AppManagerSliderItem : public QFrame, public IVolumeSliderItem
{
    Q_OBJECT
public:
    AppManagerSliderItem(std::shared_ptr<IVolumeSlider>, QLabel*, QLabel*, QPushButton*, VolumeSliderType, QWidget* = nullptr);
    ~AppManagerSliderItem() = default;

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

private:
    VolumeSliderType m_type = VolumeSliderType::VOLUME_SLIDER_INPUT;
    QLabel* m_pPercentLabel = nullptr;
    QPushButton* m_pMuteBtn = nullptr;
    bool m_muted = false;
};

#endif // APPMANAGERSLIDERITEM_H
