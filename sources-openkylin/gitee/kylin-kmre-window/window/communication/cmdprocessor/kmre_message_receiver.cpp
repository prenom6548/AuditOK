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

#include "kmre_message_receiver.h"
#include "socket_helper.h"
#include <boost/system/error_code.hpp>
#include <sys/syslog.h>

namespace ba = boost::asio;

namespace kmre {

KmreMessageReceiver::KmreMessageReceiver()
{

}

KmreMessageReceiver::KmreMessageReceiver(std::shared_ptr<boost::asio::local::stream_protocol::socket> const &socket)
{
    initBoostSocket(socket);
}

KmreMessageReceiver::KmreMessageReceiver(const std::string &path, const std::shared_ptr<KmreLoop> &loop)
    : m_boost_stream_socket(std::make_shared<boost::asio::local::stream_protocol::socket>(loop->boostService()))
{
    boost::system::error_code err;
    m_boost_stream_socket->connect(boost::asio::local::stream_protocol::endpoint(path), err);
    if (err) {
        std::string msg = "Failed to connect to socket '" + path + "', err: " + err.message();
        syslog(LOG_ERR, "KmreMessageReceiver BOOST_THROW_EXCEPTION: %s.", msg.c_str());
        BOOST_THROW_EXCEPTION(std::runtime_error(msg));
    }
    
    initBoostSocket(m_boost_stream_socket);
}

KmreMessageReceiver::~KmreMessageReceiver()
{
    closeSocket();//TODO
}

void KmreMessageReceiver::initBoostSocket(std::shared_ptr<ba::basic_stream_socket<boost::asio::local::stream_protocol>> const& s)
{
    m_boost_stream_socket = s;
    m_socket_fd = m_boost_stream_socket->native_handle();//TODO:where destroy it?
    m_boost_stream_socket->non_blocking(true);
    boost::asio::socket_base::send_buffer_size option(64 * 1024);
    m_boost_stream_socket->set_option(option);

    kmre::network::setSocketCloexec(m_socket_fd);
}

void KmreMessageReceiver::async_read_socket_data(KmreReadHandler const& handler, ba::mutable_buffers_1 const& buffer)
{
    m_boost_stream_socket->async_read_some(buffer, handler);
}

void KmreMessageReceiver::closeSocket()
{
    m_boost_stream_socket->close();
}

}  // namespace kmre
