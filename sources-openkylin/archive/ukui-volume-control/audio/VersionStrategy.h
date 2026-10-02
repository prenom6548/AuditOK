#ifndef VERSIONSTRATEGY_H
#define VERSIONSTRATEGY_H

#include "IAudioMainWidow.h"

class VersionStrategy {
public:
    virtual IAudioMainWindow* createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>&,
                                               std::unordered_map<SwitchButtonType, ISwitchButtonItem*>&,
                                               std::unordered_map<VolumeSliderType, IVolumeSliderItem*>&,
                                               std::unordered_map<SelectComboxType, ISelectComboxItem*>&,
                                               std::unordered_map<DetailSettingType, IDetailSettingsItem*>&,
                                               std::list<AppManagerItem>&,
                                               QWidget* = nullptr) = 0;
    virtual ~VersionStrategy() = default;
};

#endif // VERSIONSTRATEGY_H
