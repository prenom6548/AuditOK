/*
 * Copyright © 2013-2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#ifndef KMRE_MESSAGE_RECEIVER_H_
#define KMRE_MESSAGE_RECEIVER_H_

#include "communication/connection/kmre_service_loop.h"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio.hpp>
#include <functional>

namespace kmre {

class KmreMessageReceiver {
public:
    typedef std::function<void(boost::system::error_code const&, size_t)> KmreReadHandler;

    KmreMessageReceiver();
    KmreMessageReceiver(std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket);
    KmreMessageReceiver(const std::string &path, const std::shared_ptr<KmreLoop> &loop);
    ~KmreMessageReceiver();

    void async_read_socket_data(KmreReadHandler const& handle, boost::asio::mutable_buffers_1 const& buffer);
    void closeSocket();

protected:
    void initBoostSocket(std::shared_ptr<boost::asio::basic_stream_socket<boost::asio::local::stream_protocol>> const& socket);

private:
    std::shared_ptr<boost::asio::local::stream_protocol::socket> m_boost_stream_socket;
    int m_socket_fd;
};

} // namespace kmre

#endif // KMRE_MESSAGE_RECEIVER_H_
