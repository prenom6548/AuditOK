#ifndef CONCRETESTRATEGY_H
#define CONCRETESTRATEGY_H

#include "VersionStrategy.h"
#include "Ukui5MainWidget.h"
#include "Ukui4MainWidget.h"

class Ukui2ConcreteStrategy : public VersionStrategy {
public:
    virtual IAudioMainWindow* createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>&,
                                               std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                                               std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                                               std::unordered_map<SelectComboxType, ISelectComboxItem*>&,
                                               std::unordered_map<DetailSettingType, IDetailSettingsItem*>&,
                                               std::list<AppManagerItem>&,
                                               QWidget* = nullptr) override;
};

class Ukui4ConcreteStrategy : public VersionStrategy {
public:
    virtual IAudioMainWindow* createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>&,
                                               std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                                               std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                                               std::unordered_map<SelectComboxType, ISelectComboxItem*>&,
                                               std::unordered_map<DetailSettingType, IDetailSettingsItem*>&,
                                               std::list<AppManagerItem>&,
                                               QWidget* = nullptr) override;
};

class Ukui5ConcreteStrategy : public VersionStrategy {
public:
    virtual IAudioMainWindow* createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>&,
                                               std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                                               std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                                               std::unordered_map<SelectComboxType, ISelectComboxItem*>&,
                                               std::unordered_map<DetailSettingType, IDetailSettingsItem*>&,
                                               std::list<AppManagerItem>&,
                                               QWidget* = nullptr) override;
};

#endif // CONCRETESTRATEGY_H
