#ifndef ITRAYVOLUMESLIDERITEM_H
#define ITRAYVOLUMESLIDERITEM_H

#include <memory>
#include <QLabel>
#include <QPushButton>

class ITrayVolumeSlider {
public:
    virtual void setRang(int, int) = 0;
    virtual void setValue(int) = 0;
    virtual QWidget* getWidget() = 0;
};

class ITrayVolumeSliderItem {
public:
    ITrayVolumeSliderItem(std::shared_ptr<ITrayVolumeSlider>, QPushButton*);
    virtual ~ITrayVolumeSliderItem() {}

public:
    virtual void initUi() = 0;

    virtual void initSlots() = 0;

    virtual void setIcon(const QIcon&) = 0;

    virtual QWidget* getWidget() = 0;

    virtual void setValue(int) = 0;

    virtual QPushButton* getMuteButton() = 0;

    std::shared_ptr<ITrayVolumeSlider> getSlider() const;

    QPushButton* getIconButton() const;

protected:
    std::shared_ptr<ITrayVolumeSlider> m_pVolumeSlider = nullptr;
    QPushButton* m_pIconBtn = nullptr;
};

#endif // ITRAYVOLUMESLIDERITEM_H
