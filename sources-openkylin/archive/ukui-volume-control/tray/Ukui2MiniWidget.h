#ifndef UKUI2MINIWIDGET_H
#define UKUI2MINIWIDGET_H

#include <QWidget>
#include <QLabel>

#include "ITrayVolumeSliderItem.h"

class Ukui2MiniWidget : public QWidget
{
public:
    Ukui2MiniWidget(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>&, QPushButton*, QWidget*);
    ~Ukui2MiniWidget() = default;

public:
    void setDescription(const QString&);

private:
    void initUi();

    void initSlots();

private:
    QLabel* m_pLabel = nullptr;
    QPushButton* m_pIconBtn = nullptr;
    QPushButton* m_pSwitchModuleBtn = nullptr;
    std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& m_sysVolumeSliderItemMap;
};

#endif // UKUI2MINIWIDGET_H
