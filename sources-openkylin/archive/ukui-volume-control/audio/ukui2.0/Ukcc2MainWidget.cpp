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

#include "Ukcc2MainWidget.h"

#include <QVBoxLayout>
#include <kslider.h>

#include "include/SwitchButton.h"

using namespace UkuiAudioFramwork;

Ukcc2MainWidget::Ukcc2MainWidget(std::unordered_map<TitleLabelType, TitleLabelItem*>& titleLabelMap,
                                 std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& switchBtnMap,
                                 std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& volumeSliderMap,
                                 std::unordered_map<SelectComboxType, ISelectComboxItem*>& selectComboxMap,
                                 std::unordered_map<DetailSettingType, IDetailSettingsItem*>& detailItemMap,
                                 std::list<AppManagerItem>& appManagerList,
                                 QWidget* parent)
    : m_titleLabelItemMap(titleLabelMap),
      m_switchBtnItemMap(switchBtnMap),
      m_volumeSliderItemMap(volumeSliderMap),
      m_selectComboxItemMap(selectComboxMap),
      m_detailSettingsItemMap(detailItemMap),
      IAudioMainWindow(appManagerList),
      QWidget(parent)
{
    createdItems();
    initUi();
}

void Ukcc2MainWidget::initUi()
{
    // 添加布局
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_OUTPUT]->getWidget());
    vLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_OUTPUT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_OUTPUT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST]->getWidget());
    vLayout->addItem(new QSpacerItem(16, 16, QSizePolicy::Fixed));
    vLayout->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_MONO]->getWidget());

    vLayout->addItem(new QSpacerItem(16, 40, QSizePolicy::Fixed));
    vLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_INPUT]->getWidget());
    vLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_INPUT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_LOOPBACK]->getWidget());

    vLayout->addItem(new QSpacerItem(16, 40, QSizePolicy::Fixed));
    vLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_SOUNDEFFECT]->getWidget());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_STARTUP]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_POWEROFF]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_LOGOUT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_WAKEUP]->getWidget());

    vLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_ALERT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_SOUNDEFFECT]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_VOLUMEADJUST]->getWidget());
    vLayout->addWidget(addLine());
    vLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_NOTIFICTION]->getWidget());
    vLayout->setSpacing(0);
    setLayout(vLayout);

    vLayout->setContentsMargins(0, 0, 0, 0);
}

void Ukcc2MainWidget::initSlots()
{
    for (const auto& it : m_volumeSliderItemMap) {
        connect(static_cast<QSlider*>(it.second->getSlider()->getWidget()), &QSlider::valueChanged, this, [this](int value) {
            setVolume(1, -1, value);
        });

        if (VolumeSliderType::VOLUME_SLIDER_INPUT != it.first && VolumeSliderType::VOLUME_SLIDER_OUTPUT != it.first)
            continue;

        connect(static_cast<Ukui4VolumeSliderItem*>(it.second)->getMuteBtn(), &QPushButton::clicked, this, [it, this]() {
            setMute(EnumToInt(it.first), -1);
        });
    }
}

void Ukcc2MainWidget::setValue(int type , int value)
{
    m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setValue(value);
}

void Ukcc2MainWidget::setAppValue(const QString&, int)
{
    // TODO
}

void Ukcc2MainWidget::setAppMute(const QString&, bool)
{
    // TODO
}

void Ukcc2MainWidget::setCurrentIndex(int type, int idx)
{
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->setCurrentIndex(idx);
}

void Ukcc2MainWidget::setAppCurrentIndex(const QString&, const NodeType&, int)
{
    // TODO
}

void Ukcc2MainWidget::setAppCbxEnabled(const QString&, const NodeType&, bool)
{
    // TODO
}

void Ukcc2MainWidget::setCurrentAppItem(int idx)
{
    // TODO
}

void Ukcc2MainWidget::setChecked(int type, bool status)
{
    m_switchBtnItemMap[IntToEnum<SwitchButtonType>(type)]->setChecked(status);
}

std::list<DevicePortInfo> Ukcc2MainWidget::getDevicePortList(int) const
{
    // TODO
    return {};
}

std::list<DevicePortInfo> Ukcc2MainWidget::getDeviceManagetPortList(int) const
{
    // TODO
    return {};
}

//std::unordered_map<uint32_t, AppManagerItemWidget*> Ukcc2MainWidget::getAppItemList() const
std::list<AppManagerItem>& Ukcc2MainWidget::getAppItemList()
{
    // TODO
    return m_appWidgetList;
}

std::list<DevicePortInfo> Ukcc2MainWidget::getAppManagetPortList(int, int) const
{
    // TODO
    return {};
}

QWidget* Ukcc2MainWidget::getWidget()
{
    return this;
}

void Ukcc2MainWidget::addDeviceManagerItem(int type, const QString& displayStr, const QVariant& data)
{
    // TODO
}

void Ukcc2MainWidget::removeDeviceManagerItem(int type, int idx)
{
    // TODO
}

void Ukcc2MainWidget::setDeviceManagerItemStatus(int type, int idx, bool checked)
{
    // TODO
}

void Ukcc2MainWidget::updateDeviceManagerStatus(int type, bool hasDevice)
{
    // TODO
}

void Ukcc2MainWidget::addAppManagerItem(int, int, int, bool, const QString&, const QString&, const QList<ClientDeviceInfo>&, const QList<ClientDeviceInfo>&)
{
    // TODO
}

void Ukcc2MainWidget::removeAppManagerItem(const QString&)
{
    // TODO
}

void Ukcc2MainWidget::addPortToAppItem(const QString&, const NodeType&, const QString&, const QVariant&)
{
   // TODO
}

void Ukcc2MainWidget::removePortFromAppItem(const QString&, const NodeType&, int)
{
    // TODO
}

void Ukcc2MainWidget::showAppControlWidget()
{
    // TODO
}

void Ukcc2MainWidget::updateUIByButtonStatus(int, bool)
{

}

void Ukcc2MainWidget::createdItems()
{
    for (const auto& [k, v] : m_titleLabelKeys) {
        m_titleLabelItemMap.emplace(k, new TitleLabelItem(new QLabel(v), this));
    }

    // 创建SwitchButton子项
    for (const auto& [k, v] : m_switchButtonkeys) {
        ISwitchButtonItem* item = nullptr;
        std::shared_ptr<ISwitchButton> btn = nullptr;
        btn = std::make_shared<Ukui2SwitchButton>(new SwitchButton);

        switch (k) {
        case SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST: {
            item = new Ukui2SwitchButtonWithDescItem(btn, new QLabel(v.at(0), this), new QLabel(v.at(1)), k, this);
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_MONO: {
            item = new Ukui2SwitchButtonWithDescItem(btn, new QLabel(v.at(0), this), new QLabel(v.at(1)), k, this);
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_LOOPBACK: {
            item = new Ukui2SwitchButtonWithDescItem(btn, new QLabel(v.at(0), this), new QLabel(v.at(1)), k, this);
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL:
        case SwitchButtonType::SWITCH_BUTTON_STARTUP:
        case SwitchButtonType::SWITCH_BUTTON_POWEROFF:
        case SwitchButtonType::SWITCH_BUTTON_LOGOUT:
        case SwitchButtonType::SWITCH_BUTTON_WAKEUP:
        case SwitchButtonType::SWITCH_BUTTON_ALERT:
        case SwitchButtonType::SWITCH_BUTTON_AUTO_PAUSE: {
            item = new Ukui2SwitchButtonItem(btn, new QLabel(v.at(0), this), k, this);
            break;
        }
        default:
            break;
        }

        if (item && btn) {
            m_switchBtnItemMap.emplace(k, item);
        }
    }

    // 创建VolumeSlider子项
    for (const auto& [k, v] : m_volumeSliderKeys) {

        IVolumeSliderItem* item = nullptr;
        std::shared_ptr<IVolumeSlider> slider = nullptr;

        switch (k) {
        case VolumeSliderType::VOLUME_SLIDER_OUTPUT:
        case VolumeSliderType::VOLUME_SLIDER_INPUT: {
            auto* sliderWidget = new QSlider(this);
            sliderWidget->setAccessibleName(v.at(0));
            slider = std::make_shared<Ukui2VolumeSlider>(sliderWidget);
            slider->setRange(0, 100);
            item = new Ukui2VolumeSliderItem(slider, new QLabel(v.at(0), this), new QLabel(this), new QPushButton(this), this);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_BALANCE: {
            slider = std::make_shared<Ukui4BalanceVolumeSlider>(new kdk::KSlider(this));
            slider->setRange(0, 100);
            item = new Ukui4BalanceVolumeSliderItem(slider, new QLabel(v.at(0), this), new QLabel(v.at(1)), new QLabel(v.at(2)), this);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL: {
            auto* inputLevelBar = new QProgressBar(this);
            inputLevelBar->setAccessibleName(v.at(0));
            slider = std::make_shared<Ukui4InputLevelVolumeSlider>(inputLevelBar);
            slider->setRange(0, 99);
            item = new Ukui4InputLevelVolumeSliderItem(slider, new QLabel(v.at(0), this), this);
            m_pMonitorStream = std::make_shared<MonitorStream>(slider);
            break;
        }
        default:
            break;
        }

        if (item && slider) {
            m_volumeSliderItemMap.emplace(k, item);
        }
    }

    // 创建SelectCombox子项
    for (const auto& [k, v] : m_selectComboxKeys) {

        ISelectComboxItem* item = nullptr;
        std::shared_ptr<ISelectBox> box = nullptr;

        switch (k) {
        case SelectComboxType::SELECT_COMBOX_INPUT:
        case SelectComboxType::SELECT_COMBOX_OUTPUT:
        case SelectComboxType::SELECT_COMBOX_SOUNDEFFECT:
        case SelectComboxType::SELECT_COMBOX_VOLUMEADJUST:
        case SelectComboxType::SELECT_COMBOX_NOTIFICTION:{
            auto* comboBox = new QComboBox(this);
            comboBox->setAccessibleName(v.at(0));
            box = std::make_shared<Ukui2SelectCombox>(comboBox);
            item = new Ukui2SelectComboxItem(box, new QLabel(v.at(0), this), this);
            break;
        }
        default:
            break;
        }

        if (item && box) {
            m_selectComboxItemMap.emplace(k, item);
        }
    }
}

QFrame* Ukcc2MainWidget::addLine()
{
    auto line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}
