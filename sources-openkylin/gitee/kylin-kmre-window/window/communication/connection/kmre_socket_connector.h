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

#ifndef KMRE_SOCKET_CONNECTOR_H
#define KMRE_SOCKET_CONNECTOR_H

#include "kmre_service_loop.h"
#include "connection.h"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/exception/errinfo_errno.hpp>

namespace kmre {

class KmreSocketConnector {
public:
    explicit KmreSocketConnector(
      const std::string& socket_file, const std::shared_ptr<KmreLoop>& loop,
      const std::shared_ptr<BaseConnectionWorker<boost::asio::local::stream_protocol>>& connect_worker);
    ~KmreSocketConnector() noexcept;

    void accept_socket_client();
    void on_response_client_connection(std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket, boost::system::error_code const& err);

private:
    const std::string m_socket_file;
    std::shared_ptr<KmreLoop> m_loop;
    std::shared_ptr<BaseConnectionWorker<boost::asio::local::stream_protocol>> m_connect_worker;
    boost::asio::local::stream_protocol::acceptor m_boost_acceptor;
};

} // namespace kmre

#endif // KMRE_SOCKET_CONNECTOR_H
