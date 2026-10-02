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

#ifndef AUDIODBUSSERVER_H
#define AUDIODBUSSERVER_H

#include <dbus/dbus.h>
#include <gio/gio.h>
#include <iostream>
#include <atomic>
#include <string>

#include "Util.h"

//#define AUDIO_OBJECT_SERVICE "org.ukui.audio.framwork"
//#define AUDIO_OBJECT_PATH    "/org/ukui/audio/framwork"
//#define AUDIO_INTERFACE      "org.ukui.audio.framwork"
//#define SETTINGS_INTERFACE   "org.ukui.audio.settings"
namespace UkuiAudioFramwork
{
class AudioMethod;
class AudioDBusServer {
public:
    AudioDBusServer() = default;
    virtual ~AudioDBusServer();

public:
    void run();

    bool initialize();

    void sendSignal(const char*, const char*, const char*, GVariant*);

    void sendSignal(const char*, const char*, const char*, const char*, GVariant*);

private:
    static void handleMethodCall(GDBusConnection* connection,
                                 const char* sender,
                                 const char* objectPath,
                                 const char* interfaceName,
                                 const char* methodName,
                                 GVariant* parameters,
                                 GDBusMethodInvocation* invocation,
                                 gpointer userData);

private:
    static const char* m_pIntrospectionXml;
    static const char* m_pAudioSettingXml;
    static const char* m_pSoundThemeXml;
    static const char* m_pSoundThemePlayerXml;
    static GDBusInterfaceVTable m_interfaceVTable;
    GDBusConnection* m_pConnection = nullptr;
    GMainLoop* m_pMainLoop = nullptr;

};


}
#endif // AUDIODBUSSERVER_H
