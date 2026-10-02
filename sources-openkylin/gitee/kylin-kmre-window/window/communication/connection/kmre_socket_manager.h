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

#ifndef KMRE_SOCKET_MANAGER_H_
#define KMRE_SOCKET_MANAGER_H_

#include "connection.h"
#include "cmdprocessor/kmre_message_parser.h"
#include "cmdprocessor/kmre_message_receiver.h"

namespace kmre {

class KmreSocketManager {
public:
    KmreSocketManager(
      std::shared_ptr<KmreMessageReceiver> const& message_receiver,
      uint32_t id,
      std::shared_ptr<ConnectionBox<KmreSocketManager>> const& connections,
      std::shared_ptr<KmreMessageParser> const& parser);

    ~KmreSocketManager() noexcept;

    uint32_t id() const { return m_id; }
    void read_message();

private:
    void on_response_read_data(const boost::system::error_code& ec, std::size_t bytes_read);

    std::shared_ptr<KmreMessageReceiver> const m_message_receiver;
    uint32_t m_id;
    std::shared_ptr<ConnectionBox<KmreSocketManager>> const m_socket_connections;
    std::shared_ptr<KmreMessageParser> m_message_parser;
    std::array<std::uint8_t, 8192> m_buffer;
};

} // namespace kmre

#endif // KMRE_SOCKET_MANAGER_H_
