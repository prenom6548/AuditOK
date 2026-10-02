/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "kmre_socket_manager.h"

#include <sys/syslog.h>

namespace ba = boost::asio;

namespace kmre {

KmreSocketManager::KmreSocketManager(
    std::shared_ptr<KmreMessageReceiver> const& message_receiver,
    uint32_t id,
    std::shared_ptr<ConnectionBox<KmreSocketManager>> const& connections,
    std::shared_ptr<KmreMessageParser> const& parser)
    : m_message_receiver(message_receiver),
      m_id(id),
      m_socket_connections(connections),
      m_message_parser(parser)
{

}

KmreSocketManager::~KmreSocketManager() noexcept
{

}

void KmreSocketManager::read_message()
{
    auto callback = std::bind(&KmreSocketManager::on_response_read_data, this, std::placeholders::_1, std::placeholders::_2);
    m_message_receiver->async_read_socket_data(callback, ba::buffer(m_buffer));
}

void KmreSocketManager::on_response_read_data(const boost::system::error_code& error, std::size_t bytes_read) {

    if (error) {
        //syslog(LOG_ERR, "KmreSocketManager::on_response_read_data: error caught, then will remove connetion's id: %s.", error.message().c_str());
        m_socket_connections->remove(id());
        return;
    }
    //syslog(LOG_DEBUG, "KmreSocketManager::on_response_read_data: bytes_read:%d.", bytes_read);

    std::vector<std::uint8_t> data(bytes_read);
    std::copy(m_buffer.data(), m_buffer.data() + bytes_read, data.data());
    if (m_message_parser->process_socket_data(data))
        read_message();//TODO: coredump
    else {
        syslog(LOG_ERR, "KmreSocketManager::on_response_read_data: process socket data fail.");
        m_socket_connections->remove(id());
    }
}

}  // namespace kmre
