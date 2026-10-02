/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "module.h"

#include <dbus_public.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/addonfactory.h>
#include <fcitx/inputcontext.h>

VoiceWingModule::VoiceWingModule(fcitx::Instance *instance) : instance_(instance), bus_(dbus()->call<fcitx::IDBusModule::bus>())
{
    FCITX_INFO() << "Voice Module initialized";
    registerDBusService();
    connectSignal();
}

VoiceWingModule::~VoiceWingModule()
{
    unregisterDBusService();
}

void VoiceWingModule::connectSignal()
{
    events_.emplace_back(instance_->watchEvent(fcitx::EventType::InputContextFocusIn, fcitx::EventWatcherPhase::Default,
                                               [this](fcitx::Event & /*event*/)
                                               {
                                                   committedCharCount_ = 0;
                                                   focusChanged();
                                               }));

    events_.emplace_back(instance_->watchEvent(fcitx::EventType::InputContextFocusOut, fcitx::EventWatcherPhase::Default,
                                               [this](fcitx::Event & /*event*/)
                                               {
                                                   committedCharCount_ = 0;
                                                   focusChanged();
                                               }));
}

void VoiceWingModule::registerDBusService()
{
    service_ = new VoiceService(this);
    bus_->addObjectVTable("/voice", "org.fcitx.Fcitx5.Voice", *service_);
    bus_->requestName("org.fcitx.Fcitx5.Voice",
                      fcitx::Flags<fcitx::dbus::RequestNameFlag>{fcitx::dbus::RequestNameFlag::ReplaceExisting,
                                                                 fcitx::dbus::RequestNameFlag::Queue});
    bus_->flush();
}

void VoiceWingModule::unregisterDBusService()
{
    if (service_ != nullptr)
    {
        bus_->releaseName("org.fcitx.Fcitx5.Voice");
        delete service_;
        service_ = nullptr;
    }
}

bool VoiceWingModule::hasFocusWindow() const
{
    auto *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr)
    {
        return false;
    }

    return (inputContext->hasFocus() && !inputContext->capabilityFlags().test(fcitx::CapabilityFlag::Disable));
}

void VoiceWingModule::commitString(const std::string &text, int flags)
{
    auto *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr || text.empty())
    {
        return;
    }

    const auto charLen = fcitx::utf8::length(text);

    if (flags == 0)
    {
        inputContext->commitString(text);
        committedCharCount_ += charLen;
    }
    else if (flags == 1)
    {
        const auto n = static_cast<unsigned int>(committedCharCount_);
        if (inputContext->capabilityFlags().test(fcitx::CapabilityFlag::SurroundingText))
        {
            inputContext->deleteSurroundingText(-static_cast<int>(n), n);
        }
        else
        {
            const auto backspace = fcitx::Key(FcitxKey_BackSpace);
            for (unsigned int i = 0; i < n; ++i)
            {
                inputContext->forwardKey(backspace);
            }
        }
        inputContext->commitString(text);
        committedCharCount_ = 0;
    }
}

std::string VoiceWingModule::appName() const
{
    auto *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr)
    {
        return "";
    }

    return inputContext->program();
}

void VoiceWingModule::focusChanged()
{
    if (service_ == nullptr)
    {
        return;
    }

    service_->focusChanged();
}

class VoiceWingFactory : public fcitx::AddonFactory
{
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override
    {
        return new VoiceWingModule(manager->instance());
    }
};

FCITX_ADDON_FACTORY(VoiceWingFactory)