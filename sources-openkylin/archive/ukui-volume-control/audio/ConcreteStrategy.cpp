/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "ConcreteStrategy.h"

IAudioMainWindow* Ukui2ConcreteStrategy::createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>& titleLabelMap,
                                                          std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& switchBtnMap,
                                                          std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& volumeSliderMap,
                                                          std::unordered_map<SelectComboxType, ISelectComboxItem*>& selectComboxMap,
                                                          std::unordered_map<DetailSettingType, IDetailSettingsItem*>& detailSettingsMap,
                                                          std::list<AppManagerItem>& appManagerList,
                                                          QWidget* parent)
{
    return new Ukui4MainWidget(titleLabelMap, switchBtnMap, volumeSliderMap, selectComboxMap, detailSettingsMap, appManagerList, parent);
}

IAudioMainWindow* Ukui4ConcreteStrategy::createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>& titleLabelMap,
                                                          std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& switchBtnMap,
                                                          std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& volumeSliderMap,
                                                          std::unordered_map<SelectComboxType, ISelectComboxItem*>& selectComboxMap,
                                                          std::unordered_map<DetailSettingType, IDetailSettingsItem*>& detailSettingsMap,
                                                          std::list<AppManagerItem>& appManagerList,
                                                          QWidget* parent)
{
    return new Ukui4MainWidget(titleLabelMap, switchBtnMap, volumeSliderMap, selectComboxMap, detailSettingsMap, appManagerList, parent);
}

IAudioMainWindow* Ukui5ConcreteStrategy::createMainWindow(std::unordered_map<TitleLabelType, TitleLabelItem*>& titleLabelMap,
                                                          std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& switchBtnMap,
                                                          std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& volumeSliderMap,
                                                          std::unordered_map<SelectComboxType, ISelectComboxItem*>& selectComboxMap,
                                                          std::unordered_map<DetailSettingType, IDetailSettingsItem*>& detailSettingsMap,
                                                          std::list<AppManagerItem>& appManagerList,
                                                          QWidget* parent)
{
    return new Ukui5MainWidget(titleLabelMap, switchBtnMap, volumeSliderMap, selectComboxMap, detailSettingsMap, appManagerList, parent);
}
