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

#include "kmre_message_parser.h"
#include "communication/cmd_signal_manager.h"
#include "communication/metatypes.h"
#include "kmreenv.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <sys/syslog.h>

namespace kmre {

KmreMessageParser::KmreMessageParser()
{
    
}

KmreMessageParser::~KmreMessageParser()
{

}

bool KmreMessageParser::process_socket_data(const std::vector<std::uint8_t> &data)
{
    for (const auto &byte : data) {
        m_buffer.push_back(byte);
    }

    //syslog(LOG_DEBUG, "KmreMessageParser::process_socket_data m_buffer.size(): %d.", m_buffer.size());
    while (m_buffer.size() > 0) {
        std::string result(m_buffer.begin(), m_buffer.end());//std::string result = std::string(m_buffer.begin(), m_buffer.end());
        parse_event_data_sequence(result);//解析数据序列
        m_buffer.erase(m_buffer.begin(), m_buffer.end());
    }

    return true;
}

void KmreMessageParser::handle_notification_event(const cn::kylinos::kmre::kmrecore::Notification &event)
{
    QString text = event.has_text() ? QString::fromStdString(event.text()) : "";
    int displayId = event.has_display_id() ? event.display_id() : -1;
    bool hide = event.has_hide() ? event.hide() : false;
    bool call = event.has_incoming_call() ? event.incoming_call() : false;
    QString title = event.has_title() ? QString::fromStdString(event.title()) : "";
    emit CmdSignalManager::getInstance()->sendNotification(
        QString::fromStdString(event.app_name()), 
        QString::fromStdString(event.package_name()), 
        text, 
        displayId, 
        hide,
        call, 
        title);
}

void KmreMessageParser::handle_event_info_event(const cn::kylinos::kmre::kmrecore::EventInfo &event)
{
    QString pkgName;
    int res = event.event_id();//std::int32_t to int
    if (event.has_package_name()) {
        pkgName = QString::fromStdString(event.package_name());
    }
    emit CmdSignalManager::getInstance()->sendEventInfo(res, pkgName);
}

void KmreMessageParser::handle_launch_result_event(const cn::kylinos::kmre::kmrecore::LaunchResult &event)
{
    int app_resumed = -1;
    if (event.has_app_resumed()) {
        app_resumed = event.app_resumed() ? 1 : 0;
    }

    if (event.result()) {
        bool exists = event.has_exists() ? event.exists() : false;
        syslog(LOG_DEBUG, "[%s] Launch app(%s) succeed.", __func__, event.package_name().c_str());
        emit CmdSignalManager::getInstance()->sendLaunchInfo(
            QString::fromStdString(event.app_name()), 
            QString::fromStdString(event.package_name()), 
            event.result(), 
            event.display_id(), 
            event.width(), 
            event.height(), 
            event.density(), 
            event.fullscreen(), 
            exists, 
            app_resumed);
    }
    else {
        syslog(LOG_ERR, "[%s] Launch app(%s) failed!", __func__, event.package_name().c_str());
    }
}

void KmreMessageParser::handle_close_result_event(const cn::kylinos::kmre::kmrecore::CloseResult &event)
{
    if (event.result()) {
        syslog(LOG_DEBUG, "[%s] Close app(%s) successed.", __func__, event.package_name().c_str());
        emit CmdSignalManager::getInstance()->sendCloseInfo(QString::fromStdString(event.package_name()));
    }
    else {
        syslog(LOG_ERR, "[%s] Close app(%s) failed!", __func__, event.package_name().c_str());
    }
}

void KmreMessageParser::handle_set_clipboard_event(const cn::kylinos::kmre::kmrecore::SetClipboard &event)
{
    emit CmdSignalManager::getInstance()->sendClipboardInfo(QString::fromStdString(event.content()));
}

void KmreMessageParser::handle_virtual_screen_focus_result_event(const cn::kylinos::kmre::kmrecore::VirtualScreenFocusResult &event)
{
    if (event.result()) {
        emit CmdSignalManager::getInstance()->sendFocusWinId(event.display_id());
    }
    else {
        syslog(LOG_ERR, "[%s] Focus display(%d) Failed!", __func__, event.display_id());
    }
}

void KmreMessageParser::handle_inputmethod_request_event(const cn::kylinos::kmre::kmrecore::InputMethodRequest &event)
{
    int x = event.has_x_pos() ? event.x_pos() : 0;
    int y = event.has_y_pos() ? event.y_pos() : 0;

    emit CmdSignalManager::getInstance()->sendInputMethodRequest(event.display_id(), QString::fromStdString(event.package_name()), event.ret(), x, y);
}

// 当event.type() = 0时，表示接收的时所有安卓文件数目，android会进行多次发送，每次发送N条，如5条，直到所有数据发送完毕
// event.size(): 需要接收的数据总条数
// event.item_size(): 本次接收的数据总条数
void KmreMessageParser::handle_files_list_event(const cn::kylinos::kmre::kmrecore::FilesList &event)
{
    if (event.size() < 1) {//size is a member variable of FilesList
        return;
    }

    AndroidMetaList metalist;
    //syslog(LOG_DEBUG, "KmreMessageParser::handle_files_list_event event.size():%d, event.item_size(): %d.", event.size(), event.item_size());
    // event.item_size() should be equal with event.size()
    for (int n = 0; n < event.item_size(); n++) {
        auto file = event.item(n);//SingleFile
        AndroidMeta meta;
        meta.path = QString::fromStdString(file.data());
        meta.mimeType = QString::fromStdString(file.mime_type());
        meta.pkgName = file.has_package_name() ? QString::fromStdString(file.package_name()) : "";
        metalist << meta;
    }
    if (metalist.length() > 0) {
        /* type 0: dump all mediafile info
                1: insert file
                2: delete file
        */
        emit CmdSignalManager::getInstance()->sendFilesList(event.type(), metalist, event.size());
    }
}

void KmreMessageParser::handle_mediaplay_status_event(const cn::kylinos::kmre::kmrecore::MediaPlayStatus &event)
{
    emit CmdSignalManager::getInstance()->sendMediaPlayStatus(event.result());
}

void KmreMessageParser::handle_app_multipliers_event(const cn::kylinos::kmre::kmrecore::AppMultiplierList &event)
{
    if (event.size() < 1) {//size is a member variable of AppMultiplierList
        return;
    }

    if (event.item_size() == event.size()) {
        QJsonArray jsonArray;
        for (int n = 0; n < event.item_size(); n++) {
            auto appMultiplier = event.item(n);//AppMultiplier
            QJsonObject jsonObject;
            jsonObject.insert("packageName", QString::fromStdString(appMultiplier.package_name()));
            jsonObject.insert("multiplier", appMultiplier.multiplier());
            jsonArray.append(jsonObject);
        }
        QJsonObject rootObject;
        if (!jsonArray.isEmpty()) {
            rootObject.insert("AppMultiplier", jsonArray);
        }

        QString jsonStr = QString(QJsonDocument(rootObject).toJson());
        emit CmdSignalManager::getInstance()->initAppMultipliersList(jsonStr);
    }
}

void KmreMessageParser::handle_get_info_list_event(const cn::kylinos::kmre::kmrecore::GetInfoList &event)
{
    Q_UNUSED(event)
}

void KmreMessageParser::handle_multiplier_switch_event(const cn::kylinos::kmre::kmrecore::MultiplierSwitch &event)
{
    QString packageName = QString::fromStdString(event.package_name());
    int display_id = event.event_id();
    bool enable = event.enable();

    emit CmdSignalManager::getInstance()->sendMultiplierSwitchInfo(packageName, display_id, enable);
}

void KmreMessageParser::handle_link_open_event(const cn::kylinos::kmre::kmrecore::LinkOpen &event)
{
    QString link = QString::fromStdString(event.url());
    if (!link.isEmpty()) {
        emit CmdSignalManager::getInstance()->requestOpenUrl(link);
    }
}

void KmreMessageParser::handle_update_package_status_event(const cn::kylinos::kmre::kmrecore::UpdatePackageStatus &event)
{
    QString pkgName = QString::fromStdString(event.package_name());
    int status = event.status();
    int type = event.has_type() ? event.type() : -1;
    
    if (!pkgName.isEmpty()) {
        emit CmdSignalManager::getInstance()->requestUpdatePackageStatus(pkgName, status, type);
    }
}

void KmreMessageParser::handle_response_info_event(const cn::kylinos::kmre::kmrecore::ResponseInfo &event)
{
    QString packageName = QString::fromStdString(event.package_name());
    QString category = QString::fromStdString(event.category());
    if (packageName.isNull() || packageName.isEmpty() || category.isNull() || category.isEmpty()) {
        return;
    }

    QString info = event.has_info() ? QString::fromStdString(event.info()) : "";
    emit CmdSignalManager::getInstance()->requestResponseInfo(event.event_id(), packageName, category, event.ret(), info);

}

void KmreMessageParser::handle_container_env_boot_status_event(const cn::kylinos::kmre::kmrecore::ContainerEnvBootStatus &event)
{
    bool status = event.status();
    QString errInfo = event.has_err_info() ? QString::fromStdString(event.err_info()) : "";
    emit CmdSignalManager::getInstance()->containerEnvBooted(status, errInfo);
}

void KmreMessageParser::parse_event_data_sequence(const std::string &raw_events)
{
    if (raw_events.empty()) {
        syslog(LOG_ERR, "KmreMessageParser::parse_event_data_sequence: raw_events is empty.");
        return;
    }

    cn::kylinos::kmre::kmrecore::EventSequence seq;
    if (!seq.ParseFromString(raw_events)) {
        syslog(LOG_ERR, "KmreMessageParser::parse_event_data_sequence: Failed to parse events from raw string:");
        return;
    }

    if (seq.has_notification()) {
        this->handle_notification_event(seq.notification());
    }
    else if (seq.has_event_info()) {
        this->handle_event_info_event(seq.event_info());
    }
    else if (seq.has_launch_result()) {
        this->handle_launch_result_event(seq.launch_result());
    }
    else if (seq.has_close_result()) {
        this->handle_close_result_event(seq.close_result());
    }
    else if (seq.has_set_clipboard()) {
        this->handle_set_clipboard_event(seq.set_clipboard());
    }
//    else if (seq.has_input_focus()) {
//        this->handle_input_focus_event(seq.input_focus());
//    }
    else if (seq.has_focus_result()) {
        this->handle_virtual_screen_focus_result_event(seq.focus_result());
    }
    else if (seq.has_inputmethod_request()) {
        this->handle_inputmethod_request_event(seq.inputmethod_request());
    }
    else if (seq.has_files_list()) {
        this->handle_files_list_event(seq.files_list());
    }
    else if (seq.has_mediaplay_status()) {
        this->handle_mediaplay_status_event(seq.mediaplay_status());
    }
    else if (seq.has_app_multipliers()) {
        this->handle_app_multipliers_event(seq.app_multipliers());
    }
    else if (seq.has_get_info_list()) {
         this->handle_get_info_list_event(seq.get_info_list());
    }
    else if (seq.has_response_info()) {
        this->handle_response_info_event(seq.response_info());
    }
    else if (seq.has_multiplier_switch()) {
        this->handle_multiplier_switch_event(seq.multiplier_switch());
    }
    else if (seq.has_link_open()) {
        this->handle_link_open_event(seq.link_open());
    }
    else if (seq.has_update_package_status()) {
        this->handle_update_package_status_event(seq.update_package_status());
    }
    else if (seq.has_container_env_boot_status()) {
        this->handle_container_env_boot_status_event(seq.container_env_boot_status());
    }
}

}  // namespace kmre
