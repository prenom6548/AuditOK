/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "mpvlibload.h"

#include <QDebug>

MpvLibLoad* MpvLibLoad::m_instance = nullptr;

MpvLibLoad::MpvLibLoad() : libmpv_handle(nullptr), loaded(false) {
    // Initialize function pointers to nullptr
    mpv_client_api_version = nullptr;
    mpv_create = nullptr;
    mpv_initialize = nullptr;
    mpv_destroy = nullptr;
    mpv_set_option = nullptr;
    mpv_set_option_string = nullptr;
    mpv_command = nullptr;
    mpv_command_node = nullptr;
    mpv_set_property = nullptr;
    mpv_get_property = nullptr;
    mpv_get_property_string = nullptr;
    mpv_error_string = nullptr;
    mpv_free = nullptr;
    mpv_free_node_contents = nullptr;
    mpv_observe_property = nullptr;
    mpv_set_wakeup_callback = nullptr;
    mpv_terminate_destroy = nullptr;
    mpv_wait_event = nullptr;
    mpv_wakeup = nullptr;
    mpv_command_async = nullptr;
    mpv_set_property_async = nullptr;
    mpv_get_sub_api = nullptr;
    mpv_opengl_cb_set_update_callback = nullptr;
    mpv_opengl_cb_init_gl = nullptr;
    mpv_opengl_cb_draw = nullptr;
    mpv_opengl_cb_uninit_gl = nullptr;

    load();
}

MpvLibLoad *MpvLibLoad::getInstance()
{
    if(m_instance == nullptr)
        m_instance = new MpvLibLoad;
    return m_instance;
}

MpvLibLoad::~MpvLibLoad() {
    if (libmpv_handle) {
        dlclose(libmpv_handle);
    }
}

bool MpvLibLoad::load() {
    if (loaded) return true;

    libmpv_handle = dlopen("libmpv.so.1", RTLD_LAZY);
    if (!libmpv_handle) {
        libmpv_handle = dlopen("libmpv.so", RTLD_LAZY);
        if (!libmpv_handle) {
            std::cerr << "Failed to load libmpv: " << dlerror() << std::endl;
            return false;
        }
    }

    // Load client.h functions
    if (!loadSymbol(mpv_client_api_version, "mpv_client_api_version") ||
            !loadSymbol(mpv_create, "mpv_create") ||
            !loadSymbol(mpv_initialize, "mpv_initialize") ||
            !loadSymbol(mpv_destroy, "mpv_destroy") ||
            !loadSymbol(mpv_set_option, "mpv_set_option") ||
            !loadSymbol(mpv_set_option_string, "mpv_set_option_string") ||
            !loadSymbol(mpv_command, "mpv_command") ||
            !loadSymbol(mpv_command_node, "mpv_command_node") ||
            !loadSymbol(mpv_set_property, "mpv_set_property") ||
            !loadSymbol(mpv_get_property, "mpv_get_property") ||
            !loadSymbol(mpv_set_property_async, "mpv_set_property_async") ||
            !loadSymbol(mpv_get_property_string, "mpv_get_property_string") ||
            !loadSymbol(mpv_free, "mpv_free") ||
            !loadSymbol(mpv_free_node_contents, "mpv_free_node_contents") ||
            !loadSymbol(mpv_observe_property, "mpv_observe_property") ||
            !loadSymbol(mpv_wait_event, "mpv_wait_event") ||
            !loadSymbol(mpv_command_async, "mpv_command_async") ||
            !loadSymbol(mpv_set_wakeup_callback, "mpv_set_wakeup_callback") ||
            !loadSymbol(mpv_error_string, "mpv_error_string") ||
            !loadSymbol(mpv_terminate_destroy, "mpv_terminate_destroy") ||
            !loadSymbol(mpv_wakeup, "mpv_wakeup")) {
        qInfo() << "load client.h error";
        return false;
    }

    // Load opengl_cb.h functions
    if (!loadSymbol(mpv_get_sub_api, "mpv_get_sub_api") ||
            !loadSymbol(mpv_opengl_cb_set_update_callback, "mpv_opengl_cb_set_update_callback") ||
            !loadSymbol(mpv_opengl_cb_init_gl, "mpv_opengl_cb_init_gl") ||
            !loadSymbol(mpv_opengl_cb_draw, "mpv_opengl_cb_draw") ||
            !loadSymbol(mpv_opengl_cb_report_flip, "mpv_opengl_cb_report_flip") ||
            !loadSymbol(mpv_opengl_cb_uninit_gl, "mpv_opengl_cb_uninit_gl")) {
        qInfo() << "load opengl_cb.h error";
        return false;
    }

    loaded = true;
    return true;
}

template<typename T>
bool MpvLibLoad::loadSymbol(T &func, const char *symbol_name) {
    void* symbol = dlsym(libmpv_handle, symbol_name);
    if (!symbol) {
        std::cerr << "Failed to load symbol " << symbol_name << ": " << dlerror() << std::endl;
        return false;
    }
    func = reinterpret_cast<T>(symbol);
    return true;
}
