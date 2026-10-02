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

#include "kmre_socket_connector.h"
#include "socket_helper.h"
#include <QFile>

#include <sys/stat.h>
#include <fstream>
#include <sys/syslog.h>

namespace kmre {

KmreSocketConnector::KmreSocketConnector(
    const std::string& socket_file, const std::shared_ptr<KmreLoop>& loop,
    const std::shared_ptr<BaseConnectionWorker<boost::asio::local::stream_protocol>>& connect_worker)
    : m_socket_file(kmre::network::remove_socket_if_stale(socket_file))
      , m_loop(loop)
      , m_connect_worker(connect_worker)
      , m_boost_acceptor(loop->boostService(), m_socket_file)//当socket_file_不存在时，会抛异常退出程序
{
    if (QFile::exists(QString::fromStdString(m_socket_file))) {
        ::chmod(m_socket_file.c_str(), 0666);
    }

    kmre::network::setSocketCloexec(m_boost_acceptor.native_handle());

    accept_socket_client();
}

KmreSocketConnector::~KmreSocketConnector() noexcept
{

}

void KmreSocketConnector::accept_socket_client()
{
    // 在ASIO库中，异步方式的函数或方法名称前面都有“async_”前缀，函数参数里会要求放一个回调函数（或仿函数）
    auto socket = std::make_shared<boost::asio::local::stream_protocol::socket>(m_loop->boostService());
    m_boost_acceptor.async_accept(*socket,
                         [this, socket](boost::system::error_code const& err) {
                           on_response_client_connection(socket, err);
                         });

    /*
    // lixiang
    // 在ASIO中很多回调函数都只接受一个boost::system::error_code参数，在实际使用时肯定是不够的，所以一般使用仿函数携带一堆相关数据作为回调，或者使用boost::bind>来绑定一堆数据
    // 触发的事件只有error_code参数，所以用boost::bind把socket绑定进去
    m_boost_acceptor.async_accept(*socket, boost::bind(&KmreSocketConnector::on_response_client_connection, this, socket, _1));
    */
}

void KmreSocketConnector::on_response_client_connection(std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket, boost::system::error_code const& err)
{
    /*
    // lixiang: TODO
    if (err) {
        return;
    }
    */

    if (!err) {
        m_connect_worker->add_connection_and_read_data(socket);//读取客户端发送过来的数据
        kmre::network::setSocketCloexec(socket->native_handle());
    }

    if (err.value() == boost::asio::error::operation_aborted) {
        socket->close();//TODO
        syslog(LOG_ERR, "socket_connector: on_response_client_connection error value is boost::asio::error::operation_aborted.");
        return;
    }

    //if (err.value() == boost::system::errc::success) {
        //boost connection success
    //}

    accept_socket_client();
}

}  // namespace kmre
