/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KMRE_CONNECTION_WORKER_H_
#define KMRE_CONNECTION_WORKER_H_

#include "connection.h"
#include "kmre_socket_manager.h"
#include "cmdprocessor/kmre_message_receiver.h"
#include "kmre_service_loop.h"
#include "cmdprocessor/kmre_message_parser.h"

namespace kmre {

class KmreConnectionWorker : public BaseConnectionWorker<boost::asio::local::stream_protocol> {
public:
    typedef std::function<std::shared_ptr<KmreMessageParser>(const std::shared_ptr<KmreMessageReceiver> &)>  KmreMessageFactory;

    KmreConnectionWorker(const std::shared_ptr<KmreLoop> &loop, const KmreMessageFactory &factory);
    ~KmreConnectionWorker() noexcept;

    uint32_t next_id();
    void add_connection_and_read_data(std::shared_ptr<boost::asio::basic_stream_socket<boost::asio::local::stream_protocol>> const &socket);

private:
    std::shared_ptr<KmreLoop> m_loop;
    std::atomic<uint32_t> m_connection_id;
    std::shared_ptr<ConnectionBox<KmreSocketManager>> const m_socket_connections;
    KmreMessageFactory m_factory;//function define in kmremanager.cpp
};

}  // namespace kmre

#endif // KMRE_CONNECTION_WORKER_H_
