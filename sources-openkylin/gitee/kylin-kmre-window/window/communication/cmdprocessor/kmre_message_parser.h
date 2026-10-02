/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KMRE_MESSAGE_PARSER_H_
#define KMRE_MESSAGE_PARSER_H_

#include "communication/protobuf/KmreCore.pb.h"

#include <cstdint>
#include <vector>

namespace kmre {

class KmreMessageParser {
public:
    KmreMessageParser();
    ~KmreMessageParser();

    bool process_socket_data(const std::vector<std::uint8_t>& data);
    void parse_event_data_sequence(const std::string &event);
    void handle_notification_event(const cn::kylinos::kmre::kmrecore::Notification &event);
    void handle_event_info_event(const cn::kylinos::kmre::kmrecore::EventInfo &event);
    void handle_launch_result_event(const cn::kylinos::kmre::kmrecore::LaunchResult &event);
    void handle_close_result_event(const cn::kylinos::kmre::kmrecore::CloseResult &event);
    void handle_set_clipboard_event(const cn::kylinos::kmre::kmrecore::SetClipboard &event);
//    void handle_input_focus_event(const cn::kylinos::kmre::kmrecore::InputFocus &event);
    void handle_virtual_screen_focus_result_event(const cn::kylinos::kmre::kmrecore::VirtualScreenFocusResult &event);
    void handle_inputmethod_request_event(const cn::kylinos::kmre::kmrecore::InputMethodRequest &event);
    void handle_files_list_event(const cn::kylinos::kmre::kmrecore::FilesList &event);
    void handle_mediaplay_status_event(const cn::kylinos::kmre::kmrecore::MediaPlayStatus &event);
    void handle_app_multipliers_event(const cn::kylinos::kmre::kmrecore::AppMultiplierList &event);
    void handle_get_info_list_event(const cn::kylinos::kmre::kmrecore::GetInfoList &event);
    void handle_response_info_event(const cn::kylinos::kmre::kmrecore::ResponseInfo &event);
    void handle_multiplier_switch_event(const cn::kylinos::kmre::kmrecore::MultiplierSwitch &event);
    void handle_link_open_event(const cn::kylinos::kmre::kmrecore::LinkOpen &event);
    void handle_update_package_status_event(const cn::kylinos::kmre::kmrecore::UpdatePackageStatus &event);
    void handle_container_env_boot_status_event(const cn::kylinos::kmre::kmrecore::ContainerEnvBootStatus &event);
private:
    std::vector<std::uint8_t> m_buffer;
};

} // namespace kmre

#endif // KMRE_MESSAGE_PARSER_H_
