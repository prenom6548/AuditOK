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

#include "MediaControler.h"

#include <iostream>
#include "AudioContext.h"


namespace UkuiAudioFramwork
{
const char* mprisPlayerDestPrefix = "org.mpris.MediaPlayer2";

MediaControler& MediaControler::getInstance()
{
    static MediaControler instance;
    return instance;
}

void MediaControler::pauseAllPlayers()
{
    g_autoptr(GError) error = nullptr;
    GDBusConnection* connection = nullptr;
    if (!connection) {
        connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    }
    g_autoptr(GDBusConnection) connectionGuard = connection;
    if (!connection) {
        if (error) {
            AUDIO_ERROR("Failed to get session bus connection: {}.", error->message);
        } else {
            AUDIO_ERROR("Failed to get session bus connection.");
        }
        return;
    }

    auto playerNames = getMprisPlayers(connection);
    if (playerNames.empty()) {
        AUDIO_ERROR("No MPRIS players found.");
        return;
    }

    for (const auto& playerName : playerNames) {
        g_autoptr(GError) err = nullptr;
        g_autoptr(GDBusProxy) playerProxy = g_dbus_proxy_new_sync(connection,
                                                                  G_DBUS_PROXY_FLAGS_NONE,
                                                                  nullptr,
                                                                  playerName.c_str(),
                                                                  "/org/mpris/MediaPlayer2",
                                                                  "org.mpris.MediaPlayer2.Player",
                                                                  nullptr,
                                                                  &err);
        if (!playerProxy) {
            AUDIO_ERROR("Failed to create proxy for player: {}, error: {}.", playerName, err->message);
            continue;
        }

        g_autoptr(GVariant) pauseResult = g_dbus_proxy_call_sync(playerProxy,
                                                                 "Pause",
                                                                 nullptr,
                                                                 G_DBUS_CALL_FLAGS_NONE,
                                                                 -1,
                                                                 nullptr,
                                                                 &err);
        if (!pauseResult) {
            AUDIO_ERROR("Failed to pause player:{}, error: {}.", playerName, err->message);
        } else {
            AUDIO_DEBUG("Pausing {}.", playerName);
        }
    }
}

std::vector<std::string> MediaControler::getMprisPlayers(GDBusConnection* connection) const
{
    std::vector<std::string> playerNames;
    GError* error = nullptr;
    GDBusProxy* proxy = g_dbus_proxy_new_sync(connection,
                                              G_DBUS_PROXY_FLAGS_NONE,
                                              nullptr,
                                              "org.freedesktop.DBus",
                                              "/org/freedesktop/DBus",
                                              "org.freedesktop.DBus",
                                              nullptr,
                                              &error);
    if (!proxy) {
        AUDIO_ERROR("Failed to create proxy: {}.", error->message);
        g_error_free(error);
        return playerNames;
    }

    GVariant* result = g_dbus_proxy_call_sync(proxy,
                                              "ListNames",
                                              nullptr,
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              nullptr,
                                              &error);
    if (!result) {
        AUDIO_ERROR("Failed to call ListNames: {}.", error->message);
        g_error_free(error);
    } else {
        GVariantIter* iter;
        const gchar* name;
        g_variant_get(result, "(as)", &iter);
        while (g_variant_iter_loop(iter, "&s", &name)) {
            if (g_str_has_prefix(name, mprisPlayerDestPrefix)) {
                playerNames.push_back(name);
            }
        }
        g_variant_iter_free(iter);
        g_variant_unref(result);
    }

    g_object_unref(proxy);
    return playerNames;
}
}
