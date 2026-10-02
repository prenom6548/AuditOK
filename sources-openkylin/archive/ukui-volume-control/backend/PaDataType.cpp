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

#include "PaDataType.h"

namespace UkuiAudioFramwork
{
std::string CardInfo::toString() const
{
    std::string pcil = "[";
    pcil += FORMAT("card.name={}, card.index={}, dirver={}, active_profile={}",
                   card.name,
                   card.index,
                   card.driver,
                   card.active_profile ? card.active_profile->name : "");
    pcil += "]";

    std::string psil = "[";
    for (const auto& s : sinks) {
        psil += "(";
        psil += s->toString();
        psil += "),";
    }
    psil += "]";

    std::string psol = "[";
    for (const auto& s : sources) {
        psol += "(";
        psol += s->toString();
        psol += "),";
    }
    psol += "]";

    return FORMAT("card={}, sinks={}, sources={}", pcil, psil, psol);
}

std::string PaDataInfo::toString() const
{
    std::string cil = "[";
    for (const auto& c : cardInfoList) {
        cil += "(";
        cil += c->toString();
        cil += "),";
    }
    cil += "]";

    std::string psil = "[";
    for (const auto& si : sinkInputList) {
        psil += "(";
        psil +=  si->toString();
        psil += "),";
    }
    psil += "]";

    std::string psol = "[";
    for (const auto& so : sourceOutputList) {
        psol += "(";
        psol += so->toString();
        psol += "),";
    }
    psol += "]";

    std::string pmil = "[";
    for (const auto& m : moduleInfoList) {
        pmil += "(";
        pmil += FORMAT("index={}, name={}, argument={}, auto_unload={}",
                       m.index, m.name, m.argument, m.auto_unload);
        pmil += "),";
    }
    pmil += "]";

    return FORMAT("cardInfoList={}, sinkInputList={}, sourceOutputList={}, moduleInfoList={}",
                  cil, psil, psol, pmil);
}

void PaCardInfo::operator =(const pa_card_info& other) {
    index = other.index;
    name = other.name;
    owner_module = other.owner_module;
    driver = other.driver;
    n_profiles = other.n_profiles;
    profiles = other.profiles ? std::make_shared<PaCardProfileInfo>(*other.profiles) : nullptr;
    active_profile = other.active_profile ? std::make_shared<PaCardProfileInfo>(*other.active_profile) : nullptr;
    proplist = other.proplist;
    n_ports = other.n_ports;
    deviceDesc = pa_proplist_gets(proplist, PA_PROP_DEVICE_DESCRIPTION) ? pa_proplist_gets(proplist, PA_PROP_DEVICE_DESCRIPTION) : "";
    deviceFactor = pa_proplist_gets(proplist, PA_PROP_DEVICE_FORM_FACTOR) ? pa_proplist_gets(proplist, PA_PROP_DEVICE_FORM_FACTOR) : "";

    ports.clear();
    for (int i = 0; i < n_ports; ++i) {
        bool enable = DeviceManagerModule::getInstance().isEnabled(other.name, other.ports[i]->name);
        if (other.ports[i]->available != PA_PORT_AVAILABLE_NO) {
            deviceList.push_back(std::make_shared<DeviceInfo>(other.name,
                                                              deviceDesc,
                                                              other.ports[i]->name,
                                                              other.ports[i]->description,
                                                              other.ports[i]->priority,
                                                              other.ports[i]->direction,
                                                              other.ports[i]->available,
                                                              enable,
                                                              getDevicePortType(other.name, other.ports[i]->name)));
        }
        ports.push_back(std::make_shared<PaCardPortInfo>(name, *other.ports[i], enable));
    }

    profiles2.clear();
    for (int i = 0; i < n_profiles; ++i) {
        profiles2.push_back(std::make_shared<PaCardProfileInfo2>(*other.profiles2[i]));
    }

    active_profile2 = other.active_profile2 ? std::make_shared<PaCardProfileInfo2>(*other.active_profile2) : nullptr;
}

std::string PaModuleInfo::toString() const
{
    return FORMAT("index: {}, name: {}, argument: {}, n_used: {}, auto_unload:{}",
        index, name, argument, n_used, auto_unload);
}

uint32_t valueToPaVolume(int value)
{
    return value / UKMEDIA_VOLUME_NORMAL * PA_VOLUME_NORMAL;
}

uint32_t paVolumeToValue(int value)
{
    return (value / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}

uint32_t paCvolumeToValue(const pa_cvolume& v)
{
    uint32_t value = 0;
    for (int i = 0; i < v.channels; ++i) {
        if (value < v.values[i]) {
            value = v.values[i];
        }
    }

    return (value / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}

uint32_t paCvolumeToPaValue(const pa_cvolume& v)
{
    uint32_t value = 0;
    for (int i = 0; i < v.channels; ++i) {
        if (value < v.values[i]) {
            value = v.values[i];
        }
    }

    return value;
}
}
