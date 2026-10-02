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

#ifndef FCITX5_MODULE_H
#define FCITX5_MODULE_H

#include <string>

#include <fcitx-utils/dbus/bus.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/instance.h>

class VoiceService;

class VoiceWingModule : public fcitx::AddonInstance
{
public:
    VoiceWingModule(fcitx::Instance *instance);
    ~VoiceWingModule() override;

    bool hasFocusWindow() const;
    void commitString(const std::string &text, int flags);
    std::string appName() const;

    FCITX_ADDON_DEPENDENCY_LOADER(dbus, instance_->addonManager());

private:
    void registerDBusService();
    void unregisterDBusService();
    void connectSignal();
    void focusChanged();


private:
    fcitx::Instance *instance_;
    size_t committedCharCount_ = 0;
    fcitx::dbus::Bus *bus_ = nullptr;
    VoiceService *service_ = nullptr;
    std::vector<std::unique_ptr<fcitx::HandlerTableEntry<fcitx::EventHandler>>> events_;
};

class VoiceService : public fcitx::dbus::ObjectVTable<VoiceService>
{
public:
    explicit VoiceService(VoiceWingModule *parent) : parent_(parent)
    {
    }

    ~VoiceService() = default;

    void commitString(const std::string &text, int flags)
    {
        parent_->commitString(text, flags);
    }

    bool hasFocusWindow() const
    {
        return parent_->hasFocusWindow();
    }

    std::string appName() const
    {
        return parent_->appName();
    }

    FCITX_OBJECT_VTABLE_METHOD(commitString, "CommitString", "si", "");
    FCITX_OBJECT_VTABLE_METHOD(hasFocusWindow, "HasFocusWindow", "", "b");
    FCITX_OBJECT_VTABLE_METHOD(appName, "AppName", "", "s");

    FCITX_OBJECT_VTABLE_SIGNAL(focusChanged, "FocusChanged", "");

private:
    VoiceWingModule *parent_;
};

#endif // FCITX5_MODULE_H
