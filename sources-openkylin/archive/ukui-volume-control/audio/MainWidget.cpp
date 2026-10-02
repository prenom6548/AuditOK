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

#include "MainWidget.h"

#include "ukcc/interface/ukcccommon.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScreen>

#include <algorithm>

#include "ukui2.0/Ukcc2MainWidget.h"
#include "Ukui4MainWidget.h"
#include "Ukui5MainWidget.h"
#include "ClientMethod.h"
#include "ConcreteStrategy.h"
#include "DeviceManagerMainWidget.h"

#define SOUND_FILE_STARTUP_PATH "/usr/share/ukui-media/sounds/startup.ogg"
#define SOUND_FILE_SHUTDOWN_PATH "/usr/share/ukui-media/sounds/shutdown.ogg"
#define SOUND_FILE_LOGOUT_PATH "/usr/share/ukui-media/sounds/logout.ogg"
#define SOUND_FILE_WEAKUP_PATH "/usr/share/ukui-media/sounds/weakup.ogg"

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
{
    getDesktopEnvironmentVersion();

    initUi();

    initSlots();

    initClientManager();

    initData();
}

void MainWidget::initUi()
{
    switch (m_version) {
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2:
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI3:
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI4:
    case DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI5:
    default:
        m_pStrategy = std::make_shared<Ukui5ConcreteStrategy>();
        break;
    }

    if (m_pStrategy) {
        m_pMainWid = m_pStrategy->createMainWindow(m_titleLabelItemMap, m_switchBtnItemMap, m_volumeSliderItemMap, m_selectComboxItemMap,
                                                   m_detailSettingsItemMap, m_appWidgetList, this);
        m_pMainWid->initUi();
        m_pMainWid->initSlots();
    }
    else {
        QMessageBox::critical(this, tr("Audio"),
                              tr("Unable to obtain system version information, please check the system version!"),
                              QMessageBox::Ok);
        return;
    }

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_pMainWid->getWidget());
    setLayout(hLayout);
    hLayout->setContentsMargins(0, 0, 0, 0);
}

void MainWidget::initSystemDeviceList(const NodeType& type)
{
    auto deviceList = getAvailablePortList(type);
    auto list = m_pMainWid->getDevicePortList(EnumToInt(type));

    int direction = (type == NodeType::OUTPUT) ? PA_DIRECTION_OUTPUT : PA_DIRECTION_INPUT;
    auto dev = std::find_if(deviceList.begin(), deviceList.end(), [direction](const ClientDeviceInfo& info) {
        return direction == info.direction && info.enabled;
    });

    if (0 == deviceList.size() || dev == deviceList.end()) {
        ClientMethod::getInstance().clear(EnumToInt(type));
        ClientMethod::getInstance().addItem(EnumToInt(type), tr("None"), QVariant(std::initializer_list<QString> {"none", "none"}));

        // 31268 系统未识别到声卡输出设备时，音量调节置灰不可调
        ClientMethod::getInstance().setValue(EnumToInt(type), 0);
        ClientMethod::getInstance().setEnabled(EnumToInt(type), false);
        ClientMethod::getInstance().setCurrentIndex(EnumToInt(type), 0);

        if (NodeType::INPUT == type)
            m_pMainWid->updateUIByDevice(EnumToInt(NodeType::INPUT), false);

        return;
    }

    // 添加组合音频输出设备
    bool isExitMultiAudioDevice = false;
    if (NodeType::OUTPUT == type) {
        auto status = getMultiAudioCombineStatus(type);
        if (status) {
            auto sinkList = getSinkList();
            auto it = std::find_if(sinkList.begin(), sinkList.end(), [](const ClientDefaultDeviceInfo& info) {
                return info.name.contains("combine");
            });

            if (it != sinkList.end()) {
                isExitMultiAudioDevice = true;
                qDebug() << "Add Multiple Bluetooth outputs, (*it).activePortName " << (*it).activePortName <<
                          "(*it).cardName" << (*it).cardName;
                auto at = std::find_if(list.begin(), list.end(), [](const DevicePortInfo& info) {
                    return info.cardName == "combine-device";
                });
                if (at == list.end()) {
                    ClientMethod::getInstance().addItem(EnumToInt(NodeType::OUTPUT), tr("Multi Bluetooth Output"),
                                                        QVariant(std::initializer_list<QString> {(*it).cardName, (*it).activePortName}));
                }
            }
        }
    }

    // 如果有可用的设备端口，查找界面中是否存在无端口，存在则删除
    auto it = std::find_if(list.begin(), list.end(), [](const DevicePortInfo& info) {
        return "none" == info.cardName && "none" == info.portName;
    });
    if (it != list.end()) {
        // 31268 系统未识别到声卡输出设备时，音量调节置灰不可调
        ClientMethod::getInstance().setEnabled(EnumToInt(type), true);
        ClientMethod::getInstance().removeItem(EnumToInt(type), (*it).index);
    }

    for (const auto& dev : deviceList) {
        // 禁用的设备不显示在设备列表里面
        if (!dev.enabled)
            continue;

        auto it = std::find_if(list.begin(), list.end(), [dev](const DevicePortInfo& info) {
            return dev.cardName == info.cardName && dev.portName == info.portName;
        });

        // 没找到则添加
        if (it == list.end())
            ClientMethod::getInstance().addItem(EnumToInt(type), QString(dev.portLabel + " (" + dev.cardDesc + ")"),
                                                QVariant(std::initializer_list<QString> {dev.cardName, dev.portName}));
    }

    // 更新list
    list = m_pMainWid->getDevicePortList(EnumToInt(type));

    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        auto deviceIt = std::find_if(deviceList.begin(), deviceList.end(), [it, isExitMultiAudioDevice](const ClientDeviceInfo& info) {
            if (isExitMultiAudioDevice && "combine-device" == (*it).cardName)
                return true;
            else
                return info.enabled && (*it).cardName == info.cardName && (*it).portName == info.portName;
        });

        // 没找到或者设备被禁用则移除
        if (deviceIt == deviceList.end()) {
            qDebug() << QString("can not found device, remove card %1 port %2").arg((*it).cardName).arg((*it).portName);
            ClientMethod::getInstance().removeItem(EnumToInt(type), (*it).index);
        }
    }

    initCurrentDefaultDevice(type);
}

void MainWidget::initCurrentDefaultDevice(const NodeType& type)
{
    auto devInfo = getDefaultDevice(EnumToInt(type), -1);

    auto idx = ClientMethod::getInstance().findData(EnumToInt(type), QVariant(std::initializer_list<QString> {devInfo.cardName, devInfo.activePortName}));

    if (idx == -1) {
        qDebug() << QString("Can not found card: %1 port: %2 in %3 type combox.").arg(devInfo.cardName).arg(devInfo.activePortName).arg(EnumToInt(type));
    }

    qDebug() << QString("Set current default device, find card: %1 port: %2 on index: %3").arg(devInfo.cardName).arg(devInfo.activePortName).arg(idx);
    ClientMethod::getInstance().setCurrentIndex(EnumToInt(type), idx);

    if (NodeType::INPUT == type) {
        if (devInfo.name.contains(".monitor"))
            m_pMainWid->stopMonitor();
        else
            m_pMainWid->startMonitor(devInfo.idx);
    }

    initSystemVolumeSlider();

    // When default output is bluetooth or multi-bluetooth combine device, need to hide balance and mono frame
    if (NodeType::OUTPUT == type && (devInfo.cardName.contains("blue") || devInfo.cardName == "combine-device")) {
        m_pMainWid->updateUIByDevice(EnumToInt(type), false);
    }
    else if (NodeType::INPUT == type) {
        updateLoopbackFrameVisible(idx, devInfo);
    }
}

void MainWidget::initDeviceManagerList(const NodeType& type)
{
    auto deviceList = getAvailablePortList(type);
    auto list = m_pMainWid->getDeviceManagetPortList(EnumToInt(type));

    if (deviceList.size() <= 0) {
        m_pMainWid->updateDeviceManagerStatus(EnumToInt(type), false);
    } else {
        m_pMainWid->updateDeviceManagerStatus(EnumToInt(type), true);
    }

    for (const auto& dev : deviceList) {

        auto it = std::find_if(list.begin(), list.end(), [dev](const DevicePortInfo info) {
            return dev.cardName == info.cardName && dev.portName == info.portName;
        });
        // 没找到则添加
        if (it == list.end()) {
            m_pMainWid->addDeviceManagerItem(EnumToInt(type), QString(dev.portLabel + " (" + dev.cardDesc + ")"),
                                             QVariant(std::initializer_list<QString> {dev.cardName, dev.portName}));
            auto enabled = isEnabled(dev.cardName, dev.portName);
            list = m_pMainWid->getDeviceManagetPortList(EnumToInt(type));
            qDebug() << QString("Set card: %1 port: %2 enable: %3.").arg(dev.cardName).arg(dev.portName).arg(enabled);
            m_pMainWid->setDeviceManagerItemStatus(EnumToInt(type), list.size() - 1, enabled);
        }
    }

    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        auto deviceIt = std::find_if(deviceList.begin(), deviceList.end(),
                                     [it](const ClientDeviceInfo& info) {
                                         return (*it).cardName == info.cardName && (*it).portName == info.portName;
                                     });

        // 没找到则移除
        if (deviceIt == deviceList.end()) {
            qDebug() << QString("can not found device in device manager widget, remove card %1 port %2").arg((*it).cardName).arg((*it).portName);
            m_pMainWid->removeDeviceManagerItem(EnumToInt(type), (*it).index);
        }
    }
}

void MainWidget::updateAppDeviceList(QList<ClientDeviceInfo>& list1, const QList<ClientDefaultDeviceInfo>& list2)
{
    for (auto it = list1.begin(); it != list1.end(); ) {
        auto matchIt = std::find_if(list2.begin(), list2.end(), [it](const ClientDefaultDeviceInfo& info) {
            return info.activePortName == it->portName;
        });

        if (matchIt == list2.end()) {
            it = list1.erase(it);
        } else {
            ++it;
        }
    }
}

void MainWidget::initAppManagerList()
{
    qDebug() << __func__ << "enter";
    auto append = [&](int dir, int index, double vol, bool mute, const QString& icon, const QString& name) {
        qDebug() << "initAppManagerList append" << "dir" << dir << "index" << index << "icon" << icon
                 << "name" << name;
        addAppManagerItem(dir, index, vol, mute, icon, name);
        initAppCurrentDefaultDevice(m_appWidgetList.size() - 1
                                    , (dir ? NodeType::OUTPUT : NodeType::INPUT), index);
//        initAppCurrentDefaultDevice(m_appWidgetList.size() - 1, NodeType::INPUT, index);
    };

    // 应用管理程序---系统Item项处理
    int volumeValue = getVolume(EnumToInt(NodeType::OUTPUT), -1);
    bool muteStatus = getMute(EnumToInt(NodeType::OUTPUT), -1);
    for (const auto& [k, v] : m_appKeys) {
        /**
         * addAppManagerItem()已设置输出combobox选中项
         * 继续设置系统默认输入设备时，因input-combobox默认状态为disable
         * 需先启用combobox，再进行初始化默认输入设备
         */
        addAppManagerItem(1, MAX_INDEX, volumeValue, muteStatus, v.at(0), v.at(1));
        m_pMainWid->setAppCbxEnabled(m_kylinSettingsSystemStr.data(), NodeType::INPUT, true);
        initAppCurrentDefaultDevice(0, NodeType::INPUT, MAX_INDEX);

        if (getValue("getVolumeBoostStatus").toBool()) {
            auto val = getValue("getVolumeBoostValue").toInt();
            setAppManagerSliderRange(0, val);
            setAppManagerSliderValue(volumeValue);
        }
    }

    for (const auto& stream : getStreamMediaList()) {
        if ((stream.mediaRole.compare("filter") == 0) ||
                (stream.mediaRole.compare("abstract") == 0)) {
            continue;
        }

        for (const auto& media : stream.mediaInfoList) {
            if ((stream.iconName.compare("ukui-control-center") == 0)) {
                continue;
            }
            append(media.direction, media.index, media.volume, media.mute, stream.iconName, media.name);
        }
    }

    if (!m_appKeys.empty())
        m_pMainWid->setCurrentAppItem(0);
}

void MainWidget::initAppCurrentDefaultDevice(int itemIdx, const NodeType& type, int streamIdx)
{
    qDebug() << "initAppCurrentDefaultDevice itemIdx" << itemIdx << "type" << EnumToInt(type) << "streamIdx" << streamIdx;
    if (!m_pMainWid->getAppManagerWidget())
        return;

    if (m_appWidgetList.empty()) {
       return;
    }

    ClientDefaultDeviceInfo devInfo;

    if (itemIdx == 0) {
        devInfo = getDefaultDevice(EnumToInt(type), streamIdx);
    }
    else {
        auto t = EnumToInt(type == NodeType::OUTPUT ? NodeType::SINK_INPUT : NodeType::SOURCE_OUTPUT);
        devInfo = getDefaultDevice(t, streamIdx);
    }

    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(),
        [itemIdx](const AppManagerItem& item) { return itemIdx == item.index; });

    if (it != m_appWidgetList.end()) {
        auto cbx = (*it).wid->getSelectComboxItems()[type];
        auto index = cbx->findData(QVariant(std::initializer_list<QString> {devInfo.cardName, devInfo.activePortName}));
        qDebug() << QString("Set item cardName :%1 activePortName :%2 streamIdx :%3.").arg(devInfo.cardName).arg(devInfo.activePortName).arg(streamIdx);
        if (index >= 0) {
            m_pMainWid->setAppCurrentIndex((*it).iconName, type, index);
            qDebug() << QString("Set itemIdx %1 type %2 current index to %3.").arg(itemIdx).arg(EnumToInt(type)).arg(index);
        } else {
            qWarning() << QString("Failed to find device in combobox, cardName: %1, activePortName: %2.").arg(devInfo.cardName).arg(devInfo.activePortName);
        }
    }
    else
        qDebug() << QString("Invaild %1 index in app manager, set current index failed.").arg(itemIdx);


//    for (const auto& app : m_appWidgetList) {
////        auto vec = (type == NodeType::OUTPUT) ? app.outputStreamVec : app.inputStreamVec;
//        auto it = std::find(vec.begin(), vec.end(), idx);

//        if (it != vec.end()) {
//            auto cbx = app.wid->getSelectComboxItems()[type];
//            auto index = cbx->findData(QVariant(std::initializer_list<QString> {devInfo.cardName, devInfo.activePortName}));
//            m_pMainWid->setAppCurrentIndex(app.iconName, type, index);
//            qDebug() << QString("Set %1 current index to %2.").arg(app.index).arg(index);
//            return;
//        }
//    }

}

void MainWidget::initSwitchButtonStatus()
{
    for (const auto& it : m_switchBtnItemMap) {

        QString method;
        switch (it.first) {
        case SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST: {
            method = "getVolumeBoostStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_MONO: {
            method = "getMonoStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL: {
            method = "getEchoCancelStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_LOOPBACK: {
            method = "getLoopbackStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_STARTUP: {
            method = "getStartupStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_POWEROFF: {
            method = "getPoweroffStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_LOGOUT: {
            method = "getLogoutStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_WAKEUP: {
            method = "getWakeupStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_ALERT: {
            method = "getAlertStatus";
            break;
        }
        case SwitchButtonType::SWITCH_BUTTON_AUTO_PAUSE: {
            ClientMethod::getInstance().setChecked(EnumToInt(it.first), getAutoPauseStatus());
            break;
        }
        default:
            break;
        }

        if ("" != method) {
            auto status = getValue(method).toBool();
            ClientMethod::getInstance().setChecked(EnumToInt(it.first), status);
            m_pMainWid->updateUIByButtonStatus(EnumToInt(it.first), status);

            if (status && SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST == it.first) {
                auto val = getValue("getVolumeBoostValue").toInt();
                ClientMethod::getInstance().setRange(EnumToInt(VolumeSliderType::VOLUME_SLIDER_OUTPUT), 0, val);
            }

            else if (status && SwitchButtonType::SWITCH_BUTTON_MONO == it.first) {
                ClientMethod::getInstance().setBalanceEnabled(false);
            }
        }
    }
}

void MainWidget::initSoundThemeList()
{
    auto list = getSoundThemeList();

    for (const auto& it : list) {
        ClientMethod::getInstance().addItem(EnumToInt(SelectComboxType::SELECT_COMBOX_SOUNDEFFECT), it.description, QVariant(it.name));
    }
    // 添加自定义音效
    ClientMethod::getInstance().addItem(EnumToInt(SelectComboxType::SELECT_COMBOX_SOUNDEFFECT), tr("Custom"), QVariant("custom"));

    updateSoundTheme();
}

/**
void MainWidget::initAppVolumeList()
{
    QDBusMessage replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getSinkInputList");

    if (replyList.arguments().isEmpty()) {
        qDebug() << "arguments is nullptr...";
        return;
    }

    const QDBusArgument& siArgs = replyList.arguments().at(0).value<QDBusArgument>();

    QList<StreamInfo> streamList;
    siArgs.beginArray();
    while (!siArgs.atEnd()) {
        StreamInfo info;
        siArgs >> info;
        streamList.push_back(info);
    }
    siArgs.endArray();


    replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                         AUDIO_OBJECT_PATH,
                                                         AUDIO_INTERFACE,
                                                         "getSourceOutputList");

    if (replyList.arguments().isEmpty()) {
        qDebug() << "arguments is nullptr...";
        return;
    }

    const QDBusArgument& soArgs = replyList.arguments().at(0).value<QDBusArgument>();
    soArgs.beginArray();
    while (!siArgs.atEnd()) {
        StreamInfo info;
        soArgs >> info;
        streamList.push_back(info);
    }
    soArgs.endArray();

    for (const auto& s : streamList) {

        if ("filter" != s.role) {
            TrayClientMethod::getInstance().addAppItem(s.index, s.iconName, s.name);
        }
    }
}
*/

void MainWidget::initSystemVolumeSlider()
{
    for (const auto& [k, v] : m_volumeSliderItemMap) {
        switch (k) {
        case VolumeSliderType::VOLUME_SLIDER_OUTPUT:
        case VolumeSliderType::VOLUME_SLIDER_INPUT: {
            auto muted = getMute(EnumToInt(k), -1);
            auto volume = getVolume(EnumToInt(k), -1);
            ClientMethod::getInstance().setValue(EnumToInt(k), volume);
            ClientMethod::getInstance().setMute(EnumToInt(k), muted);
            break;
        }
        case VolumeSliderType::VOLUME_SLIDER_BALANCE: {
            double v = getBalance(EnumToInt(NodeType::OUTPUT));
            auto balanceValue = v * 100;
            ClientMethod::getInstance().setValue(EnumToInt(k), balanceValue);
            break;
        }
        default:
            break;
        }
    }
}

/**
void MainWidget::initAppVolumeSlider()
{
    for (const auto& [k, v] : m_appVolumeSliderItemMap) {
        QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                         AUDIO_OBJECT_PATH,
                                                                         AUDIO_INTERFACE,
                                                                         "getVolume",
                                                                         EnumToInt(NodeType::SINK_INPUT), "");
        TrayClientMethod::getInstance().setValue(EnumToInt(NodeType::SINK_INPUT), k, reply);
    }

}
*/

void MainWidget::initData()
{
    initSystemDeviceList(NodeType::OUTPUT);
    initSystemDeviceList(NodeType::INPUT);
    initSwitchButtonStatus();
    initSoundThemeList();
    initSystemVolumeSlider();
//    initAppVolumeSlider();
}

void MainWidget::initSlots()
{
    if (m_pMainWid) {
        m_pMainWid->addVolumeObserver([this](int type, int idx, int value) {
            // 如果是应用音量的系统对应的滑动条的值发生更改需要设置默认输出设备音量
            if (type == 2 && idx == -1) {
                setMute(1, idx, false);
                setVolume(1, idx, value);
            }
            else {
                setMute(type, idx, false);
                setVolume(type, idx, value);
            }

            qDebug() << QString("Set %1 type volume, index: %2 volume changed to %3").arg(type).arg(idx).arg(value);
            // 暂时不记录应用管理的埋点数据
            if (type > 1)
                return;

            auto slider = m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->getSlider()->getWidget();
            ukcc::UkccCommon::buriedSettings("Audio", slider->objectName(), QString("slider"), QString::number(value));
        });

        m_pMainWid->addBalanceVolumeObserver([this](int type, float value) {
            setBalance(1, value);

            // type = 2 VolumeSliderType::VOLUME_SLIDER_BALANCE
            qDebug() << QString("Set %1 type balance, balance changed to %2.").arg(type).arg(value);

            auto slider = m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->getSlider()->getWidget();
            ukcc::UkccCommon::buriedSettings("Audio", slider->objectName(), QString("slider"), QString::number(value));
        });

        m_pMainWid->addMuteObserver([this](int type, int idx) {
            // 如果是应用音量的系统对应的静音值发生更改需要设置默认输出设备静音状态
            bool mute;
            if (type == 2 && idx == -1) {
                mute = getMute(1, idx);
                setMute(1, idx, !mute);
            }
            else {
                mute = getMute(type, idx);
                setMute(type, idx, !mute);
            }

            qDebug() << QString("Set %1 type mute, index: %2 mute changed to %3").arg(type).arg(idx).arg(mute);

            // 暂时不记录应用管理的埋点数据
            if (type > 1)
                return;

//            m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setMute(!muteStatus);
            auto muteBtn = m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->getMuteBtn();
            ukcc::UkccCommon::buriedSettings("Audio", muteBtn->objectName(), QString("settings"),
                                             !mute ? "true" : "false");
        });

        m_pMainWid->addDeviceObserver([this](int type, int idx, const QString& portName, const QString& cardName) {
            // 如果是应用音量的系统对应的静音值发生更改需要设置默认输出设备静音状态
            if (type == 2 && idx == -1) {
                setDefaultDevice(1, idx, portName, cardName);
            }
            else if (type == 3 && idx == -1) {
                setDefaultDevice(0, idx, portName, cardName);
            }
            else {
                setDefaultDevice(type, idx, portName, cardName);
            }

            qDebug() << QString("Set %1 type default device, index: %2 default device changed to card: %3 port %4").arg(type).arg(idx).arg(cardName).arg(portName);
            // 暂时不记录应用管理的埋点数据
            if (type > 1)
                return;

            auto selectCbx = static_cast<QListWidget*>(m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->getSelectBox()->getWidget());
            ukcc::UkccCommon::buriedSettings("Audio", selectCbx->objectName(), "select", selectCbx->currentItem()->text());
        });

        m_pMainWid->addSettingsObserver([this](const QString& key, const QVariant& value) {
            //start
            if (0 == strcmp(key.toLatin1().data(), "setMonoStatus")) {
                if (value.toBool()) {
                    setBalance(1, 0);
                }
            }
            //end
            setValue(key, value);

            if (0 == strcmp(key.toLatin1().data(), "setSoundThemeName")) {
                updateSoundTheme();
            } else if (0 == strcmp(key.toLatin1().data(), "setAutoPauseStatus")) {
                setAutoPauseStatus(value.toBool());
            }
            qDebug() << QString("%1 setting key changed to %2").arg(key).arg(value.toString());

            if (value.isValid() && value.toBool()) {
                playSoundByMethod(key);
            }

            if ((0 == strcmp(key.toLatin1().data(), "setVolumeChangedName"))
                    || (0 == strcmp(key.toLatin1().data(), "setNotifyGeneralName"))) {
                if (value.isValid() && !value.toString().isEmpty()) {
                    playAlretSoundFromPath(this, value.toString());
                }
            }
        });

        m_pMainWid->addDetailSettingsObserver([this](int type) {
            switch (IntToEnum<DetailSettingType>(type)) {
            case DetailSettingType::DETAIL_SETTING_DEVMANAGER: {
                initDeviceManagerList(NodeType::INPUT);
                initDeviceManagerList(NodeType::OUTPUT);
                break;
            }
            case DetailSettingType::DETAIL_SETTING_APPMANAGER: {
                initAppManagerList();
                break;
            }
            default:
                break;
            }

            qDebug() << QString("Click %1 detail settings.").arg(type);
        });

        m_pMainWid->addEnableDeviceObserver([this](const QString& card, const QString& port, bool enable) {
            setEnabled(card, port, enable);
            qDebug() << QString("Set card: %1 port: %2 enabled to %3").arg(card).arg(port).arg(enable ? "enable" : "disable");
        });
    }

    connect(&DBusClient::getInstance(), SIGNAL(volumeChangedSignal(int, int, const QDBusVariant&)),
            this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)));
    connect(&DBusClient::getInstance(), SIGNAL(muteChangedSignal(int, int, bool)),
            this, SLOT(muteChangedSlots(int, int, bool)));
    connect(&DBusClient::getInstance(), SIGNAL(deviceChangedSignal(int, const QString&, const QString&)),
            this, SLOT(deviceChangedSlots(int, const QString&, const QString&)));
    connect(&DBusClient::getInstance(), SIGNAL(deviceAdjustSignal(int)),
            this, SLOT(deviceAdjustSlots(int)));
    connect(&DBusClient::getInstance(), SIGNAL(settingsChangedSignal(const QString&, const QDBusVariant&)),
            this, SLOT(settingsChangedSlots(const QString&, const QDBusVariant&)));
    connect(&DBusClient::getInstance(), SIGNAL(addStreamSignal(int, int, int, bool, const QString&, const QString&)),
            this, SLOT(addStreamSlots(int, int, int, bool, const QString&, const QString&)));
    connect(&DBusClient::getInstance(), SIGNAL(removeStreamSignal(int, int)),
            this, SLOT(removeStreamSlots(int, int)));
}

void MainWidget::initClientManager()
{
    m_pClientManager = std::make_shared<ClientManager>(m_switchBtnItemMap, m_volumeSliderItemMap, m_selectComboxItemMap);
    DBusClient::getInstance().initDbusConnect();
    ClientMethod::getInstance().setManager(m_pClientManager);
}

void MainWidget::volumeChangedSlots(int type, int idx, const QDBusVariant& v)
{
    // 输入输出设备音量更改
    if (type <= 1) {
        ClientMethod::getInstance().setValue(type, v.variant().toInt());

        // 同步输出声道平衡音量
        if (type == 1) {
            // 无可用输出时不同步系统项音量，避免拔出设备后仍显示旧音量
            if (hasEnabledPort(NodeType::OUTPUT))
                m_pMainWid->setAppValue(m_kylinSettingsSystemStr.data(), v.variant().toInt());

            double v = getBalance(type);
            auto balanceValue = v * 100;
            ClientMethod::getInstance().setValue(EnumToInt(VolumeSliderType::VOLUME_SLIDER_BALANCE), balanceValue);
        }
    }
    else {
        // 其他应用项音量始终同步，不受无输出设备影响
        auto name = getAppStreamNameByIndex(IntToEnum<NodeType>(type), idx);
        m_pMainWid->setAppValue(name, v.variant().toInt());
    }

    qDebug() << QString("Type %1 index: %2 volume changed to %3.").arg(type).arg(idx).arg(v.variant().toInt());
}

void MainWidget::muteChangedSlots(int type, int idx, bool mute)
{
    // 输入输出设备音量更改
    if (type <= 1) {
        ClientMethod::getInstance().setMute(type, mute);
        // 仅同步系统项输出静音；无输出时保持系统项静音不可用
        if (type == 1 && hasEnabledPort(NodeType::OUTPUT))
            m_pMainWid->setAppMute(m_kylinSettingsSystemStr.data(), mute);
    }
    else {
        // 其他应用项静音始终同步，不受无输出设备影响
        auto name = getAppStreamNameByIndex(IntToEnum<NodeType>(type), idx);
        m_pMainWid->setAppMute(name, mute);
    }
    qDebug() << QString("Type %1 index %2  mute changed to %3.").arg(type).arg(idx).arg(mute);
}

void MainWidget::deviceChangedSlots(int type, const QString& portName, const QString& cardName)
{
    qDebug() << QString("%1 type device changed, card: %2 port %3").arg(type).arg(cardName).arg(portName);

    switch (IntToEnum<NodeType>(type)) {
    case NodeType::INPUT:
    case NodeType::OUTPUT: {
        int idx = ClientMethod::getInstance().findData(type, QVariant(std::initializer_list<QString>{cardName, portName}));
        auto volume = getVolume(type, -1);
        auto muted = getMute(type, -1);

        ClientMethod::getInstance().setCurrentIndex(type, idx);
        ClientMethod::getInstance().setValue(type, volume);
        ClientMethod::getInstance().setMute(type, muted);

        if (IntToEnum<NodeType>(type) == NodeType::OUTPUT) {
            double v = getBalance(type);
            int balanceValue = v * 100;
            ClientMethod::getInstance().setValue(EnumToInt(VolumeSliderType::VOLUME_SLIDER_BALANCE), balanceValue);
            if (hasEnabledPort(NodeType::OUTPUT))
                m_pMainWid->setAppValue(m_kylinSettingsSystemStr.data(), volume);

            // When default output is bluetooth or multi-bluetooth combine device, need to hide balance and mono frame
            const bool isBluetoothDevice = cardName.contains("blue");
            const bool isCombineSinkDevice = cardName == "combine-device" || portName == "output-combine";
            m_pMainWid->updateUIByDevice(EnumToInt(NodeType::OUTPUT), !isBluetoothDevice && !isCombineSinkDevice);
        }
        else {
            auto devInfo = getDefaultDevice(EnumToInt(NodeType::INPUT), -1);
            if (devInfo.name.contains(".monitor")) {
                m_pMainWid->stopMonitor();
            }
            else {
                m_pMainWid->startMonitor(devInfo.idx);
            }

            updateLoopbackFrameVisible(idx, devInfo);
        }

        initAppCurrentDefaultDevice(0, IntToEnum<NodeType>(type), -1);

        //start 同步应用项的设备信息以及更新应用项的选中设备
        if (m_appWidgetList.size() > 1) {
            updateAppWidgetDeviceList(IntToEnum<NodeType>(type), true);
        }
        //end
        break;
    }
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT: {
        // TODO
        break;
    }
    default:
        break;
    }
}

void MainWidget::deviceAdjustSlots(int type)
{
    qDebug() << __func__ << "type: " << type;
    initSystemDeviceList(NodeType::INPUT);
    initSystemDeviceList(NodeType::OUTPUT);

    initDeviceManagerList(NodeType::INPUT);
    initDeviceManagerList(NodeType::OUTPUT);

    updateAppWidgetDeviceList(NodeType::INPUT);
    updateAppWidgetDeviceList(NodeType::OUTPUT);
}

void MainWidget::settingsChangedSlots(const QString& key, const QDBusVariant& v)
{
    qDebug() << "setting changed slot, key: " << key << " value:" << v.variant();
    if (EVENT_SOUND_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_ALERT), v.variant().toBool());
    }
    else if (THEME_NAME_KEY == key) {
        ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_SOUNDEFFECT),
                                                    ClientMethod::getInstance().findData(2, v.variant()));
    }
    else if (CUSTOM_THEME_KEY == key) {
        if (v.variant().toBool()) {
            ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_SOUNDEFFECT),
                                                        ClientMethod::getInstance().findData(2, QVariant("custom")));
        }
    }
    else if (VOLUME_CHANGED_KEY == key) {
        updateSoundTheme();
        // ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST),
        //                                             ClientMethod::getInstance().findData(4, v.variant()));
    }
    else if (NOTIFY_GENERAL_KEY == key) {
        updateSoundTheme();
        // ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION),
        //                                             ClientMethod::getInstance().findData(4, v.variant()));
    }
    else if (MONO_AUDIO_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_MONO), v.variant().toBool());
        DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                 AUDIO_OBJECT_PATH,
                                                 AUDIO_INTERFACE,
                                                 "LoadModule",
                                                 "module-mono", "");

        auto isVaild = isValidDevice(1);
        ClientMethod::getInstance().setBalanceEnabled(isVaild && !v.variant().toBool());
    }
    else if (NOISE_REDUCTION_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_ECHOCANCEL), v.variant().toBool());
    }
    else if (LOOPBACK_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_LOOPBACK), v.variant().toBool());
    }
    else if (VOLUME_BOOST_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_VOLUMEBOOST), v.variant().toBool());

        int value = 100;
        if (v.variant().toBool()) {
            value = getValue("getVolumeBoostValue").toInt();
        }
        setAppManagerSliderRange(0, value);
        ClientMethod::getInstance().setRange(EnumToInt(VolumeSliderType::VOLUME_SLIDER_OUTPUT), 0, value);
    }
    else if (VOLUME_BOOST_VOLUME_KEY == key) {
        // TODO
    }
    else if (STARTUP_MUSIC_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_STARTUP), v.variant().toBool());
    }
    else if (ALERT_VOLUME_KEY == key) {
        ClientMethod::getInstance().setChecked(EnumToInt(SwitchButtonType::SWITCH_BUTTON_ALERT), v.variant().toBool());
    }
}

void MainWidget::addStreamSlots(int dire, int idx, int value, bool mute, const QString& iconName, const QString& desc)
{
    qDebug() << QString("Stream %1 added, stream name: %2, stream icon: %3.").arg(idx).arg(desc).arg(iconName);
    addAppManagerItem(dire, idx, value, mute, iconName, desc);
}

void MainWidget::removeStreamSlots(int direction, int idx)
{
    qDebug() << QString("Stream %1 removed.").arg(idx);

    for (auto& l : m_appWidgetList) {
        auto& vec = direction ? l.outputStreamVec : l.inputStreamVec;
        auto it = std::remove(vec.begin(), vec.end(), idx);

        if (it == vec.end())
            continue;

        vec.erase(it, vec.end());
        if (l.outputStreamVec.empty() && l.inputStreamVec.empty()) {
            m_pMainWid->removeAppManagerItem(l.iconName);
            break;
        }
        else if (vec.empty()) {
            m_pMainWid->setAppCbxEnabled(l.iconName, IntToEnum<NodeType>(direction), false);
        }
    }
}

DesktopEnvironmentVersion MainWidget::getDesktopEnvironmentVersion()
{
    QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getDesktopEnvironmentVersion");

    return m_version = IntToEnum<DesktopEnvironmentVersion>(reply);
}

int MainWidget::getVolume(int type, int idx)
{
    QDBusReply<int> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getVolume",
                                                                     type, idx);

    return reply;
}

void MainWidget::setVolume(int type, int idx, int vaule)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setVolume",
                                             type, idx, vaule);
}

double MainWidget::getBalance(int type) const
{
    QDBusReply<double> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                        AUDIO_OBJECT_PATH,
                                                                        AUDIO_INTERFACE,
                                                                        "getBalance",
                                                                        type);

    return reply;
}

void MainWidget::setBalance(int type, double v)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setBalance",
                                             type, v);
}

bool MainWidget::getMute(int type, int idx)
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getMute",
                                                                      type, idx);

    return reply;
}

void MainWidget::setMute(int type, int idx, bool muted)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setMute",
                                             type, idx, muted);
}

void MainWidget::setEnabled(const QString& card, const QString& port, bool enable)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setEnabled",
                                             card, port, enable);
}

bool MainWidget::isEnabled(const QString& card, const QString& port) const
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "isEnabled",
                                                                      card, port);

    return reply;
}

bool MainWidget::isValidDevice(int type) const
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "isValidDevice",
                                                                      1);
    return reply;
}

ClientDefaultDeviceInfo MainWidget::getDefaultDevice(int type, int idx)
{
    QDBusMessage replyList = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getDefaultDevice",
                                                                      type, idx);

    if (replyList.arguments().isEmpty()) {
        qWarning() << "Get default device failed, arguments is nullptr.";
        return {};
    }

    qDebug() << "Get default device, type:" << type << "deviceList: " << replyList;
    const QDBusArgument& siArgs = replyList.arguments().at(0).value<QDBusArgument>();
    ClientDefaultDeviceInfo info;
    siArgs >> info;
    return info;
}

QList<ClientDefaultDeviceInfo> MainWidget::getSinkList() const
{
    QList<ClientDefaultDeviceInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getSinkList");

    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDefaultDeviceInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

QList<ClientDefaultDeviceInfo> MainWidget::getSourceList() const
{
    QList<ClientDefaultDeviceInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getSourceList");

    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDefaultDeviceInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}


void MainWidget::setDefaultDevice(int type, int idx, const QString& portName, const QString& cardName)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setDefaultDevice",
                                             type, idx, portName, cardName);
}

void MainWidget::setValue(const QString& method, const QVariant& value)
{
    qDebug() << "set " << method << " value:" << value;

    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             SETTINGS_INTERFACE,
                                             method,
                                             value);
}

QVariant MainWidget::getValue(const QString& method) const
{
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  SETTINGS_INTERFACE,
                                                                  method);
    return reply.arguments().at(0);
}

QList<ClientSoundThemeInfo> MainWidget::getSoundThemeList()
{
    QList<ClientSoundThemeInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  SOUNDTHEME_INTERFACE,
                                                                  "getSoundThemeList");
    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientSoundThemeInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

QList<ClientSoundEffectFileInfo> MainWidget::getSoundEffectFileList(const QString& name)
{
    QList<ClientSoundEffectFileInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  SOUNDTHEME_INTERFACE,
                                                                  "getSoundEffectFileList",
                                                                  name);
    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientSoundEffectFileInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

QList<ClientDeviceInfo> MainWidget::getAvailablePortList(const NodeType& type)
{
    QList<ClientDeviceInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getAvailablePortList",
                                                                  EnumToInt(type));

    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDeviceInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

QList<StreamInfo> MainWidget::getSinkInputList() const
{
    QList<StreamInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getSinkInputList");

    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        StreamInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

QList<StreamInfo> MainWidget::getSourceOutputList() const
{
    QList<StreamInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getSourceOutputList");

    if (reply.arguments().isEmpty()) {
        qDebug() << "The returned parameter is empty.";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        StreamInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

QList<ClientStreamMediaInfo> MainWidget::getStreamMediaList() const
{
    qRegisterMetaType<ClientMediaInfo>();
    qRegisterMetaType<ClientStreamMediaInfo>();
    qDBusRegisterMetaType<ClientMediaInfo>();
    qDBusRegisterMetaType<ClientStreamMediaInfo>();
    QList<ClientStreamMediaInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getStreamMediaList");

    if (reply.arguments().isEmpty()) {
        qDebug() << "The returned parameter is empty.";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientStreamMediaInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

void MainWidget::updateSoundTheme()
{
    auto value = getValue("getSoundThemeName");
    qDebug() << "updateSoundTheme value :" << value.toString();

    auto fileList = getSoundEffectFileList(value.toString());
    qDebug() << "updateSoundTheme fileList :" << fileList.size();

    // 清空音量调节和通知音效中下拉框的内容
    ClientMethod::getInstance().clear(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST));
    ClientMethod::getInstance().clear(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION));

    ClientMethod::getInstance().addItem(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST), tr("None"), QVariant(tr("None")));
    ClientMethod::getInstance().addItem(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION), tr("None"), QVariant(tr("None")));

    if (!fileList.isEmpty()) {
        // init volumeChanged/notifygeneral combox
        for (const auto& f : fileList) {
            ClientMethod::getInstance().addItem(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST), f.description, QVariant(f.name));
            ClientMethod::getInstance().addItem(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION), f.description, QVariant(f.name));
        }
    }

    auto idx = ClientMethod::getInstance().findData(EnumToInt(SelectComboxType::SELECT_COMBOX_SOUNDEFFECT), value);
    ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_SOUNDEFFECT), idx);

    if (fileList.isEmpty()) {
        ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST), 0);
        ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION), 0);
    } else {
        value = getValue("getVolumeChangedName");
        idx = ClientMethod::getInstance().findData(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST), value);
        qDebug() << "updateSoundTheme getVolumeChangedName idx:" << idx << "value:" << value;
        if (idx < 0) {
            ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST), 0);
        } else{
            ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_VOLUMEADJUST), idx);
        }

        value = getValue("getNotifyGeneralName");
        idx = ClientMethod::getInstance().findData(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION), value);
        qDebug() << "updateSoundTheme getNotifyGeneralName idx:" << idx << "value:" << value;
        if (idx < 0) {
            ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION), 0);
        } else {
            ClientMethod::getInstance().setCurrentIndex(EnumToInt(SelectComboxType::SELECT_COMBOX_NOTIFICTION), idx);
        }
    }
}

bool MainWidget::getAutoPauseStatus() const
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getAutoPauseStatus");

    return reply;
}

void MainWidget::setAutoPauseStatus(bool status)
{
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setAutoPauseStatus",
                                             status);
}

bool MainWidget::getMultiAudioCombineStatus(const NodeType& type) const
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getMultiAudioCombineStatus",
                                                                      EnumToInt(type));

    return reply;
}

/**
 * @brief MainWidget::updateAppWidgetDeviceList
 * @param type
 * @param isOnlyUpdateAppList 只更新应用项的设备列表；系统项(index==0)使用完整可用端口，
 *                            应用项仅显示 sink/source 活跃端口（过滤 USB 非活跃配置如 iec958）
 */
void MainWidget::updateAppWidgetDeviceList(const NodeType& type, bool isOnlyUpdateAppList)
{
    if (m_appWidgetList.empty())
        return;

    auto fullDeviceList = getAvailablePortList(type);
    auto activeDeviceList = fullDeviceList;
    // 应用项只显示活跃端口：与 getSinkList/getSourceList 的 activePortName 比对后剔除非活跃端口
    if (type == NodeType::OUTPUT || type == NodeType::INPUT) {
        auto list = ((type == NodeType::OUTPUT) ? getSinkList() : getSourceList());
        updateAppDeviceList(activeDeviceList, list);
    }

    for (const auto& item : m_appWidgetList) {

        //start application item only show activeport
        if (isOnlyUpdateAppList) {
            if (item.index == 0 || item.outputStreamVec.empty()) {
                continue;
            }
        }
        //end

        // 系统项展示全部可用端口；普通应用项只展示活跃端口
        const auto& deviceList = (item.index == 0) ? fullDeviceList : activeDeviceList;

        auto portList = m_pMainWid->getAppManagetPortList(item.index, EnumToInt(type));
        for (const auto& dev : deviceList) {
            qDebug() << "updateAppWidgetDeviceList dev:" << dev.cardName << dev.portName << dev.enabled
                     << "item.index:" << item.index;
            if (!dev.enabled) {
                continue;
            }

            auto it = std::find_if(portList.begin(), portList.end(), [dev](const DevicePortInfo info) {
                return dev.cardName == info.cardName && dev.portName == info.portName;
            });
            // 没找到则添加
            if (it == portList.end()) {
                qDebug() << QString("Device adjust, add card: %1 port: %2 to %3.").arg(dev.cardName).arg(dev.portName).arg(item.index);
                m_pMainWid->addPortToAppItem(item.iconName, type, QString(dev.portLabel + " (" + dev.cardDesc + ")"),
                                             QVariant(std::initializer_list<QString> {dev.cardName, dev.portName}));
            }
        }

        for (auto it = portList.rbegin(); it != portList.rend(); ++it) {
            auto deviceIt = std::find_if(deviceList.begin(), deviceList.end(),
                                         [it](const ClientDeviceInfo& info) {
                                             return (*it).cardName == info.cardName && (*it).portName == info.portName;
                                         });

            // 没找到则移除
            if (deviceIt == deviceList.end()
                    || ((*deviceIt).enabled == false)) {
                qDebug() << QString("can not found device in app manager widget, remove card %1 port %2").arg((*it).cardName).arg((*it).portName);
                m_pMainWid->removePortFromAppItem(item.iconName, type, (*it).index);
            }
        }

        auto portListReal = m_pMainWid->getAppManagetPortList(item.index, EnumToInt(type));
        if (portListReal.empty()) {
            m_pMainWid->addPortToAppItem(item.iconName, type, tr("None"),
                                         QVariant(std::initializer_list<QString> {"None", "None"}));
        }
    }

    // 端口列表变化后仅同步系统项输出音量条；其他应用项不做修改
    if (type == NodeType::OUTPUT) {
        bool hasEnabledOutput = std::any_of(fullDeviceList.begin(), fullDeviceList.end(),
                                            [](const ClientDeviceInfo& info) { return info.enabled; });
        syncAppOutputVolumeControls(hasEnabledOutput);
    }

    // 端口列表变化后，将应用设备选择与 PulseAudio 实际路由同步
    for (const auto& item : m_appWidgetList) {
        if (item.index == 0) {
            continue;
        }
        if (type == NodeType::OUTPUT && !item.outputStreamVec.empty()) {
            initAppCurrentDefaultDevice(item.index, NodeType::OUTPUT, item.outputStreamVec.front());
        } else if (type == NodeType::INPUT && !item.inputStreamVec.empty()) {
            initAppCurrentDefaultDevice(item.index, NodeType::INPUT, item.inputStreamVec.front());
        }
    }
}

bool MainWidget::hasEnabledPort(const NodeType& type)
{
    const auto deviceList = getAvailablePortList(type);
    return std::any_of(deviceList.begin(), deviceList.end(),
                       [](const ClientDeviceInfo& info) { return info.enabled; });
}

void MainWidget::updateLoopbackFrameVisible(int comboxIdx, const ClientDefaultDeviceInfo& devInfo)
{
    const bool isNullorInternalDev =
            devInfo.name.contains("auto_null") ||
            devInfo.name.contains(".monitor") ||
            devInfo.activePortName.contains("internal") ||
            devInfo.activePortDesc.contains("Digital");
    const bool showLoopback =
            hasEnabledPort(NodeType::INPUT) &&
            comboxIdx != -1 &&
            !isNullorInternalDev;
    m_pMainWid->updateUIByDevice(EnumToInt(NodeType::INPUT), showLoopback);
}

void MainWidget::syncAppOutputVolumeControls(bool hasEnabledOutput)
{
    auto* appMgr = m_pMainWid->getAppManagerWidget();
    if (!appMgr)
        return;

    // 仅同步系统项：无输出时音量 0/静音/不可调；其他应用项保持现状不做修改
    int value = 0;
    bool muted = true;
    if (hasEnabledOutput) {
        value = getVolume(EnumToInt(NodeType::OUTPUT), -1);
        muted = getMute(EnumToInt(NodeType::OUTPUT), -1);
    }
    appMgr->updateOutputVolumeControls(m_kylinSettingsSystemStr.data(), hasEnabledOutput, value, muted);
}

QString MainWidget::getAppStreamNameByIndex(const NodeType& type, int index)
{
    auto list = (type == NodeType::SINK_INPUT) ? getSinkInputList() : getSourceOutputList();

    auto it = std::find_if(list.begin(), list.end(),
        [index](const StreamInfo& info) { return index == info.index; });

    if (it != list.end())
        return (*it).iconName;

    return {};
}

void MainWidget::addAppManagerItem(int dire, int idx, int value, bool mute, const QString& iconName, const QString& desc)
{
    if (!m_pMainWid->getAppManagerWidget())
        return;

    qDebug() << QString("Add %1 direction %2 index %3 to app widget").arg(iconName).arg(dire).arg(idx);
    auto outputDevList = getAvailablePortList(NodeType::OUTPUT);
    auto inputDevList = getAvailablePortList(NodeType::INPUT);
    auto type = dire ? NodeType::OUTPUT : NodeType::INPUT;

    //start application item only show activeport
    if (idx >= 0) {
        auto sinkList = getSinkList();
        auto sourceList = getSourceList();
        updateAppDeviceList(outputDevList, sinkList);
        updateAppDeviceList(inputDevList, sourceList);
    }
    //end

    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(),
        [iconName](const AppManagerItem& item) { return iconName == item.iconName; });

    if (it != m_appWidgetList.end()) {
        qDebug() << "stream list exit, enable combox...";
        m_pMainWid->setAppCbxEnabled(iconName, type, true);
        initAppCurrentDefaultDevice((*it).index, type, idx);

        if (dire)
            (*it).outputStreamVec.emplace_back(idx);
        else
            (*it).inputStreamVec.emplace_back(idx);

    }
    else {
        qDebug() << "stream list no exit, add addAppManagerItem...";
        m_pMainWid->addAppManagerItem(dire, idx, value, mute, iconName, desc, outputDevList, inputDevList);
        m_pMainWid->setAppCbxEnabled(iconName, type, true);
        initAppCurrentDefaultDevice(m_appWidgetList.size() - 1, type, idx);
    }
}

void MainWidget::setAppManagerSliderRange(int min, int max)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [](const AppManagerItem& item) {
        return 0 == item.index;
    });

    if (it != m_appWidgetList.end()) {
        (*it).wid->setRange(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, min, max);
    }
    else {
        qDebug() << QString("Can not found app widget, set app manager slider range failed.");
    }
}

void MainWidget::setAppManagerSliderValue(int value)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [](const AppManagerItem& item) {
        return 0 == item.index;
    });

    if (it != m_appWidgetList.end()) {
        (*it).wid->setValue(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, value);
    }
    else {
        qDebug() << QString("Can not found app widget, set app manager slider value failed.");
    }
}

void MainWidget::playSoundByMethod(QString method)
{
    if (0 == strcmp(method.toLatin1().data(), "setStartupStatus")) {
        playAlretSoundFromPath(this, SOUND_FILE_STARTUP_PATH);
    } else if (0 == strcmp(method.toLatin1().data(), "setPoweroffStatus")) {
        playAlretSoundFromPath(this, SOUND_FILE_SHUTDOWN_PATH);
    } else if (0 == strcmp(method.toLatin1().data(), "setLogoutStatus")) {
        playAlretSoundFromPath(this, SOUND_FILE_LOGOUT_PATH);
    } else if (0 == strcmp(method.toLatin1().data(), "setWakeupStatus")) {
        playAlretSoundFromPath(this, SOUND_FILE_WEAKUP_PATH);
    }
}

void MainWidget::playAlretSoundFromPath (QWidget *w,QString path)
{
    caPlayForWidget(w, 0,
                    CA_PROP_APPLICATION_NAME, "Sound Preferences",
                    CA_PROP_MEDIA_FILENAME, path.toLatin1().data(),
                    CA_PROP_EVENT_DESCRIPTION, "Testing event sound",
                    CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                    CA_PROP_APPLICATION_ID, "org.mate.VolumeControl",
                #ifdef CA_PROP_CANBERRA_ENABLE
                    CA_PROP_CANBERRA_ENABLE, "1",
                #endif
                    NULL);
}

int MainWidget::caPlayForWidget(QWidget *w, uint32_t id, ...)
{
    va_list ap;
    int ret;
    ca_proplist *p;

    if ((ret = ca_proplist_create(&p)) < 0)
    {
        qDebug() << "ca_proplist_create ret:" << ret;
        return ret;
    }

    if ((ret = caProplistSetForWidget(p, w)) < 0)
    {
        qDebug() << "caProplistSetForWidget ret:" << ret;
        return -1;
    }

    va_start(ap, id);
    ret = caProplistMergeAp(p, ap);
    va_end(ap);

    if (ret < 0)
    {
        qDebug() << "caProplistMergeAp ret:" << ret;
        return -1;
    }

    if (nullptr != m_caContext)
    {
        ca_context_cancel(m_caContext, 0);
        ca_context_destroy(m_caContext);
    }

    ca_context_create(&m_caContext);
    ret = ca_context_play_full(m_caContext, id, p, NULL, NULL);
    qDebug() << "ca_context_play_full ret:" << ret;

    return ret;
}

int MainWidget::caProplistMergeAp(ca_proplist *p, va_list ap)
{
    int ret;
    for (;;) {
        const char *key, *value;

        if (!(key = va_arg(ap, const char*)))
            break;

        if (!(value = va_arg(ap, const char*)))
            return CA_ERROR_INVALID;

        if ((ret = ca_proplist_sets(p, key, value)) < 0)
            return ret;
    }

    return CA_SUCCESS;
}

int MainWidget::caProplistSetForWidget(ca_proplist *p, QWidget *widget)
{
    int ret;
    const char *t;
    QScreen *screen;
    gint x = -1;
    gint y = -1;
    gint width = -1;
    gint height = -1;
    gint screen_width = -1;
    gint screen_height = -1;

    if ((t = widget->windowTitle().toLatin1().data()))
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_NAME, t)) < 0)
            return ret;

    if (t)
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_ID, t)) < 0)
            return ret;

    if ((t = widget->windowIconText().toLatin1().data()))
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_ICON_NAME, t)) < 0)
            return ret;
    if (screen = qApp->primaryScreen()) {
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_X11_SCREEN, "%i", 0)) < 0)
            return ret;
    }

    width = widget->size().width();
    height = widget->size().height();

    if (width > 0)
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_WIDTH, "%i", width)) < 0)
            return ret;
    if (height > 0)
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_HEIGHT, "%i", height)) < 0)
            return ret;

    if (x >= 0 && width > 0) {
        screen_width = qApp->primaryScreen()->size().width();

        x += width/2;
        x = CA_CLAMP(x, 0, screen_width-1);

        /* We use these strange format strings here to avoid that libc
                         * applies locale information on the formatting of floating
                         * numbers. */

        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_HPOS, "%i.%03i",
                                    (int) (x/(screen_width-1)), (int) (1000.0*x/(screen_width-1)) % 1000)) < 0)
            return ret;
    }

    if (y >= 0 && height > 0) {
        screen_height = qApp->primaryScreen()->size().height();

        y += height/2;
        y = CA_CLAMP(y, 0, screen_height-1);

        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_VPOS, "%i.%03i",
                                    (int) (y/(screen_height-1)), (int) (1000.0*y/(screen_height-1)) % 1000)) < 0)
            return ret;
    }
    return CA_SUCCESS;
}

void MainWidget::showAppControlWidget()
{
    if (m_pMainWid)
        m_pMainWid->showAppControlWidget();
}
