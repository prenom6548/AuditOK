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

#include "Ukui4MainWidget.h"

#include <QVBoxLayout>
#include <kslider.h>
#include <QDebug>

#include "ukui4.0/Ukui4SwitchButtonItem.h"
#include "ukui4.0/Ukui4VolumeSliderItem.h"
#include "ukui4.0/Ukui4SelectComboxItem.h"

Ukui4MainWidget::Ukui4MainWidget(std::unordered_map<TitleLabelType, TitleLabelItem*>& titleLabelMap,
                                 std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& switchBtnMap,
                                 std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& volumeSliderMap,
                                 std::unordered_map<SelectComboxType, ISelectComboxItem*>& selectComboxMap,
                                 std::unordered_map<DetailSettingType, IDetailSettingsItem*>& detailMap,
                                 std::list<AppManagerItem>& appManagerList,
                                 QWidget* parent)
    : m_titleLabelItemMap(titleLabelMap),
      m_switchBtnItemMap(switchBtnMap),
      m_volumeSliderItemMap(volumeSliderMap),
      m_selectComboxItemMap(selectComboxMap),
      m_detailSettingsItemMap(detailMap),
      IAudioMainWindow(appManagerList),
      QWidget(parent)
{
    createdItems();
}

void Ukui4MainWidget::initUi()
{
    // 添加布局
    auto vLayout = new QVBoxLayout;
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

void Ukui4MainWidget::initSlots()
{
    for (const auto& it : m_volumeSliderItemMap) {

        if (VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL == it.first)
            continue;

        auto slider = static_cast<QSlider*>(it.second->getSlider()->getWidget());
        if (slider) {
            connect(slider, &QSlider::valueChanged, this, [it, this](int value) {
                if (VolumeSliderType::VOLUME_SLIDER_BALANCE == it.first) {
                    double v = value / 100.0;
                    setBalance(EnumToInt(it.first), v);
                }
                else {
                    setVolume(EnumToInt(it.first), -1, value);
                }
            });
        }

        auto muteBtn = it.second->getMuteBtn();
        if (muteBtn) {
            connect(muteBtn, &QPushButton::clicked, this, [it, this]() {
                setMute(EnumToInt(it.first), -1);
            });
        }
    }

    for (const auto& it : m_selectComboxItemMap) {

        switch (it.first) {
        case SelectComboxType::SELECT_COMBOX_INPUT:
        case SelectComboxType::SELECT_COMBOX_OUTPUT: {
            QComboBox* combox =  static_cast<QComboBox*>(it.second->getSelectBox()->getWidget());
            connect(combox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [combox, it, this](int row) {
                setDefaultDevice(EnumToInt(it.first), -1, combox->currentData().toStringList().at(0), combox->currentData().toStringList().at(1));
                qDebug() << "set default port: " << combox->currentData().toStringList().at(0) << " card:" << combox->currentData().toStringList().at(1);
            });
            break;
        }
        case SelectComboxType::SELECT_COMBOX_NOTIFICTION:

            break;
        case SelectComboxType::SELECT_COMBOX_SOUNDEFFECT:

            break;
        case SelectComboxType::SELECT_COMBOX_VOLUMEADJUST:
            break;
        }
    }

    for (const auto& it : m_switchBtnItemMap) {
        connect(static_cast<QPushButton*>(it.second->getSwitchButton()->getWidget()), &QPushButton::clicked, this, [it, this](int row) {
            setSettingsValue(m_switchMethodKeys[it.first], QVariant(static_cast<QPushButton*>(it.second->getSwitchButton()->getWidget())->isChecked()));
        });
    }
}

void Ukui4MainWidget::setValue(int type , int value)
{
    m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setValue(value);
}

void Ukui4MainWidget::setAppValue(const QString&, int)
{
    // TODO
}

void Ukui4MainWidget::setAppMute(const QString&, bool)
{
    // TODO
}

void Ukui4MainWidget::setCurrentIndex(int type, int idx)
{
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->setCurrentIndex(idx);
}

void Ukui4MainWidget::setAppCurrentIndex(const QString&, const NodeType&, int)
{
    // TODO
}

void Ukui4MainWidget::setAppCbxEnabled(const QString&, const NodeType&, bool)
{
    // TODO
}

void Ukui4MainWidget::setCurrentAppItem(int)
{
    // TODO
}

void Ukui4MainWidget::setChecked(int type, bool status)
{
    m_switchBtnItemMap[IntToEnum<SwitchButtonType>(type)]->setChecked(status);
}

std::list<DevicePortInfo> Ukui4MainWidget::getDevicePortList(int) const
{
    // TODO

    return {};
}

std::list<DevicePortInfo> Ukui4MainWidget::getDeviceManagetPortList(int) const
{
    // TODO

    return {};
}

//std::unordered_map<uint32_t, AppManagerItemWidget*> Ukui4MainWidget::getAppItemList() const
std::list<AppManagerItem>& Ukui4MainWidget::getAppItemList()
{
    // TODO

    return m_appWidgetList;
}

std::list<DevicePortInfo> Ukui4MainWidget::getAppManagetPortList(int, int) const
{
    // TODO
    return {};
}

QWidget* Ukui4MainWidget::getWidget()
{
    return this;
}

void Ukui4MainWidget::addDeviceManagerItem(int type, const QString& displayStr, const QVariant& data)
{
    // TODO
}

void Ukui4MainWidget::removeDeviceManagerItem(int type, int idx)
{
    // TODO
}

void Ukui4MainWidget::setDeviceManagerItemStatus(int type, int idx, bool checked)
{
    // TODO
}

void Ukui4MainWidget::updateDeviceManagerStatus(int type, bool hasDevice)
{
    // TODO
}

void Ukui4MainWidget::addAppManagerItem(int, int, int, bool, const QString&, const QString&, const QList<ClientDeviceInfo>&, const QList<ClientDeviceInfo>&)
{
    // TODO
}

void Ukui4MainWidget::removeAppManagerItem(const QString&)
{
    // TODO
}

void Ukui4MainWidget::addPortToAppItem(const QString&, const NodeType&, const QString&, const QVariant&)
{
    // TODO
}

void Ukui4MainWidget::removePortFromAppItem(const QString&, const NodeType&, int)
{
    // TODO
}

void Ukui4MainWidget::showAppControlWidget()
{
    // TODO
}

void Ukui4MainWidget::updateUIByButtonStatus(int, bool)
{

}

void Ukui4MainWidget::updateUIByDevice(int type, bool status)
{

}

void Ukui4MainWidget::createdItems()
{
    for (const auto& [k, v] : m_titleLabelKeys) {
        m_titleLabelItemMap.emplace(k, new TitleLabelItem(new QLabel(v), this));
    }

    // 创建SwitchButton子项
    for (const auto& [k, v] : m_switchButtonkeys) {
        ISwitchButtonItem* item = nullptr;
        std::shared_ptr<ISwitchButton> btn = nullptr;
        btn = std::make_shared<Ukui4SwitchButton>(new kdk::KSwitchButton, v.at(v.size() - 1));

        switch (k) {
        case SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST: {
            item = new Ukui4SwitchButtonWithDescItem(btn, new QLabel(v.at(0), this), new QLabel(v.at(1)), k, this);
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_MONO: {
            item = new Ukui4SwitchButtonWithDescItem(btn, new QLabel(v.at(0), this), new QLabel(v.at(1)), k, this);
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_LOOPBACK: {
            item = new Ukui4SwitchButtonWithDescItem(btn, new QLabel(v.at(0), this), new QLabel(v.at(1)), k, this);
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL:
        case SwitchButtonType::SWITCH_BUTTON_STARTUP:
        case SwitchButtonType::SWITCH_BUTTON_POWEROFF:
        case SwitchButtonType::SWITCH_BUTTON_LOGOUT:
        case SwitchButtonType::SWITCH_BUTTON_WAKEUP:
        case SwitchButtonType::SWITCH_BUTTON_ALERT:
        case SwitchButtonType::SWITCH_BUTTON_AUTO_PAUSE: {
            item = new Ukui4SwitchButtonItem(btn, new QLabel(v.at(0), this), k, this);
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
            slider = std::make_shared<Ukui4VolumeSlider>(sliderWidget, v.at(v.size() - 1));
            slider->setRange(0, 100);
            item = new Ukui4VolumeSliderItem(slider, new QLabel(v.at(0), this), new QLabel(this), new QPushButton(this), k, this);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_BALANCE: {
            slider = std::make_shared<Ukui4BalanceVolumeSlider>(new kdk::KSlider(this), v.at(v.size() - 1));
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
            qDebug() << "m_selectComboxKeys.size: " << m_selectComboxKeys.size() << " k:" << EnumToInt(k) << " v:" << v;
            auto* comboBox = new QComboBox(this);
            comboBox->setAccessibleName(v.at(0));
            box = std::make_shared<Ukui4SelectCombox>(comboBox, v.at(v.size() - 1));
            item = new Ukui4SelectComboxItem(box, new QLabel(v.at(0), this), k, this);
            break;
        }
        default:
            break;
        }

        if (item && box) {
            qDebug() << "emplace, k:" << EnumToInt(k) << "m_selectComboxItemMap。size: " << m_selectComboxItemMap.size();
            m_selectComboxItemMap.emplace(k, item);
        }
    }
}

QFrame* Ukui4MainWidget::addLine()
{
    auto line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}
