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

#include "Ukui5MainWidget.h"

#include "ukcc/interface/ukcccommon.h"

#include <QVBoxLayout>
#include <kslider.h>
#include <QDebug>
#include <QPushButton>

#include "Ukcc5DevicePortItem.h"
#include "PaDataType.h"
#include "ukui4.0/Ukui4SwitchButtonItem.h"
#include "ukui4.0/Ukui4VolumeSliderItem.h"
#include "ukui4.0/Ukui4SelectComboxItem.h"
#include "Ukui4CustomControl.h"
#include "Ukui5DetailSettingsItem.h"

Ukui5MainWidget::Ukui5MainWidget(std::unordered_map<TitleLabelType, TitleLabelItem*>& titleLabelMap,
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

void Ukui5MainWidget::initUi()
{
    // 添加布局
    auto vMainLayout = new QVBoxLayout;
    auto outputFrame1 = new QFrame();
    auto outputVLayout1 = new QVBoxLayout();
    outputVLayout1->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_OUTPUT]->getWidget());
    outputVLayout1->addWidget(addLine());
    outputVLayout1->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_OUTPUT]->getWidget());
    outputVLayout1->addWidget(addLine());
    outputVLayout1->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST]->getWidget());
    outputVLayout1->setSpacing(0);
    outputFrame1->setLayout(outputVLayout1);
    outputVLayout1->setContentsMargins(0, 0, 0, 0);
    outputFrame1->setFrameShape(QFrame::Shape::Box);

    auto outputFrame2 = new QFrame();
    auto outputVLayout2 = new QVBoxLayout();
    outputVLayout2->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->getWidget());
    outputVLayout2->addWidget(addLine());
    outputVLayout2->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_MONO]->getWidget());
    outputVLayout2->setSpacing(0);
    outputFrame2->setLayout(outputVLayout2);
    outputVLayout2->setContentsMargins(0, 0, 0, 0);
    outputFrame2->setFrameShape(QFrame::Shape::Box);

    auto inputFrame1 = new QFrame();
    auto inputVLayout1 = new QVBoxLayout();
    inputVLayout1->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_INPUT]->getWidget());
    inputVLayout1->addWidget(addLine());
    inputVLayout1->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUT]->getWidget());
    inputVLayout1->addWidget(addLine());
    inputVLayout1->addWidget(m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL]->getWidget());
    inputVLayout1->addWidget(addLine());
    inputVLayout1->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL]->getWidget());

    auto loopbackFrame = new QFrame(inputFrame1);
    auto loopbackVLayout = new QVBoxLayout(inputFrame1);

    loopbackVLayout->addWidget(addLine());
    loopbackVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_LOOPBACK]->getWidget());
    loopbackVLayout->setSpacing(0);
    loopbackVLayout->setContentsMargins(0, 0, 0, 0);
    loopbackFrame->setLayout(loopbackVLayout);

    inputVLayout1->addWidget(loopbackFrame);
    inputVLayout1->setSpacing(0);
    inputFrame1->setLayout(inputVLayout1);
    inputVLayout1->setContentsMargins(0, 0, 0, 0);
    inputFrame1->setFrameShape(QFrame::Shape::Box);

    auto soundEffectFrame = new QFrame();
    auto soundEffectVLayout = new QVBoxLayout();
    soundEffectVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_STARTUP]->getWidget());
    soundEffectVLayout->addWidget(addLine());
    soundEffectVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_POWEROFF]->getWidget());
    soundEffectVLayout->addWidget(addLine());
    soundEffectVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_LOGOUT]->getWidget());
    soundEffectVLayout->addWidget(addLine());
    soundEffectVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_WAKEUP]->getWidget());
    soundEffectFrame->setLayout(soundEffectVLayout);
    soundEffectVLayout->setSpacing(0);
    soundEffectVLayout->setContentsMargins(0, 0, 0, 0);
    soundEffectFrame->setFrameShape(QFrame::Shape::Box);

    auto soundThemeFrame = new QFrame();
    auto soundThemeVLayout = new QVBoxLayout();
    soundThemeVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_ALERT]->getWidget());
    soundThemeVLayout->addWidget(addLine());
    soundThemeVLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_SOUNDEFFECT]->getWidget());
    soundThemeVLayout->addWidget(addLine());
    soundThemeVLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_VOLUMEADJUST]->getWidget());
    soundThemeVLayout->addWidget(addLine());
    soundThemeVLayout->addWidget(m_selectComboxItemMap[SelectComboxType::SELECT_COMBOX_NOTIFICTION]->getWidget());
    soundThemeFrame->setLayout(soundThemeVLayout);
    soundThemeVLayout->setSpacing(0);
    soundThemeVLayout->setContentsMargins(0, 0, 0, 0);
    soundThemeFrame->setFrameShape(QFrame::Shape::Box);

    auto autoPauseFrame = new QFrame();
    auto autoPauseVLayout = new QVBoxLayout();
    autoPauseVLayout->addWidget(m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_AUTO_PAUSE]->getWidget());
    autoPauseFrame->setLayout(autoPauseVLayout);
    autoPauseVLayout->setSpacing(0);
    autoPauseVLayout->setContentsMargins(0, 0, 0, 0);
    autoPauseFrame->setFrameShape(QFrame::Shape::Box);

    auto advanceSettingsFrame = new QFrame();
    auto vLayout5 = new QVBoxLayout();
    vLayout5->addWidget(m_detailSettingsItemMap[DetailSettingType::DETAIL_SETTING_DEVMANAGER]->getWidget());
    vLayout5->addWidget(addLine());
    vLayout5->addWidget(m_detailSettingsItemMap[DetailSettingType::DETAIL_SETTING_APPMANAGER]->getWidget());
    advanceSettingsFrame->setLayout(vLayout5);
    vLayout5->setSpacing(0);
    vLayout5->setContentsMargins(0, 0, 0, 0);
    advanceSettingsFrame->setFrameShape(QFrame::Shape::Box);

    vMainLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_OUTPUT]->getWidget());
    vMainLayout->addWidget(outputFrame1);
    vMainLayout->addSpacing(8);
    vMainLayout->addWidget(outputFrame2);
    vMainLayout->addSpacing(30);
    vMainLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_INPUT]->getWidget());
    vMainLayout->addWidget(inputFrame1);
    vMainLayout->addSpacing(30);
    vMainLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_SOUNDEFFECT]->getWidget());
    vMainLayout->addWidget(soundEffectFrame);
    vMainLayout->addSpacing(8);
    vMainLayout->addWidget(soundThemeFrame);
    vMainLayout->addSpacing(30);
    vMainLayout->addWidget(m_titleLabelItemMap[TitleLabelType::TITLE_LABEL_ADVANCESETTINGS]->getWidget());
    vMainLayout->addWidget(autoPauseFrame);
    vMainLayout->addSpacing(8);
    vMainLayout->addWidget(advanceSettingsFrame);
    vMainLayout->setSpacing(0);
    vMainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(vMainLayout);

    // 将所有子frame添加到m_frameVector
    m_FrameVector.push_back(outputFrame2);
    m_FrameVector.push_back(loopbackFrame);
    m_FrameVector.push_back(soundThemeFrame);
}

void Ukui5MainWidget::initSlots()
{
    for (const auto& it : m_volumeSliderItemMap) {

        if (VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL == it.first)
            continue;

        auto slider = static_cast<QSlider*>(it.second->getSlider()->getWidget());
        if (slider) {
            connect(slider, &QSlider::valueChanged, this, [it, this](int value) {
                if (it.first == VolumeSliderType::VOLUME_SLIDER_BALANCE) {
                    double v = value / 100.0;
                    setBalance(EnumToInt(it.first), v);
                } else {
                    it.second->setValue(value);
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
            auto listWidget = static_cast<QListWidget*>(it.second->getSelectBox()->getWidget());
            if (listWidget) {
                connect(listWidget, &QListWidget::currentRowChanged, this, [listWidget, it, this](int row) {
                    auto item = listWidget->item(row);
                    if (!item) {
                        return;
                    }
                    auto wid = (Ukcc5DevicePortItem*) listWidget->itemWidget(item);
                    if (!wid) {
                        return;
                    }
                    setDefaultDevice(EnumToInt(it.first), -1, wid->getPortName(), wid->getCardName());
                    qDebug() << QString("Set type %1 combox to card: %2 port: %3.").arg(EnumToInt(it.first))
                                .arg(wid->getPortName()).arg(wid->getCardName());
                });
            }
            break;
        }
        case SelectComboxType::SELECT_COMBOX_SOUNDEFFECT: {
            auto box = static_cast<QComboBox*>(it.second->getSelectBox()->getWidget());
            if (box) {
                connect(box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [box, it, this](int row) {
                    // 如果是切换到自定义则只修改自定义主题的状态,切换到其他音效主题时才更新音效主题
                    if ("custom" == box->currentData()) {
                        setSettingsValue("setCustomThemeStatus", true);
                    }
                    else {
                        setSettingsValue("setCustomThemeStatus", false);
                    }
                    setSettingsValue(m_comboxMethodKeys[it.first], box->currentData());
                    ukcc::UkccCommon::buriedSettings("Audio", box->objectName(), "select", box->currentText());
                });
            }

            break;
        }
        case SelectComboxType::SELECT_COMBOX_VOLUMEADJUST: {
            auto box = static_cast<QComboBox*>(it.second->getSelectBox()->getWidget());
            if (box) {
                connect(box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [box, it, this](int row) {
                    setSettingsValue(m_comboxMethodKeys[it.first], box->currentData());
                    ukcc::UkccCommon::buriedSettings("Audio", box->objectName(), "select", box->currentText());
                });
            }

            break;
        }
        case SelectComboxType::SELECT_COMBOX_NOTIFICTION: {
            auto box = static_cast<QComboBox*>(it.second->getSelectBox()->getWidget());
            if (box) {
                connect(box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [box, it, this](int row) {
                    setSettingsValue(m_comboxMethodKeys[it.first], box->currentData());
                    ukcc::UkccCommon::buriedSettings("Audio", box->objectName(), "select", box->currentText());
                });
            }

            break;
        }
        default:
            break;
        }
    }

    for (const auto& it : m_switchBtnItemMap) {
        auto switchBtn = static_cast<QPushButton*>(it.second->getSwitchButton()->getWidget());
        if (switchBtn) {
            connect(switchBtn, &QPushButton::clicked, this, [switchBtn, it, this]() {
                setSettingsValue(m_switchMethodKeys[it.first], QVariant(switchBtn->isChecked()));
                ukcc::UkccCommon::buriedSettings("Audio", switchBtn->objectName(), QString("settings"),
                                                 switchBtn->isChecked() ? "true" : "false");
                if ((it.first == SwitchButtonType::SWITCH_BUTTON_ALERT)) {
                    qDebug() << QString("setAlertStatus %1 .").arg(switchBtn->isChecked());
                    setFrameVisible(EnumToInt(FrameType::SOUND_THEME_FRAME), switchBtn->isChecked());
                }
            });
        }
    }

    for (const auto& it : m_detailSettingsItemMap) {
        auto switchBtn = static_cast<QPushButton*>(it.second->getDetailSettings()->getWidget());
        if (switchBtn) {
            connect(switchBtn, &QPushButton::clicked, this, [it, this]() {
                if (DetailSettingType::DETAIL_SETTING_DEVMANAGER == it.first) {

                    if (!m_pDevManagerWidget) {
                        m_pDevManagerWidget = new DeviceManagerMainWidget(this);

                        setDetailSettings(EnumToInt(it.first));
                        connect(m_pDevManagerWidget, &DeviceManagerMainWidget::enableDeviceSignals, this, [it, this](const QString& card, const QString& port, bool enable) {
                            qDebug() << QString("enableDeviceSignals, card: %1, port: %2, enable: %3").arg(card).arg(port).arg(enable);
                            IAudioMainWindow::setEnabled(card, port, enable);
                        });

                        connect(m_pDevManagerWidget, &DeviceManagerMainWidget::closeWindowSignals, this, [this]() {
                            auto window = m_pDevManagerWidget->window();
                            if (window) {
                                window->close();
                                m_pDevManagerWidget = nullptr;
                            }
                            qDebug() << QString("Close device Manager Widget.");
                        });
                    }
                    m_pDevManagerWidget->show();
                }
                else {
                    if (!m_pAppManagerWidget) {
                        m_pAppManagerWidget = new AppManagerMainWidget(m_appWidgetList, this);
                        setDetailSettings(EnumToInt(it.first));

                        connect(m_pAppManagerWidget, &AppManagerMainWidget::valueChangedSignals, this, [it, this](int idx, int value) {
                            IAudioMainWindow::setVolume(2, idx, value);
                        });

                        connect(m_pAppManagerWidget, &AppManagerMainWidget::muteChangedSignals, this, [it, this](int idx) {
                            IAudioMainWindow::setMute(2, idx);
                        });

                        connect(m_pAppManagerWidget, &AppManagerMainWidget::currentIndexChangedSignals,
                                this, [this] (const NodeType& type, int idx, const QString& card, const QString& port) {
                            if (NodeType::OUTPUT == type)
                                IAudioMainWindow::setDefaultDevice(2, idx, port, card);
                            else
                                IAudioMainWindow::setDefaultDevice(3, idx, port, card);
                        });

                        connect(m_pAppManagerWidget, &AppManagerMainWidget::closeWindowSignals, this, [this]() {
                            auto window = m_pAppManagerWidget->window();
                            if (window) {
                                window->close();
                                m_pAppManagerWidget->deleteLater();
                                m_appWidgetList.clear();
                                m_pAppManagerWidget = nullptr;
                            }
                            qDebug() << QString("Close application manager Widget.");
                        });

                        connect(m_pAppManagerWidget, &AppManagerMainWidget::hideWindowSignals, this, [this]() {
                            auto window = m_pAppManagerWidget->window();
                            if (window) {
                                window->close();
                                m_pAppManagerWidget->deleteLater();
                                m_appWidgetList.clear();
                                m_pAppManagerWidget = nullptr;
                    }
                            qDebug() << QString("Hide application manager Widget.");
                        });
                    }
                    m_pAppManagerWidget->show();
                }
            });
        }
    }
}

void Ukui5MainWidget::setValue(int type, int value)
{
    m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setValue(value);
}

void Ukui5MainWidget::setAppValue(const QString& name, int value)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->setValue(name, value);
}

void Ukui5MainWidget::setAppMute(const QString& name, bool mute)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->setMute(name, mute);
}

void Ukui5MainWidget::setCurrentIndex(int type, int idx)
{
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->setCurrentIndex(idx);
}

void Ukui5MainWidget::setAppCurrentIndex(const QString& name, const NodeType& type, int cbxIndex)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->setCurrentIndex(name, type, cbxIndex);
}

void Ukui5MainWidget::setAppCbxEnabled(const QString& name, const NodeType& type, bool enable)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->setAppCbxEnabled(name, type, enable);
}

void Ukui5MainWidget::setCurrentAppItem(int idx)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->setCurrentItem(idx);
}

void Ukui5MainWidget::setChecked(int type, bool status)
{
    m_switchBtnItemMap[IntToEnum<SwitchButtonType>(type)]->setChecked(status);
}

std::list<DevicePortInfo> Ukui5MainWidget::getDevicePortList(int direction) const
{
    std::list<DevicePortInfo> list;

    auto dataList = m_selectComboxItemMap[IntToEnum<SelectComboxType>(direction)]->getDataList();
    for (const auto& item : dataList) {
          if (item.canConvert<QVariantList>()) {
              QVariantList l = item.toList();
              if (l.size() >= 3) {
                  DevicePortInfo info;
                  info.index = l.at(0).toInt();
                  info.cardName = l.at(1).toString();
                  info.portName = l.at(2).toString();
                  list.push_back(info);
              } else {
                  qDebug() << "Invalid data list size: " << l.size();
              }
          } else {
              qDebug() << "Item is not a QVariantList: " << item;
          }
      }

    return list;
}

std::list<DevicePortInfo> Ukui5MainWidget::getDeviceManagetPortList(int direction) const
{
    std::list<DevicePortInfo> list;

    if (!m_pDevManagerWidget)
        return list;

    auto dataList = m_pDevManagerWidget->getManagerWidgetByType(IntToEnum<DeviceManagerType>(direction))->getDataList();

    for (const auto& item : dataList) {
        if (item.canConvert<QVariantList>()) {
            QVariantList l = item.toList();
            if (l.size() >= 3) {
                DevicePortInfo info;
                info.index = l.at(0).toInt();
                info.cardName = l.at(1).toString();
                info.portName = l.at(2).toString();
                list.push_back(info);
            } else {
                qDebug() << "Invalid device manager data, list size: " << l.size();
            }
        } else {
            qDebug() << "Item is not a QVariantList: " << item;
        }
    }

    return list;
}

std::list<AppManagerItem>& Ukui5MainWidget::getAppItemList()
{
    std::list<AppManagerItem> list {};
    if (m_pAppManagerWidget)
        list = m_pAppManagerWidget->getAppItems();

    return list;
}

std::list<DevicePortInfo> Ukui5MainWidget::getAppManagetPortList(int idx, int direction) const
{
    std::list<DevicePortInfo> list;

    if (!m_pAppManagerWidget)
        return list;

    auto dataList = m_pAppManagerWidget->getAppWidgetByIndex(idx)->getSelectComboxItems()[IntToEnum<NodeType>(direction)]->getDataList();

    for (const auto& item : dataList) {
        if (item.canConvert<QVariantList>()) {
            QVariantList l = item.toList();
            if (l.size() >= 3) {
                DevicePortInfo info;
                info.index = l.at(0).toInt();
                info.cardName = l.at(1).toString();
                info.portName = l.at(2).toString();
                list.push_back(info);
            } else {
                qDebug() << "Invalid app widget data, dataList: " << l;
            }
        } else {
            qDebug() << "Item is not a QVariantList: " << item;
        }
    }

    return list;
}


QWidget* Ukui5MainWidget::getWidget()
{
    return this;
}

void Ukui5MainWidget::addDeviceManagerItem(int type, const QString& displayStr, const QVariant& data)
{
    if (m_pDevManagerWidget) {
        auto list = data.toStringList();
        m_pDevManagerWidget->addItem(IntToEnum<DeviceManagerType>(type), displayStr, list.at(0), list.at(1));
    }
}

void Ukui5MainWidget::removeDeviceManagerItem(int type, int idx)
{
    if (m_pDevManagerWidget) {
        m_pDevManagerWidget->removeItem(IntToEnum<DeviceManagerType>(type), idx);
    }
}

void Ukui5MainWidget::setDeviceManagerItemStatus(int type, int idx, bool checked)
{
    if (m_pDevManagerWidget) {
        m_pDevManagerWidget->setChecked(IntToEnum<DeviceManagerType>(type), idx, checked);
    }
}

void Ukui5MainWidget::updateDeviceManagerStatus(int type, bool hasDevice)
{
    if (m_pDevManagerWidget) {
        m_pDevManagerWidget->updateDeviceStatus(IntToEnum<DeviceManagerType>(type), hasDevice);
    }
}

void Ukui5MainWidget::updateUIByButtonStatus(int type, bool status)
{
    if (IntToEnum<SwitchButtonType>(type) == SwitchButtonType::SWITCH_BUTTON_ALERT) {
        qDebug() << QString("setChecked type: %1 , status: %2 .").arg(type).arg(status);
        setFrameVisible(EnumToInt(FrameType::SOUND_THEME_FRAME), status);
    }
}

/* 1. When default output is bluetooth device, need to hide balance mono frame
   2. When defualt input is NULL or Internal Mic, need to hide loopback frame */
void Ukui5MainWidget::updateUIByDevice(int type, bool status)
{
    if (IntToEnum<NodeType>(type) == NodeType::OUTPUT) {
        qDebug() << "Set Balance and Mono frame visible status:" << status;
        setFrameVisible(EnumToInt(FrameType::BALANCE_MONO_FRAME), status);
    }
    else if (IntToEnum<NodeType>(type) == NodeType::INPUT) {
        qDebug() << "Set Loopback frame visible status:" << status;
        setFrameVisible(EnumToInt(FrameType::LOOP_BACK_FRAME), status);
    }
}

void Ukui5MainWidget::addAppManagerItem(int dire, int idx, int value, bool mute, const QString& icon, const QString& desc, const QList<ClientDeviceInfo>& outputList, const QList<ClientDeviceInfo>& inputList)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->addItem(dire, idx, value, mute, icon, desc, outputList, inputList);
}

void Ukui5MainWidget::removeAppManagerItem(const QString& name)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->removeItem(name);
}

void Ukui5MainWidget::addPortToAppItem(const QString& name, const NodeType& type, const QString& text, const QVariant& data)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->addPortToAppItem(name, type, text, data);
}

void Ukui5MainWidget::removePortFromAppItem(const QString& name, const NodeType& type, int cbxIdx)
{
    if (m_pAppManagerWidget)
        m_pAppManagerWidget->removeProtFromAppItem(name, type, cbxIdx);
}

void Ukui5MainWidget::createdItems()
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
        case SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST:
        case SwitchButtonType::SWITCH_BUTTON_MONO:
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
            slider = std::make_shared<Ukui4VolumeSlider>(new UkmediaVolumeSlider(this), v.at(v.size() - 1));
            slider->setRange(0, 100);
            item = new Ukui4VolumeSliderItem(slider, new QLabel(v.at(0), this), new QLabel(this), new QPushButton(this), k, this);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_BALANCE: {
            slider = std::make_shared<Ukui4BalanceVolumeSlider>(new UkBalanceVolumeSlider(this), v.at(v.size() - 1));
            item = new Ukui4BalanceVolumeSliderItem(slider, new QLabel(v.at(0), this), new QLabel(v.at(1)), new QLabel(v.at(2)), this);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL: {
            auto* inputLevelBar = new QProgressBar(this);
            inputLevelBar->setAccessibleName(v.at(0));
            slider = std::make_shared<Ukui4InputLevelVolumeSlider>(inputLevelBar);
            slider->setRange(0, 101);
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
        case SelectComboxType::SELECT_COMBOX_OUTPUT: {
            qDebug() << "m_selectComboxKeys.size: " << m_selectComboxKeys.size() << " k:" << EnumToInt(k) << " v:" << v;
            auto* listWidget = new QListWidget(this);
            listWidget->setAccessibleName(v.at(0));
            box = std::make_shared<Ukui5SelectCombox>(listWidget, v.at(v.size() - 1));
            item = new Ukui5SelectComboxItem(box, new QLabel(v.at(0), this), k, this);
            box->setParent(item->getWidget());
            break;
        }
        case SelectComboxType::SELECT_COMBOX_SOUNDEFFECT:
        case SelectComboxType::SELECT_COMBOX_VOLUMEADJUST:
        case SelectComboxType::SELECT_COMBOX_NOTIFICTION: {
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

    for (const auto& [k, v] : m_detailSettingsKeys) {
        IDetailSettingsItem* item = nullptr;
        std::shared_ptr<IDetailSettings> setting = nullptr;

        switch (k) {
        case DetailSettingType::DETAIL_SETTING_DEVMANAGER:
        case DetailSettingType::DETAIL_SETTING_APPMANAGER: {
            auto* detailsBtn = new QPushButton(QObject::tr("Details"));
            detailsBtn->setAccessibleName(v.at(0));
            setting = std::make_shared<Ukui5DetailSettings>(detailsBtn, v.at(v.size() - 1));
            item = new Ukui5DetailSettingsItem(setting, new QLabel(v.at(0)), this);
            break;
        }
        default:
            break;
        }

        if (item && setting) {
            m_detailSettingsItemMap.emplace(k, item);
        }
    }
}

QFrame* Ukui5MainWidget::addLine()
{
    KHLineFrame* line = new KHLineFrame();
//    auto line = new QFrame(this);
//    line->setMinimumSize(QSize(550, 1));
//    line->setMaximumSize(QSize(16777215, 1));
//    line->setFrameShape(QFrame::HLine);
//    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void Ukui5MainWidget::setFrameVisible(int index, bool visible)
{
    if (index >= 0 && index < m_FrameVector.size()) {
        if (index == EnumToInt(FrameType::SOUND_THEME_FRAME)) {
            QLayout* layout = m_FrameVector[index]->layout();
            if (layout) {
                for (int i = 1; i < layout->count(); ++i) {
                    if (layout->itemAt(i) && layout->itemAt(i)->widget()) {
                        if (visible) {
                            layout->itemAt(i)->widget()->show();
                        } else {
                            layout->itemAt(i)->widget()->hide();
                        }
                    }
                }
            }
        } else {
            m_FrameVector[index]->setVisible(visible);
        }
    }
}

/* 响应任务栏跳转控制面板声音二级弹窗应用管理页面 */
void Ukui5MainWidget::showAppControlWidget()
{
    auto it = m_detailSettingsItemMap.find(DetailSettingType::DETAIL_SETTING_APPMANAGER);
    if (it != m_detailSettingsItemMap.end()) {
        IDetailSettingsItem* item = it->second;
        if (item && item->getDetailSettings()) {
            QWidget* widget = item->getDetailSettings()->getWidget();
            if (widget) {
                QPushButton* button = qobject_cast<QPushButton*>(widget);
                if (button) {
                    // 触发按钮的点击信号
                    button->click();
                }
            }
        }
    }
}
