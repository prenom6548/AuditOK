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

#include "kmre_connection_worker.h"
#include "communication/cmdprocessor/kmre_message_receiver.h"

#include <QDebug>
#include <sys/syslog.h>

namespace kmre {

KmreConnectionWorker::KmreConnectionWorker(const std::shared_ptr<KmreLoop> &loop, const KmreMessageFactory &factory)
    : m_loop(loop),
      m_connection_id(0),
      m_socket_connections(std::make_shared<kmre::ConnectionBox<KmreSocketManager>>()),
      m_factory(factory)
{

}

KmreConnectionWorker::~KmreConnectionWorker() noexcept
{
    //TODO
    //syslog(LOG_ERR, "KmreConnectionWorker::~KmreConnectionWorker() clear connections.");
    m_socket_connections->clear();
}

uint32_t KmreConnectionWorker::next_id()
{
    return m_connection_id.fetch_add(1);
}

void KmreConnectionWorker::add_connection_and_read_data(std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket)
{
    //method 1: 如果已经有socket连接了,则忽略新的socket
    /*if (m_socket_connections->size() >= 1) {
        printf("WARNING: (%s:%d) A second client tried to connect. Denied request as we already have one and only allow a single client!\n", __FUNCTION__, __LINE__);
        socket->close();
        return;
//        m_socket_connections->clear();
    }*/

    //method 2: 支持多个client的socket同时连接server,即如果已经有socket连接了,则新的socket也允许连接后进行数据传递
    auto const messenger = std::make_shared<KmreMessageReceiver>(socket);//TODO: close socket in KmreMessageReceiver::~KmreMessageReceiver()
    auto const parser = m_factory(messenger);
    auto const& socket_connection = std::make_shared<KmreSocketManager>(messenger, next_id(), m_socket_connections, parser);
    m_socket_connections->add(socket_connection);//SocketConnection中检测到socket异常时，会根据id清除connections_中的对应项
    socket_connection->read_message();
}

}  // namespace kmre
